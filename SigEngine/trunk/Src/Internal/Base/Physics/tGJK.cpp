#include "BasePch.hpp"
#include "tGJK.hpp"
#include "Math/tIntersectionGeneralSAT.hpp"
#include "tPhysicsWorld.hpp" //for debug rendering only

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	// -----------------------------------------------------
	// See tGJK_Figures.png for illustrations of how this work.
	// -----------------------------------------------------

	// Set this true to spew some info about gjk as it's happening
	devvar( bool, Physics_Collision_GJK_Debug, false );
	devvar( bool, Physics_Collision_GJK_NormalizeSearch, true ); // for better numerical accuracy
	devvar_clamp( u32, Physics_Collision_GJK_MaxIterations, 100, 0, 100, 0 );

	// With this true, gjk will terminate simply for throwing out "a" the newest vertex. this is fast and simple but inaccurate, especially for raycast.
	devvar( bool, Physics_Collision_GJK_DegenerateTerm, false );

	// This tolerance method is a distance progression check on the 'closest point' solution, tends to be overly conservative, early outing erroneously.
	devvar( bool, Physics_Collision_GJK_ToleranceDo, true );
	devvar_clamp( f32, Physics_Collision_GJK_Tolerance, 0.001f, -1.f, 1.f, 6 ); // this number is critical do not tweak

	// This tolerance method simply checks that the new vertex is forward along the search direction.
	devvar( bool, Physics_Collision_GJK_ChristerToleranceDo, true );
	devvar_clamp( f32, Physics_Collision_GJK_ChristerTolerance, 0.001f, -1.f, 1.f, 3 );

	devvar( bool, Physics_Collision_GJK_RaycastDraw, false );
	devvar_clamp( f32, Physics_Collision_GJK_RaycastTolerance, 0.001f, -1.f, 1.f, 5 );
	devvar_clamp( f32, Physics_Collision_GJK_RaycastDotTolerance, -0.00001f, -1.f, 1.f, 5 );
	

	namespace 
	{
		b32 fSameDirection( const tVec3f& a, const tVec3f& b )
		{
			return a.fDot( b ) > 0.f;
		}

		// UNNORMALIZED
		tVec3f fPlaneNormal( const tVec3f& a, const tVec3f& b, const tVec3f& c )
		{
			return (b-a).fCross( c-a );
		}

		tPlanef fUnnormalizedPlane( const tVec3f& a, const tVec3f& b, const tVec3f& c )
		{
			tVec3f normal = fPlaneNormal( a, b, c );
			return tPlanef( normal, a );
		}

		tVec3f fNormalizePlaneAndProjectOrigin( tPlanef& plane )
		{
			plane.fPlaneNormalize( );

			// project origin to plane
			tVec3f closestPt = plane.fGetNormal( ) * -plane.d;
			f32 dist = plane.fSignedDistance( closestPt );
			sigassert( fEqual( dist, 0.f, 0.01f ) );

			return closestPt;
		}

		b32 fOriginOutsidePlane( const tPlanef& plane )
		{
			return plane.d > 0.f;
		}

		tVec3f fEdgeToOriginDir( const tVec3f& ab, const tVec3f& ao )
		{
			return ab.fCross( ao.fCross( ab ) );
		}

		//GDC10 erin catto gjk
		// returns [u,v] where g = (a*u + b*v)
		tVec2f fBarycentricEdge( const tVec3f& a, const tVec3f& b )
		{
			tVec3f ab = b-a;

			f32 abLen;
			ab.fNormalize( abLen );
			sigassert( !fEqual( abLen, 0.f ) );

			tVec3f ao = -a;
			tVec3f ob = b;

			return tVec2f( ob.fDot( ab ) / abLen, ao.fDot( ab ) / abLen );
		}

		f32 fUnsignedTriArea( const tVec3f& a, const tVec3f& b, const tVec3f& c )
		{
			tVec3f cross = ( b-a ).fCross( c-a );
			return 0.5f * cross.fLength( );
		}

		f32 fSignedTriArea( const tVec3f& a, const tVec3f& b, const tVec3f& c, const tVec3f& norm )
		{
			tVec3f cross = ( b-a ).fCross( c-a );
			return 0.5f * cross.fLength( ) * fSign( norm.fDot( cross ) );
		}

		// http://mathforum.org/library/drmath/view/51837.html
		f32 fSignedTetraVolume( const tVec3f& a, const tVec3f& b, const tVec3f& c, const tVec3f& d )
		{
			tVec3f BxC = (b-a).fCross( (c-a) );
			return (1/6.f) * -(d-a).fDot( BxC );
		}

		tVec4f fBarycentricTetra( const tVec3f& a, const tVec3f& b, const tVec3f& c, const tVec3f& d )
		{
			f32 fullVolume = fSignedTetraVolume( a, b, c, d );
			sigassert( !fEqual( fullVolume, 0.f ) );

			tVec3f o = tVec3f::cZeroVector;
			return tVec4f( fSignedTetraVolume( d, c, b, o ) / fullVolume, fSignedTetraVolume( a, c, d, o ) / fullVolume, fSignedTetraVolume( a, d, b, o ) / fullVolume, fSignedTetraVolume( a, b, c, o ) / fullVolume );
		}

	}


	tVec3f tGJK::fBarycentricTri( const tVec3f& a, const tVec3f& b, const tVec3f& c )
	{
		tPlanef p = fUnnormalizedPlane( a, b, c );

		f32 aABC = fSignedTriArea( a, b, c, p.fGetNormal( ) );

		if( fEqual( aABC, 0.f, 0.0001f ) )
		{
			//log_line( 0, "degenerate tri :(" );

			//triangle is degenerate, collinear vertices
			// find an edge and use it
			tVec3f ab = b-a;
			if( ab.fLengthSquared( ) > 0.f )
			{
				tVec2f bary = fBarycentricEdge( a, b );
				return tVec3f( bary.x, bary.y, 0 );
			}
			else
			{
				tVec3f ac = c-a;
				if( ac.fLengthSquared( ) > 0.f )
				{
					tVec2f bary = fBarycentricEdge( a, c );
					return tVec3f( bary.x, 0, bary.y );
				}
				else
				{
					tVec3f bc = c-b;
					if( bc.fLengthSquared( ) > 0.f )
					{
						tVec2f bary = fBarycentricEdge( b, c );
						return tVec3f( 0, bary.x, bary.y );
					}
					else
					{
						sigassert( !"Completely degenerate triangle :(" );
					}
				}
			}
		}

		//// project origin to plane
		//p.fPlaneNormalize( );
		//tVec3f norm = p.fGetNormal( );

		//tVec3f o = norm * -p.d;
		//sigassert( fEqual( p.fSignedDistance( o ), 0.f, 0.01f ) );

		//f32 aBary = fSignedTriArea( o, b, c, norm ) / aABC;
		//f32 bBary = fSignedTriArea( o, c, a, norm ) / aABC;
		//f32 cBary = 1.f - aBary - bBary;
		//return tVec3f( aBary, bBary, cBary );



		//tVec3f ab = b - a;
		//tVec3f ac = c - a;
		//tVec3f ao = -a;

		//f32 bBar = ab.fDot( ao );
		//f32 cBar = ac.fDot( ao );
		//f32 aBar = 1.f - bBar - cBar;
		//return tVec3f( aBar, bBar, cBar );


		// http://adrianboeing.blogspot.com/2010/01/barycentric-coordinates.html
		tVec3f v0 = c - a;
		tVec3f v1 = b - a;
		tVec3f v2 = -a;

		f32 dot00 = v0.fDot( v0 );
		f32 dot01 = v0.fDot( v1 );
		f32 dot02 = v0.fDot( v2 );
		f32 dot11 = v1.fDot( v1 );
		f32 dot12 = v1.fDot( v2 );

		// Compute barycentric coordinates
		f32 invDenom = 1.f / (dot00 * dot11 - dot01 * dot01);
		f32 u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		f32 v = (dot00 * dot12 - dot01 * dot02) * invDenom;
		return tVec3f( 1.f - u - v, v, u );
	}

