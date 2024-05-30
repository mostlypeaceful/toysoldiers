#ifndef __tRingBuffer__
#define __tRingBuffer__

namespace Sig
{
	///
	/// \brief Encapsulates a dynamically-sizeable ring buffer; the ring buffer
	/// will wrap items in an array based on the user-set capacity; it will only 
	/// grow if the user explicitly calls a grow method. Queue-style, FIFO putting
	/// and getting.
	///
	/// MERGE WARNING:
	///
	/// If you're wondering why Mike broke your code, it's because having tRingBuffer::fFront/fBack( ) flipped in the reverse order compared to tDynamicArray::fFront/fBack( ) was fucking stupid.
	/// But since code seems to depend on this (!), rather than simply flipping the names and silently breaking code, he figured it'd be better to go with new names.
	/// He probably should've picked better new names, but at least it's an improvement?
	///
	/// To whoever wants to replace fFirst with fFront at a later date, consider sending out an email to make sure all the programmers are up to date.
	///
	/// Old function    -> Equivalent replacement
	///   fFront( )     -> fLast( )     YES THESE ARE KINDA FLIPPED BETWEEN OLD VS NEW
	///   fBack( )      -> fFirst( )    YES THESE ARE KINDA FLIPPED BETWEEN OLD VS NEW
	///   fPut(...)     -> fPushLast(...)
	///   fGet(...)     -> fTryPopFirst(...)
	///   fPopLIFO(...) -> fTryPopLast(...)
	///
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

		// Reserve the necessary space
		inline void fReserve( u32 needed )
		{
			const s32 toAdd = needed - ( fCapacity( ) - mNumItems );
			if( toAdd > 0 )
				fResize( ( fCapacity( ) + toAdd ) * 2 );
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
		/// \brief /!\ NOTE WELL: Equivalent to old tRingBuffer::fFront( ), and tDynamicArray::fBack( )
		inline t& fLast( )
		{
			return (*this)[ mNumItems - 1 ];
		}

		///
		/// \brief /!\ NOTE WELL: Equivalent to old tRingBuffer::fFront( ), and tDynamicArray::fBack( )
		inline const t& fLast( ) const
		{
			return (*this)[ mNumItems - 1 ];
		}

		///
		/// \brief /!\ NOTE WELL: Equivalent to old tRingBuffer::fBack( ), and tDynamicArray::fFront( )
		inline t& fFirst( )
		{
			return (*this)[ 0 ];
		}

		///
		/// \brief /!\ NOTE WELL: Equivalent to old tRingBuffer::fBack( ), and tDynamicArray::fFront( )
		inline const t& fFirst( ) const
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
				newRb.fPushLast( (*this)[i + diff] );

			fSwap( newRb );
		}

		///
		/// \brief Add an object to the head, FIFO-style.
		inline t& fPushLast( const t& object )
		{
			if( mHead == mTail && mNumItems > 0 ) // If we have no more room:
				mTail = fNextIndex( mTail ); // this is basically fPopFirst

			tBase::operator[]( mHead ) = object;
			mHead = fNextIndex( mHead );
			mNumItems = fMin<u32>( mNumItems + 1, fCapacity( ) );

			return fLast( );
		}

		inline t& fPushLast( )
		{
			fPushLast( t( ) );
			return fLast( );
		}

		inline void fPopFirst( )
		{
			sigcheckfail( mNumItems > 0, return );
			
			tCopier<t>::fAssignToDestroy( &tBase::operator[]( mTail ), t( ), 1 );
			mTail = fNextIndex( mTail );
			mNumItems -= 1;
		}

		inline void fPopFirst( t& object )
		{
			sigcheckfail( mNumItems > 0, return );

			object = tBase::operator[]( mTail );
			fPopFirst( );
		}

		///
		/// \brief Attempt to get the object at the tail, FIFO-style.
		/// \return true if there were items in the ring buffer and the
		/// tail object was successfully returned, otherwise false.
		inline b32 fTryPopFirst( t& object )
		{
			if( mNumItems > 0 )
			{
				fPopFirst( object );
				return true;
			}

			object = t( );
			return false;
		}

		inline b32 fTryPopFirst( )
		{
			t discardedObject;
			return fTryPopFirst( discardedObject );
		}

		inline void fPopLast( t& object )
		{
			sigcheckfail( mNumItems > 0, return );

			const u32 newHead = fPrevIndex( mHead );
			object = tBase::operator[]( newHead );
			tCopier<t>::fAssignToDestroy( &tBase::operator[]( newHead ), t( ), 1 );

			mHead = newHead;
			mNumItems -= 1;
		}

		///
		/// \brief Attempt to get the object at the head, LIFO-style.
		/// \return true if there were items in the ring buffer and the
		/// tail object was successfully returned, otherwise false.
		inline b32 fTryPopLast( t& object )
		{
			if( mNumItems > 0 )
			{
				fPopLast( object );
				return true;
			}

			object = t( );
			return false;
		}

		inline b32 fTryPopLast( )
		{
			t discardedObject;
			return fTryPopFirst( discardedObject );
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
			u32 index = tBase::fIndexOf( item );
			if( index != ~0 )
			{
				tBase::mItems[ index ] = this->fFront( );
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

		inline void fEraseOrdered( u32 index )
		{
			sigcheckfail( index < mNumItems, return );

			for( u32 i = index; i < mNumItems - 1; ++i )
				operator[]( i ) = operator[]( i + 1 );

			tCopier<t>::fAssignToDestroy( &operator[]( mNumItems - 1 ), t( ), 1 );

			--mNumItems;
			mHead = fPrevIndex( mHead );
		}

		template< class u >
		inline t* fFind( const u& item )
		{
			const u32 index = fIndexOf( item );
			return ( index == ~0 ) ? 0 : &operator[]( index );
		}

		template< class u >
		inline const t* fFind( const u& item ) const
		{
			const u32 index = fIndexOf( item );
			return ( index == ~0 ) ? 0 : &operator[]( index );
		}

		template< class u >
		inline u32 fIndexOf( const u& item ) const
		{
			for( u32 i = 0; i < mNumItems; ++i )
				if( operator[]( i ) == item )
					return i;
			return ~0;
		}

		template< class u >
		inline t* fFindLast( const u& item )
		{
			const u32 index = fLastIndexOf( item );
			return ( index == ~0 ) ? 0 : &operator[]( index );
		}

		template< class u >
		inline const t* fFindLast( const u& item ) const
		{
			const u32 index = fLastIndexOf( item );
			return ( index == ~0 ) ? 0 : &operator[]( index );
		}

		template< class u >
		inline u32 fLastIndexOf( const u& item ) const
		{
			for( s32 i = mNumItems - 1; i >= 0; --i )
				if( operator[]( i ) == item )
					return i;
			return ~0;
		}

	};

}

#endif//__tRingBuffer__
