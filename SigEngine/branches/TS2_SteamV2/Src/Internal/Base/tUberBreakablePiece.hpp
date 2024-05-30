#ifndef __tUberBreakablePiece__
#define __tUberBreakablePiece__
#include "tShapeEntity.hpp"
#include "tMeshEntity.hpp"

namespace Sig
{

	class tUberBreakblePieceLogic : public tLogic
	{
		define_dynamic_cast( tUberBreakblePieceLogic, tLogic );
	};

	struct tBreakMeshRef : public tRefCounter
	{
		tMeshEntityPtr mMesh;

		tBreakMeshRef( tMeshEntity* mesh )
			: mMesh( mesh )
		{ }
	};

	typedef tRefCounterPtr< tBreakMeshRef > tBreakMeshRefPtr;

	class tUberBreakablePiece;
	typedef tRefCounterPtr<tUberBreakablePiece> tUberBreakablePiecePtr;
	class tUberBreakablePiece : public tShapeEntity
	{
		define_dynamic_cast( tUberBreakablePiece, tShapeEntity );
	public:
		enum tPosition
		{
			cPosTop			= ( 1 << 0 ),
			cPosBottom		= ( 1 << 1 ),
		};
	public:
		u32 mPositionFlags;
		tGrowableArray<tUberBreakablePiecePtr> mAdjacent;
		tGrowableArray<tUberBreakablePiecePtr> mAbove;
		tGrowableArray<tUberBreakablePiecePtr> mBelow;
		tGrowableArray<tBreakMeshRefPtr> mMeshes;
		b32 mHit;
		f32 mCollapseTimer;
		Math::tVec3f mDamageVelocity;
	public:
		tUberBreakablePiece( const Math::tAabbf& objectSpaceBox );
		void fReleaseReferences( );
		void fOnHit( const Math::tVec3f& damageVelocity );
		void fHandleHit( f32 dt );
		b32 fIsBroken( ) const { return mHit || !fSceneGraph( ); }
		b32 fWillBreakSoonDueToLackOfSupport( ) const;
		b32  fDeleteIfEmpty( );
		Math::tRayf fComputeProbeUp( ) const;
		Math::tRayf fComputeProbeDown( ) const;
	};
}

#endif//__tUberBreakablePiece__
