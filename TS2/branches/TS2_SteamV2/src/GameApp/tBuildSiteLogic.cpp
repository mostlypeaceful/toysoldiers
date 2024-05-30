#include "GameAppPch.hpp"
#include "tBuildSiteLogic.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tGameArchive.hpp"

namespace Sig
{
	struct tBuildSiteSaveData : public tEntitySaveData
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tBuildSiteSaveData, 0x1F5181B5 );
	public:
		tBuildSiteSaveData( )
			: mCapturedTeam( ~0 )
			, mCapturingTeam( ~0 )
			, mCapturingPercent( 0.0f )
			, mLocked( false )
		{
		}

		tBuildSiteSaveData( tBuildSiteLogic* buildSite, tSaveGameRewindPreview& preview )
		{
			mCapturedTeam = buildSite->fCapturedTeam( );
			mCapturingTeam = buildSite->fCapturingTeam( );
			mCapturingPercent = buildSite->fCapturingPercent( );
			mLocked = buildSite->fHasPrice( );
		}

		virtual void fRestoreSavedEntity( tEntity* entity ) const
		{
			tBuildSiteLogic* buildSite = entity->fLogic( )->fDynamicCast< tBuildSiteLogic >( );
			sigassert( buildSite );

			buildSite->fResetCapturingData( mCapturedTeam, mCapturingTeam, mCapturingPercent );
			buildSite->fSetLockedState( mLocked );
		}
		
		virtual void fSaveLoadDerived( tGameArchive& archive )
		{
			fSaveLoad( archive );
		}
		
		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			tEntitySaveData::fSaveLoadDerived( archive );
			archive.fSaveLoad( mCapturedTeam );
			archive.fSaveLoad( mCapturingTeam );
			archive.fSaveLoad( mCapturingPercent );
			archive.fSaveLoad( mLocked );
		}

	public:
		u32 mCapturedTeam;
		u32 mCapturingTeam;
		f32 mCapturingPercent;
		b32 mLocked;
	};

	register_rtti_factory( tBuildSiteSaveData, false );
}

namespace Sig
{

	devrgba_clamp( Gameplay_UnitTint_PlatformHighlight, Math::tVec4f( 0.75f, 0.75f, 0.75f, 1.f ), 0, 4.f, 2 );
	devvar( f32, Gameplay_Platforms_CaptureTime, 5.0f );

	using namespace Math;

	namespace
	{
		static const tStringPtr cCaptureName( "capture" );
		static const tStringPtr cAlliedCapturedLocKey( "alliedCaptured" );
		static const tStringPtr cEnemyCapturedLocKey( "enemyCaptured" );
		static const tStringPtr cBeginCapturingLocKey( "beginCapturing" );
		static const tStringPtr cDisputeCapturingLocKey( "disputeCapturing" );

		enum tBuildSiteTints
		{
			cCaptureTintIndex = tUnitLogic::cTintTypeCount,
			cCapturePulseTintIndex,
			cPurchaseTintIndex,
			cBuildSiteStackCount
		};

		static tFixedArray<tVec4f, GameFlags::cCOUNTRY_COUNT> cCaptureTints;
		static const tVec4f cUncapturedTint( 1, 0.5f, 0, 0 );


		class tBuildSiteCapturePulse : public Gfx::tFlashingTint
		{
		public:
			tBuildSiteCapturePulse( tBuildSiteLogic& logic, const tVec4f& tint, f32 blend, f32 rate )
				: Gfx::tFlashingTint( tint, blend, rate )
				, mLogic( logic )
				, mZeroLastFrame( true )
			{ }

			virtual b32 fStep( f32 dt ) 
			{
				b32 changed = false;

				if( mBlendStrength > cActiveThreshold )
				{					
					if( mLogic.fCapturingTeam( ) != ~0 )
					{
						mUserBlend = fMin( 1.f, mLogic.fCapturingPercent( ) );
						fSetSourceTint( mLogic.fCapturingColor( ) );
						changed = true;
						mZeroLastFrame = false;
					}
					else
					{
						mUserBlend = 0;
						changed = !mZeroLastFrame;
						mZeroLastFrame = true;
					}
				}

				if( Gfx::tFlashingTint::fStep( dt ) )
					changed = true;

				return changed;
			}

