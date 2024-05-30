#ifndef __tRumbleManager__
#define __tRumbleManager__

namespace Sig { namespace Input
{

	class tRumbleEvent
	{
	public:
		tRumbleEvent( f32 intensity = 0.0f
			, f32 duration = 0.0f
			, f32 decay = 1.0f
			, f32 vibeMix = 0.5f
			, f32 maxDist = 1.f 
			, f32 falloff = 1.f )
			: mIntensity( intensity )
			, mDuration( duration )
			, mDecay( decay )
			, mVibeMix( vibeMix )
			, mMaxDist( maxDist )
			, mFallOff( falloff )
		{ }

		f32 mIntensity;//[0,1]
		f32 mDuration; //at full intensity before decay is applied
		f32 mDecay;    //rate to fall off after duration
		f32 mVibeMix;  // 0.0f is full heavy, 1.0f is full light, 0.5f is 50/50 mix
		f32 mMaxDist;
		f32 mFallOff;
	};

	class tRumbleManager
	{
	public:
		tRumbleManager( );

		void fClear( );

		void fAddEvent( const tRumbleEvent& e, f32 scale = 1.f );

		// returns true if the data should be applied to the controller
		b32  fStep( f32 dt );

		// Set this to true to stop advancing rumble manager and to mute current rumble state.
		void fSetRumblePaused( b32 paused ) { mPauseRumble = paused; }
		b32  fRumblePaused( ) const { return mPauseRumble; }

		// Set an explicit minimum rumble for this frame. Will be zeroed after fStep
		void fSetExplicitRumble( f32 er ) { mExplicitRumble = fMax( er, mExplicitRumble ); }

		// A simple way to buzz the controller, even when disabled.
		//  Like when a UI selection has been made.
		void fBuzz( f32 strength );

		// {left - heavy, right - light}
		const Math::tVec2f& fCurrentRumble( ) const { return mCurrentRumble; }

	private:
		Math::tVec2f mCurrentRumble; // {left, right}
		f32 mExplicitRumble;
		b32 mPauseRumble;

		b32 mBuzzing;
		f32 mBuzzTimer;
		f32 mBuzzStrength;

		tGrowableArray< tRumbleEvent > mEvents;

	};

}}

#endif//__tRumbleManager__
