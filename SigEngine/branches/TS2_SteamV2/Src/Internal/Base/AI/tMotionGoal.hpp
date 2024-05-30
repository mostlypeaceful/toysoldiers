#ifndef __tMotionGoal__
#define __tMotionGoal__
#include "tGoal.hpp"

namespace Sig { namespace AI
{

	class tMotionGoal : public tGoal
	{
		define_dynamic_cast( tMotionGoal, tGoal );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tMotionGoal( );
		tMotionGoal( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable ); // calls fSetMotionState
		virtual void fOnActivate( tLogic* logic );
		void fSetMotionState( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable );
		void fExecuteMotionState( tLogic* logic );
		f32  fEvaluateMotionState( tLogic* logic );
	private:
		Sqrat::Function	mApplyMotionState;
		Sqrat::Object	mMotionStateParams;
		if_devmenu( tStringPtr mStateName );
	};


	class tTimedMotionGoal : public tMotionGoal
	{
		define_dynamic_cast( tTimedMotionGoal, tMotionGoal );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tTimedMotionGoal( );
		tTimedMotionGoal( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable ); // calls fSetMotionState
		virtual void fOnActivate( tLogic* logic );
		virtual void fOnProcess( tGoalPtr& goalPtr, tLogic* logic, f32 dt );
		if_devmenu( virtual std::string fExtraWorldDebugText( ) const );
	protected:
		f32			mTimer;
		f32			mTimeToLive;
	};


	class tOneShotMotionGoal : public tTimedMotionGoal
	{
		define_dynamic_cast( tOneShotMotionGoal, tTimedMotionGoal );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tOneShotMotionGoal( );
		tOneShotMotionGoal( tLogic* logic, const char* motionStateName, const Sqrat::Object& motionStateTable ); // calls fSetMotionState
		virtual void fOnActivate( tLogic* logic );
	};

}}

#endif//__tMotionGoal__
