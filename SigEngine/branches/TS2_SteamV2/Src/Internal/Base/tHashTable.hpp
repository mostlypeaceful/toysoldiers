#ifndef __tHashTable__
#define __tHashTable__

namespace Sig
{
	namespace Hash
	{
		base_export u32 fGenericHash( const Sig::byte* data, const u32 numBytes, const u32 maxHash );
	}

	///
	/// \brief Default hash function; this version provides an arbitrary length
	/// hash function.
	/// \note This type is specialized for many common types in ways that produce
	/// either more optimal or better hashes. You are free to specialize this class
	/// for your own types.
	template<class tKey>
	class tHash
	{
	public:
		inline u32 operator( )( const tKey& key, const u32 maxSize ) const
		{
			return Hash::fGenericHash( ( Sig::byte* )&key, sizeof( key ), maxSize );
		}
	};

	///
	/// \brief Specialization for unsigned int types.
	template<>
	class base_export tHash<u32>
	{
	public:
		inline u32 operator( )( u32 key, const u32 maxSize ) const
		{
			// apparently this is used by java HashMap or something? seems good and fast.
			key ^= (key >> 20) ^ (key >> 12);
			return ( key ^ (key >> 7) ^ (key >> 4) ) % maxSize;
		}
	};

	///
	/// \brief Specialization for signed int types.
	template<>
	class base_export tHash<s32> : public tHash<u32>
	{
	public:
		inline u32 operator( )( s32 key, const u32 maxSize ) const
		{
			return tHash<u32>::operator ()( ( u32 )key, maxSize );
		}
	};

	///
	/// \brief Specialization for unsigned 64-bit int types.
	template<>
	class base_export tHash<u64>
	{
	public:
		inline u32 operator( )( u64 key, const u32 maxSize ) const
		{
			key = (~key) + (key << 18); // key = (key << 18) - key - 1;
			key = key ^ (key >> 31);
			key = key * 21; // key = (key + (key << 2)) + (key << 4);
			key = key ^ (key >> 11);
			key = key + (key << 6);
			key = key ^ (key >> 22);
			return (u32)key % maxSize;
		}
	};

	///
	/// \brief Specialization for signed 64-bit int types.
	template<>
	class base_export tHash<s64> : public tHash<u64>
	{
	public:
		inline u32 operator( )( s64 key, const u32 maxSize ) const
		{
			return tHash<u64>::operator ()( ( u64 )key, maxSize );
		}
	};

	// FIXME detect 64-bit pointer platforms
	typedef u32 tHashTablePtrInt;

	///
	/// \brief Specialization for c-style string types.
	template<>
	class base_export tHash<const char*>
	{
	public:
		inline u32 operator( )( const char* key, const u32 maxSize ) const
		{
			return Hash::fGenericHash( ( Sig::byte* )key, ( u32 )strlen( key ), maxSize );
		}
	};

	///
	/// \brief Specialization for std-style string types.
	template<>
	class base_export tHash<std::string>
	{
	public:
		inline u32 operator( )( const std::string& key, const u32 maxSize ) const
		{
			return tHash<const char*>( )( key.c_str( ), maxSize );
		}
	};

	///
	/// \brief Specialization for std-style string types. Ignoreing case
	class base_export tHashStringICase
	{
	public:
		inline u32 operator( )( const std::string& key, const u32 maxSize ) const
		{
			std::string val = key;
			std::transform(val.begin(), val.end(), val.begin(), ::tolower);
			return tHash<const char*>( )( val.c_str( ), maxSize );
		}
	};

	///
	/// \brief Partial specialization for all pointer types; uses the normal integer hash.
	/// While pointer types are unique, they don't necessarily form a good distribution.
	template<class tKey>
	class base_export tHash<tKey*> : public tHash<tHashTablePtrInt>
	{
	public:
		inline u32 operator( )( const tKey* key, const u32 maxSize ) const
		{
			return tHash<tHashTablePtrInt>::operator ()( ( tHashTablePtrInt )( size_t )key, maxSize );
		}
	};


	/// Forward declare the default hash table resize policy.
	class tHashTableExpandAndShrinkResizePolicy;

