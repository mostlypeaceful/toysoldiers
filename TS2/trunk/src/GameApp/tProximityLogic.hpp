#ifndef __tProximityLogic__
#define __tProximityLogic__
#include "tProximity.hpp"

namespace Sig
{

	class tProximityLogic : public tLogic
	{
		define_dynamic_cast( tProximityLogic, tLogic );
	public:
		tProximityLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );

		virtual void fActST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		void fSetFrequency( f32 seconds, f32 randomDeviation );
		void fAddEnumFilter( u32 key, u32 value );

		const tGrowableArray<tEntityPtr>& fEntityList( ) const { return mProximity.fEntityList( ); }

		const Math::tObbf& fShape( ) const { return mShape; }
		void fSetEnabled( b32 enable );
		b32  fEnabled( ) const { return mEnabled; }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		tProximity mProximity;
		tProximity* fProximityForScript( ) { return &mProximity; }

		//debugging
		Math::tObbf mShape;

		b32				mEnabled;
		u32				mPreviousEntityCount;
		Sqrat::Function mEntityCountChanged;
		Sqrat::Function mNewEntity;

	};

}

#endif//__tProximityLogic__