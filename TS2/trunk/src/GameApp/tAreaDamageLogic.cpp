#include "GameAppPch.hpp"
#include "tAreaDamageLogic.hpp"
#include "tShapeEntity.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "tUnitLogic.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( bool, Gameplay_Weapon_DrawAreaDamage, false );

	tAreaDamageLogic::tAreaDamageLogic( )
		: mEnabled( true )
	{
	}
	tAreaDamageLogic::~tAreaDamageLogic( )
	{
	}
	void tAreaDamageLogic::fOnSpawn( )
	{
		fOnPause( false );

		tLogic::fOnSpawn( );

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			fAddShape( fOwnerEntity( )->fChild( i ).fGetRawPtr( ) );
		}

		fAddShape( fOwnerEntity( ) );

		// only look for shape entities
		tDynamicArray<u32> spatialSetIndices;
		Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
		//spatialSetIndices.fPushBack( tShapeEntity::cSpatialSetIndex );
		mProximity.fSetSpatialSetIndices( spatialSetIndices );
		mProximity.fSetRefreshFrequency( 0.2f, 0.1f );

		if( mAttackerID.mTeam == ~0 )
		{
			sigassert( fOwnerEntity( )->fParent( ) );
			tEntity* owner = fOwnerEntity( )->fParent( )->fFirstAncestorWithLogic( );
			if( owner )
			{
				tAreaEffectParent* effectParent = owner->fDynamicCast<tAreaEffectParent>( );
				if( effectParent )
				{
					mAttackerID = effectParent->mWeapon->fBuildID( );
				}
				else
					log_warning( 0, "No weapon parent found for area damage!" );
			}
			else
				sigassert( "no owner found for area damage :(" );
		}

		if( mAttackerID.mTeam != ~0 )
		{
			mProximity.fSetFilterByLogicEnts( true );
			mProximity.fLogicFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_TEAM, GameFlags::cTEAM_NONE ) );
			mProximity.fLogicFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_TEAM, tPlayer::fDefaultEnemyTeam( mAttackerID.mTeam ) ) );
		}
		else
			mProximity.fClearShapes( );
	}
	void tAreaDamageLogic::fAddShape( tEntity* ent )
	{
		tShapeEntity* se = ent->fDynamicCast<tShapeEntity>( );
		if( se )
		{
			if( se->fShapeType( ) == tShapeEntityDef::cShapeTypeBox )
			{
				tAabbf box( se->fBox( ).fTransform( fOwnerEntity( )->fWorldToObject( ) ) );
				mProximity.fAddObb( box );
			}
			else if( se->fShapeType( ) == tShapeEntityDef::cShapeTypeSphere )
			{
				tSpheref s = se->fSphere( ).fTranslate( fOwnerEntity( )->fWorldToObject( ).fGetTranslation( ) );
				mProximity.fAddSphere( s );
			}
		}
	}
	void tAreaDamageLogic::fOnDelete( )
	{
		mProximity.fReleaseAllEntityRefsST( );
		mTargets.fSetCount( 0 );
		tLogic::fOnDelete( );
	}
	void tAreaDamageLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}
	void tAreaDamageLogic::fActST( f32 dt )
	{
		if( Gameplay_Weapon_DrawAreaDamage )
			for( u32 i = 0; i < mProximity.fShapes( ).fCount( ); ++i )
				fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mProximity.fShapes( )[ i ].fToObb( fOwnerEntity( )->fObjectToWorld( ) ), tVec4f(0,1,0,1) );

		if( mEnabled && mProximity.fEntityCount( ) )
		{
			tDamageContext damageContext;	
			damageContext.fSetAttacker( mAttackerID, GameFlags::cDAMAGE_TYPE_AREA );
			damageContext.mWorldPosition = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
			for( u32 i = 0; i < mProximity.fEntityCount( ); ++i )
			{
				tEntity* ent = mProximity.fGetEntity( i );
				sigassert( ent );
				tUnitLogic* unitLogic = ent->fLogicDerived< tUnitLogic >( );
				if( unitLogic ) 
					unitLogic->fDealDamage( damageContext );
			}
		}

		mProximity.fCleanST( );
	}
	b32 tAreaDamageLogic::fShouldDamage( tUnitLogic* logic )
	{
		if( mAttackerID.mDesc ) 
			return (mAttackerID.mDesc->fTargetPriority( logic->fUnitType( ) ) != ~0);
		else
			return true;
	}
	void tAreaDamageLogic::fCoRenderMT( f32 dt )
	{
		if( mProximity.fShapes( ).fCount( ) )
			mProximity.fRefreshMT( dt, *fOwnerEntity( ) );
	}





	tInstaKillAreaDamageLogic::tInstaKillAreaDamageLogic( )
	{
		mAttackerID = tDamageID( GameFlags::cTEAM_RED ); //SUPER HACK
	}

	void tInstaKillAreaDamageLogic::fActST( f32 dt )
	{
		if( Gameplay_Weapon_DrawAreaDamage )
			for( u32 i = 0; i < mProximity.fShapes( ).fCount( ); ++i )
				fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mProximity.fShapes( )[ i ].fToObb( fOwnerEntity( )->fObjectToWorld( ) ), tVec4f(0,1,0,1) );

		if( mProximity.fEntityCount( ) )
		{
			for( u32 i = 0; i < mProximity.fEntityCount( ); ++i )
			{
				tEntity* ent = mProximity.fGetEntity( i );
				sigassert( ent );
				tUnitLogic* unitLogic = ent->fLogicDerived< tUnitLogic >( );
				if( unitLogic ) 
				{
					if( unitLogic->fTeam( ) != mAttackerID.mTeam && unitLogic->fIsValidTarget( ) )
						unitLogic->fDestroy( mAttackerID.mTeam );
				}
			}
		}

		mProximity.fCleanST( );

		tLogic::fActST( dt );
	}
}


namespace Sig
{
	void tAreaDamageLogic::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tAreaDamageLogic, tLogic, Sqrat::NoCopy<tAreaDamageLogic> > classDesc( vm.fSq( ) );
			classDesc
				;

			vm.fRootTable( ).Bind(_SC("AreaDamageLogic"), classDesc);
		}
		{
			Sqrat::DerivedClass<tInstaKillAreaDamageLogic, tAreaDamageLogic, Sqrat::NoCopy<tInstaKillAreaDamageLogic> > classDesc( vm.fSq( ) );
			classDesc
				;

			vm.fRootTable( ).Bind(_SC("InstaKillAreaDamageLogic"), classDesc);
		}
	}
}

