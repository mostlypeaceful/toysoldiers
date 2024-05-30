#include "BasePch.hpp"
#include "tLinearFrustumCulling.hpp"

using namespace Sig::Math;

namespace Sig
{

	tLinearFrustumCulling::tLinearFrustumCulling( )
	{
	}

	tLinearFrustumCulling::~tLinearFrustumCulling( )
	{
	}

	void tLinearFrustumCulling::fClear( )
	{
		mObjects.fSetCount( 0 );
		mUserDatas.fSetCount( 0 );
		mFlags.fSetCount( 0 );
		mSpheres.fSetCount( 0 );
		mResults.fSetCount( 0 );
	}

	void tLinearFrustumCulling::fInsert( const Math::tVec4f& sphere, const tLFCObjectPtr& object, tUserData userData, tFlags flags )
	{
		sigassert( !object->fInList( ) );
		object->mIndex = mObjects.fCount( );

		mObjects.fPushBack( object );
		mUserDatas.fPushBack( userData );
		mFlags.fPushBack( flags );
		mSpheres.fPushBack( sphere );
		mResults.fPushBack( 0 );
	}

	void tLinearFrustumCulling::fMove( const Math::tVec4f& sphere, const tLFCObjectPtr& object )
	{
		mSpheres[ object->mIndex ] = sphere;
	}

	void tLinearFrustumCulling::fUpdateFlags( const tLFCObjectPtr& object, tFlags flags )
	{
		mFlags[ object->mIndex ] = flags;
	}

	void tLinearFrustumCulling::fRemove( const tLFCObjectPtr& object )
	{
		u32 index = object->mIndex;
		object->mIndex = ~0;
		sigassert( index != ~0 );
		sigassert( object == mObjects[ index ] );

		mObjects.fErase( index );
		mUserDatas.fErase( index );
		mFlags.fErase( index );
		mSpheres.fErase( index );
		mResults.fErase( index );

		// last one was swapped in, give it the correct index
		if( index != mObjects.fCount( ) )
			mObjects[ index ]->mIndex = index;
	}

	namespace
	{
		inline f32 fPlaneDist( const tVec4f& sphere, const tPlanef& plane )
		{
			return sphere.x * plane.a + sphere.y * plane.b + sphere.z * plane.c - sphere.w + plane.d;
		}

		inline f32 fFastMax( f32 a, f32 b )
		{
#ifdef platform_xbox360
			return (f32)__fsel( a-b, a, b );
#else
			return fMax( a, b );
#endif
		}

		inline f32 fFastDist( const tVec4f* sArray, const Math::tFrustumf& v )
		{
			const tVec4f& s1 = sArray[ 0 ];

			const f32 dist1 = fPlaneDist( s1, v[ 0 ] );
			const f32 dist2 = fPlaneDist( s1, v[ 1 ] );
			const f32 dist3 = fPlaneDist( s1, v[ 2 ] );
			const f32 dist4 = fPlaneDist( s1, v[ 3 ] );
			const f32 dist5 = fPlaneDist( s1, v[ 4 ] );
			const f32 dist6 = fPlaneDist( s1, v[ 5 ] );

			const f32 max1 = fFastMax( dist1, dist2 );
			const f32 max2 = fFastMax( dist3, dist4 );
			const f32 max3 = fFastMax( dist5, dist6 );
			const f32 max4 = fFastMax( max1, max2 );
			const f32 max = fFastMax( max3, max4 );

			return max;
		}
	}

	void tLinearFrustumCulling::fRefreshResults( const tFrustumArray& v )
	{
		const tFrustumArray frust = v;

		const tVec4f* sphere = mSpheres.fBegin( );
		const tVec4f* spheresEnd = mSpheres.fEnd( );

		tFrustumResultMask* results = mResults.fBegin( );

		while( sphere != spheresEnd )
		{
			fPreFetch( sphere );
			fPreFetch( results );

			*results = 0;

			for( u32 i = 0; i < frust.fCount( ); ++i )
			{
				if( fFastDist( sphere, frust[ i ] ) <= 0 )
					*results |= (1<<i);
			}

			++results;
			++sphere;
		}
	}
	
}
