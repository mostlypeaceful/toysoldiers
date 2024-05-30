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
	tMatEdMainWindow::tMatEdMainWindow( wxWindow* parent, tEditorActionStack& actionStack, const std::string& regKeyName, b32 fullEditMode )
		: tWxSlapOnDialog( "MatEd", parent, regKeyName )
		, mActionStack( actionStack )
		, mFullEditMode( fullEditMode )
		, mHeaderText( 0 )
		, mPreviewPanel( 0 )
		, mDermlBrowserPanel( 0 )
		, mDermlBrowser( 0 )
		, mPropertyPanel( 0 )
	{
		SetSize( 380, wxDefaultSize.y );
		SetMinSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetMaxSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetBackgroundColour( wxColour( 0x33, 0x33, 0x33 ) );
		SetForegroundColour( wxColour( 0x11, 0xff, 0x11 ) );

		mOnPropertyChanged.fFromMethod< tMatEdMainWindow, &tMatEdMainWindow::fOnPropertyChanged >( this );

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
		wxButton* ambButton = new wxButton( this, wxID_ANY, "Am", wxDefaultPosition, buttonSize );
		wxButton* frontButton = new wxButton( this, wxID_ANY, "Fr", wxDefaultPosition, buttonSize );
		wxButton* rimButton = new wxButton( this, wxID_ANY, "Ri", wxDefaultPosition, buttonSize );
		wxButton* backButton = new wxButton( this, wxID_ANY, "Ba", wxDefaultPosition, buttonSize );

		lSizer->Add( playButton, 0, wxALIGN_RIGHT | wxTOP, 10 );
		lSizer->Add( modeButton, 0, wxALIGN_RIGHT | wxTOP, 6 );
		lSizer->Add( resetButton, 0, wxALIGN_RIGHT | wxTOP, 6 );

		mPreviewPanel = new tMaterialPreviewPanel( this, 280, 180,
			playButton,
			modeButton,
			resetButton,
			ambButton,
			frontButton,
			rimButton,
			backButton );
		mSizer->Add( mPreviewPanel, 0, wxCENTER | wxALL, 10 );

		rSizer->Add( ambButton, 0, wxALIGN_LEFT | wxTOP, 10 );
		rSizer->Add( frontButton, 0, wxALIGN_LEFT | wxTOP, 6 );
		rSizer->Add( rimButton, 0, wxALIGN_LEFT | wxTOP, 6 );
		rSizer->Add( backButton, 0, wxALIGN_LEFT | wxTOP, 6 );

		lSizer->SetMinSize( wxSize( 32, wxDefaultSize.y ) );
		rSizer->SetMinSize( wxSize( 32, wxDefaultSize.y ) );

		hSizer->Add( lSizer, 0, wxEXPAND | wxALL, 0 );
		hSizer->Add( mSizer, 0, wxEXPAND | wxALL, 0 );
		hSizer->Add( rSizer, 0, wxEXPAND | wxALL, 0 );

		mHeaderText = new wxStaticText( this, wxID_ANY, "Shader: None" );
		GetSizer( )->Add( mHeaderText, 0, wxLEFT, 8 );
		GetSizer( )->AddSpacer( 8 );

		if( mFullEditMode )
		{
			mDermlBrowserPanel = new tWxSlapOnGroup( this, "Shader Browser", true );
			mDermlBrowser = new tDermlBrowser( this, mDermlBrowserPanel->fGetMainPanel( ), 160 );
			mDermlBrowserPanel->fGetMainPanel( )->GetSizer( )->Add( mDermlBrowser, 0, wxEXPAND | wxALL, 4 );
			mDermlBrowser->fRefresh( );
		}

		mPropertyPanel = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE | wxVSCROLL );
		mPropertyPanel->SetBackgroundColour( wxColour( 0x55, 0x55, 0x55 ) );
		mPropertyPanel->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
		GetSizer( )->Add( mPropertyPanel, 1, wxEXPAND | wxALL, 0 );

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

		// generate gui from node list
		tPropertyChangeContext context = fBeginPropertyChange( nodes.fCount( ) );

		if( nodes.fCount( ) == 0 )
			mHeaderText->SetLabel( "Shader: None" );
		else
		{
			wxString shaderName;
			if( shaderPath.fLength( ) == 0 )	shaderName = nodes.fFront( )->fMatEdName( );
			else								shaderName = shaderPath.fCStr( );
			mHeaderText->SetLabel( "Shader: " + shaderName );

			for( u32 i = 0; i < nodes.fCount( ); ++i )
			{
				mCommonProps[ i ].mOnPropertyChanged.fAddObserver( &mOnPropertyChanged );
				mCommonProps[ i ].fClearGui( );

				tShadeNodePtr shadeNode = nodes[ i ];
				if( shadeNode->fMatEdAllowEdit( ) && shadeNode->fRefreshMaterialProperties( *shadeNode, context.mNumPropsAdded ) )
				{
					++context.mNumPropsAdded;
					mCommonProps[ i ] = shadeNode->fMatProps( );
					mCommonProps[ i ].fCollectCommonPropertiesForGui( shadeNode->fMatProps( ) );
					context.mTotalHeight += tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps[ i ], false );
				}
			}
		}

		fEndPropertyChange( context );

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
		mPropertyPanel->DestroyChildren( );
		mCommonProps.fSetCount( numPotentialProps );
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
		if( context.mNumPropsAdded > 0 )
			mPropertyPanel->SetScrollbars( 0, 20, 0, context.mTotalHeight / 5 + 4 );
		else
			mPropertyPanel->SetScrollbars( 0, 0, 0, 0 );
		mPropertyPanel->Layout( );
		Thaw( );
	}

	void tMatEdMainWindow::fOnPropertyChanged( tEditableProperty& property )
	{
		mActionStack.fForceSetDirty( true );
	}

}

