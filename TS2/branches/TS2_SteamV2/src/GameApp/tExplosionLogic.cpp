#include "GameAppPch.hpp"
#include "tExplosionLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tUnitLogic.hpp"
#include "tLightEffectLogic.hpp"
#include "tLevelLogic.hpp"
#include "tSync.hpp"
#include "tRandom.hpp"
#include "tUberBreakableLogic.hpp"

#include "tDecalManager.hpp"
#include "tWeapon.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;

namespace Sig
{
	devvar( f32, Gameplay_LightEffects_ExplosionAdditionalRadius, 1.f );
	devvar( f32, Gameplay_LightEffects_ExplosionLife, 0.63f );
	devvar( f32, Gameplay_LightEffects_ExplosionHeightOffset, 1.5f );
	devvar( f32, Gameplay_Explosions_UberBreakableMinVel, 5.0f );
	devvar( f32, Gameplay_Explosions_UberBreakableMaxVel, 60.0f );

	devvar( bool, Perf_EnableLightEffects, true );


	tExplosionLogic::tExplosionLogic( )
		: mSize( 0.f )
		, mFullSize( 1.f )
		, mGrowRate( 1.f )
		, mFalloff( 0.f )
		//, mConeWorldAxis( tVec3f::cYAxis )
		//, mConeAngle( cInfinity )
		, mExplicitHitPoints( 20.f )
		, mDamageMod( 1.f )
		, mCountry( GameFlags::cCOUNTRY_DEFAULT )
		, mLightSpawned( false )
		, mLightSize( -1 )
		, mLightExpandTime( 0.0f )
		, mLightCollapseTime( Gameplay_LightEffects_ExplosionLife )
		, mLightHeight( Gameplay_LightEffects_ExplosionHeightOffset )
		, mLightColor( Math::tVec4f::cOnesVector )
		, mReuseLight( NULL )
	{
	}
	tExplosionLogic::~tExplosionLogic( )
	{
	}
	void tExplosionLogic::fOnSpawn( )
	{
		fOnPause( false );

		tLogic::fOnSpawn( );

		mSize = mGrowRate * sync_rand( fFloatInRange( 0.01f, 0.03f ) );
		mProximity.fAddSphere( tSpheref( mSize ) );
		mProximity.fSetFilterByLogicEnts( true );

		// only look for shape entities
		tDynamicArray<u32> spatialSetIndices;
		Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
		spatialSetIndices.fPushBack( tShapeEntity::cSpatialSetIndex );
		mProximity.fSetSpatialSetIndices( spatialSetIndices );

		mProximity.fSetTrackNewEnts( true );

		if( mLightSize > 0.f )
			fSpawnLight( fOwnerEntity( )->fObjectToWorld( ) );

		//tDecalManager::fInstance( ).fPlace( tGameApp::fInstance( ).fDefaultDecalTexture( )
		//	, tObbf( tSpheref( fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ), mFullSize ) ) );
	}
	void tExplosionLogic::fOnDelete( )
	{
		mDamageID = tDamageID( );
		mProximity.fReleaseAllEntityRefsST( );
		tLogic::fOnDelete( );
	}
	void tExplosionLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListThinkST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListThinkST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}
	void tExplosionLogic::fSpawnLight( const tMat3f& worldXForm )
	{
		if( Perf_EnableLightEffects && !mLightSpawned )
		{
			mLightSpawned = true;
			tEntity *parent = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( );

			f32 size = mLightSize < 0.f ? mFullSize + Gameplay_LightEffects_ExplosionAdditionalRadius : mLightSize;
			
			if( mReuseLight )
			{
				mReuseLight->fSetParameters( size, mLightExpandTime, mLightCollapseTime );
				mReuseLight->fSetColor( mLightColor );
				mReuseLight->fRestart( );
				mReuseLight->fOwnerEntity( )->fMoveTo( worldXForm );
				mReuseLight->fOwnerEntity( )->fTranslate( tVec3f( 0, mLightHeight, 0 ) );
			}
			else
			{
				tLightEffectLogic *le = NEW tLightEffectLogic( size, mLightExpandTime, mLightCollapseTime );
				Math::tVec3f trans = worldXForm.fGetTranslation( );
				tEntity* light = tLightEffectLogic::fSpawnLightEffect( worldXForm, le, *parent, mLightColor );
				light->fTranslate( tVec3f( 0, mLightHeight, 0 ) );
			}
		}
	}
	void tExplosionLogic::fActST( f32 dt )
	{
		profile( cProfilePerfExplosionLogicActST );

		tDamageContext damageContext;
		damageContext.mMaxSize = mFullSize;
		damageContext.mFalloff = mFalloff;
		damageContext.mWorldPosition = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		damageContext.fSetDamageMultiplier( mDamageMod );
		sigassert( !damageContext.mWorldPosition.fIsNan( ) );

		damageContext.fSetAttacker( mDamageID, GameFlags::cDAMAGE_TYPE_EXPLOSION );
		if( !mDamageID.mDesc )
			damageContext.fSetExplicit( mExplicitHitPoints );

		sync_event_v_c( mProximity.fNewEntityCount( ), tSync::cSCLogic );
		for( u32 i = 0; i < mProximity.fNewEntityCount( ); ++i )
		{
			tEntity* ent = mProximity.fGetNewEntity( i );
			sigassert( ent );

			tUnitLogic* unitLogic = ent->fLogicDerived< tUnitLogic >( );
			if( unitLogic )
			{
				//delta.y = 0;
				//delta.fNormalizeSafe( tVec3f::cZeroVector );
				//f32 angle = fAcos( delta.fDot( mConeWorldAxis ) );
				//if( angle <= mConeAngle )
					unitLogic->fDealDamage( damageContext );
			}
#ifdef __tUberBreakableLogic__
			else
			{
				tUberBreakablePiece* uberBreakPiece = ent->fDynamicCast< tUberBreakablePiece >( );
				if( uberBreakPiece )
				{
					tVec3f delta = ent->fObjectToWorld( ).fGetTranslation( ) - fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );

					delta.fNormalizeSafe( tVec3f::cZeroVector );
					delta.y = fAbs( delta.y );
					delta *= sync_rand( fFloatInRange( Gameplay_Explosions_UberBreakableMinVel, Gameplay_Explosions_UberBreakableMaxVel ) );
					uberBreakPiece->fOnHit( delta );
				}
			}
#endif//__tUberBreakableLogic__
		}

		tLogic::fActST( dt );
	}
	void tExplosionLogic::fThinkST( f32 dt )
	{
		profile( cProfilePerfExplosionLogicThinkST );

		mProximity.fCleanST( );
		if( fReadyForDeletion( ) )
			fOwnerEntity( )->fDelete( );
	}
	void tExplosionLogic::fCoRenderMT( f32 dt )
	{
		profile( cProfilePerfExplosionLogicCoRenderMT );

		sigassert( mProximity.fShapes( ).fCount( ) == 1 );
		mSize += mGrowRate * dt;
		sync_event_v_c( mSize, tSync::cSCLogic );
		
		mProximity.fShapes( )[ 0 ] = tProximity::tShape( tProximity::cShapeSphere, tAabbf( tSpheref( fMin( mSize, mFullSize ) ) ) );
		mProximity.fRefreshMT( dt, *fOwnerEntity( ) );
		sync_event_v_c( mProximity.fEntityCount( ), tSync::cSCDebris );
	}
	b32 tExplosionLogic::fReadyForDeletion( )
	{
		return mSize >= mFullSize;
	}
}


namespace Sig
{
	void tExplosionLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tExplosionLogic, tLogic, Sqrat::NoCopy<tExplosionLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("FullSize"),	&tExplosionLogic::mFullSize)
			.Var(_SC("GrowRate"),	&tExplosionLogic::mGrowRate)
			.Var(_SC("HitPoints"),	&tExplosionLogic::mExplicitHitPoints)
			.Var(_SC("Country"),	&tExplosionLogic::mCountry)
			.Var(_SC("LightSize"),	&tExplosionLogic::mLightSize)
			.Var(_SC("LightExpandTime"),	&tExplosionLogic::mLightExpandTime)
			.Var(_SC("LightCollapseTime"),	&tExplosionLogic::mLightCollapseTime)
			.Var(_SC("LightHeight"),		&tExplosionLogic::mLightHeight)
			.Var(_SC("LightColor"),		&tExplosionLogic::mLightColor)
			;

		vm.fRootTable( ).Bind(_SC("ExplosionLogic"), classDesc);
	}
}

