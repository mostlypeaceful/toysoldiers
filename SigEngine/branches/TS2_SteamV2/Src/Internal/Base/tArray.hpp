#ifndef __tArray__
#define __tArray__

namespace Sig
{
	///
	/// \brief Copier class provides compile-time identification of built-in versus non-built-in
	/// types; in the case of built-in types, an optimized code-path is selected (again, at compile-time,
	/// using templates) to do the copying. This generic template provides the non-optimized (but
	/// safe for all object-types) code-path.
	template<class t, bool isBuiltInType = ( tIsBuiltInType<t>::cIs || tIsPointer<t>::cIs )>
	class tCopier
	{
	public:

		static void inline fCopy( t* dst, const t* src, u32 numItems )
		{
			for( u32 i = 0; i < numItems; ++i )
				dst[i] = src[i];
		}

		static void inline fCopyOverlapped( t* dst, const t* src, u32 numItems )
		{
			fCopy( dst, src, numItems );
		}

		static void inline fAssign( t* dst, const t& object, u32 numItems )
		{
			for( u32 i = 0; i < numItems; ++i )
				dst[i] = object;
		}

		static void inline fAssignToDestroy( t* dst, const t& object, u32 numItems )
		{
			fAssign( dst, object, numItems );
		}
	};

	///
	/// \brief This is the template specialization providing the optimized code path for built-in types.
	/// The reason this code-path isn't chosen for non-built in types is because they may
	/// have non-trivial constructurs/destructors/assignment operators.
	template<class t>
	class tCopier<t,true>
	{
	public:

		static void inline fCopy( t* dst, const t* src, u32 numItems )
		{
			fMemCpy( dst, src, numItems * sizeof(t) );
		}

		static void inline fCopyOverlapped( t* dst, const t* src, u32 numItems )
		{
			fMemMove( dst, src, numItems * sizeof(t) );
		}

		static void inline fAssign( t* dst, const t& object, u32 numItems )
		{
			for( u32 i = 0; i < numItems; ++i )
				dst[i] = object;
		}

		static void inline fAssignToDestroy( t* dst, const t& object, u32 numItems )
		{
			// nothing to do, no destructors need to be called
		}
	};

	///
	/// \brief Provides fixed sized array functionality, with some handy stuff on top,
	/// like bounds checking, a count member, reflection, etc.
	template<class t, int N>
	class tFixedArray
	{
		declare_reflector( );
	private:
		t mItems[N];
	public:

		static const u32 cDimension = N;

		typedef t*			tIterator;
		typedef const t*	tConstIterator;

		inline tFixedArray( ) 
			{ }
		inline tFixedArray( tNoOpTag ) 
			{ }

		inline u32			fCount( ) const						{ return N; }
		inline t*			fBegin( )							{ return mItems; }
		inline const t*		fBegin( ) const						{ return mItems; }
		inline t*			fEnd( )								{ return mItems + N; }
		inline const t*		fEnd( ) const						{ return mItems + N; }
		inline t&			fFront( )							{ return *mItems; }
		inline const t&		fFront( ) const						{ return *mItems; }
		inline t&			fBack( )							{ return *(mItems + N - 1); }
		inline const t&		fBack( ) const						{ return *(mItems + N - 1); }
		inline t&			operator[]( const u32 i )			{ sigassert( i < N ); return mItems[i]; }
		inline const t&		operator[]( const u32 i ) const		{ sigassert( i < N ); return mItems[i]; }

		inline void fFill( const t& object )
		{
			tCopier<t>::fAssign( fBegin( ), object, fCount( ) );
		}

		inline void fZeroOut( )
		{
			::Sig::fZeroOut( fBegin( ), fCount( ) );
		}

		inline u32 fElementSizeOf( ) const
		{
			return sizeof( t );
		}

		inline u32 fTotalSizeOf( ) const
		{
			return sizeof( t ) * N;
		}

		template<class u>
		s32 fIndexOf( const u& object ) const
		{
			for( u32 i = 0; i < fCount( ); ++i )
				if( mItems[i] == object )
					return i;
			return -1;
		}

