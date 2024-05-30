#include "BasePch.hpp"
#include "tIntersectionObbObb.hpp"

namespace Sig { namespace Math
{
	// temp home to reduce build times:

	// this code kind of breaks the code standards a bit,
	//  but is entirely confined to this file. -matt

	// plus this is so entirely jacked from jiglib. -not matt


	f32 SCALAR_TINY = 0.00001f;

	struct tContactPoint
	{
	  tContactPoint( const tVec3f &pos = tVec3f::cZeroVector ) 
		  : pos(pos), count(1) 
	  {
	  }

	  tVec3f pos;
	  f32 count;
	};
	
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
	  
	  if (min0 > (max1 + collTolerance + SCALAR_TINY) || 
		  min1 > (max0 + collTolerance + SCALAR_TINY))
		return true;
	  
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
	static void GetSupportPoint(tVec3f & p,
								const tObbf & box,
								const tVec3f &axis)
	{
	  f32 as = axis.fDot( box.fAxis( 0 ) );
	  f32 au = axis.fDot( box.fAxis( 1 ) );
	  f32 ad = axis.fDot( box.fAxis( 2 ) );
	  
	  const f32 threshold = SCALAR_TINY;
	  
	  p = box.fCenter( );
	  
	  if (as < -threshold)
		p += box.fAxis( 0 ) * (box.fExtents().x);
	  else if (as >= threshold)
		p -= box.fAxis( 0 ) * (box.fExtents().x);
	  
	  if (au < -threshold)
		p += box.fAxis( 1 ) * (box.fExtents().y);
	  else if (au > threshold)
		p -= box.fAxis( 1 ) * (box.fExtents().y);
	  
	  if (ad < -threshold)
		p += box.fAxis( 2 ) * (box.fExtents().z);
	  else if (ad > threshold)
		p -= box.fAxis( 2 ) * (box.fExtents().z);
	}

	//========================================================
	// AddPoint
	// if pt is less than Sqrt(combinationDistanceSq) from one of the
	// others the original is replaced with the mean of it
	// and pt, and false is returned. true means that pt was
	// added to pts
	//========================================================
	static inline f32 PointPointDistanceSq( const tVec3f& a, const tVec3f& b )
	{
		return (b-a).fLengthSquared( );
	}
	static inline f32 PointPointDistance( const tVec3f& a, const tVec3f& b )
	{
		return (b-a).fLength( );
	}

	static inline bool AddPoint(tGrowableArray<tContactPoint> & pts,
								const tVec3f & pt,
								f32 combinationDistanceSq)
	{
	  for (unsigned i = pts.fCount( ) ; i-- != 0 ; )
	  {
		if (PointPointDistanceSq(pts[i].pos, pt) < combinationDistanceSq)
		{
		  pts[i].pos = (pts[i].count * pts[i].pos + pt) / (pts[i].count + 1);
		  pts[i].count += 1;
		  return false;
		}
	  }
	  pts.fPushBack(tContactPoint(pt));
	  return true;
	}

