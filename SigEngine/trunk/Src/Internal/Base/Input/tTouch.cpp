#include "BasePch.hpp"
#include "tTouch.hpp"

namespace Sig { namespace Input
{
	devvar( bool, Input_Spam_Touch, false );
	devvar( u32, Input_Tweak_DragThreshold, 20 );
	devvar( u32, Input_Tweak_Touch_DoubleTapTime, 10 );
	devvar( bool, Input_Workaround_DetectBadFinger, false );
	devvar_clamp( f32, Input_Tweak_Touch_DoubleTapDistance    , 10.0f, 10.0f, 100.0f, 0 );
	devvar_clamp( f32, Input_Workaround_BadFingerTimeout,  0.5f,  0.2f,  10.0f, 1 );
	devvar_clamp( f32, Input_Workaround_BadFingerHotzone,  1.8f,  0.0f,  10.0f, 1 );

	namespace
	{
		struct tBadFingerUpEntry
		{
			tTouch::tFinger mFinger;
			f32 mAge;

			b32 fTooOld() const
			{
				return mAge >= Input_Workaround_BadFingerTimeout;
			}
		};

		struct tFingerTooOld
		{
			b32 operator()( const tBadFingerUpEntry& entry ) const
			{
				return entry.fTooOld();
			}
		};

		tGrowableArray<tBadFingerUpEntry> gBadFingerUps;

		u32 fMinHistory()
		{
			return fMax<u32>( 2u, Input_Tweak_Touch_DoubleTapTime );
		}
	}

	const tTouch tTouch::cNullTouch;

	tTouch::tTouch( )
		: mStateHistory( 2 )
		, mWindowHandle( 0 )
	{
		tTouchStateData dummyData;
		tStateData dummyStateData;
		dummyStateData.fFill( dummyData );
		mStateHistory.fFill( dummyStateData );
	}

	tTouch::~tTouch( )
	{
		fShutdown( );
	}

	void tTouch::fCaptureState( f32 dt )
	{
		for( u32 i=0 ; i<gBadFingerUps.fCount( ) ; ++i )
		{
			tBadFingerUpEntry& entry = gBadFingerUps[i];
			tTouchStateData& data = mRawState[entry.mFinger];
			entry.mAge += dt;

			if( entry.fTooOld() )
			{
				data.mFlags.fTurnBitOff( tTouch::cFlagFingerHeld );
				data.mFlags.fTurnBitOn( tTouch::cFlagFingerUpEvent );
				if( !data.mFlags.fGetBit( tTouch::cFlagFingerDragging ) )
				{
					data.mFlags.fTurnBitOn( tTouch::cFlagFingerTapEvent );
					// TODO: Double tap?
				}
				log_warning( "Evicting a bad finger up, timeout too low or was false positive (finger really did go up)" );
			}
		}
		gBadFingerUps.fEraseAllOrderedWhere( tFingerTooOld() );

		tStateData stateData;
		fCaptureStateUnbuffered( stateData, dt );
		if( mStateHistory.fCapacity( ) < fMinHistory( ) )
			fSetHistorySize( fMinHistory( ) );
		mStateHistory.fPushLast( stateData );
		fStepDrag(dt);
		fClearEventFlags( mRawState );

		if( Input_Spam_Touch )
		{
			static int accum = 0;
			accum += 1;
			if ( accum!=4 ) return;
			accum -= 4;

			if( !( fFingerUp(tTouch::cFingerAny) || fFingerDragging(tTouch::cFingerAny) || fFingerTapped(tTouch::cFingerAny) ) )
				return;
			log_line( 0, "TOUCH " << fCountFingersHeld( ) << " finger(s):" );
			for( u32 i=0 ; i<cFingerCount ; ++i )
			{
				const tTouch::tFinger finger = (tTouch::tFinger)i;

				if( !( fFingerUp(finger) || fFingerDragging(finger) || fFingerTapped(finger) ) )
					continue;

				const char* down = fFingerDown(finger)		? "D":" ";
				const char* held = fFingerHeld(finger)		? "H":" ";
				const char* up   = fFingerUp(finger)		? "U":" ";
				const char* drag = fFingerDragging(finger)	? "~":" ";
				const char* tap  = fFingerTapped(finger)	? ".":" ";
				const Math::tVec2f pos = fFingerPosition(finger);
				//const char* symbols = down?"D" : up?"U" : held?"H" : " ";

				//if ( fFingerHeld(finger) || fFingerUp(finger) )
				log_line( 0, "  " << i << " " << down << held << up << drag << tap << " " << std::setw(10) << (u32)stateData[finger].mHwFingerId << " " << (u32)pos.x << "," << (u32)pos.y << std::setw(0) );
			}
		}
	}

