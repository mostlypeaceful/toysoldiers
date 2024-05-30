#ifndef __tSpatialEntity__
#define __tSpatialEntity__
#include "tStateableEntity.hpp"

namespace Sig
{
	define_smart_ptr( base_export, tRefCounterPtr, tSpatialEntity );

	///
	/// \brief
	class base_export tSpatialEntity : public tStateableEntity, public tEntity::tSpatialSetObject
	{
		friend class tEntity;
		define_dynamic_cast( tSpatialEntity, tStateableEntity );
	private:
		Math::tAabbf				mObjectSpaceBox;			///< represents the bounding aabb in object space
		u16							mLazyUpdateListIndex;
		u16							pad0;

	public:
		static void fSetObjectSpaceBoxOverride( tEntity& root, const Math::tAabbf& newObjSpaceBox );
	public:
		tSpatialEntity( );
		explicit tSpatialEntity( const Math::tAabbf& objectSpaceBox, u16 stateMask = tStateableEntity::cMaxStateMaskValue );
		virtual void				fOnSpawn( );
		virtual void				fOnDelete( );
		void						fToObjectSpace( Math::tFrustumf& fInObject, const Math::tFrustumf& fInWorld ) const;
		void						fSetObjectSpaceBox( const Math::tAabbf& objectSpaceBox );
		void						fSetObjectSpaceBox( const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld );
		inline tSpatialSetObject*	fToSpatialSetObject( ) { return this; }
		void						fRayCastDefault( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual u32					fSpatialSetIndex( ) const { return 0; }
		virtual void				fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const { }
		virtual void				fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const { }
		virtual void				fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32					fIntersects( const Math::tFrustumf& v ) const;
		virtual b32					fIntersects( const Math::tAabbf& v ) const;
		virtual b32					fIntersects( const Math::tObbf& v ) const;
		virtual b32					fIntersects( const Math::tSpheref& v ) const;

		const Math::tAabbf&			fObjectSpaceBox( ) const			{ return mObjectSpaceBox; }
		const Math::tAabbf&			fWorldSpaceBox( ) const				{ return tEntity::tSpatialSetObject::mWorldSpaceBox; }

		inline void					tSpatialEntity::fResetLazyUpdateListIndex( )			{ mLazyUpdateListIndex = 0xFFFF; }
		inline void					tSpatialEntity::fSetLazyUpdateListIndex( u16 index )	{ mLazyUpdateListIndex = index; }
		inline b32					tSpatialEntity::fLazyUpdateListIndexInvalid( ) const	{ return (mLazyUpdateListIndex == 0xFFFF); }
		inline u16					tSpatialEntity::fLazyUpdateListIndex( ) const			{ return mLazyUpdateListIndex; }

	protected:
		virtual void				fOnMoved( b32 recomputeParentRelative  );
		void						fRecomputeWorldBounds( );
	};


}

#endif//__tSpatialEntity__
