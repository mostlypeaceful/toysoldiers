#ifndef __tGrowableArray__
#define __tGrowableArray__

#include "tCopier.hpp"
#include "tDynamicArray.hpp"

namespace Sig
{
	///
	/// \brief Basically a replacement for std::vector, but not a syntactical drop in; i.e.,
	/// functions are named differently, and behavior is different in certain areas. The basic
	/// idea is a an array that keeps its capacity separate from its actual count, so you can
	/// pre-allocate and fill the array lazily, or just use fPushBack, etc.
	///
	/// \note One thing that's very nice about tGrowableArray is that it will use template trickery
	/// to determine if your type is built-in or not (i.e., an int, a pointer, etc.), and use
	/// optimal copying methods (i.e., memcpy) appropriately. On the other hand, if you have non-POD
	/// types with constructors, etc, it will appropriately choose that path, all statically (i.e.,
	/// no run-time overhead).
	template<class t>
	class tGrowableArray
	{
	private:

		u32					mCount;
		tDynamicArray<t>	mItems;

	public:

		typedef t*			tIterator;
		typedef const t*	tConstIterator;

		typedef t			tValue;
		typedef const t		tConstValue;

		inline tGrowableArray( )
			: mCount( 0 )
		{
		}

		inline explicit tGrowableArray( u32 startItems )
			: mCount( startItems ), mItems( startItems )
		{
		}

		inline explicit tGrowableArray( const t* begin, const t* end )
			: mCount( 0 ), mItems( (u32)(end-begin) )
		{
			fInsert( 0, begin, end );
		}

		inline u32			fCount( ) const						{ return mCount; }
		inline u32			fCapacity( ) const					{ return mItems.fCount( ); }
		inline t*			fBegin( )							{ return mItems.fBegin( ); }
		inline const t*		fBegin( ) const						{ return mItems.fBegin( ); }
		inline t*			fEnd( )								{ return mItems.fBegin( ) + mCount; }
		inline const t*		fEnd( ) const						{ return mItems.fBegin( ) + mCount; }
		inline t&			fFront( )							{ return mItems.fFront( ); }
		inline const t&		fFront( ) const						{ return mItems.fFront( ); }
		inline t&			fBack( )							{ cmp_assert( mCount, >, 0 ); return mItems[mCount - 1]; }
		inline const t&		fBack( ) const						{ cmp_assert( mCount, >, 0 ); return mItems[mCount - 1]; }
		inline t&			operator[]( const u32 i )			{ cmp_assert( i, <, mCount ); return mItems[i]; }
		inline const t&		operator[]( const u32 i ) const		{ cmp_assert( i, <, mCount ); return mItems[i]; }
		inline t&			fIndex( const u32 i )				{ cmp_assert( i, <, mCount ); return mItems[i]; }
		inline const t&		fIndex( const u32 i ) const			{ cmp_assert( i, <, mCount ); return mItems[i]; }

		inline tArraySleeve<t> fSleeve( ) { return tArraySleeve<t>( fBegin( ), fCount( ) ); }
		inline tArraySleeve<const t> fSleeve( ) const { return tArraySleeve<const t>( fBegin( ), fCount( ) ); }

		inline tArraySleeve<t> fSplitSleeve( u32 start, u32 count = ~0 ) 
		{ 
			sigcheckfail( start < fCount( ), return tArraySleeve<t>( ) );
			return tArraySleeve<t>( fBegin( ) + start, fMin( count, fCount( ) - start ) ); 
		}

		inline tArraySleeve<const t> fSplitSleeve( u32 start, u32 count = ~0 ) const 
		{
			sigcheckfail( start < fCount( ), return tArraySleeve<const t>( ) );
			return tArraySleeve<const t>( fBegin( ) + start, fMin( count, fCount( ) - start ) ); 
		}

		inline void fDeleteArray( )
		{
			mCount = 0;
			mItems.fDeleteArray( );
		}

		inline void fFill( const t& object )
		{
			tCopier<t>::fAssign( fBegin( ), object, fCount( ) );
		}

		inline u32 fElementSizeOf( ) const
		{
			return sizeof( t );
		}

		inline u32 fTotalSizeOf( ) const
		{
			return sizeof( t ) * fCount( );
		}

		inline void fSwap( tGrowableArray& other )
		{
			Sig::fSwap( mCount, other.mCount );
			mItems.fSwap( other.mItems );
		}

		inline void fDisown( tArraySleeve<t> & nextOwner ) 
		{
			mItems.fDisown( nextOwner );

			// Reset with the count of items that have actually been constructed
			// The underlying memory manager uses headers to determine size for freeing
			// so we don't have to worry about it here
			nextOwner = tArraySleeve<t>( nextOwner.fBegin( ), mCount );

			mCount = 0;
		}

