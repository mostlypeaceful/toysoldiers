#ifndef __tIntersectionTriangleFrustum__
#define __tIntersectionTriangleFrustum__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionTriangleFrustum
	{
	protected:

		b32				mContained; ///< This will be true if the tri is fully contained in the frustum
		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

	public:

		inline tIntersectionTriangleFrustum( ) { }

		inline tIntersectionTriangleFrustum( const tTriangle<t>& tri, const tFrustum<t>& frustum )
		{
			fIntersect( tri, frustum );
		}

		inline tIntersectionTriangleFrustum( const tFrustum<t>& frustum, const tTriangle<t>& tri )
		{
			fIntersect( tri, frustum );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline b32 fContained( ) const { return mContained; }

		inline void fIntersect( const tTriangle<t>& tri, const tFrustum<t>& frustum )
		{

			// Extremely basic in/out test on each vertex. This will not 
			// catch edges that penetrate the frustum.
			mContained = true;
			mIntersects = true;
			b32 outs[3] = { false, false, false };
			for( u32 i = 0; i < tFrustum<t>::cPlaneCount; ++i )
			{
				if( frustum[ i ].fSignedDistance( tri.mA ) <= 0 )
					outs[ 0 ] = true;

				if( frustum[ i ].fSignedDistance( tri.mB ) <= 0 )
					outs[ 1 ] = true;

				if( frustum[ i ].fSignedDistance( tri.mC ) <= 0 )
					outs[ 2 ] = true;
			}

			const u32 numOuts = outs[0] + outs[1] + outs[2];

			if( numOuts == 0 )
			{
				mContained = true;
				mIntersects = false;
			}
			else if( numOuts == 3) 
			{
				mContained = false;
				mIntersects = false;
			}
			else
			{
				mContained = false;
				mIntersects = true;
			}

		}

	};

}}

#endif//__tIntersectionTriangleFrustum__
