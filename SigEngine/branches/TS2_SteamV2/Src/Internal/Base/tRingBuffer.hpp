#ifndef __tRingBuffer__
#define __tRingBuffer__

namespace Sig
{


	///
	/// \brief Encapsulates a dynamically-sizeable ring buffer; the ring buffer
	/// will wrap items in an array based on the user-set capacity; it will only 
	/// grow if the user explicitly calls a grow method. Queue-style, FIFO putting
	/// and getting.
	template<class t>
	class tRingBuffer : protected tDynamicArray<t>
	{
		declare_reflector( );

	private:

		u32 mHead;
		u32 mTail;
		u32 mNumItems;

	private:

		inline u32 fWrapIndex( u32 index ) const
		{
			return index % fCapacity( );
		}

		inline u32 fNextIndex( u32 index ) const
		{
			return ( index + 1 ) % fCapacity( );
		}

		inline u32 fPrevIndex( u32 index ) const
		{
			return ( index > 0 ) ? ( index - 1 ) : ( fCapacity( ) - 1 );
		}

	public:

		typedef tDynamicArray<t> tBase;

		///
		/// \brief No op constructor, required for load-in-place.
		inline tRingBuffer( tNoOpTag )
			: tBase( cNoOpTag )
		{
		}

		///
		/// \brief Default constructor, does nothing appreciable.
		inline tRingBuffer( )
			: mHead( 0 )
			, mTail( 0 )
			, mNumItems( 0 )
		{
		}

		///
		/// \brief Construct with initial capacity.
		inline explicit tRingBuffer( u32 capacity ) 
			: tBase( capacity )
			, mHead( 0 )
			, mTail( 0 )
			, mNumItems( 0 )
		{
		}

		///
		/// \brief Query for the capacity of the ring buffer. This value will only
		/// change if you call fResize( ). Otherwise, the queue will wrap and overwrite
		/// previous older entries if you overflow (this is often desirable, as in a history buffer).
		inline u32 fCapacity( ) const
		{
			return tBase::fCount( );
		}

		///
		/// \brief Query for the number of items inserted into the ring buffer.
		/// \note This value will be capped against the capacity.
		inline u32 fNumItems( ) const
		{
			return mNumItems;
		}

		///
		/// \brief Access the ring buffer in "logical" order, where index 0
		/// corresponds to the first item inserted, and index fNumItems( ) - 1
		/// corresponds to the latest itme inserted.
		inline t& operator[]( u32 i )
		{
			sigassert( i < mNumItems );
			return tBase::operator []( fWrapIndex( mTail + i ) );
		}

		///
		/// \brief Access the ring buffer in "logical" order, where index 0
		/// corresponds to the first item inserted, and index fNumItems( ) - 1
		/// corresponds to the latest itme inserted.
		inline const t& operator[]( u32 i ) const
		{
			sigassert( i < mNumItems );
			return tBase::operator []( fWrapIndex( mTail + i ) );
		}

		///
		/// \brief Retrieve reference to the latest item inserted.
		inline t& fFront( )
		{
			return (*this)[ mNumItems - 1 ];
		}

		///
		/// \brief Retrieve reference to the latest item inserted.
		inline const t& fFront( ) const
		{
			return (*this)[ mNumItems - 1 ];
		}

		///
		/// \brief Retrieve reference to the first item inserted.
		inline t& fBack( )
		{
			return (*this)[ 0 ];
		}

		///
		/// \brief Retrieve reference to the first item inserted.
		inline const t& fBack( ) const
		{
			return (*this)[ 0 ];
		}

		///
		/// \brief Swap the underlying data of two ring-buffers; no allocations or
		/// deallocations required, very quick.
		inline void fSwap( tRingBuffer& other )
		{
			tBase::fSwap( other );
			Sig::fSwap( mHead, other.mHead );
			Sig::fSwap( mTail, other.mTail );
			Sig::fSwap( mNumItems, other.mNumItems );
		}

