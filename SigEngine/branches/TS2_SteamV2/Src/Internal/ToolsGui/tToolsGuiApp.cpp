#include "ToolsGuiPch.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tAssetPluginDll.hpp"
#include "Input/tKeyboard.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tPhongMaterial.hpp"
#include "Editor/tEditablePropertyColor.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tSceneGraphFile.hpp"
#include "Gfx/tRenderContext.hpp"
#include "tGlobalLightDirGizmoGeometry.hpp"

namespace Sig
{
	class tToolsGuiWxApp : public wxApp
	{
	public:
		virtual bool OnInit( ) { return true; }
		virtual int  OnExit( ) { return 0; }
	};
}

IMPLEMENT_APP_NO_MAIN( ::Sig::tToolsGuiWxApp )

namespace Sig
{
	const char tToolsGuiApp::cNewDocTitle[] = "(untitled)";

	tToolsGuiApp::tToolsGuiApp( const char* appName, HINSTANCE hinst, tNewMainWindow newMainWindow )
		: mAppName( appName )
		, mDocName( cNewDocTitle )
		, mRegKeyName( ToolsPaths::fGetSignalRegistryKeyName( ) + "\\" + appName )
		, mHinst( hinst )
		, mNewMainWindow( newMainWindow )
		, mMainWindow( 0 )
		, mRecentFiles( fRegKeyName( ) + "\\" + ToolsPaths::fGetCurrentProjectName( ) + "\\RecentFiles" )
		, mEditableObjects( 0 )
		, mSaveInProgress( false )
	{
		mOptions.mResourceLoadingTimeoutMS = 10.0f;

		// load up saved recent files
		mRecentFiles.fLoad( );

		mEditableObjects = new tEditableObjectContainer( fSceneGraph( ), fResourceDepot( ) );

		const f32 yBoundsOffset = 550.f;// TODOHACK need to provide way in editor of positioning primary split plane on Y-axis
		fSceneGraph( )->fSetSpatialBounds( Math::tAabbf(
			Math::tVec3f( -1000.f, -1000.f + yBoundsOffset, -1000.f ), 
			Math::tVec3f( +1000.f, +1000.f + yBoundsOffset, +1000.f ) ) );

		// create default world light
		{
			Gfx::tLight light;
			light.fSetTypeDirection( );
			light.fColor( Gfx::tLight::cColorTypeAmbient ) = Math::tVec4f( 0.2f, 0.2f, 0.2f, 1.f );
			light.fColor( Gfx::tLight::cColorTypeFront ) = Math::tVec4f( 1.25f, 1.25f, 1.25f, 1.0f );
			light.fColor( Gfx::tLight::cColorTypeRim ) = Math::tVec4f( 0.0f, 0.0f, 0.0f, 1.0f );
			light.fColor( Gfx::tLight::cColorTypeBack ) = Math::tVec4f( 0.3f, 0.3f, 0.6f, 1.0f );
			Math::tMat3f lightMatrix = Math::tMat3f::cIdentity;
			lightMatrix.fOrientZAxis( -Math::tVec3f( 0.7f, 1.0f, 0.6f ).fNormalize( ), Math::tVec3f::cYAxis );
			mDefaultLight.fReset( new Gfx::tLightEntity( lightMatrix, light, "PrimaryDirectional" ) );
			mDefaultLight->fSetCastsShadow( true );
			mDefaultLight->fSpawnImmediate( fSceneGraph( )->fRootEntity( ) );
		}

		// load asset plugins right-off the bat
		tAssetPluginDllDepot::fInstance( ).fLoadPluginsBasedOnCurrentProjectFile( );

		// intialize wxWidget-based UI
		int argc=0; char* argv[] = {""};
		if( wxEntryStart( argc, argv ) )
			wxSetInstance( GetModuleHandle( 0 ) );
	}

	tToolsGuiApp::~tToolsGuiApp( )
	{
		delete mEditableObjects;
		mEditableObjects = 0;

		delete mMainWindow;
		mMainWindow = 0;

		mSceneGraph->fClear( );
		mSceneGraph.fRelease( );

		// shutdown wxWidget-based UI
		if( wxTheApp )
			wxEntryCleanup( );
	}

