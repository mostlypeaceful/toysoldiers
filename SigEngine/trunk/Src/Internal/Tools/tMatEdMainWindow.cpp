#include "ToolsPch.hpp"
#include "tMatEdMainWindow.hpp"
#include "tWxSlapOnControl.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tWxDirectoryBrowser.hpp"
#include "Derml.hpp"
#include "tMaterialPreviewPanel.hpp"
#include "Editor/tEditorAction.hpp"
#include "Editor/tEditablePropertyTypes.hpp"

namespace Sig
{
	namespace
	{
		enum tAction
		{
			cActionRefreshDirectory = 1,
			cActionSelectShader,
		};
	}

	class tDermlBrowser : public tWxDirectoryBrowser
	{
		tMatEdMainWindow* mMatEd;
	public:
		tDermlBrowser( tMatEdMainWindow* editorWindow, wxWindow* parent, u32 minHeight )
			: tWxDirectoryBrowser( parent, minHeight )
			, mMatEd( editorWindow )
		{			
			Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( tDermlBrowser::fOnItemRightClick ) );
			Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tDermlBrowser::fOnAction ) );
		}
		virtual void fOnSelChanged( wxTreeEvent& event, const tFileEntryData& fileEntryData )
		{
		}
		virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData )
		{
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".derml" ) )
				return wxColour( 0x00, 0x00, 0x77 );
			return wxColour( 0, 0, 0 );
		}
		virtual b32 fFilterPath( const tFilePathPtr& path )
		{
			return !StringUtil::fCheckExtension( path.fCStr( ), ".derml" );
		}
		virtual tFilePathPtr fXmlPathToBinaryPath( const tFilePathPtr& xmlPath )
		{
			return tFilePathPtr( );//tFilePathPtr::fSwapExtension( xmlPath, ".mtlb" );
		}
		void fOnItemRightClick( wxTreeEvent& event )
		{
			mRightClickItem = event.GetItem( );
			wxMenu menu;
			if( !fIsFileItem( event.GetItem( ) ) )
				menu.Append( cActionRefreshDirectory, _T("&Refresh"));
			else
				menu.Append( cActionSelectShader, _T("&Select Shader"));
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
			case cActionSelectShader:
				{
					if( !mRightClickItem.IsOk( ) )
						break;
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
					if( fileEntryData )
					{
						mMatEd->fOnShaderSelected( fileEntryData->fXmlPath( ) );
					}
					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;
			default:
				event.Skip( );
				break;
			}
		}
	};
}