#ifdef sig_devmenu
	b32 tSimplex::operator == ( const tSimplex& other ) const
	{
		if( mVerts.fCount( ) != other.mVerts.fCount( ) )
			return false;

		for( u32 i = 0; i < mVerts.fCount( ); ++i )
			if( !(mVerts[ i ] == other.mVerts[ i ]) )
				return false;

		return true;
	}
#endif

	tGJK::tGJK( tSupportMapping* a, tSupportMapping* b, b32 forRaycast )
		: mA( a )
		, mB( b )
		, mForRaycast( forRaycast )
		, mIntersects( false )
		, mTerminate( true )
		, mClosestPtA( tVec3f::cZeroVector )
		, mClosestPtB( tVec3f::cZeroVector )
		, mClosestDist( 0.f )
		, mResumable( mA->fIndexable( ) && mB->fIndexable( ) )
	{
	}

	void tGJK::fReset( )
	{
		sigassert( mA && mB );

		mSimplex.fClear( );

		tVec3f firstDir = tVec3f::cYAxis;
		mSimplex.mVerts.fPushBack( fSupport( firstDir ) );
		mSimplex.mClosestPt = mSimplex.mVerts[ 0 ].mMinkowski;
		mClosestDist = mSimplex.mClosestPt.fLengthSquared( );
		mPenetration = 0;
		mNormal = tVec3f::cYAxis;

		// first direction is towards the origin.
		mSimplex.mSearchDir = -mSimplex.mClosestPt;

		mIntersects = false;
		mTerminate = false;
	}

	void tGJK::fResume( )
	{
		if( !mResumable )
		{
			fReset( );
			return;
		}

		for( u32 i = 0; i < mSimplex.mVerts.fCount( ); ++i )
		{
			tSimplexVertex& v = mSimplex.mVerts[ i ];
			mA->fWorldIndexedSupport( v.mA );
			mB->fWorldIndexedSupport( v.mB );
			v.mMinkowski = v.mA.mP - v.mB.mP;
		}

		mIntersects = false;
		mTerminate = false;
		mClosestDist = cInfinity;
		fEvolveSimplex( );
	}

	tSimplexVertex tGJK::fSupport( const Math::tVec3f& worldD )
	{
		tSupportMapping::tSupport a = mA->fWorldSupport( worldD );
		tSupportMapping::tSupport b = mB->fWorldSupport( -worldD );
		return tSimplexVertex( a, b, a.mP - b.mP );
	}

	void tGJK::fCompute( )
	{
		if( !mSimplex.mVerts.fCount( ) )
			fReset( );

		if( !mTerminate )
		{
			if_devmenu( mHistory.fClear( ); )

			// clamp iterations
			b32 clampedOut = true;
			u32 it = 0;
			for( ; it < Physics_Collision_GJK_MaxIterations; ++it )
			{
				if( Physics_Collision_GJK_NormalizeSearch )
					mSimplex.mSearchDir.fNormalizeSafe( );

				tSimplexVertex newPt = fSupport( mSimplex.mSearchDir );				
				fUpdateSimplex( newPt );

				if( mTerminate )
				{
					clampedOut = false;
					break;
				}
			}

			if( Physics_Collision_GJK_Debug )
				log_line( 0, "It: " << it );

#ifdef sig_devmenu
			if( clampedOut )
			{
				log_warning( "GJK " << (mForRaycast ? "Raycast" : "") << " maxed out. SimpA: " << mA->mType << " SimpB: " << mB->mType );
				//mHistory.fLog( );
			}
#endif
		}

		fUpdateClosestPts( );
	}

	void tGJK::fUpdateSimplex( const tSimplexVertex& newVert )
	{
		tSimplex::tVertList& verts = mSimplex.mVerts;
		sigassert( verts.fCount( ) && "simplex needs to be intialized!" );

		b32 alreadyContained = mResumable ? mSimplex.fContainsPt( newVert ) : mSimplex.fContainsPt( newVert.mMinkowski );
		if( alreadyContained )
		{
			mTerminate = true;

			if( Physics_Collision_GJK_Debug )
				log_line( 0, "Repeat Vert Term" );

			return;
		}

		if( Physics_Collision_GJK_ChristerToleranceDo )
		{
			// Termination condition from Real-Time Collision Detection - Christer Ericson
			//  New minkoswki point is no farther along the search direction than the previous closest pt.
			if( mSimplex.mSearchDir.fDot( newVert.mMinkowski - mSimplex.mClosestPt ) < Physics_Collision_GJK_ChristerTolerance )
			{
				mTerminate = true;

				if( Physics_Collision_GJK_Debug )
					log_line( 0, "Chirster Term" );

				return;
			}
		}

		// Something i was trying, to fix ping ponging. ensure new vertex is not the one we just threw out last frame.
		//if( !Physics_Collision_GJK_DegenerateTerm && mLastThrownOutSet && !alreadyContained )
		//{
		//	// alternate way to test for degeneracy. if this vertex was the same as the last vertex we through out.
		//	if( mResumable ? mLastThrownOutVertex == newVert : mLastThrownOutVertex == newVert.mMinkowski )
		//	{
		//		mTerminate = true;
		//		return;
		//	}
		//}

		verts.fPushBack( newVert );
		fEvolveSimplex( );

#ifdef sig_devmenu
		// This block catches failures in the termination condition.
		//  If we ever get here with mTerminate false and the last two mHistory frames equal each other, something went wrong.
		u32 hisCount = mHistory.mEvents.fCount( );
		if( Physics_Collision_GJK_Debug && !mTerminate && hisCount > 1 )
		{
			if( mHistory.mEvents[ hisCount - 1 ] == mHistory.mEvents[ hisCount - 2 ] )
			{
				mHistory.fLog( );
				sigassert( 0 );

				// logic is repeated here so you can step through it after it fails.
				verts.fPushBack( newVert );
				fEvolveSimplex( );
			}
		}
#endif
	}

	b32 tGJK::fShouldUpdate( const tVec3f& newVert )
	{
		if( Physics_Collision_GJK_ToleranceDo )
		{
			f32 vLen = newVert.fLengthSquared( );
			f32 distanceChanged = mClosestDist - vLen; // positive if we've moved towards zero.

			// need to move at least this much to keep going.
			if( distanceChanged < (Physics_Collision_GJK_Tolerance*Physics_Collision_GJK_Tolerance) )
			{
				// the newly added vertex was no good.
				mTerminate = true;
				mSimplex.mVerts.fPopBack( );
				if( Physics_Collision_GJK_Debug )
					log_line( 0, "Toler Term: " << vLen );
				return false;
			}

			mClosestDist = vLen;
			if( Physics_Collision_GJK_Debug )
				log_line( 0, "Dist: " << mClosestDist );
		}

		return true;
	}

