#include "BasePch.hpp"
#include "tIndexBufferVRamSlice.hpp"
#include "tGeometryBufferVRamSlice.hpp"
#include "tProfiler.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		struct tIndexBufferVRamSliceFake
		{
			tIndexBufferVRamAllocator* mBuffer;
			u32 mStartIndex;
			u32 mNumIds;
		};

		struct tSortIndexBufferSliceByAddress
		{
			inline b32 operator()( const tIndexBufferVRamSliceFake& a, const tIndexBufferVRamSliceFake& b ) const
			{
				return a.mStartIndex < b.mStartIndex;
			}
		};

		sig_static_assert( sizeof( tIndexBufferVRamSliceFake ) == sizeof( tIndexBufferVRamSlice ) );
	}

	tIndexBufferVRamSlice::tIndexBufferVRamSlice( )
		: mStartIndex( 0 )
		, mNumIds( 0 )
	{
	}

	tIndexBufferVRamSlice::tIndexBufferVRamSlice( u32 startIndex, u32 numIds )
		: mStartIndex( startIndex )
		, mNumIds( numIds )
	{
	}

	tIndexBufferVRamSlice::tIndexBufferVRamSlice( tIndexBufferVRamAllocator* buffer, u32 startIndex, u32 numIds )
		: mBuffer( buffer )
		, mStartIndex( startIndex )
		, mNumIds( numIds )
	{
	}


	void tIndexBufferVRamAllocator::fGlobalPreRender( )
	{
		const tGrowableArray<tIndexBufferVRamAllocator*>& allocators = fGlobalList( );
		for( u32 i = 0; i < allocators.fCount( ); ++i )
			allocators[ i ]->fPreRender( );
	}

	void tIndexBufferVRamAllocator::fGlobalPostRender( )
	{
		const tGrowableArray<tIndexBufferVRamAllocator*>& allocators = fGlobalList( );
		for( u32 i = 0; i < allocators.fCount( ); ++i )
			allocators[ i ]->fPostRender( );
	}

	tIndexBufferVRamAllocator::tIndexBufferVRamAllocator( )
		: mTotalAllocated( 0 )
		, mDirty( false )
		, mCurrentFreedSlot( 0 )
	{
		mFreedSlices.fSetCount( tGeometryBufferVRamAllocator::fMultiBufferCount( ) );
	}

	tIndexBufferVRamAllocator::~tIndexBufferVRamAllocator( )
	{
		profile_geom_allocator( *this, 0, true );
	}

	void tIndexBufferVRamAllocator::fAllocate( const tDevicePtr& device, const tIndexFormat& format, u32 numIds, u32 numPrims, u32 allocFlags )
	{
		fDeallocate( );

		tIndexBufferVRam::fAllocate( device, format, numIds, numPrims, allocFlags );

		fDeepLock( );

		mSlices.fSetCount( 1 );
		mSlices[ 0 ] = tIndexBufferVRamSlice( 0u, numIds );
	}

	void tIndexBufferVRamAllocator::fDeallocate( )
	{
		tIndexBufferVRamAllocatorPtr safe( this );
		for( u32 i = 0; i < mFreedSlices.fCount( ); ++i )
			fPostRender( );
		for( u32 i = 0; i < mSlices.fCount( ); ++i )
			mSlices[ i ].mBuffer.fRelease( );
		mSlices.fSetCount( 0 );
		tIndexBufferVRam::fDeallocate( );
	}

	tIndexBufferVRamSlice tIndexBufferVRamAllocator::fGetSlice( u32 numIds )
	{
		//fConsolidate( );

		tIndexBufferVRamSlice o;

		// find the first N slices that fit
		const u32 maxN = 10;
		u32 n = 0;
		u32 best = 0;

		for( u32 i = 0; i < mSlices.fCount( ); ++i )
		{
			if( mSlices[ i ].mNumIds == numIds )
			{
				// incroyable, this slice fits perfectly, just return it immediately
				o = mSlices[ i ];
				o.mBuffer.fReset( this );
				mTotalAllocated += o.mNumIds * mFormat.mSize;
				mSlices.fErase( i );
				mDirty = true;
				return o;
			}
			else if( mSlices[ i ].mNumIds > numIds )
			{
				// candidate
				if( n == 0 || mSlices[ i ].mNumIds < mSlices[ best ].mNumIds )
					best = i;
				if( ++n >= maxN )
					break; // found enough to consider
			}
		}

		if( n > 0 )
		{
			// we have a valid candidate, shave off the end and insert waste
			o = mSlices[ best ];			
			mSlices[ best ] = tIndexBufferVRamSlice( o.mStartIndex + numIds, o.mNumIds - numIds );
			o.mNumIds = numIds;
			mDirty = true;
		}

		o.mBuffer.fReset( this );
		mTotalAllocated += o.mNumIds * mFormat.mSize;
		return o;
	}

	void tIndexBufferVRamAllocator::fReturnSlice( tIndexBufferVRamSlice& slice )
	{
		if( fIndexCount( ) > 0 ) // otherwise we've already de-allocated
		{
			mFreedSlices[ mCurrentFreedSlot ].fPushBack( slice );

			// clear buffer slice
			slice = tIndexBufferVRamSlice( this );
		}
		else
			slice = tIndexBufferVRamSlice( );
	}

	void tIndexBufferVRamAllocator::fReturnSliceInternal( tIndexBufferVRamSlice& slice )
	{
		sigassert( slice.mBuffer.fGetRawPtr( ) == this );

		mTotalAllocated -= slice.mNumIds * mFormat.mSize;
		sigassert( mTotalAllocated >= 0 );

		if( !slice.fEmpty( ) )
		{
			mSlices.fPushBack( slice );
			mSlices.fBack( ).mBuffer.fRelease( ); // avoid cyclical references
			mDirty = true;
		}

		// clear buffer slice
		slice = tIndexBufferVRamSlice( this );
	}

	void tIndexBufferVRamAllocator::fGetAndReturn( tIndexBufferVRamSlice& slice, u32 numIds )
	{
		fReturnSlice( slice );
		if( numIds > 0 )
			slice = fGetSlice( numIds );
	}

	void tIndexBufferVRamAllocator::fPreRender( )
	{
		if( tGeometryBufferVRamAllocator::fBufferLockingEnabled( ) )
			fDeepUnlock( );
	}

	void tIndexBufferVRamAllocator::fPostRender( )
	{
		fConsolidate( );

		fDeepLock( );

		const u32 maxSlots = mFreedSlices.fCount( );
		if( mCurrentFreedSlot == 0 )
			mCurrentFreedSlot = maxSlots - 1; // wrap around
		else
			mCurrentFreedSlot -= 1;

		for( u32 i = 0; i < mFreedSlices[ mCurrentFreedSlot ].fCount( ); ++i )
			fReturnSliceInternal( mFreedSlices[ mCurrentFreedSlot ][ i ] );
		mFreedSlices[ mCurrentFreedSlot ].fSetCount( 0 );
	}

	void tIndexBufferVRamAllocator::fConsolidate( )
	{
		if( !mDirty || mSlices.fCount( ) <= 1 )
			return;

		// sort by "address"
		std::sort( 
			reinterpret_cast<tIndexBufferVRamSliceFake*>( mSlices.fBegin( ) ),
			reinterpret_cast<tIndexBufferVRamSliceFake*>( mSlices.fEnd( ) ),
			tSortIndexBufferSliceByAddress( ) );

		// collapse adjacent
		for( u32 i = 0; i < mSlices.fCount( ) - 1; ++i )
		{
			if( mSlices[ i ].mStartIndex + mSlices[ i ].mNumIds == mSlices[ i + 1 ].mStartIndex )
			{
				mSlices[ i ].mNumIds += mSlices[ i + 1 ].mNumIds;
				mSlices.fEraseOrdered( i + 1 );
				--i;
			}
		}

		mDirty = false;
	}


}}


