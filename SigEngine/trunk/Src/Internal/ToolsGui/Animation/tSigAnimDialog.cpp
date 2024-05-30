#include "ToolsGuiPch.hpp"
#include "tSigAnimDialog.hpp"
#include "tApplication.hpp"
#include "FileSystem.hpp"
#include "tAssetGenScanner.hpp"
#include "tFileReader.hpp"
#include "tStrongPtr.hpp"
#include "..\tConfigurableBrowserTree.hpp"
#include "..\tProjectXMLDialog.hpp"
#include "tSigAnimNodeControlPanel.hpp"

#include "Momap.hpp"
#include "Animap.hpp"
#include "tMotionMap.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "tLoadInPlaceDeserializer.hpp"

#include "tAnimPackFile.hpp"
#include "tAnimPackData.hpp"
#include "tSkeletableSgFileRefEntity.hpp"


namespace Sig
{
	enum tAction
	{
		cActionNewDoc = 1,
		cActionOpenDoc,
		cActionOpenRecent,
		cActionSave = cActionOpenRecent + tSigAnimDialog::cMaxRecentlyOpenedFiles,
		cActionSaveAs,
		cActionBuild,
		cActionQuit,

		cActionUndo,
		cActionRedo,
		cActionCopy,
		cActionPaste,

		//cActionEditContexts,

		cActionAbout,

		cActionFrameSelected,
		cActionFrameAll
	};

	namespace
	{
		const std::string gMomapExt = Momap::fGetFileExtension( );

		b32 fIsMoMapFile( const tFilePathPtr& path )
		{
			return StringUtil::fCheckExtension( path.fCStr( ), gMomapExt.c_str( ) );
		}

		const std::string gAnimapExt = Animap::fGetFileExtension( );

		b32 fIsAniMapFile( const tFilePathPtr& path )
		{
			return StringUtil::fCheckExtension( path.fCStr( ), gAnimapExt.c_str( ) );
		}
	}

	class tMomapBrowser : public tConfigurableBrowserTree
	{
		tSigAnimDialog* mParent;

	public:
		tMomapBrowser( wxWindow* parent, tSigAnimDialog* mainParent, u32 minHeight )
			: tConfigurableBrowserTree( parent, fIsMoMapFile, minHeight, true, true )
			, mParent( mainParent )
		{ }

	private:
		virtual void fOpenDoc( const tFilePathPtr& file )
		{
			mParent->fOpenDoc( file );
		}
	};

	class tAnimapBrowser : public tConfigurableBrowserTree
	{
		tSigAnimDialog* mParent;

	public:
		tAnimapBrowser( wxWindow* parent, tSigAnimDialog* mainParent, u32 minHeight )
			: tConfigurableBrowserTree( parent, fIsAniMapFile, minHeight, true, true )
			, mParent( mainParent )
		{ }

	private:
		virtual void fOpenDoc( const tFilePathPtr& file )
		{
			//mParent->fOpenDoc( file );
		}
	};

	class tAnimPackTree : public wxTreeCtrl
	{
	public:
		tAnimPackTree( wxWindow* parent )
			: wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT | wxTR_SINGLE )
		{
			tAnimPackDataAgent::fInstance( );
			mOnResourceLoaded.fFromMethod<tAnimPackTree, &tAnimPackTree::fOnResourceLoaded>( this );
		}

		void fSetSkeleton( const tFilePathPtr& skelFile )
		{
			fUnloadResources( );

			DeleteAllItems( );
			tAnimPackDataAgent::fInstance( ).fGetAnimPacksForSkeleton( skelFile, mAnimPacks );

			wxTreeItemId root = AddRoot( "" ); 
			for( u32 i = 0; i < mAnimPacks.fCount( ); ++i )
			{
				tAnimPackData* data = new tAnimPackData( );
				data->mAnimPackFilepath = mAnimPacks[ i ]->fGetPath( ).fCStr( );

				std::string display = "[" + StringUtil::fNameFromPath( data->mAnimPackFilepath.c_str( ), true ) + "] " + data->mAnimPackFilepath;
				AppendItem( GetRootItem( ), display, -1, -1, data );

				mAnimPacks[ i ]->fLoadDefault( this );
				mAnimPacks[ i ]->fCallWhenLoaded( mOnResourceLoaded );
			}
		}

		void fUnloadResources( )
		{
			mAnimPacks.fSetCount( 0 );
		}

		void fOnResourceLoaded( tResource & theResource, b32 success )
		{
			tResourcePtr resPtr( &theResource );

			Freeze( );

			wxTreeItemIdValue cookie = this;
			wxTreeItemId nextItem = GetFirstChild( GetRootItem( ), cookie );
			while( nextItem.IsOk( ) )
			{
				wxTreeItemId item = nextItem;
				nextItem = GetNextChild( nextItem, cookie );

				tAnimPackData* data = (tAnimPackData*)GetItemData( item );

				if( data && data->mAnimPackFilepath == theResource.fGetPath( ).fCStr( ) )
				{
					if( fAddAnimations( item, resPtr ) )
						DeleteChildren( item );
				}
			}

			Thaw( );
		}

		b32 fAddAnimations( const wxTreeItemId& packNode, const tResourcePtr& apResource )
		{
			const tAnimPackFile * file = apResource->fCast<tAnimPackFile>( );
			if( !file )
			{
				log_warning( "Failed to add anim pack file[" << apResource->fGetPath( ) << "]" );
				return false;
			}

			const u32 animCount = file->mAnims.fCount( );
			for( u32 a = 0; a < animCount; ++a )
			{
				const tStringPtr & name = file->mAnims[a].mName->fGetStringPtr( );
				AppendItem( packNode, name.fCStr( ) );
			}
			return true;
		}

		b32 fGetCurrentSelection( std::string& animPackOut, std::string& animOut )
		{
			wxTreeItemId sel = GetSelection( );
			if( sel.IsOk( ) )
			{
				wxTreeItemId parent = GetItemParent( sel );
				if( parent.IsOk( ) )
				{
					tAnimPackData* data = (tAnimPackData*)GetItemData( parent );

					if( data )
					{
						animPackOut = tAnimPackFile::fConvertToSource( tFilePathPtr( data->mAnimPackFilepath ) ).fCStr( );
						animOut = GetItemText( sel );
						return true;
					}
				}
			}

			return false;
		}

	private:
		tGrowableArray< tResourcePtr > mAnimPacks;
		tResource::tOnLoadComplete::tObserver mOnResourceLoaded;

