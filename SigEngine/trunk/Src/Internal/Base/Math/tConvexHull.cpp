#include "BasePch.hpp"
#include "tConvexHull.hpp"
#include "tMeshEntity.hpp"
#include "tMesh.hpp"

// for hull quick tests
#include "Physics/tGJK.hpp"
#include "Physics/tSupportMapping.hpp"

namespace Sig { namespace Math
{

	devvar( f32, Physics_HullThresh, 0.2f );

	namespace
	{
		const f32 cOutsideFaceEpsilon = 0.005f;
		const f32 cSamePtEpsilon = 0.01f;
	}


	tConvexHull::tFace::tFace( u16 a, u16 b, u16 c )
		: mA( a )
		, mB( b )
		, mC( c )
	{
		sigassert( a != b );
		sigassert( a != c );
		sigassert( b != c );

		sigassert( a <= std::numeric_limits<u16>::max( ) );
		sigassert( b <= std::numeric_limits<u16>::max( ) );
		sigassert( c <= std::numeric_limits<u16>::max( ) );
	}

	void tConvexHull::tFace::fFlip( )
	{
		fSwap( mA, mB );
	}

	tVec3f tConvexHull::tFace::fNormal( const tDynamicArray<tVec3f>& verts ) const
	{
		// matches tTriangle
		return (verts[ mB ] - verts[ mA ] ).fCross( verts[ mC ] - verts[ mB ] );
	}

	tTrianglef tConvexHull::fMakeTriangle( u32 face ) const
	{
		tTrianglef tri;
		tri.mA = mVerts[ mFaces[ face ].mA ];
		tri.mB = mVerts[ mFaces[ face ].mB ];
		tri.mC = mVerts[ mFaces[ face ].mC ];
		return tri;
	}

	void tConvexHull::fComputeNormal( u32 face )
	{
		sigassert( face > 0 && "Must compute first face normal manually" );
		tVec3f normal = mFaces[ face ].fNormal( mVerts );
		normal.fNormalize( );

		const tVec3f& faceVert = mVerts[ mFaces[ face ].mA ];

		for( u32 i = 0; i < mVerts.fCount( ); ++i )
		{
			// if any point already on the hull is outside the new face, the new face normal is wrong.
			if( (mVerts[ i ] - faceVert).fDot( normal ) >= cEpsilon )
			{
				mFaces[ face ].fFlip( );
				break;
			}
		}
	}

	namespace
	{
		b32 fCompareEdge( u32 a, u32 b, u32 a2, u32 b2 )
		{
			return (a == a2 && b == b2) || (a == b2 && b == a2);
		}

		struct tEdgeList
		{
			struct tEdge
			{
				u32 mA, mB;
				tEdge( u32 a = 0, u32 b = 0 ) 
					: mA( a ), mB( b )
				{ }
			};

			tGrowableArray<tEdge> mList;

			void fAddEdge( u32 a, u32 b )
			{
				sigassert( a != b );

				for( u32 i = 0; i < mList.fCount( ); ++i )
				{
					tEdge& e = mList[ i ];
					if( fCompareEdge( a, b, e.mA, e.mB ) )
					{
						//this edge was removed twice, it has been destroyed.
						mList.fErase( i );
						return;
					}
				}

				mList.fPushBack( tEdge( a, b ) );					
			}
		};
	}

	void tConvexHull::fConstruct( tMeshEntity* mesh, const Math::tMat3f& invTransform )
	{
		const tSubMesh* sm = mesh->fSubMesh( );

		tGrowableArray<tVec3f> verts;
		verts.fInsert( 0, sm->mVertices.fBegin( ), sm->mVertices.fCount( ) );

		fConstruct( verts, invTransform );
	}

	void tConvexHull::fConstruct( const tDynamicArray<tVec3f>& verts, const tDynamicArray<u32>& indices )
	{
		fClear( );

		mVerts.fDeleteArray( );
		mVerts.fInsert( 0, verts.fBegin( ), verts.fCount( ) );

		u32 faceCount = indices.fCount( ) / 3;
		mFaces.fNewArray( faceCount );
		for( u32 i = 0; i < faceCount; ++i )
			mFaces[ i ] = tConvexHull::tFace( indices[ i * 3 + 0 ], indices[ i * 3 + 1 ], indices[ i * 3 + 2 ] );

		//TODO? edges

		sigassert( fValid( ) );
	}