		private:
			tBuildSiteLogic&	mLogic;
			b32					mZeroLastFrame;
		};
	}

	tBuildSiteLogic::tBuildSiteLogic( )
		: mSize( GameFlags::cBUILD_SITE_SMALL )
		, mBuildSiteShape( NULL )
		, mCaptureShape( NULL )
		, mReserved( false )
		, mCapturedTeam( ~0 )
		, mCapturingTeam( ~0 )
		, mCapturingPercent( 0.f )
		, mLastPrice( 0.f )
	{
		mCaptureTime.fFill( 0.f );
		mCaptureTeamIn.fFill( false );
		mDontLightUpChildren = true;
	}

	void tBuildSiteLogic::fOnDelete( )
	{
		mCaptureProximity.fReleaseAllEntityRefsST( );
		if( mPrice )
			mPrice->fClear( );
		mPrice.fRelease( );
		tUnitLogic::fOnDelete( );
	}

	void tBuildSiteLogic::fOnSpawn( )
	{
		fOnPause( false );

		fOwnerEntity( )->fAddGameTags( GameFlags::cFLAG_SAVEABLE );

		tUnitLogic::fOnSpawn( );

		mTintStack.fStack( ).fSetCount( cBuildSiteStackCount );
		mTintStack.fStack( )[ cTintTypeSelected ]->fSetCurrentTint( Gameplay_UnitTint_PlatformHighlight );
		mTintStack.fStack( )[ cPurchaseTintIndex ].fReset( NEW Gfx::tSolidTint( Math::tVec4f(3,1,1,1), 0.2f ) );
		mTintStack.fStack( )[ cPurchaseTintIndex ]->fSetTargetBlendStrength( 0.f );

		mSize = ( GameFlags::tBUILD_SITE )fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_BUILD_SITE );		
		if( mSize == ~0 )
		{
			log_warning( 0, "Build site is missing Build Site enum!" );
			mSize = GameFlags::cBUILD_SITE_COUNT;
		}

		tEntity* owner = fOwnerEntity( );
		for( u32 i = 0; i < owner->fChildCount( ); ++i )
		{
			if( owner->fChild( i )->fQueryEnumValue( GameFlags::cENUM_BUILD_SITE ) != ~0 )
			{
				tShapeEntity* shape = owner->fChild( i )->fDynamicCast< tShapeEntity >( );
				if( shape )
					mBuildSiteShape = shape;
			}
			if( owner->fChild( i )->fName( ) == cCaptureName )
			{
				tShapeEntity* shape = owner->fChild( i )->fDynamicCast< tShapeEntity >( );
				if( shape )
					mCaptureShape = shape;
			}
		}

		if( mBuildSiteShape == 0 )
			log_warning( 0, "Build site is missing shape tagged Build Site" );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( level );

		if( level->fEnablePlatformLocking( ) && fSelectionEnabled( ) )
			fSetLockedState( true );
		else
			fSetLockedState( false );

