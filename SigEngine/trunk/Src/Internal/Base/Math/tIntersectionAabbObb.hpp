#ifndef __tIntersectionAabbObb__
#define __tIntersectionAabbObb__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionAabbObb
	{
	protected:

		b32				mContained; ///< This will be true if the aabb 'a' is fully contained in aabb 'b'
		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

	public:

		inline tIntersectionAabbObb( ) { }

		inline tIntersectionAabbObb( const tAabb<t>& a, const tObb<t>& b )
		{
			fIntersect( a, b );
		}
		inline tIntersectionAabbObb( const tObb<t>& a, const tAabb<t>& b )
		{
			fIntersect( b, a );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline b32 fContained( ) const { return mContained; }

		inline void fIntersect( const tAabb<t>& aAabb, const tObb<t>& b )
		{
			mIntersects = false;
			mContained = false;

			// first do quick sphere-sphere based rejection
			if( !tSphere<t>( aAabb ).fIntersects( tSphere<t>( b ) ) ) return;

			const tObb<t> a( aAabb );

			if( a.fSeparatedOnAxis( b, a.fAxis( 0 ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 1 ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 2 ) ) ) return;
			if( a.fSeparatedOnAxis( b, b.fAxis( 0 ) ) ) return;
			if( a.fSeparatedOnAxis( b, b.fAxis( 1 ) ) ) return;
			if( a.fSeparatedOnAxis( b, b.fAxis( 2 ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 0 ).fCross( b.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 0 ).fCross( b.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 0 ).fCross( b.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 1 ).fCross( b.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 1 ).fCross( b.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 1 ).fCross( b.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 2 ).fCross( b.fAxis( 0 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 2 ).fCross( b.fAxis( 1 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;
			if( a.fSeparatedOnAxis( b, a.fAxis( 2 ).fCross( b.fAxis( 2 ) ).fNormalizeSafe( tVec3f::cYAxis ) ) ) return;

			mIntersects = true;
		}

	};

}}

#endif//__tIntersectionAabbObb__
