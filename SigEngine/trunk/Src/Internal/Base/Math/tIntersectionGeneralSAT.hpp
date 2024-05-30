#ifndef __tIntersectionGeneralSAT__
#define __tIntersectionGeneralSAT__

#include "Physics/tContactPoint.hpp"

namespace Sig { namespace Math
{

	class tIntersectionGeneralSAT
	{
	protected:

		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

		// The points are on B, and the normal points to A

	public:
		struct tBody
		{
			tGrowableArray< Math::tVec3f > mFaces;
			tGrowableArray< Math::tVec3f > mEdges;
			tGrowableArray< Math::tVec3f > mPoints;

			inline void fMinMaxProject( f32& min, f32& max, const tVec3f& alongThisNormal) const
			{
				max = -cInfinity;
				min = cInfinity;

				for( u32 i = 0; i < mPoints.fCount( ); ++i )
				{
					f32 s = mPoints[ i ].fDot( alongThisNormal );
					min = fMin( s, min );
					max = fMax( s, max );
				}
			}

			tVec3f fSupportingPoint( const tVec3f& axis ) const
			{
				f32 min = cInfinity;
				u32 minID = 0;

				for( u32 i = 0; i < mPoints.fCount( ); ++i )
				{
					f32 s = mPoints[ i ].fDot( axis );
					if( s < min )
					{
						min = s;
						minID = i;
					}
				}

				return mPoints[ minID ];
			}

			tVec3f fCenter( ) const
			{
				tVec3f result = tVec3f::cZeroVector;

				for( u32 i = 0; i < mPoints.fCount( ); ++i )
					result += mPoints[ i ];

				result /= (f32)mPoints.fCount( );
				return result;
			}
		};

		tBody mBodyA;
		tBody mBodyB;
		Physics::tContactPoint mResult;

		tIntersectionGeneralSAT( ) 
		{ }

		// Will return normals sticking out of B.
		void fIntersect( b32 keepNonPenetrating = false );

		b32 fIntersects( ) const	{ return mIntersects; }
	};

}}

#endif//__tIntersectionGeneralSAT__
