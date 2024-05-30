#include "GameAppPch.hpp"
#include "tDropLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "Physics/tGroundRayCastCallback.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;

namespace Sig
{
	tDropLogic::tDropLogic( )
		: mFalling( true )
	{ 
	}

	tDropLogic::~tDropLogic( )
	{ }

	void tDropLogic::fOnSpawn( )
	{
		fOnPause( false );

		tProjectileLogic::fOnSpawn( );

		mPhysics.mPosition = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		mPhysics.mVelocity = mLaunchVector;

		mAudio.fReset( NEW Audio::tSource( "Drop smoke" ) );
		if( mSpawnAudioEvent.fExists( ) )
			mAudio->fHandleEvent( mSpawnAudioEvent );

		fEnableParticles( );
	}

	void tDropLogic::fOnDelete( )
	{ 
		tProjectileLogic::fOnDelete( );

		if( mAudio )
		{
			if( mDeleteAudioEvent.fExists( ) )
				mAudio->fHandleEvent( mDeleteAudioEvent );
			mAudio->fDelete( ); //mAudio is not a child, so that we get on empty next on fOwnerEntity
		}

		mAudio.fRelease( );
	}

	void tDropLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}

	void tDropLogic::fActST( f32 dt )
	{
		tProjectileLogic::fActST( dt );
	}

	void tDropLogic::fMoveST( f32 dt )
	{
		if( mFalling )
		{
			fOwnerEntity( )->fMoveTo( mNextPos );
			if( mAudio )
				mAudio->fMoveTo( mNextPos );
		}
	}

	void tDropLogic::fCoRenderMT( f32 dt )
	{	
		if( mFalling )
			tProjectileLogic::fCoRenderMT( dt );
	}

	void tDropLogic::fComputeNewPosition( f32 dt )
	{
		dt *= mTimeMultiplier;
		mNextPos.fSetTranslation( mPhysics.fStep( dt ) );
		sigassert( !mNextPos.fIsNan( ) );

		fCheckLevelBounds( );
	}

	void tDropLogic::fHitSomething( const tEntityPtr& ent )
	{
		if( mFalling )
		{
			mFalling = false;
			if( mAudio && mLandAudioEvent.fExists( ) )
				mAudio->fHandleEvent( mLandAudioEvent );
		}
	}

	void tDropLogic::fRayCast( )
	{
		const tVec3f currPos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		const tVec3f delta = mNextPos.fGetTranslation( ) - currPos;
		tRayf ray( currPos, delta );

		Physics::tGroundRayCastCallback rayCastcb( *fOwnerEntity( ), GameFlags::cFLAG_GROUND );

		fOwnerEntity( )->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastcb );

		if( rayCastcb.mHit.fHit( ) )
		{
			mEntityHitReal.fReset( rayCastcb.mFirstEntity );
			mEntityHitWithLogic.fReset( mEntityHitReal->fFirstAncestorWithLogic( ) );
			mNextPos.fSetTranslation( ray.fEvaluate( rayCastcb.mHit.mT ) );
		}
	}

}


namespace Sig
{
	void tDropLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tDropLogic, tProjectileLogic, Sqrat::NoCopy<tDropLogic> > classDesc( vm.fSq( ) );
		classDesc
			;

		vm.fRootTable( ).Bind(_SC("DropLogic"), classDesc);
	}
}