		template<class u>
		const t* fFind( const u& object ) const
		{
			for( u32 i = 0; i < fCount( ); ++i )
				if( mItems[i] == object )
					return &mItems[i];
			return 0;
		}

		template<class u>
		t* fFind( const u& object )
		{
			for( u32 i = 0; i < fCount( ); ++i )
				if( mItems[i] == object )
					return &mItems[i];
			return 0;
		}
	};


	///
	/// \brief Provides handy array functionality by wrapping a pointer and a count.
	/// \note Does not consider itself the owner of the memory provided in the array 
	/// pointer; hence, it is your responsibility to delete any memory your object
	/// may be pointing to before it is re-assigned or otherwise lost (you can, however,
	/// simplify this task by using the fNewArray and fDeleteArray methods).
	template<class t>
	class tArraySleeve
	{
		declare_reflector( );
	protected:
		t*		mItems;		dynamic_array_count(mCount)
		u32		mCount;
	public:
		typedef t*			tIterator;
		typedef const t*	tConstIterator;

		inline tArraySleeve( ) 
			: mItems( 0 ), mCount( 0 ) { }
		inline tArraySleeve( tNoOpTag ) 
			{ }
		inline tArraySleeve( t* items, u32 count ) 
			: mItems( items ), mCount( count ) { }

		inline void			fSet( t* items, u32 count ) 		{ mItems = items; mCount = count; }
		inline void			fClear( )							{ mItems = 0; mCount = 0; }
		inline u32			fCount( ) const						{ return mCount; }
		inline t*			fBegin( )							{ return mItems; }
		inline const t*		fBegin( ) const						{ return mItems; }
		inline t*			fEnd( )								{ return mItems + mCount; }
		inline const t*		fEnd( ) const						{ return mItems + mCount; }
		inline t&			fFront( )							{ return mItems[0]; }
		inline const t&		fFront( ) const						{ return mItems[0]; }
		inline t&			fBack( )							{ return mItems[mCount - 1]; }
		inline const t&		fBack( ) const						{ return mItems[mCount - 1]; }
		inline b32			fNull( ) const						{ return !mItems; }
		inline void			fNewArray( u32 count )				{ if( count > 0 ) { mItems = NEW_TYPED( t ) t[count]; sigassert( mItems && "Out of memory?" ); } else { mItems = 0; } mCount = count; }
		inline void			fDeleteArray( )						{ delete [] mItems; mItems = 0; mCount = 0; }
		inline void			fSwap( tArraySleeve& other )		{ Sig::fSwap( mItems, other.mItems ); Sig::fSwap( mCount, other.mCount ); }
		inline void			fDisown( tArraySleeve & newOwner )	{ sigassert( newOwner.mItems == NULL ); newOwner.mItems = mItems; newOwner.mCount = mCount; mItems = 0; mCount = 0; }
		inline void			fDisown( )							{ fClear( ); }
		inline t&			operator[]( u32 i )					{ sigassert( i < mCount ); return mItems[i]; }
		inline const t&		operator[]( u32 i ) const			{ sigassert( i < mCount ); return mItems[i]; }
		inline t&			fIndex( const u32 i )				{ sigassert( i < mCount ); return mItems[i]; }
		inline const t&		fIndex( const u32 i ) const			{ sigassert( i < mCount ); return mItems[i]; }

		inline void fFill( const t& object )
		{
			tCopier<t>::fAssign( fBegin( ), object, fCount( ) );
		}

		inline void fZeroOut( )
		{
			::Sig::fZeroOut( fBegin( ), fCount( ) );
		}

		inline u32 fElementSizeOf( ) const
		{
			return sizeof( t );
		}

		inline u32 fTotalSizeOf( ) const
		{
			return sizeof( t ) * fCount( );
		}

		///
		/// \brief You probably shouldn't be using this method. Only useful (or safe) for
		/// relocatin of load-in-place resources. Any other use will cause memory leaks
		/// and general memory corruption.
		inline void fRelocateInPlace( ptrdiff_t delta )
		{
			mItems = ( t* )( ( Sig::byte* )mItems + delta );
		}
	};

