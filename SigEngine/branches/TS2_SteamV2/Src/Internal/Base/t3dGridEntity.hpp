#ifndef __t3dGridEntity__
#define __t3dGridEntity__
#include "tEntityDef.hpp"
#include "tSpatialEntity.hpp"

namespace Sig
{
	class base_export t3dGridEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( t3dGridEntityDef, 0x784929C6 );
	public:
		Math::tVec3u mCellCounts;
	public:
		t3dGridEntityDef( );
		t3dGridEntityDef( tNoOpTag );
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};

	class base_export t3dGridEntity : public tSpatialEntity
	{
		define_dynamic_cast( t3dGridEntity, tSpatialEntity );
	public:
		t3dGridEntity( const t3dGridEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld );
		explicit t3dGridEntity( const Math::tAabbf& objectSpaceBox, const Math::tVec3u& cellCounts );

		const Math::tVec3u& fCellCounts( ) const { return mCellCounts; }

		virtual void fPropagateSkeleton( tAnimatedSkeleton& skeleton );
		virtual b32 fIsHelper( ) const { return true; }
		virtual u32	fSpatialSetIndex( ) const { return ~0; } // no spatial set for 3d grids
		b32 fContains( const Math::tVec3f& point ) const;
		b32 fIntersects( const t3dGridEntity& otherShape ) const;
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32 fIntersects( const Math::tFrustumf& v ) const;
		virtual b32 fIntersects( const Math::tAabbf& v ) const;
		virtual b32 fIntersects( const Math::tObbf& v ) const;
		virtual b32 fIntersects( const Math::tSpheref& v ) const;
		virtual void fOnMoved( b32 recomputeParentRelative );
		inline const Math::tObbf& fBox( ) const { return mBox; }
		Math::tObbf fParentRelativeBox( ) const;
		Math::tVec3f fClosestPoint( const Math::tVec3f& point ) const;

	private:
		Math::tObbf fComputeBox( ) const;

	private:
		const t3dGridEntityDef*		mEntityDef;
		Math::tObbf					mBox;
		Math::tVec3u				mCellCounts;
	};

	define_smart_ptr( base_export, tRefCounterPtr, t3dGridEntity );

}

#endif//__t3dGridEntity__