		if( mCaptureShape )
		{
			mCaptureProximity.fSetRefreshFrequency( 0.25f, 0.1f );
			mCaptureProximity.fAddObb( mCaptureShape->fParentRelativeBox( ).fToAabb( ) );
			mCaptureProximity.fSetFilterByLogicEnts( true );
			mCaptureProximity.fLogicFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_UNIT_ID, GameFlags::cUNIT_ID_INFANTRY_OFFICER_01 ) );
			
			mTintStack.fStack( )[ cCaptureTintIndex ].fReset( NEW Gfx::tSolidTint( cUncapturedTint, 0.5f ) );
			mTintStack.fStack( )[ cCaptureTintIndex ]->fSetActivationTime( -1.f );
			mTintStack.fStack( )[ cCapturePulseTintIndex ].fReset( NEW tBuildSiteCapturePulse( *this, cUncapturedTint, 0.2f, 1.0f ) );

			cCaptureTints.fFill( tVec4f::cZeroVector );
			// Changing to country, for TS1 content (and trying to keep to BLUE and RED teams)
			cCaptureTints[ GameFlags::cCOUNTRY_USA ] = tVec4f( 1, 0, 0, 0 );
			cCaptureTints[ GameFlags::cCOUNTRY_USSR ] = tVec4f( 0, 0, 1, 0 );
			cCaptureTints[ GameFlags::cCOUNTRY_BRITISH ] = tVec4f( 0, 1, 0, 0 );
			cCaptureTints[ GameFlags::cCOUNTRY_GERMAN ] = tVec4f( 1, 1, 0, 0 );
			cCaptureTints[ GameFlags::cCOUNTRY_FRENCH ] = tVec4f( 1, 0, 1, 0 );

			if( mCapturedTeam != ~0 )
			{
				mTintStack.fStack( )[ cCaptureTintIndex ]->fSetCurrentTint( cCaptureTints[ mCapturedTeam ] );
				mTintStack.fStack( )[ cCaptureTintIndex ]->fSetActivationTime( -1.0f );
				mTintStack.fForceChanged( );
			}
		}
	}

	void tBuildSiteLogic::fOnPause( b32 paused )
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

	tRefCounterPtr<tEntitySaveData> tBuildSiteLogic::fStoreSaveGameData( b32 entityIsPartOfLevelFile, tSaveGameRewindPreview& preview )
	{
		return tRefCounterPtr<tEntitySaveData>( NEW tBuildSiteSaveData( this, preview ) );
	}

	void tBuildSiteLogic::fResetCapturingData( u32 capturedTeam, u32 capturingTeam, f32 capturingPercent )
	{
		mCapturedTeam = capturedTeam;
		mCapturingTeam = ~0;
		mCapturingPercent = 0.0f;
	}

	Gui::tRadialMenuPtr tBuildSiteLogic::fCreateSelectionRadialMenu( tPlayer& player )
	{
		Gui::tRadialMenuPtr radialMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptBuildSiteOptions ), player.fUser( ), player.fGameController( ) ) );

		Sqrat::Object params( Sqrat::Table( ).SetValue( "Unit", this ).SetValue( "Player", &player ) );
		Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "DefaultSetup" ).Execute( params );

		return radialMenu;
	}

	void tBuildSiteLogic::fActST( f32 dt )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level && mLastPrice != level->fPlatformPrice( ) )
			fSetPrice( level->fPlatformPrice( ) );
		
		tLogic::fActST( dt );

		if( mCaptureShape )
		{
			mCaptureProximity.fCleanST( );
			fProcessCapturing( dt );
		}

		fStepTintStack( dt );
	}

	b32 tBuildSiteLogic::fIsUsableBy( u32 team ) const
	{
		return !fCapturable( ) || mCapturedTeam == team;
	}

	b32 tBuildSiteLogic::fIsValid( u32 team ) const
	{
		b32 validTeam = mTeam == GameFlags::cTEAM_NONE || mTeam == team;
		validTeam = validTeam && fIsUsableBy( team );
		return validTeam && mSize != GameFlags::cBUILD_SITE_COUNT && !fIsOccupied( ) && !mPrice;
	}

	b32 tBuildSiteLogic::fIsOccupied( ) const
	{
		return !( mBuildSiteShape && mBuildSiteShape->fChildCount( ) == 0 ) || mReserved;
	}

	tEntity* tBuildSiteLogic::fUnit( )
	{
		if( mBuildSiteShape->fChildCount( ) == 0 )
			return NULL;
		else
			return mBuildSiteShape->fChild( 0 ).fGetRawPtr( );
	}

	b32 tBuildSiteLogic::fTryToUse( tPlayer* player )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level && player->fAttemptPurchase( (s32)level->fPlatformPrice( ) ) )
		{
			level->fPlatformPurchased( fOwnerEntity( ) );
			fSetLockedState( false );
			return true;
		}
		else
			return false;
	}

	void tBuildSiteLogic::fSetPrice( f32 price )
	{
		if( mPrice )
		{
			mLastPrice = price;
			const Gui::tWorldSpaceText::tTextArray& textObj = mPrice->fAccessText( );

			for( u32 i = 0; i < textObj.fCount( ); ++i )
			{
				tLocalizedString moneyString = tLocalizedString::fConstructMoneyString( StringUtil::fToString( fRound<u32>( price ) ).c_str( ) );
				textObj[ i ].mText->fBake( moneyString, Gui::tText::cAlignCenter );
				textObj[ i ].mText->fSetAlpha( 1.f );
			}
		}
	}

	void tBuildSiteLogic::fSetLockedState( b32 locked )
	{
		if( !locked )
		{
			if( mTintStack.fHasIndex( cPurchaseTintIndex ) )
				mTintStack.fStack( )[ cPurchaseTintIndex ]->fSetTargetBlendStrength( 0.f );
			
			fOwnerEntity( )->fRemoveGameTags( GameFlags::cFLAG_SELECTABLE );
			sigassert( !fSelectionEnabled( ) );

			if( mPrice )
			{
				mPrice->fClear( );
				mPrice->fDelete( );
			}
			mPrice.fRelease( );
		}
		else
		{
			if( mPrice )
			{
				mPrice->fClear( );
				mPrice->fDelete( );
			}
			mPrice.fRelease( );

			mPrice.fReset( NEW Gui::tWorldSpaceText( ) );
			mPrice->fSpawn( *fOwnerEntity( ) );
			Math::tMat3f mat = Math::tMat3f::cIdentity;
			mat.fSetTranslation( Math::tVec3f( 0,2,0 ) );
			mPrice->fSetParentRelativeXform( mat );

			const tUserArray& localUsers = tGameApp::fInstance( ).fLocalUsers( );
			for( u32 i = 0; i < localUsers.fCount( ); ++i )
			{
				std::stringstream ss;
				ss << "viewport" << localUsers[ i ]->fViewportIndex( );
				mPrice->fAdd( tGameApp::fInstance( ).fLocFont( tGameApp::cFontSimpleSmall ), *localUsers[ i ], tGameApp::fInstance( ).fHudLayer( ss.str( ) ).fToCanvasFrame( ) );
			}
			//mPrice->fCreate( tGameApp::fInstance( ).fLocFont( tGameApp::cFontSimpleSmall ), tGameApp::fInstance( ).fLocalUsers( ), tGameApp::fInstance( ).fRootCanvas( ) );

			fSetPrice( 20 );

			if( mTintStack.fHasIndex( cPurchaseTintIndex ) )
				mTintStack.fStack( )[ cPurchaseTintIndex ]->fSetActivationTime( -1.f );
		}
	}

	void tBuildSiteLogic::fProcessCapturing( f32 dt )
	{
		if( mCapturingPercent > 0.0f && mCapturingTeam != ~0 && mCapturedTeam != mCapturingTeam )
		{
			if( Gameplay_Platforms_CaptureTime != 0.0f )
				mCapturingPercent += dt / Gameplay_Platforms_CaptureTime;
			tPlayer* player = tGameApp::fInstance( ).fGetPlayerByTeam( mCapturingTeam );
			if( player->fPointCaptureUI( ) )
				player->fPointCaptureUI( )->fSetPercent( mCapturingTeam, mCapturingPercent );
		}

		if( mCaptureProximity.fRefreshMT( dt, *fOwnerEntity( ) ) )
		{
			u32 teamsIn = 0;

			// process capturing
			if( fIsOccupied( ) )
			{
				// capture status can't change while theres a unit on it
				mCaptureTeamIn.fFill( false );
				mCaptureTime.fFill( 0.f );
			}	
			else
			{
				// look for new capturer
				mCaptureTeamIn.fFill( false );

				for( u32 i = 0; i < mCaptureProximity.fEntityCount( ); ++i )
				{
					tUnitLogic* unit = mCaptureProximity.fEntityList( )[ i ]->fLogicDerived<tUnitLogic>( );
					if( unit && !mCaptureTeamIn[ unit->fTeam( ) ] && mCaptureShape->fContains( unit->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) ) )
					{
						++teamsIn;
						mCaptureTeamIn[ unit->fTeam( ) ] = true;
					}
				}

				if( teamsIn > 1 && mCapturingTeam != ~0 )
				{
					// disputed
					tPlayer* player = tGameApp::fInstance( ).fGetPlayerByTeam( mCapturingTeam );
					//tGameApp::fInstance( ).fCurrentLevelDemand( )->fImpactText( cDisputeCapturingLocKey, *player );
					if( player->fPointCaptureUI( ) )
						player->fPointCaptureUI( )->fDisputed( true );
				}
			}

			u32 newCaptureTeam = ~0;
			f32 highestCaptureTime = 0.f;
			mCapturingPercent = 0.f;

			for( u32 i = 0; i < mCaptureTeamIn.fCount( ); ++i )
			{
				if( teamsIn == 1 && mCaptureTeamIn[ i ] )
				{
					mCaptureTime[ i ] += mCaptureProximity.fRefreshFrequency( );
					if( mCaptureTime[ i ] > highestCaptureTime )
					{
						highestCaptureTime = mCaptureTime[ i ];
						newCaptureTeam = i;
						mCapturingPercent = highestCaptureTime / Gameplay_Platforms_CaptureTime;
					}
				}
				else
					mCaptureTime[ i ] = 0.f;
			}

			if( newCaptureTeam != ~0 && newCaptureTeam == mCapturedTeam )
				newCaptureTeam = ~0;

			if( newCaptureTeam != mCapturingTeam )
			{
				// Started capturing
				mCapturingTeam = newCaptureTeam;
				if( mCapturingTeam != ~0 )
				{
					tPlayer* player = tGameApp::fInstance( ).fGetPlayerByTeam( mCapturingTeam );
					//tGameApp::fInstance( ).fCurrentLevelDemand( )->fImpactText( cBeginCapturingLocKey, *player );
					//player->fPointCaptureUI( )->fShow( true );
				}
			}

			/*if( mCapturingPercent > 0.0f && mCapturingTeam != ~0 )
			{
				tPlayer* player = tGameApp::fInstance( ).fGetPlayerByTeam( mCapturingTeam );
				player->fPointCaptureUI( )->fSetPercent( mCapturingTeam, mCapturingPercent );
			}*/

			if( mCapturingTeam != ~0 )
			{
				mCapturingColor = cCaptureTints[ mCapturingTeam ];
				mTintStack.fStack( )[ cCapturePulseTintIndex ]->fSetActivationTime( 0.5f );
			}

			if( mCapturingTeam != ~0 && highestCaptureTime > Gameplay_Platforms_CaptureTime )
			{
				// Finish Capturing
				mCapturedTeam = mCapturingTeam;
				fSetCurrentTint( mCapturingColor );

				tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
				for( u32 i = 0; i < players.fCount( ); ++i )
				{
					b32 captor = (players[ i ]->fTeam( ) == mCapturingTeam);

					if( captor )
						players[ i ]->fStats( ).fIncStat( GameFlags::cSESSION_STATS_VERSUS_PLATFORMS_CAPTURED, 1.f );

					const tStringPtr& locKey = captor ? cAlliedCapturedLocKey : cEnemyCapturedLocKey;
					//tGameApp::fInstance( ).fCurrentLevelDemand( )->fImpactText( locKey, *players[ i ] );

					if( players[ i ]->fPointCaptureUI( ) )
					{
						if( captor )
							players[ i ]->fPointCaptureUI( )->fCapture( mCapturingTeam );
						else
							players[ i ]->fPointCaptureUI( )->fLost( mCapturingTeam );
					}
				}
			}
		}
	}

	void tBuildSiteLogic::fSetCurrentTint( const Math::tVec4f& tint )
	{
		if( mTintStack.fHasIndex( cCaptureTintIndex ) )
			mTintStack.fStack( )[ cCaptureTintIndex ]->fSetCurrentTint( tint );
		mTintStack.fForceChanged( );
	}
}

namespace Sig
{
	void tBuildSiteLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tBuildSiteLogic, tUnitLogic, Sqrat::NoCopy<tBuildSiteLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("TryToUse"), &tBuildSiteLogic::fTryToUse)
			.Prop(_SC("Occupied"), &tBuildSiteLogic::fIsOccupied)
			.Prop(_SC("Unit"), &tBuildSiteLogic::fUnit)
			;
		vm.fRootTable( ).Bind(_SC("BuildSiteLogic"), classDesc);
	}
}

