#include "GameAppPch.hpp"
#include "tTransitionCamera.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tSceneGraphCollectTris.hpp"

namespace Sig
{
	namespace
	{
		struct tGroundRayCastCallback
		{
			mutable Math::tRayCastHit		mHit;

			explicit tGroundRayCastCallback( )
			{
			}

			inline void fRayCastAgainstSpatial( const Math::tRayf& ray, tSpatialEntity* spatial ) const
			{
				if( spatial->fToSpatialSetObject( )->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( !hit.fHit( ) )
					return;

				if( hit.mT < mHit.mT )
					mHit = hit;
			}
			
			inline void operator( )( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;

				tSpatialEntity* spatial = static_cast<tSpatialEntity*>( i );
				if( !spatial->fHasGameTagsAny( GameFlags::cFLAG_GROUND ) )
					return;

				fRayCastAgainstSpatial( ray, spatial );
			}
		};
	}

	tTransitionCamera::tTransitionCamera( tPlayer& player, const Math::tMat3f& start, const Math::tMat3f& end, f32 transitionTime )
		: Gfx::tCameraController( player.fUser( )->fViewport( ) )
		, mPlayer( player )
		, mStartXForm( start )
		, mEndXForm( end )
		, mTransitionTime( transitionTime )
		, mCounter( 0.f )
	{
		fAddUser( player.fUser( ) );
	}
	tTransitionCamera::~tTransitionCamera( )
	{
	}
	void tTransitionCamera::fOnTick( f32 dt )
	{
		Gfx::tCamera camera = fViewport( )->fRenderCamera( );

		const Math::tVec3f startPos = mStartXForm.fGetTranslation( );
		const Math::tVec3f endPos = mEndXForm.fGetTranslation( );

		const Math::tVec3f startLook = mStartXForm.fZAxis( );
		const Math::tVec3f endLook = mEndXForm.fZAxis( );
		
		mCounter += dt;

		const f32 t = fClamp( mCounter / mTransitionTime, 0.f, 1.f );
		const Math::tVec3f eye = Math::fLerp( startPos, endPos, t );
		const Math::tVec3f view = Math::fLerp( startLook, endLook, t );
		Math::tVec3f lookAt = eye + view;

		const Math::tRayf ray( eye, view * Math::cInfinity );
		tGroundRayCastCallback rayCastcb;

		tGameApp::fInstance( ).fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastcb );

		if( rayCastcb.mHit.fHit( ) )
			lookAt = ray.fEvaluate( rayCastcb.mHit.mT );
		
		camera.fSetEye( eye );
		camera.fSetLookAt( lookAt );

		fViewport( )->fSetCameras( camera );
	}
	void tTransitionCamera::fOnActivate( b32 active )
	{
	}
}