		///
		/// \brief Increase or decrease the underlying capacity of the ring buffer.
		///
		/// If the new capacity is larger, the number of items will be the same after
		/// the call, and all items will be in the same "relative" place; if the new capacity
		/// is smaller than the current number of items, then the latest 'newCapacity' items
		/// will be kept, and number of items will equal capacity.
		inline void fResize( u32 newCapacity )
		{
			tRingBuffer newRb( newCapacity );
			const u32 minSize = fMin( mNumItems, newRb.fCapacity( ) );

			// we factor in the difference so as to dispose of the oldest items 
			// when inserting into new ring buffer where the capacity is smaller
			// than the current number of items; if the capacity is larger or equal, 
			// than this value will be zero.
			const u32 diff = mNumItems - minSize;

			// insert newest items into new ring buffer
			for( u32 i = 0; i < minSize; ++i )
				newRb.fPut( (*this)[i + diff] );

			fSwap( newRb );
		}

		///
		/// \brief Add an object to the head, FIFO-style.
		inline void fPut( const t& object )
		{
			if( mHead == mTail && mNumItems > 0 )
				mTail = fNextIndex( mTail );
			tBase::operator[]( mHead ) = object;
			mHead = fNextIndex( mHead );
			mNumItems = fMin<u32>( mNumItems + 1, fCapacity( ) );
		}

		///
		/// \brief Attempt to get the object at the tail, FIFO-style.
		/// \return true if there were items in the ring buffer and the
		/// tail object was successfully returned, otherwise false.
		inline b32 fGet( t& object )
		{
			if( mNumItems > 0 )
			{
				object = tBase::operator[]( mTail );
				mTail = fNextIndex( mTail );
				mNumItems -= 1;
				return true;
			}

			object = t( );
			return false;
		}

		inline b32 fGet( )
		{
			t discardedObject;
			return fGet( discardedObject );
		}

		///
		/// \brief Attempt to get the object at the head, LIFO-style.
		/// \return true if there were items in the ring buffer and the
		/// tail object was successfully returned, otherwise false.
		inline b32 fPopLIFO( t& object )
		{
			if( mNumItems > 0 )
			{
				const u32 newHead = fPrevIndex( mHead );
				object = tBase::operator[]( newHead );
				mHead = newHead;
				mNumItems -= 1;
				return true;
			}

			object = t( );
			return false;
		}

		///
		/// \brief Replicates 'object' throughout the entire ring buffer,
		/// up to its capacity. Following this call, number of items is
		/// guaranteed to be equal to capacity.
		inline void fFill( const t& object )
		{
			mHead = 0;
			mTail = 0;
			mNumItems = fCapacity( );
			for( u32 i = 0; i < mNumItems; ++i )
				tBase::operator[]( i ) = object;
		}

		///
		/// \brief Reset the ring buffer
		inline void fReset( )
		{
			tCopier<t>::fAssignToDestroy( tBase::fBegin( ), t( ), tBase::fCount( ) );

			mHead = 0;
			mTail = 0;
			mNumItems = 0;
		}


		///
		/// \brief Finds and erases an object, not ordered
		inline b32 fErase( const t& item )
		{
			u32 index = fIndexOf( item );
			if( index != ~0 )
			{
				tBase::mItems[ index ] = fFront( );
				--mNumItems;

				if( !mNumItems )
				{
					mHead = 0;
					mTail = 0;
				}
				else if( index == mHead )
					mHead = fNextIndex( mHead );
				else
					mTail = fPrevIndex( mTail );

				return true;
			}

			return false;
		}

		inline t* fFind( const t& item )
		{
			return tBase::fFind( item );
		}

		inline u32 fIndexOf( const t& item )
		{
			for( u32 i = 0; i < mNumItems; ++i )
				if( operator[]( i ) == item )
					return i;
			return ~0;
		}

	};

}

#endif//__tRingBuffer__
