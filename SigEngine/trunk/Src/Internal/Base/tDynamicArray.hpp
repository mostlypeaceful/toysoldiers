#ifndef __tDynamicArray__
#define __tDynamicArray__

#include "tArraySleeve.hpp"
#include "tCopier.hpp"

namespace Sig
{
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
		{
		}
		inline tDynamicArray( const tDynamicArray& other ) 
		{
			fCopy( other );
		}
		inline tDynamicArray( tNoOpTag ) 
			: tBase( cNoOpTag )
		{
		}
		inline explicit tDynamicArray( u32 count )
		{
			fNewArray( count );
		}
		inline ~tDynamicArray( )
		{
			tBase::fDeleteArray( );
		}

		tArraySleeve<t> fToArraySleeve()
		{
			return tArraySleeve< t >( fBegin(), fCount() );
		}

		tArraySleeve<const t> fToArraySleeve() const
		{
			return tArraySleeve< const t >( fBegin(), fCount() );
		}

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
			cmp_assert( index, <=, tBase::mCount );

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
		s32 fIndexOf( const u& object, u32 indexStart = 0 ) const
		{
			for( u32 i = indexStart; i < tBase::mCount; ++i )
				if( tBase::mItems[i] == object )
					return i;
			return -1;
		}

		template<class U>
		void fJoin( const U& other )
		{
			fInsert( tBase::fCount( ), other.fBegin( ), other.fCount( ) );
		}

		// You should probably be using tGrowableArray
		inline void fErase( u32 index )
		{
			cmp_assert( index, <, this->mCount );
			this->mItems[index] = this->mItems[this->mCount-1];
			fResize( this->mCount - 1 );
		}

		// You should probably be using tGrowableArray
		void fEraseOrdered( u32 start, u32 count )
		{
			cmp_assert( start, <, this->mCount );
			log_assert( start + count <= this->mCount, "start=" << start << " count=" << count << " start+count=" << (start+count) << " mCount=" << this->mCount );

			tCopier<t>::fCopyOverlapped(
				tBase::fBegin( ) + start,
				tBase::fBegin( ) + start + count,
				this->mCount - start - count );

			fResize( this->mCount - count );
		}

		// You should probably be using tGrowableArray
		inline void fEraseOrdered( u32 index )
		{
			fEraseOrdered( index, 1 );
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
}

#endif //ndef __tDynamicArray__
