#include "GameAppPch.hpp"
#include "tBatteryCharger.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tVehicleLogic.hpp"

namespace Sig
{

	using namespace Math;

	devvar( f32, Gameplay_BatteryCharger_LightsDormant, 0.1f );
	devvar( f32, Gameplay_BatteryCharger_LightsActive, 2.0f );
	devvar( f32, Gameplay_BatteryCharger_BarDelta, 0.5f );
	devvar( f32, Gameplay_BatteryCharger_RefillRate, 0.5f );
	devvar( f32, Gameplay_BatteryCharger_TextHeight, 5.0f );

	namespace
	{
		static const tStringPtr cSpawnPt( "spawnPt" );
		static const tStringPtr cSpawnVehicleAnimEvent( "spawn_vehicle" );	
		static const tStringPtr cRedLight( "redLight" );
		static const tStringPtr cGreenLight( "greenLight" );
		static const tStringPtr cBatteryBar( "batteryBar" );
		const f32 cMaxCharge = 0.5f; //texture mapping specific
		const f32 cThresh = 0.98f;

		static const tStringPtr cAvailable( "bat_charage_avail" );
	}

	tBatteryCharger::tBatteryCharger( )
		: mShape( NULL )
		, mVehicle( NULL )
		, mBarProgress( 0.5f )
		, mTargetBarProgress( 0.f )
		, mHasPurchased( 0 )
	{
	}

	void tBatteryCharger::fOnDelete( )
	{
		mVehicle = NULL;
		mVehicleEnt.fRelease( );
		mProximity.fReleaseAllEntityRefsST( );
		mHasPurchasedEnt.fRelease( );
		mPurchaser.fRelease( );
		mSpawnPt.fRelease( );
		mText.fRelease( );

		tAnimatedBreakableLogic::fOnDelete( );
	}

	void tBatteryCharger::fOnSpawn( )
	{
		fOnPause( false );

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			const tEntityPtr& e = fOwnerEntity( )->fChild( i );

			tShapeEntity* se = e->fDynamicCast<tShapeEntity>( );
			if( se )
				mShape = se;
			
			if( e->fName( ) == cSpawnPt )
				mSpawnPt = e;
		}

		if( !mSpawnPt )
			mSpawnPt.fReset( fOwnerEntity( ) );

