#include "GameAppPch.hpp"
#include "tBarrageController.hpp"
#include "tLevelLogic.hpp"

namespace Sig
{

	const f32 tBarrageController::cSpinTime = 3.0f;
	const tBarragePtr tBarrageController::cNullBarrage;

	tBarrageController::tBarrageController( tPlayer* player )
		: mCurrentBarrageIndex( -1 )
		, mBarrageState( BARRAGE_STATE_NONE )
		, mBarrageMeterPercent( 0.0f )
		, mSpinTimer( 0.0f )
		, mPlayer( player )
		, mPreventDuplicateBarrages( false )
	{
		mBarrageUI.fReset( NEW Gui::tBarrageUI( mPlayer->fUser( ) ) );
		mBarrages.fSetCount( 0 );
	}

	tBarrageController::~tBarrageController( )
	{
		fOnDelete( );
	}

	void tBarrageController::fOnDelete( )
	{
		if( mBarrageUI )
		{
			mBarrageUI->fCanvas( ).fDeleteSelf( );
			mBarrageUI.fRelease( );
		}
	}

	void tBarrageController::fAddBarrage( tBarragePtr& barrage )
	{
		mBarrages.fPushBack( barrage );
	}

	void tBarrageController::fSetBarrageMeterValue( f32 newValue )
	{
		mBarrageMeterPercent = newValue;

		switch( mBarrageState )
		{
		case BARRAGE_STATE_ACTIVE:
		case BARRAGE_STATE_IN_USE:
			{
				if( mBarrageUI ) 
					mBarrageUI->fUpdateTimer( newValue );

				if( newValue <= 0.f )
				{
					fEndBarrage( );
					return;
				}

				const b32 barrageUsable = mCurrentBarrage.fCodeObject( )->fBarrageUsable( ) && !mPlayer->fDisableYAccessDueToFocus( );
				if( mBarrageState != BARRAGE_STATE_IN_USE && barrageUsable )
				{
					mBarrageState = BARRAGE_STATE_IN_USE;
					if( mBarrageUI )
						mBarrageUI->fSetUsable( true );
				}
				else if( mBarrageState == BARRAGE_STATE_IN_USE && !barrageUsable )
				{
					mBarrageState = BARRAGE_STATE_ACTIVE;
					if( mBarrageUI )
						mBarrageUI->fSetUsable( false );
				}

				break;
			}

		case BARRAGE_STATE_NONE:
			{
				break;
			}
		} 
	}

	void tBarrageController::fStepBarrage( f32 dt )
	{
		switch( mBarrageState )
		{
		case BARRAGE_STATE_SPINNING:
			{
				mSpinTimer -= dt;

				if( mSpinTimer <= 0.0f )
				{
					mBarrageState = BARRAGE_STATE_AVAILABLE;
					if( mCurrentBarrageIndex >= 0 )
					{
						if( mBarrageUI )
							mBarrageUI->fSetAvailable( );

						fCurrentBarrage( ).fCodeObject( )->fSelected( mPlayer );

						tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
						if( level )
						{
							tTutorialEvent event( GameFlags::cTUTORIAL_EVENT_BARRAGE_RECEIVED );

							event.mWeaponID = fCurrentBarrage( ).fCodeObject( )->mDevName;
							event.mPlayer = mPlayer;
							if( mPlayer->fCurrentUnit( ) )
								event.mCurrentUnitID = mPlayer->fCurrentUnit( )->fUnitID( );

							level->fHandleTutorialEvent( event );
						}
					}

				}

				break;
			}

		case BARRAGE_STATE_ACTIVE:
		case BARRAGE_STATE_IN_USE:
			{
				f32 newMeterValue = 1.f - mCurrentBarrage.fProcessST( mPlayer, dt );
				fSetBarrageMeterValue( newMeterValue );
				break;
			}

		case BARRAGE_STATE_AVAILABLE:
			{
				if( !mPlayer->fDisableYAccessDueToFocus( ) && mPlayer->fGamepad( ).fButtonDown( Input::tGamepad::cButtonY ) )
				{
					if( !mCurrentBarrage.fIsNull( ) )
					{
						mBarrageState = BARRAGE_STATE_ACTIVE;
						mRestrictBarrage = tStringPtr::cNullPtr;

						if( mBarrageUI )
							mBarrageUI->fBegin( );
						mCurrentBarrage.fBegin( mPlayer );

						tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
						if( level )
						{
							tTutorialEvent event( GameFlags::cTUTORIAL_EVENT_BARRAGE_ACTIVATED );
							event.mWeaponID = fCurrentBarrage( ).fCodeObject( )->mDevName;
							event.mPlayer = mPlayer;
							level->fHandleTutorialEvent( event );
						}

						mPlayer->fStats( ).fIncStat( GameFlags::cSESSION_STATS_BARRAGES_CALLED_IN, 1 );
						mPlayer->fStats( ).fIncStat( GameFlags::cSESSION_STATS_MOST_BARRAGES_IN_A_SINGLE_MAP, 1 );
					}
				}
				break;
			}
		}
	}

	void tBarrageController::fEndBarrage( )
	{
		if( fBarrageActive( ) )
		{
			if( mPlayer->fStats( ).fBarrageAvailable( mPlayer->fCurrentUnit( ) ) ) //this prevents continuously barraging
				mBarrageState = BARRAGE_STATE_WAITING_FOR_COMBO_LOSS;
			else
				mBarrageState = BARRAGE_STATE_NONE;

			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			if( level )
			{
				tTutorialEvent event( GameFlags::cTUTORIAL_EVENT_BARRAGE_ENDED );
				event.mWeaponID = fCurrentBarrage( ).fCodeObject( )->mDevName;
				level->fHandleTutorialEvent( event );
			}

			mCurrentBarrage.fReset( mPlayer );
			mCurrentBarrage = tBarragePtr( );
			mCurrentBarrageIndex = -1;

			if( mBarrageUI ) 
				mBarrageUI->fEnd( );
			mPlayer->fBarrageHasEnded( );
		}
	}

