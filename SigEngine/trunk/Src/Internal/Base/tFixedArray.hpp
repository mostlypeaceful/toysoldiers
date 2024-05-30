#ifndef __tFixedArray__
#define __tFixedArray__

#include "Memory/tRawAlignedStorage.hpp"
#include "Metaprogramming/Conditionals.hpp"
#include "tCopier.hpp"

namespace Sig
{
	/// \brief tFixedArrayBase allows us to inherit tUncopyable if t did.
	/// We'll also include mItems here so we don't need to rely on EBCO being applied to a full tUncopyable <- tFixedArrayBase <- tFixedArray chain.
	template<class t, int N, bool uncopyable>
	class tFixedArrayBase;

	template<class t, int N>
	class tFixedArrayBase<t,N,false> : public Memory::tRawMinAlignedStorage<t,N>
	{
	};

	template<class t, int N>
	class tFixedArrayBase<t,N,true> : public Memory::tRawMinAlignedStorage<t,N>, public tUncopyable
	{
	};

	/// \brief Provides fixed sized array functionality, with some handy stuff on top,
	/// like bounds checking, a count member, reflection, etc.
	template<class t, int N>
	class tFixedArray : public tFixedArrayBase<t,N,can_convert(t,tUncopyable)>
	{
		declare_explicit_reflector( );
		dependent_static_assert( sizeof(tFixedArrayBase<t,N,can_convert(t,tUncopyable)>) == sizeof(t)*N );
	private:
		struct tMakeConstructorUnavailableTag {};
		typedef typename Metaprogramming::SelectIfC<  can_convert(t,tUncopyable), tMakeConstructorUnavailableTag, true, tFixedArray >::tResult tFixedArrayIfCopyable;
		typedef typename Metaprogramming::SelectIfC< !can_convert(t,tUncopyable), tMakeConstructorUnavailableTag, true, tFixedArray >::tResult tFixedArrayIfUncopyable;

		inline tFixedArray( const tFixedArrayIfUncopyable& ) { sigassert(!"implemented"); } // not implemented... disable copy construction if can_convert(t,tUncopyable)
		inline tFixedArray& operator=( const tFixedArrayIfUncopyable& other ) { sigassert(!"implemented"); return *this; } // not implemented... disable copy construction if can_convert(t,tUncopyable)
	public:
		static const u32 cDimension = N;

		typedef t*			tIterator;
		typedef const t*	tConstIterator;

		inline tFixedArray( ) 
		{
			for( t* i = fBegin( ); i != fEnd( ); ++i )
				new ( i ) t;
		}

		inline tFixedArray( tNoOpTag ) 
		{
			for( t* i = fBegin( ) ; i != fEnd( ) ; ++i )
				new ( i ) t( cNoOpTag );
		}

		tArraySleeve<t> fToArraySleeve()
		{
			return tArraySleeve< t >( fBegin(), fCount() );
		}

		tArraySleeve<const t> fToArraySleeve() const
		{
			return tArraySleeve< const t >( fBegin(), fCount() );
		}

		/// COMPILE ERRORS HERE?
		/// You probably fed tFixedArray an uncopyable object that doesn't inherit (or privately inherits) tUncopyable.
		///  Please publically inherit tUncopyable for types that should not be copied.
		inline tFixedArray( const tFixedArrayIfCopyable& otherArray )
		{
			for( u32 i = 0 ; i < N; ++i )
				new (fBegin( ) + i) t( otherArray[i] );
		}

		template< typename iterT >
		inline tFixedArray( iterT begin, iterT end )
		{
			cmp_assert( end-begin, <=, cDimension );

			t* dst = fBegin();
			iterT src = begin;

			for( ; dst != fEnd() && begin != end; ++dst, ++src )
				new ( dst ) t( *src );

			for( ; dst != fEnd(); ++dst )
				new ( dst ) t;
		}

		/// COMPILE ERRORS HERE?
		/// You probably fed tFixedArray an unassignable object that doesn't inherit (or privately inherits) tUncopyable.
		///  Please publically inherit tUncopyable for types that should not be copied.
		inline tFixedArray& operator=( const tFixedArrayIfCopyable& other )
		{
			if( static_cast<const void*>(this) == static_cast<const void*>(&other) ) // tFixedArray* and tFixedArrayIfCopyable* might not be related
				return *this;

			for( u32 i = 0; i < N; ++i )
				((t*)this->mItems)[i] = ((t*)other.mItems)[i];

			return *this;
		}

		inline ~tFixedArray( )
		{
			for( t* i = fEnd( ) - 1; i != fBegin( ) - 1; --i )
				i->~t();
		}

		inline u32			fCount( ) const						{ return N; }
		inline t*			fBegin( )							{ return (t*)this->mItems; }
		inline const t*		fBegin( ) const						{ return (t*)this->mItems; }
		inline t*			fEnd( )								{ return ((t*)this->mItems) + N; }
		inline const t*		fEnd( ) const						{ return ((t*)this->mItems) + N; }
		inline t&			fFront( )							{ return *(t*)this->mItems; }
		inline const t&		fFront( ) const						{ return *(t*)this->mItems; }
		inline t&			fBack( )							{ return *(((t*)this->mItems) + N - 1); }
		inline const t&		fBack( ) const						{ return *(((t*)this->mItems) + N - 1); }
		inline t&			operator[]( const u32 i )			{ cmp_assert( i, <, N ); return ((t*)this->mItems)[i]; }
		inline const t&		operator[]( const u32 i ) const		{ cmp_assert( i, <, N ); return ((t*)this->mItems)[i]; }

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
		s32 fIndexOf( const u& object, u32 indexStart = 0 ) const
		{
			for( u32 i = indexStart; i < fCount( ); ++i )
				if( operator[]( i ) == object )
					return i;
			return -1;
		}

		template<class u>
		const t* fFind( const u& object ) const
		{
			for( u32 i = 0; i < fCount( ); ++i )
				if( operator[]( i ) == object )
					return &operator[]( i );
			return 0;
		}

		template<class u>
		t* fFind( const u& object )
		{
			for( u32 i = 0; i < fCount( ); ++i )
				if( operator[]( i ) == object )
					return &operator[]( i );
			return 0;
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
}

#endif //ndef __tFixedArray__
