#include "BasePch.hpp"
#include "tFpsCounter.hpp"

namespace Sig
{
	tFpsCounter::tFpsCounter( f32 nSeconds )
		: mUpdateEveryNSeconds( nSeconds )
		, mNumTicks( 0 )
		, mFps( 0.f )
		, mLogicMs( 0.f )
		, mTempLogicMs( 0.f )
	{
	}
	void tFpsCounter::fPreRender( )
	{
		++mNumTicks;

		mTempLogicMs = mLogic.fGetElapsedMs( );

		const f32 elapsedS = mTimer.fGetElapsedS( );
		if( elapsedS >= mUpdateEveryNSeconds )
		{
			mFps = mNumTicks / mUpdateEveryNSeconds;
			mLogicMs = mTempLogicMs / mNumTicks;
			mNumTicks = 0;
			mTempLogicMs = 0.f;
			mTimer.fResetElapsedS( std::fmod( elapsedS, mUpdateEveryNSeconds ) );
		}
	}
	void tFpsCounter::fPostRender( )
	{
		mLogic.fResetElapsedS( mTempLogicMs / 1000.f );
	}
}