	void tBarrageController::fComboLost( )
	{
		if( mBarrageState == BARRAGE_STATE_WAITING_FOR_COMBO_LOSS )
		{
			mBarrageState = BARRAGE_STATE_NONE;
		}
	}

	u32 tBarrageController::fNextBarrageIndex( tPlayer* coopPlayer )
	{
		u32 index = sync_rand( fIntInRange( 0, mBarrages.fCount( ) - 1 ) );

		if( mRestrictBarrage.fExists( ) )
		{
			for( u32 i = 0; i < mBarrages.fCount( ); ++i )
			{
				if( mBarrages[ i ].fCodeObject( )->mDevName == mRestrictBarrage )
				{
					index = i; 
					mUniqueBarragesRemaining.fFindAndErase( index );
					break;
				}
			}
		}
		else if( mPreventDuplicateBarrages && mUniqueBarragesRemaining.fCount( ) )
		{
			// this is essentially trial/demo mode.
			u32 rid = Math::fModulus( index, mUniqueBarragesRemaining.fCount( ) );
			index = mUniqueBarragesRemaining[ rid ];
			mUniqueBarragesRemaining.fErase( rid );
		}
		else
		{
			mPreventDuplicateBarrages = false;

			// If there is a coop player and more than one available barrage
			if( coopPlayer && mBarrages.fCount( ) > 1 )
			{
				// Make sure this player gets a different barrage than the coop player
				const tBarragePtr& coopBarrage = coopPlayer->fCurrentBarrage( );

				// this has to be done by name because the arrays may be different.
				if( !coopBarrage.fIsNull( ) && coopBarrage.fCodeObject( )->mDevName == mBarrages[ index ].fCodeObject( )->mDevName )
				{
					u32 shift = sync_rand( fIntInRange( 1, mBarrages.fCount( ) - 1 ) );
					index = Math::fModulus( index + shift, mBarrages.fCount( ) );
				}
			}
		}

		return index;
	}

	void tBarrageController::fGiveBarrage( tPlayer* coopPlayer, b32 skipInto, b32 forTutorial )
	{
		if( mBarrageState == BARRAGE_STATE_NONE )
		{
			// Choose new barrage
			if( mBarrages.fCount( ) > 0 )
			{
				// Random Barrage from available barrages
				mCurrentBarrageIndex = fNextBarrageIndex( coopPlayer );

				mRestrictBarrage = tStringPtr::cNullPtr;
				mCurrentBarrage = mBarrages[ mCurrentBarrageIndex ];
				mCurrentOrLastBarrage = mCurrentBarrage;
				mBarrageState = BARRAGE_STATE_SPINNING;
				mSpinTimer = cSpinTime;

				if( forTutorial )
					mCurrentBarrage.fCodeObject( )->fForTutorial( );

				if( mBarrageUI ) 
					mBarrageUI->fStartSpinning( *mCurrentBarrage.fCodeObject( ), cSpinTime, skipInto );

			}
		}
	}

	void tBarrageController::fRestrictBarrage( const tStringPtr& name )
	{
		mRestrictBarrage = name;
	}

	void tBarrageController::fSkipSpinAndEnter( const tStringPtr& name, tEntity* target )
	{
		if( mBarrageState == BARRAGE_STATE_NONE )
		{
			fRestrictBarrage( name );
			fGiveBarrage( NULL, true, true );

			// do this before fStartSpinning so it can disable audio.
			mCurrentBarrage.fCodeObject( )->fSetTarget( mPlayer, target );
			mCurrentBarrage.fCodeObject( )->fSkipInto( mPlayer );
			mCurrentBarrage.fBegin( mPlayer ); 
			mBarrageState = BARRAGE_STATE_ACTIVE;

			if( mBarrageUI )
			{
				mBarrageUI->fSetAvailable( );
				mBarrageUI->fBegin( );
			}
		}
	}

	void tBarrageController::fShowUI( b32 show )
	{
		if( mBarrageUI )
			mBarrageUI->fShow( show );
	}

	const tBarragePtr& tBarrageController::fCurrentBarrage() const
	{
		return mCurrentBarrage;
	}

	void tBarrageController::fBeginNoRepeats( )
	{
		mUniqueBarragesRemaining.fSetCount( mBarrages.fCount( ) );
		for( u32 i = 0; i < mUniqueBarragesRemaining.fCount( ); ++i )
			mUniqueBarragesRemaining[ i ] = i;
		mPreventDuplicateBarrages = (mUniqueBarragesRemaining.fCount( ) != 0);
	}

	void tBarrageController::fEndNoRepeats( )
	{
		mPreventDuplicateBarrages = false;
	}

	void tBarrageController::fReactivateBarrage( s32 index )
	{
		if( index == -1 )
			return;

		mCurrentBarrageIndex = index;

		mRestrictBarrage = tStringPtr::cNullPtr;
		mCurrentBarrage = mBarrages[ mCurrentBarrageIndex ];
		mCurrentOrLastBarrage = mCurrentBarrage;
		mBarrageState = BARRAGE_STATE_AVAILABLE;

		if( mBarrageUI ) 
		{
			mBarrageUI->fStartSpinning( *mCurrentBarrage.fCodeObject( ), cSpinTime, false );
			mBarrageUI->fSetAvailable( );
		}
	}

}
