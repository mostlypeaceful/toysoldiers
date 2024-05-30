#include "GameAppPch.hpp"
#include "tProximityLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tShapeEntity.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( bool, Debug_Proximity_RenderLogicStatus, false );

	tProximityLogic::tProximityLogic( )
		: mPreviousEntityCount( 0 )
		, mEnabled( true )
	{
	}

	void tProximityLogic::fOnSpawn( )
	{
		fOnPause( false );

		tShapeEntity* shape = fOwnerEntity( )->fDynamicCast<tShapeEntity>( );
		if( shape )
		{
			switch( shape->fShapeType( ) )
			{
			case tShapeEntityDef::cShapeTypeBox:
				mShape = Math::tObbf( shape->fObjectSpaceBox( ), fOwnerEntity( )->fObjectToWorld( ) );
				mProximity.fAddObb( shape->fObjectSpaceBox( ) );
				break;
			case tShapeEntityDef::cShapeTypeSphere:
				mProximity.fAddSphere( tSpheref( shape->fSphere( ).fRadius( ) ) );
				break;
			default:
				log_warning( 0, "tProximityLogic could not recognize shape type!" );
				break;
			}

			// only look for shape entities
			tDynamicArray<u32> spatialSetIndices;
			Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
			mProximity.fSetSpatialSetIndices( spatialSetIndices );
			mProximity.fSetFilterByLogicEnts( true );
			mProximity.fRefreshMT( 1.0f, *fOwnerEntity( ) ); //allocate some arrays and what not
		}
		else
			log_warning( 0, "tProximityLogic not assigned to a tShapeEntity!" );
	}
	void tProximityLogic::fOnDelete( )
	{
		mProximity.fReleaseAllEntityRefsST( );
		mEntityCountChanged = Sqrat::Function( );
		mNewEntity = Sqrat::Function( );
		tLogic::fOnDelete( );
	}
	void tProximityLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}
	void tProximityLogic::fActST( f32 dt )
	{
		profile( cProfilePerfProximityLogicActST );

		if( mEnabled )
		{
			mProximity.fCleanST( );

			if( mProximity.fEntityCount( ) != mPreviousEntityCount )
			{
				if( !mEntityCountChanged.IsNull( ) )
					mEntityCountChanged.Execute( this ); 
				mPreviousEntityCount = mProximity.fEntityCount( );
			}

			if( !mNewEntity.IsNull( ) )
			{
				for( u32 i = 0; i < mProximity.fNewEntityCount( ); ++i )
					mNewEntity.Execute( mProximity.fGetNewEntity( i ) );
			}
		}
	}
	void tProximityLogic::fCoRenderMT( f32 dt )
	{		
		if( mEnabled )
		{
			profile( cProfilePerfProximityLogicCoRenderMT );
			mProximity.fRefreshMT( dt, *fOwnerEntity( ) );
		}

		if( Debug_Proximity_RenderLogicStatus )
		{
			Math::tVec4f color;
			if( mProximity.fEntityList( ).fCount( ) > 0 )
				color = Math::tVec4f( 0,1,0, 0.25f );
			else
				color = Math::tVec4f( 1,0,0, 0.25f );
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mShape, color );
		}
	}
	void tProximityLogic::fSetFrequency( f32 seconds, f32 randomDeviation )
	{
		mProximity.fSetRefreshFrequency( seconds, randomDeviation );
	}
	void tProximityLogic::fAddEnumFilter( u32 key, u32 value )
	{
		mProximity.fLogicFilter( ).fAddProperty( tEntityEnumProperty( key, value ) );
	}
	void tProximityLogic::fSetEnabled( b32 enable )
	{
		if( !enable )
		{
			mProximity.fReleaseAllEntityRefsST( );
			fActST( 0.01f );
		}

		mEnabled = enable;
	}

}


namespace Sig
{
	void tProximityLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tProximityLogic, tLogic, Sqrat::NoCopy<tProximityLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("Proximity"),					&tProximityLogic::fProximityForScript)
			.Var(_SC("EntityCountChangedCallback"),	&tProximityLogic::mEntityCountChanged)
			.Var(_SC("NewEntCallback"),				&tProximityLogic::mNewEntity)
			.Prop(_SC("Enabled"),					&tProximityLogic::fEnabled, &tProximityLogic::fSetEnabled)
			;

		vm.fRootTable( ).Bind(_SC("ProximityLogic"), classDesc);
	}
}

