//------------------------------------------------------------------------------
// \file tSigAnimMainWindow.cpp - 18 Jul 2008
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "SigAnimPch.hpp"
#include "tSigAnimMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxToolsPanel.hpp"
#include "tWxToolsPanelContainer.hpp"
#include "tSigAnimEdDialog.hpp"
#include "tFxFileRefEntity.hpp"
#include "FileSystem.hpp"
#include "Anifig.hpp"
#include "WxUtil.hpp"
#include "tSkeletonFile.hpp"
#include "tAnimPackFile.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tSklmlBrowser.hpp"
#include "tAssetGenScanner.hpp"
#include "Input\tMouse.hpp"
#include "tSigAnimMainWindowContextActions.hpp"

// Entity
#include "Animation\tSkeletableSgFileRefEntity.hpp"

// tWxToolsPanel
#include "tPlaceObjectCursor.hpp"
#include "tLoadObjectPanel.hpp"
#include "tManipulateObjectPanel.hpp"
#include "tAnimationTreePanel.hpp"
#include "tObjectListPanel.hpp"
#include "tEntityControlPanel.hpp"
#include "tAnifigPanel.hpp"
#include "tSigAnimTimeline.hpp"
#include "tSigAnimWindowHotKeys.hpp"

// Need to set some event id for keyframe animations:
#include "tKeyFrameAnimTrack.hpp"

#include "Animation\tSigAnimDialog.hpp"

#include "tSearchableOpenFilesDialog.hpp"


namespace Sig
{
	namespace
	{
		static const wxColour cBackColor = wxColour( 0x44, 0x44, 0x55 );
		static const wxColour cTextColor = wxColour( 0xff, 0xff, 0x99 );
		static const wxColour cGroupTitleColour = wxColour( 0xff, 0xff, 0xff );
	}

	///
	/// \class tEditorFrameSelectedHotKey
	/// \brief 
	class tEditorFrameSelectedHotKey : public tEditorHotKey
	{
		tToolsGuiMainWindow & mWindow;

	public:
		tEditorFrameSelectedHotKey( tEditorHotKeyTable & table, tToolsGuiMainWindow & window ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonF, 0 ), mWindow( window ) { }
		virtual void fFire( ) const { mWindow.fFrameSelection( ); }
	};

	enum tAction
	{
		cActionOpen = 1,
		cActionSave,
		cActionCreateAnipk,
		cActionCreateMomap,
		cActionReload,
		cActionAnimEd,
		cActionLoadBackupFile,
		cActionBackupFileSelect = 100,
		cActionBackupFileSelectMax = 105,
	};

	BEGIN_EVENT_TABLE(tSigAnimMainWindow, wxFrame)
		EVT_CLOSE( tSigAnimMainWindow::fOnClose )
		EVT_MENU(				cActionOpen,				tSigAnimMainWindow::fOnAction)
		EVT_MENU(				cActionSave,				tSigAnimMainWindow::fOnAction)
		EVT_MENU(				cActionCreateAnipk,			tSigAnimMainWindow::fOnAction)
		EVT_MENU(				cActionCreateMomap,			tSigAnimMainWindow::fOnAction)
		EVT_MENU(				cActionReload,				tSigAnimMainWindow::fOnAction)
		EVT_MENU(				cActionAnimEd,				tSigAnimMainWindow::fOnAction)
		EVT_MENU(				cActionLoadBackupFile,		tSigAnimMainWindow::fOnAction)
	END_EVENT_TABLE()

	tSigAnimMainWindow::tSigAnimMainWindow( tToolsGuiApp& guiApp )
		: tToolsGuiMainWindow( guiApp )
		, mMainSizer( 0 )
		, mToolsPanelContainer( 0 )
		, mMoMapEditorDialog( new tSigAnimDialog( ) )
		, mOpenDialog( NULL )
	{
		for( u32 i = cActionBackupFileSelect; i <= cActionBackupFileSelectMax; ++i )
		{
			Connect( i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tSigAnimMainWindow::fOnAction ) );
		}

		// set icon
		SetIcon( wxIcon( "appicon" ) );

		// create primary window containers
		mRenderPanelContainer = new tWxRenderPanelContainer( this, guiApp.fRegKeyName( ) + "\\RenderPanels", true );

		// Tools Panel
		fAddTools( );

