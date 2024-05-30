#include "SigEdPch.hpp"
#include "tEditorAppWindow.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "FileSystem.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxToolsPanelContainer.hpp"
#include "tEditableObjectProperties.hpp"
#include "tRemapReferencesDialog.hpp"
#include "tSigEdExplicitDependenciesDialog.hpp"
#include "tObjectBrowserDialog.hpp"
#include "tManipulationGizmo.hpp"
#include "tManipulateObjectPanel.hpp"
#include "tLayerPanel.hpp"
#include "tHeightFieldMaterialPaintPanel.hpp"
#include "tWxProgressDialog.hpp"
#include "tAssetGenScanner.hpp"
#include "tSelectObjectsCursor.hpp"
#include "tToolsMouseAndKbCamera.hpp"
#include "tEditorAppWindowContextActions.hpp"
#include "tEditorAppWindowHotKeys.hpp"
#include "tManipulationGizmoAction.hpp"
#include "tSearchableOpenFilesDialog.hpp"
#include "tMemoryViewerDialog.hpp"
#include "tHeightFieldPaintCursor.hpp"
#include "tGroundCoverPanel.hpp"
#include "Gfx/tGeometryFile.hpp" // tGeometryStreamMgr
#include "Gfx/tCamera.hpp"
#include "WxUtil.hpp"
#include "tAssetPluginDll.hpp"
#include "tWxNotificationMessage.hpp"

namespace Sig
{
	class tSaveLoadProgressDialog : public tWxProgressDialog
	{
	public:
		tSaveLoadProgressDialog( wxWindow* parent, const char* title )
			: tWxProgressDialog( parent, title ) { SetSize( 300, GetSize( ).y ); }
		virtual void fUpdate( u32 ithObject, u32 totalObjects ) = 0;
	};

	class tSaveProgressDialog : public tSaveLoadProgressDialog
	{
	public:
		tSaveProgressDialog( wxWindow* parent, const char* title )
			: tSaveLoadProgressDialog( parent, title ) { }
		virtual void fUpdate( u32 ithObject, u32 totalObjects )
		{
			const f32 progress = ithObject / ( f32 )totalObjects;
			tWxProgressDialog::fUpdate( progress, "Saving objects..." );
		}
	};

	class tLoadProgressDialog : public tSaveLoadProgressDialog
	{
		tResourceDepotPtr mResourceDepot;
		u32 mMaxResources;
	public:
		tLoadProgressDialog( wxWindow* parent, const char* title, const tResourceDepotPtr& resourceDepot )
			: tSaveLoadProgressDialog( parent, title ), mResourceDepot( resourceDepot ), mMaxResources( 10 )
		{
			tWxProgressDialog::fUpdate( 0.01f, "Reading file from disk (this may stall for a sec)..." );
		}

		virtual void fUpdate( u32 ithObject, u32 totalObjects )
		{
			const f32 progressFromObjects = ithObject / ( f32 )totalObjects;
			tWxProgressDialog::fUpdate( 0.5f * progressFromObjects, "Loading objects..." );

			if( ithObject >= totalObjects - 1 )
			{
				for(	u32 resourcesLeft = mResourceDepot->fLoadingResourceCount( ); 
						resourcesLeft > 0;
						resourcesLeft = mResourceDepot->fLoadingResourceCount( ) )
				{
					mResourceDepot->fUpdateLoadingResources( );
					mMaxResources = fMax( resourcesLeft, mMaxResources );
					const f32 progressFromResources = 1.f - resourcesLeft / ( f32 )( mMaxResources + 1.f );
					tWxProgressDialog::fUpdate( 0.5f + 0.5f * progressFromResources, "Loading resources..." );
				}
			}
		}
	};

}

namespace Sig
{