namespace Sig
{
	tMatEdMainWindow::tMatEdMainWindow( wxWindow* parent, tEditorActionStack& actionStack, const std::string& regKeyName, b32 enableShaderBrowsing, b32 enableLightProbeEditing, b32 enableMaterialEditing, u32 renderWidth, u32 renderHeight )
		: tWxSlapOnDialog( "MatEd", parent, regKeyName )
		, mActionStack( actionStack )
		, mEnableShaderBrowsing( enableShaderBrowsing )
		, mEnableUserPropertyEditing( enableLightProbeEditing )
		, mHeaderText( 0 )
		, mPreviewPanel( 0 )
		, mDermlBrowserPanel( 0 )
		, mDermlBrowser( 0 )
		, mMaterialPropertyGroup( 0 )
		, mUserPropertyGroup( NULL )
	{
		SetSize( renderWidth + 100, renderHeight + 400 );
		SetMinSize( wxSize( GetSize( ).x, renderHeight + 400 ) );
		SetMaxSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetBackgroundColour( wxColour( 0x33, 0x33, 0x33 ) );
		SetForegroundColour( wxColour( 0x11, 0xff, 0x11 ) );

		mOnMaterialPropertyChanged.fFromMethod< tMatEdMainWindow, &tMatEdMainWindow::fOnMaterialPropertyChanged >( this );
		mOnUserPropertyChanged.fFromMethod< tMatEdMainWindow, &tMatEdMainWindow::fOnUserPropertyChanged >( this );		

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( hSizer, 0, wxEXPAND | wxALL, 0 );

		wxSizer* lSizer = new wxBoxSizer( wxVERTICAL );
		wxSizer* mSizer = new wxBoxSizer( wxVERTICAL );
		wxSizer* rSizer = new wxBoxSizer( wxVERTICAL );

		const int buttonW = 24;
		const int buttonH = 24;
		const wxSize buttonSize = wxSize( buttonW, buttonH );

		wxButton* playButton = new wxButton( this, wxID_ANY, "||", wxDefaultPosition, buttonSize );
		wxButton* modeButton = new wxButton( this, wxID_ANY, "Cm", wxDefaultPosition, buttonSize );
		wxButton* resetButton = new wxButton( this, wxID_ANY, "Re", wxDefaultPosition, buttonSize );
		//these don't do anything... no point in having buttons that do nothing...
		wxButton* ambButton = 0;//new wxButton( this, wxID_ANY, "Am", wxDefaultPosition, buttonSize );
		wxButton* frontButton = 0;//new wxButton( this, wxID_ANY, "Fr", wxDefaultPosition, buttonSize );
		wxButton* rimButton = 0;//new wxButton( this, wxID_ANY, "Ri", wxDefaultPosition, buttonSize );
		wxButton* backButton = 0;//new wxButton( this, wxID_ANY, "Ba", wxDefaultPosition, buttonSize );

		lSizer->Add( playButton, 0, wxALIGN_RIGHT | wxTOP, 10 );
		lSizer->Add( modeButton, 0, wxALIGN_RIGHT | wxTOP, 6 );
		lSizer->Add( resetButton, 0, wxALIGN_RIGHT | wxTOP, 6 );

		mPreviewPanel = new tMaterialPreviewPanel( this, renderWidth, renderHeight,
			playButton,
			modeButton,
			resetButton,
			ambButton,
			frontButton,
			rimButton,
			backButton );
		mSizer->Add( mPreviewPanel, 0, wxCENTER | wxALL, 10 );

		if( ambButton )
			rSizer->Add( ambButton, 0, wxALIGN_LEFT | wxTOP, 10 );
		if( frontButton )
			rSizer->Add( frontButton, 0, wxALIGN_LEFT | wxTOP, 6 );
		if( rimButton )
			rSizer->Add( rimButton, 0, wxALIGN_LEFT | wxTOP, 6 );
		if( backButton )
			rSizer->Add( backButton, 0, wxALIGN_LEFT | wxTOP, 6 );

		lSizer->SetMinSize( wxSize( 32, wxDefaultSize.y ) );
		rSizer->SetMinSize( wxSize( 32, wxDefaultSize.y ) );

		hSizer->Add( lSizer, 0, wxEXPAND | wxALL, 0 );
		hSizer->Add( mSizer, 0, wxEXPAND | wxALL, 0 );
		hSizer->Add( rSizer, 0, wxEXPAND | wxALL, 0 );

		if( mEnableShaderBrowsing || enableMaterialEditing )
		{
			mHeaderText = new wxStaticText( this, wxID_ANY, "Shader: None" );
			GetSizer( )->Add( mHeaderText, 0, wxLEFT, 8 );
			GetSizer( )->AddSpacer( 8 );
		}

		if( mEnableShaderBrowsing )
		{
			mDermlBrowserPanel = new tWxSlapOnGroup( this, "Shader Browser", true );
			mDermlBrowser = new tDermlBrowser( this, mDermlBrowserPanel->fGetMainPanel( ), 160 );
			mDermlBrowserPanel->fGetMainPanel( )->GetSizer( )->Add( mDermlBrowser, 0, wxEXPAND | wxALL, 4 );
			mDermlBrowser->fRefresh( );
		}

		if( enableMaterialEditing )
		{
			mMaterialPropertyGroup = new tWxSlapOnGroup( this, "Material Properties", true, true, true );
			mMaterialPropertyGroup->fGetMainPanel()->SetBackgroundColour( wxColour( 0x55, 0x55, 0x55 ) );
			mMaterialPropertyGroup->fGetMainPanel()->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
			
		}

		if( mEnableUserPropertyEditing )
		{
			mUserPropertyGroup		= new tWxSlapOnGroup( this, "Properties", true, true, true );
			mUserPropertyGroup->fGetMainPanel()->SetBackgroundColour( wxColour( 0x55, 0x55, 0x55 ) );
			mUserPropertyGroup->fGetMainPanel()->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );

			mUserProps.mOnPropertyChanged.fAddObserver( &mOnUserPropertyChanged );
		}