		// Sizer
		{

			wxPanel* bottompanel = new wxPanel( this );
			wxBoxSizer* bottomsizer = new wxBoxSizer( wxVERTICAL );
			bottompanel->SetSizer( bottomsizer );

			wxBoxSizer* rendercontrolsplit = new wxBoxSizer( wxVERTICAL );
			wxBoxSizer* Keylinesplit = new wxBoxSizer( wxHORIZONTAL );

			rendercontrolsplit->Add( mRenderPanelContainer->fGetContainerPanel( ), 1, wxEXPAND | wxALL, 0 );
			rendercontrolsplit->Add( bottompanel, 0, wxEXPAND | wxALL, 0 );
			Keylinesplit->Add( rendercontrolsplit, 1, wxEXPAND | wxALL, 0 );
			Keylinesplit->Add( mToolsPanelContainer->fGetContainerPanel( ), 0, wxEXPAND | wxALL, 0 ); 
			SetSizer( Keylinesplit );

			mTimeline = new tSigAnimTimeline( bottompanel, mAnimControlPanel );
			mAnimationTreePanel->fSetAnimationTimeline( mTimeline );
			bottomsizer->Add( mTimeline, 0, wxEXPAND | wxALL, 0 );

			//mMainSizer = new wxBoxSizer( wxHORIZONTAL );

			//mMainSizer->Add( mRenderPanelContainer->fGetContainerPanel( ), 1, wxEXPAND | wxALL, 0 );
			//mMainSizer->Add( mToolsPanelContainer->fGetContainerPanel( ), 0, wxEXPAND | wxALL, 0 ); 

			//SetSizer( mMainSizer );
		}


		mOpenDialog = new tSearchableOpenFilesDialog( this, ToolsPaths::fGetCurrentProjectResFolder() );
		tGrowableArray<const char*> additionalFilters;
		additionalFilters.fPushBack( ".anifig" );
		mOpenDialog->fSetAdditionalExtensionFilters( additionalFilters );


		// Add Toolbar Options
		wxToolBar* mainToolBar = mRenderPanelContainer->fGetToolBar();
		if( mainToolBar )
		{
			mainToolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
			mainToolBar->AddTool( cActionOpen, "Open Scene", wxBitmap( "opendoc" ), wxNullBitmap, wxITEM_NORMAL, "Load an anim configuration" );
			mainToolBar->AddTool( cActionLoadBackupFile, "Open Backup Scene", wxBitmap( "opendoc" ), wxNullBitmap, wxITEM_NORMAL, "Load backup anim configuration" );
			mainToolBar->AddTool( cActionSave, "Save Scene", wxBitmap( "savedoc" ), wxNullBitmap, wxITEM_NORMAL, "Save current anim configuration" );
			mainToolBar->AddTool( cActionCreateAnipk, "Create Anipk", wxBitmap("createanipk"), wxNullBitmap, wxITEM_NORMAL, "Create a new anipk" );
			mainToolBar->AddTool( cActionCreateMomap, "Create Momap", wxBitmap("createmomap"), wxNullBitmap, wxITEM_NORMAL, "Create a new momap" );
			mainToolBar->AddTool( cActionReload, "Rebuild/Reload Resources", wxBitmap("reloadres"), wxNullBitmap, wxITEM_NORMAL, "Rebuild/Reload resources" );
			mainToolBar->AddTool( cActionAnimEd, "Animation Editor", wxBitmap("animed"), wxNullBitmap, wxITEM_NORMAL, "Edit animation properties" );

			mRefreshSklmlBrowser = true;
			mSklmlBrowser = new tSklmlBrowser( new wxDialog( this, wxID_ANY, "Select Skeleton...", wxDefaultPosition ) );

			mainToolBar->Realize( );
		}

		// Hot keys
		mFrameCameraHotKey.fReset( new tEditorFrameSelectedHotKey( fGuiApp( ).fHotKeys( ), *this ) );
		mNextFrameHotKey.fReset( new tSigAnimFrameForwardHotKey( fGuiApp( ).fHotKeys( ), this ) );
		mPreviousFrameHotKey.fReset( new tSigAnimFrameBackwardHotKey( fGuiApp( ).fHotKeys( ), this ) );
		mSetAnimEventHotKey.fReset( new tSigAnimAnimEventHotKey( fGuiApp( ).fHotKeys( ), this ) );


		fLoadLayout( );
		SetFocus( );

		mOnObjectAdded.fFromMethod<tSigAnimMainWindow, &tSigAnimMainWindow::fOnObjectAdded>( this );
		guiApp.fEditableObjects( ).fGetObjectAddedEvent( ).fAddObserver( &mOnObjectAdded );