	void tConvexHull::fConstruct( const tGrowableArray<tVec3f>& points, const Math::tMat3f& invTransform )
	{
		fClear( );

		// Transform and add verts, ensure no duplicates
		tGrowableArray<tVec3f> verts;
		verts.fSetCapacity( points.fCount( ) );
		for( u32 i = 0; i < points.fCount( ); ++i )
		{
			tVec3f newVert = invTransform.fXformPoint( points[ i ] );

			b32 dup = false;
			for( u32 j = 0; j < verts.fCount( ); ++j )
			{
				if( verts[ j ].fEqual( newVert, cSamePtEpsilon ) )
				{
					dup = true;
					break;
				}
			}
			if( !dup )
				verts.fPushBack( newVert );
		}

		sigassert( verts.fCount( ) >= 4 );

		// make a face out of the farthest verts
		mVerts.fPushBack( verts[ 0 ] );
		verts.fErase( 0 );
		f32 farthest = -cInfinity;

		for( u32 i = 0; i < verts.fCount( ); ++i )
		{
			tVec3f& newVert = verts[ i ];
			f32 len = (newVert - mVerts[ 0 ]).fLength( );
			if( len > farthest )
			{
				farthest = len;
				fSwap( verts[ 0 ], newVert ); //keep the verts we used in the front of the array
			}
		}
		mVerts.fPushBack( verts[ 0 ] );
		verts.fErase( 0 );

		tVec3f triLeg1 = mVerts[ 1 ] - mVerts[ 0 ];

		farthest = -cInfinity;
		for( u32 i = 0; i < verts.fCount( ); ++i )
		{
			tVec3f& newVert = verts[ i ];
			f32 area = (newVert - mVerts[ 0 ]).fCross( triLeg1 ).fLengthSquared( );
			if( area > farthest )
			{
				farthest = area;
				fSwap( verts[ 0 ], newVert ); //keep the verts we used in the front of the array
			}
		}
		mVerts.fPushBack( verts[ 0 ] );
		verts.fErase( 0 );

		sigassert( mVerts.fCount( ) == 3 );
		mFaces.fPushBack( tFace( 0, 1, 2 ) );	
		tVec3f tri0Normal = mFaces[ 0 ].fNormal( mVerts );
		tri0Normal.fNormalize( ); //guaranteed to be safe due to the above search logic

		//find a forth vertex that's farthest outside
		farthest = -cInfinity;
		u32 nonPlanarVert = ~0;
		f32 nonPlanarDot = 0.f;
		for( u32 i = 0; i < verts.fCount( ); ++i )
		{
			const tVec3f& newVert = verts[ i ];
			f32 dot = (newVert - mVerts[ 0 ]).fDot( tri0Normal );
			f32 absDot = fAbs( dot );
			if( absDot > farthest )
			{
				farthest = absDot;
				nonPlanarVert = i;
				nonPlanarDot = dot;
			}
		}

		if( nonPlanarDot > 0 )
			mFaces[ 0 ].fFlip( );

		// build tetrahedron
		sigassert( nonPlanarVert != ~0 && "Degenerate hull." );
		mVerts.fPushBack( verts[ nonPlanarVert ] );
		verts.fErase( nonPlanarVert );

		mFaces.fPushBack( tFace( 0, 1, 3 ) );
		mFaces.fPushBack( tFace( 1, 2, 3 ) );	
		mFaces.fPushBack( tFace( 0, 2, 3 ) );
		for( u32 i = 1; i < 4; ++i )
			fComputeNormal( i );

		// add all the remaining vertices:
		// the forth vertex was removed, so we're still starting at 3 here.
		while( verts.fCount( ) )
		{
			//find farthest vert:
			farthest = cOutsideFaceEpsilon;
			u32 vert = ~0;
			for( s32 i = 0; i < (s32)verts.fCount( ); ++i )
			{
				const tVec3f& newVert = verts[ i ];

				b32 insideHull = true;

				for( s32 f = mFaces.fCount( ) - 1; f >= 0 ; --f )
				{
					tFace& face = mFaces[ f ];
					const tVec3f& triVert = mVerts[ face.mA ];

					f32 dot = (newVert - triVert).fDot( face.fNormal( mVerts ).fNormalize( ) );
					if( dot >= farthest )
					{
						farthest = dot;
						vert = i;
					}

					if( dot >= 0.f )
						insideHull = false;
				}

				if( insideHull )
				{
					sigassert( vert != i );
					verts.fErase( i );
				}
			}

			if( vert != ~0 )
			{
				const tVec3f& newVert = verts[ vert ];

				tEdgeList createdEdges;

				for( s32 f = mFaces.fCount( ) - 1; f >= 0 ; --f )
				{
					tFace& face = mFaces[ f ];
					const tVec3f& triVert = mVerts[ face.mA ];
					if( (newVert - triVert).fDot( face.fNormal( mVerts ).fNormalize( ) ) >= 0.f )
					{
						//point is outside this face.
						createdEdges.fAddEdge( face.mA, face.mB );
						createdEdges.fAddEdge( face.mB, face.mC );
						createdEdges.fAddEdge( face.mC, face.mA );
						mFaces.fErase( f );
					}
				}

				if( createdEdges.mList.fCount( ) )
				{
					mVerts.fPushBack( newVert );

					u32 originalFaceCount = mFaces.fCount( );
					for( u32 e = 0; e < createdEdges.mList.fCount( ); ++e )
						mFaces.fPushBack( tFace( createdEdges.mList[ e ].mA, createdEdges.mList[ e ].mB, mVerts.fCount( )-1 ) );

					for( u32 f = originalFaceCount; f < mFaces.fCount( ); ++f )
						fComputeNormal( f );
				}

				verts.fErase( vert );
			}
			else
				break;
		}

		// remove unreferenced verts
		tGrowableArray<u32> usedVerts;
		usedVerts.fSetCount( mVerts.fCount( ) );
		for( u32 i = 0; i < mVerts.fCount( ); ++i )
			usedVerts[ i ] = i;

		for( u32 i = 0; i < mFaces.fCount( ); ++i )
		{
			tFace& face = mFaces[ i ];
			usedVerts.fFindAndErase( face.mA );
			usedVerts.fFindAndErase( face.mB );
			usedVerts.fFindAndErase( face.mC );
		}

		std::sort( usedVerts.fBegin( ), usedVerts.fEnd( ) );
		for( s32 i = usedVerts.fCount( ) - 1; i >= 0; --i )
			fRemoveVertex( usedVerts[ i ] );

		// cache edges
		for( u32 i = 0; i < mFaces.fCount( ); ++i )
		{
			tFace& face = mFaces[ i ];
			fTestAddEdge( face.mA, face.mB );
			fTestAddEdge( face.mB, face.mC );
			fTestAddEdge( face.mA, face.mC );
		}

		sigassert( fValid( ) );
	}

