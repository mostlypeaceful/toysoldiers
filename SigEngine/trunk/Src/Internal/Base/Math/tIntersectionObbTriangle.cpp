#include "BasePch.hpp"
#include "tIntersectionObbTriangle.hpp"

namespace Sig { namespace Math
{

	devvar( bool, Physics_Debug_TestTriEdges, true );


	namespace 
	{
		f32 SCALAR_TINY = 0.00001f;
	
		//========================================================
		// fDisjoint Returns true if disjoint.  Returns false if intersecting,
		// and sets the overlap depth, d scaled by the axis length
		//========================================================
		static bool fDisjoint(f32 & d,
							 const tVec3f & axis, 
							 const tObbf & box0, 
							 const tTrianglef & tri,
							 f32 collTolerance)
		{
		  f32 min0, max0, min1, max1;
		  
		  box0.fMinMaxProject(min0, max0, axis);
		  tri.fMinMaxProject(min1, max1, axis);

		  f32 min0Sep = min0 - (max1 + collTolerance + SCALAR_TINY);
		  f32 min1Sep = min1 - (max0 + collTolerance + SCALAR_TINY);

		  if ( min0Sep > 0 || min1Sep > 0 )
		  {
			  d = -fMax( min0Sep, min1Sep );
			  return true;
		  }
		  
		  if ( (max0 > max1) && (min1 > min0) )
		  {
			// box1 is inside - choose the min dist to move it out
			d = fMin(max0 - min1, max1 - min0);
		  }
		  else if ( (max1 > max0) && (min0 > min1) )
		  {
			// box0 is inside - choose the min dist to move it out
			d = fMin(max1 - min0, max0 - min1);
		  }
		  else
		  {
			// boxes overlap
			d  = (max0 < max1) ? max0 : max1;
			d -= (min0 > min1) ? min0 : min1;
		  }
		  
		  return false;
		}