		fSetTopMost( true );
		fLoad( );
	}

	tMatEdMainWindow::~tMatEdMainWindow( )
	{
	}

	void tMatEdMainWindow::fSetupPreviewWindow( const Gfx::tDevicePtr& device )
	{
		if( !mPreviewPanel )
			return; // no preview panel

		if( device )
			mDevice = device;
		else
			mDevice.fReset( new Gfx::tDevice( ( u64 )mPreviewPanel->GetHWND( ) ) );
		sigassert( mDevice );

		mPreviewPanel->fSetup( mDevice );
	}

	b32 tMatEdMainWindow::fHasSameDevice( const Gfx::tDevicePtr& device ) const
	{
		if( mDevice == device )
			return true;
		if( mDevice && device && mDevice->fGetDevice( ) == device->fGetDevice( ) )
			return true;
		return false;
	}

	void tMatEdMainWindow::fClear( )
	{
		if( mPreviewPanel )
			mPreviewPanel->fClear( );
		tPropertyChangeContext context = fBeginPropertyChange( 0 );
		if( mHeaderText )
			mHeaderText->SetLabel( "Shader: None" );
		fEndPropertyChange( context );
	}

	Dx9Util::tTextureCachePtr tMatEdMainWindow::fTextureCache( ) const
	{
		if( mPreviewPanel )
			return mPreviewPanel->fTextureCache( );
		return Dx9Util::tTextureCachePtr( );
	}

	Gfx::tLightEntityPtr tMatEdMainWindow::fDefaultLight( ) const
	{
		if( mPreviewPanel )
			return mPreviewPanel->fDefaultLight( );
		return Gfx::tLightEntityPtr( );
	}

	tMaterialPreviewBundlePtr tMatEdMainWindow::fPreviewBundle( ) const
	{
		if( mPreviewPanel )
			return mPreviewPanel->fPreviewBundle( );
		return tMaterialPreviewBundlePtr( );
	}

	void tMatEdMainWindow::fSetPreviewBundle( const tMaterialPreviewBundlePtr& bundle )
	{
		if( mPreviewPanel )
			mPreviewPanel->fSetPreviewBundle( bundle );
	}

	void tMatEdMainWindow::fEnableShaderBrowser( b32 enable )
	{
		if( mDermlBrowserPanel )
		{
			mDermlBrowserPanel->fGetMainPanel( )->Enable( enable!=0 );
			//mDermlBrowser->fRefresh( );
		}
	}

	void tMatEdMainWindow::fEnableMaterialEdit( b32 enable )
	{
		if( mMaterialPropertyGroup )
		{
			mMaterialPropertyGroup->fGetMainPanel()->Enable( enable!=0 );
		}
	}

	void tMatEdMainWindow::fSetUserProperties( tEditablePropertyTable& table )
	{
		u32 currentScroll = mUserPropertyGroup->fGetMainPanel()->GetScrollPos( wxVSCROLL );
		mUserProps.fClearGui( );
		mUserPropertyGroup->fGetMainPanel()->DestroyChildren( );
		mUserPropertyGroup->fRebuildGui(); // Rebuilds the collapse button/static text.

		mUserProps = table;
		mUserProps.fCollectCommonPropertiesForGui( table );
		tEditablePropertyTable::fAddPropsToWindow( mUserPropertyGroup->fGetMainPanel(), mUserProps, false );

		mUserPropertyGroup->fGetMainPanel()->SetScrollbars( 0, 20, 0, 100 + 4, 0, currentScroll );

		mUserPropertyGroup->fGetMainPanel()->Layout( );
		Layout();
	}

	void tMatEdMainWindow::fFromDermlFile( const Derml::tFile& derml, const tFilePathPtr& shaderPath )
	{
		Derml::tMtlFile mtlFile;
		mtlFile.fFromShaderFile( derml, shaderPath );
		fFromDermlMtlFile( mtlFile );
	}

	void tMatEdMainWindow::fFromDermlMtlFile( const Derml::tMtlFile& mtlFile )
	{
		const tFilePathPtr& shaderPath = mtlFile.mShaderPath;
		const Derml::tNodeList& nodes = mtlFile.mNodes;

		if( mMaterialPropertyGroup )
		{
			// generate gui from node list
			tPropertyChangeContext context = fBeginPropertyChange( nodes.fCount( ) );

			if( nodes.fCount( ) == 0 )
			{
				if( mHeaderText )
					mHeaderText->SetLabel( "Shader: None" );
			}
			else
			{
				if( mHeaderText )
				{
					wxString shaderName;
					if( shaderPath.fLength( ) == 0 )	
						shaderName = nodes.fFront( )->fMatEdName( );
					else								
						shaderName = shaderPath.fCStr( );

					mHeaderText->SetLabel( "Shader: " + shaderName );
				}

				for( u32 i = 0; i < nodes.fCount( ); ++i )
				{
					mMaterialCommonProps[ i ].mOnPropertyChanged.fAddObserver( &mOnMaterialPropertyChanged );
					mMaterialCommonProps[ i ].fClearGui( );

					tShadeNodePtr shadeNode = nodes[ i ];
					if( shadeNode->fMatEdAllowEdit( ) && shadeNode->fRefreshMaterialProperties( *shadeNode, context.mNumPropsAdded ) )
					{
						++context.mNumPropsAdded;
						mMaterialCommonProps[ i ] = shadeNode->fMatProps( );
						mMaterialCommonProps[ i ].fCollectCommonPropertiesForGui( shadeNode->fMatProps( ) );
						context.mTotalHeight += tEditablePropertyTable::fAddPropsToWindow( mMaterialPropertyGroup->fGetMainPanel(), mMaterialCommonProps[ i ], false );
					}
				}
			}

			fEndPropertyChange( context );
		}

		if( mPreviewPanel )
			mPreviewPanel->fFromDermlMtlFile( mtlFile );
	}

	b32 tMatEdMainWindow::fSetDefaultBrowseDirectory( const tFilePathPtr& defaultBrowseDirectory )
	{
		if( !mPreviewPanel )
			return false;

		const Derml::tNodeList& nodes = mPreviewPanel->fMtlFile( ).mNodes;
		for( u32 i = 0; i < nodes.fCount( ); ++i )
		{
			tShadeNodePtr shadeNode = nodes[ i ];
			for( tEditablePropertyTable::tIterator i = shadeNode->fMatProps( ).fBegin( ); i != shadeNode->fMatProps( ).fEnd( ); ++i )
			{
				if( i->mKey.length( ) )
					tEditablePropertyFileNameString::fSetDefaultBrowseDirectory( i->mKey, defaultBrowseDirectory.fCStr( ) );
			}
		}
		return true;
	}

	b32 tMatEdMainWindow::fOnTick( )
	{
		if( !IsVisible( ) || fIsMinimized( ) )
		{
			if( mPreviewPanel )
				mPreviewPanel->fSetTimeRunning( false );
			return false;
		}

		if( mPreviewPanel )
			mPreviewPanel->fSetTimeRunning( true );

		// update render preview
		if( mPreviewPanel )
			mPreviewPanel->fRender( );

		return true;
	}

	tMatEdMainWindow::tPropertyChangeContext tMatEdMainWindow::fBeginPropertyChange( u32 numPotentialProps )
	{
		tPropertyChangeContext o;

		Freeze( );

		if( mMaterialPropertyGroup)
		{
			mMaterialPropertyGroup->fGetMainPanel( )->DestroyChildren( );
			mMaterialPropertyGroup->fRebuildGui(); // Rebuilds the collapse button.
		}

		mMaterialCommonProps.fSetCount( numPotentialProps );
		o.mLabelWidth = tWxSlapOnControl::fLabelWidth( );
		o.mControlWidth = tWxSlapOnControl::fControlWidth( );
		tWxSlapOnControl::fSetLabelWidth( 100 );
		tWxSlapOnControl::fSetControlWidth( 200 );

		return o;
	}

	void tMatEdMainWindow::fEndPropertyChange( const tPropertyChangeContext& context )
	{
		tWxSlapOnControl::fSetLabelWidth( context.mLabelWidth );
		tWxSlapOnControl::fSetControlWidth( context.mControlWidth );

		if( mMaterialPropertyGroup )
		{
			if( context.mNumPropsAdded > 0 )
				mMaterialPropertyGroup->fGetMainPanel( )->SetScrollbars( 0, 20, 0, context.mTotalHeight / 5 + 4 );
			else
				mMaterialPropertyGroup->fGetMainPanel( )->SetScrollbars( 0, 0, 0, 0 );
			mMaterialPropertyGroup->fGetMainPanel( )->Layout( );
		}

		Layout( );
		Thaw( );
	}

	void tMatEdMainWindow::fOnMaterialPropertyChanged( tEditableProperty& property )
	{
		mActionStack.fForceSetDirty( true );
	}

	void tMatEdMainWindow::fOnUserPropertyChanged( tEditableProperty& property )
	{
		mActionStack.fForceSetDirty( true );
	}

}

