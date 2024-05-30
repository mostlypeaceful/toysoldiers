#ifndef __tFpsCounter__
#define __tFpsCounter__

namespace Sig
{

	///
	/// \brief Simple utility class for tracking frames per second.
	class base_export tFpsCounter
	{
	private:
		f32 mUpdateEveryNSeconds;
		u32 mNumTicks;
		f32 mFps;
		f32 mLogicMs;
		f32 mTempLogicMs;
		Time::tStopWatch mTimer;
		Time::tStopWatch mLogic;
	public:

		///
		/// \param nSeconds Will update current fps every N seconds.
		tFpsCounter( f32 nSeconds = 1.f );

		///
		/// \brief Call this once per frame, before rendering.
		void fPreRender( );

		///
		/// \brief Call this once per frame, after rendering.
		void fPostRender( );

		///
		/// \brief Retrieve the current fps.
		inline f32 fFps( ) const { return mFps; }

		///
		/// \brief Retrieve the logic ms.
		inline f32 fLogicMs( ) const { return mLogicMs; }
	};

}

#endif//__tFpsCounter__

