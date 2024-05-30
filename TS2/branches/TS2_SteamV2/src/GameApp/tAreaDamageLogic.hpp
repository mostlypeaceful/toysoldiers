#ifndef __tAreaDamageLogic__
#define __tAreaDamageLogic__

#include "tProximity.hpp"
#include "tUnitLogic.hpp"

namespace Sig
{
	class tWeapon;
	class tUnitLogic;

	class tAreaDamageLogic : public tLogic
	{
		define_dynamic_cast( tAreaDamageLogic, tLogic );
	public:
		tAreaDamageLogic( );
		virtual ~tAreaDamageLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		static void fExportScriptInterface( tScriptVm& vm );

		void fSetDamageID( const tDamageID& id ) { mAttackerID = id; }

		void fEnable( b32 enable ) { mEnabled = enable; }

	protected:
		tProximity mProximity;
		tGrowableArray<tEntityPtr> mTargets;

		b32 mEnabled;
		tDamageID mAttackerID;
		b32 fShouldDamage( tUnitLogic* logic );
		void fAddShape( tEntity* ent );
	};

	class tInstaKillAreaDamageLogic : public tAreaDamageLogic
	{
		define_dynamic_cast( tAreaDamageLogic, tLogic );
	public:
		tInstaKillAreaDamageLogic( );

		virtual void fActST( f32 dt );
	};
}

#endif//__tAreaDamageLogic__