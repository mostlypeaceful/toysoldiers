#include "GameAppPch.hpp"
#include "tGoalBoxLogic.hpp"
#include "tWaypointLogic.hpp"
#include "tWorldSpaceFloatingText.hpp"

using namespace Sig::Math;

namespace Sig
{
	devrgba_clamp( Gameplay_ToyBox_Tint_Entered, tVec4f( 2.f, 0.f, 0.f, 1.f ), 0, 1.f, 2 );
	devvar( f32, Gameplay_ToyBox_Tint_EnteredDuration, 3.0f );
	devvar( f32, Gameplay_ToyBox_Tint_EnteredBlend, 0.2f );
	devvar( f32, Gameplay_ToyBox_Tint_EnteredFlashRate, 0.75f );
	devvar( f32, Gameplay_ToyBox_GamerTag_Scale, 1.0f );
	devrgba_clamp( Gameplay_ToyBox_GamerTag_Tint, tVec4f( 1.f, 1.f, 1.f, 1.f ), 0, 3.f, 2 );

	namespace
	{
		enum tToyBoxTints
		{
			cToyBoxTintEntered,
			cToyBoxTintCount
		};

		static const tStringPtr cGamerTag( "gamerTag" );
		static const tStringPtr cZone( "zone" );
	}

	tGoalBoxLogic::tGoalBoxLogic( )
		: mActive( true )
	{
		mTintStack.fStack( ).fSetCount( cToyBoxTintCount );
		mTintStack.fStack( )[ cToyBoxTintEntered ].fReset( NEW Gfx::tFlashingTint( Gameplay_ToyBox_Tint_Entered, Gameplay_ToyBox_Tint_EnteredBlend, Gameplay_ToyBox_Tint_EnteredFlashRate ) );
	}
	void tGoalBoxLogic::fOnSpawn( )
	{
		tAnimatedBreakableLogic::fOnSpawn( );

		tEntity* gamerTag = NULL;

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			tEntity* child = fOwnerEntity( )->fChild( i ).fGetRawPtr( );
			if( child->fName( ) == cZone )
			{
				tShapeEntity* shape = child->fDynamicCast< tShapeEntity >( );
				sigassert( shape );
				mGoalZone.fReset( shape );
			}
			else if( child->fName( ) == cGamerTag )
			{
				gamerTag = child;
			}
		}

		if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			sigassert( gamerTag && "No gamertag spawn pt in goal box sigml!" );
			tPlayer* me = tGameApp::fInstance( ).fGetPlayerByTeam( fTeam( ) );
			sigassert( me );
			tPlayer* them = tGameApp::fInstance( ).fOtherPlayer( me );
			sigassert( them );

			Gui::tWorldSpaceFloatingText* text = them->fSpawnLocText( me->fUser( )->fGamerTag( ), gamerTag->fObjectToWorld( ).fGetTranslation( ), Gameplay_ToyBox_GamerTag_Tint, 1.f );
			sigassert( text );

			text->fSetTimeToLive( -1.f );
			text->fSetFloatSpeed( 0.f );

			mGamerTag.fReset( text );
		}
	}
	b32 tGoalBoxLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		return tAnimatedBreakableLogic::fHandleLogicEvent( e );
	}

	const tShapeEntityPtr& tGoalBoxLogic::fGetGoalZone( ) const
	{
		sigassert( mGoalZone && "Goal Box has no goal zone! Check the model has shape called zone." );

		return mGoalZone;
	}

	b32 tGoalBoxLogic::fCheckInBounds( tUnitLogic* logic ) const
	{
		const tObbf& box = fGetGoalZone( )->fBox( );
		tVec3f pos = logic->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		//if( logic->fUnitType( ) != GameFlags::cUNIT_TYPE_AIR )
		//	pos.y = box.mCenter.y; //y value always passes
		return box.fContains( pos );
	}
	void tGoalBoxLogic::fSomeoneEntered( )
	{
		mTintStack.fStack( )[ cToyBoxTintEntered ]->fSetActivationTime( Gameplay_ToyBox_Tint_EnteredDuration );
	}
	void tGoalBoxLogic::fActive( b32 active )
	{
		if( mActive != active )
		{
			// Play animation
			for( u32 i = 0; i < mPathEnds.fCount( ); ++i )
			{
				tWaypointLogic* waypointLogic = mPathEnds[ i ]->fLogicDerived<tWaypointLogic>( );
				sigassert( waypointLogic );
				waypointLogic->fSetAccessible( active );
			}
		}

		mActive = active;
	}

	void tGoalBoxLogic::fRegisterPathEnd( tEntity* endPoint )
	{
		tPathEntity* pe = endPoint->fDynamicCast<tPathEntity>( );

		if( pe )
			mPathEnds.fPushBack( tPathEntityPtr( pe ) );

		if( !mActive )
		{
			tWaypointLogic* waypointLogic = mPathEnds.fBack( )->fLogicDerived<tWaypointLogic>( );
			sigassert( waypointLogic );
			waypointLogic->fSetAccessible( false );
		}
	}
}

namespace Sig
{
	void tGoalBoxLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tGoalBoxLogic, tAnimatedBreakableLogic, Sqrat::NoCopy<tGoalBoxLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("Activate"), &tGoalBoxLogic::fActivate)
			.Func(_SC("Deactivate"), &tGoalBoxLogic::fDeactivate)
			.Func(_SC("IsActive"), &tGoalBoxLogic::fIsActive)
			;
		vm.fRootTable( ).Bind(_SC("GoalBoxLogic"), classDesc);
	}
}