	///
	/// \brief Provides full templatized hash table functionality, using linear probing. Resizing is
	/// possible and the default behavior (see the resize policy parameter), though you can also have
	/// a hash table with no resizing (and zero overhead associated with resizing).
	/// The most important template parameters of the hash table are the key type and the value type,
	/// which are also the most self-explanatory.
	/// The tResizePolicy parameter allows you to specify the resizing behavior of the
	/// hash table using compile time code assemblage (templates). This way, for those that don't
	/// want resizing functionality, there's zero overhead. Additionally, you can implement your
	/// own policy if the default provided ones don't work for you.
	/// The last parameters are the key hash functor and the key compare functor. You can look
	/// at the default class parameters for examples of how to implement your own (if desired).
	template< 
		class tKey, 
		class tValue, 
		class tResizePolicy = tHashTableExpandAndShrinkResizePolicy,
		class tKeyHash		= tHash<tKey>, 
		class tKeyEqual		= tEqual<tKey> 
	>
	class tHashTable : public tResizePolicy, public tKeyHash, public tKeyEqual
	{
	public:

		///
		/// \brief Hash table entry-type is templatized to allow for partial
		/// specialization, so that hash tables with pointer values can avoid
		/// using extra storage for "null" and "removed" state variables.
		template<class tEntryKey, class tEntryValue>
		struct tEntry
		{
			mutable tEntryValue		mValue;
			tEntryKey				mKey;
			u8						mState;

			enum { cNull, cRemoved, cOccupied };

			inline tEntry( ) 
				: mValue( tEntryValue( ) ), mKey( tEntryKey( ) ), mState( cNull )
			{ } 
			inline tEntry( const tEntryKey& k, const tEntryValue& v )
				: mValue(v), mKey(k), mState( cOccupied )
			{ } 

			inline b32 fNull( ) const			{ return mState == cNull; }
			inline b32 fRemoved( ) const		{ return mState == cRemoved; }
			inline b32 fNullOrRemoved( ) const  { return mState == cNull || mState == cRemoved; }
			inline void fSetRemoved( )			{ mState = cRemoved; mKey = tEntryKey( ); mValue = tEntryValue( ); }
		};

		///
		/// \brief Partial specialization for pointer values; instead of
		/// extra variables to store "null" and "removed" state, we instead
		/// assume certain invalid pointer values to track whether the
		/// entry is null or removed.
		template<class tEntryKey, class tEntryValue>
		struct tEntry<tEntryKey, tEntryValue*>
		{
		private:

			static inline tEntryValue* fNullSentinel( ) { return ( tEntryValue* )( size_t )0xffffffff; }
			static inline tEntryValue* fRemovedSentinel( ) { return ( tEntryValue* )( size_t )0xfffffffe; }

		public:

			mutable tEntryValue*	mValue; 
			tEntryKey				mKey;

			inline tEntry( ) 
				: mValue( fNullSentinel( ) ), mKey( tEntryKey( ) )
			{ } 
			inline tEntry( const tEntryKey& k, tEntryValue* v )
				: mValue(v), mKey(k)
			{ } 

			inline b32 fNull( ) const			{ return mValue == fNullSentinel( ); }
			inline b32 fRemoved( ) const		{ return mValue == fRemovedSentinel( ); }
			inline b32 fNullOrRemoved( ) const  { return mValue == fNullSentinel( ) || mValue == fRemovedSentinel( ); }
			inline void fSetRemoved( )			{ mValue = fRemovedSentinel( ); mKey = tEntryKey( ); }
		};

		typedef tEntry<tKey, tValue>						tMyEntry;
		typedef tDynamicArray<tMyEntry>						tEntryList;
		typedef typename tEntryList::tIterator				tIterator;
		typedef typename tEntryList::tConstIterator			tConstIterator;

		template< typename T >
		class tIterNoNullOrRemoved
		{
		public:
			tIterNoNullOrRemoved( const T& begin, const T& end ) : mIt( begin ), mEnd( end ) { while( mIt != mEnd && mIt->fNullOrRemoved( ) ) { ++mIt; } } //iterate to first non null or removed
			tIterNoNullOrRemoved& operator ++ ( ) { while( ++mIt != mEnd && mIt->fNullOrRemoved( ) ) {  } return *this; } //increment until non null or removed

			T operator -> ( ) const { return mIt; }
			operator T ( ) const { return mIt; }
			
		private:
			T mIt;
			T mEnd;
		};
		typedef tIterNoNullOrRemoved< tIterator >			tIteratorNoNullOrRemoved;
		typedef tIterNoNullOrRemoved< tConstIterator >		tConstIteratorNoNullOrRemoved;