#define vert_simplex( vert ) \
	if( fShouldUpdate( vert.mMinkowski ) ) \
	{ \
		mSimplex.mClosestPt = vert.mMinkowski; \
		mSimplex.mSearchDir = -vert.mMinkowski; \
		verts[ 0 ] = vert; \
		verts.fSetCount( 1 ); \
	} \

#define edge_simplex( vert1, vert2, edgeBary ) \
	tVec3f closestPt = vert1.mMinkowski * edgeBary.x + vert2.mMinkowski * edgeBary.y; \
	if( fShouldUpdate( closestPt ) ) \
	{ \
		mSimplex.mClosestPt = closestPt; \
		mSimplex.mSearchDir = fEdgeToOriginDir( vert2.mMinkowski-vert1.mMinkowski, -vert1.mMinkowski ); \
		tSimplexVertex v1Copy = vert1, v2Copy = vert2; \
		verts[ 0 ] = v2Copy; \
		verts[ 1 ] = v1Copy; \
		verts.fSetCount( 2 ); \
	} \

#define plane_simplex( plane, v1, v2, v3, iToErase, faceBary ) \
	tVec3f closestPt = fNormalizePlaneAndProjectOrigin( plane ); \
	if( fShouldUpdate( closestPt ) ) \
	{ \
		mSimplex.mClosestPt = closestPt; \
		mSimplex.mSearchDir = -closestPt; \
		if( iToErase != -1 ) \
			verts.fEraseOrdered( iToErase ); \
		if( !fOriginOutsidePlane( plane ) ) \
			fSwap( verts[ 0 ], verts[ 1 ] ); \
	} \

