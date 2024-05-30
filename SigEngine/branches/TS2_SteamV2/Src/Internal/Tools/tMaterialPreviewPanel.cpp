#include "ToolsPch.hpp"
#include "tMaterialPreviewPanel.hpp"
#include "tApplication.hpp"

namespace Sig
{
	namespace
	{
		static const wxColour cBgColor = wxColour( 0xaa, 0xaa, 0xaa );
	}

	tMaterialPreviewPanel::tMaterialPreviewPanel( wxWindow* parent, u32 width, u32 height,
		wxButton* playButton,
		wxButton* modeButton,
		wxButton* resetButton,
		wxButton* ambButton,
		wxButton* frontButton,
		wxButton* rimButton,
		wxButton* backButton )
		: wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( width, height ), wxBORDER_NONE )
		, mOgSize( width, height )
		, mSceneGraph( new tSceneGraph( tLogicThreadPoolPtr( new tLogicThreadPool( 1 ) ) ) )
		, mNormalClearColor( cBgColor.Red( )/255.f, cBgColor.Green( )/255.f, cBgColor.Blue( )/255.f )
		, mErrorClearColor( 0x77/255.f, 0x33/255.f, 0x33/255.f )
		, mLastElapsed( 0.f )
		, mYaw( 0.f )
		, mDoYaw( false )
		, mCurrentPreviewGeometry( 0 )
		, mPlayButton( playButton )
		, mModeButton( modeButton )
		, mResetButton( resetButton )
		, mAmbButton( ambButton )
		, mFrontButton( frontButton )
		, mRimButton( rimButton )
		, mBackButton( backButton )
	{
		SetBackgroundColour( cBgColor );
		mSceneGraph->fSetSpatialBounds( 100.f, 0.f, 50.f, 0.f );
	}
	tMaterialPreviewPanel::~tMaterialPreviewPanel( )
	{
	}
	void tMaterialPreviewPanel::fSetup( const Gfx::tDevicePtr& device )
	{
		mTextureCache.fReset( new Dx9Util::tTextureCache( device ) );

		// create screen
		Gfx::tScreenCreationOptions screenCreateOpts;
		screenCreateOpts.mWindowHandle = ( u64 )GetHWND( );
		screenCreateOpts.mFullScreen = false;
		screenCreateOpts.mBackBufferWidth = GetSize( ).x;
		screenCreateOpts.mBackBufferHeight = GetSize( ).y;
		screenCreateOpts.mVsync = false;
		mScreen.fReset( new Gfx::tScreen( device, mSceneGraph, screenCreateOpts ) );

		// create single viewport
		mScreen->fSetViewportCount( 1 );
		mScreen->fViewport( 0 )->fSetCameras( Gfx::tCamera( 
			Gfx::tLens( 0.1f, 1000.f, 1.f, ( f32 )GetSize( ).y / ( f32 )GetSize( ).x ),
			Gfx::tTripod( Math::tVec3f( 0.f, 0.0f, 3.5f ), Math::tVec3f::cZeroVector, Math::tVec3f( 0.f, 1.f, 0.f ) ) ) );

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
			mDefaultLight->fSetCastsShadow( false );
			mDefaultLight->fSpawnImmediate( mSceneGraph->fRootEntity( ) );
		}

		if( mPlayButton )
			mPlayButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonPlay ), NULL, this );
		if( mModeButton )
			mModeButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonMode ), NULL, this );
		if( mResetButton )
			mResetButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonReset ), NULL, this );
		if( mAmbButton )
			mAmbButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonAmb ), NULL, this );
		if( mFrontButton )
			mFrontButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonFront ), NULL, this );
		if( mRimButton )
			mRimButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonRim ), NULL, this );
		if( mBackButton )
			mBackButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMaterialPreviewPanel::fOnButtonBack ), NULL, this );
	}
	void tMaterialPreviewPanel::fSetPreviewBundle( const tMaterialPreviewBundlePtr& bundle )
	{
		mPreviewBundle = bundle;
	}
	void tMaterialPreviewPanel::fRender( )
	{
		if( !mScreen )
			return;

		const f32 elapsed = mTimer.fGetElapsedS( );
		const f32 dt = elapsed - mLastElapsed;
		mLastElapsed = elapsed;

		if( int( elapsed - dt ) < int( elapsed ) )
			mTextureCache->fRefresh( ); // refresh once per second

		if( mDoYaw )
			mYaw += 0.5f * dt;

		if( mPreviewBundle && mTextureCache )
		{
			mPreviewBundle->fUpdateMaterial( mMaterialFile, *mTextureCache );
			mPreviewBundle->fSetYaw( mYaw );
			mPreviewBundle->fSetCurrentPreviewGeometry( mCurrentPreviewGeometry );
			mPreviewBundle->fAddToSceneGraph( *mSceneGraph );
		}

		mSceneGraph->fAdvanceTime( dt );
		mSceneGraph->fKickCoRenderMTRunList( );

		const Math::tVec3f& clearColor = true ? mNormalClearColor : mErrorClearColor;
		mScreen->fSetRgbaClearColor( clearColor.x, clearColor.y, clearColor.z );
		mScreen->fRender( );

		if( mPreviewBundle )
			mPreviewBundle->fRemoveFromSceneGraph( );
	}
	void tMaterialPreviewPanel::fSetTimeRunning( b32 run )
	{
		if( run && !mTimer.fRunning( ) )
		{
			SetSize( mOgSize );
			mTimer.fStart( );
		}
		else if( !run && mTimer.fRunning( ) )
		{
			SetSize( 1, 1 );
			mTimer.fStop( );
		}
	}
	void tMaterialPreviewPanel::fClear( )
	{
		mPreviewBundle.fRelease( );
		mMaterialFile = Derml::tMtlFile( );
	}
	void tMaterialPreviewPanel::fFromDermlMtlFile( const Derml::tMtlFile& mtlFile )
	{
		mMaterialFile = mtlFile;
	}
	void tMaterialPreviewPanel::fOnButtonPlay( wxCommandEvent& )
	{
		mDoYaw = !mDoYaw;
	}
	void tMaterialPreviewPanel::fOnButtonMode( wxCommandEvent& )
	{
		mCurrentPreviewGeometry = ( mCurrentPreviewGeometry + 1 ) % 32;
	}
	void tMaterialPreviewPanel::fOnButtonReset( wxCommandEvent& )
	{
		mYaw = 0.f;
	}
	void tMaterialPreviewPanel::fOnButtonAmb( wxCommandEvent& )
	{
	}
	void tMaterialPreviewPanel::fOnButtonFront( wxCommandEvent& )
	{
	}
	void tMaterialPreviewPanel::fOnButtonRim( wxCommandEvent& )
	{
	}
	void tMaterialPreviewPanel::fOnButtonBack( wxCommandEvent& )
	{
	}
}
