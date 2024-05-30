#ifndef __tFixedGrowingArray__
#define __tFixedGrowingArray__

#include "tCopier.hpp"

namespace Sig
{
	// Allows you to use "growing" behavior in a fixed (stack) memory footprint.
	//  (Use a ring buffer if you want to use heap memory)
	template<class t, int N>
	class tFixedGrowingArray
	{
		declare_reflector( );
	private:
		t	mItems[N];
		u32 mUsedCount;
	public:
		static const u32 cDimension = N;

		tFixedGrowingArray( u32 intialCount = 0 )
			: mUsedCount( intialCount )
		{ 
			cmp_assert( intialCount, <=, cDimension );
		}

		tArraySleeve<t> fToArraySleeve()
		{
			return tArraySleeve< t >( fBegin(), fCount() );
		}

		tArraySleeve<const t> fToArraySleeve() const
		{
			return tArraySleeve< const t >( fBegin(), fCount() );
		}

		inline b32			fAtCapacity( ) const				{ return fCapacity( ) == fCount( ); }
		inline u32			fCapacity( ) const					{ return cDimension; }
		inline u32			fCount( ) const						{ return mUsedCount; }
		inline t&			operator[]( const u32 i )			{ cmp_assert( i, < ,mUsedCount ); return mItems[i]; }
		inline const t&		operator[]( const u32 i ) const		{ cmp_assert( i, < ,mUsedCount ); return mItems[i]; }

		inline void			fPushBack( const t& item )			{ cmp_assert( mUsedCount, <, cDimension ); mItems[ mUsedCount ] = item; ++mUsedCount; }

		inline t fPopBack( )
		{
			cmp_assert( mUsedCount, >, 0 );
			t o = fBack( );
			--mUsedCount;
			mItems[mUsedCount] = t( );
			return o;
		}

		inline t*			fBegin( )							{ return mItems; }
		inline const t*		fBegin( ) const						{ return mItems; }
		inline t*			fEnd( )								{ return mItems + mUsedCount; }
		inline const t*		fEnd( ) const						{ return mItems + mUsedCount; }

		inline t&			fFront( )							{ return mItems[0]; }
		inline const t&		fFront( ) const						{ return mItems[0]; }
		inline t&			fBack( )							{ return mItems[mUsedCount - 1]; }
		inline const t&		fBack( ) const						{ return mItems[mUsedCount - 1]; }

		inline void			fSetCapacity( u32 count )			{ cmp_assert( count, <=, cDimension ); /*no-op*/ }
		inline void			fSetCount( u32 count )
		{ 
			cmp_assert( count, <=, cDimension );

			if( count < mUsedCount )
			{
				// assign default object to all "dead" objects
				tCopier<t>::fAssignToDestroy( mItems + count, t( ), mUsedCount - count );
			}

			mUsedCount = count; 
		}
		inline void fClear( ) { fSetCount( 0 ); }

		
		template<class u>
		s32 fIndexOf( const u& object, u32 indexStart = 0 ) const
		{
			for( u32 i = indexStart; i < fCount( ); ++i )
				if( mItems[i] == object )
					return i;
			return -1;
		}

		void fFill( const t& val )
		{
			for( u32 i = 0; i < fCount( ); ++i )
				mItems[i] = val;
		}

		inline void fErase( u32 index )
		{
			cmp_assert( index, <, mUsedCount );
			mItems[index] = mItems[mUsedCount-1];
			mItems[mUsedCount-1] = t( );
			mUsedCount -= 1;
		}

		void fEraseOrdered( u32 start, u32 count )
		{
			cmp_assert( start, <, mUsedCount );
			log_assert( start + count <= mUsedCount, "start=" << start << " count=" << count << " start+count=" << (start+count) << " mUsedCount=" << mUsedCount );

			tCopier<t>::fCopyOverlapped(
				fBegin( ) + start,
				fBegin( ) + start + count,
				mUsedCount - start - count );

			tCopier<t>::fAssignToDestroy( &mItems[ mUsedCount - count ], t( ), count );
			mUsedCount -= count;
		}

		inline void fEraseOrdered( u32 index )
		{
			fEraseOrdered( index, 1 );
		}

		void fInsertSafe( u32 index, const t& object, u32 numItems=1 )
		{
			if( index > fCount( ) ) index = fCount( );
			fInsert( index, object, numItems );
		}

		void fInsert( u32 index, const t& object, u32 numItems=1 )
		{
			cmp_assert( index, <, mUsedCount );

			const u32 count = mUsedCount + numItems;
			cmp_assert( count, <, cDimension );

			// Items before index are already in the right spot

			// Move items at or after index up by numItems
			tCopier<t>::fCopyOverlapped( mItems + index + numItems, mItems + index, mUsedCount - index );

			// Now insert the items
			tCopier<t>::fAssign( mItems + index, object, numItems );

			mUsedCount = count;
		}

		template<class u>
		const t* fFind( const u& object ) const
		{
			for( u32 i = 0; i < mUsedCount; ++i )
				if( mItems[i] == object )
					return &mItems[i];
			return 0;
		}

		template<class u>
		t* fFind( const u& object )
		{
			for( u32 i = 0; i < mUsedCount; ++i )
				if( mItems[i] == object )
					return &mItems[i];
			return 0;
		}

		template<class u>
		t& fFindOrAdd( const u& object )
		{
			for( u32 i = 0; i < mUsedCount; ++i )
				if( mItems[i] == object )
					return mItems[i];
			fPushBack( object );
			return fBack( );
		}

	};
}

#endif //ndef __tFixedGrowingArray__