	//==============================================================
	// GetAABox2EdgeIntersectionPoint
	// The AABox has a corner at the origin and size sides
	//==============================================================
	static unsigned GetAABox2EdgeIntersectionPoints(
	  tGrowableArray<tContactPoint> &pts,
	  const tVec3f &sides,
	  const tObbf &box,
	  const tVec3f &edgePt0, 
	  const tVec3f &edgePt1,
	  const tMat3f &origBoxOrient,
	  const tVec3f &origBoxPos,
	  const tVec3f &dirToAABB,
	  f32 combinationDistanceSq)
	{
	  // The AABox faces are aligned with the world directions. Loop 
	  // over the 3 directions and do the two tests. We know that the
	  // AABox has a corner at the origin
	  tVec3f edgeDir = (edgePt1 - edgePt0).fNormalizeSafe( tVec3f::cYAxis );
	  unsigned num = 0;
	  for (unsigned iDir = 3 ; iDir-- != 0 ; )
	  {
		// skip edge/face tests if nearly parallel
		if (fAbs(edgeDir[iDir]) < 0.1f)
		  continue;
		const unsigned jDir = (iDir + 1) % 3;
		const unsigned kDir = (iDir + 2) % 3;
		// one plane goes through the origin, one is offset
		const f32 faceOffsets[] = {0.0f, sides[iDir]};
		for (unsigned iFace = 2 ; iFace-- != 0 ; )
		{
		  // distance of each point from to the face plane
		  const f32 dist0 = edgePt0[iDir] - faceOffsets[iFace];
		  const f32 dist1 = edgePt1[iDir] - faceOffsets[iFace];
		  f32 frac = -1.0f;
		  if (dist0 * dist1 < -SCALAR_TINY)
			frac = -dist0 / (dist1 - dist0);
		  else if (fAbs(dist0) < SCALAR_TINY)
			frac = 0.0f;
		  else if (fAbs(dist1) < SCALAR_TINY)
			frac = 1.0f;
		  if (frac >= 0.0f)
		  {
			sigassert(frac <= 1.0f);
			tVec3f pt = (1.0f - frac) * edgePt0 + frac * edgePt1;
			// check the point is within the face rectangle
			if ((pt[jDir] > -SCALAR_TINY) && 
				(pt[jDir] < sides[jDir] + SCALAR_TINY) && 
				(pt[kDir] > -SCALAR_TINY) && 
				(pt[kDir] < sides[kDir] + SCALAR_TINY) )
			{
			  // woohoo got a point
			  tVec3f pos = origBoxOrient.fXformVector( origBoxPos ) + pt;
			  AddPoint(pts, pos, combinationDistanceSq);
			  if (++num == 2)
				return num;
			}
		  }
		}
	  }
	  return num;
	}

	//==============================================================
	// GetAABox2BoxEdgesIntersectionPoints
	// Pushes intersection points (in world space) onto the back of pts.
	// Intersection is between an AABox faces and an orientated box's
	// edges. orient and pos are used to transform the points from the
	// AABox frame back into the original frame.
	//==============================================================
	static unsigned GetAABox2BoxEdgesIntersectionPoints(
	  tGrowableArray<tContactPoint> & pts,
	  const tVec3f & sides,
	  const tObbf & box,
	  const tMat3f & origBoxOrient,
	  const tVec3f & origBoxPos,
	  const tVec3f &dirToAABB,
	  f32 combinationDistanceSq)
	{
	  unsigned num = 0;
	  
	  tVec3f boxPts[8];

	  for( u32 c = 0; c < 8; ++c )
		boxPts[c] = box.fCorner( c );

	  u32 edges[24] = { 0, 1
						, 1, 2
						, 2, 3
						, 3, 0

						, 4, 5
						, 5, 6
						, 6, 7
						, 7, 4

						, 0, 4
						, 1, 5
						, 2, 6
						, 3, 7 };

	  
	  for (unsigned iEdge = 0 ; iEdge < 12 ; ++iEdge)
	  {
		const tVec3f & edgePt0 = boxPts[ edges[iEdge*2 + 0] ];
		const tVec3f & edgePt1 = boxPts[ edges[iEdge*2 + 1] ];
		num += GetAABox2EdgeIntersectionPoints(pts, 
											   sides, 
											   box, 
											   edgePt0, edgePt1,
											   origBoxOrient, origBoxPos,
											   dirToAABB,
											   combinationDistanceSq);
		// Don't think we can get more than 8... and anyway if we get too many 
		// then the penetration must be so bad who cares about the details?
		if (num >= 8)
		  return num;
	  }
	  return num;
	}