		inline void fDisown( tGrowableArray<t> & nextOwner ) 
		{
			mItems.fDisown( nextOwner.mItems );
			nextOwner.mCount = mCount;
			mCount = 0;
		}

		void fSetCapacity( u32 capacity )
		{
			tDynamicArray<t> newArray( capacity );
			mCount = fMin( mCount, capacity );
			tCopier<t>::fCopy( newArray.fBegin( ), mItems.fBegin( ), mCount );
			mItems.fSwap( newArray );
		}

		inline void fGrowCapacity( u32 increaseBy )
		{
			fSetCapacity( mItems.fCount( ) + increaseBy );
		}

		void fReserve( u32 needed )
		{
			const s32 toAdd = needed - ( fCapacity( ) - mCount );
			if( toAdd > 0 )
				fGrowCapacity( toAdd );
		}

		void fSetCount( u32 count )
		{
			if( count < mCount )
			{
				// assign default object to all "dead" objects
				tCopier<t>::fAssignToDestroy( mItems.fBegin( ) + count, t( ), mCount - count );
			}
			else if( count > mItems.fCount( ) )
			{
				tDynamicArray<t> newArray( count );
				tCopier<t>::fCopy( newArray.fBegin( ), mItems.fBegin( ), mCount );
				mItems.fSwap( newArray );
			}

			mCount = count;
		}

		inline void fClear( )
		{
			fSetCount( 0 );
		}

		inline u32 fComputeIncreasedCapacity( u32 newCount )
		{
			static const u32 minSize = 10;
			static const u32 mul = 2;
			static const u32 maxSize = 2048;
			return fMin( fMax( minSize, mul * newCount ), newCount + maxSize );
		}

		void fGrowCount( u32 increaseBy )
		{
			const u32 count = mCount + increaseBy;

			if( count > mItems.fCount( ) )
			{
				const u32 capacity = fComputeIncreasedCapacity( count );
				cmp_assert( capacity, >=, count );
				tDynamicArray<t> newArray( capacity );
				tCopier<t>::fCopy( newArray.fBegin( ), mItems.fBegin( ), mCount );
				mItems.fSwap( newArray );
			}

			mCount = count;
		}

		void fInsertSafe( u32 index, const t& object )
		{
			if( index > fCount( ) ) index = fCount( );
			fInsert( index, object );
		}

		void fInsert( u32 index, const t& object )
		{
			fInsert( index, &object, 1 );
		}

		void fInsert( u32 index, const t* objects, u32 numItems=1 )
		{
			log_sigcheckfail( objects + numItems <= fBegin( ) || fEnd( ) <= objects, "Container cannot insert into itself safely!", Log::fFatalError( "fInsert exploded" ) );
			cmp_assert( index, <=, mCount );

			const u32 count = mCount + numItems;

			if( count <= fCapacity( ) )
			{
				// move elements after insertion point back to make room
				tCopier<t>::fCopyOverlapped( fBegin() + index + numItems, fBegin() + index, mCount - index );

				// now copy 'numItems' many copies of the new desired object starting at the supplied index
				tCopier<t>::fCopy( fBegin( ) + index, objects, numItems );
			}
			else
			{
				const u32 capacity = fComputeIncreasedCapacity( count );
				tDynamicArray<t> newArray( capacity );

				// first copy in old items to new array
				tCopier<t>::fCopy( newArray.fBegin( ), mItems.fBegin( ), index );

				// now copy ['objects'..'objects'+numItems) to the new array at the supplied index
				tCopier<t>::fCopy( newArray.fBegin( ) + index, objects, numItems );

				// now copy the rest of the old items to the end of the new array
				tCopier<t>::fCopy( newArray.fBegin( ) + index + numItems, mItems.fBegin( ) + index, mCount - index );

				mItems.fSwap( newArray );
			}

			mCount = count;
		}

		void fInsert( u32 index, const t* begin, const t* end )
		{
			fInsert( index, begin, (u32)(end-begin) );
		}

		template< class U >
		inline void fJoin( const U& other )
		{
			fInsert( fCount( ), other.fBegin( ), other.fCount( ) );
		}

		template< class U >
		inline void fIntersect( const U& other )
		{
			for( s32 i = fCount( ) - 1; i >= 0; --i )
			{
				if( !other.fFind( operator[]( i ) ) )
					fErase( i );
			}
		}

		template< class U >
		inline void fEraseAll( const U& other )
		{
			for( s32 i = fCount( ) - 1; i >= 0; --i )
			{
				if( other.fFind( operator[]( i ) ) )
					fErase( i );
			}
		}

		inline void fErase( u32 index )
		{
			cmp_assert( index, <, mCount );
			mItems[index] = mItems[mCount-1];
			mItems[mCount-1] = t( );
			mCount -= 1;
		}

