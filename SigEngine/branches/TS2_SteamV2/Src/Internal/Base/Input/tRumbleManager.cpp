#include "BasePch.hpp"
#include "tRumbleManager.hpp"

namespace Sig { namespace Input
{
	tRumbleManager::tRumbleManager( )
		: mCurrentRumble( 0.f )
		, mExplicitRumble( 0.f )
		, mPauseRumble( false )
		, mBuzzing( false )
		, mBuzzTimer( -1.f )
		, mBuzzStrength( 0.f )
	{ }


	void tRumbleManager::fClear( )
	{
		mExplicitRumble = 0.f;
		mCurrentRumble = Math::tVec2f::cZeroVector; 
		mEvents.fSetCount( 0 );
	}

	void tRumbleManager::fAddEvent( const tRumbleEvent& e, f32 scale )
	{
		// don't accumulate events while we're paused. this shouldn't happen in the game, and could only badness.
		if( !mPauseRumble )
		{
			mEvents.fPushBack( e );
			mEvents.fBack( ).mIntensity *= scale;
		}
	}

	b32 tRumbleManager::fStep( f32 dt )
	{
		b32 postValues = !mPauseRumble;

		if( !mPauseRumble )
		{
			mCurrentRumble.x = mExplicitRumble;
			mCurrentRumble.y = mExplicitRumble;
			mExplicitRumble = 0.f;

			for( u32 i = 0; i < mEvents.fCount( ); )
			{
				tRumbleEvent &e = mEvents[i];

				mCurrentRumble.x = fMax( mCurrentRumble.x, e.mIntensity * (1.0f - e.mVibeMix) );
				mCurrentRumble.y = fMax( mCurrentRumble.y, e.mIntensity * e.mVibeMix);
				
				if( e.mDuration > 0.0f ) 
					e.mDuration -= dt;
				else if ( e.mIntensity > 0.0f ) 
					e.mIntensity -= (1.0f / e.mDecay) * dt;
				else
				{
					mEvents.fErase( i );
					continue;
				}

				++i;
			}
		}

		if( mBuzzing )
		{
			mBuzzTimer -= dt; 
			if( mBuzzTimer < 0.f ) 
				mBuzzing = false;

			mCurrentRumble.x = fMax( mCurrentRumble.x, mBuzzStrength );
			mCurrentRumble.y = fMax( mCurrentRumble.y, mBuzzStrength );
			postValues = true;
		}

		return postValues;
	}


	void tRumbleManager::fBuzz( f32 strength )
	{
		mBuzzing = true;
		mBuzzTimer = 0.25f;
		mBuzzStrength = strength;
	}

	
}}