	//==============================================================
	// GetObbfBoxIntersectionPoints
	// Pushes intersection points onto the back of pts. Returns the
	// number of points found.
	// Points that are close together (compared to 
	// combinationDistance) get combined
	// dirToBody0 is the collision normal towards box0
	//==============================================================
	static unsigned GetBoxBoxIntersectionPoints(
	  tGrowableArray<tContactPoint> & pts,
	  const tObbf & box0,
	  const tObbf & box1,
	  const tVec3f &dirToBox0,
	  f32 combinationDistance,
	  f32 collTolerance)
	{
	  // first transform box1 into box0 space - there box0 has a corner
	  // at the origin and faces parallel to the world planes. Then intersect
	  // each of box1's edges with box0 faces, transforming each point back into
	  // world space. Finally combine points
	  tVec3f tol(0.5f * collTolerance);
	  combinationDistance += collTolerance * 2.0f * fSqrt(3.0f);
	  for (unsigned iBox = 0 ; iBox < 2 ; ++iBox)
	  {
		const tObbf & boxA = iBox ? box1 : box0;
		const tObbf & boxB = iBox ? box0 : box1;
	    
		const tMat3f boxAInvOrient = boxA.fGetTransform( ).fInverse( );

		//tVec3f pos = boxAInvOrient.fXformVector(boxB.fCenter( ) - boxA.fCenter( ));
		//tMat3f trans = boxAInvOrient * boxB.fGetTransform( );
		//trans.fSetTranslation( pos );

		//tObbf box( tAabbf( -boxB.fExtents( ), boxB.fExtents( ) ), trans );

		
		tObbf box = boxB;
		box.fTransform( boxAInvOrient );


		/*
		box.Expand(tol);
		box.SetPos(box.GetPos() + tol);
		*/

		tVec3f dirToBoxA = iBox ? boxAInvOrient.fXformVector( dirToBox0 ) : boxAInvOrient.fXformVector( -dirToBox0 );
	    
		// if we get more than a certain number of points back from this call,
		// and iBox == 0, could probably skip the other test...
		GetAABox2BoxEdgesIntersectionPoints(
		  pts, boxA.fExtents() * 2.0f/* + tol*/, 
		  box, boxA.fGetTransform( ), boxA.fCenter( ), dirToBoxA, pow(combinationDistance,2) );
	  }
	  return pts.fCount( );
	}
	
