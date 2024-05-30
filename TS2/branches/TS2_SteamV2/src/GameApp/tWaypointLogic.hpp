#ifndef __tWaypointLogic__
#define __tWaypointLogic__

#include "tPathEntity.hpp"

namespace Sig
{
	class tWaypointLogic : public tLogic
	{
		define_dynamic_cast( tWaypointLogic, tLogic );
	public:
		tWaypointLogic( );
		virtual ~tWaypointLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual b32  fHandleLogicEvent( const Logic::tEvent& e );
		virtual void fActST( f32 dt );

	public:
		b32	fIsAccessible( ) const { return mAccessible; }
		void fSetAccessible( b32 accessible );
	private:
		void fPropagateAccessibility( tPathEntity* path );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

		enum tPathType
		{
			cGoalPath,
			cTrenchPath,
			cExitGeneratorPath,
			cRandomFlyingPath,
			cContextPath,

			cPathTypeCount
		};

		tPathType fPathType( ) const { return (tPathType)mPathType; }

	private:
		b32	mAccessible;
		u32 mPathType;
	};

}

#endif//__tWaypointLogic__

