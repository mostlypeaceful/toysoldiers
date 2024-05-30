#include "BasePch.hpp"
#include "tIntersectionObbObb.hpp"

//for debugging
#include "Physics/tPhysicsWorld.hpp"

namespace Sig { namespace Math
{
	f32 SCALAR_TINY = 0.00001f;
	
	//========================================================
	// fDisjoint Returns true if disjoint.  Returns false if intersecting,
	// and sets the overlap depth, d scaled by the axis length
	//========================================================
	static bool fDisjoint(f32 & d,
						 const tVec3f & axis, 
						 const tObbf & box0, 
						 const tObbf & box1,
						 f32 collTolerance)
	{
	  f32 min0, max0, min1, max1;
	  
	  box0.fMinMaxProject(min0, max0, axis);
	  box1.fMinMaxProject(min1, max1, axis);
	  
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

	void tIntersectionObbObbWithContact::fIntersect( const tObb<f32>& a, const tObb<f32>& b, b32 keepNonPenetrating, b32 sphereCheckFirst )
	{
		//first do quick sphere-sphere based rejection
		if( sphereCheckFirst && !tSpheref( a ).fIntersects( tSpheref( b ) ) ) 
			return;

		mIntersects = true; //until proved otherwise

		const tObb<f32>& box0 = a;
		const tObb<f32>& box1 = b;

		f32 collTolerance = 0.01f;
		const u32 numAxes = 15;
		tVec3f axes[numAxes];

		axes[0] = box0.fAxis( 0 );
		axes[1] = box0.fAxis( 1 );
		axes[2] = box0.fAxis( 2 );
		axes[3] = box1.fAxis( 0 );
		axes[4] = box1.fAxis( 1 );
		axes[5] = box1.fAxis( 2 );
		axes[6] = box0.fAxis( 0 ).fCross( box1.fAxis( 0 ) );
		axes[7] = box0.fAxis( 0 ).fCross( box1.fAxis( 1 ) );
		axes[8] = box0.fAxis( 0 ).fCross( box1.fAxis( 2 ) );
		axes[9] = box0.fAxis( 1 ).fCross( box1.fAxis( 0 ) );
		axes[10] = box0.fAxis( 1 ).fCross( box1.fAxis( 1 ) );
		axes[11] = box0.fAxis( 1 ).fCross( box1.fAxis( 2 ) );
		axes[12] = box0.fAxis( 2 ).fCross( box1.fAxis( 0 ) );
		axes[13] = box0.fAxis( 2 ).fCross( box1.fAxis( 1 ) );
		axes[14] = box0.fAxis( 2 ).fCross( box1.fAxis( 2 ) );

		// the overlap depths along each axis
		f32 overlapDepths[numAxes];

		// see if the boxes are separate along any axis, and if not keep a 
		// record of the depths along each axis
		unsigned i;
		for (i = 0 ; i < numAxes ; ++i)
		{
			overlapDepths[i] = cInfinity;
			// If we can't normalise the axis, skip it
			f32 l2 = axes[i].fLengthSquared( );
			if (l2 < SCALAR_TINY)
				continue;
			b32 disJoint = fDisjoint(overlapDepths[i], axes[i], box0, box1, collTolerance);

			if( disJoint )
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
		tVec3f diff = box1.fCenter( ) - box0.fCenter( );
        if( diff.fDot( axis ) > 0 )
            axis = -axis;

		b32 onEdge = false;

		tVec3f SATPoint(0.0f);

		switch(minAxis)
		{
			//-----------------------------------------------------------------
			// Box0 face, Box1 Corner collision
			//-----------------------------------------------------------------
			case 0:
			case 1:
			case 2:
			{
				SATPoint = box1.fSupportingPoint( -axis );

				Physics::tFullContactWitness witness;
				witness.mB = (Physics::tRigidBody*)1; //b body is the one with the vertex.
				witness.mPoint = SATPoint;
				witness.mDepth = depth;
				witness.mNormal = -axis;
				mFullResult = witness;

				break;
			}
			//-----------------------------------------------------------------
			// We have a Box0 corner/Box1 face collision
			//-----------------------------------------------------------------
			case 3:
			case 4:
			case 5:
			{
				SATPoint = box0.fSupportingPoint( axis );

				Physics::tFullContactWitness witness;
				witness.mA = (Physics::tRigidBody*)1; //a body is the one with the vertex.
				witness.mPoint = SATPoint;
				witness.mDepth = depth;
				witness.mNormal = axis;
				mFullResult = witness;

				SATPoint += axis * depth;
				break;
			}
			//-----------------------------------------------------------------
			// We have an edge/edge colliiosn
			//-----------------------------------------------------------------
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			{ 
				{
					onEdge = true;

					//-----------------------------------------------------------------
					// Retrieve which edges collided.
					//-----------------------------------------------------------------
					int i = minAxis-6;
					int ia = i / 3;
					int ib = i - ia * 3;
					sigassert(ia >= 0);
					sigassert(ia < 3);
					sigassert(ib >= 0);
					sigassert(ib < 3);

					tVec3f P0;
					tVec3f P1;
					P0 = box0.fSupportingPoint( axis );
					P1 = box1.fSupportingPoint( -axis );

					if( !Physics::tFullContactWitness::fClosestPointOnEdges( P0, box0.fAxis( ia ), P1, box1.fAxis( ib ), axis, SATPoint ) )
					{
						// we should probably do something smarter than this. like if the axes are parrallel dont include them in the test ilst.
						mIntersects = false;
						return;
					}

					Physics::tFullContactWitness witness;
					witness.mPoint = SATPoint;
					witness.mDepth = depth;
					witness.mNormal = axis;
					witness.mEa = box0.fAxis( ia );
					witness.mEb = box0.fAxis( ib );
					witness.mEdgeCollision = true;
					mFullResult = witness;
				}
				break;
			}
			default:
			sigassert(!"Impossible switch");
		}
		
		mResult = Physics::tContactPoint( SATPoint, axis, depth );
	}


}}