	void tIntersectionObbObbWithContact::fIntersect( const tObb<f32>& a, const tObb<f32>& b )
	{
		f32 collTolerance = 0.01f;

		mIntersects = false;

		// first do quick sphere-sphere based rejection
		if( !tSphere<f32>( a ).fIntersects( tSphere<f32>( b ) ) ) 
			return;

		const tObb<f32> box0( a );
		const tObb<f32> box1( b );
		const tObb<f32> oldBox0( a );
		const tObb<f32> oldBox1( b );

		const u32 numAxes = 15;
		tVec3f axes[numAxes];

		axes[0] = box0.fAxis( 0 );
		axes[1] = box0.fAxis( 1 );
		axes[2] = box0.fAxis( 2 );
		axes[3] = box1.fAxis( 0 );
		axes[4] = box1.fAxis( 1 );
		axes[5] = box1.fAxis( 2 );
		axes[6] = box0.fAxis( 0 ).fCross( box1.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis );
		axes[7] = box0.fAxis( 0 ).fCross( box1.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis );
		axes[8] = box0.fAxis( 0 ).fCross( box1.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis );
		axes[9] = box0.fAxis( 1 ).fCross( box1.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis );
		axes[10] = box0.fAxis( 1 ).fCross( box1.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis );
		axes[11] = box0.fAxis( 1 ).fCross( box1.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis );
		axes[12] = box0.fAxis( 2 ).fCross( box1.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis );
		axes[13] = box0.fAxis( 2 ).fCross( box1.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis );
		axes[14] = box0.fAxis( 2 ).fCross( box1.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis );

		// the overlap depths along each axis
		f32 overlapDepths[numAxes];

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
			if (fDisjoint(overlapDepths[i], axes[i], box0, box1, collTolerance))
				return;
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

		if (minAxis == -1)
			return;

		sigassert( minAxis != -1 );

		tVec3f axis = axes[minAxis];
		f32 depth = overlapDepths[minAxis];

		tVec3f diff = box1.fCenter( ) - box0.fCenter( );
        if( diff.fDot( axis ) > 0 )
            axis = -axis;

		f32 combinationDist = 
			0.05f * fMin( (box0.fExtents()*2.0f).fMin( ) , (box1.fExtents()*2.0f).fMin( ) );
	  
		/// the contact points
		bool contactPointsFromOld = true;
		tGrowableArray<tContactPoint> pts;

		//if (depth > -SCALAR_TINY)
		//	GetBoxBoxIntersectionPoints(pts, oldBox0, oldBox1, axis, combinationDist, collTolerance);
		
		unsigned numPts = pts.fCount();
		//if (numPts == 0)
		//{
		//	contactPointsFromOld = false;
		//	GetBoxBoxIntersectionPoints(pts, box0, box1, axis, combinationDist, collTolerance);
		//}
		//numPts = pts.fCount();

		const tVec3f& body0OldPos = tVec3f::cZeroVector;
		const tVec3f& body1OldPos = tVec3f::cZeroVector;
		const tVec3f& body0NewPos = tVec3f::cZeroVector;
		const tVec3f& body1NewPos = tVec3f::cZeroVector;

		const tVec3f bodyDelta = (body0NewPos - body0OldPos) - (body1NewPos - body1OldPos);
		const f32 bodyDeltaLen = bodyDelta.fDot( axis );
		f32 oldDepth = depth + bodyDeltaLen;

		b32 onEdge = false;

		tVec3f SATPoint(0.0f);
		tVec3f SATNormal(0.0f);
		switch(minAxis)
		{
			//-----------------------------------------------------------------
			// Box0 face, Box1 Corner collision
			//-----------------------------------------------------------------
			case 0:
			case 1:
			case 2:
			{
				//-----------------------------------------------------------------
				// Get the lowest point on the box1 along box1 normal
				//-----------------------------------------------------------------
				GetSupportPoint(SATPoint, box1, -axis);
				break;
			}
			//-----------------------------------------------------------------
			// We have a Box0 corner/Box1 face collision
			//-----------------------------------------------------------------
			case 3:
			case 4:
			case 5:
			{
				//-----------------------------------------------------------------
				// Find with vertex on the triangle collided
				//-----------------------------------------------------------------
				GetSupportPoint(SATPoint, box0, axis);
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
					//-----------------------------------------------------------------
					// find two P0, P1 point on both edges. 
					//-----------------------------------------------------------------
					tVec3f P0;
					tVec3f P1;
					GetSupportPoint( P0, box0, axis);
					GetSupportPoint( P1, box1, -axis);

					//-----------------------------------------------------------------
					// Find the edge intersection. 
					//-----------------------------------------------------------------

					//-----------------------------------------------------------------
					// plane along N and F, and passing through PB
					//-----------------------------------------------------------------
					tVec3f planeNormal = axis.fCross( box1.fAxis( ib ) );
					f32 planeD = planeNormal.fDot( P1 );

					//-----------------------------------------------------------------
					// find the intersection t, where Pintersection = P0 + t*box edge dir
					//-----------------------------------------------------------------
					f32 div = box0.fAxis( ia ).fDot( planeNormal );

					//-----------------------------------------------------------------
					// plane and ray colinear, skip the intersection.
					//-----------------------------------------------------------------
					if (fAbs(div) < SCALAR_TINY)
						return;

					f32 t = (planeD - P0.fDot( planeNormal )) / div;

					//-----------------------------------------------------------------
					// point on edge of box0
					//-----------------------------------------------------------------
					P0 += box0.fAxis( ia ) * t;

					SATPoint = (P0 + (0.5f * depth) * axis);
				}
				break;
			}
			default:
			sigassert(!"Impossible switch");
		}
		

		// distribute the depth according to the distance to the SAT point
		//if (numPts > 0)
		//{
		// Not currently supporting multiple contacts
		//	sigassert(!"Determine multiple point normals");
		//	f32 minDist = cInfinity;
		//	f32 maxDist = -cInfinity;
		//	for (u32 i = 0 ; i < numPts ; ++i)
		//	{
		//		f32 dist = PointPointDistance(pts[i].pos, SATPoint);
		//		if (dist < minDist)
		//			minDist = dist;
		//		if (dist > maxDist)
		//			maxDist = dist;
		//	}
		//	if (maxDist < minDist + SCALAR_TINY)
		//		maxDist = minDist + SCALAR_TINY;

		//	// got some intersection points
		//	for (u32 i = 0 ; i < numPts ; ++i)
		//	{
		//		static f32 minDepthScale = 0.0f;
		//		f32 dist = PointPointDistance(pts[i].pos, SATPoint);
		//		f32 depthScale = (dist - minDist) / (maxDist - minDist);
		//		f32 depth = (1.0f - depthScale) * oldDepth + minDepthScale * depthScale * oldDepth;
		//		if (contactPointsFromOld)
		//		{
		//			mResults.fPushBack( tIObbObbContact( pts[i].pos, tVec3f(0), 0.f, true ) );
		//			
		//			//tCollPointInfo(pts[i].pos- body0OldPos, 
		//							//pts[i].pos - body1OldPos, 
		//							//depth));
		//		}
		//		else
		//		{
		//			mResults.fPushBack( tIObbObbContact( pts[i].pos, tVec3f(0), 0.f, true ) );

