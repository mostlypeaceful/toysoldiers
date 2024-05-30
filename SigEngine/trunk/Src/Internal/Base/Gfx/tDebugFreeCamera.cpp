#include "BasePch.hpp"
#include "tDebugFreeCamera.hpp"
#include "tGameAppBase.hpp"
#include "tProximity.hpp"
#include "tMeshEntity.hpp"
#include "tGameAppBase.hpp"
#include "Gui/tText.hpp"

namespace Sig { namespace Gfx
{
#ifdef sig_devmenu
	devvar( bool, Debug_EntityStats_Render, false );
	devvar_clamp( f32, Debug_EntityStats_Distance, 50.f, 1.f, 1000.f, 1 );
	namespace
	{
		static tEntityPtr gDebugEntity;
		static Gui::tTextPtr gStatsText;
	}
#endif//sig_devmenu

	void tDebugFreeCamera::fGlobalAppTick( )
	{
#ifdef sig_devmenu
		tGameAppBase* gameApp = tApplication::fInstance( ).fDynamicCast< tGameAppBase >( );
		if( !gameApp )
			return;

		tSceneGraph& sg = *gameApp->fSceneGraph( );

		if( Debug_EntityStats_Render && gDebugEntity && !sg.fIsPaused( ) )
		{
			sg.fDebugGeometry( ).fRenderOnce( tMeshEntity::fCombinedWorldSpaceBox( *gDebugEntity  ), Math::tVec4f( 0.f, 1.f, 0.f, 0.08f ) );
		}

		if( gDebugEntity )
		{
			if( !gStatsText )
			{
				gStatsText.fReset( NEW Gui::tText( ) );
				gStatsText->fSetDevFont( );
			}

			std::string statsText;
			gDebugEntity->fComputeDebugText( statsText );
			gStatsText->fBakeBox( 800, statsText.c_str( ), 0, Gui::tText::cAlignLeft );
			gStatsText->fSetPosition( Math::tVec3f( 1240.f - 500.f, 40.f, 0.0f ) );
			gStatsText->fSetRgbaTint( Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
			gameApp->fScreen( )->fAddScreenSpaceDrawCall( gStatsText->fDrawCall( ) );
		}
#endif//sig_devmenu
	}

	void tDebugFreeCamera::fSetDebugEntity( const tEntityPtr& e )
	{
#ifdef sig_devmenu
		gDebugEntity.fReset( NULL );
		gDebugEntity = e;
#endif//sig_devmenu
	}

	tEntityPtr tDebugFreeCamera::fGetDebugEntity( )
	{
#ifdef sig_devmenu
		return gDebugEntity;
#else
		return tEntityPtr( );
#endif//sig_devmenu
	}

	tLogic* tDebugFreeCamera::fGetDebugEntityLogic( )
	{
#ifdef sig_devmenu
		if( gDebugEntity )
			return gDebugEntity->fLogic( );
#endif//sig_devmenu
		return NULL;
	}
	
	void tDebugFreeCamera::fReleaseDebugEntity( )
	{
#ifdef sig_devmenu
		gDebugEntity.fRelease( );
#endif//sig_devmenu
	}

	tDebugFreeCamera::tDebugFreeCamera( const tUserPtr& user )
		: tFreeCamera( user )
	{
	}
	void tDebugFreeCamera::fOnTick( f32 dt )
	{
		tFreeCamera::fOnTick( dt );

#ifdef sig_devmenu
		if( gDebugEntity && !gDebugEntity->fSceneGraph( ) )
			gDebugEntity.fRelease( );

		const Input::tGamepad& rawGamepad =  mUsers.fFront( )->fRawGamepad( );
		if( rawGamepad.fButtonDown( Input::tGamepad::cButtonStart ) && !rawGamepad.fButtonHeld( Input::tGamepad::cButtonSelect ) )
		{
			tGameAppBase* gameApp = tApplication::fInstance( ).fDynamicCast< tGameAppBase >( );
			gameApp->fPause( !gameApp->fPaused( ) );
		}

		if( !Debug_EntityStats_Render )
			return;

		tGameAppBase* gameApp = tApplication::fInstance( ).fDynamicCast< tGameAppBase >( );
		if( !gameApp )
			return;

		tSceneGraph& sg = *gameApp->fSceneGraph( );

		if( sg.fIsPaused( ) )
			sg.fDebugGeometry( ).fClear( );

		const Gfx::tCamera& camera = fViewport( )->fLogicCamera( );
		const Input::tGamepad& gamepad = mUsers.fFront( )->fRawGamepad( ); 

		if( mFollowingEntity )
		{
			// move the camera local to the following entity
			const Math::tVec3f cameraY = mFollowingEntity->fObjectToWorld( ).fInverseXformVector( camera.fLocalToWorld( ).fYAxis( ) );
			const Math::tVec3f cameraX = mFollowingEntity->fObjectToWorld( ).fInverseXformVector( camera.fLocalToWorld( ).fXAxis( ) );
			const Math::tVec2f rotate = gamepad.fRightStick( ) * 0.05f;
			const Math::tVec2f pan = gamepad.fLeftStick( ) * 1.f;
			f32 upV = 0.f;

			if( !gamepad.fButtonHeld( Input::tGamepad::cButtonSelect ) )
			{
				if( gamepad.fButtonHeld( Input::tGamepad::cButtonLShoulder ) ) upV -= 1.f;
				if( gamepad.fButtonHeld( Input::tGamepad::cButtonRShoulder ) ) upV += 1.f;
			}

			mRotateVel += rotate;
			mPanVel += pan;
			mUpVel += upV;
			mRotateVel *= 0.5f;
			mPanVel *= 0.5f;
			mUpVel *= 0.5f;

			Math::tQuatf rotateY( Math::tAxisAnglef( Math::tVec3f::cYAxis, -mRotateVel.x ) );
			Math::tQuatf rotateE( Math::tAxisAnglef( cameraX, -mRotateVel.y ) );
			Math::tQuatf rotateQ = rotateE * rotateY;
			
			f32 viewLen;
			Math::tVec3f viewDir = mFollowingLookAt - mFollowingEye;
			viewDir.fNormalize( viewLen );
			viewDir = rotateQ.fRotate( viewDir );

			mFollowingEye += viewDir * mPanVel.y + cameraX * -mPanVel.x + cameraY * mUpVel;
			mFollowingLookAt = mFollowingEye + viewDir * viewLen;

			Gfx::tTripod tripod;
			tripod.mUp = Math::tVec3f::cYAxis;
			tripod.mLookAt = mFollowingEntity->fObjectToWorld( ).fXformPoint( mFollowingLookAt );
			tripod.mEye = mFollowingEntity->fObjectToWorld( ).fXformPoint( mFollowingEye );

			Gfx::tCamera newCamera = camera;
			newCamera.fSetTripod( tripod );
			fViewport( )->fSetCameras( newCamera );
		}

		const Math::tVec3f probePoint = camera.fGetTripod( ).mEye + Debug_EntityStats_Distance * camera.fZAxis( );
		const Math::tSpheref proxSphere = Math::tSpheref( probePoint, 10.f );
		sg.fDebugGeometry( ).fRenderOnce( Math::tSpheref( probePoint, 0.5f ), Math::tVec4f( 1.f, 0.f, 0.f, 1.0f ) );
		sg.fDebugGeometry( ).fRenderOnce( proxSphere, Math::tVec4f( 1.f, 1.f, 1.f, 0.5f ) );

		tProximity proximity;
		proximity.fAddSphere( proxSphere );
		proximity.fSetFilterByLogicEnts( true );
		tDynamicArray<u32> spatialSetIndices;
		Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
		proximity.fSetSpatialSetIndices( spatialSetIndices );
		proximity.fRefreshMT( 0.f, sg.fRootEntity( ) );

		tEntity* bestEntity = 0;
		f32 bestEntityDist = 10000.f;
		for( u32 i = 0; i < proximity.fEntityCount( ); ++i )
		{
			tEntity* targetEnt = proximity.fGetEntity( i );

			const f32 dist = ( targetEnt->fObjectToWorld( ).fGetTranslation( ) - probePoint )./*fProjectToXZ( ).*/fLength( );
			if( dist < bestEntityDist )
			{
				bestEntity = targetEnt;
				bestEntityDist = dist;
			}
		}

		f32 boxAlpha = 0.5f;
		if( bestEntity == mFollowingEntity ) boxAlpha = 0.08f;

		if( bestEntity && bestEntity != gDebugEntity )
			sg.fDebugGeometry( ).fRenderOnce( tMeshEntity::fCombinedWorldSpaceBox( *bestEntity ), Math::tVec4f( 1.f, 0.f, 0.f, boxAlpha ) );

		if( gamepad.fButtonDown( Input::tGamepad::cButtonX ) )
			gDebugEntity.fReset( bestEntity );

		if( gamepad.fButtonDown( Input::tGamepad::cButtonY ) )
		{
			// we may zoomed in to close to not have a best entity for the target we want
			//  if that's the case, use the gDebugEntity.
			tEntity *potentialFollow = bestEntity ? bestEntity : gDebugEntity.fGetRawPtr( );

			if( mFollowingEntity )
				mFollowingEntity.fRelease( );
			else
			{
				mFollowingEntity.fReset( potentialFollow );
				if( mFollowingEntity )
				{
					Gfx::tTripod tripod = camera.fGetTripod( );
					mFollowingEye = mFollowingEntity->fWorldToObject( ).fXformPoint( tripod.mEye );
					mFollowingLookAt = mFollowingEntity->fWorldToObject( ).fXformPoint( tripod.mLookAt );

					mPanVel = Math::tVec2f::cZeroVector;
					mRotateVel = Math::tVec2f::cZeroVector;
					mUpVel = 0;
				}
			}
		}

#endif//sig_devmenu
	}
	void tDebugFreeCamera::fOnActivate( b32 active )
	{
		tFreeCamera::fOnActivate( active );
	}

}}

