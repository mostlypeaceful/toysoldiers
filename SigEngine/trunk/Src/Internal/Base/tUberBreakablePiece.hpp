//------------------------------------------------------------------------------
// \file tUberBreakablePiece.hpp - 06 Sep 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tUberBreakablePiece__
#define __tUberBreakablePiece__
#include "tShapeEntity.hpp"
#include "tMeshEntity.hpp"

namespace Sig
{


	///
	/// \class tUberBreakblePieceLogic
	/// \brief 
	class base_export tUberBreakblePieceLogic : public tLogic
	{
		define_dynamic_cast( tUberBreakblePieceLogic, tLogic );
	};

	///
	/// \class tBreakMeshRef
	/// \brief 
	struct base_export tBreakMeshRef : public tRefCounter
	{
		tMeshEntityPtr mMesh;

		tBreakMeshRef( tMeshEntity* mesh )
			: mMesh( mesh )
		{ }
	};

	typedef tRefCounterPtr< tBreakMeshRef > tBreakMeshRefPtr;

	class tUberBreakablePiece;
	typedef tRefCounterPtr<tUberBreakablePiece> tUberBreakablePiecePtr;

	///
	/// \class tUberBreakablePiece
	/// \brief 
	class base_export tUberBreakablePiece : public tBoxEntity
	{
		define_dynamic_cast( tUberBreakablePiece, tBoxEntity );
		define_class_pool_new_delete( tUberBreakablePiece, 256 );
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