		mOnSelectionChanged.fFromMethod<tSigAnimMainWindow, &tSigAnimMainWindow::fOnSelectionChanged>( this );
		guiApp.fEditableObjects( ).fGetSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelectionChanged );
		

		Anim::tKeyFrameAnimTrack::fSetKeyFrameEventID( 0 );

		tWin32Window::fMessagePump( );

		mOpenDocPath = tFilePathPtr( "untitled" );

		mTimeSinceRender = 0;
		mLockThirtyFPS = false;

		fContextActions().fPushBack( tEditorContextActionPtr( new tOpenSelectedObjectContextMenu( this ) ) );
		fContextActions().fPushBack( tEditorContextActionPtr( new tAnimOptionsContextMenu( this ) ) );
	}

	tSigAnimMainWindow::~tSigAnimMainWindow( )
	{
	}

	void tSigAnimMainWindow::fSetupRendering( )
	{
		mRenderPanelContainer->fSetupRendering( fGuiApp( ) );

		fGuiApp( ).fEditableObjects( ).fReset(
			fGuiApp( ).fGfxDevice( ),
			mRenderPanelContainer->fGetSolidColorMaterial( ), 
			mRenderPanelContainer->fGetSolidColorGeometryAllocator( ),
			mRenderPanelContainer->fGetSolidColorIndexAllocator( ) );

		tAnimPackDataAgent::fInstance( ).fLoad( );

		mCurrentStackText.fReset( new Gui::tText( ) );
		mCurrentStackText->fSetDevFont( );
		mCurrentStackText->fSetRgbaTint( Math::tVec4f( 1.f, 0.3f, 0.3f, 1.f ) );
	}

	void tSigAnimMainWindow::fOnTick( )
	{
		if( mRefreshSklmlBrowser )
		{
			mRefreshSklmlBrowser = false;
			mSklmlBrowser->fRefresh( );
		}

		fProcessAnimPackWaitList( );

		const b32 onTop = fBeginOnTick( );

		if( onTop )
		{
			f32 dt = fGuiApp( ).fGetFrameDeltaTime( );

			// Keep track of time since last render
			// Use total time since last render as dt if mLockThirtyFPS is set
			mTimeSinceRender += dt;
			dt = mLockThirtyFPS ? mTimeSinceRender : dt;

			if( ( !mLockThirtyFPS ) || ( mLockThirtyFPS && mTimeSinceRender >= 0.033 ) )
			{
				mAnimationTreePanel->fHandleDialogs( );

				fRenderSkeletons( );
				fRenderStackText( );
				fUpdateTimeline( );

				// step render panels
				mRenderPanelContainer->fOnTick( );

				// step cursor
				if( !fGuiApp( ).fCurrentCursor( ).fNull( ) )
					fGuiApp( ).fCurrentCursor( )->fOnTick( );

				// step the timeline
				mTimeline->fOnTick( dt );

				// step tools panels
				mToolsPanelContainer->fOnTick( );

				// step hot keys
				if( !fDialogInputActive( ) )
					fGuiApp( ).fHotKeys( ).fUpdateHotKeys( Input::tKeyboard::fInstance( ) );

				// step the scene graph
				fGuiApp( ).fSceneGraph( )->fAdvanceTime( dt );
				fGuiApp( ).fSceneGraph( )->fKickCoRenderMTRunList( );

				// render all views
				mRenderPanelContainer->fRender( 0 );
				mTimeSinceRender = 0;
			}

			// save window layout
			fSaveLayout( );
		}
		else
		{
			fSleep( 1 );
		}

	}

	b32 tSigAnimMainWindow::fSerializeDoc( const tFilePathPtr& path )
	{
		return fSerializeAnifigFile( path );
	}

	void tSigAnimMainWindow::fOpenDoc( const tFilePathPtr& file )
	{
		if( FileSystem::fFileExists( file ) )
		{
			if( !fDeserializeSceneFile( file ) )
			{
				wxMessageBox( "The specified scene file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
				SetFocus( );
				fSetStatus( "Open failed" );
				return;
			}

			// Success
			fSetStatus( "Opened successfully" );

			mOpenDocPath = file;
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
			SetFocus( );
		}
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fAddTools( )
	{
		mToolsPanelContainer = new tWxToolsPanelContainer( *this );
		tWxToolsPanel * panel = new tWxToolsPanel( mToolsPanelContainer, 325, cBackColor );

		mManipTools = new tManipulateObjectPanel( panel );
		mAnimationTreePanel = new tAnimationTreePanel( panel );
		mAnimControlPanel = new tEntityControlPanel( panel );
		mAnifigPanel = new tAnifigPanel( panel, *this );
		mLoadObjectPanel = new tLoadObjectPanel( panel );
		new tObjectListPanel( panel );

		mToolsPanelContainer->fAfterAllToolsPanelsAdded( );
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fProcessAnimPackWaitList( )
	{
		// Nothing to do
		if( !mWaitingForAnimPacks.fCount( ) )
			return;

		// See if there's only one object if none are selected
		tEntity * oneObject = 0;
		if( !fGuiApp( ).fEditableObjects( ).fGetSelectionList( ).fCount( ) )
		{
			tGrowableArray<tSkeletableSgFileRefEntity *> ents;
			fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( 
				ents, tSkeletableSgFileRefEntity::fIsCursor );

			if( ents.fCount( ) )
				oneObject = ents[ 0 ];
		}

		for( s32 e = mWaitingForAnimPacks.fCount( ) - 1; e >= 0; --e )
		{
			tSkeletableSgFileRefEntity * entity = 
				( tSkeletableSgFileRefEntity * )mWaitingForAnimPacks[ e ].fGetRawPtr( );

			// Still waiting for the base resource
			if( !entity->fSkeletonResource( ) )
				continue;

			// Get the pack info
			tAnimPackInfo & packInfo = entity->fAnimPackInfo( );

			// Begin processing new skeletons
			const u32 toProcessCount = packInfo.mSkeletonsToProcess.fCount( );
			for( u32 s = 0; s < toProcessCount; ++s )
			{
				tResourcePtr skelRes = packInfo.mSkeletonsToProcess[ s ];
				
				fAddResource( skelRes );
				packInfo.mSkeletonsProcessing.fPushBack( skelRes );
			}

			packInfo.mSkeletonsToProcess.fSetCount( 0 );

			// Process any available and loaded skeletons
			for( s32 s = packInfo.mSkeletonsProcessing.fCount( ) - 1; s >= 0; --s )
			{
				tResourcePtr skelRes = packInfo.mSkeletonsProcessing[ s ];

				// Still loading
				if( skelRes->fLoading( ) )
					continue;

				// Successfull load so get the anim packs
				if( skelRes->fLoaded( ) )
				{
					tGrowableArray< tResourcePtr > animPacks;
					tAnimPackDataAgent::fInstance( ).fGetAnimPacksForSkeleton( skelRes->fGetPath( ), animPacks );

					for( u32 a = 0; a < animPacks.fCount( ); ++a )
						entity->fAddAnimPack( animPacks[ a ] );
				}

				fRemoveResource( skelRes );
				packInfo.mSkeletonsProcessing.fErase( s );
			}

			// Are we done processing
			if( !packInfo.mSkeletonsToProcess.fCount( ) && !packInfo.mSkeletonsProcessing.fCount( ) )
			{
				packInfo.mSkeletonsProcessed.fDeleteArray( );
				packInfo.mSkeletonsProcessing.fDeleteArray( );
				packInfo.mSkeletonsToProcess.fDeleteArray( );

				mWaitingForAnimPacks.fErase( e );
			}

			if( entity->fGetSelected( ) || entity == oneObject )
				mAnimationTreePanel->fMarkForRefresh( );
		}
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fAddResource( const tResourcePtr & res )
	{
		u32 * loadCount = mResources.fFind( res );
		if( !loadCount )
		{
			res->fLoadDefault( this );
			loadCount = mResources.fInsert( res, 0 );
		}

		++( *loadCount );
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fRemoveResource( const tResourcePtr & res )
	{
		u32 * loadCount = mResources.fFind( res );
		sigassert( loadCount && *loadCount );

		if ( --( *loadCount ) )
			return;

		mResources.fRemove( loadCount );
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fOnObjectAdded( 
		tEditableObjectContainer & container, const tEntityPtr & entityPtr )
	{
		tSkeletableSgFileRefEntity * entity = entityPtr->fDynamicCast<tSkeletableSgFileRefEntity>( );

		// Not our type
		if( !entity )
			return;

		// Don't mess with the cursor object
		if( entity->fIsCursor( ) )
			return;

		// Valid, but only add it once
		mWaitingForAnimPacks.fFindOrAdd( entityPtr );
	}


	void tSigAnimMainWindow::fOnSelectionChanged( tEditorSelectionList& list )
	{
		tSkeletableSgFileRefEntity* skelEnt = NULL;

		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			skelEnt = dynamic_cast<tSkeletableSgFileRefEntity*>( list[ i ].fGetRawPtr( ) );
			if( skelEnt )
				break;
		}

		mMoMapEditorDialog->fSetSkeletonEntity( skelEnt );
	}

	void tSigAnimMainWindow::fOnClose( wxCloseEvent& event )
	{
		mAnimationTreePanel->fWarnAndSaveAnyDirtyAnimPacks();

		// If the backup folder doesn't exist, create it
		wxString backFileName = Win32Util::fGetCurrentApplicationFileName( );
		backFileName.Remove( backFileName.find_last_of( "\\" ) + 1 );
		tFilePathPtr backupFolder( backFileName + "BACKUP" );
		if( !FileSystem::fFolderExists(  backupFolder ) )
		{
			FileSystem::fCreateDirectory( backupFolder );
		}

		// Save backup of current document
		wxString openDoc = StringUtil::fStripExtension( mOpenDocPath.fCStr( ) );
		openDoc.Remove( 0, openDoc.find_last_of( "\\" ) + 1 );
		fSerializeDoc( tFilePathPtr( wxString( backupFolder.fCStr( ) ) + "\\" + Time::tDateTime::fNowLocal( ).fDateTimeString( ) + openDoc + "_backup" + ".anifig" ) );

		// Close
		PostQuitMessage( 0 );
		event.Skip( );
	}

	void tSigAnimMainWindow::fOnAction(wxCommandEvent& event)
	{
		const s32 id = event.GetId( );

		if( id >= cActionBackupFileSelect && id <= cActionBackupFileSelectMax )
		{
			fOpenBackupDoc( mBackupFileList.fCount( ) - 1 - ( id - cActionBackupFileSelect ) );
			return;
		}

		switch( id )
		{
		case cActionOpen: fOpenDoc( ); break;
		case cActionSave: fGuiApp( ).fSaveDoc( true ); break;
		case cActionCreateAnipk: fCreateAnipk( ); break;
		case cActionCreateMomap: fCreateMomap( ); break;
		case cActionReload: fReloadResources( ); break;
		case cActionAnimEd: fToggleAnimEd( ); break;
		case cActionLoadBackupFile: fShowBackupScenes( event ); break;

		default: { log_warning( "Unrecognized action!" ); }
				 break;
		}
	}

	void tSigAnimMainWindow::fToggleAnimEd( )
	{
		mAnimationTreePanel->fToggelAnimEd( );
	}

	void tSigAnimMainWindow::fLockFPS( b32 lock )
	{
		mLockThirtyFPS = lock;
	}

	void tSigAnimMainWindow::fShowBackupScenes( wxCommandEvent& event )
	{
		// Find files in the backup folder
		fUpdateBackupFileList( );

		// Populate the menu
		wxMenu menu;
		for(u32 i = 0; ( i + cActionBackupFileSelect < cActionBackupFileSelectMax ) && ( i < mBackupFileList.fCount( ) ); ++i)
		{
			// Reverse order of the file list as it's sorted alphabetically which puts newer files at the back of the list
			menu.Append( cActionBackupFileSelect + i, mBackupFileList[ mBackupFileList.fCount( ) - i - 1 ].fCStr( ) );
		}

		// Popup menu at cursor location
		POINT cursorPos;
		GetCursorPos( &cursorPos );
		wxPoint curPos( cursorPos.x, cursorPos.y );
		curPos = ScreenToClient( curPos );		
		PopupMenu( &menu, curPos.x, curPos.y );
	};

	void tSigAnimMainWindow::fOpenBackupDoc( u32 index )
	{
		// Construct application path backup folder
		wxString backupFileName = Win32Util::fGetCurrentApplicationFileName( );
		backupFileName.Remove( backupFileName.find_last_of( "\\" ) + 1 );
		backupFileName += "BACKUP";

		// Open file
		fOpenDoc( tFilePathPtr( backupFileName + "\\" + mBackupFileList[ index ].fCStr( ) ) );

		// Extract original filename from backup file
		wxString openFileName( mBackupFileList[ index ].fCStr( ) );
		openFileName = openFileName.Remove( 0, mDateMarkerLength );
		openFileName = openFileName.Remove( openFileName.find_last_of( "_" ) ) + ".anifig";
		mOpenDocPath = tFilePathPtr( openFileName );
	}

	void tSigAnimMainWindow::fUpdateBackupFileList( )
	{
		// Search backup directory for files
		wxString backupFileName = Win32Util::fGetCurrentApplicationFileName( );
		backupFileName.Remove( backupFileName.find_last_of( "\\" ) + 1 );
		backupFileName += "BACKUP";
		mBackupFileList.fDeleteArray( );
		FileSystem::fGetFileNamesInFolder( mBackupFileList, tFilePathPtr( backupFileName ) );
	}

	void tSigAnimMainWindow::fNextFrame( )
	{
		f32 currentTime = mTimeline->fScrub( )->fGetTime( );
		currentTime += mAnimControlPanel->fStepSize( );
		mTimeline->fSetUpdateSkeletonToScrub( );
		mTimeline->fScrub( )->fSetXThruTime( currentTime );
	}

	void tSigAnimMainWindow::fPreviousFrame( )
	{
		f32 currentTime = mTimeline->fScrub( )->fGetTime( );
		currentTime -= mAnimControlPanel->fStepSize( );
		mTimeline->fSetUpdateSkeletonToScrub( );
		mTimeline->fScrub( )->fSetXThruTime( currentTime );
	}

	void tSigAnimMainWindow::fOpenDoc( )
	{
		tFilePathPtr file = mOpenDialog->fGetSelectedFile( );

		if( file.fExists() )
			fOpenDoc( file );
	}

	void tSigAnimMainWindow::fOnRightClick( wxWindow* window, wxMouseEvent& event )
	{
		if( fGuiApp( ).fCurrentCursor( ) && dynamic_cast< tPlaceObjectCursorBase* >( fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) ) )
			mManipTools->fSetSelectionCursor( );
		else
			tToolsGuiMainWindow::fOnRightClick( window, event );
	}

	b32 tSigAnimMainWindow::fSerializeAnifigFile( const tFilePathPtr& filePath )
	{
		Anifig::tFile anifigFile;
		fSerializeAnifigFile( anifigFile );

		b32 success = anifigFile.fSaveXml( filePath, true );

		SetFocus( );

		return success;
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fSerializeAnifigFile( Anifig::tFile & file )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

		const u32 count = ents.fCount( );
		file.mObjects.fSetCapacity( count );
		for( u32 e = 0; e < count; ++e )
		{
			tSkeletableSgFileRefEntity * entity = ents[ e ];

			// Skip cursors
			if( entity->fIsCursor( ) )
				continue;

			Anifig::tObject object;

			// Main path
			object.mSgFile = ToolsPaths::fMakeResRelative(
				tSceneGraphFile::fConvertToSource(entity->fResourcePath( ) ) );

			// Skeleton path
			if( !entity->fSkeletonResIsFromSgFile( ) )
				object.mSkelFile = ToolsPaths::fMakeResRelative(
					tSkeletonFile::fConvertToSource( entity->fSkeletonResourcePath( ) ) );

			// Transform
			object.mWorldXform = entity->fObjectToWorld( );

			file.mObjects.fPushBack( object );
		}
	}

	//------------------------------------------------------------------------------
	b32 tSigAnimMainWindow::fDeserializeSceneFile( const tFilePathPtr& filePath )
	{				
		if(Sig::StringUtil::fCheckExtension(filePath.fCStr(),"anifig"))
		{
			Anifig::tFile anifigFile;
			if( !anifigFile.fLoadXml( filePath ) )
				return false;

			fDeserializeAnifigFile( anifigFile );
		}
		else if(Sig::StringUtil::fCheckExtension(filePath.fCStr(),"sigml"))
		{
			Sigml::tFile sigmlFile;
			if( !sigmlFile.fLoadXml( filePath ))
				return false;

			fDeserializeSigmlFile( sigmlFile );
		}
		else
		{
			// Unrecognized file type
			log_warning( "Unrecognized file type" );
			return false;
		}

		SetFocus( );
		return true;
	}

	void tSigAnimMainWindow::fDeserializeSigmlFile( const Sigml::tFile & file )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

		// Destroy the old entities
		const u32 oldEntCount = ents.fCount( );
		for( u32 e = 0; e < oldEntCount; ++e )
		{
			if( !ents[ e ]->fIsCursor( ) )
			{
				ents[ e ]->fDeleteImmediate( );
				ents[ e ]->fRemoveFromWorld( );
			}
		}

		// Add the new ones
		const u32 newEntCount = file.mObjects.fCount( );
		for( u32 e = 0; e < newEntCount; ++e )
		{
			const Sigml::tObjectPtr obj = file.mObjects[ e ];
			Sigml::tSigmlReferenceObject * sigRefObj = obj->fDynamicCast<Sigml::tSigmlReferenceObject>();

			if( sigRefObj )
			{
				tFilePathPtr fileName = file.mSrcFile;
				tResourceId rid = tResourceId::fMake<tSceneGraphFile>( tSceneGraphFile::fConvertToBinary(sigRefObj->mReferencePath) );
				tResourcePtr sgFile = fGuiApp( ).fResourceDepot( )->fQuery( rid );

				tFilePathPtr skelFilePath = file.fGetSkeletonSourcePath();
				tSkeletableSgFileRefEntity * entity;
				if( !skelFilePath.fLength( ) )
				{
					entity = new tSkeletableSgFileRefEntity( 
						fGuiApp( ).fEditableObjects( ), rid, sgFile );
				}
				else
				{
					tResourcePtr skelFile = fGuiApp( ).fResourceDepot( )->fQuery(
						tResourceId::fMake<tSkeletonFile>(
						tSkeletonFile::fConvertToBinary( skelFilePath ) ) );

					entity = new tSkeletableSgFileRefEntity(
						fGuiApp( ).fEditableObjects( ), rid, sgFile, skelFile );
				}

				entity->fMoveTo( obj->mXform);
				entity->fResetBaseXform( );
				entity->fAddToWorld( );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fDeserializeAnifigFile( const Anifig::tFile & file )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

		// Destroy the old entities
		const u32 oldEntCount = ents.fCount( );
		for( u32 e = 0; e < oldEntCount; ++e )
		{
			if( !ents[ e ]->fIsCursor( ) )
			{
				ents[ e ]->fDeleteImmediate( );
				ents[ e ]->fRemoveFromWorld( );
			}
		}

		// Add the new ones
		const u32 newEntCount = file.mObjects.fCount( );
		for( u32 e = 0; e < newEntCount; ++e )
		{
			const Anifig::tObject & obj = file.mObjects[ e ];
			if( obj.mSgFile.fNull( ) )
				continue;

			tResourceId rid = tResourceId::fMake<tSceneGraphFile>( tSceneGraphFile::fConvertToBinary(obj.mSgFile) );
			tResourcePtr sgFile = fGuiApp( ).fResourceDepot( )->fQuery( rid );

			tSkeletableSgFileRefEntity * entity;
			if( !obj.mSkelFile.fLength( ) )
			{
				entity = new tSkeletableSgFileRefEntity( 
					fGuiApp().fEditableObjects(), rid, sgFile );
			}
			else
			{
				tResourcePtr skelFile = fGuiApp( ).fResourceDepot( )->fQuery(
					tResourceId::fMake<tSkeletonFile>(
						tSkeletonFile::fConvertToBinary( obj.mSkelFile ) ) );

				entity = new tSkeletableSgFileRefEntity(
					fGuiApp( ).fEditableObjects( ), rid, sgFile, skelFile );
			}

			entity->fMoveTo( obj.mWorldXform );
			entity->fResetBaseXform( );
			entity->fAddToWorld( );
		}
	}
	
	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fCreateMomap( )
	{
		mMoMapEditorDialog->Show( );
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fCreateAnipk( )
	{
		if( !mSklmlBrowser->fBrowse( ) )
			return;

		const tFilePathPtr currentResPath = ToolsPaths::fGetCurrentProjectResFolder( );

		std::string path;
		b32 picked = WxUtil::fBrowseForFile( 
			path, 
			this, 
			"Save Animation Pack", 
			currentResPath.fCStr( ),
			0, // defFile
			"*.anipk", 
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

		if( !picked )
			return;

		Anipk::tFile anipkFile;
		anipkFile.mSkeletonRef = ToolsPaths::fMakeResRelative( mSklmlBrowser->fFilePath( ) );

		const tFilePathPtr pathPtr = tFilePathPtr( path );
		if( anipkFile.fSaveXml( pathPtr, true ) )
		{
			tAssetGenScanner::fProcessSingleFile( pathPtr, false );
			mAnimationTreePanel->fMarkForRefresh( );
		}
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fReloadResources( )
	{
		Anifig::tFile anifigFile;
		fSerializeAnifigFile( anifigFile );

		fGuiApp( ).fSetCurrentCursor( tEditorCursorControllerPtr( ) );
		fGuiApp( ).fActionStack( ).fReset( );

		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

		tGrowableArray<tFilePathPtr> paths;

		// Gather files to build
		const u32 entCount = ents.fCount( );
		for( u32 e = 0; e < entCount; ++e )
		{
			tSkeletableSgFileRefEntity * entity = ents[ e ];
			paths.fFindOrAdd( tSceneGraphFile::fConvertToSource( entity->fResourcePath( ) ) );
			paths.fFindOrAdd( tSkeletonFile::fConvertToSource( entity->fSkeletonResourcePath( ) ) );

			const tAnimPackInfo & packInfo = entity->fAnimPackInfo( );
			const u32 packCount = packInfo.mAnimPacks.fCount( );
			for( u32 p = 0; p < packCount; ++p )
			{
				paths.fFindOrAdd( 
					tAnimPackFile::fConvertToSource( packInfo.mAnimPacks[ p ]->fGetPath( ) ) );
			}
			
			// Destroy any objects
			entity->fDeleteImmediate( );
			entity->fRemoveFromWorld( );
		}

		mWaitingForAnimPacks.fDeleteArray( );
		tAnimPackDataAgent::fInstance( ).fRefresh( );

		{
			tHashTable<tResourcePtr, u32>::tIterator itr = mResources.fBegin( );
			tHashTable<tResourcePtr, u32>::tIterator itrEnd = mResources.fEnd( );

			while( itr != itrEnd )
			{
				tHashTable<tResourcePtr, u32>::tIterator curr = itr;
				++itr;

				if( curr->fNullOrRemoved( ) )
					continue;
				
				tResourcePtr res = curr->mKey;
				u32 refCount = curr->mValue;
				for( u32 i = 0; i < refCount; ++i )
					fRemoveResource( res );

				itr = mResources.fBegin( );
				itrEnd = mResources.fEnd( );
			}
		}

		// Rebuild the paths
		const u32 pathCount = paths.fCount( );
		for( u32 p = 0; p < pathCount; ++p )
			tAssetGenScanner::fProcessSingleFile( paths[ p ], false );

		// Recreate the objects
		fDeserializeAnifigFile( anifigFile );

		// TODO: handle animation stack reloading as well

		mAnimationTreePanel->fMarkForRefresh( );
		mAnifigPanel->fMarkForRefresh( );
		mLoadObjectPanel->fMarkForRefresh( );
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fRenderSkeletons( )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fGuiApp( ).fEditableObjects( ).fCollectByType( ents );

		// Go through all skeleton renderables, and add them to each screen
		const u32 entCount = ents.fCount( );
		for( u32 e = 0; e < entCount; ++e )
		{
			tSkeletonVisualizer * skeleton = ents[ e ]->fGetRenderSkeleton( );

			// Not visible so skip
			if( !skeleton )
				continue;

			// Reset device objects if necessary
			if( skeleton->fNeedsDeviceObjects( ) )
			{
				skeleton->fResetDeviceObjects( 
					fGuiApp( ).fGfxDevice( ), 
					mRenderPanelContainer->fGetSolidColorMaterial( ), 
					mRenderPanelContainer->fGetSolidColorGeometryAllocator( ),
					mRenderPanelContainer->fGetSolidColorIndexAllocator( ) );
				skeleton->fSetSkeleton( ents[ e ]->fSkeleton( ) );
			}

			// Add to all the panels
			for( tWxRenderPanel** renderPanel = mRenderPanelContainer->fRenderPanelsBegin( );
				 renderPanel != mRenderPanelContainer->fRenderPanelsEnd( );
				 ++renderPanel )
			{
				if( !*renderPanel || !(*renderPanel)->fIsVisible( ) )
					continue;

				skeleton->fAddToScreen( (*renderPanel)->fGetScreen( ) );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tSigAnimMainWindow::fRenderStackText( )
	{
		tWxRenderPanel* activePanel = fRenderPanelContainer( )->fGetFocusRenderPanel( );
		if( !activePanel )
			return;

		const Gfx::tScreenPtr& screen = activePanel->fGetScreen( );

		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( 
			ents, tSkeletableSgFileRefEntity::fIsCursor );

		u32 lineCount = 0;
		std::stringstream ss;

		if( ents.fCount( ) > 1 )
		{
			lineCount = 1;
			ss << "Stack information only available for one entity at a time" << std::endl;
		}
		else if( ents.fCount( ) == 1 )
		{
			const Anim::tAnimatedSkeletonPtr & skeleton = ents[ 0 ]->fSkeleton( );
			if( !skeleton.fNull( ) )
			{
				lineCount = skeleton->fTrackCount( );
				if( !lineCount )
				{
					lineCount = 1;
					ss << "No tracks";
				}
				else
				{
					skeleton->fAddDebugText( ss );
				}
			}
			else
			{
				lineCount = 1;
				ss << "Skeleton not set or still loading" << std::endl;
			}
		}

		const std::string text = ss.str( );

		f32 textHeight = mCurrentStackText->fBakeBox( 
			2000, 
			text.c_str( ), 
			( u32 )text.length( ), 
			Gui::tText::cAlignLeft );

		float yPos = screen->fCreateOpts( ).mBackBufferHeight - textHeight - 10.f;
		mCurrentStackText->fSetPosition( Math::tVec3f( 10.f, yPos, 0.f ) );

		screen->fAddScreenSpaceDrawCall( mCurrentStackText->fDrawCall( ) );
	}


	void tSigAnimMainWindow::fUpdateTimeline( )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( ents, tSkeletableSgFileRefEntity::fIsCursor );


		f32 currentAnimTime = 0.0f;
		if( ents.fCount( ) >= 1 )
		{
			const Anim::tAnimatedSkeletonPtr & skeleton = ents[ 0 ]->fSkeleton( );
			if( !skeleton.fNull( ) )
			{
				u32 trackCount = skeleton->fTrackCount( );
				if( trackCount > 0 )
				{
					u32 bestTrack = 0;
					f32 longest = 0.0f;
					for( u32 i = 0; i < trackCount; ++i)
					{
						if( skeleton->fTrack( i ).fMaxTime( ) > longest )
						{
							longest = skeleton->fTrack( i ).fMaxTime( );
							bestTrack = i;
						}
					}

					mTimeline->fSetLifetime( skeleton->fTrack( bestTrack ).fMaxTime( ) );
					currentAnimTime = skeleton->fTrack( bestTrack ).fCurrentTime( );

					if( mTimeline->fUpdateSkeletonToScrub( ) )
					{
						f32 idealTime = mTimeline->fLifetime( ) * mTimeline->fScrub( )->fGetX( );
						if( mTimeline->fScrub( )->fGetX( ) >= 1.f )
							idealTime -= 0.001f;		// roll back a thousandth of a sec here...

						skeleton->fTrack( bestTrack ).fSetCurrentTime( idealTime );
						mAnimControlPanel->fSetPaused( true );
					}
				}
			}
		}
		
		mTimeline->fScrub( )->fSetXThruTime( currentAnimTime );
	}



}

