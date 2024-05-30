#ifndef __tIntersectionRayTriangle__
#define __tIntersectionRayTriangle__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionRayTriangle
	{
	protected:

		tVector3<t>	mNormal;
		t			mT;
		tVector3<t>	mBarycentric;
		b32			mIntersects;

	public:

		inline tIntersectionRayTriangle( ) { }

		inline tIntersectionRayTriangle( const tRay<t>& ray, const tTriangle<t>& tri )
		{
			fIntersect( ray, tri );
		}

		inline tIntersectionRayTriangle( const tTriangle<t>& tri, const tRay<t>& ray )
		{
			fIntersect( ray, tri );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline t fT( ) const { return mT; }
		inline const tVector3<t>& fNormal( ) const { return mNormal; }
		inline const tVector3<t>& fBarycentric( ) const { return mBarycentric; }

		inline void fIntersect( const tRay<t>& ray, const tTriangle<t>& tri )
		{
			const t zeroTolerance = 0.000001f;

			// compute the offset origin, edges, and normal
			const tVector3<t> diff = ray.mOrigin - tri.mA;
			const tVector3<t> edge1 = tri.mB - tri.mA;
			const tVector3<t> edge2 = tri.mC - tri.mA;
			mNormal = edge1.fCross(edge2);

			// Solve Q + t*D = b1*E1 + b2*E2 (Q = diff, D = ray direction,
			// E1 = edge1, E2 = edge2, N = Cross(E1,E2)) by
			//   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
			//   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
			//   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)

			t dDotN = ray.mExtent.fDot(mNormal);
			t sign;
			if (dDotN < -zeroTolerance)
			{
				sign = (t)-1.0;
				dDotN = -dDotN;
			}
			else
			{
				// Ray and triangle are either parallel or the normal is pointing away from the ray
				mIntersects = false;
				return;
			}

			t dDotQxE2 = sign*ray.mExtent.fDot(diff.fCross(edge2));
			if (dDotQxE2 >= (t)0.0)
			{
				t dDotE1xQ = sign*ray.mExtent.fDot(edge1.fCross(diff));
				if (dDotE1xQ >= (t)0.0)
				{
					if (dDotQxE2 + dDotE1xQ <= dDotN)
					{
						// line intersects triangle, check if ray does
						t fQdN = -sign*diff.fDot(mNormal);
						if (fQdN >= (t)0.0)
						{
							// ray intersects triangle
							t inv = ((t)1.0)/dDotN;
							mT = fQdN*inv;
							mBarycentric.y = dDotQxE2*inv;
							mBarycentric.z = dDotE1xQ*inv;
							mBarycentric.x = (t)1.0 - mBarycentric.y - mBarycentric.z;
							mIntersects = fInBounds( mT, (t)0, (t)1 );
							return;
						}
						// else: t < 0, no intersection
					}
					// else: b1+b2 > 1, no intersection
				}
				// else: b2 < 0, no intersection
			}
			// else: b1 < 0, no intersection

			mIntersects = false;
		}

	};

}}

#endif//__tIntersectionRayTriangle__