// Alternate method for computing closest pt on plane.
// tVec3f closestPt = v1.mMinkowski * faceBary.x + v2.mMinkowski * faceBary.y + v3.mMinkowski * faceBary.z; \

// threw out a, which was the newest. newest point not productive. terminate.
#define degenerate_terminate( ) if( Physics_Collision_GJK_DegenerateTerm ) { mTerminate = true; }
	

	void tGJK::fEvolveSimplex( )
	{
		tSimplex::tVertList& verts = mSimplex.mVerts;
		sigassert( verts.fCount( ) && "simplex needs to be intialized!" );

		switch( verts.fCount( ) )
		{
		case 1:
			{
				vert_simplex( verts[ 0 ] );
				break;
			}
		case 2:
			{
				// Line case, a is the newer pt.
				const tSimplexVertex& a = verts[ 1 ];
				const tSimplexVertex& b = verts[ 0 ];

				tVec2f abBary = fBarycentricEdge( a.mMinkowski, b.mMinkowski );

				if( abBary.x > 0 )
				{
					if( abBary.y > 0 )
					{
						edge_simplex( a, b, abBary )
					}
					else
					{
						// a is closest
						vert_simplex( a );
					}
				}
				else
				{
					// b is closest
					vert_simplex( b );

					degenerate_terminate( );
				}

				break;
			}
		case 3:
			{
				// triangle case, a is the newer pt.
				//  see fig. 2
				const tSimplexVertex& a = verts[ 2 ];
				const tSimplexVertex& b = verts[ 1 ];
				const tSimplexVertex& c = verts[ 0 ];

				//sigassert( fUnsignedTriArea( a.mMinkowski, b.mMinkowski, c.mMinkowski ) > 0.0001f );

				tVec2f abBary = fBarycentricEdge( a.mMinkowski, b.mMinkowski );
				tVec2f bcBary = fBarycentricEdge( b.mMinkowski, c.mMinkowski );
				tVec2f caBary = fBarycentricEdge( c.mMinkowski, a.mMinkowski );

				if( abBary.y <= 0 && caBary.x <= 0 )
				{
					//vertex a is closest
					vert_simplex( a );
				}
				else if( abBary.x <= 0 && bcBary.y <= 0 )
				{
					//vertex b is closest
					vert_simplex( b );

					degenerate_terminate( );
				}
				else if( bcBary.x <= 0 && caBary.y <= 0 )
				{
					//vertex c is closest
					vert_simplex( c );

					degenerate_terminate( );
				}
				else
				{
					//edge cases
					tVec3f uvw = fBarycentricTri( a.mMinkowski, b.mMinkowski, c.mMinkowski );

					if( uvw.z <= 0 && abBary.x > 0 && abBary.y > 0 )
					{
						//ab is closest
						edge_simplex( a, b, abBary );
					}
					else if( uvw.y <= 0 && caBary.x > 0 && caBary.y > 0 )
					{
						//ca is closest
						edge_simplex( c, a, caBary );
					}
					else if( uvw.x <= 0 && bcBary.x > 0 && bcBary.y > 0 )
					{
						//bc is closest
						edge_simplex( b, c, bcBary );

						degenerate_terminate( );
					}
					else
					{
						//origin closest to face
						sigassert( uvw.x > 0 && uvw.y > 0 && uvw.z > 0 );
						tPlanef facePlane = fUnnormalizedPlane( a.mMinkowski, b.mMinkowski, c.mMinkowski );
						plane_simplex( facePlane, a, b, c, -1, uvw ); //no vertex to erase
					}
				}
				break;
			}
		case 4:
			{
				// tetrahedron case, a is the newer pt.
				//  see fig. 3
				tSimplexVertex& a = verts[ 3 ];
				tSimplexVertex& b = verts[ 2 ];
				tSimplexVertex& c = verts[ 1 ];
				tSimplexVertex& d = verts[ 0 ];

				if( fSignedTetraVolume( a.mMinkowski, b.mMinkowski, c.mMinkowski, d.mMinkowski ) < 0 )
				{
					//tetrahedron is inside out.
					fSwap( c, d );

					if( Physics_Collision_GJK_Debug )
						log_line( 0, "Tetra Insideout" );
				}

				tPlanef p1 = fUnnormalizedPlane( a.mMinkowski, b.mMinkowski, c.mMinkowski );
				tPlanef p2 = fUnnormalizedPlane( a.mMinkowski, d.mMinkowski, b.mMinkowski );
				tPlanef p3 = fUnnormalizedPlane( a.mMinkowski, c.mMinkowski, d.mMinkowski );
				tPlanef p4 = fUnnormalizedPlane( d.mMinkowski, c.mMinkowski, b.mMinkowski );
				b32 out1 = fOriginOutsidePlane( p1 );
				b32 out2 = fOriginOutsidePlane( p2 );
				b32 out3 = fOriginOutsidePlane( p3 );
				b32 out4 = fOriginOutsidePlane( p4 );

				//sigassert( p4.fSignedDistance( a ) < 0.00001f && "Tetrahedron does not have 'height'" );

#ifdef build_debug
				// This should have been caught by the fSignedTetraVolume and fSwap. apparently that is not sufficient.
				sigassert( (!out1 || !out2 || !out3 || !out4) && "Not possible. hedron inside out!" );
#else
				if( !(!out1 || !out2 || !out3 || !out4) ) log_warning( "Not possible. hedron inside out!" );
#endif

				if( !out1 && !out2 && !out3 && !out4 )
				{
					//origin is within all 4 planes, contained, intersection.
					mIntersects = true;
					mTerminate = true;

					if( Physics_Collision_GJK_Debug )
						log_line( 0, "Containment Term" );
				}
				else
				{
					tVec2f abBary = fBarycentricEdge( a.mMinkowski, b.mMinkowski );
					tVec2f acBary = fBarycentricEdge( a.mMinkowski, c.mMinkowski );
					tVec2f adBary = fBarycentricEdge( a.mMinkowski, d.mMinkowski );
					tVec2f cdBary = fBarycentricEdge( c.mMinkowski, d.mMinkowski );
					tVec2f cbBary = fBarycentricEdge( c.mMinkowski, b.mMinkowski );
					tVec2f dbBary = fBarycentricEdge( d.mMinkowski, b.mMinkowski );

					if( abBary.y <= 0 && acBary.y <= 0 && adBary.y <= 0 )
					{
						// a closest
						vert_simplex( a );
					}
					else if( abBary.x <= 0 && dbBary.x && cbBary.x <= 0 )
					{
						// b closest
						vert_simplex( b );

						degenerate_terminate( );
					}
					else if( cdBary.y <= 0 && acBary.x && cbBary.y <= 0 )
					{
						// c closest
						vert_simplex( c );

						degenerate_terminate( );
					}
					else if( cdBary.x <= 0 && dbBary.y && adBary.x <= 0 )
					{
						// d closest
						vert_simplex( d );

						degenerate_terminate( );
					}
					else
					{
						//edge cases
						tVec3f abc = fBarycentricTri( a.mMinkowski, b.mMinkowski, c.mMinkowski );
						tVec3f adb = fBarycentricTri( a.mMinkowski, d.mMinkowski, b.mMinkowski );
						tVec3f acd = fBarycentricTri( a.mMinkowski, c.mMinkowski, d.mMinkowski );
						tVec3f dcb = fBarycentricTri( d.mMinkowski, c.mMinkowski, b.mMinkowski );

						const f32 cEps = 0.00001f;

						if( abBary.x > 0 && abBary.y > 0 && abc.z <= cEps && adb.y <= cEps )
						{
							//ab closest
							edge_simplex( a, b, abBary );
						}
						else if( acBary.x > 0 && acBary.y > 0 && abc.y <= cEps && acd.z <= cEps )
						{
							//ac closest
							edge_simplex( a, c, acBary );
						}
						else if( adBary.x > 0 && adBary.y > 0 && acd.y <= cEps && adb.z <= cEps )
						{
							//ad closest
							edge_simplex( a, d, adBary );
						}
						else if( cbBary.x > 0 && cbBary.y > 0 && abc.x <= cEps && dcb.x <= cEps )
						{
							//cb closest
							edge_simplex( c, b, cbBary );

							degenerate_terminate( );
						}
						else if( cdBary.x > 0 && cdBary.y > 0 && acd.x <= cEps && dcb.z <= cEps )
						{
							//cd closest
							edge_simplex( c, d, cdBary );

							degenerate_terminate( );
						}
						else if( dbBary.x > 0 && dbBary.y > 0 && adb.x <= cEps && dcb.y <= cEps )
						{
							//db closest
							edge_simplex( d, b, dbBary );

							degenerate_terminate( );
						}
						else
						{
							//face cases
							if( out1 && abc.fMin( ) >= 0.f )
							{
								//sigassert( (!out2 && !out3 && !out4 ) );
								plane_simplex( p1, a, b, c, 0, abc );
							}
							else if( out2 && adb.fMin( ) >= 0.f )
							{
								//sigassert( (!out1 && !out3 && !out4 ) );
								plane_simplex( p2, a, d, b, 1, adb );
							}
							else if( out3 && acd.fMin( ) >= 0.f )
							{
								//sigassert( (!out1 && !out2 && !out4 ) );
								plane_simplex( p3, a, c, d, 2, acd );
							}
							else if( out4 && dcb.fMin( ) >= 0.f )
							{
								//sigassert( (!out1 && !out2 && !out3 ) );
								plane_simplex( p4, d, c, b, 3, dcb );

								degenerate_terminate( );
							}
							else
							{
								//really should not have gotten here. but seems to be ok in most cases
								// Should have been picked up by fully-contained, verts, edges, or faces.
								log_warning( "GJK fEvolveSimplex error :(" );
							}
						}
					}
				}

				break;
			}
		default:
			{
				sigassert( !"Should not have gotten here." );
			}
		}

		if_devmenu( if( Physics_Collision_GJK_Debug ) mHistory.mEvents.fPushBack( mSimplex ) );
	}

	namespace
	{
		void fGatherUniqueVertices( const tVec3f& a, tFixedGrowingArray<tVec3f, 4>& verts )
		{
			for( u32 y = 0; y < verts.fCount( ); ++y )
				if( verts[ y ].fEqual( a ) )
					return;

			verts.fPushBack( a );
		}

		void fFillSATBody( tFixedGrowingArray<tVec3f, 4>& verts, const tVec3f& center, tIntersectionGeneralSAT::tBody& body )
		{
			body.mPoints.fSetCount( 0 );
			body.mPoints.fInsert( 0, verts.fBegin( ), verts.fCount( ) );
			body.mPoints.fPushBack( center );

			body.mEdges.fSetCount( 0 );
			for( u32 i = 1; i < verts.fCount( ); ++i )
			{
				tVec3f edge = verts[ i - 1] - verts[ i ];
				edge.fNormalizeSafe( tVec3f::cZeroVector );
				body.mEdges.fPushBack( edge );
			}
			if( verts.fCount( ) > 2 )
			{
				tVec3f edge = verts.fFront( ) - verts.fBack( );
				edge.fNormalizeSafe( tVec3f::cZeroVector );
				body.mEdges.fPushBack( edge );
			}

			body.mFaces.fSetCount( 0 );
			if( verts.fCount( ) == 3 )
			{
				tPlanef p = fUnnormalizedPlane( verts[ 0 ], verts[ 1 ], verts[ 2 ] );
				if( p.fGetNormal( ).fLengthSquared( ) > cEpsilon )
					body.mFaces.fPushBack( p.fGetNormalUnit( ) );
			}
			else if( verts.fCount( ) == 4 )
			{
				if( fSignedTetraVolume( verts[ 0 ], verts[ 1 ], verts[ 2 ], verts[ 3 ] ) < 0 )
					fSwap( verts[ 0 ], verts[ 1 ] );

				tPlanef p[ 4 ] = { fUnnormalizedPlane( verts[ 0 ], verts[ 1 ], verts[ 2 ] ), fUnnormalizedPlane( verts[ 0 ], verts[ 3 ], verts[ 1 ] ), fUnnormalizedPlane( verts[ 0 ], verts[ 2 ], verts[ 3 ] ), fUnnormalizedPlane( verts[ 3 ], verts[ 2 ], verts[ 1 ] ) };
				for( u32 i = 0; i < 4; ++i )
					if( p[ i ].fGetNormal( ).fLengthSquared( ) > cEpsilon )
						body.mFaces.fPushBack( p[ i ].fGetNormalUnit( ) );
			}
		}
	}

	void tGJK::fUpdateClosestPts( )
	{
		tSimplex::tVertList& verts = mSimplex.mVerts;
		sigassert( verts.fCount( ) && "simplex needs to be intialized!" );

		switch( verts.fCount( ) )
		{
		case 1:
			{
				const tSimplexVertex& a = verts[ 0 ];

				mClosestPtA = a.mA.mP;
				mClosestPtB = a.mB.mP;				

				break;
			}
		case 2:
			{
				// Line case, a is the newer pt.
				const tSimplexVertex& a = verts[ 1 ];
				const tSimplexVertex& b = verts[ 0 ];

				tVec2f bary = fBarycentricEdge( a.mMinkowski, b.mMinkowski );

				mClosestPtA = a.mA.mP * bary.x + b.mA.mP * bary.y;
				mClosestPtB = a.mB.mP * bary.x + b.mB.mP * bary.y;				

				break;
			}
		case 3:
			{
				// triangle case, a is the newer pt.
				//  see fig. 2
				const tSimplexVertex& a = verts[ 2 ];
				const tSimplexVertex& b = verts[ 1 ];
				const tSimplexVertex& c = verts[ 0 ];
				
				tVec3f bary = fBarycentricTri( a.mMinkowski, b.mMinkowski, c.mMinkowski );
				mClosestPtA = a.mA.mP * bary.x + b.mA.mP * bary.y + c.mA.mP * bary.z;
				mClosestPtB = a.mB.mP * bary.x + b.mB.mP * bary.y + c.mB.mP * bary.z;

				break;
			}
		case 4:
			{
				// tetrahedron case
				tFixedGrowingArray<tVec3f, 4> aVerts, bVerts;

				for( u32 i = 0; i < 4; ++i )
				{
					fGatherUniqueVertices( verts[ i ].mA.mP, aVerts );
					fGatherUniqueVertices( verts[ i ].mB.mP, bVerts );
				}

				// this whole test is very nonsensical. 
				//  It's a really half-assed attempt at just trying to separate the bodies.
				//  It would probably better to use an OBB and separate the two shapes.

				tIntersectionGeneralSAT sat;
				fFillSATBody( aVerts, mA->fWorldCenter( ), sat.mBodyA );
				fFillSATBody( bVerts, mB->fWorldCenter( ), sat.mBodyB );
				
				sat.fIntersect( true );

				// Therefore no need to assert these nonsensical results.
				//sigassert( sat.fIntersects( ) );

				mClosestPtB = sat.mResult.mPoint;
				mClosestPtA = mClosestPtB - sat.mResult.mNormal * sat.mResult.mDepth;
				mPenetration = sat.mResult.mDepth;
				mNormal = sat.mResult.mNormal;

				break;
			}
		default:
			{
				sigassert( !"Should not have gotten here." );
			}
		}
	}

	tContactPoint tGJK::fMakeContactPt( const Math::tVec3f* normalOverride ) const
	{
		const f32 totalExtraRadius = mA->mExtraRadius + mB->mExtraRadius;

		tVec3f norm;
		f32 penetration;

		if( fIntersects( ) )
		{
			norm = fNormalBToA( );
			penetration = totalExtraRadius + fPenetration( );
		}
		else
		{
			norm = fClosestPtA( ) - fClosestPtB( );

			f32 len;
			norm.fNormalizeSafe( tVec3f::cYAxis, len );
			penetration = totalExtraRadius - len;
		}

		if( normalOverride )
			norm = *normalOverride;

		return tContactPoint( fClosestPtB( ) + norm * mB->mExtraRadius, norm, penetration );
	}


	void tGJK::fDrawSimplexes( )
	{
		u32 count = mSimplex.mVerts.fCount( );
		if( count > 1 )
		{
			for( u32 i = 0; i < count - 1; ++i )
			{
				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( mSimplex.mVerts[ i ].mA.mP, mSimplex.mVerts[ i+1 ].mA.mP, tVec4f( 1,0,0,1 ) );
				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( mSimplex.mVerts[ i ].mB.mP, mSimplex.mVerts[ i+1 ].mB.mP, tVec4f( 0,0,1,1 ) );
			}
			tPhysicsWorld::fDebugGeometry( ).fRenderOnce( mSimplex.mVerts[ 0 ].mA.mP, mSimplex.mVerts[ count-1 ].mA.mP, tVec4f( 1,0,0,1 ) );
			tPhysicsWorld::fDebugGeometry( ).fRenderOnce( mSimplex.mVerts[ 0 ].mB.mP, mSimplex.mVerts[ count-1 ].mB.mP, tVec4f( 0,0,1,1 ) );
		}
	}


	namespace
	{
		// returns "t" of intersection for ray and plane.
		f32 fClipRay( const Math::tRayf& ray, const tVec3f& normal, const tVec3f& pt, const tVec4f& debugColor, b32 debugRender )
		{
			if( debugRender && Physics_Collision_GJK_RaycastDraw )
			{
				tPlanef plane( normal, pt );
				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( plane, pt, 2.f, debugColor );
			}

			f32 t = (pt - ray.mOrigin).fDot( normal ) / ray.mExtent.fDot( normal );
			return t;
		}
	}

	tGJKRaycast::tGJKRaycast( tSupportMapping* a, const Math::tRayf& ray )
		: mT( 0.f )
		, mRaySupport( NEW tPointSphereSupportMapping( ray.mOrigin, 0.f ) )
		, mGJK( a, mRaySupport, true )
		, mRay( ray )
		, mIntersects( false )
		, mDebugColor( tVec4f( tPhysicsWorld::fRandomColor( ), 0.5f ) )
		, mDebugRender( true )
	{
	}

	void tGJKRaycast::fReset( )
	{
		mGJK.fReset( );
	}

	void tGJKRaycast::fCompute( )
	{
		const f32 cTolerance = Physics_Collision_GJK_RaycastTolerance;
		const f32 cToleranceSqr = cTolerance * cTolerance;

		tVec3f lastNormal = tVec3f::cYAxis;
		b32 hasStepped = false;

		mIntersects = false;
		mT = 0.f;
		mRaySupport->mCenter = mRay.fPointAtTime( mT );

		for( ;; )
		{
			mGJK.fReset( );
			//mGJK.fResume( );
			mGJK.fCompute( );

			b32 hit = false;
			
			if( mGJK.fIntersects( ) )
			{
				if( !hasStepped )
				{
					// origin is contained
					//  miss
					return;
				}
				else
				{
					// a very very accurate result. likely right on the surface.
					hit = true;
				}
			}

			// points from shape to ray.
			tVec3f diff = mRaySupport->mCenter - mGJK.fClosestPtA( );
			if( hit || diff.fLengthSquared( ) < cToleranceSqr )
			{
				mIntersects = true;
				mNormal = lastNormal;
				mPoint = mGJK.fClosestPtA( );
				mT = fClipRay( mRay, lastNormal, mGJK.fClosestPtA( ), mDebugColor, false );
				return;
			}

			if( diff.fDot( mRay.mExtent ) >= Physics_Collision_GJK_RaycastDotTolerance )
			{
				//clip plane and ray are either coplanar or facing the same direction.
				// miss.
				return;
			}

			hasStepped = true;
			lastNormal = diff;
			lastNormal.fNormalize( );			

			// plane on convex shape.
			mT = fClipRay( mRay, lastNormal, mGJK.fClosestPtA( ), mDebugColor, mDebugRender );

			if( mDebugRender && Physics_Collision_GJK_RaycastDraw )
			{
				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( tSpheref( mGJK.fClosestPtA( ), 0.2f ), tVec4f(1,0,0,0.75f) );
				mGJK.fDrawSimplexes( );
			}

			if( mT > 1.f )
			{
				// beyond ray length.
				//  miss
				return;
			}

			mRaySupport->mCenter = mRay.fPointAtTime( mT );
		}
	}
	
	void tGJKHistory::fLog( )
	{
		const char* cNames[ ] = { "D", "C", "B", "A" };

		for( u32 i = 0; i < mEvents.fCount( ); ++i )
		{
			log_line( 0, i << ": " << mEvents[ i ].mVerts.fCount( ) );
			for( u32 s = 0; s < mEvents[ i ].mVerts.fCount( ); ++s )
			{
				log_line( 0, " " << cNames[ 3 - s ] << ": a: " << mEvents[ i ].mVerts[ s ].mA.mID << " b: " << mEvents[ i ].mVerts[ s ].mB.mID );
			}
		}
	}





	void tGJK::fTest( )
	{
		{
			//trivial hit
			tGJKRaycast raycast( NEW tAabbSupportMapping( tVec3f( 1 ), 0 ), tRayf( tVec3f( 0,2,0 ), tVec3f( 0,-2,0 ) ) );
			raycast.fCompute( );
			sigassert( raycast.mIntersects );
			sigassert( raycast.mPoint.fEqual( tVec3f::cYAxis ) );
			sigassert( raycast.mNormal.fEqual( tVec3f::cYAxis ) );
		}
		{
			//trivial miss
			tGJKRaycast raycast( NEW tAabbSupportMapping( tVec3f( 1 ), 0 ), tRayf( tVec3f( 0,4,0 ), tVec3f( 0,-2,0 ) ) );
			raycast.fCompute( );
			sigassert( !raycast.mIntersects );
		}

		{
			// wide hit
			tGJKRaycast raycast( NEW tAabbSupportMapping( tVec3f( 1 ), 0 ), tRayf( tVec3f( 0,2,0 ), tVec3f( -1.9f,-2,0 ) ) );
			raycast.fCompute( );
			sigassert( raycast.mIntersects );
			sigassert( raycast.mNormal.fEqual( tVec3f::cYAxis ) );
		}

		{
			// narrow miss
			tGJKRaycast raycast( NEW tAabbSupportMapping( tVec3f( 1 ), 0 ), tRayf( tVec3f( 0,2,0 ), tVec3f( -2.2f,-2,0 ) ) );
			raycast.fCompute( );
			sigassert( !raycast.mIntersects );
		}

		{
			// origin contained
			tGJKRaycast raycast( NEW tAabbSupportMapping( tVec3f( 1 ), 0 ), tRayf( tVec3f( 0,0.9f,0 ), tVec3f( -2.2f,-2,0 ) ) );
			raycast.fCompute( );
			sigassert( !raycast.mIntersects );
		}




		tSupportMappingPtr a( NEW tAabbSupportMapping( tVec3f( 1 ), 0 ) );
		tSupportMappingPtr b( NEW tAabbSupportMapping( tVec3f( 1 ), 0 ) );

		tMat3f xform = tMat3f::cIdentity;

		{
			xform.fSetTranslation(  tVec3f( 2.25f, 0.25f, 0.25f ) );
			b->fSetWorldXform( xform );
			tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
			gjk.fCompute( );
			sigassert( !gjk.fIntersects( ) );
		}

		{
			// .25 overlap
			xform.fSetTranslation( tVec3f( 1.75f, 0.25f, 0.25f ) );
			b->fSetWorldXform( xform );
			tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
			gjk.fCompute( );
			sigassert( gjk.fIntersects( ) );
		}
		{

			tVec3f unitDiag = tVec3f(1).fNormalize( );
			b.fReset( NEW tAabbSupportMapping( unitDiag, 0 ) );

			// box b has aligned length of two units.
			// box b has a diagonal of two units.
			// rotate box b so that diagonal points along x.
			tMat3f xform( tQuatf( tAxisAnglef( unitDiag, tVec3f::cXAxis ) ) );

			{
				// .01 overlap
				xform.fSetTranslation( tVec3f( 1.99f, 0, 0 ) );
				b->fSetWorldXform( xform );
				tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
				gjk.fCompute( );
				sigassert( gjk.fIntersects( ) );
			}
			{
				// .01 sep
				xform.fSetTranslation( tVec3f( 2.01f, 0, 0 ) );
				b->fSetWorldXform( xform );
				tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
				gjk.fCompute( );
				sigassert( !gjk.fIntersects( ) );
			}
		}

		//spheres
		{

			a.fReset( NEW tSphereSupportMapping( 1.f ) );
			b.fReset( NEW tSphereSupportMapping( 1.f ) );

			{
				// .01 overlap
				xform.fSetTranslation( tVec3f( 1.99f, 0, 0 ) );
				b->fSetWorldXform( xform );
				tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
				gjk.fCompute( );
				sigassert( gjk.fIntersects( ) );
			}
			{
				// .01 sep
				xform.fSetTranslation( tVec3f( 2.01f, 0, 0 ) );
				b->fSetWorldXform( xform );
				tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
				gjk.fCompute( );
				sigassert( !gjk.fIntersects( ) );
			}
		}

		//combo
		{

			a.fReset( NEW tAabbSupportMapping( 1.f, 0 ) );
			b.fReset( NEW tSphereSupportMapping( 1.f ) );

			{
				// .01 overlap
				xform.fSetTranslation( tVec3f( 1.98f, 0, 0 ) );
				b->fSetWorldXform( xform );
				tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
				gjk.fCompute( );
				sigassert( gjk.fIntersects( ) );
			}
			{
				// .01 sep
				xform.fSetTranslation( tVec3f( 2.01f, 0, 0 ) );
				b->fSetWorldXform( xform );
				tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
				gjk.fCompute( );
				sigassert( !gjk.fIntersects( ) );
			}
			{
				// corner
				xform.fSetTranslation( tVec3f( 0.5f, 0.5f, 0.5f ) );
				b->fSetWorldXform( xform );
				tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
				gjk.fCompute( );
				sigassert( gjk.fIntersects( ) );
			}
			{
				// missing corner
				xform.fSetTranslation( tVec3f( 1.8f, 1.8f, 1.8f ) );
				b->fSetWorldXform( xform );
				tGJK gjk( a.fGetRawPtr( ), b.fGetRawPtr( ) );
				gjk.fCompute( );
				sigassert( !gjk.fIntersects( ) );
			}
		}

	}

} }
