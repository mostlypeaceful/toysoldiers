#ifndef __tUberBreakableClump__
#define __tUberBreakableClump__
#include "tUberBreakablePiece.hpp"

namespace Sig
{
	class tUberBreakableClump;
	typedef tRefCounterPtr<tUberBreakableClump> tUberBreakableClumpPtr;
	class tUberBreakableClump : public tRefCounter
	{
		tEntityPtr mOwnerEntity;
		Math::tAabbf mObjSpaceBounds;
		Math::tVec3u mNumSegments;
		tGrowableArray<tUberBreakablePiecePtr> mPieces;
		u32 mCurrentColorizedPiece;

		b8 mHasBroke;
		b8 pad0;
		b8 pad1;
		b8 pad2;

	public:
		tUberBreakableClump( tEntity& ownerEntity, const Math::tAabbf& objSpaceBounds, const Math::tVec3u& numSegments );
		void fComputeSubdivision( const tGrowableArray<tBreakMeshRefPtr>& meshes );
		void fLinkClumps( const tGrowableArray<tUberBreakableClumpPtr>& clumps );
		void fOnDelete( );
		void fThinkST( f32 dt );
		void fIntersect( const Math::tSpheref& worldSphere, tGrowableArray<tUberBreakablePiece*>& pieces );
		void fQueryBrokenPieceCounts( u32& numPiecesBroken, u32& numPiecesTotal, b32 includePiecesThatWillBreak );

		const Math::tAabbf& fObjSpaceBounds( ) const { return mObjSpaceBounds; }

	protected:
		u32 fLinearPieceIndex( u32 x, u32 y, u32 z ) const;

		// Setup
		void fCollectPieces( );
		void fComputeAdjacentPieces( );
		void fAssignMeshesToPieces( const tGrowableArray<tBreakMeshRefPtr>& meshes );
		void fRemoveEmptyPieces( );
		void fLinkClumps( tUberBreakablePiece& piece, b32 up, const tGrowableArray<tUberBreakableClumpPtr>& clumps );

		// Runtime
		void fReleaseHitPieces( f32 dt );

	public: // debug stuff
		b32 fStepDebugRendering( f32 dt );
		void fRenderDebug( );
	};
}

#endif//__tUberBreakableClump__
