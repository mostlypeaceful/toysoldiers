#ifndef __tGoalBoxLogic__
#define __tGoalBoxLogic__

#include "tBreakableLogic.hpp"
#include "tShapeEntity.hpp"
#include "tPathEntity.hpp"

namespace Sig
{
	class tGoalBoxLogic : public tAnimatedBreakableLogic
	{
		define_dynamic_cast( tGoalBoxLogic, tAnimatedBreakableLogic );
	public:
		tGoalBoxLogic( );
		virtual void fOnSpawn( );
		virtual b32  fHandleLogicEvent( const Logic::tEvent& e );

	public:
		const tShapeEntityPtr& fGetGoalZone( ) const;
		virtual void fAddToMiniMap( ) { }
		void fActivate( ) { fActive( true ); }
		void fDeactivate( ) { fActive( false ); }
		void fActive( b32 active );
		b32  fIsActive( ) const { return mActive; }

		b32  fCheckInBounds( tUnitLogic* logic ) const;
		void fSomeoneEntered( );
		void fRegisterPathEnd( tEntity* endPoint );
		const tGrowableArray<tPathEntityPtr>& fPathEnds( ) const { return mPathEnds; }
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tShapeEntityPtr	mGoalZone;
		b32 mActive;
		tGrowableArray<tPathEntityPtr>	mPathEnds;

		tEntityPtr mGamerTag; //for VS
	};

}

#endif//__tGoalBoxLogic__

