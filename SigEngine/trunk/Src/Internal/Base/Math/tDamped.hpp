#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tDamped__
#define __tDamped__

namespace Sig { namespace Math
{
	template<class t>
	class tDamped
	{
	public:
		explicit tDamped( const t& startVal = t( ), f32 blendStart = 0.f, f32 blendEnd = 1.f, f32 duration = 1.f ) 
			: mT( startVal )
			, mBlendStart(blendStart)
			, mBlendEnd(blendEnd)
			, mDuration(duration)
			, mTimer(0.f)
		{
		}

		void fStep( const t& target, f32 dt )
		{
			const f32 frameRateCompensation = dt / (1.f/30.f);
			const f32 blendT = frameRateCompensation * fLerp( mBlendStart, mBlendEnd, fMin( mTimer / mDuration, 1.f ) );
			mT = Math::fLerp( mT, target, blendT );
			mTimer += dt;
		}

		const t& fValue( ) const { return mT; }
		
		void fSetValue( const t& v ) { mT = v; }
		void fSetBlends( const f32& blendStart, const f32& blendEnd ) { mBlendStart = blendStart; mBlendEnd = blendEnd; }
		void fSetBlends( const f32& blendStartAndEnd ) { mBlendStart = blendStartAndEnd; mBlendEnd = blendStartAndEnd; }

	private:
		t mT;
		f32 mBlendStart;
		f32 mBlendEnd;
		f32 mDuration;
		f32 mTimer;
	};

	typedef tDamped<f32>	tDampedFloat;
	typedef tDamped<tVec2f> tDampedVec2f;
	typedef tDamped<tVec3f> tDampedVec3f;
	typedef tDamped<tVec4f> tDampedVec4f;

	template<class t>
	class tPDDamped
	{
	public:
		tPDDamped( const t& startVal = t( ), const t& spring = 10.f, const t& damper = 5.f )
			: mT( startVal )
			, mV( 0 )
			, mSpring( spring )
			, mDamper( damper )
		{ }

		void fStep( const t& target, f32 dt )
		{
			t error = target - mT;
			t errorV = -mV;

			t a = mSpring * error + mDamper * errorV;
			
			mV += a * dt;
			mT += mV * dt;
		}

		const t& fValue( ) const 
		{ 
			return mT; 
		}

		void fSetValue( const t& val ) 
		{ 
			mT = val; 
			mV = 0.0f; 
		}

		void fSetPD( const t& spring, const t& damper ) 
		{ 
			mSpring = spring; 
			mDamper = damper; 
		}

		void fSetBlends( const t& blends ) 
		{ 
			mSpring = blends * 10.f; 
			mDamper = mSpring * 0.5f; 
		}

	private:
		t mT;
		t mV;
		t mSpring;
		t mDamper;
	};

	typedef tPDDamped<f32>	tPDDampedFloat;
	typedef tPDDamped<tVec2f> tPDDampedVec2f;
	typedef tPDDamped<tVec3f> tPDDampedVec3f;
	typedef tPDDamped<tVec4f> tPDDampedVec4f;


	class base_export tVelocityDamped
	{
		f32 mCurrentValue;
		f32 mVelocity;
		f32 mAcceleration;

	public:
		tVelocityDamped( )
			: mCurrentValue( 0.f )
			, mVelocity( 0.f )
			, mAcceleration( 2.f )
		{ }

		void fSetVelocity( f32 vel ) { mVelocity = vel; }
		void fSetAcceleration( f32 acc ) { mAcceleration = acc; }
		void fSetValue( f32 val ) { mCurrentValue = val; }
		f32  fValue( ) const { return mCurrentValue; }
		void fStep( f32 target, f32 dt );
	};


}}

#endif//__tDamped__