	private:

		u32			mItemCount;
		tEntryList	mEntries;

	public:

		///
		/// \brief Ctor, initializes hash table with zero entries; does not allocate any memory.
		inline tHashTable( )
			: mItemCount( 0 )
		{
		}

		///
		/// \brief Ctor, intializes the hash table with specified capacity and zero item count.
		inline explicit tHashTable( u32 startCapacity )
			: mItemCount( 0 ), mEntries( startCapacity )
		{
		}

		///
		/// \brief Dtor, clears hash table and deallocates all memory.
		inline ~tHashTable( ) 
		{
			fClear( );
		}

		inline u32				fGetItemCount( ) const	{ return mItemCount; }
		inline u32				fGetCapacity( ) const	{ return mEntries.fCount( ); }
		inline tIterator		fBegin( )				{ return mEntries.fBegin( ); }
		inline tConstIterator	fBegin( ) const			{ return mEntries.fBegin( ); }
		inline tIterator		fEnd( )					{ return mEntries.fEnd( ); }
		inline tConstIterator	fEnd( ) const			{ return mEntries.fEnd( ); }
		inline void				fClear( u32 newCapacity = 0 ) { mEntries = tEntryList( newCapacity ); mItemCount = 0; }

		///
		/// \brief Efficiently swaps the contents of the two hash tables; no allocations/deallocations.
		void fSwap( tHashTable& ht )
		{
			tResizePolicy::fSwap( ht );
			Sig::fSwap( mItemCount, ht.mItemCount );
			mEntries.fSwap( ht.mEntries );
		}

		///
		/// \brief Set the capacity of the hash table. If the new capacity is
		/// greater than or equal to the existing capacity, all the previous entries
		/// will be re-inserted; if the new capacity is less, than only the min
		/// of the existing item count and the new capacity will get re-inserted.
		void fSetCapacity( u32 numEntries )
		{
			tHashTable newHt( numEntries );

			const u32 insertCount = fMin( mItemCount, numEntries-1 );

			u32 inserted = 0;
			for( tIterator i = fBegin( ); inserted < insertCount && i != fEnd( ); ++i )
			{
				if( i->fNullOrRemoved( ) )
					continue;

				newHt.fInsert( i->mKey, i->mValue );
				++inserted;
			}

			fSwap( newHt );
		}

		///
		/// \brief Insert a new entry into the hash table.
		/// \note The hash table must preserve at least one empty slot.
		/// Therefore, you must set your capacity to be (at least) one greater
		/// than the actual number of items you intend to insert. This method
		/// will therefore sigassert if you try to insert past the allowed maximum.
		tValue* fInsert( const tKey& key, const tValue& value )
		{
			// notify the resize policy that we're about to insert
			tResizePolicy::fPreInsert( *this );

			sigassert( mItemCount+1 < mEntries.fCount( ) );

			// hash the key value for an initial index
			u32 index = tKeyHash::operator( )( key, mEntries.fCount( ) );

			// use linear probing to find the first available slot
			if_assert( u32 probes = 0; )
			while( 
				!mEntries[ index ].fNull( ) &&
				!mEntries[ index ].fRemoved( ) )
			{
				index = ( index + 1 ) % mEntries.fCount( );
				sigassert( probes++ < mEntries.fCount( ) );
			}

			// insert the item
			tMyEntry& myEntry = mEntries[ index ];

			// let the resize policy know that we might be over-writing a removed item
			tResizePolicy::fOnInsert( *this, myEntry.fRemoved( ) );

			// assign the new (key,value)
			myEntry = tMyEntry( key, value );

			// increment item count
			++mItemCount;

			// return the address of the item
			return &myEntry.mValue;
		}

		///
		/// \brief Find an entry in the hash table.
		/// \return Null if the entry is not found, otherwise a pointer to the value.
		/// \note The return value (if non-null) can be used as an argument to fRemove.
		/// This is more optimal than calling fFind with a key, and then fRemove with a
		/// key, as the hash table will have to do two searches.
		tValue* fFind( const tKey& key ) const
		{
			if( mEntries.fCount( ) > 0 )
			{
				// hash the key value for an initial index
				u32 index = tKeyHash::operator( )( key, mEntries.fCount( ) );

				// use linear probing to find the first slot matching the specified key
				if_assert( u32 probes = 0; )
				while( !mEntries[ index ].fNull( ) )
				{
					if( !mEntries[ index ].fRemoved( ) && 
						tKeyEqual::operator( )( mEntries[ index ].mKey, key ) )
					{
						return &mEntries[ index ].mValue;
					}

					index = ( index + 1 ) % mEntries.fCount( );
					sigassert( probes++ < mEntries.fCount( ) );
				}
			}

			// couldn't find it
			return 0;
		}