	tEditorAppWindow::tEditorAppWindow( tToolsGuiApp& guiApp )
		: tToolsGuiMainWindow( guiApp )
		, mReadyToOpenDoc( false )
		, mPreviewPlatform( cPlatformPcDx9 )
		, mRefreshObjectProperties( false )
		, mMainSizer( 0 )
		, mManipTools( 0 )
		, mRefObjPanel( 0 )
		, mLayerPanel( 0 )
		, mVisibilityPanel( 0 )
		, mHeightFieldMaterials( 0 )
		, mGCPanel( 0 )
		, mToolsPanelContainer( 0 )
		, mObjectProperties( 0 )
		, mGlobalProperties( 0 )
		, mRemapReferencesDialog( 0 )
		, mExplicitDependenciesDialog( 0 )
		, mObjectBrowserDialog( 0 )
		, mOpenDialog( 0 )
		, mTilePaintPanel( 0 )
	{
		// create primary window containers

		//wxPanel* leftPanel = new wxPanel( this, 0, 0, 360, -1 );
		//leftPanel->SetMinSize( wxSize( 360, -1 ) );
		//leftPanel->SetMaxSize( wxSize( 360, -1 ) );

		mRenderPanelContainer = new tWxRenderPanelContainer( this, guiApp.fRegKeyName( ) + "\\RenderPanels", true );
		mToolsPanelContainer = new tWxToolsPanelContainer( *this );
		mObjectProperties = new tEditableObjectProperties( this, false );
		mObjectProperties->fSetOnFocusCallback( &tEditorAppWindow::fOnObjectPropertiesFocus );
		mGlobalProperties = new tEditableObjectProperties( this, true );
		mGlobalProperties->fSetOnFocusCallback( &tEditorAppWindow::fOnGlobalPropertiesFocus );
		mRemapReferencesDialog = new tRemapReferencesDialog( this );
		mExplicitDependenciesDialog = new tSigEdExplicitDependenciesDialog( this );
		mObjectBrowserDialog = new tObjectBrowserDialog( this );
		mMemoryViewerDialog = new tMemoryViewerDialog( this );
		//mScriptEd = new tScriptEditorDialog( this );
		mEditorDialogs.fPushBack( mObjectProperties );
		mEditorDialogs.fPushBack( mGlobalProperties );
		mEditorDialogs.fPushBack( mRemapReferencesDialog );
		mEditorDialogs.fPushBack( mExplicitDependenciesDialog );
		mEditorDialogs.fPushBack( mObjectBrowserDialog );
		mEditorDialogs.fPushBack( mMemoryViewerDialog );
		//mEditorDialogs.fPushBack( mScriptEd );

		// create main sizer
		mMainSizer = new wxBoxSizer( wxHORIZONTAL );
		SetSizer( mMainSizer );
		//mMainSizer->Add( leftPanel, 0, wxEXPAND | wxALL, 0 );
		mMainSizer->Add( mRenderPanelContainer->fGetContainerPanel( ), 1, wxEXPAND | wxALL, 0 );
		mMainSizer->Add( mToolsPanelContainer->fGetContainerPanel( ), 0, wxEXPAND | wxALL, 0 );

		// create hot keys
		tEditorHotKeyTable& hotKeyTable = guiApp.fHotKeys( );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorNewHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorSaveHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorOpenHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorBrowseHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorBuildHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorPreviewHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorPreviewPlaytestHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorUndoHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorRedoHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorFrameSelectedHotKey( hotKeyTable, this ) ) );
		//mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorFrameAllHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorToggleObjectPropsHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorToggleGlobalPropsHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorToggleFindHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorHideSelectedHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorHideUnselectedHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorUnhideAllHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorToggleViewMode( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorCutHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorCopyHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorPasteHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorDuplicateHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorGroupHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorBreakGroupHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorSelectAllHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorShiftLeftHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorShiftRightHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorShiftUpHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorShiftDownHotKey( hotKeyTable, this ) ) );

		// set delegates
		mOnDirty.fFromMethod<tEditorAppWindow, &tEditorAppWindow::fOnDirty>( this );
		mOnAddAction.fFromMethod<tEditorAppWindow, &tEditorAppWindow::fOnActionUndoOrRedo>( this );
		fGuiApp( ).fActionStack( ).mOnDirty.fAddObserver( &mOnDirty );
		fGuiApp( ).fActionStack( ).mOnAddAction.fAddObserver( &mOnAddAction );
		fGuiApp( ).fActionStack( ).mOnUndo.fAddObserver( &mOnAddAction );
		fGuiApp( ).fActionStack( ).mOnRedo.fAddObserver( &mOnAddAction );
		mOnSelChanged.fFromMethod<tEditorAppWindow, &tEditorAppWindow::fOnSelChanged>( this );
		fGuiApp( ).fSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelChanged );

		// add context actions
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tRotationGizmoSettingsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tHideObjectsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tFreezeObjectsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSnapObjectsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tChangeShadeModeContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tToggleFadeAlphaContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSaveHeightMapBitmapsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tOpenSigmlContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tDisplayObjectPropertiesContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertToAttachmentPointsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertToShapesContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertToCamerasContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertToNavGraphContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSelectEntireNavGraphContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertAttachmentPointsToWayPointsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSelectEntirePathContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSelectPathDecalContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tFindInRefBrowserContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tViewCameraContextAction( this ) ) );

		mObjectProperties->fSetFocusHooks( );
		mGlobalProperties->fSetFocusHooks( );
	}

	tEditorAppWindow::~tEditorAppWindow( )
	{
		mHotKeyStorage.fDeleteArray( );
	}

	void tEditorAppWindow::fSetupRendering( )
	{
		mRenderPanelContainer->fSetupRendering( fGuiApp( ) );

		fGuiApp( ).fEditableObjects( ).fReset(
			fGuiApp( ).fGfxDevice( ),
			mRenderPanelContainer->fGetSolidColorMaterial( ), 
			mRenderPanelContainer->fGetSolidColorGeometryAllocator( ),
			mRenderPanelContainer->fGetSolidColorIndexAllocator( ) );

		mHeightFieldMaterials->fClearAtlases( );

		mReadyToOpenDoc = true;
	}

	namespace
	{
		struct tPluginLoader
		{
			tEditorAppWindow* mOwner;
			wxMenu *mMenu;

			tPluginLoader( tEditorAppWindow* owner, wxMenu *menu )
				: mOwner( owner ), mMenu( menu )
			{ }

			b32 operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const
			{
				iSigEdPlugin* plugin = assetPlugin.fGetSigEdPluginInterface( );
				if( plugin )
				{
					plugin->fConstruct( mOwner );

					u32 index = mOwner->fPlugins( ).fCount( );
					std::string name = plugin->fName( );
					wxMenuItem* item = mMenu->Append( wxID_ANY, name );	
					mOwner->Connect( item->GetId( ), wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tEditorAppWindow::fTogglePlugin ), (wxObject*)index, mOwner );			

					mOwner->fPlugins( ).fPushBack( plugin );
				}

				return true;
			}
		};

	}

	void tEditorAppWindow::fLoadPlugins( )
	{
		wxMenuBar* bar = fGuiApp( ).fMainWindow( ).GetMenuBar( );
		wxMenu *toolsMenu = new wxMenu;
		bar->Append( toolsMenu, "Plugins" );

		tAssetPluginDllDepot::fInstance( ).fForEachPlugin( tPluginLoader( this, toolsMenu ) );
	}

	void tEditorAppWindow::fOnTick( )
	{
		f32 dt = 0.f;
		const b32 onTop = fBeginOnTick( &dt );

		if( onTop )
		{
			// step dialog boxes
			b32 windowActive = ( GetForegroundWindow( ) == GetHwnd( ) );
			for( u32 i = 0; i < mEditorDialogs.fCount( ); ++i )
			{
				mEditorDialogs[ i ]->fOnTick( );
			}	

			// step camera
			mRenderPanelContainer->fOnTick( );

			// step cursor
			if( !fGuiApp( ).fCurrentCursor( ).fNull( ) )
				fGuiApp( ).fCurrentCursor( )->fOnTick( );

			// step render-only gizmos
			fGuiApp( ).fTickRenderGizmos( );

			// update any multi-segment objects
			fGuiApp( ).fEditableObjects( ).fUpdateObjectsTick( );

			// step side bar guis
			mToolsPanelContainer->fOnTick( );

			// step hot keys
			if( !fDialogInputActive( ) )
				fGuiApp( ).fHotKeys( ).fUpdateHotKeys( Input::tKeyboard::fInstance( ) );

			// clean the spatial set before rendering
			fGuiApp( ).fSceneGraph( )->fAdvanceTime( dt );
			fGuiApp( ).fSceneGraph( )->fKickCoRenderMTRunList( );

			// render active views of the scene
			Gfx::tDisplayStats selectedDisplayStats;
			fGuiApp( ).fSelectionList( ).fComputeSelectedDisplayStats( selectedDisplayStats );
			mRenderPanelContainer->fRender( &selectedDisplayStats );

			fSaveLayout( );

			if( mRefreshObjectProperties )
			{
				mRefreshObjectProperties = false;
				mObjectProperties->fOnSelectionChanged( );
				mMemoryViewerDialog->fOnSelectionChanged( fGuiApp( ).fSelectionList() );
			}

			std::stringstream objectStats;
			objectStats << "Objects: selected = " << fGuiApp( ).fSelectionList( ).fCount( ) << 
							" | hidden = " << fGuiApp( ).fEditableObjects( ).fGetHiddenCount( ) << 
							" | frozen = " << fGuiApp( ).fEditableObjects( ).fGetFrozenCount( ) << 
							" | total = " << fGuiApp( ).fEditableObjects( ).fGetObjectCount( );
			SetStatusText( objectStats.str( ).c_str( ), 3 );
		}
		else
		{
			fSleep( 1 );
		}
	}

	void tEditorAppWindow::fClearScene( b32 closing )
	{
		tToolsGuiMainWindow::fClearScene( closing );

		mExplicitDependenciesDialog->fSetDependencyList( tFilePathPtrList( ) );
		mGlobalProperties->fSetGlobalProperties( Sigml::tFile( ).mEditableProperties );

		mLayerPanel->fReset( );
		mVisibilityPanel->fReset( );
		mGCPanel->fReset( );

		mHeightFieldMaterials->fClearAtlases( );
		mPluginData.fSetCount( 0 );

		mObjectBrowserDialog->fClear( );

		if( mReadyToOpenDoc )
			fSetSelectionCursor( );

		mOpenDocDirectory.Clear();
		mOpenDocName.Clear();

		for( u32 i = 0; i < mPlugins.fCount( ); ++i )
			mPlugins[ i ]->fFileOpened( );
	}

	void tEditorAppWindow::fNewDoc( )
	{
		if( fGuiApp( ).fClearScene( ) )
			fSetStatus( "New scene" );
	}

	b32 tEditorAppWindow::fBuild( )
	{
		if( fGuiApp( ).fDocName( ) == tToolsGuiApp::cNewDocTitle )
		{
			wxMessageBox( "You must save your file before building or previewing it.", "Save First", wxOK | wxICON_WARNING );
			return false;
		}
		else if( fGuiApp( ).fActionStack( ).fIsDirty( ) )
		{
			fGuiApp( ).fSaveDoc( false );
		}

		tAssetGenScanner::fProcessSingleFile( tFilePathPtr( fGuiApp( ).fDocName( ) ), true, 
			fPlatformIdFlag( cCurrentPlatform ) | fPlatformIdFlag( mPreviewPlatform ) );

		return true;
	}

	void tEditorAppWindow::fPreview( tPreviewBuildConfig config )
	{
		if( !fBuild( ) ) return;

		const tFilePathPtr relSigmlPath = ToolsPaths::fMakeResRelative( tFilePathPtr( fGuiApp( ).fDocName( ).c_str( ) ) );
		if( FileSystem::fFileExists( Sigml::fSigmlPathToSigb( relSigmlPath ) ) )
		{
			log_line( 0, "Previewing current document" );
			std::string cmdLine;
			switch( config )
			{
			case cPreviewBuildConfigInternal:
				break;
			case cPreviewBuildConfigPlaytest:
				cmdLine += "-playtest ";
				break;
			case cPreviewBuildConfigRelease:
				cmdLine += "-release ";
				break;
			}

			cmdLine += std::string( "-preview " ) + relSigmlPath.fCStr( );
			ToolsPaths::fLaunchGame( mPreviewPlatform, cmdLine );
		}
		else
		{
			log_line( 0, "Error running AssetGen on current document" );
		}
	}

	void tEditorAppWindow::fOpenDoc( )
	{
		if( !mReadyToOpenDoc ) return;

		tFilePathPtr file = mOpenDialog->fGetSelectedFile( );
		if( !file.fNull( ) )
		{
			if( !fGuiApp( ).fClearScene( ) )
				return; // user cancelled, don't try to open new file

			fOpenDoc( file );
		}
	}

	void tEditorAppWindow::fBrowseDoc( )
	{
		if( !mReadyToOpenDoc ) return;

		std::string path;
		const b32 success = WxUtil::fBrowseForFile( 
			path, 
			mRenderPanelContainer,
			"Open Anim Pack Config", 
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ), 
			wxString( "untitled.anifig" ), 
			wxString( "Scene files|*.anifig;*.sigml" ), 
			wxFD_OPEN );

		if( success )
		{
			if( !fGuiApp( ).fClearScene( ) )
				return; // user cancelled, don't try to open new file

			fOpenDoc( tFilePathPtr( path.c_str( ) ) );
		}
	}

	void tEditorAppWindow::fOpenDoc( const tFilePathPtr& file )
	{
		// Deep copy the file name so it doesn't change after we move it to the top of the recent files list
		wxString fileName = file.fCStr( );

		if( !mReadyToOpenDoc ) return;

		if( !fGuiApp( ).fClearScene( ) )
			return; // user cancelled, don't try to open new file

		if( FileSystem::fFileExists( file ) )
		{
			if( !fDeserializeSigmlFile( file ) )
			{
				wxMessageBox( "The specified .sigml file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
				SetFocus( );
				fSetStatus( "Scene open failed" );
				return;
			}

			// set up new scene
			fGuiApp( ).fSetDocName( file.fCStr( ) );
			SetTitle( fGuiApp( ).fMakeWindowTitle( ) );
			fSetStatus( "Scene opened successfully" );

			fGuiApp( ).fAddRecentFile( file );
			fUpdateRecentFileMenu( );

			fFrameAllEveryViewport( );

			// Keep track of opened doc filename and directory
			// Remove everything before the last '\'
			mOpenDocName = fileName;
			mOpenDocName.Remove( 0, mOpenDocName.find_last_of( "\\" ) + 1 );

			// Remove everything after the last '\' 
			mOpenDocDirectory = StringUtil::fStripExtension( fileName ) ;
			mOpenDocDirectory.Remove( mOpenDocDirectory.find_last_of( "\\" ) );

			tEditablePropertyFileNameString::fSetDefaultBrowseDirectory(Sigml::tObjectProperties::fEditablePropScriptName( ), mOpenDocDirectory.c_str());

			for( u32 i = 0; i < mPlugins.fCount( ); ++i )
				mPlugins[ i ]->fFileOpened( );
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
			SetFocus( );
		}
	}

	void tEditorAppWindow::fImportDoc( )
	{
		if( !mReadyToOpenDoc ) return;

		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mRenderPanelContainer, 
			"Import",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "untitled.sigml" ),
			wxString( "*.sigml" ),
			wxFD_OPEN ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			fImportDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
		}
	}

	void tEditorAppWindow::fImportDoc( const tFilePathPtr& file )
	{
		if( !mReadyToOpenDoc ) return;

		if( FileSystem::fFileExists( file ) )
		{
			if( !fImportAndMerge( file ) )
			{
				wxMessageBox( "The specified .sigml file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
				SetFocus( );
				fSetStatus( "Scene open failed" );
				return;
			}

			fFrameAllEveryViewport( );
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
			SetFocus( );
		}
	}

	void tEditorAppWindow::fUndo( )
	{
		fGuiApp( ).fActionStack( ).fUndo( );
	}

	void tEditorAppWindow::fRedo( )
	{
		fGuiApp( ).fActionStack( ).fRedo( );
	}

	void tEditorAppWindow::fDuplicateSelected( )
	{
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tDuplicateObjectsAction( *this, fGuiApp().fEditableObjects().fGetSelectionList() ) ) );
	}

	void tEditorAppWindow::fGroupSelected( )
	{
		if( fGuiApp().fSelectionList().fCount() <= 1 )
			return;

		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tGroupSelectedObjectsAction( *this ) ) );
	}

	void tEditorAppWindow::fBreakSelected( )
	{
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tBreakSelectedGroupsAction( *this ) ) );
	}

	void tEditorAppWindow::fSelectAll( )
	{
		tEditorSelectionList previousSelection = fGuiApp( ).fSelectionList( );

		tEditableObjectContainer::tEntityMasterList shownEnts;
		fGuiApp( ).fEditableObjects( ).fGetShown( shownEnts );

		fGuiApp( ).fSelectionList( ).fClear( );
		for( u32 i = 0; i < shownEnts.fCount(); ++i )
			fGuiApp( ).fSelectionList( ).fAdd( shownEnts[i] );

		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( *this, previousSelection ) ) );
	}

	namespace 
	{
		tFilePathPtr fTempFilePath( )
		{
			FileSystem::fCreateDirectory( tFilePathPtr( std::string( ToolsPaths::fGetEngineTempFolder( ).fCStr( ) ) + "/SigEd/" ) );
			return tFilePathPtr( std::string( ToolsPaths::fGetEngineTempFolder( ).fCStr( ) ) + "/SigEd/CopyPasteTemp.sigml" );
		}
	}

	void tEditorAppWindow::fCut( )
	{
		fSerializeSigmlFile( fTempFilePath( ), true );
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tDeleteSelectedObjectsAction( fGuiApp( ).fMainWindow( ) ) ) );
	}

	void tEditorAppWindow::fCopy( )
	{
		fSerializeSigmlFile( fTempFilePath( ), true );
	}

	void tEditorAppWindow::fPaste( )
	{
		fImportAndMerge( fTempFilePath( ) );
		fGuiApp().fActionStack().fForceSetDirty();
	}

	void tEditorAppWindow::fToggleObjectProperties( )
	{
		if( mObjectProperties ) mObjectProperties->fToggle( );
	}

	void tEditorAppWindow::fToggleGlobalProperties( )
	{
		if( mGlobalProperties ) mGlobalProperties->fToggle( );
	}

	void tEditorAppWindow::fToggleObjectBrowser( )
	{
		if( mObjectBrowserDialog ) mObjectBrowserDialog->fToggle( );
	}

	void tEditorAppWindow::fTogglePlugin( wxCommandEvent& event )
	{
		u32 pluginIndex = (u32)event.m_callbackUserData;
		mPlugins[ pluginIndex ]->fToggle( );
	}

	void tEditorAppWindow::fToggleViewMode( )
	{
		fGuiApp( ).fMainWindow( ).fRenderPanelContainer( )->fToggleViewMode( );
	}

	void tEditorAppWindow::fHideSelected( )
	{
		if( fGuiApp( ).fSelectionList( ).fCount( ) == 0 )
			return;

		tFreezeHideAction* action = new tFreezeHideAction( *this, tEditableObject::cStateHidden );
		fGuiApp( ).fEditableObjects( ).fHideSelected( fGuiApp( ).fSelectionList( ) );
		action->fFinishConstruction( );
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
	}

	void tEditorAppWindow::fHideUnselected( )
	{
		if( fGuiApp( ).fEditableObjects( ).fGetShownCount( ) == 0 )
			return;

		tFreezeHideAction* action = new tFreezeHideAction( *this, tEditableObject::cStateHidden );
		fGuiApp( ).fEditableObjects( ).fHideUnselected( fGuiApp( ).fSelectionList( ) );
		action->fFinishConstruction( );
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
	}

	void tEditorAppWindow::fUnhideAll( )
	{
		if( fGuiApp( ).fEditableObjects( ).fGetHiddenCount( ) == 0 )
			return;

		tFreezeHideAction* action = new tFreezeHideAction( *this, tEditableObject::cStateHidden );
		fGuiApp( ).fEditableObjects( ).fUnhideAll( );
		action->fFinishConstruction( );
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
	}


	void tEditorAppWindow::fFreezeSelected( )
	{
		if( fGuiApp( ).fSelectionList( ).fCount( ) == 0 )
			return;

		tFreezeHideAction* action = new tFreezeHideAction( *this, tEditableObject::cStateFrozen );
		fGuiApp( ).fEditableObjects( ).fFreezeSelected( fGuiApp( ).fSelectionList( ) );
		action->fFinishConstruction( );
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
	}

	void tEditorAppWindow::fFreezeUnselected( )
	{
		if( fGuiApp( ).fEditableObjects( ).fGetShownCount( ) == 0 )
			return;

		tFreezeHideAction* action = new tFreezeHideAction( *this, tEditableObject::cStateFrozen );
		fGuiApp( ).fEditableObjects( ).fFreezeUnselected( fGuiApp( ).fSelectionList( ) );
		action->fFinishConstruction( );
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
	}

	void tEditorAppWindow::fUnfreezeAll( )
	{
		if( fGuiApp( ).fEditableObjects( ).fGetFrozenCount( ) == 0 )
			return;

		tFreezeHideAction* action = new tFreezeHideAction( *this, tEditableObject::cStateFrozen );
		fGuiApp( ).fEditableObjects( ).fUnfreezeAll( );
		action->fFinishConstruction( );
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
	}

	void tEditorAppWindow::fSetSelectionCursor( )
	{
		mManipTools->fSetSelectionCursor( );
	}

	void tEditorAppWindow::fSnapToGround( b32 onlySnapIfSpecified )
	{
		const tEditorSelectionList& selList = fGuiApp( ).fSelectionList( );
		tManipulationGizmoAction* action = new tManipulationGizmoAction( *this, selList );

		selList.fSnapToGround( onlySnapIfSpecified );

		action->fEnd( );
		fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
	}

	void tEditorAppWindow::fRefreshHeightFieldMaterialTileFactors( )
	{
		tDynamicArray<f32> tilingFactors;
		fAcquireHeightFieldMaterialTileFactors( tilingFactors );

		tGrowableArray< tEditableTerrainGeometry* > terrainObjs;
		fGuiApp( ).fEditableObjects( ).fCollectAllByType< tEditableTerrainGeometry >( terrainObjs );
		for( u32 i = 0; i < terrainObjs.fCount( ); ++i )
			terrainObjs[ i ]->fUpdateMaterialTilingFactors( tilingFactors );
	}

	void tEditorAppWindow::fAcquireHeightFieldMaterialTileFactors( tDynamicArray<f32>& tilingFactors )
	{
		if( mHeightFieldMaterials )
			mHeightFieldMaterials->fAcquireHeightFieldMaterialTileFactors( tilingFactors );
	}

	void tEditorAppWindow::fCleanHeightFieldMaterials( u32 firstMat, u32 secondMat, u32 thirdMat )
	{
		tEditorSelectionList& selectedList = fGuiApp( ).fSelectionList( );
		if( selectedList.fCount( ) == 0 )
			return;

		for( u32 i = 0; i < selectedList.fCount( ); ++i )
		{
			const tEntityPtr& thisEnt = selectedList[ i ];

			tEditableTerrainEntity* terraintEnt = thisEnt->fDynamicCast< tEditableTerrainEntity >( );
			if( !terraintEnt )
				continue;

			tModifyTerrainMaterialAction* action = new tModifyTerrainMaterialAction( *this, thisEnt );
			terraintEnt->fCleanTextureMaterials( firstMat, secondMat, thirdMat );
			action->fEnd( );
			fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
		}
	}

	void tEditorAppWindow::fAddEditorDialog( tEditorDialog * dlg )
	{
		(void)mEditorDialogs.fFindOrAdd( dlg );
	}

	void tEditorAppWindow::fRemoveEditorDialog( tEditorDialog * dlg )
	{
		mEditorDialogs.fFindAndErase( dlg );
	}

	void tEditorAppWindow::fOpenRecent( u32 ithRecentFile )
	{
		const Win32Util::tRecentlyOpenedFileList& recentFiles = fGuiApp( ).fRecentFiles( );
		fOpenDoc( recentFiles[ ithRecentFile ] );
	}

	void tEditorAppWindow::fOnClose( )
	{
		for( u32 i = 0; i < mEditorDialogs.fCount( ); ++i )
			mEditorDialogs[ i ]->fSave( );
	}

	void tEditorAppWindow::fOnActionUndoOrRedo( tEditorActionStack& stack )
	{
		mRefreshObjectProperties = true;
		fRefreshLayers( );
	}

	void tEditorAppWindow::fOnDirty( tEditorActionStack& stack )
	{
		SetTitle( fGuiApp( ).fMakeWindowTitle( ) );
	}

	void tEditorAppWindow::fOnSelChanged( tEditorSelectionList& selectedObjects )
	{
		mRefreshObjectProperties = true;
	}

	void tEditorAppWindow::fRefreshObjectProperties( )
	{
		mRefreshObjectProperties = true;
	}

	void tEditorAppWindow::fRefreshLayers( )
	{
		mRefreshObjectProperties = true;
		mLayerPanel->fRefreshLists( );
		mVisibilityPanel->fRefreshLists( );
	}

	void tEditorAppWindow::fOnObjectSerialized( u32 nObjectsSerialized, u32 nObjectsTotal )
	{
		sigassert( !mProgressBar.fNull( ) );
		mProgressBar->fUpdate( nObjectsSerialized, nObjectsTotal );
	}

	b32 tEditorAppWindow::fSerializeDoc( const tFilePathPtr& path )
	{
		return fSerializeSigmlFile( path, false );
	}

	b32 tEditorAppWindow::fSerializeSigmlFile( const tFilePathPtr& filePath, b32 selected )
	{
		mProgressBar.fReset( new tSaveProgressDialog( mRenderPanelContainer, "Saving File" ) );

		Sigml::tFile sigmlFile;

		tEditableObjectContainer::tOnObjectSerialized cb = 
			make_delegate_memfn( tEditableObjectContainer::tOnObjectSerialized, tEditorAppWindow, fOnObjectSerialized );
		fGuiApp( ).fEditableObjects( ).fSerialize( sigmlFile.mObjects, &cb, selected );

		mHeightFieldMaterials->fToSigmlFile( sigmlFile );
		sigmlFile.mPluginData = mPluginData;

		mProgressBar->tWxProgressDialog::fUpdate( 1.f, "Writing file to disk (this may stall for a sec)..." );

		sigmlFile.mExplicitDependencies = mExplicitDependenciesDialog->fDependencyList( );
		sigmlFile.mEditableProperties = mGlobalProperties->fGlobalProperties( );
		mLayerPanel->fSaveLayers( sigmlFile, selected ? &fGuiApp( ).fEditableObjects( ).fGetSelectionList( ) : NULL );
		mVisibilityPanel->fSaveLayers( sigmlFile, selected ? &fGuiApp( ).fEditableObjects( ).fGetSelectionList( ) : NULL );
		mGCPanel->fSave( sigmlFile );

		b32 success = sigmlFile.fSaveXml( filePath, true );

		mProgressBar.fRelease( );
		SetFocus( );

		return success;
	}

	b32 tEditorAppWindow::fDeserializeSigmlFile( const tFilePathPtr& filePath )
	{
		mProgressBar.fReset( new tLoadProgressDialog( mRenderPanelContainer, "Loading File", fGetResourceDepot( ) ) );

		// Check if the file is checked out or out of date. Candidate to move to its own function.
		std::stringstream msgTextSS;
		std::string userList;
		if ( ToolsPaths::fIsCheckedOut( filePath, userList ) ) // Checked out
			msgTextSS << "The file [" << filePath.fCStr( ) << "] is checked out by:\n\n" << userList;

		if( ToolsPaths::fIsOutOfDate( filePath ) ) // Out of date
		{
			if ( msgTextSS.str().length() )
				msgTextSS << "\n\n";

			msgTextSS << "The file [" << filePath.fCStr( ) << "] is out-of-date.";
		}

		tWxNotificationMessage notify( this, "Warning!", msgTextSS.str().c_str() );
		if ( msgTextSS.str( ).length( ) )
		{
#if defined( build_release ) && defined( target_tools)
			notify.Show( );
#endif //defined( build_release ) && defined( target_tools)
		}

		Sigml::tFile sigmlFile;
		if( !sigmlFile.fLoadXml( filePath ) )
			return false;

		tEditableObjectContainer::tOnObjectSerialized cb = 
			make_delegate_memfn( tEditableObjectContainer::tOnObjectSerialized, tEditorAppWindow, fOnObjectSerialized );

		tEditableObjectContainer& edObjs = fGuiApp( ).fEditableObjects( );

		edObjs.fSetLayers( sigmlFile.mLayers );

		const b32 success = edObjs.fDeserialize( sigmlFile.mObjects, &cb );

		mHeightFieldMaterials->fFromSigmlFile( sigmlFile );
		mPluginData = sigmlFile.mPluginData;

		mProgressBar.fRelease( );

		if( edObjs.fGetObjectCount( ) < sigmlFile.mObjects.fCount( ) )
		{
			wxMessageBox( "Not all objects in the current file could be loaded. "
				"This means your file contains invalid object types, which were skipped.  "
				"This can happen when you try to open a file that was created in another editor (e.g., Maya).", "Incomplete file", wxOK | wxICON_WARNING );
		}

		mExplicitDependenciesDialog->fSetDependencyList( sigmlFile.mExplicitDependencies );
		mGlobalProperties->fSetGlobalProperties( sigmlFile.mEditableProperties );

		mLayerPanel->fReset( sigmlFile );
		mVisibilityPanel->fReset( sigmlFile );
		mGCPanel->fReset( sigmlFile );

		fRefreshHeightFieldMaterialTileFactors( );

		SetFocus( );

		// If the box hasn't been dismissed, catch here until it is.
		if( notify.IsShown( ) )
			notify.ShowModal( );

		return success;
	}

	b32 tEditorAppWindow::fImportAndMerge( const tFilePathPtr& filePath )
	{
		mProgressBar.fReset( new tLoadProgressDialog( mRenderPanelContainer, "Loading File", fGetResourceDepot( ) ) );

		Sigml::tFile sigmlFile;
		if( !sigmlFile.fLoadXml( filePath ) )
		{
			mProgressBar.fRelease( );
			SetFocus( );
			return false;
		}

		tEditableObjectContainer::tOnObjectSerialized cb = 
			make_delegate_memfn( tEditableObjectContainer::tOnObjectSerialized, tEditorAppWindow, fOnObjectSerialized );

		tEditableObjectContainer& edObjs = fGuiApp( ).fEditableObjects( );

		// Import and update GUIDs to prevent collisions.
		edObjs.fImportObjects( sigmlFile.mObjects, &cb );

		mProgressBar.fRelease( );

		if( edObjs.fGetObjectCount( ) < sigmlFile.mObjects.fCount( ) )
		{
			wxMessageBox( "Not all objects in the current file could be loaded. "
				"This means your file contains invalid object types, which were skipped.  "
				"This can happen when you try to open a file that was created in another editor (e.g., Maya).", "Incomplete file", wxOK | wxICON_WARNING );
		}

		// Merge the layer information from this file.
		edObjs.fMergeLayerColorEntries( sigmlFile.mLayers );
		mLayerPanel->fMergeLayers( sigmlFile );
		mVisibilityPanel->fMergeLayers( sigmlFile );

		if( sigmlFile.mPluginData.fCount( ) )
			log_warning( "Plugin data skipped! Figure out how to merge this data." );

		SetFocus( );

		return true;
	}

	void tEditorAppWindow::fOnGlobalPropertiesFocus( )
	{
		wxString scriptPathFileName = StringUtil::fStripExtension( mOpenDocName ) + "_logic.nut";
		tEditablePropertyFileNameString::fSetDefaultFileName(Sigml::tObjectProperties::fEditablePropScriptName( ), scriptPathFileName.c_str());
	}

	void tEditorAppWindow::fOnObjectPropertiesFocus( )
	{
		wxString scriptPathFileName = StringUtil::fStripExtension( mOpenDocName ) + "_obj.nut";
		tEditablePropertyFileNameString::fSetDefaultFileName(Sigml::tObjectProperties::fEditablePropScriptName( ), scriptPathFileName.c_str());
	}
}
