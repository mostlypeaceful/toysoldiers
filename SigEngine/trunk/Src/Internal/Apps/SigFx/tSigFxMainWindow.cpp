#include "SigFxPch.hpp"
#include "tSigFxMainWindow.hpp"
#include "tSigFxMatEd.hpp"
#include "tMeshSelectorDialog.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tSigFxKeyline.hpp"
#include "Editor/tEditorAction.hpp"
#include "tAssetGenScanner.hpp"
#include "tSigFxContextMenuActions.hpp"
#include "tGizmoGeometry.hpp"
#include "tTranslationGizmo.hpp"
#include "tTranslationGizmoGeometry.hpp"
#include "Fxml.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tWxToolsPanelContainer.hpp"
#include "FileSystem.hpp"
#include <wx/aui/auibook.h>
#include "tSigFxGraphline.hpp"
#include "tSigFxWindowHotKeys.hpp"
#include "FxEditor/tSigFxAttractor.hpp"
#include "FxEditor/tSigFxMeshSystem.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tToolsMouseAndKbCamera.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "tSceneGraphFile.hpp"
#include "tManipulationGizmoAction.hpp"
#include "Threads/tProcess.hpp"
#include "tCmdLineOption.hpp"
#include "tFxFileRefEntity.hpp"

// For lod streaming
#include "Gfx/tGeometryFile.hpp"

namespace Sig
{
	const char tSigFxMainWindow::cNewDocTitle[] = "(untitled)";

	enum tAction
	{
		cActionNew = 1,
		cActionOpen,
		cActionOpenRecent,
		cActionSave = cActionOpenRecent + tToolsGuiApp::cMaxRecentlyOpenedFiles,
		cActionSaveAs,
		cActionBuild,
		cActionRecord,
		cActionCaptureFrame,
		cActionImportFromTemplate,
		cActionImport = cActionImportFromTemplate + tSigFxMainWindow::cMaxTemplateFiles,
		
		cActionQuit,

		cActionUndo,
		cActionRedo,

		cActionAbout,
		cActionHotKeyHelp,

		cActionFrameSel,
		cActionFrameAll,

		cActionToggleMatEd,
		cActionToggleMeshSelector,
		cActionObjectProperties,
		cActionGlobalProperties,
		cActionRemapReferences,

		cActionPreviewInternal,
		cActionPreviewPlaytest,
		cActionPreviewRelease,

		cActionHotKeyLaunchSigEd,
	};

	BEGIN_EVENT_TABLE( tSigFxMainWindow, wxFrame )

		EVT_CLOSE(										tSigFxMainWindow::fOnClose )
		EVT_MOUSE_EVENTS(								tSigFxMainWindow::fOnMouseEvents )
		
		EVT_MENU(				cActionNew,				tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionOpen,			tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionSave,			tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionSaveAs,			tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionImport,			tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionBuild,			tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionRecord,			tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionCaptureFrame,	tSigFxMainWindow::fOnAction )
		
		EVT_MENU(				cActionQuit,			tSigFxMainWindow::fOnAction )

		EVT_MENU(				cActionUndo,			tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionRedo,			tSigFxMainWindow::fOnAction )

		EVT_MENU(				cActionAbout,			tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionHotKeyHelp,		tSigFxMainWindow::fOnAction )
		
		EVT_MENU(				cActionFrameSel,		tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionFrameAll,		tSigFxMainWindow::fOnAction )

		EVT_MENU(				cActionToggleMatEd,		tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionToggleMeshSelector,tSigFxMainWindow::fOnAction )
		
		EVT_MENU(				cActionObjectProperties,tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionGlobalProperties,tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionRemapReferences,	tSigFxMainWindow::fOnAction )

		EVT_MENU(				cActionPreviewInternal,  tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionPreviewPlaytest,  tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionPreviewRelease,	tSigFxMainWindow::fOnAction )
		EVT_MENU(				cActionHotKeyLaunchSigEd,	tSigFxMainWindow::fOnAction )		
	END_EVENT_TABLE( )

