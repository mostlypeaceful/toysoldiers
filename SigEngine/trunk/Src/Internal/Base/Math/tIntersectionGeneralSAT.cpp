#include "BasePch.hpp"
#include "tIntersectionGeneralSAT.hpp"

//for debugging
#include "Physics/tPhysicsWorld.hpp"

namespace Sig { namespace Math
{
	namespace
	{
		f32 SCALAR_TINY = 0.00001f;
	}
	
	//========================================================
	// fDisjoint Returns true if disjoint.  Returns false if intersecting,
	// and sets the overlap depth, d scaled by the axis length
	//========================================================
	static bool fDisjoint(f32 & d,
						 const tVec3f & axis, 
						 const tIntersectionGeneralSAT::tBody & box0, 
						 const tIntersectionGeneralSAT::tBody & box1,
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

	void tIntersectionGeneralSAT::fIntersect( b32 keepNonPenetrating )
	{
		mIntersects = true; //until proved otherwise

		f32 collTolerance = 0.01f;

		tGrowableArray< tVec3f > axes;
		axes.fJoin( mBodyA.mFaces );
		axes.fJoin( mBodyB.mFaces );

		for( u32 i = 0; i < mBodyA.mEdges.fCount( ); ++i )
			for( u32 j = 0; j < mBodyB.mEdges.fCount( ); ++j )
				axes.fPushBack( mBodyA.mEdges[ i ].fCross( mBodyB.mEdges[ j ] ) );

		u32 numAxes = axes.fCount( );

		// the overlap depths along each axis
		tGrowableArray< f32 > overlapDepths;
		overlapDepths.fSetCount( numAxes );

		// see if the boxes are separate along any axis, and if not keep a 
		// record of the depths along each axis
		unsigned i;
		for (i = 0 ; i < numAxes; ++i)
		{
			// If we can't normalise the axis, skip it
			f32 l2 = axes[i].fLengthSquared( );
			if (l2 < SCALAR_TINY)
				continue;
			overlapDepths[i] = cInfinity;
			b32 disJoint = fDisjoint(overlapDepths[i], axes[i], mBodyA, mBodyB, collTolerance);

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
		u32 minAxis = -1;

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

		sigassert( minAxis != ~0 && "NO SAT found!?" );

		tVec3f axis = axes[minAxis];
		f32 depth = overlapDepths[minAxis];

		// make normal point into box0
		tVec3f diff = mBodyB.fCenter( ) - mBodyA.fCenter( );
        if( diff.fDot( axis ) > 0 )
            axis = -axis;

		b32 onEdge = false;

		tVec3f SATPoint(0.0f);

		if( minAxis < mBodyA.mFaces.fCount( ) )
		{
			// Body a face, body b point
			SATPoint = mBodyB.fSupportingPoint( -axis );
		}
		else if( (minAxis - mBodyA.mFaces.fCount( )) < mBodyB.mFaces.fCount( ) )
		{
			// Body b face, body a point
			SATPoint = mBodyA.fSupportingPoint( axis );
			SATPoint += axis * depth;
		}
		else
		{
			onEdge = true;
			u32 i = minAxis - mBodyA.mFaces.fCount( ) - mBodyB.mFaces.fCount( );
			u32 ia = i / mBodyB.mEdges.fCount( );
			u32 ib = i % mBodyB.mEdges.fCount( );
			sigassert(ia >= 0);
			sigassert(ia < mBodyA.mEdges.fCount( ));
			sigassert(ib >= 0);
			sigassert(ib < mBodyB.mEdges.fCount( ));

			tVec3f P0;
			tVec3f P1;
			P0 = mBodyA.fSupportingPoint( axis );
			P1 = mBodyB.fSupportingPoint( -axis );

			if( !Physics::tFullContactWitness::fClosestPointOnEdges( P0, mBodyA.mEdges[ ia ], P1, mBodyB.mEdges[ ib ], axis, SATPoint ) )
			{
				// we should probably do something smarter than this. like if the axes are parallel dont include them in the test list.
				mIntersects = false;
				return;
			}
		}
		
		mResult = Physics::tContactPoint( SATPoint, axis, depth ) ;
	}


}}