		if( mShape )
		{
			mProximity.fSetRefreshFrequency( 0.5f, 0.1f );
			mProximity.fAddObb( mShape->fParentRelativeBox( ).fToAabb( ) );
			mProximity.fSetFilterByLogicEnts( true );
			mProximity.fLogicFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_LOGIC_TYPE, GameFlags::cLOGIC_TYPE_VEHICLE ) );
		}

		// shape should be found before automatic shape is created
		tAnimatedBreakableLogic::fOnSpawn( );

		if( fIsPurchasePlatform( ) )
		{
			if( tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).fChallengeVehicles( ) )
			{
				sigassert( fCountry( ) < GameFlags::cCOUNTRY_COUNT && fCountry( ) != GameFlags::cCOUNTRY_DEFAULT && "Selectable battery chargers need a country" );
				tPlayer* player = tGameApp::fInstance( ).fGetPlayerByTeam( fTeam( ) );
				sigassert( player );

				mText.fReset( NEW Gui::tWorldSpaceFloatingText( player->fUser( ), tGameApp::cFontSimpleSmall ) );
				mText->fSpawn( *fOwnerEntity( ) );

				tMat3f xform = tMat3f::cIdentity;
				xform.fSetTranslation( tVec3f( 0, Gameplay_BatteryCharger_TextHeight, 0 ) );
				mText->fSetParentRelativeXform( xform );
				mText->fSetTimeToLive( -1.0f );
				mText->fSetFloatSpeed( 0.f );
				fApplyText( );
			}
			else
				fOwnerEntity( )->fRemoveGameTags( GameFlags::cFLAG_SELECTABLE );
		}

		mTakesDamage = 0;
	}

	void tBatteryCharger::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListAnimateMT );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListAnimateMT );
			fRunListInsert( cRunListCoRenderMT );
		}
	}

	void tBatteryCharger::fSetVehicle( tVehicleLogic* veh )
	{
		if( mVehicle != veh )
		{
			if( mVehicle )
				mVehicle->fHideShowBToExit( false );

			mVehicle = veh;
			mVehicleEnt.fReset( veh ? veh->fOwnerEntity( ) : NULL );

			if( mVehicle && !fIsPurchasePlatform( ) )
				mVehicle->fHideShowBToExit( true );
		}
	}

	void tBatteryCharger::fApplyText( )
	{
		sigassert( mText );
		
		tStringPtr str;

		if( !mHasPurchased )
			str = cAvailable;

		if( str.fExists( ) )
		{
			mText->fSetText( tGameApp::fInstance( ).fLocString( str ).fToCString( ).c_str( ) );
			mText->fFadeIn( );
		}
		else
			mText->fFadeOut( );
	}

	b32 tBatteryCharger::fShouldShowStatus( tVehicleLogic* vehicle ) const
	{
		return (vehicle->fHasWaitTimer( ) || vehicle->fPowerLevel( ) > cThresh );
	}

	void tBatteryCharger::fMoveST( f32 dt )
	{
		mProximity.fCleanST( );

		if( mVehicle )
		{
			if( mVehicle->fIsDestroyed( ) || !mVehicleEnt->fSceneGraph( ) )
			{
				fSetVehicle( NULL );
				mTargetBarProgress = 0.f;
			}
			else
			{
				if( !mVehicle->fUnderUserControl( ) && !fIsPurchasePlatform( ) )
				{
					if( mVehicle->fChargeIfNotCharging( ) )
					{
						tTutorialEvent event( GameFlags::cTUTORIAL_EVENT_BEGIN_CHARGING );
						event.mEntity = fOwnerEntity( );
						event.mPlayerKillerLogic = mVehicle;

						tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
						if( level )
							level->fHandleTutorialEvent( event );
					}
				}

				mTargetBarProgress = mVehicle->fPowerLevel( );
			}
		}
		else
			mTargetBarProgress = 0.f;

		if( mHasPurchasedEnt && !mHasPurchasedEnt->fSceneGraph( ) )
		{
			mHasPurchased = 0;
			mHasPurchasedEnt.fRelease( );
			fApplyText( );
		}

		f32 barDelta = mTargetBarProgress - mBarProgress;
		f32 barChange = fSign( barDelta ) * fMin( Gameplay_BatteryCharger_BarDelta * dt, fAbs( barDelta ) );
		mBarProgress += barChange;

		//animated breakable calls act in move st :( ghetto! but optimized :
		tAnimatedBreakableLogic::fMoveST( dt );

		// debug rendering
		//fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tObbf( mShape->fParentRelativeBox( ).fToAabb( ), fOwnerEntity( )->fObjectToWorld( ) ), tVec4f(1,0,0,0.5f) );
	}

	void tBatteryCharger::fCoRenderMT( f32 dt )
	{		
		if( mShape && mProximity.fRefreshMT( dt, *fOwnerEntity( ) ) )
		{
			tVehicleLogic* newVehicle = NULL;
			for( u32 i = 0; i < mProximity.fEntityCount( ); ++i )
			{
				tVehicleLogic* vehicle = mProximity.fGetEntity( i )->fLogicDerived<tVehicleLogic>( );
				if( vehicle && mShape->fContains( vehicle->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) ) )
				{
					if( vehicle == mVehicle || !newVehicle )
						newVehicle = vehicle;
				}
			}

			fSetVehicle( newVehicle );
		}

		tAnimatedBreakableLogic::fCoRenderMT( dt );
	}

	Math::tVec4f tBatteryCharger::fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const
	{
		static const tStringPtr cRedLight( "redLight" );
		static const tStringPtr cGreenLight( "greenLight" );
		static const tStringPtr cBatteryBar( "batteryBar" );

		b32 green = mVehicle && mVehicle->fPowerLevel( ) >= cThresh;
		b32 red = mVehicle && mVehicle->fPowerLevel( ) < cThresh && fShouldShowStatus( mVehicle );

		if( varName == cGreenLight )
			return green ? tVec4f( Gameplay_BatteryCharger_LightsActive ) : tVec4f( Gameplay_BatteryCharger_LightsDormant );
		else if( varName == cRedLight )
			return red ? tVec4f( Gameplay_BatteryCharger_LightsActive ) : tVec4f( Gameplay_BatteryCharger_LightsDormant );
		else if( varName == cBatteryBar )
			return tVec4f( mBarProgress * cMaxCharge );

		return tAnimatedBreakableLogic::fQueryDynamicVec4Var( varName, viewportIndex );
	}

	b32 tBatteryCharger::fPurchaseVehicle( tPlayer* player, u32 unitID, u32 price )
	{
		if( !mHasPurchased && player->fAttemptPurchase( price ) )
		{
			player->fStats( ).fIncStat( GameFlags::cSESSION_STATS_VEHICLES_PURCHASED, 1.f );
			mHasPurchased = unitID;
			mPurchaser.fReset( player );
			fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_REAPPLY_ONESHOT_MOTION_STATE ) );
			return true;
		}

		return false;
	}

	Gui::tRadialMenuPtr tBatteryCharger::fCreateSelectionRadialMenu( tPlayer& player )
	{
		if( !mHasPurchased )
		{
			Gui::tRadialMenuPtr radialMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptPurchaseVehicleOptions ), player.fUser( ), player.fGameController( ) ) );

			Sqrat::Object params( Sqrat::Table( ).SetValue( "Unit", this ).SetValue( "Player", &player ) );
			Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "DefaultSetup" ).Execute( params );

			return radialMenu;
		}
		else
			return Gui::tRadialMenuPtr( );
	}

	void tBatteryCharger::fActuallySpawn( )
	{
		sigassert( mHasPurchased );

		tEntity* newEnt = tGameApp::fInstance( ).fCurrentLevelDemand( )->fOwnerEntity( )->fSpawnChild( tGameApp::fInstance( ).fUnitResourcePath( mHasPurchased, fCountry( ) ) );
		sigassert( newEnt );
		mHasPurchasedEnt.fReset( newEnt );
		mHasPurchasedEnt->fAddGameTags( GameFlags::cFLAG_SELECTABLE );
		mHasPurchasedEnt->fMoveTo( mSpawnPt->fObjectToWorld( ) );
		fApplyText( );

		tVehicleLogic* vehicle = newEnt->fLogicDerived<tVehicleLogic>( );
		sigassert( vehicle );
		vehicle->fSetDontRespawn( true );
		vehicle->fSetPurchasedBy( mPurchaser.fGetRawPtr( ) );
	}

	b32 tBatteryCharger::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_ANIMATION:
			{
				const tKeyFrameEventContext* context = e.fContext<tKeyFrameEventContext>( );
				if( context && context->mTag == cSpawnVehicleAnimEvent )
				{
					fActuallySpawn( );
					return true;
				}
			}
		}

		return tAnimatedBreakableLogic::fHandleLogicEvent( e );
	}
}

namespace Sig
{
	void tBatteryCharger::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tBatteryCharger, tAnimatedBreakableLogic, Sqrat::NoCopy<tBatteryCharger> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("PurchaseVehicle"),	&tBatteryCharger::fPurchaseVehicle)
			;
		vm.fRootTable( ).Bind(_SC("BatteryChargerLogic"), classDesc);
	}
}

