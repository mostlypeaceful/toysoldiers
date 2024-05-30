// Barrage Controller
#ifndef __tBarrageController__
#define __tBarrageController__

#include "tBarrage.hpp"
#include "tBarrageUI.hpp"

namespace Sig
{
	class tBarrageController : public tRefCounter
	{
	private:
		static const float cSpinTime;

	public:
		enum tBarrageState
		{
			BARRAGE_STATE_NONE,
			BARRAGE_STATE_SPINNING,
			BARRAGE_STATE_AVAILABLE,
			BARRAGE_STATE_ACTIVE,
			BARRAGE_STATE_IN_USE,
			BARRAGE_STATE_WAITING_FOR_COMBO_LOSS,
			BARRAGE_STATE_COUNT
		};

	public:
		tBarrageController( tPlayer* player );
		~tBarrageController( );

		void fAddBarrage( tBarragePtr& barrage );
		const tGrowableArray< tBarragePtr >& fBarrages( ) const { return mBarrages; }
		const tBarragePtr& fCurrentBarrage( ) const;
		const tBarragePtr& fCurrentOrLastBarrage( ) const { return mCurrentOrLastBarrage; }

		b32 fDormant( ) const { return mBarrageState == BARRAGE_STATE_NONE; }
		b32 fBarrageActive( ) const { return mBarrageState == BARRAGE_STATE_ACTIVE || mBarrageState == BARRAGE_STATE_IN_USE; }

		void fStepBarrage( f32 dt );
		void fComboLost( );

		void fGiveBarrage( tPlayer* coopPlayer, b32 skipInto, b32 forTutorial );
		void fRestrictBarrage( const tStringPtr& name );
		tStringPtr fGetRestrictedBarrage( ) { return mRestrictBarrage; }

		void fSkipSpinAndEnter( const tStringPtr& name, tEntity* target );

		void fShowUI( b32 show );

		void fEndBarrage( );
		void fOnDelete( );

		void fBeginNoRepeats( );
		void fEndNoRepeats( );

		s32 fCurrentBarrageIndex( ) const { return mCurrentBarrageIndex; }
		void fReactivateBarrage( s32 index );

		const static tBarragePtr cNullBarrage;

	private:
		tGrowableArray< tBarragePtr > mBarrages;
		tStringPtr mRestrictBarrage;
		tBarragePtr mCurrentBarrage;
		tBarragePtr mCurrentOrLastBarrage;
		s32 mCurrentBarrageIndex;
		tBarrageState mBarrageState;
		Gui::tBarrageUIPtr mBarrageUI;
		f32 mBarrageMeterPercent;
		f32 mSpinTimer;
		tPlayer* mPlayer;

		b32 mPreventDuplicateBarrages;
		tGrowableArray<u32> mUniqueBarragesRemaining;

		u32 fNextBarrageIndex( tPlayer* coopPlayer );

		void fSetBarrageMeterValue( f32 newValue );
	};

	typedef tRefCounterPtr< tBarrageController > tBarrageControllerPtr;
}

#endif //__tBarrageController__