		//			//collPts.fPushBack(tCollPointInfo(pts[i].pos - body0NewPos, 
		//			//				pts[i].pos - body1NewPos, 
		//			//				depth));
		//		}
		//	}
		//}
		//else
		{

			SATNormal = axes[ minAxis ];

			// If the normal is pointing into box1, flip it outwards
			if( SATNormal.fDot( (SATPoint - box1.fCenter( )) ) < 0.f )
				SATNormal *= -1.f;

			mResults.fPushBack( tIObbObbContact( SATPoint, SATNormal, depth ) );

			//tVec3f localPt = box0.fToLocal( SATPoint );
			//u32 index = localPt.fMaxAxisMagnitudeIndex( );

			//tVec3f normal(0);
			//normal[index] = 1;

			//if( localPt.fDot( normal ) < 0.0f )
			//	normal = -normal;

			//mNormals.fPushBack( box0.fAxis(0) * normal.x + box0.fAxis(1) * normal.y + box0.fAxis(2) * normal.z );

		}

		//mPoint = a.fCenter( ) + a.fProjection( mNormal ) + mNormal * minRadius;
		mIntersects = true;
	}

	

	//	void tIntersectionAabbObbWithContact::fIntersect( const tAabb<f32>& aAabb, const tObb<f32>& b )
	//{
	//	mIntersects = false;

	//	// first do quick sphere-sphere based rejection
	//	if( !tSphere<f32>( aAabb ).fIntersects( tSphere<f32>( b ) ) ) 
	//		return;

	//	const tObb<f32> box0( aAabb );
	//	const tObb<f32> box1( b );
	//	const tObb<f32> oldBox0( aAabb );
	//	const tObb<f32> oldBox1( b );

	//	tVec3f vectors[15];

	//	vectors[0] = box0.fAxis( 0 );
	//	vectors[1] = box0.fAxis( 1 );
	//	vectors[2] = box0.fAxis( 2 );
	//	vectors[3] = box1.fAxis( 0 );
	//	vectors[4] = box1.fAxis( 1 );
	//	vectors[5] = box1.fAxis( 2 );
	//	vectors[6] = box0.fAxis( 0 ).fCross( box1.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis );
	//	vectors[7] = box0.fAxis( 0 ).fCross( box1.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis );
	//	vectors[8] = box0.fAxis( 0 ).fCross( box1.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis );
	//	vectors[9] = box0.fAxis( 1 ).fCross( box1.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis );
	//	vectors[10] = box0.fAxis( 1 ).fCross( box1.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis );
	//	vectors[11] = box0.fAxis( 1 ).fCross( box1.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis );
	//	vectors[12] = box0.fAxis( 2 ).fCross( box1.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis );
	//	vectors[13] = box0.fAxis( 2 ).fCross( box1.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis );
	//	vectors[14] = box0.fAxis( 2 ).fCross( box1.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis );

	//	f32 minRadius = 0.0f;
	//	s32 minAxis = -1;

	//	for( u32 i = 0; i < 15; ++i )
	//	{	
	//		f32 sepRadius = box0.fSeparationOnAxis( box1, vectors[0] );
	//		if( sepRadius > 0.0f ) 
	//			return; //no collision

	//		//sepRadius is negative

	//		//look for smallest negative number
	//		if( sepRadius > minRadius || minAxis == -1 )
	//		{
	//			minRadius = sepRadius;
	//			minAxis = i;
	//		}				
	//	}

	//	sigassert( minAxis != -1 );

	//	mNormal = vectors[minAxis];
	//	f32 depth = -minRadius;

	//	tVec3f diff = box1.fCenter( ) - box0.fCenter( );
 //       if( diff.fDot( mNormal ) < 0 )
 //           mNormal = -mNormal;

	//	mPoint = a.fCenter( ) + a.fProjection( mNormal ) + mNormal * minRadius;
	//	mIntersects = true;
	//}

}}