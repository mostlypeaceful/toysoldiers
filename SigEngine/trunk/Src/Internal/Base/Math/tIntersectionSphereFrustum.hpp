//------------------------------------------------------------------------------
// \file tIntersectionSphereFrustum.hpp - 06 Sep 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tIntersectionSphereFrustum__
#define __tIntersectionSphereFrustum__

namespace Sig { namespace Math
{

	///
	/// \class tIntersectionSphereFrustum
	/// \brief 
	template<class t>
	class tIntersectionSphereFrustum
	{
	protected:

		b32 mContained;
		b32	mIntersects;

	public:

		inline tIntersectionSphereFrustum( ) { }

		inline tIntersectionSphereFrustum( const tSphere<t>& sphere, const tFrustum<t>& frustum )
		{
			fIntersect( sphere, frustum );
		}

		inline tIntersectionSphereFrustum( const tFrustum<t>& frustum, const tSphere<t>& sphere )
		{
			fIntersect( sphere, frustum );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline b32 fContains( ) const { return mContained; }

		inline void fIntersect( const tSphere<t>& sphere, const tFrustum<t>& frustum )
		{
			mIntersects = frustum.fIntersectsOrContains( sphere, mContained );
		}
	};

}} // ::Sig::Math

#endif//__tIntersectionSphereFrustum__