	void tTouch::fSetHistorySize( u32 newSize )
	{
		mStateHistory.fResize( fMax( 2u, newSize ) );
	}


	const tTouch::tStateDataBuffer& tTouch::fGetStateHistory( ) const
	{
		return mStateHistory;
	}

	const tTouch::tStateData& tTouch::fGetState( ) const
	{
		return mStateHistory[ mStateHistory.fNumItems( ) - 1 ];
	}

	b32 tTouch::fUpdated( ) const
	{
		const tStateData& now  = mStateHistory[ mStateHistory.fNumItems( ) - 1 ];
		const tStateData& prev = mStateHistory[ mStateHistory.fNumItems( ) - 2 ];

		sigassert( now.fCount() == prev.fCount() );
		for( u32 i=0 ; i<now.fCount() ; ++i )
		{
			bool different =
				( now[i].mFlags      != prev[i].mFlags      ) ||
				( now[i].mHwFingerId != prev[i].mHwFingerId ) ||
				( now[i].mPosition   != prev[i].mPosition   );
			if( different ) return true;
		}

		return false;
	}

	b32 tTouch::fActive( ) const
	{
		return fUpdated() || fFingerHeld( cFingerAny );
	}

	u32 tTouch::fCountFingersHeld( ) const
	{
		u32 num = 0;

		for( u32 i=0 ; i<cFingerCount ; ++i )
			if( fFingerHeld((tFinger)i) )
				++num;

		return num;
	}

	b32 tTouch::fFingerDoubleTapped( tFinger finger ) const
	{
		return fCheckFingerFlag( mStateHistory.fLast( ), finger, cFlagFingerDoubleTapEvent );
	}

	b32 tTouch::fFingerTapped( tFinger finger ) const
	{
		return fCheckFingerFlag( mStateHistory.fLast( ), finger, cFlagFingerTapEvent );
	}

	b32 tTouch::fFingerHeld( tFinger finger ) const
	{
		return fCheckFingerFlag( mStateHistory.fLast( ), finger, cFlagFingerHeld );
	}

	b32 tTouch::fFingerDown( tFinger finger ) const
	{
		return fCheckFingerFlag( mStateHistory.fLast( ), finger, cFlagFingerDownEvent );
	}

	b32 tTouch::fFingerUp( tFinger finger ) const
	{
		return fCheckFingerFlag( mStateHistory.fLast( ), finger, cFlagFingerUpEvent );
	}

	b32 tTouch::fFingerDragging( tFinger finger ) const
	{
		return fCheckFingerFlag( mStateHistory.fLast( ), finger, cFlagFingerDragging );
	}

	tTouch::tFinger tTouch::fFirstDoubleTappedFinger( ) const
	{
		for( u32 i=0 ; i<cFingerCount ; ++i )
			if( fFingerDoubleTapped((tFinger)i) )
				return (tFinger)i;
		return cFingerNone;
	}

	tTouch::tFinger tTouch::fFirstTappedFinger( ) const
	{
		for( u32 i=0 ; i<cFingerCount ; ++i )
			if( fFingerTapped((tFinger)i) )
				return (tFinger)i;
		return cFingerNone;
	}

	tTouch::tFinger tTouch::fFirstHeldFinger( ) const
	{
		for( u32 i=0 ; i<cFingerCount ; ++i )
			if( fFingerHeld((tFinger)i) )
				return (tFinger)i;
		return cFingerNone;
	}

	tTouch::tFinger tTouch::fFirstDownFinger( ) const
	{
		for( u32 i=0 ; i<cFingerCount ; ++i )
			if( fFingerDown((tFinger)i) )
				return (tFinger)i;
		return cFingerNone;
	}

	tTouch::tFinger tTouch::fFirstUpFinger( ) const
	{
		for( u32 i=0 ; i<cFingerCount ; ++i )
			if( fFingerUp((tFinger)i) )
				return (tFinger)i;
		return cFingerNone;
	}

	b32 tTouch::fFingerWithin( tFinger finger, const Math::tRectf& area ) const
	{
		switch( finger )
		{
		case cFingerAny:
			for( u32 i=0 ; i<cFingerCount ; ++i )
				if( fFingerHeld((tFinger)i) && fFingerWithin((tFinger)i,area) )
					return true;
			return false;
		default:
			return area.fContains( fFingerPosition(finger) );
		}
	}

	Math::tVec2f tTouch::fFingerPosition( tFinger finger ) const
	{
		sigassert( 0<=finger && finger<cFingerCount );
		return mStateHistory[ mStateHistory.fNumItems( ) - 1 ][ finger ].mPosition;
	}

