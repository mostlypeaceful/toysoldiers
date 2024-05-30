#ifndef __tTinkStack__
#define __tTinkStack__

namespace Sig { namespace Gfx
{

	class tTintEntry : public tRefCounter
	{
	public:
		static const f32 cActiveThreshold;
		static const f32 cFullStrengthThreshold;

		tTintEntry( const Math::tVec4f& tint = Math::tVec4f::cZeroVector, f32 blend = 0.2f ) 
			: mCurrentTint( tint )
			, mBlendStrength( 0.f )
			, mBlendStrengthUser( 1.f )
			, mTargetBlendStrength( 0.f )
			, mBlendLerp( blend )
			, mActivateTime( -1.f )
			, mActiveLastFrame( false )
			, mFullLastFrame( false )
		{ }

		virtual ~tTintEntry( ) { }

		// returns true if the entry has changed since last step
		virtual b32 fStep( f32 dt ) 
		{
			if( mActivateTime > 0.f )
			{
				mActivateTime -= dt;
				if( mActivateTime <= 0.f )
					mTargetBlendStrength = 0.f;
			}

			mBlendStrength = Math::fLerp( mBlendStrength, mTargetBlendStrength, mBlendLerp );
			
			// track the state changes from off, on, and full. That way we dont apply the tint if it hasnt changed.	
			b32 active = ( mBlendStrength > cActiveThreshold );
			b32 changed = true;

			if( !active )
			{
				if( !mActiveLastFrame ) changed = false;
				mFullLastFrame = false;
			}
			else
			{
				b32 full = ( mBlendStrength > cFullStrengthThreshold );
				if( full && mFullLastFrame ) changed = false;
				mFullLastFrame = full;
			}

			mActiveLastFrame = active;
			return changed;			
		}

		void fSetCurrentTint( const Math::tVec4f& tint ) { mCurrentTint = tint; mFullLastFrame = false; } //this forces a "changed" state
		const Math::tVec4f& fCurrentTint( ) const { return mCurrentTint; }
		f32 fBlendStrength( ) const { return mBlendStrength * mBlendStrengthUser; }

		// Blend strength is lerped to the target, and timed in some cases
		void fSetBlendStrength( f32 strength ) { mBlendStrength = strength; }
		void fSetTargetBlendStrength( f32 strength ) { mTargetBlendStrength = strength; }
		void fSetBlendLerp( f32 lerp ) { mBlendLerp = lerp; }

		// blend strength user is not lerped and is user controlled
		void fSetBlendStrengthUser( f32 strength ) { mBlendStrengthUser = strength; }
		f32 fBlendStrengthUser( ) const { return mBlendStrengthUser; }

		/// Activation time, set < 0 to stay on. > 0 to fade out after time.
		void fSetActivationTime( f32 time ) { mTargetBlendStrength = 1.f; mActivateTime = time; }

	protected:
		Math::tVec4f mCurrentTint;
		f32 mBlendStrengthUser;
		f32 mBlendStrength;
		f32 mTargetBlendStrength;
		f32 mBlendLerp;
		f32 mActivateTime;

		b16 mActiveLastFrame;
		b16 mFullLastFrame;
	};

	typedef tRefCounterPtr< tTintEntry > tTintEntryPtr;

	class tSolidTint : public tTintEntry
	{
	public:
		tSolidTint( const Math::tVec4f& tint, f32 blend ) 
			: tTintEntry( tint, blend )
		{ }
	};

	class tFlashingTint : public tTintEntry
	{
	public:
		tFlashingTint( const Math::tVec4f& tint, f32 blend, f32 rate ) 
			: tTintEntry( tint, blend )
			, mRate( rate )
			, mPulse( 0.f )
			, mSourceTint( tint )
			, mUserBlend( 1.f )
		{ }

		virtual b32 fStep( f32 dt ) 
		{
			b32 changed = false;

			if( mBlendStrength > cActiveThreshold )
			{
				mPulse += dt / mRate;
				f32 blend = mUserBlend * Math::fRemapZeroToOne( -1.f, 1.f, Math::fSin( mPulse * Math::c2Pi ) );
				mCurrentTint = mSourceTint * blend;
				changed = true;
			}

			if( tTintEntry::fStep( dt ) ) changed = true;

			return changed;
		}

		void fSetSourceTint( const Math::tVec4f& src ) { mSourceTint = src; }

	private:
		f32 mRate;
		f32 mPulse;
		Math::tVec4f mSourceTint;
	protected:
		f32 mUserBlend; //control this from derived types to modify the blend
	};

	


	class tTintStack
	{
	public:
		tTintStack( );

		void fStep( f32 dt );
		tGrowableArray< tTintEntryPtr >& fStack( ) { return mStack; }
		b32 fHasIndex( u32 index ) const { return index < mStack.fCount( ) && mStack[ index ]; }

		// If this is false, no need to apply the current tint.
		b32 fChanged( ) const { return mChanged; }
		void fForceChanged( ) { mChanged = true; } //call this to ensure fChanged returns true
		const Math::tVec4f& fCurrentTint( ) const { return mCurrentTint; }

		void fSetAllBlendStrengths( f32 blend ) { for( u32 i = 0; i < mStack.fCount( ); ++i ) if( mStack[ i ] ) mStack[ i ]->fSetBlendStrength( blend ); }
		
	protected:
		tGrowableArray< tTintEntryPtr > mStack;
		Math::tVec4f mCurrentTint;

		b16 mChanged;
		b16 mForceChanged;
	};

}}

#endif//__tTinkStack__