		struct tAnimPackData : public wxTreeItemData
		{
			std::string mAnimPackFilepath;
		};
	};

	class tAnimapEditor : public wxScrolledWindow
	{
	public:
		static const u32 cBorder = 5;

		tAnimapEditor( wxWindow* parent, tSigAnimDialog* siganim )
			: wxScrolledWindow( parent, wxID_ANY )
			, mDocName( tSigAnimDialog::cNewDocTitle )
			, mIsDirty( false )
			, mSigAnim( siganim )
		{

			wxSizer* vert = new wxBoxSizer( wxVERTICAL );
			SetSizer( vert );

			mStatus = new wxStaticText( this, wxID_ANY, "Status:" );
			mStatus->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ) );
			vert->Add( mStatus, 0, wxEXPAND | wxHORIZONTAL, cBorder );

			// document control buttons
			{
				wxSizer* horz = new wxBoxSizer( wxHORIZONTAL );
				vert->Add( horz, 0, wxEXPAND | wxHORIZONTAL, cBorder );

				wxButton* newButt = new wxButton( this, wxID_ANY, "New" );
				newButt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAnimapEditor::fNewDocCmd ), NULL, this );
				horz->Add( newButt, 0, wxEXPAND | wxHORIZONTAL );
				wxButton* openButt = new wxButton( this, wxID_ANY, "Open" );
				openButt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAnimapEditor::fOpenDocCmd ), NULL, this );
				horz->Add( openButt, 0, wxEXPAND | wxHORIZONTAL );
				wxButton* saveButt = new wxButton( this, wxID_ANY, "Save" );
				saveButt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAnimapEditor::fSaveDocCmd ), NULL, this );
				horz->Add( saveButt, 0, wxEXPAND | wxHORIZONTAL );
				wxButton* saveAsButt = new wxButton( this, wxID_ANY, "S. As" );
				saveAsButt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAnimapEditor::fSaveDocAsCmd ), NULL, this );
				horz->Add( saveAsButt, 0, wxEXPAND | wxHORIZONTAL );
			}

			wxSizer* horz = new wxBoxSizer( wxHORIZONTAL );
			vert->Add( horz, 2, wxEXPAND | wxALL );

			// Mo state stuff
			{
				wxSizer* vMostateSizer = new wxBoxSizer( wxVERTICAL );
				horz->Add( vMostateSizer, 1, wxEXPAND | wxALL, cBorder );

				wxStaticText* header = new wxStaticText( this, wxID_ANY, "Input Nodes:" );
				vMostateSizer->Add( header, 0, 0 );

				mAnimInputNodes = new wxComboBox( this, wxID_ANY );
				mAnimInputNodes->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( tAnimapEditor::fOnAnimInputSelected ), NULL, this );
				vMostateSizer->Add( mAnimInputNodes, 0, wxEXPAND | wxHORIZONTAL );

				header = new wxStaticText( this, wxID_ANY, "Anim Tracks:" );
				vMostateSizer->Add( header, 0, 0 );

				mAnimTracks = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( 100, wxDefaultSize.y ) );
				mAnimTracks->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxCommandEventHandler( tAnimapEditor::fUpdateAnimProps ), NULL, this );
				vMostateSizer->Add( mAnimTracks, 1, wxEXPAND | wxVERTICAL );
			}

			// Add remove track buttons.
			{
				wxSizer* vButtonsSizer = new wxBoxSizer( wxVERTICAL );
				horz->Add( vButtonsSizer, 0, wxEXPAND | wxALL, cBorder );

				const u32 cButtonSize = 20;
				wxButton* button = new wxButton( this, wxID_ANY, "<", wxDefaultPosition, wxSize( cButtonSize, cButtonSize ) );
				button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAnimapEditor::fAddAnim ), NULL, this );
				vButtonsSizer->Add( button, 0, 0 );
				button = new wxButton( this, wxID_ANY, "-", wxDefaultPosition, wxSize( cButtonSize, cButtonSize ) );
				button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAnimapEditor::fRemoveAnim ), NULL, this );
				vButtonsSizer->Add( button, 0, 0 );
				button = new wxButton( this, wxID_ANY, "+", wxDefaultPosition, wxSize( cButtonSize, cButtonSize ) );
				button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAnimapEditor::fAddSwitch ), NULL, this );
				vButtonsSizer->Add( button, 0, 0 );

				vButtonsSizer->AddSpacer( 20 );
				button = new wxButton( this, wxID_ANY, "?", wxDefaultPosition, wxSize( cButtonSize, cButtonSize ) );
				button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAnimapEditor::fRefreshInputNodes ), NULL, this );
				vButtonsSizer->Add( button, 0, 0 );
			}

			// anipack stuff
			{
				wxSizer* vAnipkSizer = new wxBoxSizer( wxVERTICAL );
				horz->Add( vAnipkSizer, 1, wxEXPAND | wxALL );

				wxStaticText* header = new wxStaticText( this, wxID_ANY, "Anipks:" );
				vAnipkSizer->Add( header, 0, 0 );

				mTree = new tAnimPackTree( this );
				vAnipkSizer->Add( mTree, 1, wxEXPAND | wxALL, 5 );
			}

			// Editable properties
			{
				// Property panel
				mPropertyPanel = new wxScrolledWindow( this );
				mPropertyPanel->SetBackgroundColour( wxColour( 0x55, 0x55, 0x55 ) );
				mPropertyPanel->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
				GetSizer( )->Add( mPropertyPanel, 1, wxEXPAND | wxALL, 0 );

				mOnPropertyChanged.fFromMethod< tAnimapEditor, &tAnimapEditor::fOnPropertyChanged >( this );
				mCommonProps.mOnPropertyChanged.fAddObserver( &mOnPropertyChanged );
			}

			fNewDoc( );
		}

		b32 fClearScene( )
		{
			if( mIsDirty )
			{
				const int result = wxMessageBox( "You have unsaved Animap changes - would you like to save them before resetting?",
					"Save Animap Changes?", wxYES | wxNO | wxCANCEL | wxICON_WARNING );

				if(			result == wxYES )			{ if( !fSaveDoc( false ) ) return false; }
				else if(	result == wxNO )			{ }
				else if(	result == wxCANCEL )		{ return false; }
				else									{ log_warning( "Unknown result returned from Message Box" ); }
			}

			mDocName = tSigAnimDialog::cNewDocTitle;
			mFile = Animap::tFile( );
			mIsDirty = false;

			return true;
		}

		void fNewDoc( )
		{
			if( !fClearScene( ) )
				return;

			fSetStatus( "New animap" );
			fPopulate( );
		}

		b32 fSaveDoc( b32 saveAs )
		{
			if( saveAs || mDocName == tSigAnimDialog::cNewDocTitle )
			{
				// browse for a new path
				tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
					this, 
					"Save Momap As",
					wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
					wxString( "untitled" + gAnimapExt ),
					wxString( "*" + gAnimapExt ),
					wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

				if( openFileDialog->ShowModal( ) != wxID_OK )
					return false; // cancelled

				mDocName = openFileDialog->GetPath( );
			}
			else
			{
				// not doing a save as; if we're not dirty, then skip
				if( !mIsDirty )
					return true;
			}

			if( !fSerialize( tFilePathPtr( mDocName.c_str( ) ) ) )
				return false;

			mIsDirty = false;
			fSetStatus( "Saved success" );
			return true;
		}

		void fOpenDoc( )
		{
			if( !fClearScene( ) )
				return; // user cancelled, don't try to open new file

			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				this, 
				"Open Momap",
				wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
				wxString( "untitled" + gAnimapExt ),
				wxString( "*" + gAnimapExt ),
				wxFD_OPEN ) );

			SetFocus( );

			if( openFileDialog->ShowModal( ) == wxID_OK )
			{
				fOpenDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
			}
		}

		void fOpenDoc( const tFilePathPtr& file )
		{
			if( !fClearScene( ) )
				return; // user cancelled, don't try to open new file

			if( FileSystem::fFileExists( file ) )
			{
				if( !fDeserialize( file ) )
				{
					wxMessageBox( "The specified .animap file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
					fSetStatus( "Open failed" );
					return;
				}

				// set up new scene
				mDocName = file.fCStr( );
				fSetStatus( "Open success" );
				fPopulate( );
			}
			else
			{
				wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
			}
		}

		void fNewDocCmd( wxCommandEvent& )
		{
			fNewDoc( );
		}

		void fOpenDocCmd( wxCommandEvent& )
		{
			fOpenDoc( );
		}

		void fSaveDocCmd( wxCommandEvent& )
		{
			fSaveDoc( false );
		}

		void fSaveDocAsCmd( wxCommandEvent& )
		{
			fSaveDoc( true );
		}

		void fSetStatus( const std::string& status )
		{
			mStatus->SetLabel( status + " " + StringUtil::fNameFromPath( mDocName.c_str( ) ) + ( mIsDirty ? "*" : "" ) );
		}

		b32 fSerialize( const tFilePathPtr& path )
		{
			// save to xml
			return mFile.fSaveXml( path, true );
		}

		b32 fDeserialize( const tFilePathPtr& path )
		{
			return mFile.fLoadXml( path );
		}

		void fPopulate( )
		{
			mAnimInputNodes->Clear( );
			for( u32 i = 0; i < mFile.mMappings.fCount( ); ++i )
			{
				mAnimInputNodes->Append( mFile.mMappings[ i ].mName );
			}

			fPopulateTracks( );
		}

		void fPopulateTracks( )
		{
			mAnimTracks->DeleteAllItems( );

			Animap::tFile::tMapping* curMap = fCurrentMapping( );
			if( curMap )
			{
				mAnimTracks->AddRoot( "Root" );
				mAnimTracks->SetItemData( mAnimTracks->GetRootItem( ), new tAnimTreeData( &curMap->mRoot ) );

				fPopulateTracksRecursive( curMap->mRoot, mAnimTracks->GetRootItem( ), true );
			}

			mAnimTracks->ExpandAll( );

			fUpdateAnimProps( wxCommandEvent( ) );
		}

		void fPopulateTracksRecursive( Animap::tFile::tContextSwitch& context, wxTreeItemId& parent, b32 isSwitch )
		{
			for( u32 i = 0; i < context.mBranches.fCount( ); ++i )
			{
				wxString text;

				if( isSwitch ) 
				{
					text += "Switch - ";
					const tProjectFile::tGameEnumeratedType* c = tProjectFile::fInstance( ).fFindEnumeratedTypeByKey( context.mBranches[ i ].mContextKey );
					text += c ? c->mName : StringUtil::fToString( (s32)context.mBranches[ i ].mContextKey );
				}
				else
				{
					// parent must be switch
					tAnimTreeData* parentData = (tAnimTreeData*)mAnimTracks->GetItemData( parent );
					sigassert( parentData && parentData->mIsSwitch );

					const tProjectFile::tGameEnumeratedType* c = tProjectFile::fInstance( ).fFindEnumeratedTypeByKey( parentData->mContext->mContextKey );
					if( !c )
					{
						wxMessageBox( "Error finding context. key: " + StringUtil::fToString( parentData->mContext->mContextKey ) );
						return;
					}

					u32 itemIndex = c->fFindValueIndexByKey( context.mBranches[ i ].mContextKey );
					if( itemIndex == ~0 )
					{
						wxMessageBox( "Error finding context value. key: " + StringUtil::fToString( context.mBranches[ i ].mContextKey ) );
						return;
					}

					text = c->mValues[ itemIndex ].mName;
				}
	

				wxTreeItemId node = mAnimTracks->AppendItem( parent, text );
				mAnimTracks->SetItemData( node, new tAnimTreeData( &context.mBranches[ i ], isSwitch ) );
				fPopulateTracksRecursive( context.mBranches[ i ], node, !isSwitch );
			}

			for( u32 i = 0; i < context.mLeaves.fCount( ); ++i )
			{
				wxTreeItemId node = mAnimTracks->AppendItem( parent, context.mLeaves[ i ].mAnimName + " - " + context.mLeaves[ i ].mAnipack );
				mAnimTracks->SetItemData( node, new tAnimTreeData( &context.mLeaves[ i ] ) );
			}
		}

		void fOnAnimInputSelected( wxCommandEvent& )
		{
			fPopulateTracks( );
		}

		Animap::tFile::tMapping* fCurrentMapping( )
		{
			s32 selIndex = mAnimInputNodes->GetSelection( );
			if( selIndex != -1 )
				return &mFile.mMappings[ selIndex ];
			else
				return NULL;
		}

		void fAddAnim( wxCommandEvent& )
		{
			std::string pack;
			std::string anim;

			Animap::tFile::tMapping* curMap = fCurrentMapping( );

			if( curMap )
			{
				wxTreeItemId selectedOutput = mAnimTracks->GetSelection( );
				if( selectedOutput.IsOk( ) )
				{
					tAnimTreeData* data = (tAnimTreeData*)mAnimTracks->GetItemData( selectedOutput );
					if( data && data->mContext && !data->mIsSwitch )
					{
						if( mTree->fGetCurrentSelection( pack, anim ) )
						{
							Animap::tFile::tAnimRef ref( pack, anim );

							if( !data->mContext->mLeaves.fFind( ref ) )
							{
								data->mContext->mLeaves.fPushBack( ref );
								mIsDirty = true;
								fSetStatus( "Updated" );
								fPopulateTracks( );
							}
						}
						else
							wxMessageBox( "Right side selection not appropriate.", "Error." );

						// left side was appropriate, that's how we got here. so dont log warning about that.
						return;
					}
				}
			}

			wxMessageBox( "Left side selection not appropriate.", "Error." );
		}

		template< typename t >
		static void fRemoveByAddress( t* item, tGrowableArray<t>& items )
		{
			for( u32 i = 0; i < items.fCount( ); ++i )
			{
				if( &items[ i ] == item )
				{
					items.fEraseOrdered( i );
					return;
				}
			}
		}

		void fRemoveAnim( wxCommandEvent& )
		{
			Animap::tFile::tMapping* curMap = fCurrentMapping( );

			if( curMap )
			{
				wxTreeItemId selectedOutput = mAnimTracks->GetSelection( );
				if( selectedOutput.IsOk( ) )
				{
					tAnimTreeData* data = (tAnimTreeData*)mAnimTracks->GetItemData( selectedOutput );
					if( data )
					{
						// now attain parent.
						wxTreeItemId parent = mAnimTracks->GetItemParent( selectedOutput );
						if( parent.IsOk( ) )
						{
							tAnimTreeData* parentData = (tAnimTreeData*)mAnimTracks->GetItemData( parent );
							sigassert( parentData );

							b32 hasChildren = data->mContext && (data->mContext->mLeaves.fCount( ) || data->mContext->mBranches.fCount( ));
							if( hasChildren && wxMessageBox( "This will remove all children of this node also. Remove?", "Remove children?", wxYES | wxNO ) == wxNO )
								return;
							
							// actually remove tree.
							if( data->mContext )
								fRemoveByAddress( data->mContext, parentData->mContext->mBranches );
							else if( data->mAnim )
								fRemoveByAddress( data->mAnim, parentData->mContext->mLeaves );

							mIsDirty = true;
							fSetStatus( "Updated" );
							fPopulateTracks( );

							// left side was appropriate, that's how we got here. so dont log warning about that.
							return;
						}
					}
				}
			}

			wxMessageBox( "Left side selection not appropriate.", "Error." );
		}

		void fAddSwitch( wxCommandEvent& )
		{
			Animap::tFile::tMapping* curMap = fCurrentMapping( );

			if( curMap )
			{
				wxTreeItemId selectedOutput = mAnimTracks->GetSelection( );
				if( selectedOutput.IsOk( ) )
				{
					tAnimTreeData* data = (tAnimTreeData*)mAnimTracks->GetItemData( selectedOutput );
					if( data && data->mContext )
					{
						if( data->mIsSwitch )
						{
							// add switch values for this switch
							wxArrayString choices;

							const tProjectFile::tGameEnumeratedType* context = tProjectFile::fInstance( ).fFindEnumeratedTypeByKey( data->mContext->mContextKey );
							if( !context )
							{
								wxMessageBox( "Could not find selected context!" );
								return;
							}

							for( u32 i = 0; i < context->mValues.fCount( ); ++i )
								choices.push_back( context->mValues[ i ].mName );

							s32 result = wxGetSingleChoiceIndex( "Add switch value: ", "Add switch value: ", choices, this );
							if( result > -1 && result < (s32)choices.size( ) )
							{
								data->mContext->mBranches.fPushBack( Animap::tFile::tContextSwitch( ) );
								data->mContext->mBranches.fBack( ).mContextKey = context->mValues[ result ].mKey;

								mIsDirty = true;
								fSetStatus( "Updated" );
								fPopulateTracks( );							
							}
						}
						else
						{
							// potentially add new switches
							wxArrayString choices;

							const tGrowableArray<tProjectFile::tGameEnumeratedType>& enumContexts = tProjectFile::fInstance( ).mGameEnumeratedTypes;
							for( u32 i = 0; i < enumContexts.fCount( ); ++i )
								choices.push_back( enumContexts[ i ].mName );

							s32 result = wxGetSingleChoiceIndex( "Add switch: ", "Add switch: ", choices, this );
							if( result > -1 && result < (s32)choices.size( ) )
							{
								data->mContext->mBranches.fPushBack( Animap::tFile::tContextSwitch( ) );
								data->mContext->mBranches.fBack( ).mContextKey = enumContexts[ result ].mKey;

								mIsDirty = true;
								fSetStatus( "Updated" );
								fPopulateTracks( );							
							}
						}

						// left side was appropriate, that's how we got here. so dont log warninga bout that.
						return;
					}
				}
			}

			wxMessageBox( "Left side selection not appropriate.", "Error." );
		}

		void fRefreshInputNodes( wxCommandEvent& )
		{
			const tDAGNodeCanvas::tDAGNodeList& list = mSigAnim->fCanvas( )->fAllNodes( );

			tGrowableArray< std::string > uniqueNames;

			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				const tAnimInputNode* animNode = dynamic_cast<tAnimInputNode*>( list[ i ].fGetRawPtr( ) );
				if( animNode )
				{
					uniqueNames.fFindOrAdd( animNode->fAnimName( ) );
				}
			}

			tGrowableArray< std::string > newNames;
			tGrowableArray< std::string > removedNames;

			for( u32 i = 0; i < mFile.mMappings.fCount( ); ++i )
			{
				if( !uniqueNames.fFind( mFile.mMappings[ i ].mName ) )
					removedNames.fPushBack( mFile.mMappings[ i ].mName );
			}

			for( u32 i = 0; i < uniqueNames.fCount( ); ++i )
			{
				if( !mFile.mMappings.fFind( uniqueNames[ i ] ) )
					newNames.fPushBack( uniqueNames[ i ] );
			}

			if( removedNames.fCount( ) || newNames.fCount( ) )
			{
				std::string msg;

				if( removedNames.fCount( ) )
				{
					msg += "This will REMOVE: \n";
					for( u32 i = 0; i < removedNames.fCount( ); ++i ) msg += removedNames[ i ] + "\n";
					msg += "\n";
				}

				if( newNames.fCount( ) )
				{
					msg += "This will add: \n";
					for( u32 i = 0; i < newNames.fCount( ); ++i ) msg += newNames[ i ] + "\n";
					msg += "\n";
				}

				msg += "\nAre you sure you want to proceed?";

				if( wxMessageBox( msg, "Confirm Changes", wxYES | wxNO | wxICON_QUESTION ) == wxYES )
				{
					for( u32 i = 0; i < removedNames.fCount( ); ++i )
						mFile.mMappings.fFindAndEraseOrdered( removedNames[ i ] );

					for( u32 i = 0; i < newNames.fCount( ); ++i )
						mFile.mMappings.fPushBack( Animap::tFile::tMapping( newNames[ i ] ) );					

					mIsDirty = true;
					fSetStatus( "Updated" );
					fPopulate( );
				}
			}
		}

		void fSetSkeleton( const tFilePathPtr& skeletonPath )
		{
			mTree->fSetSkeleton( skeletonPath );
		}

		const Animap::tFile& fFile( ) const { return mFile; }

	private:
		wxStaticText*	mStatus;
		wxComboBox*		mAnimInputNodes;
		wxTreeCtrl*		mAnimTracks;
		tAnimPackTree*	mTree;
		wxScrolledWindow* mPropertyPanel;
		tEditablePropertyTable mCommonProps;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnPropertyChanged;

		Animap::tFile	mFile;

		tSigAnimDialog* mSigAnim;

		b32 mIsDirty;
		std::string mDocName;

		struct tAnimTreeData : public wxTreeItemData
		{
			Animap::tFile::tContextSwitch* mContext;
			Animap::tFile::tAnimRef* mAnim;
			b32 mIsSwitch;

			tAnimTreeData( Animap::tFile::tContextSwitch* context = NULL, b32 isSwitch = false ) 
				: mContext( context ) 
				, mIsSwitch( isSwitch )
				, mAnim( NULL )
			{ }

			tAnimTreeData( Animap::tFile::tAnimRef* anim ) 
				: mContext( NULL ) 
				, mIsSwitch( false )
				, mAnim( anim )
			{ }
		};

		void fUpdateAnimProps( wxCommandEvent& )
		{
			Freeze( );
			mPropertyPanel->DestroyChildren( );
			mCommonProps.fClearGui( );

			wxTreeItemId selectedOutput = mAnimTracks->GetSelection( );
			if( selectedOutput.IsOk( ) )
			{
				tAnimTreeData* data = (tAnimTreeData*)mAnimTracks->GetItemData( selectedOutput );
				if( data && data->mAnim )
				{
					mCommonProps = data->mAnim->mProperties;
					mCommonProps.fCollectCommonPropertiesForGui( data->mAnim->mProperties );
					tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps, false );
				}
			}

			mPropertyPanel->Layout( );
			Layout( );
			Thaw( );
		}


		void fOnPropertyChanged( tEditableProperty& property )
		{
			mIsDirty = true;
			fSetStatus( "Updated" );
		}
	};

	// Deprecated, using GameFlags enum.

	//class tEditMoMapContextsDialog : public wxDialog
	//{
	//public:
	//	tEditMoMapContextsDialog( wxWindow* parent, Momap::tContextData* data )
	//		: wxDialog( parent, wxID_ANY, wxString( "Edit Momap Contexts" )  )
	//		, mData( data )
	//		, mChanged( false )
	//		, mSave( false )
	//	{ 
	//		wxBoxSizer* vSizer = new wxBoxSizer( wxVERTICAL );
	//		SetSizer( vSizer );

	//		wxStaticText* header = new wxStaticText( this, wxID_ANY, "Contexts: " );
	//		vSizer->Add( header, 0, wxEXPAND | wxHORIZONTAL );

	//		{
	//			wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
	//			vSizer->Add( hSizer, 0, wxEXPAND | wxHORIZONTAL );

	//			mContexts = new wxComboBox( this, wxID_ANY, "" );
	//			mContexts->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( tEditMoMapContextsDialog::fOnContextSelected ), NULL, this );
	//			hSizer->Add( mContexts, 1, wxEXPAND | wxHORIZONTAL );

	//			wxButton* b = new wxButton( this, wxID_ANY, "Add Context" );
	//			b->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditMoMapContextsDialog::fAddContext ), NULL, this );
	//			hSizer->Add( b, 0, 0 );

	//			b = new wxButton( this, wxID_ANY, "Remove Context" );
	//			b->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditMoMapContextsDialog::fRemoveContext ), NULL, this );
	//			hSizer->Add( b, 0, 0 );
	//		}

	//		header = new wxStaticText( this, wxID_ANY, "Values: " );
	//		vSizer->Add( header, 0, wxEXPAND | wxHORIZONTAL );

	//		{
	//			wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
	//			vSizer->Add( hSizer, 3, wxEXPAND | wxHORIZONTAL );

	//			mValues = new wxListBox( this, wxID_ANY );
	//			hSizer->Add( mValues, 1, wxEXPAND | wxALL );

	//			{
	//				wxSizer* vSizer = new wxBoxSizer( wxVERTICAL );
	//				hSizer->Add( vSizer, 0, wxEXPAND | wxVERTICAL );

	//				wxButton* b = new wxButton( this, wxID_ANY, "Add" );
	//				b->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditMoMapContextsDialog::fAddOption ), NULL, this );
	//				vSizer->Add( b, 0, 0 );

	//				b = new wxButton( this, wxID_ANY, "Remove" );
	//				b->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditMoMapContextsDialog::fRemoveOption ), NULL, this );
	//				vSizer->Add( b, 0, 0 );
	//			}

	//			mNewValues = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxMULTIPLE );
	//			hSizer->Add( mNewValues, 1, wxEXPAND | wxALL );
	//		}

	//		vSizer->AddSpacer( 10 );

	//		{
	//			wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
	//			vSizer->Add( hSizer, 0, wxEXPAND | wxHORIZONTAL );

	//			wxButton* b = new wxButton( this, wxID_ANY, "Save" );
	//			b->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditMoMapContextsDialog::fSave ), NULL, this );
	//			hSizer->Add( b, 0, wxEXPAND | wxHORIZONTAL );

	//			b = new wxButton( this, wxID_ANY, "Discard" );
	//			b->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditMoMapContextsDialog::fDiscard ), NULL, this );
	//			hSizer->Add( b, 0, 0 );
	//		}

	//		fPopulate( );

	//		Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( tEditMoMapContextsDialog::fOnClose ), NULL, this );
	//	}

	//	b32 fShowModal( )
	//	{
	//		ShowModal( );
	//		return mSave;
	//	}

	//	void fPopulate( )
	//	{
	//		mContexts->Clear( );

	//		for( u32 i = 0; i < mData->mContexts.fCount( ); ++i )
	//			mContexts->Append( mData->mContexts[ i ].mName );

	//		fPopulateValues( );
	//	}

	//	void fPopulateValues( )
	//	{
	//		mValues->Clear( );
	//		mNewValues->Clear( );

	//		Momap::tContext* c = fCurrentContext( );

	//		if( c )
	//		{
	//			for( u32 i = 0; i < c->mOptions.fCount( ); ++i )
	//				mValues->Append( c->mOptions[ i ].mName );
	//		}
	//	}

	//	Momap::tContext* fCurrentContext( )
	//	{
	//		s32 index = mContexts->GetSelection( );
	//		if( index != -1 )
	//		{
	//			return &mData->mContexts[ index ];
	//		}

	//		return NULL;
	//	}

	//	void fOnContextSelected( wxCommandEvent& )
	//	{
	//		fPopulateValues( );
	//	}

	//	void fAddContext( wxCommandEvent& )
	//	{
	//		wxString str = wxGetTextFromUser( "Enter Context Name: ", "Add context value.", "", this );
	//		std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
	//		if( result.length( ) > 0 )
	//		{
	//			mData->fAddContext( result );
	//			fPopulate( );
	//			mContexts->Select( mContexts->GetCount( ) - 1 );
	//			fPopulateValues( );
	//			fSetChanged( true );
	//		}
	//	}

	//	void fRemoveContext( wxCommandEvent& )
	//	{
	//		s32 id = mContexts->GetSelection( );
	//		if( id > -1 )
	//		{
	//			if ( wxMessageBox("Remove context: " + mData->mContexts[ id ].mName + "?",
	//				"Please confirm",
	//				wxICON_QUESTION | wxYES_NO) == wxYES )
	//			{
	//				mData->mContexts.fEraseOrdered( id );
	//				fPopulate( );
	//				mContexts->Select( id - 1);
	//				fPopulateValues( );
	//				fSetChanged( true );
	//			}
	//		}
	//	}

	//	wxString fInsertValues( const wxString& newLineDelVals, u32 insertIndex )
	//	{
	//		wxString notInserted;

	//		Momap::tContext* c = fCurrentContext( );
	//		sigassert( c );

	//		tGrowableArray< std::string > strs;
	//		StringUtil::fSplit( strs, newLineDelVals.c_str( ), "\n" );

	//		insertIndex = fMin( insertIndex, c->mOptions.fCount( ) );
	//		u32 lastIndex = insertIndex;

	//		for( u32 i = 0; i < strs.fCount( ); ++i )
	//		{
	//			std::string trimed = StringUtil::fEatWhiteSpace( strs[ i ] );
	//			trimed = StringUtil::fReplaceAllOf( trimed, "_", " " );
	//			if( c->fFindValueIndexByName( trimed ) == ~0 )
	//			{
	//				c->fInsertValue( trimed, insertIndex + i );
	//				lastIndex = insertIndex + i + 1;
	//			}
	//			else
	//				notInserted += trimed + "\n";
	//		}

	//		if( notInserted.length( ) > 0 )
	//			wxMessageBox( "Already exist:\n" + notInserted );

	//		fSetChanged( true );
	//		fPopulateValues( );
	//		mValues->SetSelection( lastIndex );

	//		return notInserted;
	//	}

	//	void fAddOption( wxCommandEvent& )
	//	{
	//		Momap::tContext* c = fCurrentContext( );
	//		if( c )
	//		{
	//			mNewValues->SetValue( fInsertValues( mNewValues->GetValue( ), -1 ) );
	//		}
	//		else
	//			wxMessageBox( "No context selected." );
	//	}
	//	
	//	void fRemoveOption( wxCommandEvent& )
	//	{
	//		Momap::tContext* c = fCurrentContext( );
	//		if( c )
	//		{
	//			s32 id = mValues->GetSelection( );
	//			if( id > -1 )
	//			{
	//				if ( wxMessageBox("Remove value: " + c->mOptions[ id ].mName + "?",
	//					"Please confirm",
	//					wxICON_QUESTION | wxYES_NO) == wxYES )
	//				{
	//					fSetChanged( true );
	//					c->mOptions.fEraseOrdered( id );
	//					fPopulateValues( );
	//					mValues->SetSelection( id );
	//				}
	//			}
	//			else
	//				wxMessageBox( "No context value selected." );
	//		}
	//		else
	//			wxMessageBox( "No context selected." );
	//	}

	//	void fSave( wxCommandEvent& )
	//	{
	//		mSave = true;
	//		mChanged = false;
	//		Close( );
	//	}

	//	void fDiscard( wxCommandEvent& )
	//	{
	//		Close( );
	//	}

	//	void fSetChanged( b32 changed )
	//	{
	//		mChanged = changed;
	//		
	//		wxString title = GetTitle( );
	//		if( mChanged && title.Last( ) != '*' )
	//			SetTitle( title + "*" );
	//	}

	//	void fOnClose( wxCloseEvent& event )
	//	{
	//		if ( mChanged && wxMessageBox("The file has not been saved... continue closing?",
	//			"Please confirm",
	//			wxICON_QUESTION | wxYES_NO) != wxYES )
	//		{
	//			event.Veto();
	//			return;
	//		}

	//		Destroy( );
	//		event.Skip( );
	//	}

	//private:
	//	b32 mChanged;
	//	b32 mSave;

	//	wxComboBox* mContexts;
	//	wxListBox* mValues;
	//	wxTextCtrl* mNewValues;

	//	Momap::tContextData* mData;
	//};


	const char tSigAnimDialog::cNewDocTitle[] = "(untitled)";

	tSigAnimDialog::tSigAnimDialog( )
		: wxFrame( (wxFrame *)NULL, -1, "SigAnim V2" )
		, mSavedLayout( ToolsPaths::fGetSignalRegistryKeyName( ) + "\\SigAnimV2\\MainWindow" )
		, mToolBar( 0 )
		, mCanvas( 0 )
		, mRecentFilesMenu( 0 )
		, mDocName( cNewDocTitle )
		, mRecentFiles( ToolsPaths::fGetSignalRegistryKeyName( ) + "\\SigAnimV2\\MainWindow\\" + ToolsPaths::fGetCurrentProjectName( ) + "\\RecentFiles" )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( tSigAnimDialog::fOnClose ) );
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tSigAnimDialog::fOnAction ) );
		Connect( wxEVT_IDLE, wxIdleEventHandler( tSigAnimDialog::fOnIdle ) );

		mRecentFiles.fLoad( );
		fCreateMainMenu( );
		fAddToolbar( );

		// status bar...
		const int statusWidths[] = { 0, 60, -1 };
		wxStatusBar* statusBar = CreateStatusBar( array_length(statusWidths), wxST_SIZEGRIP|wxFULL_REPAINT_ON_RESIZE, 0, wxT("Status:") );
		SetStatusWidths( array_length(statusWidths), statusWidths );
		SetStatusText( "      Status:", 1 );

		wxSizer* mainSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( mainSizer, 1, wxEXPAND | wxALL );

		// create canvas, control panel, and material editor

		wxBoxSizer* vSizer = new wxBoxSizer( wxVERTICAL );
		mainSizer->Add( vSizer, 2, wxALIGN_LEFT | wxEXPAND | wxALL );

		//{
		//	wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
		//	vSizer->Add( hSizer, 1, wxEXPAND | wxALL );

		//	////////////////////////////////////////////////////////////////////////////////
		//	{
		//		wxBoxSizer* browseSizer = new wxBoxSizer( wxVERTICAL );
		//		hSizer->Add( browseSizer, 1, wxEXPAND | wxALL );

		//		wxStaticText* header = new wxStaticText( this, wxID_ANY, "Momaps:" );
		//		browseSizer->Add( header, 0, 0 );

		//		tMomapBrowser* browse = new tMomapBrowser( this, this, wxDefaultSize.y );
		//		browseSizer->Add( browse, 1, wxEXPAND | wxALL, 5 );
		//		browse->fRefresh( );
		//	}

		//	////////////////////////////////////////////////////////////////////////////////
		//	{
		//		wxBoxSizer* browseSizer = new wxBoxSizer( wxVERTICAL );
		//		hSizer->Add( browseSizer, 1, wxEXPAND | wxALL );
		//		
		//		wxStaticText* header = new wxStaticText( this, wxID_ANY, "Animaps:" );
		//		browseSizer->Add( header, 0, 0 );

		//		tAnimapBrowser* browse2 = new tAnimapBrowser( this, this, wxDefaultSize.y );
		//		browseSizer->Add( browse2, 1, wxEXPAND | wxALL, 5 );
		//		browse2->fRefresh( );
		//	}
		//}

		mAnimapEditor = new tAnimapEditor( this, this );
		vSizer->Add( mAnimapEditor, 1, wxEXPAND | wxALL, tAnimapEditor::cBorder );

		// add node canvas...
		mCanvas = new tSigAnimNodeCanvas( this, this );
		mainSizer->Add( mCanvas, 6, wxALIGN_CENTER | wxEXPAND | wxALL );
		SetBackgroundColour( mCanvas->fBgColor( ) );

		// add control panel
		mControlPanel = new tSigAnimNodeControlPanel( this, mCanvas );
		mainSizer->Add( mControlPanel, 0, wxALIGN_RIGHT | wxEXPAND | wxALL );

		// load saved UI settings
		fLoadLayout( );

		// set delegates
		mOnDirty.fFromMethod<tSigAnimDialog, &tSigAnimDialog::fOnDirty>( this );
		mOnAddAction.fFromMethod<tSigAnimDialog, &tSigAnimDialog::fOnActionUndoOrRedo>( this );
		mOnSelChanged.fFromMethod<tSigAnimDialog,&tSigAnimDialog::fOnSelChanged>( this );
		mCanvas->mOnSelChanged.fAddObserver( &mOnSelChanged );
		mCanvas->fEditorActions( ).mOnDirty.fAddObserver( &mOnDirty );
		mCanvas->fEditorActions( ).mOnAddAction.fAddObserver( &mOnAddAction );
		mCanvas->fEditorActions( ).mOnUndo.fAddObserver( &mOnAddAction );
		mCanvas->fEditorActions( ).mOnRedo.fAddObserver( &mOnAddAction );

		fNewDoc( );

		mCanvas->SetFocus( );
	}
	tSigAnimDialog::~tSigAnimDialog( )
	{
	}
	void tSigAnimDialog::fSetStatus( const char* status )
	{
		SetStatusText( status, 2 );
	}
	void tSigAnimDialog::fLoadLayout( )
	{
		//if( mSavedLayout.fLoad( ) && mSavedLayout.mVisible )
		//{
		//	mSavedLayout.fToWxWindow( this );
		//	if( mSavedLayout.mMaximized )
		//		Maximize( );
		//}
		//else
		//{
			SetSize( 1024, 768 );
			Center( );
		//	Maximize( );
		//	Show( true );
		//}
	}
	void tSigAnimDialog::fSaveLayout( )
	{
		if( IsIconized( ) || !IsShown( ) )
			return; // window is minimized, don't save

		tWxSavedLayout layout( mSavedLayout.fRegistryKeyName( ) );
		layout.fFromWxWindow( this );

		if( layout.fIsInBounds( 2048 ) && layout != mSavedLayout )
		{
			layout.fSave( );
			mSavedLayout = layout;
		}
		else if( layout.mMaximized && !mSavedLayout.mMaximized )
		{
			// when maximized, the result of the layout settings are kind of screwy,
			// so we just set the maximized flag and save whatever the previous settings were
			mSavedLayout.mMaximized = true;
			mSavedLayout.fSave( );
		}
	}
	void tSigAnimDialog::fOnDirty( tEditorActionStack& stack )
	{
		SetTitle( fMakeWindowTitle( ) );
	}
	void tSigAnimDialog::fOnActionUndoOrRedo( tEditorActionStack& stack )
	{
		// TODO: refresh stuff that needs it
	}
	void tSigAnimDialog::fCreateMainMenu( )
	{
		wxMenuBar* mainMenu = new wxMenuBar;

		{
			wxMenu* subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&File" );

			subMenu->Append( cActionNewDoc, "&New\tCtrl+N" );
			subMenu->Append( cActionOpenDoc, "&Open...\tCtrl+O" );
			subMenu->Append( cActionSave, "&Save\tCtrl+S" );
			subMenu->Append( cActionSaveAs, "Save &As..." );

			mRecentFilesMenu = new wxMenu;
			subMenu->AppendSubMenu( mRecentFilesMenu, "Recen&t Files" );
			fUpdateRecentFileMenu( );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionBuild, "&Build\tCtrl+Shift+B" );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionQuit, "E&xit" );
		}
		{
			wxMenu *subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&Edit" );

			subMenu->Append( cActionUndo, "&Undo\tCtrl+Z" );
			subMenu->Append( cActionRedo, "&Redo\tCtrl+Y" );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionCopy, "&Copy\tCtrl+C" );
			subMenu->Append( cActionPaste, "&Paste\tCtrl+V" );

			//subMenu->AppendSeparator( );
			//subMenu->Append( cActionEditContexts, "Edit Contexts" );
		}
		// add "window" menu; might change this later
		{
			wxMenu *subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&Window" );

			subMenu->Append( cActionFrameSelected, "Frame &Selected\t F" );
			subMenu->Append( cActionFrameAll, "Frame &All\t A" );
			subMenu->AppendSeparator();
		}
		{
			wxMenu* subMenu = new wxMenu;
			mainMenu->Append( subMenu, "&Help" );

			subMenu->Append( cActionAbout, "&About..." );
		}

		SetMenuBar( mainMenu );
	}
	void tSigAnimDialog::fAddToolbar( )
	{
		// setup primary/default toolbar
		mToolBar = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHORIZONTAL );
		GetSizer( )->Add( mToolBar, 0, wxEXPAND | wxALL, 0 );
		wxToolBar* mainToolBar = mToolBar;
		mainToolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
		mainToolBar->AddTool( cActionNewDoc, "New Scene", wxBitmap( "newdoc" ), wxNullBitmap, wxITEM_NORMAL, "Create a new, empty scene" );
		mainToolBar->AddTool( cActionOpenDoc, "Open Scene", wxBitmap( "opendoc" ), wxNullBitmap, wxITEM_NORMAL, "Load an existing scene" );
		mainToolBar->AddTool( cActionSave, "Save Scene", wxBitmap( "savedoc" ), wxNullBitmap, wxITEM_NORMAL, "Save current scene" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionUndo, "Undo", wxBitmap( "undo" ), wxNullBitmap, wxITEM_NORMAL, "Undo last action" );
		mainToolBar->AddTool( cActionRedo, "Redo", wxBitmap( "redo" ), wxNullBitmap, wxITEM_NORMAL, "Redo previous action" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionAbout, "About", wxBitmap( "help" ), wxNullBitmap, wxITEM_NORMAL, "Get help" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionFrameSelected, "FrameSel", wxBitmap( "framesel" ), wxNullBitmap, wxITEM_NORMAL, "Frame Selected" );
		mainToolBar->AddTool( cActionFrameAll, "FrameAll", wxBitmap( "frameall" ), wxNullBitmap, wxITEM_NORMAL, "Frame All" );

		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionBuild, "Build Current Momap", wxBitmap( "build" ), wxNullBitmap, wxITEM_NORMAL, "Build the current Momap" );		
		
		mainToolBar->Realize( );
	}
	void tSigAnimDialog::fOnClose( wxCloseEvent& event )
	{
		fSaveLayout( );

		if( fClearScene( ) && mAnimapEditor->fClearScene( ) )
			event.Skip( );
		else
			event.Veto( );
	}
	void tSigAnimDialog::fOnAction( wxCommandEvent& event )
	{
		const int id = event.GetId( );
		if( id >= cActionOpenRecent && id < cActionOpenRecent + cMaxRecentlyOpenedFiles )
		{
			// open recent action
			fOpenRecent( id - cActionOpenRecent );
		}
		else
		{
			switch( id )
			{
				// file menu
			case cActionNewDoc:		fNewDoc( ); break;
			case cActionOpenDoc:	fOpenDoc( ); break;
			case cActionSave:		fSaveDoc( false ); break;
			case cActionSaveAs:		fSaveDoc( true ); break;
			case cActionBuild:		fBuild( ); break;
			case cActionQuit:		Close( false ); break;

				// edit menu
			case cActionUndo:		mCanvas->fUndo( ); break;
			case cActionRedo:		mCanvas->fRedo( ); break;
			case cActionCopy:		mCanvas->fCopy( ); break;
			case cActionPaste:		mCanvas->fPaste( ); break;
			//case cActionEditContexts: 
			//	{ 
			//		Momap::tContextData tempData = mContextData;

			//		tEditMoMapContextsDialog* diag = new tEditMoMapContextsDialog( this, &tempData ); 
			//		if( diag->fShowModal( ) )
			//		{
			//			mContextData = tempData;
			//			mCanvas->fEditorActions( ).fForceSetDirty( true );
			//		}
			//	}
			//	break;

				// window menu
			case cActionFrameSelected:	mCanvas->fFrame( true ); break;
			case cActionFrameAll:		mCanvas->fFrame( false ); break;
			case cActionAbout:
				wxMessageBox( "SigAnim is a visual, node-based anim blend tree tool.", "About SigAnim", wxOK | wxICON_INFORMATION );
				break;

			default: { log_warning( "Unrecognized action!" ); }
				break;
			}
		}
	}
	void tSigAnimDialog::fOnIdle( wxIdleEvent& event )
	{
	}
	void tSigAnimDialog::fOnSelChanged( )
	{
		if( mControlPanel )
			mControlPanel->fOnCanvasSelectionChanged( mCanvas->fSelectedNodes( ), mCanvas->fSelectedConnections( ), mCanvas->fSelectedOutputs( ) );
	}
	std::string tSigAnimDialog::fMakeWindowTitle( ) const
	{
		return "SigAnim V2 ~ " + mDocName + ( mCanvas->fEditorActions( ).fIsDirty( ) ? "*" : "" );
	}
	b32 tSigAnimDialog::fClearScene( )
	{
		if( mCanvas->fEditorActions( ).fIsDirty( ) )
		{
			const int result = wxMessageBox( "You have unsaved Momap changes - would you like to save them before resetting?",
						  "Save Momap Changes?", wxYES | wxNO | wxCANCEL | wxICON_WARNING );

			if(			result == wxYES )			{ if( !fSaveDoc( false ) ) return false; }
			else if(	result == wxNO )			{ }
			else if(	result == wxCANCEL )		{ return false; }
			else									{ log_warning( "Unknown result returned from Message Box" ); }
		}

		mCanvas->fClearCanvas( );
		mDocName = cNewDocTitle;
		//mContextData = Momap::tContextData( );

		return true;
	}
	void tSigAnimDialog::fNewDoc( )
	{
		if( !fClearScene( ) )
			return;
		fSetStatus( "New momap" );
		mCanvas->fAddDefaultNode( );
		mCanvas->fEditorActions( ).fReset( );
		SetTitle( fMakeWindowTitle( ) );
	}
	b32 tSigAnimDialog::fSaveDoc( b32 saveAs )
	{
		if( saveAs || mDocName == cNewDocTitle )
		{
			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				mCanvas, 
				"Save Momap As",
				wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
				wxString( "untitled" + gMomapExt ),
				wxString( "*" + gMomapExt ),
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

			if( openFileDialog->ShowModal( ) != wxID_OK )
				return false; // cancelled

			mDocName = openFileDialog->GetPath( );
			fAddRecentFile( tFilePathPtr( mDocName.c_str( ) ) );
			fUpdateRecentFileMenu( );
		}
		else
		{
			// not doing a save as; if we're not dirty, then skip
			if( !mCanvas->fEditorActions( ).fIsDirty( ) )
				return true;
		}

		if( !fSerialize( tFilePathPtr( mDocName.c_str( ) ) ) )
			return false;

		mCanvas->fEditorActions( ).fClearDirty( );
		SetTitle( fMakeWindowTitle( ) );
		fSetStatus( "Document saved successfully" );
		return true;
	}
	void tSigAnimDialog::fOpenDoc( )
	{
		if( !fClearScene( ) )
			return; // user cancelled, don't try to open new file

		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mCanvas, 
			"Open Momap",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "untitled" + gMomapExt ),
			wxString( "*" + gMomapExt ),
			wxFD_OPEN ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			fOpenDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
		}
	}
	void tSigAnimDialog::fOpenDoc( const tFilePathPtr& file )
	{
		if( !fClearScene( ) )
			return; // user cancelled, don't try to open new file

		if( FileSystem::fFileExists( file ) )
		{
			if( !fDeserialize( file ) )
			{
				wxMessageBox( "The specified .momap file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
				fSetStatus( "Momap open failed" );
				return;
			}

			// set up new scene
			mDocName = file.fCStr( );
			SetTitle( fMakeWindowTitle( ) );
			fSetStatus( "Momap opened successfully" );
			fAddRecentFile( file );
			fUpdateRecentFileMenu( );

			mCanvas->fFrame( );
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
		}
	}
	void tSigAnimDialog::fOpenRecent( u32 ithRecentFile )
	{
		const Win32Util::tRecentlyOpenedFileList& recentFiles = mRecentFiles;
		fOpenDoc( recentFiles[ ithRecentFile ] );
	}
	void tSigAnimDialog::fAddRecentFile( const tFilePathPtr& path )
	{
		mRecentFiles.fAdd( path );
		mRecentFiles.fSave( );
	}
	void tSigAnimDialog::fUpdateRecentFileMenu( )
	{
		if( !mRecentFilesMenu )
			return;

		while( mRecentFilesMenu->GetMenuItems( ).size( ) > 0 )
			mRecentFilesMenu->Delete( mRecentFilesMenu->GetMenuItems( ).front( ) );

		const Win32Util::tRecentlyOpenedFileList& recentFiles = mRecentFiles;
		const u32 min = fMin( cMaxRecentlyOpenedFiles, recentFiles.fCount( ) );
		for( u32 i = 0; i < min; ++i )
			mRecentFilesMenu->Append( cActionOpenRecent + i, recentFiles[ i ].fCStr( ) );
	}

	b32 tSigAnimDialog::fSerialize( const tFilePathPtr& path )
	{
		Momap::tFile file;

		// collect nodes/connections etc
		mCanvas->fToFile( file.mMoState );
		//file.mMoState.mContexts = mContextData;

		// save to xml
		return file.fSaveXml( path, true );
	}
	b32 tSigAnimDialog::fDeserialize( const tFilePathPtr& path )
	{
		Momap::tFile file;
		if( !file.fLoadXml( path ) )
			return false;

		// create nodes/connections etc
		mCanvas->fFromFile( file.mMoState );
		//mContextData = file.mMoState.mContexts;

		return true;
	}
	b32 tSigAnimDialog::fBuild( )
	{
		if( mDocName == cNewDocTitle )
		{
			wxMessageBox( "You must save your file before building it.", "Save First", wxOK | wxICON_WARNING );
			return false;
		}
		else if( mCanvas->fEditorActions( ).fIsDirty( ) )
		{
			fSaveDoc( false );
		}

		tAssetGenScanner::fProcessSingleFile( tFilePathPtr( mDocName ), true );
		return true;
	}


	void tSigAnimDialog::fSetSkeletonEntity( tSkeletableSgFileRefEntity* entity )
	{
		tFilePathPtr skeletonPath;
		if( entity )
			skeletonPath = entity->fSkeletonResourcePath( );

		mAnimapEditor->fSetSkeleton( skeletonPath );
		mCurrentlyEditing.fReset( entity );
	}

	namespace
	{
		struct tSimpleBuffer
		{
			tGrowableArray< byte > mData;

			void operator()( const void* data, u32 numBytes )
			{
				mData.fInsert( mData.fCount( ), (byte*)data, numBytes );
			}
		};

		template< typename t >
		void fFakeSaveLoad( t*& item )
		{
			tSimpleBuffer data;
			tLoadInPlaceSerializer ser;
			ser.fSave<t>( *item, data, cCurrentPlatform );
			delete item;

			tLoadInPlaceDeserializer deser;
			deser.fLoad<t>( data.mData.fBegin( ) );

			// we disown into a sleeve so no one frees this memory
			tArraySleeve<byte> sleeve;
			data.mData.fDisown( sleeve );

			item = (t*)sleeve.fBegin( );
			//hackathon, communicate down the resource depot's owner.
			tResource fakeResource( tResourceId(), NULL );
			fakeResource.fSetOwner( tApplication::fInstance( ).fResourceDepot( ).fGetRawPtr( ) );
			item->fInitializeLoadInPlaceTables( fakeResource );

			//// NOTE: sub resources might need to be loaded here?
			//tResourcePtrList subResources;
			//fakeResource.fGatherSubResources( subResources );
			//for( u32 i = 0; i < subResources.fCount( ); ++i )
			//{
			//	subResources[ i ]->fLoadDefault( &tApplication::fInstance( ) );
			//	subResources[ i ]->fBlockUntilLoaded( );
			//}

			item->fOnFileLoaded( fakeResource );
			fakeResource.fSetOwner( NULL ); // reset owner so it doesnt try to do propper cleanup.
		}
	}

	Anim::tSigAnimMoMap* tSigAnimDialog::fMakeMoMap( )
	{
		if( mCurrentlyEditing )
		{
			Momap::tFile file;
			mCanvas->fToFile( file.mMoState );
			//file.mMoState.mContexts = mContextData;

			tAniMapFile* animap = mAnimapEditor->fFile( ).fMakeAnimapFile( );			
			tMotionMapFile* moMapFile = file.fBuildMoMapFile( false );

			fFakeSaveLoad( animap );
			fFakeSaveLoad( moMapFile );

			Anim::tSigAnimMoMap* moMap = new Anim::tSigAnimMoMap( *moMapFile, *animap, *mCurrentlyEditing->fSkeleton( ) );
			mCurrentlyEditing->fSetMotionMap( moMap );

			// leak these since tSigAnimMomap isn't holding a reference to them, normal behavior in the game unfortunately.
			//delete moMapFile;
			//delete moMapFile;

			return moMap;
		}

		return NULL;
	}

}