	u32 tConvexHull::fTestAddEdge( u32 a, u32 b )
	{
		for( u32 j = 0; j < mEdges.fCount( ); ++j )
			if( fCompareEdge( a, b, mEdges[ j ].x, mEdges[ j ].y ) )
				return j;

		//wasn't found, add it
		mEdges.fPushBack( tVec2u( a, b ) );
		return mEdges.fCount( ) - 1;
	}

	void tConvexHull::fRemoveVertex( u32 vert )
	{
		mVerts.fEraseOrdered( vert );

		for( u32 i = 0; i < mFaces.fCount( ); ++i )
		{
			tFace& face = mFaces[ i ];
			sigassert( face.mA != vert && "Vert was still referenced" );
			sigassert( face.mB != vert && "Vert was still referenced" );
			sigassert( face.mC != vert && "Vert was still referenced" );
			if( face.mA > vert ) --face.mA;
			if( face.mB > vert ) --face.mB;
			if( face.mC > vert ) --face.mC;
		}
	}

	b32 tConvexHull::fTestRemoveEdge( u32 face, u32 a, u32 b )
	{
		tVec3f edge = mVerts[ b ] - mVerts[ a ];
		if( edge.fLength( ) < Physics_HullThresh )
		{
			// Edge is short
			// Copy the hull
			// Remove faces and vertex
			// Test hull for validity
			// If valid, save results
			tConvexHull testhull = *this;

			u32 removeVert = b;
			u32 replaceWithVert = a;
			sigassert( removeVert != replaceWithVert );

			testhull.mFaces.fErase( face );

			// ensure there is only one other face sharing this edge
			u32 faceFound = false;
			for( s32 i = testhull.mFaces.fCount( ) - 1; i >= 0; --i )
			{
				//find other face
				if( fCompareEdge( a, b, testhull.mFaces[ i ].mA, testhull.mFaces[ i ].mB )
					|| fCompareEdge( a, b, testhull.mFaces[ i ].mB, testhull.mFaces[ i ].mC )
					|| fCompareEdge( a, b, testhull.mFaces[ i ].mA, testhull.mFaces[ i ].mC ))
				{
					sigassert( !faceFound );
					faceFound = true;
					testhull.mFaces.fErase( i );
				}
			}
			sigassert( faceFound );

			for( u32 i = 0; i < testhull.mFaces.fCount( ); ++i )
			{
				// update dead vertex
				tFace& face = testhull.mFaces[ i ];
				if( face.mA == removeVert ) face.mA = replaceWithVert;
				if( face.mB == removeVert ) face.mB = replaceWithVert;
				if( face.mC == removeVert ) face.mC = replaceWithVert;
			}

			testhull.fRemoveVertex( removeVert );
			if( testhull.fValid( ) )
			{
				*this = testhull;
				return true;
			}
		}

		return false;
	}

