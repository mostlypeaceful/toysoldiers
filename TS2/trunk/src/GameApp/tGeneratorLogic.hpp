#ifndef __tGeneratorLogic__
#define __tGeneratorLogic__

#include "tBreakableLogic.hpp"
#include "tAttachmentEntity.hpp"
#include "tProximity.hpp"
#include "tPathEntity.hpp"
#include "tWaveList.hpp"

namespace Sig
{
	class tGeneratorLogic : public tAnimatedBreakableLogic
	{
		define_dynamic_cast( tGeneratorLogic, tAnimatedBreakableLogic );

	public:
		tGeneratorLogic( );
		virtual ~tGeneratorLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fMoveST( f32 dt );
		virtual void fOnPause( b32 paused );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	public:
		void			fBeginReadying( ) { mReadying = true; }

		tGeneratorWave*	fStartGenerating( tWaveDesc& desc, u32 spawnCount );

		// if this wave hasn't been passed to fStartGenerating it will in this cal.
		void			fLaunch( tWaveDesc& desc, u32 spawnCount );

		b32				fSpawnedEnough( ) const;
		void			fStopGenerating( );
		void			fKillEveryone( );
		void			fReleaseTrenched( );

		b32 fProximityBlocked( ) const { return mProximity.fEntityCount( ); }

		const tFilePathPtr& fSmokeSigml( ) const { return mDropSmokeSigml; }

		b32 fSpawnLightAndSound( ) const { return !fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_MINIGAME_UNIT ); }
		tEntity* fVerryLightPoint( ) const { return mVeryLightPt; }
		Math::tVec3f fSpawnPoint( ) const { return mSpawnPoint->fObjectToWorld( ).fGetTranslation( ); }

		void fSetParentGenerator( tEntity* parent ) { mParentGenerator.fReset( parent ); }
		tGeneratorLogic* fParentGenerator( ) const { return mParentGenerator ? mParentGenerator->fLogicDerivedStaticCast<tGeneratorLogic>( ) : NULL; }

		void fFindAndLinkParent( tGrowableArray<tEntityPtr>& gens );

	protected:
		virtual void fAddToMiniMap( ) { }

	private:
		void fRefreshActiveUnitCounts( );
		void fSetupProximity( );

		b32 fOpenDoors( ) const { return mOpenDoors; }
		b32 fReadying( ) const { return mReadying; }
		void fStepDoors( );
		void fRequestOpen( tEntity* from, b32 open );
		void fRequestReady( tEntity* from, b32 ready );

		tGrowableArray<tWavePtr> mWaves;

		b8 mActive;
		b8 mOpenDoors;
		b8 mReadying;
		b8 mWasReadying;

		tEntity*	mSpawnPoint;
		tEntity*	mVeryLightPt;

		// How often we update our unit counts
		f32 mRefreshActiveUnitTimer;

		tProximity mProximity;

		tFilePathPtr mDropSmokeSigml;

		tEntityPtr mParentGenerator; // Tell this guy to open his doors for us
		tGrowableArray<tEntity*> mOpenRequests; //from our children //this is just a super safe way to set a flag, if the count is non zero, open up
		tGrowableArray<tEntity*> mReadyRequests; //from our children //this is just a super safe way to set a flag, if the count is non zero, open up
	};

}

#endif//__tGeneratorLogic__

