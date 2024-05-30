//------------------------------------------------------------------------------
// \file tIntersectionLineCylinder.hpp - 22 Jul 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tIntersectionLineCylinder__
#define __tIntersectionLineCylinder__

#include "Math/tRay.hpp"
#include "Math/tConvexHull.hpp" // tCylinder
#include "Math/tSphere.hpp"
#include "Math/tIntersectionLineSphere.hpp"

namespace Sig { namespace Math
{
	namespace LineCylinderDetails
	{
		// "Cylinder Space" is defined as unscaled, with the cylinder at origin and +y as the primary axis.

		/// \brief In cylinder space, any line <-> cylinder intersection when viewed on the +Y facing plane will
		/// simplify to a line <-> circle intersection.  Project the ray to said plane and find intersections.
		template<class t>
		tIntersectionSegment<t> fLineWithinRadiusOfAxis( const tRay<t>& rayInCylinderSpace, f32 radius )
		{
			//                            _______,-'                
			//                        ,-""    ,-'""-,               
			//           __,         /     ,-'  maxT \              
			//          ,-'|        /   ,-'           \             
			//       ,-' .mExtent  | ,-'               |            
			//      '             ,+'minT       radius |            
			//rayInCylinderSpace-' |         o---------+            
			//     .mOrigin ,-'    |      (0,0,0)      |  z         
			//           ( )       |                   |  ^         
			//        ,-'           \                 /   |         
			//     ,-'               \               /    |         
			//  ,-'                   "-__       __-"     +---> x   
			//-'                          """""""                   

			const tVector3<t>	yPlaneOrigin			= tVector3<t>( rayInCylinderSpace.mOrigin ).fProjectToXZ( );
			const tVector3<t>	yPlaneExtent			= tVector3<t>( rayInCylinderSpace.mExtent ).fProjectToXZ( );
			const tRay<t>		yPlaneRay				( yPlaneOrigin, yPlaneExtent );
			const tSphere<t>	circle					( radius );

			return fIntersectLineSphere( yPlaneRay, circle );
		}

		/// \brief In cylinder space, any line <-> cylinder intersection will be within and potentially bounded by the
		/// planes defined by the caps of the cylinder.  Find said intersections.
		template<class t>
		tIntersectionSegment<t> fLineWithinCapPlanes( const tRay<t>& rayInCylinderSpace, f32 halfHeight )
		{
			//         Side view of Capsule                    
			//    With example lines #ed with Cases            
			//                     /              /            
			//---(3)--------------/--------------/-            
			//                   /              /              
			//              cap /              /               
			//. . . . . .______/_____. . . . ./. .             
			//          |     /      |       /                 
			//---(2)----+----/-----+-+------/------  y         
			//          |   /  o   |_|     /         ^         
			//          |  /origin   |    /          |         
			//. . . . . |_/__________| . / . . . .   |         
			//           /   cap        /            +---> x   
			//          /(1)           /(also 1)               

			if( !fEqual( rayInCylinderSpace.mExtent.y, 0 ) )
			{
				// Case 1: Ray is not at a right angle to cylinder axis, and will thus (eventually) intersect cap planes./
				//
				//	We solve Solve t for both equations:
				//		+halfHeight = mOrigin.y + t * mExtent.y
				//		-halfHeight = mOrigin.y + t * mExtent.y
				//	Where the left hand side represents the cylinder cap equation, and the right the line itself.

				const t a = ( +halfHeight - rayInCylinderSpace.mOrigin.y ) / rayInCylinderSpace.mExtent.y;
				const t b = ( -halfHeight - rayInCylinderSpace.mOrigin.y ) / rayInCylinderSpace.mExtent.y;
				
				const t min = fMin( a, b );
				const t max = fMax( a, b );

				return tIntersectionSegment<t>( min, max );
			}
			else if( fInBounds( rayInCylinderSpace.mOrigin.y, -halfHeight, +halfHeight ) )
			{
				// Case 2: Ray at a right angle to cylinder axis.  Will never intersect cap planes but is between them.
				return tIntersectionSegment<t>::cInfinite;
			}
			else
			{
				// Case 3: Ray at a right angle to cylinder axis.  Will never intersect cap planes and is outside them.
				return tIntersectionSegment<t>::cEmpty;
			}
		}
	}

	template<class t>
	tIntersectionSegment<t> fIntersectLineCylinder( const tRay<t>& ray, const tCylinder& c )
	{
		const tRay<t> rayInCylinderSpace = ray.fTransform( c.fGetTransform( ).fInverse( ) );

		using namespace LineCylinderDetails;

		return	fLineWithinRadiusOfAxis( rayInCylinderSpace, c.mRadius )
			&	fLineWithinCapPlanes( rayInCylinderSpace, c.mHalfHeight );
	}

	template<class t>
	tIntersectionSegment<t> fIntersectLineCylinder( const tCylinder& s, const tRay<t>& ray )
	{
		return fIntersectLineCylinder( ray, s );
	}
}} // namespace Sig::Math

#endif //ndef __tIntersectionLineCylinder__
