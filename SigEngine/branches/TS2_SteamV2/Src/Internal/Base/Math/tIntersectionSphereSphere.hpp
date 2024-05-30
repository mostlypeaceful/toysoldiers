#ifndef __tIntersectionSphereSphere__
#define __tIntersectionSphereSphere__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionSphereSphereWithContact
	{
	public:

		inline tIntersectionSphereSphereWithContact( ) 
			: mIntersects( false )
		{ }

		inline tIntersectionSphereSphereWithContact( const tSphere<t>& a, const tSphere<t>& b )
		{
			fIntersect( b, a );
		}

		inline void fIntersect( const tSphere<t>& a, const tSphere<t>& b )
		{
			mNormalFromAToB = b.fCenter( ) - a.fCenter( );
			t dist;
			mNormalFromAToB.fNormalizeSafe( tVector3<t>::cYAxis, dist );

			f32 touchingDist = a.fRadius( ) + b.fRadius( );
			mPenetration = touchingDist - dist;
			mIntersects = (mPenetration >= 0.f);
		}

		b32 fIntersects( ) const { return mIntersects; }

		// This stuff is not valid unless fIntersects is true.
		const tVector3<t>&	fNormalA( ) const		{ return mNormalFromAToB; }
		f32					fPenetration( ) const	{ return mPenetration; }

	private:
		b32 mIntersects;
		f32 mPenetration;
		tVector3<t> mNormalFromAToB;
	};

}}

#endif//__tIntersectionSphereSphere__
