#ifndef __tIntersectionAabbAabb__
#define __tIntersectionAabbAabb__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionAabbAabb
	{
	protected:

		b32				mContained; ///< This will be true if the aabb 'a' is fully contained in aabb 'b'
		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

	public:

		inline tIntersectionAabbAabb( ) { }

		inline tIntersectionAabbAabb( const tAabb<t>& a, const tAabb<t>& b )
		{
			fIntersect( a, b );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline b32 fContained( ) const { return mContained; }

		inline void fIntersect( const tAabb<t>& a, const tAabb<t>& b )
		{
			if( !a.fIntersects( b ) )
			{
				mIntersects = false;
				mContained = false;
			}
			else if( b.fContains( a ) )
			{
				mIntersects = true;
				mContained = true;
			}
			else
			{
				mIntersects = true;
				mContained = false;
			}
		}

	};

}}

#endif//__tIntersectionAabbAabb__