		///
		/// \brief Remove an entry by key.
		b32 fRemove( const tKey& key )
		{
			// look for the entry by key
			tValue* find = fFind( key );
			if( !find )
				return false;

			// found the entry, remove it
			fRemove( find );
			return true;
		}

		///
		/// \brief Remove an entry using the value that was returned by fFind.
		/// \note This version of fRemove is more optimal than the version that
		/// removes by key, so prefer this one if you've already done a find.
		void fRemove( tValue* addressReturnedByFind )
		{
			// decrement item count
			--mItemCount;

			// a bit of trickery, but it's okay... the value is the first member
			// of the entry struct, so it's sure to be the same address
			tMyEntry* entry = ( tMyEntry* )addressReturnedByFind;

			// mark the entry as removed
			entry->fSetRemoved( );

			// notify resize policy that we removed something
			tResizePolicy::fPostRemove( *this );
		}

		///
		/// \brief For the lazy, we provide direct array-style indexing.
		tValue& operator[]( const tKey& key )
		{
			tValue* find = fFind( key );
			if( !find )
				find = fInsert( key, tValue() );
			sigassert( find );
			return *find;
		}
	};

	///
	/// \brief Hash table resizing policy; this one will neither grow nor shrink as items
	/// are inserted and removed.
	class base_export tHashTableNoResizePolicy
	{
	public:

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		void fSwap( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht )
		{
		}

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		inline void fPreInsert( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht )
		{
		}

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		inline void fOnInsert( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht, b32 wasRemoved )
		{
		}

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		inline void fPostRemove( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht )
		{
		}
	};

	///
	/// \brief Hash table resizing policy; this one will only expand as items are added to the hash table;
	/// it will not shrink as items are removed from the hash table.
	class base_export tHashTableExpandOnlyResizePolicy : public tHashTableNoResizePolicy
	{
	public:

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		inline void fPreInsert( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht )
		{
			const f32 density = ( ht.fGetItemCount( ) + 2.f ) / ( ht.fGetCapacity( ) + 1.f );
			if( density > 0.6f )
				ht.fSetCapacity( fMax<u32>( 10, ht.fGetCapacity( ) * 2 ) );
		}

	};

	///
	/// \brief Hash table resizing policy; this one will both expand as items are added to the hash table,
	/// as well as shrink as items are removed from the hash table.
	class base_export tHashTableExpandAndShrinkResizePolicy
	{
		u32 mRemovedCount;

	public:

		tHashTableExpandAndShrinkResizePolicy( )
			: mRemovedCount( 0 )
		{
		}

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		inline void fSwap( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht )
		{
			Sig::fSwap( mRemovedCount, ht.mRemovedCount );
		}

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		inline void fPreInsert( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht )
		{
			const f32 density = ( ht.fGetItemCount( ) + mRemovedCount + 2.f ) / ( ht.fGetCapacity( ) + 1.f );
			if( density > 0.6f )
				ht.fSetCapacity( fMax<u32>( 10, ht.fGetCapacity( ) * 2 ) );
		}

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		inline void fOnInsert( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht, b32 wasRemoved )
		{
			if( wasRemoved )
			{
				sigassert( mRemovedCount > 0 );
				--mRemovedCount;
			}
		}

		template< class tKey, class tValue, class tResizePolicy, class tKeyHash, class tKeyEqual >
		inline void fPostRemove( tHashTable<tKey,tValue,tResizePolicy,tKeyHash,tKeyEqual>& ht )
		{
			++mRemovedCount;

			const f32 density = ( ht.fGetItemCount( ) + 1.f ) / ( ht.fGetCapacity( ) + 1.f );
			if( density < 0.25f )
				ht.fSetCapacity( fMax<u32>( 10, ht.fGetItemCount( ) * 3 ) );
		}

	};

}


#endif//__tHashTable__
