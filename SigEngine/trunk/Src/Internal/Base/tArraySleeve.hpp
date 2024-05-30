#ifndef __tArraySleeve__
#define __tArraySleeve__

namespace Sig
{
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
		inline void			fNewArray( u32 count )				{ if( count > 0 ) { mItems = NEW_TYPED( t )[count]; sigassert( mItems && "Out of memory?" ); } else { mItems = 0; } mCount = count; }
		inline void			fDeleteArray( )						{ delete [] mItems; mItems = 0; mCount = 0; }
		inline void			fSwap( tArraySleeve& other )		{ Sig::fSwap( mItems, other.mItems ); Sig::fSwap( mCount, other.mCount ); }
		inline void			fDisown( tArraySleeve & newOwner )	{ cmp_assert( newOwner.mItems, ==, NULL ); newOwner.mItems = mItems; newOwner.mCount = mCount; mItems = 0; mCount = 0; }
		inline void			fDisown( )							{ fClear( ); }
		inline t&			operator[]( u32 i )					{ cmp_assert( i, <, mCount ); return mItems[i]; }
		inline const t&		operator[]( u32 i ) const			{ cmp_assert( i, <, mCount ); return mItems[i]; }
		inline t&			fIndex( const u32 i )				{ cmp_assert( i, <, mCount ); return mItems[i]; }
		inline const t&		fIndex( const u32 i ) const			{ cmp_assert( i, <, mCount ); return mItems[i]; }

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

		///
		/// \brief You probably shouldn't be using this method. Only useful (or safe) for
		/// relocatin of load-in-place resources. Any other use will cause memory leaks
		/// and general memory corruption.
		inline void fRelocateInPlace( ptrdiff_t delta )
		{
			mItems = ( t* )( ( Sig::byte* )mItems + delta );
		}
	};

	template< typename T, typename U >
	inline b32 operator==( const tArraySleeve<T>& lhs, const tArraySleeve<U>& rhs )
	{
		if( lhs.fCount() != rhs.fCount() )
			return false;
		for( u32 i = 0; i < lhs.fCount(); ++i )
		{
			if( lhs[ i ] != rhs[ i ] )
				return false;
		}
		return true;
	}

	template< typename T, typename U >
	inline b32 operator!=( const tArraySleeve<T>& lhs, const tArraySleeve<U>& rhs )
	{
		return !(lhs == rhs);
	}
}

#endif //ndef __tArraySleeve__