	tSigFxMainWindow::tSigFxMainWindow( tToolsGuiApp& guiApp )
		: tToolsGuiMainWindow( guiApp )
		, mMatEd( 0 )
		, mMeshSelector( 0 )
		, mToolsPanelContainer( 0 )
		, mTabControl( 0 )
		, mKeyline( 0 )
		, mGraphline( 0 )
		, mDocName( cNewDocTitle )
		, mPlaybackTimeFrequency( 0 )
		, mBackgroundColourPicker( 0 )
		, mIdealFrameDt( 1.f/30.f )
		, mLastElapsed( 0.f )
		, mTimeSinceLastIdealUpdate( 0.f )
		, mSelectionChanged( false )
		, mCaptureFrame( false )
		, mPreviewingCurrentDoc( false )
		, mCheckCommandArgs( true )
		, mSavingInProgress( false )
		, mLastNumSceneObjects( 0 )
		, mSyncNextFrame( false )
	{
		SetBackgroundColour( wxColour( 145, 138, 127 ) );

		// set icon
		SetIcon( wxIcon( "appicon" ) );

		// this thing here gets added to the scene graph so
		// no need to call delete on it!
		mFxScene = new FX::tSigFxSystem( );
		mFxScene->fSpawnImmediate( fGuiApp( ).fSceneGraph( )->fRootEntity( ) );
		
		mLastDirectory = mLastImportDirectory = wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) + wxString( "\\effects\\fx\\" ) );
		mTemplateFxFilePath = wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) + wxString( "\\Effects\\Fx\\" ) );

		u32 seed = Time::fGetStamp( );
		tRandom::fSubjectiveRand( ) = tRandom( seed );
		
		// setup context menu...
		//fContextActions( ).fPushBack( tEditorContextActionPtr( new tAddNewParticleSystemContextAction( this, mFxScene ) ) );
		fContextActions( ).fPushBack( tEditorContextActionPtr( new tCloneSelectedFxObjectsContextAction( this ) ) );

		// create primary window containers
		mRenderPanelContainer = new tWxRenderPanelContainer( this, guiApp.fRegKeyName( ) + "\\RenderPanels", true );

		// create primary side-panel toolbox container
		mToolsPanelContainer = new tWxToolsPanelContainer( *this );

		// create material editor window
		mMatEd = new tSigFxMatEd( this, fGuiApp( ).fActionStack( ), guiApp.fRegKeyName( ) + "\\MatEd" );
		mMatEd->SetTitle( "MatEd in SigFx" );

		// create mesh selector window
		mMeshSelector = new tMeshSelectorDialog( this, fGuiApp( ).fActionStack( ), guiApp.fRegKeyName( ) + "\\MeshSelector" );


		// create the bottom panel that consists of the Keyline, Graphline and the Timeline w/Scrubber.
		wxPanel* bottompanel = new wxPanel( this );
		wxBoxSizer* bottomsizer = new wxBoxSizer( wxVERTICAL );
		bottompanel->SetSizer( bottomsizer );
		
		// SIZERS
		wxBoxSizer* rendercontrolsplit = new wxBoxSizer( wxVERTICAL );
		wxBoxSizer* Keylinesplit = new wxBoxSizer( wxHORIZONTAL );
		rendercontrolsplit->Add( mRenderPanelContainer, 1, wxEXPAND | wxALL, 0 );

		rendercontrolsplit->Add( bottompanel, 0, wxEXPAND | wxALL, 0 );
		//rendercontrolsplit->Add( mTabControl, 0, wxEXPAND | wxALL, 0 );


		Keylinesplit->Add( rendercontrolsplit, 1, wxEXPAND | wxALL, 0 );
		Keylinesplit->Add( mToolsPanelContainer->fGetContainerPanel( ), 0, wxEXPAND | wxALL, 0 );
		SetSizer( Keylinesplit );

		mTimeline = new tSigFxTimeline( bottompanel );
		mTabControl = new tTabControl( bottompanel, this );
		
		bottomsizer->Add( mTimeline, 0, wxEXPAND | wxALL, 0 );
		bottomsizer->Add( mTabControl, 0, wxEXPAND | wxALL, 0 );

		mKeyline = new tSigFxKeyline( mTabControl, this, mFxScene, mTimeline );
		mGraphline = new tSigFxGraphline( mTabControl, mKeyline );
		mKeyline->fSetGraphline( mGraphline );
		mTimeline->fSetKeyline( mKeyline );

		// order of Graphline or Keyline first showing up. Make this a reg thing.
		mTabControl->AddPage( mGraphline, wxString( "Graphline" ) );
		mTabControl->AddPage( mKeyline, wxString( "Keyline" ) );

		// HOT KEYS
		tEditorHotKeyTable& hotKeyTable = guiApp.fHotKeys( );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxFrameSelectedHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxPlayPauseHotKey( hotKeyTable, mKeyline ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxResetSceneHotKey( hotKeyTable, mKeyline ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxUndoHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxRedoHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxSaveHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxSBuildHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxOpenHotKey( hotKeyTable, this ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxCenterSelectedObjectsHotKey( hotKeyTable, this ) ) );

		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxFrameForwardHotKey( hotKeyTable, mKeyline ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxFrameBackwardHotKey( hotKeyTable, mKeyline ) ) );

		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxSwitchTabControlHotKey( hotKeyTable, mTabControl, 0, Sig::Input::tKeyboard::cButtonG ) ) );
		mHotKeyStorage.fPushBack( tEditorHotKeyPtr( new tSigFxSwitchTabControlHotKey( hotKeyTable, mTabControl, 1, Sig::Input::tKeyboard::cButtonK ) ) );
			
		//	EVENTS
		mOnDirty.fFromMethod<tSigFxMainWindow, &tSigFxMainWindow::fOnDirty>( this );
		mOnSelChanged.fFromMethod<tSigFxMainWindow, &tSigFxMainWindow::fOnSelChanged>( this );
		mOnAddAction.fFromMethod<tSigFxMainWindow, &tSigFxMainWindow::fOnActionUndoOrRedo>( this );
	
		fGuiApp( ).fSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelChanged );
		fGuiApp( ).fActionStack( ).mOnDirty.fAddObserver( &mOnDirty );
		fGuiApp( ).fActionStack( ).mOnAddAction.fAddObserver( &mOnAddAction );

		// hook up the recent file menu
		for( u32 i = 0; i < tToolsGuiApp::cMaxRecentlyOpenedFiles; ++i )
			Connect( cActionOpenRecent + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tSigFxMainWindow::fOnAction ), 0, this );

		// HIDE THE PANELS AT FIRST
		mTabControl->Hide( );
		mToolsPanelContainer->Hide( );
		mTimeline->Hide( );

		fAddMenus( );
		fAddToolPanel( );

		fLoadLayout( );
		SetFocus( );
	}

	void tSigFxMainWindow::fOnDirty( tEditorActionStack& stack )
	{
		SetTitle( fGuiApp( ).fMakeWindowTitle( ) );
	}
			
	void tSigFxMainWindow::fOnMouseEvents( wxMouseEvent& event )
	{
		mGraphline->fOnMouseWheel( event );
		event.Skip( );
	}

	void tSigFxMainWindow::fOnActionUndoOrRedo( tEditorActionStack& stack )
	{
		if( stack.fIsDirty( ) && !mSelectionChanged )
			fSync( );
	}

	void tSigFxMainWindow::fUndo( )
	{
		fGuiApp( ).fActionStack( ).fUndo( );
		fSync( );
	}

	void tSigFxMainWindow::fRedo( )
	{
		fGuiApp( ).fActionStack( ).fRedo( );
		fSync( );
	}

	void tSigFxMainWindow::fCenterSelectedObjects( )
	{
		tEditorActionPtr action( new tManipulationGizmoAction( *this, fGuiApp( ).fSelectionList( ) ) );
		fGuiApp( ).fActionStack( ).fClearDirty( );
		fGuiApp( ).fActionStack( ).fAddAction( action );

		for( u32 i = 0; i < fGuiApp( ).fSelectionList( ).fCount( ); ++i )
		{
			tEntityPtr entity = fGuiApp( ).fSelectionList( )[ i ];
			tEditableObject* master = entity->fDynamicCast< tEditableObject >( );
			if( !master )
				continue;
			master->fMoveTo( Math::tVec3f::cZeroVector );
		}

		action->fEnd( );
	}

	void tSigFxMainWindow::fCopySelectedObjects( )
	{
		tEditorSelectionList original;
		original.fReset( fGuiApp( ).fSelectionList( ), false );

		fGuiApp( ).fSelectionList( ).fClear( );

		for( u32 i = 0; i < original.fCount( ); ++i )
		{
			tEntityPtr entity = original[ i ];
			tEditableObject* master = entity->fDynamicCast< tEditableObject >( );
			if( !master )
				continue;

			Sigml::tObjectPtr sigmlObject = master->fSerialize( true );
			tEditableObject* eo = sigmlObject->fCreateEditableObject( fGuiApp( ).fEditableObjects( ) );
			sigassert( eo );

			tSigFxParticleSystem* cloneParticleSystem = eo->fDynamicCast< tSigFxParticleSystem >( );
			tSigFxAttractor* cloneAttractor = eo->fDynamicCast< tSigFxAttractor >( );
			tSigFxMeshSystem* cloneMeshSystem = eo->fDynamicCast< tSigFxMeshSystem >( );

			if( cloneParticleSystem )
			{
				tSigFxParticleSystem* masterParticleSystem = master->fDynamicCast< tSigFxParticleSystem >( );
				cloneParticleSystem->fClone( *masterParticleSystem );
			}
			if( cloneAttractor )
			{
				tSigFxAttractor* masterAttractor = master->fDynamicCast< tSigFxAttractor >( );
				cloneAttractor->fClone( *masterAttractor );
			}
			if( cloneMeshSystem  )
			{
				tSigFxMeshSystem* masterMeshSystem = master->fDynamicCast< tSigFxMeshSystem >( );
				cloneMeshSystem->fClone( *masterMeshSystem );
			}

			eo->fMoveTo( master->fObjectToWorld( ) );
			eo->fAddToWorld( );

			fGuiApp( ).fSelectionList( ).fAdd( eo->fToSmartPtr( ) );
		}
	}

	void tSigFxMainWindow::fSync( )
	{
		mSyncNextFrame = true;
	}

	void tSigFxMainWindow::fDoActualSync( )
	{
		mSyncNextFrame = false;
		tEditableObjectContainer& list = fGuiApp( ).fEditableObjects( );

		tGrowableArray< tSigFxParticleSystem* > systems;
		tGrowableArray< tSigFxAttractor* > attractors;
		tGrowableArray< tSigFxMeshSystem* > meshSystems;
		tGrowableArray< tSigFxLight* > lights;

		list.fCollectByType( systems );
		list.fCollectByType( attractors );
		list.fCollectByType( meshSystems );
		list.fCollectByType( lights );

		mFxScene->fClearSystems( );

		for ( u32 i = 0; i < attractors.fCount( ); ++i )
		{
			if( attractors[ i ]->fAttractorName( ).fLength( ) < 1 )
			{
				tStringPtr name( wxString::Format( "Attractor %i", i ) );
				attractors[ i ]->fSetAttractorName( name );
			}
			else
			{
				// check for duplicate file names
				for( u32 j = 0; j < i; ++j )
				{
					if( attractors[ i ]->fAttractorName( ).fCStr( ) == attractors[ j ]->fAttractorName( ).fCStr( ) )
					{
						std::string str = attractors[ i ]->fAttractorName( ).fCStr( );
						str += " Copy";
						attractors[ i ]->fSetAttractorName( tStringPtr( str.c_str( ) ) );
					}
				}
			}
			mFxScene->fAddParticleAttractor( attractors[ i ]->fGetAttractor( ) );
		}

		for( u32 i = 0; i < systems.fCount( ); ++i )
		{			
			if( systems[ i ]->fParticleSystemName( ).fLength( ) < 1 )
			{
				tStringPtr name( wxString::Format( "Particle System %i", i ) );
				systems[ i ]->fSetParticleSystemName( name );
			}
			else
			{
				// check for duplicate file names
				for( u32 j = 0; j < i; ++j )
				{
					if( systems[ i ]->fParticleSystemName( ).fCStr( ) == systems[ j ]->fParticleSystemName( ).fCStr( ) )
					{
						std::string str = systems[ i ]->fParticleSystemName( ).fCStr( );
						str += " Copy";
						systems[ i ]->fSetParticleSystemName( tStringPtr( str.c_str( ) ) );
					}
				}
			}
			mFxScene->fAddParticleSystem( systems[ i ]->fGetParticleSystem( ) );
			
			tAttachmentEntity* attach = mFxScene->fCheckForAttachmentToAttachmentEntities( systems[ i ]->fGetParticleSystem( ) );
			systems[ i ]->fSetAttachmentEntity( attach );
		}

		for( u32 i = 0; i < meshSystems.fCount( ); ++i )
		{
			
			if( meshSystems[ i ]->fFxMeshSystemName( ).fLength( ) < 1 )
			{
				tStringPtr name( wxString::Format( "Mesh System %i", i ) );
				meshSystems[ i ]->fSetMeshSystemName( name );
			}
			else
			{
				// check for duplicate file names
				for( u32 j = 0; j < i; ++j )
				{
					if( meshSystems[ i ]->fFxMeshSystemName( ).fCStr( ) == meshSystems[ j ]->fFxMeshSystemName( ).fCStr( ) )
					{
						std::string str = meshSystems[ i ]->fFxMeshSystemName( ).fCStr( );
						str += " Copy";
						meshSystems[ i ]->fSetMeshSystemName( tStringPtr( str.c_str( ) ) );
					}
				}
			}
			mFxScene->fAddFxMeshSystem( meshSystems[ i ]->fFxMeshSystem( ) );
		}

		for( u32 i = 0; i < lights.fCount( ); ++i )
		{
			
			if( lights[ i ]->fGetName( ).length( ) < 1 )
			{
				std::string name( wxString::Format( "Light %i", i ) );
				lights[ i ]->fSetEditableProperty( name, Sigml::tObject::fEditablePropObjectName( ) );
			}
			else
			{
				// check for duplicate file names
				for( u32 j = 0; j < i; ++j )
				{
					if( lights[ i ]->fGetName( ).c_str( ) == lights[ j ]->fGetName( ).c_str( ) )
					{
						std::string str = lights[ i ]->fGetName( ).c_str( );
						str += " Copy";
						lights[ i ]->fSetEditableProperty( str, Sigml::tObject::fEditablePropObjectName( ) );
					}
				}
			}
			mFxScene->fAddLight( lights[ i ]->fGetLight( ) );
		}


		mGraphline->fRefreshNodes( );
		mKeyline->fImmediateUpdate( true );
		mCreateNewEffectPanel->fRefresh( );
	}

	void tSigFxMainWindow::fOnSelChanged( tEditorSelectionList& selectedObjects )
	{
		mSelectionChanged = false;
		if( selectedObjects.fCount( ) > 0 )
			mSelectionChanged = true;
		
		u32 currentCount = fGuiApp( ).fEditableObjects( ).fGetShownCount( );
		if( currentCount != mLastNumSceneObjects )
			fSync( );
		mLastNumSceneObjects = currentCount;

		mTabControl->fBuildPageFromEntities( selectedObjects );
		mFxmlPropertiesPanel->fUpdateSelectedList( selectedObjects ); // Just following the convention for consistency.
		mParticleSystemPropertiesPanel->fUpdateSelectedList( selectedObjects );
		mAttractorPropertiesPanel->fUpdateSelectedList( selectedObjects );
		mMeshSystemPropertiesPanel->fUpdateSelectedList( selectedObjects );
		mCreateNewEffectPanel->fUpdateSelectedList( selectedObjects );
		mLightPropertiesPanel->fUpdateSelectedList( selectedObjects );

		if( mMeshSelector )
			mMeshSelector->fUpdateSelectedList( selectedObjects );
	}

	tSigFxMainWindow::~tSigFxMainWindow( )
	{
		mHotKeyStorage.fDeleteArray( );

		delete mManipPanel;
		delete mCreateNewEffectPanel;
		delete mFxmlPropertiesPanel;
		delete mParticleSystemPropertiesPanel;
		delete mAttractorPropertiesPanel;
		delete mMeshSystemPropertiesPanel;
		delete mLightPropertiesPanel;
		delete mReferenceEntityPanel;
	}

	void tSigFxMainWindow::fOnClose( wxCloseEvent& event )
	{
		if( mMatEd )
			mMatEd->fSave( );
		if( mMeshSelector )
			mMeshSelector->fSave( );

		PostQuitMessage( 0 );
		event.Skip( );
	}

	void tSigFxMainWindow::fSetupRendering( )
	{
		mRenderPanelContainer->fSetupRendering( fGuiApp( ) );

		for( tWxRenderPanel** ipanel = mRenderPanelContainer->fRenderPanelsBegin( ); ipanel != mRenderPanelContainer->fRenderPanelsEnd( ); ++ipanel )
		{
			if( !*ipanel || !(*ipanel)->fIsVisible( ) )
				( *ipanel )->fGetCamera( )->fDisablePerspectiveChanges( );
		}

		if( mMatEd )
		{
			mMatEd->fSetupPreviewWindow( fGuiApp( ).fGfxDevice( ) );
			fGuiApp( ).fEditableObjects( ).fSetTextureCache( mMatEd->fTextureCache( ) );
		}

		fGuiApp( ).fEditableObjects( ).fReset(
			fGuiApp( ).fGfxDevice( ),
			mRenderPanelContainer->fGetSolidColorMaterial( ), 
			mRenderPanelContainer->fGetSolidColorGeometryAllocator( ),
			mRenderPanelContainer->fGetSolidColorIndexAllocator( ) );

		mManipPanel->fSetSelectionCursor( );

		mToolsPanelContainer->fGetContainerPanel( )->Show( );
		mTabControl->Show( );
		mTimeline->Show( );
		fAddToolbar( );
	}

	void tSigFxMainWindow::fImmediateRefresh( b32 forcekeyframeupdate )
	{
		mKeyline->fImmediateUpdate( forcekeyframeupdate );
	}

	void tSigFxMainWindow::fOnTick( )
	{
		if( mSyncNextFrame )
			fDoActualSync( );

		if( mCheckCommandArgs )
		{
			// determine command line options
			const std::string& cmdLineBuffer = fGuiApp().fGetCmdLine( );
			const tCmdLineOption openFile( "open", cmdLineBuffer );
			
			if( openFile.fFound( ) )
			{
				const tFilePathPtr previewPath = tFilePathPtr( openFile.fGetTypedOption<std::string>( ) );
				fOpenDoc( previewPath );
			}

			const tCmdLineOption openSaveFile( "opensave", cmdLineBuffer );

			if( openSaveFile.fFound() )
			{
				const tFilePathPtr previewPath = tFilePathPtr( openSaveFile.fGetTypedOption<std::string>() );
				fOpenDoc( previewPath );
				fGuiApp( ).fActionStack( ).fForceSetDirty( );
				fSaveDoc( false );
				fGuiApp( ).fQuitAsync( );
			}

			mCheckCommandArgs = false;
		}

		const b32 onTop = fBeginOnTick( );

		if( onTop )
		{
			if( !mTimer.fRunning( ) )
				mTimer.fStart( );

			f32 elapsed = mTimer.fGetElapsedS( );
			const f32 frameDt = fClamp( elapsed - mLastElapsed, 0.f, 1.f/10.f );	// largest 'dt' we can have is 1/10th of a second.
			f32 dt = frameDt;
			mLastElapsed = elapsed;

			//const Gfx::tScreenPtr& screen = tFxSystem::fGetScreen( );
			//const f32 dt = screen->fCapturing( ) ? ( 1.f / 30.f ) : fGuiApp( ).fGetFrameDeltaTime( ) * mFrequency;
			if( mIdealFrameDt > 0.f )
				dt = mIdealFrameDt;

			if( mMatEd ) // needs to happen before the render panels are ticked
				mMatEd->fOnTick( );
			if( mMeshSelector )
				mMeshSelector->fOnTick( );

			// step render panels
			mRenderPanelContainer->fOnTick( );

			// The Cursor assumes that it's being called once per frame.
			// It registers the same draw call twice if you don't call container->Render() the same frame.
			if( !fGuiApp( ).fCurrentCursor( ).fNull( ) )
				fGuiApp( ).fCurrentCursor( )->fOnTick( );

			if( !fDialogInputActive( ) )
				fGuiApp( ).fHotKeys( ).fUpdateHotKeys( Input::tKeyboard::fInstance( ) );

			//Not sure if these 3 updates work/look better inside the idealFrameDt scope below. Causes more jittery but might help show the frame-rate better to the user.
			mTabControl->fOnTickSelectedTab( frameDt );
			mTimeline->fOnTick( frameDt );
			mToolsPanelContainer->fOnTick( );

			mTimeSinceLastIdealUpdate += frameDt;
			if( mTimeSinceLastIdealUpdate >= mIdealFrameDt )
			{
				mTimeSinceLastIdealUpdate = 0.f;

				// step the scene graph
				fGuiApp( ).fSceneGraph( )->fAdvanceTime( dt );
				fGuiApp( ).fSceneGraph( )->fKickCoRenderMTRunList( );
				// render all views
			}

			// See above note next to CurrentCursor ticking.
			mRenderPanelContainer->fRender( 0 );

			// save window layout
			fSaveLayout( );

			std::stringstream objectStats;
			//objectStats << "Objects: selected = " << fGuiApp( ).fSelectionList( ).fCount( ) << 
			//				" | hidden = " << fGuiApp( ).fEditableObjects( ).fGetHiddenCount( ) << 
			//				" | frozen = " << fGuiApp( ).fEditableObjects( ).fGetFrozenCount( ) << 
			//				" | total = " << fGuiApp( ).fEditableObjects( ).fGetObjectCount( );

			if( mFxScene )
			{
				if( mBackgroundLevelEntity && !mBackgroundLevelEntity->fSceneGraph( ) )
					mBackgroundLevelEntity->fSpawnImmediate( fGuiApp( ).fSceneGraph( )->fRootEntity( ) );

				u32 selectedParticleCount = 0;
				for( u32 i = 0; i < fGuiApp( ).fSelectionList( ).fCount( ); ++i )
				{
					tEntityPtr entity = fGuiApp( ).fSelectionList( )[ i ];
					tEditableObject* master = entity->fDynamicCast< tEditableObject >( );
					if( !master )		continue;

					tSigFxParticleSystem* system = master->fDynamicCast< tSigFxParticleSystem >( );
					if( system )
						selectedParticleCount += system->fGetParticleSystem( )->fParticleCount( );
				}						

				objectStats <<	"particle systems = " << mFxScene->fParticleSystemCount( );
				objectStats <<	" | attractors = " << mFxScene->fAttractorCount( );
				objectStats <<	" | mesh systems = " << mFxScene->fMeshSystemCount( );
				objectStats <<	" | total particles = " << mFxScene->fParticleCount( );
				objectStats <<	" | total meshes = " << mFxScene->fMeshCount( );
				objectStats <<	" | selected system particle count = " << selectedParticleCount;
			}
			SetStatusText( objectStats.str( ).c_str( ), 3 );
		}
		else
		{
			if( mTimer.fRunning( ) )
				mTimer.fStop( );
			fSleep( 1 );
		}

		if( mCaptureFrame )
		{
			mCaptureFrame = false;
			const Gfx::tScreenPtr& screen = fRenderPanelContainer( )->fGetFocusRenderPanel( )->fGetScreen( );
			if( screen->fCapturing( ) )
				screen->fEndCaptureDump( );
		}
	}

	void tSigFxMainWindow::fNewDoc( )
	{
		fGuiApp( ).fClearScene( );

		if( mFxmlPropertiesPanel )
			mFxmlPropertiesPanel->fSetRandomStartTime( false );

		fSync( );

		fSetStatus( "New scene" );
	}
	

	b32 tSigFxMainWindow::fImportFxmlFile( const tFilePathPtr& filePath )
	{
		mProgressBar.fReset( new tEffectsFileSaveLoadProgressDialog( mRenderPanelContainer, "Importing File...", true ) );

		Fxml::tFile fxmlFile;
		if( !fxmlFile.fLoadXml( filePath ) )
			return false;

		tEditableObjectContainer::tOnObjectSerialized cb = 
			make_delegate_memfn( tEditableObjectContainer::tOnObjectSerialized, tSigFxMainWindow, fOnObjectSerialized );

		tEditableObjectContainer& edObjs = fGuiApp( ).fEditableObjects( );

		tGrowableArray< tSigFxParticleSystem* > previousParticleSystems;
		edObjs.fCollectByType< tSigFxParticleSystem >( previousParticleSystems );

		tGrowableArray< tSigFxAttractor* > previousAttractors;
		edObjs.fCollectByType< tSigFxAttractor >( previousAttractors );

		tGrowableArray< tSigFxMeshSystem* > previousMeshSystems;
		edObjs.fCollectByType< tSigFxMeshSystem >( previousMeshSystems);


		//mKeyline->fSetCurrentTime( 0.f );
		//mTimeline->fSetLifetime( fxmlFile.mLifetime );
		const b32 success = edObjs.fDeserialize( fxmlFile.mObjects, &cb );


		tGrowableArray< tSigFxParticleSystem* > allParticleSystems;
		edObjs.fCollectByType< tSigFxParticleSystem >( allParticleSystems );

		tGrowableArray< tSigFxAttractor* > allAttractors;
		edObjs.fCollectByType< tSigFxAttractor >( allAttractors );

		tGrowableArray< tSigFxMeshSystem* > allMeshSystems;
		edObjs.fCollectByType< tSigFxMeshSystem >( allMeshSystems );
		
		for( u32 j = previousAttractors.fCount( ); j < allAttractors.fCount( ); ++j )
		{
			for( u32 p = 0; p < previousParticleSystems.fCount( ); ++p )
			{
				previousParticleSystems[ p ]->fAddAttractorIgnoreId( allAttractors[ j ]->fGetAttractor( )->fId( ) );
			}
			for( u32 m = 0; m < previousMeshSystems.fCount( ); ++m )
			{
				previousMeshSystems[ m ]->fFxMeshSystem( )->fFxMeshSystemData( )->fAddAttractorIgnoreId( allAttractors[ j ]->fGetAttractor( )->fId( ) );
			}
		}
		
		for( u32 j = 0; j < previousAttractors.fCount( ); ++j )
		{
			for( u32 p = previousParticleSystems.fCount( ); p < allParticleSystems.fCount( ); ++p )
			{
				allParticleSystems[ p ]->fAddAttractorIgnoreId( previousAttractors[ j ]->fGetAttractor( )->fId( ) );
			}
			for( u32 m = previousMeshSystems.fCount( ); m < allMeshSystems.fCount( ); ++m )
			{
				allMeshSystems[ m ]->fFxMeshSystem( )->fFxMeshSystemData( )->fAddAttractorIgnoreId( previousAttractors[ j ]->fGetAttractor( )->fId( ) );
			}
		}


		mProgressBar.fRelease( );

		if( edObjs.fGetObjectCount( ) < fxmlFile.mObjects.fCount( ) )
		{
			wxMessageBox( "Not all objects in the current file could be loaded. "
				"This means your file contains invalid object types, which were skipped.  "
				"This can happen when you try to open a file that was created in another editor (e.g., Maya).", "Incomplete file", wxOK | wxICON_WARNING );
		}

		SetFocus( );

		if( success )
			fSync( );

		return success;
	}

	b32 tSigFxMainWindow::fDeserializeFxmlFile( const tFilePathPtr& filePath )
	{
		mProgressBar.fReset( new tEffectsFileSaveLoadProgressDialog( mRenderPanelContainer, "Loading File...", true ) );

		Fxml::tFile fxmlFile;
		if( !fxmlFile.fLoadXml( filePath ) )
			return false;

		tEditableObjectContainer::tOnObjectSerialized cb = 
			make_delegate_memfn( tEditableObjectContainer::tOnObjectSerialized, tSigFxMainWindow, fOnObjectSerialized );

		tEditableObjectContainer& edObjs = fGuiApp( ).fEditableObjects( );

		// Set properties in the properties thing.
		mFxmlPropertiesPanel->fSetRandomStartTime( fxmlFile.mFlags & tEffectRefEntityDef::cStartupFlagRandomTime );

		mKeyline->fSetCurrentTime( 0.f );
		mTimeline->fSetLifetime( fxmlFile.mLifetime );
		const b32 success = edObjs.fDeserialize( fxmlFile.mObjects, &cb );

		mProgressBar.fRelease( );

		if( edObjs.fGetObjectCount( ) < fxmlFile.mObjects.fCount( ) )
		{
			wxMessageBox( "Not all objects in the current file could be loaded."
				"This means your file contains invalid object types, which were skipped."
				"Either the file version is very out-of-date, or somebody manually edited the file...", "Incomplete file", wxOK | wxICON_WARNING );
		}

		SetFocus( );

		if( success )
			fSync( );

		return success;
	}


	void tSigFxMainWindow::fImport( )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mRenderPanelContainer, 
			"Import Scene",
			mLastImportDirectory,
			wxString( "untitled.fxml" ),
			wxString( "*.fxml" ),
			wxFD_OPEN ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			tFilePathPtr file( openFileDialog->GetPath( ).c_str( ) );
			if( FileSystem::fFileExists( file ) )
			{
				mLastImportDirectory = wxString( openFileDialog->GetDirectory( ) );
				if( !fImportFxmlFile( file ) )
				{
					wxMessageBox( "The specified .fxml file is corrupt or out of date; importing failed.", "Invalid File", wxOK | wxICON_WARNING );
					SetFocus( );
					fSetStatus( "Importing fx file failed" );
					return;
				}
			}
		}
	}


	b32 tSigFxMainWindow::fSaveDoc( b32 saveAs )
	{
		if( mSavingInProgress )
			return false;
		mSavingInProgress = true;

		if( saveAs || fGuiApp().fDocName( ) == tToolsGuiApp::cNewDocTitle )
		{
			const std::string ext = fEditableFileExt( );

			// browse for a new path
			tStrongPtr<wxFileDialog> saveFileDialog( new wxFileDialog( 
				fRenderPanelContainer( ), 
				"Save Scene As",
				mLastDirectory,
				wxString( "untitled" + ext ),
				wxString( "*" + ext ),
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

			SetFocus( );

			if( saveFileDialog->ShowModal( ) != wxID_OK )
			{
				mSavingInProgress = false;
				return false; // cancelled
			}

			mLastDirectory = wxString( saveFileDialog->GetDirectory( ) );
			fGuiApp( ).fSetDocName( saveFileDialog->GetPath( ).c_str( ) );
			fGuiApp( ).fAddRecentFile( tFilePathPtr( fGuiApp( ).fDocName( ).c_str( ) ) );

			fUpdateRecentFileMenu( );
		}
		else
		{
			// not doing a save as; if we're not dirty, then skip
			if( !fGuiApp( ).fActionStack( ).fIsDirty( ) )
			{
				mSavingInProgress = false;
				return true;
			}
		}

		fSerializeDoc( tFilePathPtr( fGuiApp( ).fDocName( ).c_str( ) ) );

		

		fGuiApp( ).fActionStack( ).fClearDirty( );
		SetTitle( fGuiApp( ).fMakeWindowTitle( ) );

		fSetStatus( "Document saved successfully" );
		mSavingInProgress = false;
		return true;
	}

	void tSigFxMainWindow::fOpenDoc( const tFilePathPtr& file )
	{
		if( !fGuiApp( ).fClearScene( ) )
			return; // user cancelled, don't try to open new file

		mPreviewingCurrentDoc = false;

		if( FileSystem::fFileExists( file ) )
		{
			if( !fDeserializeFxmlFile( file ) )
			{
				wxMessageBox( "The specified .fxml file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
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

		//	fFrameAll( );
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
			SetFocus( );
		}
	}

	void tSigFxMainWindow::fOpenDoc( )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mRenderPanelContainer, 
			"Open Scene",
			mLastDirectory,
			wxString( "untitled.fxml" ),
			wxString( "*.fxml" ),
			wxFD_OPEN ) );

		SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			fGuiApp( ).fClearScene( );
			fOpenDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
			mLastDirectory = wxString( openFileDialog->GetDirectory( ) );
		}
	}

	void tSigFxMainWindow::fOpenRecent( u32 ithRecentFile )
	{
		const Win32Util::tRecentlyOpenedFileList& recentFiles = fGuiApp( ).fRecentFiles( );
		fOpenDoc( recentFiles[ ithRecentFile ] );
	}

	void tSigFxMainWindow::fCaptureTemplateFrame( )
	{
		mCaptureFrame = true;
		const Gfx::tScreenPtr& screen = fRenderPanelContainer( )->fGetFocusRenderPanel( )->fGetScreen( );
		tFilePathPtr saveLocation = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectRootFolder( ), tFilePathPtr( "Capture\\SigFx\\Screens" ) );
		screen->fBeginCaptureDump( saveLocation );
	}

	void tSigFxMainWindow::fStartRecording( )
	{
		const Gfx::tScreenPtr& screen = fRenderPanelContainer( )->fGetFocusRenderPanel( )->fGetScreen( );
		if( screen->fCapturing( ) )
		{
			screen->fEndCaptureDump( );

			wxToolBar* mainToolBar = mRenderPanelContainer->fGetToolBar( );
			mainToolBar->SetToolTip( "Begin Recording" );
			mainToolBar->SetToolNormalBitmap( cActionRecord, wxBitmap( "record" ) );
		}
		else
		{
			tFilePathPtr saveLocation = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectRootFolder( ), tFilePathPtr( "Capture\\SigFx\\Video" ) );
			screen->fBeginCaptureDump( saveLocation );

			wxToolBar* mainToolBar = mRenderPanelContainer->fGetToolBar( );
			mainToolBar->SetToolTip( "End Recording" );
			mainToolBar->SetToolNormalBitmap( cActionRecord, wxBitmap( "stoprecord" ) );
		}
	}


	b32 tSigFxMainWindow::fBuild( )
	{
		if( fGuiApp( ).fDocName( ) == tSigFxMainWindow::cNewDocTitle )
		{
			fGuiApp( ).fSaveDoc( true );
		}
		else if( fGuiApp( ).fActionStack( ).fIsDirty( ) )
		{
			fGuiApp( ).fSaveDoc( false );
		}
		
		if( mPreviewingCurrentDoc )
		{
			mPreviewSaveFile = ToolsPaths::fMakeResAbsolute( tFilePathPtr( "Effects\\editor_preview\\preview_fx.fxml" ) );
			fSerializeDoc( mPreviewSaveFile );
			tAssetGenScanner::fProcessSingleFile( mPreviewSaveFile, false );
		}

		tAssetGenScanner::fProcessSingleFile( tFilePathPtr( fGuiApp( ).fDocName( ) ), true );
		return true;
	}


	void tSigFxMainWindow::fPreview( u32 id )
	{
		mPreviewingCurrentDoc = true;

		if( !fBuild( ) ) return;

		const tFilePathPtr relSigbPath = Sigml::fSigmlPathToSigb( ToolsPaths::fMakeResRelative( tFilePathPtr( "Effects\\editor_preview\\fx_preview_level.sigml" ) ) );
		if( FileSystem::fFileExists( relSigbPath ) )
		{
			log_line( 0, "Previewing current document" );
			std::string cmdLine;
			switch( id )
			{
			case cActionPreviewInternal : cmdLine += "-internal ";	break;
			case cActionPreviewPlaytest : cmdLine += "-playtest ";	break;
			case cActionPreviewRelease : cmdLine += "-release ";	break;
			}

			cmdLine += std::string( "-preview " ) + relSigbPath.fCStr( );
			ToolsPaths::fLaunchGame( cPlatformXbox360, cmdLine );
		}
		else
		{
			log_line( 0, "Error running AssetGen on current document" );
		}
	}


	void fLaunchSigEd()
	{
		const char* sigEdName =
#ifdef _DEBUG
			"\\SigEdd.exe";
#else
			"\\SigEd.exe";
#endif

		const std::string sigEdPath = std::string( ToolsPaths::fGetEngineBinFolder( ).fCStr( ) ) + sigEdName;

		// wxWidgets expects the first element of the command line be the executable path for some reason.
		std::stringstream cmdLine;
		cmdLine << "\"" << sigEdPath;// << "\" " << commandLine.c_str( );

		Threads::tProcess::fSpawnAndForget( sigEdPath.c_str( ), cmdLine.str( ).c_str( ) );
	}

	void tSigFxMainWindow::fOnAction( wxCommandEvent& event )
	{
		const s32 id = event.GetId( );

		if( id >= cActionOpenRecent && id < cActionOpenRecent + tToolsGuiApp::cMaxRecentlyOpenedFiles )
		{
			// open recent action
			fOpenRecent( id - cActionOpenRecent );
		}
		else if( id >= cActionImportFromTemplate && id < cActionImportFromTemplate + tSigFxMainWindow::cMaxTemplateFiles )
		{
			// open from the template list
			u32 fileIdx = id - cActionImportFromTemplate;
			wxString fileName = wxString( mLinearTemplatePaths[ fileIdx ].fCStr( ) );
			//fOpenDoc(tFilePathPtr( fileName.c_str( ) ) );
			fImportFxmlFile( tFilePathPtr( fileName.c_str( ) ) );
		}
		else
		{
			switch( id )
			{
				// file menu
			case cActionNew:				fNewDoc( ); break;
			case cActionOpen:				fOpenDoc( ); break;
			case cActionSave:				fSaveDoc( false ); break;
			case cActionSaveAs:				fSaveDoc( true ); break;
			case cActionImport:				fImport( );	break;
			case cActionBuild:				fBuild( ); break;
			case cActionRecord:				fStartRecording( ); break;
			case cActionCaptureFrame:		fCaptureTemplateFrame( );	break;
			case cActionPreviewInternal:	fPreview( id );	break;
			case cActionPreviewPlaytest:	fPreview( id );	break;
			case cActionPreviewRelease:		fPreview( id );	break;
				
			case cActionQuit:				Close( false ); break;
				// edit menu
			case cActionUndo:				fUndo( ); break;
			case cActionRedo:				fRedo( ); break;

			case cActionToggleMatEd:		if( mMatEd ) mMatEd->fToggle( ); break;
			case cActionToggleMeshSelector:	if( mMeshSelector ) mMeshSelector->fToggle( ); break;


				// window menu
			case cActionFrameSel:			fFrameSelection( ); break;
			case cActionFrameAll:			fFrameAll( ); break;
				/*
			case cActionObjectProperties:
				fToggleObjectProperties( );
				break;
			case cActionGlobalProperties:
				fToggleGlobalProperties( );
				break;
			case cActionRemapReferences:
				{
					if( mRemapReferencesDialog->fIsMinimized( ) )
						mRemapReferencesDialog->fRestoreFromMinimized( );
					else
						mRemapReferencesDialog->Show( !mRemapReferencesDialog->IsShown( ) );
				}
				break;
			*/
				// help menu
			case cActionAbout:
				wxMessageBox( "SigFx allows you to create particle-based effects.", "About SigFx", wxOK | wxICON_INFORMATION );
				break;
			case cActionHotKeyHelp:
				wxMessageBox( "\n---- Universal SigFx HotKeys ----\n\n"

							  "Space - Pause/Resume playback\n"
							  "Enter - Stop playback and track back to t=0\n"
							  "C - Translate Selected Object(s) to (0, 0, 0)\n"
							  "F - Frame View to Selected Object(s)\n"							  
							  "G - View Graphline\n"
							  "K - View Keyline\n"							  
							  "Ctrl+Z - Undo\n"
							  "Ctrl+Y - Redo\n"
							  "\n\n---- In The Graph Window...----\n\n"
							  "Right Click - Context Menu\n"
							  "Middle Click + Drag - Scroll Range\n"
							  "Wheel Scroll - Increase/Decrease Range", "SigFx Hotkey Help", wxOK | wxICON_INFORMATION );
				break;
			case cActionHotKeyLaunchSigEd:
				fLaunchSigEd();
				break;
			default: { log_warning( "Unrecognized action!" ); }
				break;
			}
		}
	}

	void tSigFxMainWindow::fAddToolPanel( )
	{
		tWxToolsPanel* panel = new tWxToolsPanel( mToolsPanelContainer, 320, wxColour( 145, 138, 127 ), wxColour( 0xff, 0xff, 0xff ) );
		
		mManipPanel = new tManipulateEffectPanel( panel, mKeyline );
		mCreateNewEffectPanel = new tCreateNewEffectPanel( panel, fGuiApp( ).fEditableObjects( ), *this );
		mFxmlPropertiesPanel = new tFxmlPropertiesPanel( panel, this );
		mParticleSystemPropertiesPanel = new tParticleSystemPropertiesPanel( panel, fGuiApp( ).fEditableObjects( ), this );
		mAttractorPropertiesPanel = new tAttractorPropertiesPanel( this, panel );
		mMeshSystemPropertiesPanel = new tMeshSystemPropertiesPanel( this, fGuiApp( ).fEditableObjects( ), panel );
		mLightPropertiesPanel = new tLightPropertiesPanel( this, panel );
		mReferenceEntityPanel = new tReferenceEntityPanel( this, panel );

		// collapse the panels from the start, then when an effect is selected
		// they will uncollapse!
		// Do not collapse Fxml properties.
		mParticleSystemPropertiesPanel->fSetCollapsed( true );
		mAttractorPropertiesPanel->fSetCollapsed( true );
		mMeshSystemPropertiesPanel->fSetCollapsed( true );
		mLightPropertiesPanel->fSetCollapsed( true );

		mToolsPanelContainer->fAfterAllToolsPanelsAdded( );
	}

	void tSigFxMainWindow::fAddMenus( )
	{
		wxMenuBar *menuBar = new wxMenuBar;

		// add file menu
		{
			wxMenu *subMenu = new wxMenu;
			menuBar->Append( subMenu, "&File" );

			subMenu->Append( cActionNew, "&New\tCtrl+N" );
			subMenu->Append( cActionOpen, "&Open...\tCtrl+O" );
			subMenu->Append( cActionSave, "&Save\tCtrl+S" );
			subMenu->Append( cActionSaveAs, "Save &As..." );

			subMenu->AppendSeparator( );

			mRecentFilesMenu = new wxMenu;
			mBaseRecentFileActionId = cActionOpenRecent;
			subMenu->AppendSubMenu( mRecentFilesMenu, "Recen&t Files" );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionBuild, "&Build\tCtrl+Shift+B" );
			subMenu->AppendSeparator( );
			subMenu->Append( cActionPreviewInternal, "Preview Internal XBOX" );
			subMenu->Append( cActionPreviewPlaytest, "Preview Playtest XBOX" );
			subMenu->Append( cActionPreviewRelease, "Preview Release XBOX" );
			subMenu->AppendSeparator( );

			fUpdateRecentFileMenu( );

			subMenu->AppendSeparator( );
			subMenu->Append( cActionQuit, "E&xit" );
		}

		// add edit menu
		{
			wxMenu *subMenu = new wxMenu;
			menuBar->Append( subMenu, "&Edit" );

			subMenu->Append( cActionUndo, "&Undo\tCtrl+Z" );
			subMenu->Append( cActionRedo, "&Redo\tCtrl+Y" );
		}

		// add import menu
		{
			wxMenu *subMenu = new wxMenu( );
			menuBar->Append( subMenu, "&Import" );

			wxMenu *templateMenu = new wxMenu( );
			tTemplateFileFolder baseTemplateFolder( tFilePathPtr( mTemplateFxFilePath.c_str( ) ), templateMenu );
			u32 actionIdCnt = 0;
			fBuildImportTemplateMenu( baseTemplateFolder, true, actionIdCnt );

			wxMenu *importMenu = new wxMenu( ); 
			subMenu->AppendSubMenu( templateMenu, "From Project" );
			subMenu->Append( cActionImport, "From File" );
		}

		/*
		// add "window" menu; might change this later
		{
			wxMenu *subMenu = new wxMenu;
			menuBar->Append( subMenu, "&Window" );

			subMenu->Append( cActionFrameSel, "&Frame Selected\tF" );
			subMenu->Append( cActionFrameAll, "Frame &All\tA" );
			subMenu->AppendSeparator();
			subMenu->Append( cActionObjectProperties, "&Object Properties\tCtrl+Shift+O" );
			subMenu->Append( cActionGlobalProperties, "&Global Properties\tCtrl+Shift+G" );
			subMenu->Append( cActionRemapReferences, "&Remap References" );
		}
		*/

		// add help menu
		{
			wxMenu *menuHelp = new wxMenu;
			menuBar->Append( menuHelp, "&Help" );
			menuHelp->Append( cActionAbout, "&About..." );
			menuHelp->Append( cActionHotKeyHelp, "&Hotkeys..." );			
		}

		SetMenuBar( menuBar );
	}

	void tSigFxMainWindow::fBuildImportTemplateMenu( tTemplateFileFolder& curTemplateFolder, b32 root, u32 &menuIdxCount )
	{
		//first build up all the list of files
		FileSystem::fGetFileNamesInFolder( curTemplateFolder.mFileList, curTemplateFolder.mFolderPath );

		wxMenu* menu = root ? curTemplateFolder.mParentMenu : new wxMenu( );
		u32 removals = 0;
		for( u32 i = 0; i < curTemplateFolder.mFileList.fCount( ); ++i )
		{
			std::string str( curTemplateFolder.mFileList[ i ].fCStr( ) );
			int spot = str.find( ".fxml" );
			if( spot <= 0 )
			{
				++removals;
				continue;
			}
			u32 fxmlcnt = i - removals;
			wxMenuItem* item = menu->Append( menuIdxCount + cActionImportFromTemplate + fxmlcnt, wxString( curTemplateFolder.mFileList[ i ].fCStr( ) ) );
			Connect( menuIdxCount + cActionImportFromTemplate + fxmlcnt, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tSigFxMainWindow::fOnAction ), 0, this );
			mLinearTemplatePaths.fPushBack( tFilePathPtr( wxString( curTemplateFolder.mFolderPath.fCStr( ) ) + "\\" + wxString( curTemplateFolder.mFileList[ i ].fCStr( ) ) ) );
		}
		menuIdxCount += curTemplateFolder.mFileList.fCount( ) - removals;
		
		if( !root )
		{
			wxString shortenedDir( curTemplateFolder.mFolderPath.fCStr( ) );
			s32 pos = shortenedDir.Last( '\\' );
			if( pos > 0 )
				shortenedDir = shortenedDir.Remove( 0, pos+1 );

			wxMenuItem* item = curTemplateFolder.mParentMenu->AppendSubMenu( menu, shortenedDir );
			curTemplateFolder.mParentMenu->Remove( item );
			curTemplateFolder.mParentMenu->Insert( 0, item );
		}

		curTemplateFolder.mParentMenu = menu;

		// Now do the folder structure
		FileSystem::fGetFolderNamesInFolder( curTemplateFolder.mFolderList, curTemplateFolder.mFolderPath );

		for( s32 i = curTemplateFolder.mFolderList.fCount( )-1; i >= 0; --i )
		{
			const tFilePathPtr currentPath( ( wxString( curTemplateFolder.mFolderPath.fCStr( ) ) + "\\" + wxString( curTemplateFolder.mFolderList[ i ].fCStr( ) ) ).c_str( ) );
			tTemplateFileFolder newFolder( currentPath, curTemplateFolder.mParentMenu );
			fBuildImportTemplateMenu( newFolder, false, menuIdxCount );
			mTemplateFoldersList.fPushBack( newFolder );
		}

		if( curTemplateFolder.mFolderList.fCount( ) > 0 && curTemplateFolder.mFileList.fCount( ) )
			curTemplateFolder.mParentMenu->InsertSeparator( curTemplateFolder.mFolderList.fCount( ) );

		if( !root && ( !curTemplateFolder.mFileList.fCount( ) && !curTemplateFolder.mFolderList.fCount( ) ) )
		{
			wxMenuItem* item = menu->Append( -1, wxString( "Empty" ) );
			item->Enable( false );
		}
	}

	void tSigFxMainWindow::fAddToolbar( )
	{
		// setup primary/default toolbar
		wxToolBar* mainToolBar = mRenderPanelContainer->fGetToolBar( );
		mainToolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
		mainToolBar->AddTool( cActionNew, "New Scene", wxBitmap( "newdoc" ), wxNullBitmap, wxITEM_NORMAL, "Create a new, empty scene" );
		mainToolBar->AddTool( cActionOpen, "Open Scene", wxBitmap( "opendoc" ), wxNullBitmap, wxITEM_NORMAL, "Load an existing scene" );
		mainToolBar->AddTool( cActionSave, "Save Scene", wxBitmap( "savedoc" ), wxNullBitmap, wxITEM_NORMAL, "Save current scene" );
		
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionUndo, "Undo", wxBitmap( "undo" ), wxNullBitmap, wxITEM_NORMAL, "Undo last action" );
		mainToolBar->AddTool( cActionRedo, "Redo", wxBitmap( "redo" ), wxNullBitmap, wxITEM_NORMAL, "Redo previous action" );
		
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionBuild, "Build", wxBitmap( "build" ), wxNullBitmap, wxITEM_NORMAL, "Build the current file" );
		mainToolBar->AddTool( cActionRecord, "Record", wxBitmap( "record" ), wxNullBitmap, wxITEM_NORMAL, "Begin Recording" );
		mainToolBar->AddTool( cActionCaptureFrame, "CaptureFrame", wxBitmap( "record" ), wxNullBitmap, wxITEM_NORMAL, "Capture Frame" );


		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionFrameSel, "FrameSel", wxBitmap( "framesel" ), wxNullBitmap, wxITEM_NORMAL, "Frame Selected" );
		mainToolBar->AddTool( cActionFrameAll, "FrameAll", wxBitmap( "frameall" ), wxNullBitmap, wxITEM_NORMAL, "Frame All" );
		
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionToggleMatEd, "MaterialEditor", wxBitmap( "objectproperties" ), wxNullBitmap, wxITEM_NORMAL, "Material Editor" );

		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionToggleMeshSelector, "MeshSelector", wxBitmap( "objectproperties" ), wxNullBitmap, wxITEM_NORMAL, "Mesh Selector" );


		wxStaticText* txt = new wxStaticText( mainToolBar, wxID_ANY, "Update Frequency: " );
		mPlaybackTimeFrequency = new wxChoice( mainToolBar, wxID_ANY, wxDefaultPosition, wxSize( 128, wxDefaultSize.y ) );
		txt->SetBackgroundColour( mainToolBar->GetBackgroundColour( ) );

		mainToolBar->AddSeparator( );

		mainToolBar->AddControl( txt );
		mainToolBar->AddControl( mPlaybackTimeFrequency );
		
		mPlaybackTimeFrequency->AppendString( "Frame Time" );
		mPlaybackTimeFrequency->AppendString( "Fixed 30fps" );
		mPlaybackTimeFrequency->AppendString( "Fixed 60fps" );

		mPlaybackTimeFrequency->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tSigFxMainWindow::fOnPlaybackTimeChanged ), NULL, this );
		mPlaybackTimeFrequency->Select( 1 );

		mainToolBar->AddSeparator( );

		wxCheckBox* showLevelBckgrnd = new wxCheckBox( mainToolBar, wxID_ANY, "Show Level In Background  " );
		showLevelBckgrnd->SetBackgroundColour( mainToolBar->GetBackgroundColour( ) );
		showLevelBckgrnd->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( tSigFxMainWindow::fOnShowLevelInBackgroundClicked ), NULL, this );
		mainToolBar->AddControl( showLevelBckgrnd );

		const Math::tVec4f rgbaClearColor = mRenderPanelContainer->fGetFocusRenderPanel( )->fGetScreen( )->fCreateOpts( ).mDefaultClearColor;
		wxColourPickerCtrl* mBackgroundColourPicker = new wxColourPickerCtrl( mainToolBar, wxID_ANY, wxColour( rgbaClearColor.x*255, rgbaClearColor.y*255, rgbaClearColor.z*255 ) );
		mBackgroundColourPicker->Connect( wxEVT_COMMAND_COLOURPICKER_CHANGED, wxColourPickerEventHandler( tSigFxMainWindow::fOnBackgroundColourChanged ), NULL, this );
		mBackgroundColourPicker->SetBackgroundColour( mainToolBar->GetBackgroundColour( ) );

		mainToolBar->AddControl( mBackgroundColourPicker );

		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionHotKeyHelp, "Hot Keys", wxBitmap( "help" ), wxNullBitmap, wxITEM_NORMAL, "Get Hot Key Help" );
		mainToolBar->AddSeparator( );
		mainToolBar->AddTool( cActionHotKeyLaunchSigEd, "SigEd", wxBitmap( "SigEdAppIcon" ), wxNullBitmap, wxITEM_NORMAL, "Launch SigEd" );

		mainToolBar->Realize( );
	}

	void tSigFxMainWindow::fOnPlaybackTimeChanged( wxCommandEvent& event )
	{	
		f32 frequencies[ ] = { -1.f, 1.f/30.f, 1.f/60.f };
		mIdealFrameDt = frequencies[ mPlaybackTimeFrequency->GetSelection( ) ];
	}

	void tSigFxMainWindow::fOnShowLevelInBackgroundClicked( wxCommandEvent& event )
	{
		if( event.IsChecked( ) )
		{
			tResourceId id = tResourceId::fMake< tSceneGraphFile >( tFilePathPtr( "Effects\\editor_preview\\fx_preview_level.sigb" ) );
			tResourcePtr resource = fGuiApp( ).fEditableObjects( ).fGetResourceDepot( )->fQuery( id );
			mBackgroundLevelEntity.fReset( new tEditableSgFileRefEntity( fGuiApp( ).fEditableObjects( ), id, resource ) );
		}
		else
		{
			if( mBackgroundLevelEntity )
			{
				mBackgroundLevelEntity->fDeleteImmediate( );
				mBackgroundLevelEntity.fRelease( );
			}
		}
	}

	void tSigFxMainWindow::fOnBackgroundColourChanged( wxColourPickerEvent& event )
	{
		wxColour c = event.GetColour( );

		f32 r = ( f32 )c.Red( ) / 255.f;
		f32 g = ( f32 )c.Green( ) / 255.f;
		f32 b = ( f32 )c.Blue( ) / 255.f;

		mRenderPanelContainer->fGetFocusRenderPanel( )->fGetScreen( )->fSetRgbaClearColor( Math::tVec4f( r, g, b, 1.f ) );
	}

	std::string tSigFxMainWindow::fEditableFileExt( ) const
	{
		return Fxml::fGetFileExtension( );
	}

	void tSigFxMainWindow::fOnObjectSerialized( u32 nObjectsSerialized, u32 nObjectsTotal )
	{
		sigassert( !mProgressBar.fNull( ) );
		mProgressBar->fUpdate( nObjectsSerialized, nObjectsTotal );
	}

	b32 tSigFxMainWindow::fSerializeDoc( const tFilePathPtr& path )
	{
		mProgressBar.fReset( new tEffectsFileSaveLoadProgressDialog( mRenderPanelContainer, "Saving File", false ) );

		Fxml::tFile fxmlFile;

		tEditableObjectContainer::tOnObjectSerialized cb = 
			make_delegate_memfn( tEditableObjectContainer::tOnObjectSerialized, tSigFxMainWindow, fOnObjectSerialized );
		
		fGuiApp( ).fEditableObjects( ).fSerialize( fxmlFile.mObjects, &cb );

		mProgressBar->tWxProgressDialog::fUpdate( 1.f, "Writing file to disk (this may stall for a sec)..." );

		fxmlFile.mLifetime = mTimeline->fLifetime( );
		//fxmlFile.mEditableProperties = mGlobalProperties->fGlobalProperties( );
		//mLayerPanel->fSaveLayers( sigmlFile );

		// Save out any property flags. These should probably just be Editable Props for flexibility. BUT ALAS!
		if( mFxmlPropertiesPanel->fRandomStartTime() )
			fxmlFile.mFlags |= tEffectRefEntityDef::cStartupFlagRandomTime;

		b32 success = fxmlFile.fSaveXml( path, true );

		mProgressBar.fRelease( );
		SetFocus( );

		return success;
	}



}

