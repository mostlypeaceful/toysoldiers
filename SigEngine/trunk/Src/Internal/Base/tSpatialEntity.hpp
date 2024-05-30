#ifndef __tSpatialEntity__
#define __tSpatialEntity__
#include "tStateableEntity.hpp"

namespace Sig
{
	class base_export tSpatialEntity : public tStateableEntity
	{
		debug_watch( tSpatialEntity );
		declare_uncopyable( tSpatialEntity );
		define_dynamic_cast( tSpatialEntity, tStateableEntity );
	private:
		Math::tAabbf				mObjectSpaceBox;			///< represents the bounding aabb in object space
		Math::tAabbf				mWorldSpaceBox;
		u16							mLazyUpdateListIndex;
		u8							mWantsToBeInSpatialTree;
		u8							pad0;
		tEntity::tSpatialSetObjectPtr mSpatial;

	public:
		static void fSetObjectSpaceBoxOverride( tEntity& root, const Math::tAabbf& newObjSpaceBox );
	public:
		tSpatialEntity( );
		explicit tSpatialEntity( const Math::tAabbf& objectSpaceBox, u16 stateMask = tStateableEntity::cMaxStateMaskValue );
		virtual ~tSpatialEntity( );

		virtual void				fOnSpawn( );
		virtual void				fOnDelete( );
		void						fToObjectSpace( Math::tFrustumf& fInObject, const Math::tFrustumf& fInWorld ) const;
		void						fSetObjectSpaceBox( const Math::tAabbf& objectSpaceBox );
		void						fSetObjectSpaceBox( const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld );
		inline tSpatialSetObject*	fToSpatialSetObject( ) { return mSpatial.fGetRawPtr( ); }
		inline const tSpatialSetObject*	fToSpatialSetObject( ) const { return mSpatial.fGetRawPtr( ); }
		void						fRayCastDefault( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		
		virtual u32					fSpatialSetIndex( ) const { return 0; }

		//------------------------------------------------------------------------------
		// Culling
		//------------------------------------------------------------------------------
		virtual void				fSetNotPotentiallyVisible( b32 notPotentiallyVisible ) { } ///< N.B. not all spatial entities implement this
		virtual b32					fNotPotentiallyVisible( ) const { return false; } ///< N.B. not all spatial entities implement this

		//------------------------------------------------------------------------------
		// Collectors
		//------------------------------------------------------------------------------
		virtual void				fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const { }
		virtual void				fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const { }

		//------------------------------------------------------------------------------
		// Ray casts
		//------------------------------------------------------------------------------
		virtual void				fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;

		//------------------------------------------------------------------------------
		// Intersection tests
		//------------------------------------------------------------------------------
		virtual b32					fIntersects( const Math::tFrustumf& v ) const;
		virtual b32					fIntersects( const Math::tAabbf& v ) const;
		virtual b32					fIntersects( const Math::tObbf& v ) const;
		virtual b32					fIntersects( const Math::tSpheref& v ) const;

		const Math::tAabbf&			fObjectSpaceBox( ) const			{ return mObjectSpaceBox; }
		const Math::tAabbf&			fWorldSpaceBox( ) const				{ return mWorldSpaceBox; }

		inline void					fResetLazyUpdateListIndex( )			{ mLazyUpdateListIndex = 0xFFFF; }
		inline void					fSetLazyUpdateListIndex( u16 index )	{ mLazyUpdateListIndex = index; }
		inline b32					fLazyUpdateListIndexInvalid( ) const	{ return (mLazyUpdateListIndex == 0xFFFF); }
		inline u16					fLazyUpdateListIndex( ) const			{ return mLazyUpdateListIndex; }

	protected:
		virtual void				fOnMoved( b32 recomputeParentRelative  );
		void						fRecomputeWorldBounds( );

		void fRemoveFromSpatialTree( );
		void fAddToSpatialTree( );
	};


}

#endif//__tSpatialEntity__