	void tToolsGuiApp::fConfigureApp( tStartupOptions& optsOut, tPlatformStartupOptions& platformOptsOut )
	{
		optsOut.mGameName = mAppName;
		optsOut.mUpdateDelay = 0.f;
		optsOut.mFrameTimeDeltaClamp = 1.f / 10.f;
		optsOut.mAutoDetectGameRoot = false;
		optsOut.mGameRootOverride = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformPcDx9 );

		platformOptsOut.mCreateWindowOverride.fFromMethod<tToolsGuiApp, &tToolsGuiApp::fCreateMainWindow>( this );
	}

	void tToolsGuiApp::fTickApp( )
	{

		f32 dt = 1.0f;
		Input::tKeyboard::fInstance( ).fCaptureState( dt );

		sigassert( mMainWindow );
		mMainWindow->fOnTick( );
	}

	void tToolsGuiApp::fStartupApp( )
	{
		log_line( 0, "Startup " << mAppName << "!" );

		// create device
		mGfxDevice.fReset( new Gfx::tDevice( tApplication::fGetWindowHandleGeneric( ) ) );
		Gfx::tDevice::fSetDefaultDevice( mGfxDevice );

		const b32 castShadow = mDefaultLight->fCastsShadow( );
		mDefaultLight->fSetupShadowCasting( mGfxDevice, Gfx::tScreenCreationOptions( ).mShadowMapResolution );
		mDefaultLight->fSetCastsShadow( castShadow );

		sigassert( mMainWindow );
		mMainWindow->fSetupRendering( );

		// create global light dir gizmo
		tGizmoGeometryPtr geom(
			new tGlobalLightDirGizmoGeometry( 
			mGfxDevice, 
			mMainWindow->fRenderPanelContainer( )->fGetSolidColorMaterial( ), 
			mMainWindow->fRenderPanelContainer( )->fGetSolidColorGeometryAllocator( ), 
			mMainWindow->fRenderPanelContainer( )->fGetSolidColorIndexAllocator( ) ) );

		mGlobalLightDirGizmo.fReset( new tGlobalLightDirGizmo( geom ) );
		mGlobalLightDirGizmo->fSetActive( false );
	}

	void tToolsGuiApp::fShutdownApp( )
	{
		log_line( 0, "Shutdown " << mAppName << "!" );
		Gfx::tDevice::fSetDefaultDevice( Gfx::tDevicePtr( ) );
	}

	HWND tToolsGuiApp::fCreateMainWindow( )
	{
		if( wxTheApp )
		{
			::wxInitAllImageHandlers( );

			sigassert( mNewMainWindow );
			mMainWindow = (*mNewMainWindow)( *this );
			sigassert( mMainWindow );

			wxGetApp( ).SetTopWindow( mMainWindow );
			return ( HWND )mMainWindow->GetHWND( );
		}

		return 0;
	}

	void tToolsGuiApp::fAddRecentFile( const tFilePathPtr& path )
	{
		mRecentFiles.fAdd( path );
		mRecentFiles.fSave( );
	}

	void tToolsGuiApp::fSetCurrentCursor( const tEditorCursorControllerPtr& newCursor )
	{
		if( mCurrentCursor )
			mCurrentCursor->fOnNextCursor( newCursor.fGetRawPtr( ) );
		mPrevCursor = mCurrentCursor;
		mCurrentCursor = newCursor;
	}

	void tToolsGuiApp::fTickRenderGizmos( )
	{
		if( !mGlobalLightDirGizmo.fNull( ) )
			mGlobalLightDirGizmo->fTick( *mMainWindow );

		for( u32 i = 0; i < mRenderOnlyGizmos.fCount( ); ++i )
			mRenderOnlyGizmos[ i ]->fTick( *mMainWindow );
	}

	void tToolsGuiApp::fDisableFog( )
	{
		Gfx::fGetDefaultFog( mSavedFogValues, mSavedFogColor );
		Gfx::fSetDefaultFog( Math::tVec4f( 0.f, 0.f, 0.f, 0.f ), Math::tVec3f( 0.f, 0.f, 0.f ) );
	}

	void tToolsGuiApp::fEnableFog( )
	{
		Gfx::fSetDefaultFog( mSavedFogValues, mSavedFogColor );
	}

	std::string tToolsGuiApp::fMakeWindowTitle( ) const
	{
		return fAppName( ) + " ~ " + mDocName + ( fActionStack( ).fIsDirty( ) ? "*" : "" );
	}

	b32 tToolsGuiApp::fClearScene( b32 closing )
	{
		if( fActionStack( ).fIsDirty( ) )
		{
			const int result = wxMessageBox( "You have unsaved changes - would you like to save them before resetting?",
						  "Save Changes?", wxYES | wxNO | wxCANCEL | wxICON_WARNING );
			fMainWindow( ).SetFocus( );

			if(			result == wxYES )			{ if( !fSaveDoc( false ) ) return false; }
			else if(	result == wxNO )			{ }
			else if(	result == wxCANCEL )		{ return false; }
			else									{ log_warning( 0, "Unknown result returned from Message Box" ); }
		}

		fActionStack( ).fClearDirty( );
		fActionStack( ).fReset( );
		fEditableObjects( ).fRemoveAllFromWorld( );
		fSelectionList( ).fClear( );
		mDocName = cNewDocTitle;

		fMainWindow( ).fClearScene( closing );

		return true;
	}
	
	b32 tToolsGuiApp::fSaveDoc( b32 saveAs )
	{
		if( mSaveInProgress )
			return false;
		mSaveInProgress = true;

		if( saveAs || mDocName == cNewDocTitle )
		{
			const std::string ext = fMainWindow( ).fEditableFileExt( );

			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				fMainWindow( ).fRenderPanelContainer( ), 
				"Save Scene As",
				wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
				wxString( "untitled" + ext ),
				wxString( "*" + ext ),
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

			fMainWindow( ).SetFocus( );

			if( openFileDialog->ShowModal( ) != wxID_OK )
			{
				mSaveInProgress = false;
				return false; // cancelled
			}

			mDocName = openFileDialog->GetPath( );
			fAddRecentFile( tFilePathPtr( mDocName.c_str( ) ) );

			fMainWindow( ).fUpdateRecentFileMenu( );
		}
		else
		{
			// not doing a save as; if we're not dirty, then skip
			if( !fActionStack( ).fIsDirty( ) )
			{
				mSaveInProgress = false;
				return true;
			}
		}

		if( !fMainWindow( ).fSerializeDoc( tFilePathPtr( mDocName.c_str( ) ) ) )
		{
			mSaveInProgress = false;
			return false;
		}

		fActionStack( ).fClearDirty( );
		fMainWindow( ).SetTitle( fMakeWindowTitle( ) );

		fMainWindow( ).fSetStatus( "Document saved successfully" );

		mSaveInProgress = false;
		return true;
	}

	void tToolsGuiApp::fSyncDefaultLight( tEditablePropertyTable& properties )
	{
		mDefaultLight->fSetOn( properties.fGetValue( Sigml::tFile::fEditablePropDefaultApplyInEditorName( ), true ) != 0 );

		tSceneGraphDefaultLight lightDesc;
		Sigml::tFile::fConvertToDefaultLightDesc( properties, lightDesc );

		mDefaultLight->fSetCastsShadow( lightDesc.mCastShadow );
		
		mDefaultLight->fColor( Gfx::tLight::cColorTypeFront ) = Math::tVec4f( lightDesc.mFrontColor, 1.f );
		mDefaultLight->fColor( Gfx::tLight::cColorTypeBack ) = Math::tVec4f( lightDesc.mBackColor, 1.f );
		mDefaultLight->fColor( Gfx::tLight::cColorTypeRim ) = Math::tVec4f( lightDesc.mRimColor, 1.f );
		mDefaultLight->fColor( Gfx::tLight::cColorTypeAmbient ) = Math::tVec4f( lightDesc.mAmbientColor, 1.f );

		const Math::tVec3f lightDir = Math::tVec3f( lightDesc.mDirection ).fNormalizeSafe( );
		mDefaultLight->fUpdateDefaultLightDirection( lightDir );

		if( !mGlobalLightDirGizmo.fNull( ) )
		{
			mGlobalLightDirGizmo->fSetActive( properties.fGetValue( Sigml::tFile::fEditablePropShowGlobalLightDirection( ), true ) );
			mGlobalLightDirGizmo->fSetDirection( lightDir );
		}
	}

	tEditorSelectionList&				tToolsGuiApp::fSelectionList( ) { return fEditableObjects( ).fGetSelectionList( ); }
	tEditorActionStack&					tToolsGuiApp::fActionStack( ) { return fEditableObjects( ).fGetActionStack( ); }
	const tEditorActionStack&			tToolsGuiApp::fActionStack( ) const { return fEditableObjects( ).fGetActionStack( ); }

}

