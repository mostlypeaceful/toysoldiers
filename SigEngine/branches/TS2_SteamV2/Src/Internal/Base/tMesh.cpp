#include "BasePch.hpp"
#include "tMesh.hpp"
#include "tPolySoupOctree.hpp"
#include "Gfx/tGeometryBufferVRam.hpp"
#include "Gfx/tDevice.hpp"

// intersections
#include "Math/tIntersectionAabbFrustum.hpp"



namespace Sig
{
	///
	/// \section tSubMesh
	///

	tSubMesh::tSubMesh( )
		: mGeometryBufferIndex( 0 )
		, mIndexBufferIndex( 0 )
		, mMaterial( 0 )
		, mVertexFormat( 0 )
	{
		mBounds.fInvalidate( );
	}

	tSubMesh::tSubMesh( tNoOpTag )
		: mBounds( cNoOpTag )
		, mVertices( cNoOpTag )
		, mTriangles( cNoOpTag )
	{
	}

	tSubMesh::~tSubMesh( )
	{
	}

	void tSubMesh::fOnFileLoaded( )
	{
		// vertex formats may be shared amongst sub-meshes, so
		// only allocate if the object hasn't yet been allocated
		if( !mVertexFormat->fAllocated( ) )
			mVertexFormat->fAllocateInPlace( Gfx::tDevice::fGetDefaultDevice( ) );
	}

	void tSubMesh::fOnFileUnloading( )
	{
		// vertex formats may be shared amongst sub-meshes, so
		// only deallocate if the object hasn't yet been deallocated
		if( mVertexFormat->fAllocated( ) )
			mVertexFormat->fDeallocateInPlace( );
	}

	void tSubMesh::fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		mPolySoupKDTree.fCollectTris( trisOut, aabb, mVertices, mTriangles );
	}

	b32 tSubMesh::fTestRay( const Math::tRayf& rayInObject, tPolySoupRayCastHit& bestHit ) const
	{
		bestHit = tPolySoupRayCastHit( );

		mPolySoupKDTree.fTestRay( bestHit, rayInObject, mVertices, mTriangles );

		return bestHit.fHit( );
	}

	b32 tSubMesh::fTestFrustum( const Math::tFrustumf& frustumInObject ) const
	{
		return mPolySoupKDTree.fTestFrustum( frustumInObject, mVertices, mTriangles );
	}

	///
	/// \section tSkin::tInfluence
	///

	tSkin::tInfluence::tInfluence( )
		: mName( 0 )
	{
	}

	tSkin::tInfluence::tInfluence( tNoOpTag )
	{
	}

	///
	/// \section tSkin
	///

	tSkin::tSkin( )
	{
	}

	tSkin::tSkin( tNoOpTag )
		: mInfluences( cNoOpTag )
	{
	}

	///
	/// \section tMesh
	///

	tMesh::tMesh( )
		: mGeometryFile( 0 )
		, mSkin( 0 )
	{
		mBounds.fInvalidate( );
	}

	tMesh::tMesh( tNoOpTag )
		: mBounds( cNoOpTag )
		, mSubMeshes( cNoOpTag )
	{
	}

	tMesh::~tMesh( )
	{
		delete mSkin;
	}

	void tMesh::fOnFileLoaded( )
	{
		for( u32 i = 0; i < mSubMeshes.fCount( ); ++i )
			mSubMeshes[i].fOnFileLoaded( );
	}

	void tMesh::fOnFileUnloading( )
	{
		for( u32 i = 0; i < mSubMeshes.fCount( ); ++i )
			mSubMeshes[i].fOnFileUnloading( );
	}

	void tMesh::fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		// collect tris from all submeshes
		for( u32 i = 0; i < mSubMeshes.fCount( ); ++i )
			mSubMeshes[ i ].fCollectTris( aabb, trisOut );
	}

	b32 tMesh::fTestRay( const Math::tRayf& rayInObject, tPolySoupRayCastHit& bestHit ) const
	{
		bestHit = tPolySoupRayCastHit( );

		if( !mBounds.fIntersectsOrContains( rayInObject ) )
			return false;

		// test against each sub-mesh
		for( u32 i = 0; i < mSubMeshes.fCount( ); ++i )
		{
			tPolySoupRayCastHit hit;
			if( mSubMeshes[ i ].fTestRay( rayInObject, hit ) && hit.mT < bestHit.mT )
			{
				bestHit = hit;
			}
		}

		return bestHit.fHit( );
	}

	b32 tMesh::fTestFrustum( const Math::tFrustumf& frustumInObject ) const
	{
		Math::tIntersectionAabbFrustum<f32> quickReject( frustumInObject, mBounds );
		if( quickReject.fContained( ) )
			return true;
		if( !quickReject.fIntersects( ) )
			return false;

		// test against each sub-mesh
		for( u32 i = 0; i < mSubMeshes.fCount( ); ++i )
			if( mSubMeshes[ i ].fTestFrustum( frustumInObject ) )
				return true;

		return false;
	}

}