	void tConvexHull::fOptimize( )
	{
		sigassert( fValid( ) );

		// Tries to collapse an edge on a temporary hull.
		//  If the resultant hull is still valid, the collapsed mesh is copied back to this one until
		//  this process can no longer be repeated.

		b32 searching = true;
		while( searching )
		{
			searching = false;

			for( s32 i = mFaces.fCount( ) - 1; i >= 0; --i )
			{
				tFace& face = mFaces[ i ];
				if( fTestRemoveEdge( i, face.mA, face.mB )
					|| fTestRemoveEdge( i, face.mB, face.mC )
					|| fTestRemoveEdge( i, face.mA, face.mC ) )
				{
					searching = true;
					break;
				}
			}

			if( !searching )
				break;
		}

		sigassert( fValid( ) );
	}

	tConvexHull tConvexHull::fTransform( const Math::tMat3f& xform ) const
	{
		sigassert( xform.fGetScale( ).fEqual( tVec3f::cOnesVector ) && "Scaling not supported!" );

		tConvexHull newHull = *this;

		for( u32 i = 0; i < mVerts.fCount( ); ++i )
			newHull.mVerts[ i ] = xform.fXformPoint( newHull.mVerts[ i ] );

		return newHull;
	}

	tAabbf tConvexHull::fToAABB( ) const
	{
		tAabbf bounds;
		bounds.fInvalidate( );

		for( u32 i = 0; i < mVerts.fCount( ); ++i )
			bounds |= mVerts[ i ];

		return bounds;
	}

	tVec3f tConvexHull::fComputeCenter( ) const
	{
		tVec3f center = tVec3f::cZeroVector;

		if( mVerts.fCount( ) )
		{
			for( u32 i = 0; i < mVerts.fCount( ); ++i )
				center += mVerts[ i ];
			center /= (f32)mVerts.fCount( );
		}

		return center;
	}

	u32 tConvexHull::fSupportingVertexIndex( const Math::tVec3f& dir ) const
	{
		u32 best = 0;
		f32 maxD = mVerts[ 0 ].fDot( dir );

		for( u32 i = 1; i < mVerts.fCount( ); ++i )
		{
			f32 d = mVerts[ i ].fDot( dir );
			if( d > maxD )
			{
				maxD = d;
				best = i;
			}
		}

		return best;
	}

	tVec3f tConvexHull::fSupportingVertex( const Math::tVec3f& dir ) const
	{
		return mVerts[ fSupportingVertexIndex( dir ) ];
	}

	b32 tConvexHull::fContainsPtGJK( const Math::tMat3f& hullXform, const Math::tVec3f& pt ) const
	{
		Physics::tGJK gjk( NEW Physics::tHullSupportMapping( this, Math::tMat3f::cIdentity, 0 ), NEW Physics::tPointSphereSupportMapping( pt, 0 ) );
		gjk.fA( )->fSetWorldXform( hullXform );
		gjk.fCompute( );
		return gjk.fIntersects( );
	}

	b32 tConvexHull::fValid( ) const
	{
		for( u32 f = 0; f < mFaces.fCount( ); ++f )
		{
			// ensure that there is only one other face with this edge:
			u32 shareCount = 0;
			for( u32 f2 = 0; f2 < mFaces.fCount( ); ++f2 )
			{
				if( fCompareEdge( mFaces[ f ].mA, mFaces[ f ].mB, mFaces[ f2 ].mA, mFaces[ f2 ].mB )
					|| fCompareEdge( mFaces[ f ].mA, mFaces[ f ].mB, mFaces[ f2 ].mB, mFaces[ f2 ].mC )
					|| fCompareEdge( mFaces[ f ].mA, mFaces[ f ].mB, mFaces[ f2 ].mA, mFaces[ f2 ].mC ) )
					++shareCount;
			}
			if( shareCount > 2 )
				return false;

			tTrianglef tri = fMakeTriangle( f );
			tPlanef plane( tri.fComputeUnitNormal( ), tri.mA );

			for( u32 v = 0; v < mVerts.fCount( ); ++v )
			{
				// ensure all vertices lie behind plane.
				if( plane.fSignedDistance( mVerts[ v ] ) > cEpsilon )
					return false;
			}
		}

		return true;
	}

	
	const tCapsule tCapsule::cNonZeroSized = tCapsule( tVec3f::cZeroVector, tVec3f::cYAxis, 1.f, 1.f );

}}

