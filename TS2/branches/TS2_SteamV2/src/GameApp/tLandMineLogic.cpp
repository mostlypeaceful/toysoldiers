#include "GameAppPch.hpp"
#include "tLandMineLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tExplosionLogic.hpp"
#include "Math/ProjectileUtility.hpp"

using namespace Sig::Math;

namespace Sig
{
	namespace{
		const tFilePathPtr cProjectileUSAProximityPath( "Gameplay/projectiles/usa/proximity.sigml" );
	}//unnamed namespace
	tLandMineLogic::tLandMineLogic( )
		: mType( cTimedMine )
		, mParam( 1.0f )
		, mTime( 0.0f )
		, mTriggered( false )
		, mFullSize( 10.0f )
		, mGrowRate( 20.0f )
		, mExplicitHitPoints( 20.f )
		, mFalling( false )
		, mWasThrown( false )
		, mLayedByTeam( GameFlags::cTEAM_NONE )
	{
	}
	void tLandMineLogic::fOnSpawn( )
	{
		fOnPause( false );

		tProjectileLogic::fOnSpawn( );

		if( mType == cProximityMine )
		{
			mProximity.fAddSphere( Math::tSpheref( mParam ) );
			mProximity.fSetFilterByLogicEnts( true );

			// only look for shape entities
			tDynamicArray<u32> spatialSetIndices;
			Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
			mProximity.fSetSpatialSetIndices( spatialSetIndices );
		}
	}
	void tLandMineLogic::fOnDelete( )
	{
		mPlacedBy.fRelease( );
		mProximity.fReleaseAllEntityRefsST( );
		tProjectileLogic::fOnDelete( );
	}
	void tLandMineLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}
	void tLandMineLogic::fExplode( )
	{
		tEntity* explosion = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( mExplosionPath );
		if( explosion )
		{
			sigassert( explosion );

			explosion->fMoveTo( fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) );		
			explosion->fAcquireLogic( NEW tLogicPtr( NEW tExplosionLogic( ) ) );

			tExplosionLogic* explosionLogic = explosion->fLogicDerived< tExplosionLogic >( );
			//explosionLogic->fSetFiredBy( mPlacedBy.fGetRawPtr( ) );

			{
				explosionLogic->fSetFullSize( mFullSize );
				explosionLogic->fSetGrowRate( mGrowRate );
				explosionLogic->fSetHitPoints( mExplicitHitPoints );
			}
		}
		else
			log_warning( 0, "Unable to spawn explosion entity [" << mExplosionPath << "]" );

		// Lookup ent's "material type" and play audio, show some kind of effect

		fProjectileDelete( );
	}
	void tLandMineLogic::fActST( f32 dt )
	{
		if( mTriggered ) 
		{
			fExplode( );
			mTriggered = false;
		}

		tProjectileLogic::fActST( dt );
	}

	void tLandMineLogic::fInitPhysics( )
	{
		mPhysics.mPosition = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		mPhysics.mVelocity = mLaunchVector;
	}

	void tLandMineLogic::fComputeNewPosition( f32 dt )
	{
		dt *= mTimeMultiplier;
		mNextPos.fSetTranslation( mPhysics.fStep( dt ) );

		fCheckLevelBounds( );
	}

	void tLandMineLogic::fMoveST( f32 dt )
	{
		if( mWasThrown )
		{
			if( mFalling )
			{
				fOwnerEntity( )->fMoveTo( mNextPos );
			}
			else
			{
				/*Math::tMat3f xform = Math::tMat3f::cIdentity;
				xform.fSetTranslation( mTarget );*/
				Math::tMat3f xForm = mEntityHitWithLogic->fObjectToWorld( );
				xForm *= mRelativeTransform;
				fOwnerEntity( )->fMoveTo( xForm );
			}

		}

		//tProjectileLogic::fMoveST( dt );

		mProximity.fCleanST( );
	}
	void tLandMineLogic::fCoRenderMT( f32 dt )
	{		
		switch( mType )
		{
		case cProximityMine:
			mProximity.fRefreshMT( dt, *fOwnerEntity( ) );
			for( u32 e = 0; e < mProximity.fEntityCount( ); ++e )
			{
				tEntity* ent = mProximity.fGetEntity( e );
				if( ent == mPlacedBy )
					continue;

				tUnitLogic* unitLogic = ent->fLogicDerived< tUnitLogic >( );
				if( !unitLogic || !unitLogic->fIsValidTarget( ) )
					continue;

				b32 wantsToHitThisType = true; // TODO
				if( wantsToHitThisType && unitLogic->fTeam( ) != mLayedByTeam )
				{
					mTriggered = true;
					break;
				}
			}
			break;
		case cTimedMine:
			mTime += dt;
			if( mTime >= mParam )
				mTriggered = true;
			break;
		default:
			break;
		};

		if( mFalling )
			tProjectileLogic::fCoRenderMT( dt );
	}

	void tLandMineLogic::fHitSomething( const tEntityPtr& ent )
	{
		if( mFalling )
		{
			mFalling = false;

			Math::tMat3f theirT = ent->fObjectToWorld( );
			Math::tMat3f myT = fOwnerEntity( )->fObjectToWorld( );
			mRelativeTransform = theirT.fInverse( ) * myT;
		}

		tProjectileLogic::fHitSomething( ent );
	}

	void tLandMineLogic::fSetMineType( tMineType type, f32 param )
	{
		mType = type;
		mParam = param;
	}
	void tLandMineLogic::fSetMineTypeScript( u32 type, f32 param )
	{
		fSetMineType( (tMineType)type, param );
	}
	void tLandMineLogic::fSetPlacedBy( tEntity* placedBy )
	{
		mPlacedBy.fReset( placedBy );

		tUnitLogic *ul = placedBy->fLogicDerived<tUnitLogic>( );
		mLayedByTeam = ul ? ul->fTeam( ) : GameFlags::cTEAM_NONE;
	}

	/*static*/ void tLandMineLogic::fThrow( const Math::tMat3f& origin, const Math::tVec3f& target, const Math::tVec3f& normal, tEntity* owner )
	{
		sigassert( 0 ); //THIS NEEDS TO SET THE DAMAGEID
		const f32 speed = 30.0f;
		f32 angle = 0.f;

		const Math::tVec3f muzzlePos = origin.fGetTranslation( );
		if( ProjectileUtility::fComputeLaunchAngle( angle, speed, muzzlePos, target, tShellPhysics( ).mGravity.y, false ) )
		{
			tVec3f delta = target - muzzlePos;
			delta.y = 0.0f;
			delta.fNormalizeSafe( tVec3f::cZAxis );
			
			tVec3f xAxis = tVec3f::cYAxis.fCross( delta );
			tQuatf rotation( tAxisAnglef( xAxis, -angle ) );
			delta = rotation.fRotate( delta );

			tEntity* proj = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( cProjectileUSAProximityPath );

			if( proj )
			{
				proj->fMoveTo( origin );

				tLandMineLogic* logic = proj->fLogicDerived< tLandMineLogic >( );
				if( logic )
				{
					logic->fSetSpeed( speed );
					logic->fSetLaunchVector( delta * speed );
					logic->fInitPhysics( );

					logic->mFalling = true;
					logic->mWasThrown = true;
					logic->mTarget = target;
					logic->mNormal = normal;
				}
			}
			else
			{
				log_warning( 0, "Attempting to create land mine projectile from a scene graph file that was not loaded: " << cProjectileUSAProximityPath );
			}
		}
	}

}


namespace Sig
{
	void tLandMineLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tLandMineLogic, tLogic, Sqrat::NoCopy<tLandMineLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("Explode"),		&tLandMineLogic::fExplode )
			.Func(_SC("SetMineType"),	&tLandMineLogic::fSetMineTypeScript )
			.Func(_SC("SetExplosionPath"),	&tLandMineLogic::fSetExplosionPath )
			.Prop(_SC("FullSize"),	&tLandMineLogic::fFullSize,	&tLandMineLogic::fSetFullSize)
			.Prop(_SC("GrowRate"),	&tLandMineLogic::fGrowRate,	&tLandMineLogic::fSetGrowRate)
			.Prop(_SC("HitPoints"),	&tLandMineLogic::fHitPoints, &tLandMineLogic::fSetHitPoints)
			;

		vm.fRootTable( ).Bind(_SC("LandMineLogic"), classDesc);
	}
}

