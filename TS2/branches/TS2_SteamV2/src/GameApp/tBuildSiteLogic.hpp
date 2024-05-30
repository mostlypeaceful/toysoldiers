#ifndef __tBuildSiteLogic__
#define __tBuildSiteLogic__

#include "tShapeEntity.hpp"
#include "tUnitLogic.hpp"
#include "Gui/tWorldSpaceText.hpp"
#include "tProximity.hpp"

namespace Sig
{
	class tBuildSiteLogic : public tUnitLogic
	{
		define_dynamic_cast( tBuildSiteLogic, tUnitLogic );
	public:
		tBuildSiteLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );

		virtual Gui::tRadialMenuPtr fCreateSelectionRadialMenu( tPlayer& player );

	public:
		u32 fBuildSiteSize( ) const { return mSize; }

		b32 fIsUsableBy( u32 team ) const; //considers capturing only
		b32 fIsValid( u32 team ) const;
		b32 fIsOccupied( ) const;
		b32 fIsReserved( ) const { return mReserved; }
		void fSetReserved( b32 reserved ) { mReserved = reserved; }
		tEntity* fUnit( ); //the unit built on this site if it exists
		
		b32 fTryToUse( tPlayer* player );

		void fSetPrice( f32 price );
		f32 fPrice( ) const { return mLastPrice; }
		void fSetLockedState( b32 locked );
		b32 fHasPrice( ) const { return !mPrice.fNull( ); }

		tShapeEntity* fShape( ) const { return mBuildSiteShape; }

		b32					fCapturable( ) const { return mCaptureShape != NULL; }
		u32					fCapturingTeam( ) const { return mCapturingTeam; }
		u32					fCapturedTeam( ) const { return mCapturedTeam; }
		f32					fCapturingPercent( ) const { return mCapturingPercent; }
		const Math::tVec4f& fCapturingColor( ) const { return mCapturingColor; }

		tRefCounterPtr<tEntitySaveData> fStoreSaveGameData( b32 entityIsPartOfLevelFile, tSaveGameRewindPreview& preview );
		void fResetCapturingData( u32 capturedTeam, u32 capturingTeam, f32 capturingPercent );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		GameFlags::tBUILD_SITE	mSize;
		tShapeEntity*			mBuildSiteShape;

		tShapeEntity*			mCaptureShape;
		tProximity				mCaptureProximity;
		u32						mCapturedTeam;
		tFixedArray< f32, GameFlags::cTEAM_COUNT >   mCaptureTime;
		tFixedArray< b8, GameFlags::cTEAM_COUNT >	 mCaptureTeamIn;
		b16						mReserved;

		u32 mCapturingTeam;
		f32 mCapturingPercent;
		Math::tVec4f mCapturingColor;

		f32 mLastPrice;
		Gui::tWorldSpaceTextPtr mPrice;

		void fProcessCapturing( f32 dt );
		void fSetCurrentTint( const Math::tVec4f& tint );
	};

}

#endif//__tBuildSiteLogic__