	template<class t>
	class tGrowableArray;

	///
	/// \brief Wraps tArraySleeve, providing similar functionality, but with ownership semantics over
	/// the memory it points to.
	///
	/// tDynamicArray will deallocate memory on destruction and on assignment, providing full first-class
	/// ADT operations. tDynamicArray is also safe to use as a load in place structure; i.e., it can be used
	/// tools side, dynamically allocating memory, as well as used "game" side as a load in place buffer.
	template<class t>
	class tDynamicArray : public tArraySleeve<t>
	{
		declare_reflector( );
	public:

		typedef tArraySleeve<t> tBase;

		inline tDynamicArray( ) 
			{ }
		inline tDynamicArray( const tDynamicArray& other ) 
			{ fCopy( other ); }
		inline tDynamicArray( tNoOpTag ) 
			: tBase( cNoOpTag ) { }
		inline explicit tDynamicArray( u32 count )		
			{ fNewArray( count ); }
		inline ~tDynamicArray( )				
			{ tBase::fDeleteArray( ); }

		tDynamicArray& operator=( const tDynamicArray& other )
		{
			if( this != &other )
			{
				tBase::fDeleteArray( );
				fCopy( other );
			}
			return *this;
		}

		tDynamicArray& operator=( const tGrowableArray<t>& other );

		inline void fFill( const t& object )
		{
			tCopier<t>::fAssign( tBase::fBegin( ), object, tBase::fCount( ) );
		}

		inline void fZeroOut( )
		{
			::Sig::fZeroOut( tBase::fBegin( ), tBase::fCount( ) );
		}

		inline u32 fElementSizeOf( ) const
		{
			return sizeof( t );
		}

		inline u32 fTotalSizeOf( ) const
		{
			return sizeof( t ) * tBase::fCount( );
		}

		inline void fSwap( tDynamicArray& other )
		{
			tBase::fSwap( other );
		}

		inline void	fNewArray( u32 count )
		{
			tBase::fDeleteArray( ); tBase::fNewArray( count );
		}

		///
		/// \brief Create a new underlying array by copying data from the
		/// supplied null terminated array; this is useful for storing strings.
		void fCreateNullTerminated( const t* nullTerminatedArray )
		{
			const u32 len = fNullTerminatedLength( nullTerminatedArray );
			tBase::fDeleteArray( );
			fNewArray( len+1 );
			tCopier<t>::fCopy( tBase::mItems, nullTerminatedArray, tBase::mCount );
		}

		///
		/// \brief Adds one element to the array, increasing the allocated size.
		void fPushBack( const t& object )
		{
			tDynamicArray temp( tBase::fCount( ) + 1 );
			tCopier<t>::fCopy( temp.fBegin( ), tBase::fBegin( ), tBase::mCount );
			temp[ temp.fCount( ) - 1 ] = object;
			fSwap( temp );
		}

		void fResize( u32 newCount )
		{
			tDynamicArray temp( newCount );
			tCopier<t>::fCopy( temp.fBegin( ), tBase::fBegin( ), fMin( tBase::mCount, newCount ) );
			fSwap( temp );
		}

		void fInsert( u32 index, const t* objects, u32 numItems=1 )
		{
			const u32 newCount = tBase::mCount + numItems;
			
			tDynamicArray<t> newArray( newCount );

			// first copy in old items to new array
			tCopier<t>::fCopy( newArray.fBegin( ), tBase::mItems, index );

			// now copy supplied items into new array starting from supplied index
			tCopier<t>::fCopy( newArray.fBegin( ) + index, objects, numItems );

			// now copy the rest of the old items to the end of the new array
			tCopier<t>::fCopy( newArray.fBegin( ) + index + numItems, tBase::mItems + index, tBase::mCount - index );

			fSwap( newArray );
		}

		void fInitialize( const t * objects, u32 numItems=1 )
		{
			tDynamicArray<t> newArray( numItems );

			tCopier<t>::fCopy( newArray.fBegin( ), objects, numItems );

			fSwap( newArray );
		}