		//==============================================================
		// GetSupportPoint
		//==============================================================


	}


	
	void tIntersectionObbTriangle::fIntersect( const tObbf& a, const tTrianglef& triangle, b32 keepNonPenetrating )
	{
		f32 collTolerance = 0.01f;

		mIntersects = true;

		const tObb<f32>& box0 = a;
		tVec3f triCenter = triangle.mA + triangle.mB + triangle.mC;
		triCenter /= 3.f;

		// http://www.gamedev.net/topic/323553-obb-triangle-intersection/
		const u32 maxNumEdges = 13;
		u32 numAxes = maxNumEdges;
		tVec3f axes[maxNumEdges];

		mTriangleNormal = triangle.fComputeUnitNormal( );
		axes[0] = mTriangleNormal;
		axes[1] = box0.fAxis( 0 );
		axes[2] = box0.fAxis( 1 );
		axes[3] = box0.fAxis( 2 );

		tVec3f edges[3] = { triangle.mB - triangle.mA, triangle.mC - triangle.mB, triangle.mA - triangle.mC };
		if( Physics_Debug_TestTriEdges )
		{
			axes[4] =  edges[ 0 ].fCross( box0.fAxis( 0 ) );
			axes[5] =  edges[ 0 ].fCross( box0.fAxis( 1 ) );
			axes[6] =  edges[ 0 ].fCross( box0.fAxis( 2 ) );
			axes[7] =  edges[ 1 ].fCross( box0.fAxis( 0 ) );
			axes[8] =  edges[ 1 ].fCross( box0.fAxis( 1 ) );
			axes[9] =  edges[ 1 ].fCross( box0.fAxis( 2 ) );
			axes[10] = edges[ 2 ].fCross( box0.fAxis( 0 ) );
			axes[11] = edges[ 2 ].fCross( box0.fAxis( 1 ) );
			axes[12] = edges[ 2 ].fCross( box0.fAxis( 2 ) );
		}
		else
			numAxes = 4;


		// the overlap depths along each axis
		f32 overlapDepths[maxNumEdges];

		// see if the boxes are separate along any axis, and if not keep a 
		// record of the depths along each axis
		unsigned i;
		for (i = 0 ; i < numAxes ; ++i)
		{
			// If we can't normalise the axis, skip it
			f32 l2 = axes[i].fLengthSquared( );
			if (l2 < SCALAR_TINY)
				continue;
			overlapDepths[i] = cInfinity;
			b32 disjoint = fDisjoint(overlapDepths[i], axes[i], box0, triangle, collTolerance);

			if( disjoint )
			{
				mIntersects = false;
				
				if( !keepNonPenetrating )
					return;
			}
		}

		//-----------------------------------------------------------------
		// The box overlap, find the separation depth closest to 0.
		//-----------------------------------------------------------------
		f32 minDepth = cInfinity;
		int minAxis = -1;

		for(i = 0; i < numAxes; ++i)
		{
			// If we can't normalise the axis, skip it
			f32 l2 = axes[i].fLengthSquared();
			if (l2 < SCALAR_TINY)
				continue;

			//-----------------------------------------------------------------
			// Normalise the separation axis and the depth
			//-----------------------------------------------------------------
			f32 invl = 1.0f / fSqrt(l2);
			axes[i] *= invl;
			overlapDepths[i] *= invl;

			//-----------------------------------------------------------------
			// If this axis is the minimum, select it
			//-----------------------------------------------------------------
			if (overlapDepths[i] < minDepth)
			{
				minDepth = overlapDepths[i];
				minAxis = i;
			}
		}

		sigassert( minAxis != -1 && "NO SAT found!?" );

		tVec3f axis = axes[minAxis];
		f32 depth = overlapDepths[minAxis];

		// make normal point into box0
		tVec3f diff = triCenter - box0.fCenter( );
        if( diff.fDot( axis ) > 0 )
            axis = -axis;

		b32 onEdge = false;

		tVec3f SATPoint(0.0f);
		switch(minAxis)
		{
			//-----------------------------------------------------------------
			// tri face face, Box1 Corner collision
			//-----------------------------------------------------------------
			case 0:
			{
				SATPoint = box0.fSupportingPoint( axis );
				//SATPoint = triangle.fClampPtInTriangle( SATPoint );
				break;
			}
			//-----------------------------------------------------------------
			// We have a Box0 corner/tri face collision
			//-----------------------------------------------------------------
			case 1:
			case 2:
			case 3:
			{
				SATPoint = triangle.fSupportingCorner( -axis );
				SATPoint += axis * depth;
				break;
			}
			//-----------------------------------------------------------------
			// We have an edge/edge colliiosn
			//-----------------------------------------------------------------
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			{ 
				{
					onEdge = true;

					//-----------------------------------------------------------------
					// Retrieve which edges collided.
					//-----------------------------------------------------------------
					int i = minAxis-4;
					int iT = i / 3;
					int ib = i - iT * 3;
					sigassert(iT >= 0);
					sigassert(iT < 3);
					sigassert(ib >= 0);
					sigassert(ib < 3);
					
					tVec3f P0;
					tVec3f P1;
					P0 = triangle.fSupportingCorner( -axis );
					P1 = box0.fSupportingPoint( axis );

					edges[ iT ].fNormalizeSafe( tVec3f::cYAxis );

					if( !Physics::tFullContactWitness::fClosestPointOnEdges( P0, edges[ iT ], P1, box0.fAxis( ib ), axis, SATPoint ) )
					{
						// we should probably do something smarter than this. like if the axes are parrallel dont include them in the test ilst.
						mIntersects = false;
						return;
					}

					SATPoint = triangle.fClampPtInTriangle( SATPoint );
				}
				break;
			}
			default:
			sigassert(!"Impossible switch");
		}
		
		mResult = Physics::tContactPoint( SATPoint, axis, depth );
	}


}}