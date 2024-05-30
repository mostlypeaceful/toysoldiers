#include "GameAppPch.hpp"
#include "tWaveLaunchArrowUI.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{
	devvar( f32, Gameplay_Gui_WaveLaunchArrow_BounceMag, 25.0f );
	devvar( f32, Gameplay_Gui_WaveLaunchArrow_BounceFreq, 0.5f );
	devvar( f32, Gameplay_Gui_WaveLaunchArrow_ScreenBorder, 30.0f );
	devvar( f32, Gameplay_Gui_WaveLaunchArrow_Offset, 0.f );
	devvar( f32, Gameplay_Gui_WaveLaunchArrow_FadeIn, 0.5f );
	devvar( f32, Gameplay_Gui_WaveLaunchArrow_FadeOut, 1.0f );

	using namespace Sig::Math;

	namespace { static const tStringPtr cCanvasCreateWaveLaunchArrowUI( "CanvasCreateWaveLaunchArrowUI" ); }

	tWaveLaunchArrowUI::tWaveLaunchArrowUI( const tResourcePtr& scriptResource, const tUserPtr& user, f32 duration )
		: mControl( scriptResource )
		, mUser( user )
		, mBounceTime( 0.0f )
		, mLifeLeft( 0.0f )
		, mCurrentAngle( 0.f )
		, mAngleBlend( 1.f )
		, mDuration( duration )
	{

		sigassert( mUser );
		sigassert( scriptResource->fLoaded( ) );
		mControl.fCreateControlFromScript( cCanvasCreateWaveLaunchArrowUI, this );
		sigassert( !mControl.fCanvas( ).fIsNull( ) );

		tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ).fAddChild( mControl.fCanvas( ) );
		fShow( );

		if( mUser->fIsViewportVirtual( ) )
			mControl.fCanvas( ).fCanvas( )->fSetInvisible( true );
	}

	void tWaveLaunchArrowUI::fOnSpawn( )
	{
		fOnPause( false );
	}
	
	void tWaveLaunchArrowUI::fOnDelete( )
	{
		fDestroy( );
		tEntity::fOnDelete( );
	}

	void tWaveLaunchArrowUI::fOnPause( b32 pause )
	{
		if( pause )
			fRunListRemove( cRunListWorldSpaceUIST );
		else
			fRunListInsert( cRunListWorldSpaceUIST );
	}

	void tWaveLaunchArrowUI::fWorldSpaceUIST( f32 dt )
	{
		if( mLifeLeft > 0.0f )
		{
			tVec3f worldPoint = fObjectToWorld( ).fGetTranslation( );

			mBounceTime += dt;

			tRect rect = mUser->fComputeViewportSafeRect( );
			rect.fInflate( -Gameplay_Gui_WaveLaunchArrow_ScreenBorder );

			tVec3f screenPt3;
			b32 onScreen = mUser->fProjectToScreenClamped( worldPoint, 0.f, screenPt3 );
			tVec2f screenPos = screenPt3.fXY( );
			tVec2f uiPoint( screenPos.x, screenPos.y );

			onScreen = onScreen && rect.fContains( uiPoint );
			if( !onScreen )
				uiPoint = rect.fClampToEdge( uiPoint );

			tVec2f direction;
			f32 targetBlend = 1.f; //hard blending off screen
			
			if( onScreen )
			{
				mAngleBlend = 0.2f; //soft blending on screen
				targetBlend = mAngleBlend;
				direction = tVec2f( 0, 1 );
			}
			else
				direction = uiPoint - rect.fCenter( );

			// compute new angle and rectify
			f32 newAngle = fAtan2( direction.y, direction.x ) - cPiOver2;
			f32 shortestDelta = fShortestWayAround( mCurrentAngle, newAngle );
			mCurrentAngle = newAngle - shortestDelta;

			// control blending
			mAngleBlend = fLerp( mAngleBlend, targetBlend, 0.2f );
			mCurrentAngle = fLerp( mCurrentAngle, newAngle, mAngleBlend );

			direction.fNormalizeSafe( tVec2f::cZeroVector );

			f32 bounce = fAbs( sin( mBounceTime * c2Pi * Gameplay_Gui_WaveLaunchArrow_BounceFreq ) );
			bounce *= bounce;
			bounce *= Gameplay_Gui_WaveLaunchArrow_BounceMag;
			direction *= (bounce + Gameplay_Gui_WaveLaunchArrow_Offset);
			uiPoint -= direction;

			mLifeLeft -= dt;

			f32 alpha = 1.0f;
			if( mLifeLeft > mDuration )
				alpha = 1.0f - (mLifeLeft - mDuration) / Gameplay_Gui_WaveLaunchArrow_FadeIn;
			else if( mLifeLeft < Gameplay_Gui_WaveLaunchArrow_FadeOut && Gameplay_Gui_WaveLaunchArrow_FadeIn != 0.0f )
				alpha = mLifeLeft / Gameplay_Gui_WaveLaunchArrow_FadeOut;

			if( mLifeLeft <= 0.0f )
				fDelete( );
			else
				Sqrat::Function( mControl.fCanvas( ).fScriptObject( ), "Set" ).Execute( uiPoint, mCurrentAngle, alpha );
		}
	}

	void tWaveLaunchArrowUI::fShow( )
	{
		mLifeLeft = mDuration + Gameplay_Gui_WaveLaunchArrow_FadeIn;
		mBounceTime = 0.0f;

		Sqrat::Function( mControl.fCanvas( ).fScriptObject( ), "Show" ).Execute( true );
	}

	void tWaveLaunchArrowUI::fDestroy( )
	{
		if( !mControl.fCanvas( ).fIsNull( ) )
			mControl.fCanvas( ).fDeleteSelf( );
		//Sqrat::Function( mControl.fCanvas( ).fScriptObject( ), "Show" ).Execute( false );
	}
}}


namespace Sig { namespace Gui
{
	void tWaveLaunchArrowUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tWaveLaunchArrowUI, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		//classDesc
		//	;
		vm.fNamespace(_SC("Gui")).Bind( _SC("WaveLaunchArrowUI"), classDesc );
	}
}}