		template<class u>
		s32 fIndexOf( const u& object ) const
		{
			for( u32 i = 0; i < tBase::mCount; ++i )
				if( tBase::mItems[i] == object )
					return i;
			return -1;
		}

		template<class u>
		const t* fFind( const u& object ) const
		{
			for( u32 i = 0; i < tBase::mCount; ++i )
				if( tBase::mItems[i] == object )
					return &tBase::mItems[i];
			return 0;
		}

		template<class u>
		t* fFind( const u& object )
		{
			for( u32 i = 0; i < tBase::mCount; ++i )
				if( tBase::mItems[i] == object )
					return &tBase::mItems[i];
			return 0;
		}

		void fJoin( const tDynamicArray& other )
		{
			fInsert( tBase::fCount( ), other.fBegin( ), other.fCount( ) );
		}

	private:

		inline void	fSet( t* items, u32 count ) 		{ tBase::fSet( items, count ); }
		inline void	fClear( )							{ tBase::fClear( ); }

		void fCopy( const tDynamicArray& other )
		{
			// IMPORTANT! It is the responsibility of the caller to
			// first deallocate any existing data (using fDeleteArray) if necessary
			fNewArray( other.mCount );
			tCopier<t>::fCopy( tBase::mItems, other.fBegin( ), tBase::mCount );
		}
	};

	typedef tDynamicArray< byte > tDynamicBuffer;

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

