#ifndef __tBreakableLogic__
#define __tBreakableLogic__
#include "tUnitLogic.hpp"
#include "tGameArchive.hpp"
#include "Logic/tAnimatable.hpp"

namespace Sig
{
	class tBreakableLogic : public tUnitLogic
	{
		define_dynamic_cast( tBreakableLogic, tUnitLogic );
	public:
		tBreakableLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );

		virtual void fRegisterUnit( );

	protected:
		virtual void fOnStateChanged( );
		virtual tRefCounterPtr<tEntitySaveData> fStoreSaveGameData( b32 entityIsPartOfLevelFile, tSaveGameRewindPreview& preview );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		GameFlags::tTRIGGERED_BREAK_STATE mBreakState;
	};

	class tAnimatedBreakableLogic : public tBreakableLogic
	{
		define_dynamic_cast( tAnimatedBreakableLogic, tBreakableLogic );
	public:
		tAnimatedBreakableLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fAnimateMT( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fOnSkeletonPropagated( );

		virtual Logic::tAnimatable* fQueryAnimatable( ) { return &mAnimatable; }

		b32 mApplyRefFrame;

	protected:
		Logic::tAnimatable	mAnimatable;
	};

}

#endif//__tBreakableLogic__
