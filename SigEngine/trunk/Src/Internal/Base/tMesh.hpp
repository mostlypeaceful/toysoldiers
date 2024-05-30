#ifndef __tMesh__
#define __tMesh__
#include "tPolySoupOctree.hpp"
#include "tPolySoupKDTree.hpp"

namespace Sig { namespace Gfx
{
	class tMaterial;
	class tVertexFormatVRam;
}}

namespace Sig
{
	class tLoadInPlaceResourcePtr;

	///
	/// \brief The sub mesh binds a single set of geometry with a single material,
	/// both of which may be referenced or shared. 
	class base_export tSubMesh
	{
		declare_reflector( );
	public:

		// raycast geometry for the sub-mesh
		Math::tAabbf			mBounds;
		tPolySoupVertexList		mVertices;
		tPolySoupTriangleList	mTriangles;
		tPolySoupKDTree			mPolySoupKDTree;

		// index into parent mesh's geometry file's array of geometry buffers
		u32						mGeometryBufferIndex;

		// index into parent mesh's geometry file's array of index buffers
		u32						mIndexBufferIndex;

		// material for this sub-mesh
		Gfx::tMaterial*			mMaterial;

		// this vertex format represents a "modified"
		// vertex format, one with the minimal semantics
		// for the material, but with the correct offsets for
		// this specific piece of geometry
		Gfx::tVertexFormatVRam*	mVertexFormat;

	public:
		tSubMesh( );
		tSubMesh( tNoOpTag );
		~tSubMesh( );
		void fOnFileLoaded( );
		void fOnFileUnloading( );
		void fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		b32  fTestRay( const Math::tRayf& rayInObject, tPolySoupRayCastHit& bestHit ) const;
		b32  fTestFrustum( const Math::tFrustumf& frustumInObject ) const;
	};
	typedef tDynamicArray<tSubMesh> tSubMeshArray;

	class base_export tSkin
	{
		declare_reflector( );
	public:
		class base_export tInfluence
		{
			declare_reflector( );
		public:
			tLoadInPlaceStringPtr* mName;
		public:
			tInfluence( );
			tInfluence( tNoOpTag );
		};
		typedef tDynamicArray< tInfluence > tInfluenceList;
	public:
		tInfluenceList mInfluences;
	public:
		tSkin( );
		tSkin( tNoOpTag );
	};

	class base_export tMesh
	{
		declare_reflector( );
	public:
		Math::tAabbf				mBounds;
		tLoadInPlaceResourcePtr*	mGeometryFile;
		tSubMeshArray				mSubMeshes;
		tSkin*						mSkin;

	public:
		tMesh( );
		tMesh( tNoOpTag );
		~tMesh( );
		b32 fIsSkinned( ) const { return mSkin != 0; }
		void fOnFileLoaded( );
		void fOnFileUnloading( );
		void fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		b32  fTestRay( const Math::tRayf& rayInObject, tPolySoupRayCastHit& bestHit ) const;
		b32  fTestFrustum( const Math::tFrustumf& frustumInObject ) const;
	};

}


#endif//__tMesh__