		inline u32			fCount( ) const						{ return mCount; }
		inline u32			fCapacity( ) const					{ return mItems.fCount( ); }
		inline t*			fBegin( )							{ return mItems.fBegin( ); }
		inline const t*		fBegin( ) const						{ return mItems.fBegin( ); }
		inline t*			fEnd( )								{ return mItems.fBegin( ) + mCount; }
		inline const t*		fEnd( ) const						{ return mItems.fBegin( ) + mCount; }
		inline t&			fFront( )							{ return mItems.fFront( ); }
		inline const t&		fFront( ) const						{ return mItems.fFront( ); }
		inline t&			fBack( )							{ return mItems[mCount - 1]; }
		inline const t&		fBack( ) const						{ return mItems[mCount - 1]; }
		inline t&			operator[]( const u32 i )			{ sigassert( i < mCount ); return mItems[i]; }
		inline const t&		operator[]( const u32 i ) const		{ sigassert( i < mCount ); return mItems[i]; }
		inline t&			fIndex( const u32 i )				{ sigassert( i < mCount ); return mItems[i]; }
		inline const t&		fIndex( const u32 i ) const			{ sigassert( i < mCount ); return mItems[i]; }

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
			if( count <= mCount )
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
				sigassert( capacity >= count );
				tDynamicArray<t> newArray( capacity );
				tCopier<t>::fCopy( newArray.fBegin( ), mItems.fBegin( ), mCount );
				mItems.fSwap( newArray );
			}

			mCount = count;
		}

		void fInsertSafe( u32 index, const t& object, u32 numItems=1 )
		{
			if( index > fCount( ) ) index = fCount( );
			fInsert( index, object, numItems );
		}

		void fInsert( u32 index, const t& object, u32 numItems=1 )
		{
			const u32 count = mCount + numItems;
			
			const u32 capacity = fComputeIncreasedCapacity( count );
			tDynamicArray<t> newArray( capacity );

			// first copy in old items to new array
			tCopier<t>::fCopy( newArray.fBegin( ), mItems.fBegin( ), index );

			// now copy 'numItems' many copies of the new desired object starting at the supplied index
			tCopier<t>::fAssign( newArray.fBegin( ) + index, object, numItems );

			// now copy the rest of the old items to the end of the new array
			tCopier<t>::fCopy( newArray.fBegin( ) + index + numItems, mItems.fBegin( ) + index, mCount - index );

			mItems.fSwap( newArray );
			mCount = count;
		}

		void fInsert( u32 index, const t* objects, u32 numItems=1 )
		{
			const u32 count = mCount + numItems;
			
			const u32 capacity = fComputeIncreasedCapacity( count );
			tDynamicArray<t> newArray( capacity );

			// first copy in old items to new array
			tCopier<t>::fCopy( newArray.fBegin( ), mItems.fBegin( ), index );

			// now copy supplied items into new array starting from supplied index
			tCopier<t>::fCopy( newArray.fBegin( ) + index, objects, numItems );

			// now copy the rest of the old items to the end of the new array
			tCopier<t>::fCopy( newArray.fBegin( ) + index + numItems, mItems.fBegin( ) + index, mCount - index );

			mItems.fSwap( newArray );
			mCount = count;
		}

		inline void fJoin( const tGrowableArray& other )
		{
			fInsert( fCount( ), other.fBegin( ), other.fCount( ) );
		}

		inline void fErase( u32 index )
		{
			sigassert( index < mCount );
			mItems[index] = mItems[mCount-1];
			mItems[mCount-1] = t( );
			mCount -= 1;
		}

		void fEraseOrdered( u32 index )
		{
			sigassert( index < mCount );

			tCopier<t>::fCopyOverlapped( 
				mItems.fBegin( ) + index, 
				mItems.fBegin( ) + index + 1,
				mCount - index - 1 );

			mItems[mCount-1] = t( );
			mCount -= 1;
		}

		template<class u>
		s32 fIndexOf( const u& object ) const
		{
			for( u32 i = 0; i < mCount; ++i )
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

		inline void fPushBack( const t& object )
		{
			fGrowCount( 1 );
			fBack( ) = object;
		}

		inline void fPushBackNoGrow( const t& object )
		{
			++mCount;
			sigassert( mCount <= mItems.fCount( ) );
			fBack( ) = object;
		}

		inline t fPopBack( )
		{
			sigassert( mCount > 0 );
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
			sigassert( mCount > 0 );
			t o = fFront( );
			fEraseOrdered( 0 );
			return o;
		}
	};

	typedef tGrowableArray< byte > tGrowableBuffer;


	template<class t>
	tDynamicArray<t>& tDynamicArray<t>::operator=( const tGrowableArray<t>& other )
	{
		fNewArray( other.fCount( ) );
		tCopier<t>::fCopy( tBase::fBegin( ), other.fBegin( ), tBase::fCount( ) );
		return *this;
	}

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
			sigassert( intialCount <= cDimension );
		}

		inline u32			fCount( ) const						{ return mUsedCount; }
		inline t&			operator[]( const u32 i )			{ sigassert( i < mUsedCount ); return mItems[i]; }
		inline const t&		operator[]( const u32 i ) const		{ sigassert( i < mUsedCount ); return mItems[i]; }

		inline void			fPushBack( const t& item )			{ sigassert( mUsedCount < cDimension ); mItems[ mUsedCount ] = item; ++mUsedCount; }

		inline t&			fBack( )							{ return mItems[mUsedCount - 1]; }
		inline const t&		fBack( ) const						{ return mItems[mUsedCount - 1]; }

		inline void			fSetCount( u32 count )				{ sigassert( count <= cDimension ); mUsedCount = count; }
		inline void			fSetCapacity( u32 count )			{ sigassert( count <= cDimension ); /*no-op*/ }

		template<class u>
		s32 fIndexOf( const u& object ) const
		{
			for( u32 i = 0; i < fCount( ); ++i )
				if( mItems[i] == object )
					return i;
			return -1;
		}

		void fFill( const t& val )
		{
			for( u32 i = 0; i < fCount( ); ++i )
				mItems[i] = val;
		}
	};



	///
	/// \brief Enum wrapper, so that you can control the storage size.
	template<class tRealEnum, class tStorage>
	class tEnum
	{
		declare_reflector( );
	private:
		tStorage mValue;
	public:
		inline tEnum( ) { }
		inline tEnum( tRealEnum value ) : mValue( ( tStorage )value ) { }
		inline operator tRealEnum( ) const { return ( tRealEnum )mValue; }
	};
}

#endif//__tArray__