	Math::tVec2f tTouch::fHeldFingerPosition( tFinger finger ) const
	{
		switch( finger )
		{
		case cFingerAny:
			for( u32 i=0 ; i<cFingerCount ; ++i )
				if( fFingerHeld((tFinger)i) )
					return fFingerPosition((tFinger)i);
			return Math::tVec2f::cZeroVector;
		default:
			if( !fFingerHeld(finger) )
				return Math::tVec2f::cZeroVector;;
			return mStateHistory[ mStateHistory.fNumItems( ) - 1 ][ finger ].mPosition;
		}
	}

	Math::tVec2f tTouch::fFingerDragThisFrame( tFinger finger ) const
	{
		switch( finger )
		{
		case cFingersNone:
			return Math::tVec2f(0,0);
		case cFingersDragging:
		case cFingersAll:
			{
				Math::tVec2f average(0,0);
				f32 fingersSoFar = 0;

				for( u32 i=0 ; i<cFingerCount ; ++i )
				{
					const tFinger f = (tFinger)i;

					const b32 isAll = (finger==cFingersAll) && (fFingerHeld(f) && !fFingerDown(f));
					const b32 isDrag = (finger==cFingersDragging) && fFingerDragging(f);

					if( isAll || isDrag )
					{
						average = average * fingersSoFar / (fingersSoFar+1.0f);
						fingersSoFar += 1.0f;
						average += fFingerDragThisFrame(f) / fingersSoFar;
					}
				}

				return average;
			}
		default:
			sigassert( 0<=finger && finger<cFingerCount );

			if( fFingerDown(finger) || !(fFingerHeld(finger) || fFingerUp(finger)) )
				return Math::tVec2f::cZeroVector;

			return	mStateHistory[ mStateHistory.fNumItems( ) - 1 ][ finger ].mPosition
				-	mStateHistory[ mStateHistory.fNumItems( ) - 2 ][ finger ].mPosition;
		}
	}

	Math::tVec2f tTouch::fFingerDragTotal( tFinger finger ) const
	{
		switch( finger )
		{
		case cFingersNone:
			return Math::tVec2f(0,0);
		case cFingersDragging:
		case cFingersAll:
			{
				Math::tVec2f average(0,0);
				f32 fingersSoFar = 0;

				for( u32 i=0 ; i<cFingerCount ; ++i )
				{
					const tFinger f = (tFinger)i;

					const b32 isAll = (finger==cFingersAll) && (fFingerHeld(f) && !fFingerDown(f));
					const b32 isDrag = (finger==cFingersDragging) && fFingerDragging(f);

					if( isAll || isDrag )
					{
						average = average * fingersSoFar / (fingersSoFar+1.0f);
						fingersSoFar += 1.0f;
						average += fFingerDragTotal(f) / fingersSoFar;
					}
				}

				return average;
			}
		default:
			sigassert( 0<=finger && finger<cFingerCount );
			return mInstanceData[ finger ].mDragCurrent;
		}
	}

	b32 tTouch::fCanReuseSlot( const tTouchStateData& slot )
	{
		return !slot.mFlags.fAny( );
	}
	
	void tTouch::fStepDrag( f32 dt )
	{
		for( u32 i = 0; i < cFingerCount; ++i )
		{
			tFinger finger = (tFinger)i;

			if( !fFingerHeld( finger ) && !fFingerUp( finger ) )
				mInstanceData[ finger ].mDragCurrent = Math::tVec2f::cZeroVector; //reset
			else // finger held or just up
			{
				Math::tVec2f currentPos = fFingerPosition( finger );

				if( fFingerDown( finger ) )
					mInstanceData[ finger ].mDragStart = currentPos; //calibrate

				Math::tVec2f drag = currentPos - mInstanceData[ finger ].mDragStart; //compute
				mInstanceData[ finger ].mDragCurrent = drag;

				if( !fFingerUp(finger) && !fFingerDragging(finger) && (drag.fLengthSquared() > ((u32)Input_Tweak_DragThreshold*Input_Tweak_DragThreshold)) )
				{
					mRawState[i].mFlags.fSetBit( cFlagFingerDragging, true );
				}
			}
		}
	}

	b32 tTouch::fCheckFingerFlag( const tStateData& data, tFinger finger, u32 bitNum ) const
	{
		switch( finger )
		{
		case cFingerAny:
			for( u32 i=0 ; i<cFingerCount ; ++i )
				if( data[i].mFlags.fGetBit(bitNum) )
					return true;
			return false;
		default:
			sigassert( 0<=finger && finger<cFingerCount );
			return data[finger].mFlags.fGetBit(bitNum);
		}
	}

