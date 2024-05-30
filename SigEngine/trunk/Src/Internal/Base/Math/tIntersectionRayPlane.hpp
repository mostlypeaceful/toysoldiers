#ifndef __tIntersectionRayPlane__
#define __tIntersectionRayPlane__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionRayPlane
	{
	protected:

		t				mT;
		b32				mIntersects;
		b32				mParallel;

	public:

		inline tIntersectionRayPlane( ) { }

		inline tIntersectionRayPlane( const tRay<t>& ray, const tPlane<t>& plane )
		{
			fIntersect( ray, plane );
		}

		inline tIntersectionRayPlane( const tPlane<t>& plane, const tRay<t>& ray )
		{
			fIntersect( ray, plane );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline b32 fParallel( ) const { return mParallel; }
		inline t fT( ) const { return mT; }

		inline void fIntersect( const tRay<t>& ray, const tPlane<t>& plane )
		{
			const t dirDotN = ray.mExtent.fDot( plane.fGetNormal( ) );
			const t signedD = plane.fSignedDistance( ray.mOrigin );

			if( fAbs( dirDotN ) < (t)0.00001f )
			{
				// ray is parallel to the plane

				mT = (t)1; // arbitrary, technically all of the ray could be in the plane (or not in the plane)
				mParallel = true;

				// we say the ray intersects if it is coincident with the plane
				mIntersects = fAbs( signedD ) < (t)0.00001f;
				return;
			}

			// ray intersects
			mT = -signedD / dirDotN;
			mParallel = false;
			mIntersects = fInBounds( mT, (t)0, (t)1 );
		}
	};

}}

#endif//__tIntersectionRayPlane__
