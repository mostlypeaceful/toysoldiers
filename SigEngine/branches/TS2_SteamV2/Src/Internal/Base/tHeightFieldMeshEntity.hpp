#ifndef __tHeightFieldMeshEntity__
#define __tHeightFieldMeshEntity__
#include "tHeightFieldMesh.hpp"
#include "tReferenceFrameEntity.hpp"
#include "Gfx/tGroundCoverCloud.hpp"
#include "Gfx/tRenderableEntity.hpp"

namespace Sig
{

	///
	/// \brief Represents a single heightfield object as placed in the editor. Creates multiple
	/// tHeightFieldMeshEntity entities in fCollectEntities, representing the individual chunks
	/// that can be rendered, but still conceptually a single heightfield.
	class base_export tHeightFieldMeshEntityDef : public tHeightFieldMesh
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tHeightFieldMeshEntityDef, 0xE36FC9A1 );
	public:
		struct tChunkDesc
		{
			declare_reflector( );
		public:
			u32 mTriCount;
			u32 mVtxCount;
		public:
			tChunkDesc( ) : mTriCount( 0 ), mVtxCount( 0 ) { }
			tChunkDesc( u32 triCount, u32 vtxCount ) : mTriCount( triCount ), mVtxCount( vtxCount ) { }
			b32 fIsCulled( ) const { return mTriCount==0; }
		};
		typedef tDynamicArray<tChunkDesc> tChunkDescList;
	public:

		tChunkDescList					mChunkDescs;
		Gfx::tGroundCoverCloudDefList	mGroundCoverDefs;
		tLoadInPlaceResourcePtr*		mGeometryFile;
		Gfx::tMaterial*					mMaterial;

	public:
		tHeightFieldMeshEntityDef( );
		tHeightFieldMeshEntityDef( const Math::tVec2f& worldSpaceLengths, const Math::tVec2i& vertexRes );
		tHeightFieldMeshEntityDef( tNoOpTag );
		~tHeightFieldMeshEntityDef( );

		virtual b32 fHasRenderableBounds( ) const { return true; }
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;

	protected:

		virtual tHeightFieldRenderVertex* fLockRenderVerts( u32 startVertex = 0, u32 numVerts = ~0 );
		virtual void fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts );
		virtual u16* fLockRenderIds( u32 startIndex = 0, u32 numIds = ~0 );
		virtual void fUnlockRenderIds( u16* lockedIds );
	};

	class base_export tHeightFieldMeshReferenceFrameEntity : public tReferenceFrameEntity
	{
		define_dynamic_cast( tHeightFieldMeshReferenceFrameEntity, tReferenceFrameEntity );
	public:
		explicit tHeightFieldMeshReferenceFrameEntity( const tEntityDefProperties* entityDef ) : tReferenceFrameEntity( entityDef ) { }
		tHeightFieldMeshReferenceFrameEntity( const tEntityDefProperties* entityDef, const Math::tMat3f& objectToWorld ) : tReferenceFrameEntity( entityDef, objectToWorld ) { }
	};

	///
	/// \brief Represents a single renderable portion of a larger heightfield object.
	class base_export tHeightFieldMeshEntity : public Gfx::tRenderableEntity
	{
		define_dynamic_cast( tHeightFieldMeshEntity, Gfx::tRenderableEntity );
	private:
		const tHeightFieldMesh* mEntityDef; // FIXME raw pointer is not so safe
	public:
		tHeightFieldMeshEntity( 
			const Gfx::tRenderBatchPtr& batchPtr,
			const tHeightFieldMesh* entityDef,
			const Math::tAabbf& objectSpaceBox );
		//virtual u32	fSpatialSetIndex( ) const { return fUseEffectSpatialSet( ) ? cEffectSpatialSetIndex : cHeightFieldSpatialSetIndex; }
		virtual void fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		virtual void fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32 fIntersects( const Math::tFrustumf& v ) const;
		void fFakeRayCastUpDown( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		f32 fSampleHeight( const Math::tVec3f& worldPos ) const;
		b32 fOffHeightField( const Math::tVec3f& worldPos ) const; // basically, outside of heightfield in XZ plane, or else below
		const tHeightFieldMesh* fEntityDef( ) const { return mEntityDef; }
		virtual const tEntityDef* fQueryEntityDef( ) const { return mEntityDef; }

		// from tRenderInstance
		virtual const Math::tMat3f* fRI_ObjectToLocal( ) const { return &mEntityDef->mObjectToLocal; }
		virtual const Math::tMat3f* fRI_LocalToObject( ) const { return &mEntityDef->mLocalToObject; }
	};


}

#endif//__tHeightFieldMeshEntity__

