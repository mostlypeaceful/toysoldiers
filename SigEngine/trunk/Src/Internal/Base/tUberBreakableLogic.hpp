#ifndef __tUberBreakableLogic__
#define __tUberBreakableLogic__

#include "tUberBreakableClump.hpp"
#include "Logic/tDamageable.hpp"

namespace Sig
{

	class tUberBreakableLogic;
	class t3dGridEntity;


	class base_export tUberBreakableDamagable : public Logic::tDamageable
	{
		define_dynamic_cast( tUberBreakableDamagable, Logic::tDamageable );
	public:
		// Represents the whole breakable
		// TODO, currently this is never called, i couldn't find a good place to call this.
		virtual void fOnBreakableBroken( tUberBreakableLogic& logic );

		// Represents one grid in siged
		virtual void fOnClumpBroken( tUberBreakableLogic& logic, tUberBreakableClump& clump );

		// Represents one cell of the grid
		virtual void fOnCellBroken( tUberBreakableLogic& logic, tUberBreakablePiece& piece );

	};


	class base_export tUberBreakableLogic : public tLogic
	{
		define_dynamic_cast( tUberBreakableLogic, tLogic );
	public:
		typedef tUberBreakableDamagable* (*tCreateDamageableInterface)( tUberBreakableLogic& uberBreakable );
	public:
		static tCreateDamageableInterface gCreateDamageableInterface;
		static u32 cGroundFlags;
		static u32 cCollisionFlags;
		static u32 cDebrisFlags;
		static u32 cGameFlags;
		static u32 cOwnerEntityFlags;
		static b32 cCollideWithShapeSpatialSet;
		static f32 cDebrisMaxV;
		static b32 cAttachPieceLogic;
		static b32 cAcquireProperties;
		static const tStringPtr cOnDestroyFX;

	private:
		u32 mCurrentColorizedGroup;
		Time::tStopWatch mColorizerTimer;
		tGrowableArray<tUberBreakableClumpPtr> mClumps;
		tGrowableArray<tBreakMeshRefPtr> mBreakMeshes;
		tGrowableArray<tMeshEntity*> mPermaMeshes;
		tGrowableArray<tMeshEntity*> mUnbrokenMeshes;
		tGrowableArray<tMeshEntity*> mUnbrokenPermaMeshes;

		tUberBreakableDamagable* mDamageableInterface;
		b32 mFirstBreak;

		tStringPtr mOnDestroyEffect;
		tStringPtr mOnClumpDestroyEffect;
		tStringPtr mOnPieceDestroyEffect;

	public:
		tUberBreakableLogic( );
		virtual ~tUberBreakableLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );
		virtual Logic::tDamageable* fQueryDamageable( ) { return mDamageableInterface; }

		void fOnBreak( );
		f32 fQueryPercentageBroken( b32 includePiecesThatWillBreak ) const;

		void fSpawnDestroyFX( );
		void fSpawnDestroyFX( tUberBreakableClump& clump );
		void fSpawnDestroyFX( tUberBreakablePiece& piece );

		void fSetDestroyFX( const tStringPtr& path ) { mOnDestroyEffect = path; }
		void fSetClumpDestroyFX( const tStringPtr& path ) { mOnClumpDestroyEffect = path; }
		void fSetPieceDestroyFX( const tStringPtr& path ) { mOnPieceDestroyEffect = path; }

		const tStringPtr& fDestroyFX( ) const { return mOnDestroyEffect; }
		const tStringPtr& fClumpDestroyFX( ) const { return mOnClumpDestroyEffect; }
		const tStringPtr& fPieceDestroyFX( ) const { return mOnPieceDestroyEffect; }

		tUberBreakableDamagable* fDamageable( ) { return mDamageableInterface; }

        Math::tObbf fGetOBB ( );

	public:
		void fIntersect( const Math::tSpheref& worldSphere, tGrowableArray<tUberBreakablePiece*>& pieces );
	protected:
		void fGatherGridsAndMeshes( tEntity& root, tGrowableArray<t3dGridEntity*>& grids );
		void fConfigureInitialMeshStates( );
		void fStepDebugRendering( f32 dt );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};
}

#endif//__tUberBreakableLogic__