	void tTouch::fClearEventFlags( tStateData& data )
	{
		// clear all event flags
		for( u32 i=0 ; i<tTouch::cFingerCount ; ++i )
		{
			if( !data[i].mFlags.fGetBit( cFlagFingerHeld ) && !data[i].mFlags.fGetBit( cFlagFingerUpEvent ) )
				data[i].mFlags.fTurnBitOff( cFlagFingerDragging );
			data[i].mFlags.fTurnBitOff( cFlagFingerDownEvent );
			data[i].mFlags.fTurnBitOff( cFlagFingerUpEvent );
			data[i].mFlags.fTurnBitOff( cFlagFingerTapEvent );
			data[i].mFlags.fTurnBitOff( cFlagFingerDoubleTapEvent );
		}
	}

	void tTouch::fHandleRawUp( tTouch::tFinger finger, Math::tVec2f position )
	{
		if( Input_Workaround_DetectBadFinger && position == mRawState[finger].mPosition )
		{
			// Bad finger up detected, don't actually release this bit...!
			tBadFingerUpEntry entry;
			entry.mAge = 0.f;
			entry.mFinger = finger;
			gBadFingerUps.fPushBack(entry);
		}
		else // Good finger up
		{
			mRawState[finger].mFlags.fTurnBitOff( tTouch::cFlagFingerHeld );
			mRawState[finger].mFlags.fTurnBitOn( tTouch::cFlagFingerUpEvent );
			if( !mRawState[finger].mFlags.fGetBit( tTouch::cFlagFingerDragging ) )
			{
				if ( fPreviousTapNear( mRawState[finger].mPosition, Input_Tweak_Touch_DoubleTapTime, Input_Tweak_Touch_DoubleTapDistance ) )
					mRawState[finger].mFlags.fTurnBitOn( tTouch::cFlagFingerDoubleTapEvent );
				mRawState[finger].mFlags.fTurnBitOn( tTouch::cFlagFingerTapEvent );
			}
		}
	}

	void tTouch::fHandleRawDown( tTouch::tFinger finger, Math::tVec2f position )
	{
		mRawState[ finger ].mFlags.fSetBit( tTouch::cFlagFingerDownEvent, true );
		mRawState[ finger ].mFlags.fSetBit( tTouch::cFlagFingerHeld     , true );
		mRawState[ finger ].mFlags.fSetBit( tTouch::cFlagFingerTapEvent , false );
		mRawState[ finger ].mFlags.fSetBit( tTouch::cFlagFingerDoubleTapEvent , false );
		if( mRawState[finger].mFlags.fGetBit(tTouch::cFlagFingerDragging))
		{
			mRawState[ finger ].mFlags.fSetBit( tTouch::cFlagFingerDragging , false );
		}
	}

	/// \brief Tries to check if this was a bad finger, and if so, handle that.
	/// Returns true if it was indeed a recovered bad finger.
	b32 tTouch::fRecoverBadFingers( tHwFingerId hwFinger, const Math::tVec2f& fingerPos )
	{
		if( !Input_Workaround_DetectBadFinger )
			return false;

		f32 nearestDistSq = 9001.0f; // arbitrary
		for( u32 i=0 ; i<gBadFingerUps.fCount( ) ; ++i )
		{
			const tTouch::tFinger finger = gBadFingerUps[i].mFinger;
			f32 distSq = (mRawState[finger].mPosition-fingerPos).fLengthSquared();
			const f32 maxDist = Input_Workaround_BadFingerHotzone;
			if( distSq < nearestDistSq )
				nearestDistSq = distSq;
			if( distSq < maxDist*maxDist )
			{
				// Exact position match, probably a bad finger.
				mRawState[finger].mHwFingerId = hwFinger;
				mRawState[finger].mPosition = fingerPos;
				gBadFingerUps.fErase(i);
				return true;
			}
		}

		if( gBadFingerUps.fCount( )>0 )
			log_warning( "New down finger while some bad, missed by " << sqrtf(nearestDistSq) );

		return false;
	}

	b32 tTouch::fPreviousTapNear( const Math::tVec2f& fingerPos, u32 maxAge, f32 maxDist )
	{
		const u32 beg = fMax( 0, (s32)mStateHistory.fCapacity( ) - (s32)maxAge - 1 );
		const u32 end = fMax( 0, (s32)mStateHistory.fCapacity( ) - 2 ); // don't include the current frame

		for( u32 i=beg ; i<=end ; ++i )
		{
			for( u32 tapFinger=0 ; tapFinger<cFingerCount ; ++tapFinger )
			{
				if( mStateHistory[i][tapFinger].mFlags.fGetBit(cFlagFingerTapEvent) && (mStateHistory[i][tapFinger].mPosition - fingerPos).fLengthSquared() < Math::fSquare(maxDist) )
				{
					return true;
				}
			}
		}

		return false;
	}
}}
