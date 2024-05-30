#ifndef __tGoalDriven__
#define __tGoalDriven__
#include "AI/tGoal.hpp"

namespace Sig { namespace Logic
{

	class tGoalDriven
	{
	private:
		AI::tGoalPtr	mMasterGoal;
		tLogic			*mLogic;
	public:
		tGoalDriven( );
		void fOnDelete( tLogic* logic );
		void fClearGoals( tLogic* logic );
		void fOnSpawn( tLogic* logic );
		void fActST( tLogic* logic, f32 dt );
		b32  fHandleLogicEvent( tLogic* logic, const Logic::tEvent& e );
		const AI::tGoalPtr& fMasterGoal( ) const { return mMasterGoal; }
		void fSetMasterGoal( const AI::tGoalPtr& goal ) { mMasterGoal = goal; }
		void fSetLogic( tLogic* logic ) { mLogic = logic; }
		if_devmenu( void fAddWorldDebugText( std::stringstream& ss ) const { mMasterGoal.fAddWorldDebugText( ss, 0 ); } );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	private:// script-specific private member functions (exposed only to script, not code)
		Sqrat::Object				fScriptGoalObject( ) const { return mMasterGoal.fScriptObject( ); }
		void						fSetScriptGoalObject( const Sqrat::Object& obj );
	};

}}

#endif//__tGoalDriven__