		void fEraseOrdered( u32 start, u32 count )
		{
			cmp_assert( start, <, mCount );
			log_assert( start + count <= mCount, "start=" << start << " count=" << count << " start+count=" << (start+count) << " mCount=" << mCount );

			tCopier<t>::fCopyOverlapped(
				mItems.fBegin( ) + start,
				mItems.fBegin( ) + start + count,
				mCount - start - count );

			tCopier<t>::fAssignToDestroy( &mItems[ mCount - count ], t( ), count );
			mCount -= count;
		}

		template < typename F >
		inline void fEraseAllOrderedWhere( F condition )
		{
			t* begin = fBegin();
			t* new_end = std::remove_if( begin, fEnd(), condition );
			mCount = (u32)(new_end - begin);
		}

		inline void fEraseOrdered( u32 index )
		{
			fEraseOrdered( index, 1 );
		}

		template<class u>
		s32 fIndexOf( const u& object, u32 indexStart = 0 ) const
		{
			for( u32 i = indexStart; i < mCount; ++i )
				if( mItems[i] == object )
					return i;
			return -1;
		}

		template<class u>
		const t* fFind( const u& object ) const
		{
			for( u32 i = 0; i < mCount; ++i )
				if( mItems[i] == object )
					return &mItems[i];
			return 0;
		}

		template<class u>
		t* fFind( const u& object )
		{
			for( u32 i = 0; i < mCount; ++i )
				if( mItems[i] == object )
					return &mItems[i];
			return 0;
		}

		template<class F>
		const t* fFindIf( const F& functor ) const
		{
			for( u32 i = 0; i < mCount; ++i )
				if( functor( mItems[i] ) )
					return &mItems[i];
			return 0;
		}

		template<class F>
		t* fFindIf( const F& functor )
		{
			for( u32 i = 0; i < mCount; ++i )
				if( functor( mItems[i] ) )
					return &mItems[i];
			return 0;
		}

		template<class u>
		t& fFindOrAdd( const u& object )
		{
			for( u32 i = 0; i < mCount; ++i )
				if( mItems[i] == object )
					return mItems[i];
			tGrowableArray<t>::fPushBack( object );
			return tGrowableArray<t>::fBack( );
		}

		template<class u>
		b32 fFindAndErase( const u& object )
		{
			const t* i = fFind( object );

			if( i )
			{
				fErase( fPtrDiff( i, fBegin( ) ) );
				return true;
			}

			return false;
		}

		template<class u>
		inline b32 fFindAndEraseOrdered( const u& object )
		{
			const t* i = fFind( object );

			if( i )
			{
				fEraseOrdered( fPtrDiff( i, fBegin( ) ) );
				return true;
			}

			return false;
		}

		inline t& fPushBack( const t& object )
		{
			fInsert( mCount, object );
			return fBack( );
		}
		
		inline t & fPushBack( )
		{
			fGrowCount( 1 );
			return fBack( );
		}

		inline void fPushBackNoGrow( const t& object )
		{
			++mCount;
			cmp_assert( mCount, <=, mItems.fCount( ) );
			fBack( ) = object;
		}

		inline t fPopBack( )
		{
			cmp_assert( mCount, >, 0 );
			t o = fBack( );
			--mCount;
			mItems[mCount] = t( );
			return o;
		}

		inline void fPushFront( const t& object )
		{
			fInsert( 0, object );
		}

		inline t fPopFront( )
		{
			cmp_assert( mCount, >, 0 );
			t o = fFront( );
			fEraseOrdered( 0 );
			return o;
		}

		// Compare this array to another.
		template<class u>
		inline b32 fEqual( const u& otherArray ) const
		{
			if( fCount( ) != otherArray.fCount( ) )
				return false;

			for( u32 i = 0; i < fCount( ); ++i )
				if( operator[]( i ) != otherArray[ i ] )
					return false;

			return true;
		}
	};
	
	template< typename T, typename U >
	inline b32 operator==( const tGrowableArray<T>& lhs, const tGrowableArray<U>& rhs )
	{
		if( lhs.fCount() != rhs.fCount() )
			return false;
		for( u32 i=0; i<lhs.fCount(); ++i )
			if( lhs[i] != rhs[i] )
				return false;
		return true;
	}
	
	template< typename T, typename U >
	inline b32 operator!=( const tGrowableArray<T>& lhs, const tGrowableArray<U>& rhs )
	{
		return !(lhs == rhs);
	}

	template<class t>
	tDynamicArray<t>& tDynamicArray<t>::operator=( const tGrowableArray<t>& other )
	{
		fNewArray( other.fCount( ) );
		tCopier<t>::fCopy( tBase::fBegin( ), other.fBegin( ), tBase::fCount( ) );
		return *this;
	}

	typedef tGrowableArray< byte > tGrowableBuffer;
}

#endif //ndef __tGrowableArray__
