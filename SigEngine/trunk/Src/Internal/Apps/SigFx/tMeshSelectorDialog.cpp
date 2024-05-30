#include "SigFxPch.hpp"
#include "tMeshSelectorDialog.hpp"
#include "tSigFxMainWindow.hpp"
#include "tWxDirectoryBrowser.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"

namespace Sig
{
	enum tAction
	{
		cActionRefreshDirectory = 1,
	};

	class tMeshSystemResourceBrowserTree : public tWxDirectoryBrowser
	{
	private:
		tMeshSelectorDialog*		mMeshSelectorDialog;
		tSigFxMainWindow*			mEditorWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
		DECLARE_EVENT_TABLE( )

	public:
		tMeshSystemResourceBrowserTree( tMeshSelectorDialog* meshSelectorDialog, tSigFxMainWindow* editorWindow, wxWindow* parent, u32 minHeight )
			: tWxDirectoryBrowser( parent, minHeight )
			, mMeshSelectorDialog( meshSelectorDialog )
			, mEditorWindow( editorWindow )
			, mList( 0 )
		{			
			Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( tMeshSystemResourceBrowserTree::fOnItemRightClick ), 0, this );
		}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }

		virtual void fOnSelChanged( wxTreeEvent& event, const tFileEntryData& fileEntryData )
		{
			// this denotes an actual file was selected; load model and display as cursor
			const tFilePathPtr meshFile = fileEntryData.fXmlPath( );
			// pass that path into our systems to set that as our current mesh!

			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fSetMeshResourceFile( meshFile );
				}
			}
			
			mEditorWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
			SetItemTextColour( event.GetItem( ), fileEntryData.fGetTextItemColor( ) );

			mMeshSelectorDialog->fUpdateText( );
		}
		virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData )
		{
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".sigml" ) )
				return wxColour( 0x00, 0x77, 0x00 );
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".mshml" ) )
				return wxColour( 0x00, 0x00, 0x77 );
			return wxColour( 0, 0, 0 );
		}
		virtual b32 fFilterPath( const tFilePathPtr& path )
		{
			return !Sigml::fIsSigmlFile( path );
		}
		virtual std::string fConvertDisplayName( const std::string& simplePath, u32& sortValue )
		{
			std::stringstream displayName;

			if( StringUtil::fStrStrI( simplePath.c_str( ), ".mshml" ) )
			{
				displayName << "[mesh] ";
				sortValue = 1;
			}
			else
				displayName << "[scene] ";

			displayName << StringUtil::fStripExtension( simplePath.c_str( ) );

			return displayName.str( );
		}
		virtual tFilePathPtr fXmlPathToBinaryPath( const tFilePathPtr& xmlPath )
		{
			return Sigml::fSigmlPathToSigb( xmlPath );
		}
		void fOnItemRightClick( wxTreeEvent& event )
		{
			mRightClickItem = event.GetItem( );
			wxMenu menu;
			if( !fIsFileItem( event.GetItem( ) ) )
				menu.Append( cActionRefreshDirectory, _T("&Refresh directory"));
			PopupMenu(&menu, event.GetPoint( ).x, event.GetPoint( ).y);
			event.Skip( );
		}
		void fOnAction( wxCommandEvent& event )
		{
			switch( event.GetId( ) )
			{
			case cActionRefreshDirectory:
				{
					if( !mRightClickItem.IsOk( ) )
						break;
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
					if( !fileEntryData )
					{
						// TODO refresh sub-directory only
						fRefresh( );
					}

					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;

			default: { log_warning( "Unrecognized action!" ); }
				event.Skip( );
				break;
			}
		}
	};

	BEGIN_EVENT_TABLE( tMeshSystemResourceBrowserTree, wxTreeCtrl )
		EVT_MENU( cActionRefreshDirectory,	tMeshSystemResourceBrowserTree::fOnAction )
	END_EVENT_TABLE( )
}

