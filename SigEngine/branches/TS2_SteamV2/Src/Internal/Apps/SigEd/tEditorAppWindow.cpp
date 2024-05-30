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
#include "tHeightFieldPaintCursor.hpp"
#include "tGroundCoverPanel.hpp"

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
					mResourceDepot->fUpdateLoadingResources( 10.f );
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
		, mLayerPanel( 0 )
		, mHeightFieldMaterials( 0 )
		, mGCPanel( 0 )
		, mToolsPanelContainer( 0 )
		, mObjectProperties( 0 )
		, mGlobalProperties( 0 )
		, mRemapReferencesDialog( 0 )
		, mExplicitDependenciesDialog( 0 )
	{
		// create primary window containers

		//wxPanel* leftPanel = new wxPanel( this, 0, 0, 360, -1 );
		//leftPanel->SetMinSize( wxSize( 360, -1 ) );
		//leftPanel->SetMaxSize( wxSize( 360, -1 ) );

		mRenderPanelContainer = new tWxRenderPanelContainer( this, guiApp.fRegKeyName( ) + "\\RenderPanels", true );
		mToolsPanelContainer = new tWxToolsPanelContainer( *this );
		mObjectProperties = new tEditableObjectProperties( this, false );
		mGlobalProperties = new tEditableObjectProperties( this, true );
		mRemapReferencesDialog = new tRemapReferencesDialog( this );
		mExplicitDependenciesDialog = new tSigEdExplicitDependenciesDialog( this );
		mObjectBrowserDialog = new tObjectBrowserDialog( this );
		//mScriptEd = new tScriptEditorDialog( this );
		mEditorDialogs.fPushBack( mObjectProperties );
		mEditorDialogs.fPushBack( mGlobalProperties );
		mEditorDialogs.fPushBack( mRemapReferencesDialog );
		mEditorDialogs.fPushBack( mExplicitDependenciesDialog );
		mEditorDialogs.fPushBack( mObjectBrowserDialog );
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
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorBuildHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorPreviewHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorPreviewReleaseHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorUndoHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorRedoHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorFrameSelectedHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorFrameAllHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorToggleObjectPropsHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorToggleGlobalPropsHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorToggleFindHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorHideSelectedHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorToggleViewMode( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorCutHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorCopyHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tEditorPasteHotKey( hotKeyTable, this ) ) );

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
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tHideObjectsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tFreezeObjectsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSnapObjectsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tChangeShadeModeContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSaveHeightMapBitmapsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tOpenSigmlContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tDisplayObjectPropertiesContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertToAttachmentPointsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertToShapesContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertToNavGraphContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSelectEntireNavGraphContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tConvertAttachmentPointsToWayPointsContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSelectEntirePathContextAction( this ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tSelectPathDecalContextAction( this ) ) );
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

	void tEditorAppWindow::fOnTick( )
	{
		f32 dt = 0.f;
		const b32 onTop = fBeginOnTick( &dt );

		if( onTop )
		{
			// step dialog boxes
			for( u32 i = 0; i < mEditorDialogs.fCount( ); ++i )
			{
				mEditorDialogs[ i ]->fSetTopMost( true );
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
			for( u32 i = 0; i < mEditorDialogs.fCount( ); ++i )
				mEditorDialogs[ i ]->fSetTopMost( false );
			fSleep( 1 );
		}
	}

	void tEditorAppWindow::fClearScene( b32 closing )
	{
		tToolsGuiMainWindow::fClearScene( closing );

		mExplicitDependenciesDialog->fSetDependencyList( tFilePathPtrList( ) );
		mGlobalProperties->fSetGlobalProperties( Sigml::tFile( ).mEditableProperties );

		mLayerPanel->fReset( );
		mGCPanel->fReset( );

		mHeightFieldMaterials->fClearAtlases( );

		mObjectBrowserDialog->fClear( );

		if( mReadyToOpenDoc )
			fSetSelectionCursor( );
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

	void tEditorAppWindow::fPreview( b32 release, b32 profile )
	{
		if( !fBuild( ) ) return;

		const tFilePathPtr relSigbPath = Sigml::fSigmlPathToSigb( ToolsPaths::fMakeResRelative( tFilePathPtr( fGuiApp( ).fDocName( ).c_str( ) ) ) );
		if( FileSystem::fFileExists( relSigbPath ) )
		{
			log_line( 0, "Previewing current document" );
			std::string cmdLine;
			if( profile )		cmdLine += "-profile ";
			else if( release )	cmdLine += "-release ";
			cmdLine += std::string( "-preview " ) + relSigbPath.fCStr( );
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

	void tEditorAppWindow::fOpenDoc( const tFilePathPtr& file )
	{
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

		mProgressBar->tWxProgressDialog::fUpdate( 1.f, "Writing file to disk (this may stall for a sec)..." );

		sigmlFile.mExplicitDependencies = mExplicitDependenciesDialog->fDependencyList( );
		sigmlFile.mEditableProperties = mGlobalProperties->fGlobalProperties( );
		mLayerPanel->fSaveLayers( sigmlFile, selected ? &fGuiApp( ).fEditableObjects( ).fGetSelectionList( ) : NULL );
		mGCPanel->fSave( sigmlFile );

		b32 success = sigmlFile.fSaveXml( filePath, true );

		mProgressBar.fRelease( );
		SetFocus( );

		return success;
	}

	b32 tEditorAppWindow::fDeserializeSigmlFile( const tFilePathPtr& filePath )
	{
		mProgressBar.fReset( new tLoadProgressDialog( mRenderPanelContainer, "Loading File", fGetResourceDepot( ) ) );

		Sigml::tFile sigmlFile;
		if( !sigmlFile.fLoadXml( filePath, true ) )
			return false;

		tEditableObjectContainer::tOnObjectSerialized cb = 
			make_delegate_memfn( tEditableObjectContainer::tOnObjectSerialized, tEditorAppWindow, fOnObjectSerialized );

		tEditableObjectContainer& edObjs = fGuiApp( ).fEditableObjects( );

		edObjs.fSetLayers( sigmlFile.mLayers );

		mHeightFieldMaterials->fFromSigmlFile( sigmlFile );

		const b32 success = edObjs.fDeserialize( sigmlFile.mObjects, &cb );

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
		mGCPanel->fReset( sigmlFile );

		fRefreshHeightFieldMaterialTileFactors( );

		SetFocus( );

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

		SetFocus( );

		return true;
	}

}
