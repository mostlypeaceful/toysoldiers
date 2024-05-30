#ifndef __tIntersectionRaySphere__
#define __tIntersectionRaySphere__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionRaySphere
	{
	protected:

		t					mT;
		b32					mIntersects;

	public:

		inline tIntersectionRaySphere( ) { }

		inline tIntersectionRaySphere( const tRay<t>& ray, const tSphere<t>& s )
		{
			fIntersect( ray, s );
		}

		inline tIntersectionRaySphere( const tSphere<t>& s, const tRay<t>& ray )
		{
			fIntersect( ray, s );
		}

		inline t fT( ) const { return mT; }
		inline b32 fIntersects( ) const { return mIntersects; }

		inline void fIntersect( const tRay<t>& ray, const tSphere<t>& s )
		{
			// compute a,b,c for solving quadratic equation
			const t a = ray.mExtent.fLengthSquared();
			const t b = 2 * ray.mExtent.fDot( ray.mOrigin - s.mCenter );
			const t c = ( ray.mOrigin - s.mCenter ).fLengthSquared() - fSquare( s.mRadius );

			// compute discriminant
			const t delta = b*b - 4*a*c;

			// check for no real solutions
			if( delta < 0 )
			{
				mIntersects = false;
				return;
			}
			else
			{
				// compute min and max intersection
				// time values
				const t deltaroot = fSqrt( delta );
				const t tmin = ( -b - deltaroot ) / ( 2 * a );
				const t tmax = ( -b + deltaroot ) / ( 2 * a );

				// now just deduce which of the two,
				// if any, is the correct intersection
				if( tmax < 0 || tmin > 1 )
				{
					mIntersects = false;
					return;
				}
				else if( tmin < 0 )
					mT = tmax;
				else
					mT = tmin;
			}

			mIntersects = true;
		}
	};

}}

#endif//__tIntersectionRaySphere__
