#ifndef __tTimedSpawnLogic__
#define __tTimedSpawnLogic__

namespace Sig
{
	class tTimedSpawnLogic : public tLogic
	{
		define_dynamic_cast( tTimedSpawnLogic, tLogic );
	public:
		tTimedSpawnLogic( );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		void fSpawnIt( );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		b32				mSpawned;
		f32				mTimeRemaining;
		f32				mRepeatTime;
		tFilePathPtr	mFilepath;
		tStringPtr		mEffectID;
	};

}

#endif//__tTimedSpawnLogic__