namespace Sig
{
	tMeshSelectorDialog::tMeshSelectorDialog( tSigFxMainWindow* parent, tEditorActionStack& actionStack, const std::string& regKeyName )
		: tWxSlapOnDialog( "MeshSelector", parent, regKeyName )
		, mSigFx( parent )
		, mRefreshed( false )
	{
		SetSize( 380, 520 );
		SetMinSize( wxSize( GetSize( ).x, GetSize( ).y ) );
		SetMaxSize( wxSize( GetSize( ).x, GetSize( ).y ) );
		SetBackgroundColour( wxColour( 0x33, 0x33, 0x33 ) );
		SetForegroundColour( wxColour( 0x11, 0xff, 0x11 ) );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mSelectedMeshText = new wxStaticText( this, wxID_ANY, wxString( "Particle Mesh: [no selection]" ) );

		mQuadStyleButton = new wxButton( this, wxID_ANY, "Clear Particle Mesh (Quad-Style Particles)" );
		mQuadStyleButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMeshSelectorDialog::fOnClearMesh ), NULL, this );

		wxPanel* browserPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
		mMeshSystemResourceBrowser = new tMeshSystemResourceBrowserTree( this, mSigFx, browserPanel, 256 );
		mMeshSystemResourceBrowser->SetSize( 354, 400 );

		GetSizer( )->Add( mSelectedMeshText, 0, wxALL | wxEXPAND, 8 );
		GetSizer( )->Add( mQuadStyleButton, 0, wxALL | wxEXPAND, 8 );
		GetSizer( )->Add( browserPanel, 0, wxALL | wxEXPAND, 5 );

		SetIcon( wxIcon( "appicon" ) );
		fSetTopMost( true );
		fLoad( );

		mMeshSystemResourceBrowser->Enable( false );
		mQuadStyleButton->Enable( false );
	}
	void tMeshSelectorDialog::fOnTick( )
	{
		if( !mRefreshed )
		{
			mRefreshed = true;
			mMeshSystemResourceBrowser->fRefresh( );
		}
		fAutoHandleTopMost( ( HWND )mSigFx->GetHWND( ) );
		if( fIsActive( ) )
			mSigFx->fSetDialogInputActive( );
	}
	void tMeshSelectorDialog::fUpdateSelectedList( tEditorSelectionList& list )
	{
		mParticleSystemsList.fSetCount( 0 );
		list.fCullByType< tSigFxParticleSystem >( mParticleSystemsList );
		mMeshSystemResourceBrowser->fSetList( &mParticleSystemsList );

		const bool enabled = mParticleSystemsList.fCount( ) > 0;
		fUpdateText( );
		mMeshSystemResourceBrowser->Enable( enabled );
		mQuadStyleButton->Enable( enabled );
	}
	void tMeshSelectorDialog::fUpdateText( )
	{
		const bool enabled = mParticleSystemsList.fCount( ) > 0;
		std::stringstream ss; ss << "Particle Mesh: ";
		if( enabled )
		{
			tFilePathPtr meshPath = mParticleSystemsList.fFront( )->fMeshResourcePath( );
			b32 conflict = false;
			for( u32 i = 1; !conflict && i < mParticleSystemsList.fCount( ); ++i )
				conflict = ( meshPath != mParticleSystemsList[ i ]->fMeshResourcePath( ) );
			const std::string meshString = meshPath.fLength( ) > 0 ? meshPath.fCStr( ) : "Quad-Style";
			ss << "[" << ( conflict ? "multiple meshes" : meshString.c_str( ) ) << "]";
		}
		else
			ss << "[no selection]";

		const std::string str = ss.str( );
		mSelectedMeshText->SetLabel( str );
	}
	void tMeshSelectorDialog::fOnClearMesh( wxCommandEvent& event )
	{
		for( u32 i = 0; i < mParticleSystemsList.fCount( ); ++i )
			mParticleSystemsList[ i ]->fSetMeshResourceFile( tFilePathPtr( ) );
		fUpdateText( );
	}
}
