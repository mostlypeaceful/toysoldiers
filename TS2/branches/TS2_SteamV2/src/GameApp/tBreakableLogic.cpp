#include "GameAppPch.hpp"
#include "tBreakableLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tWaypointLogic.hpp"
#include "tGameArchive.hpp"


namespace Sig
{
	struct tBreakableSaveData : public tEntitySaveData
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tBreakableSaveData, 0x7A840518 );
	public:
		tBreakableSaveData( )
			: mStateIndex( 0 )
		{
		}
		virtual void fRestoreSavedEntity( tEntity* entity ) const
		{
			tUnitLogic* breakable = entity->fLogicDerived< tUnitLogic >( );
			if( breakable )
				breakable->fSetCurrentBreakState( mStateIndex );
		}
		virtual void fSaveLoadDerived( tGameArchive& archive ) { fSaveLoad( archive ); }
		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			tEntitySaveData::fSaveLoadDerived( archive );
			archive.fSaveLoad( mStateIndex );
		}

	public:
		u8 mStateIndex;
	};
	register_rtti_factory( tBreakableSaveData, false );
}

namespace Sig
{	
	namespace 
	{ 
	}

	tBreakableLogic::tBreakableLogic( ) 
		: mBreakState( GameFlags::cTRIGGERED_BREAK_STATE_COUNT )
	{
	}
	void tBreakableLogic::fOnSpawn( )
	{
		// Dont add this by default! lots of wave list units use breakables and this will leak memory.
		//fOwnerEntity( )->fAddGameTags( GameFlags::cFLAG_SAVEABLE ); // do this before tUnitLogic::fOnSpawn

		tUnitLogic::fOnSpawn( );
		fComputeCollisionShapeIfItDoesntExist( );

		//defaults to trigger after first state transition
		mBreakState = (GameFlags::tTRIGGERED_BREAK_STATE) fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_TRIGGERED_BREAK_STATE, 0 );
		
		// Setting cFLAG_USE_DEFAULT_TRANSITION will cause the sigml to use mshmls as debris
		if( fOwnerEntity( )->fHasGameTagsAll( tEntityTagMask( GameFlags::cFLAG_DEFAULT_END_TRANSITION ) ) )
			fSetUseDefaultEndTransition( true );
	}
	void tBreakableLogic::fOnDelete( )
	{
		tUnitLogic::fOnDelete( );
	}
	void tBreakableLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
		}
		else
		{
			fRunListInsert( cRunListActST );
		}
	}
	void tBreakableLogic::fOnStateChanged( )
	{
		sigassert( fOwnerEntity( ) );

		// Change to >= in case we skip a state
		if( mCurrentState >= (s32) mBreakState 
			&& !fOwnerEntity( )->fName( ).fNull( ) )
		{
			//disable waypoints with same name
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			sigassert( level );

			const tGrowableArray<tPathEntityPtr>& waypoints = level->fNamedWayPoints( );

			for( u32 i = 0; i < waypoints.fCount( ); ++i )
			{
				sigassert( waypoints[ i ] );
				if( fOwnerEntity( )->fName( ) == waypoints[ i ]->fName( ) )
				{
					tWaypointLogic* waypoint = waypoints[ i ]->fLogicDerived< tWaypointLogic >( );
					if( waypoint )
						waypoint->fSetAccessible( false );
				}
			}
		}

		tUnitLogic::fOnStateChanged( );
	}
	tRefCounterPtr<tEntitySaveData> tBreakableLogic::fStoreSaveGameData( b32 entityIsPartOfLevelFile, tSaveGameRewindPreview& preview )
	{
		tBreakableSaveData* saveData = 0;
		if( entityIsPartOfLevelFile )
		{
			saveData = NEW tBreakableSaveData( );
			sigassert( mCurrentState <= 0xFF000000 );
			saveData->mStateIndex = s8( mCurrentState );

		}
		return tRefCounterPtr<tEntitySaveData>( saveData );
	}
	void tBreakableLogic::fRegisterUnit( ) 
	{ 
		tGameApp::fInstance( ).fCurrentLevel( )->fRegisterSpecialLevelObject( fOwnerEntity( ) );
	}

	tAnimatedBreakableLogic::tAnimatedBreakableLogic( )
		: mApplyRefFrame( false )
	{
		mAnimatable.fSetLogic( this );
	}
	void tAnimatedBreakableLogic::fOnSpawn( )
	{
		tBreakableLogic::fOnSpawn( );
		mAnimatable.fOnSpawn( );
	}
	void tAnimatedBreakableLogic::fOnDelete( )
	{
		mAnimatable.fOnDelete( );
		tBreakableLogic::fOnDelete( );
	}
	void tAnimatedBreakableLogic::fOnSkeletonPropagated( )
	{
		tBreakableLogic::fOnSkeletonPropagated( );
		mAnimatable.fListenForAnimEvents( *this );
	}
	void tAnimatedBreakableLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListAnimateMT );
			fRunListRemove( cRunListMoveST );
		}
		else
		{
			fRunListInsert( cRunListAnimateMT );
			fRunListInsert( cRunListMoveST );
		}
	}
	void tAnimatedBreakableLogic::fAnimateMT( f32 dt )
	{
		fStepTintStack( dt );

		profile( cProfilePerfBreakableAnimateMT );

		mAnimatable.fAnimateMT( dt );
	}
	void tAnimatedBreakableLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfBreakableMoveST );

		if( mApplyRefFrame && mAnimatable.fAnimatedSkeleton( ) )
		{
			Math::tMat3f result = fOwnerEntity( )->fObjectToWorld( );
			mAnimatable.fAnimatedSkeleton( )->fRefFrameDelta( ).fApplyAsRefFrameDelta( result, 1.f );
			fOwnerEntity( )->fMoveTo( result );
		}

		mAnimatable.fMoveST( dt );
		tUnitLogic::fActST( dt ); //lol, MoveSt has multiple purposes to keep the run list count down on props
	}
}


namespace Sig
{
	void tBreakableLogic::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tBreakableLogic, tUnitLogic, Sqrat::NoCopy<tBreakableLogic> > classDesc( vm.fSq( ) );
			//classDesc
			//	;

			vm.fRootTable( ).Bind(_SC("BreakableLogic"), classDesc);
		}
		{
			Sqrat::DerivedClass<tAnimatedBreakableLogic, tBreakableLogic, Sqrat::NoCopy<tAnimatedBreakableLogic> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("ApplyRefFrame"), &tAnimatedBreakableLogic::mApplyRefFrame)
				;

			vm.fRootTable( ).Bind(_SC("AnimatedBreakableLogic"), classDesc);
		}
	}
}

