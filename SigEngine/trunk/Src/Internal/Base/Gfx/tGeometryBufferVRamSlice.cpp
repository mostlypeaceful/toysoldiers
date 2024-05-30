#include "BasePch.hpp"
#include "tGeometryBufferVRamSlice.hpp"
#include "tProfiler.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		struct tGeometryBufferVRamSliceFake
		{
			tGeometryBufferVRamAllocator* mBuffer;
			u32 mStartVertex;
			u32 mNumVerts;
		};

		struct tSortGeometryBufferSliceByAddress
		{
			inline b32 operator()( const tGeometryBufferVRamSliceFake& a, const tGeometryBufferVRamSliceFake& b ) const
			{
				return a.mStartVertex < b.mStartVertex;
			}
		};

		static_assert( sizeof( tGeometryBufferVRamSliceFake ) == sizeof( tGeometryBufferVRamSlice ) );
	}


	tGeometryBufferVRamSlice::tGeometryBufferVRamSlice( )
		: mStartVertex( 0 )
		, mNumVerts( 0 )
	{
	}

	tGeometryBufferVRamSlice::tGeometryBufferVRamSlice( u32 startVertex, u32 numVerts )
		: mStartVertex( startVertex )
		, mNumVerts( numVerts )
	{
	}

	tGeometryBufferVRamSlice::tGeometryBufferVRamSlice( tGeometryBufferVRamAllocator* allocator, u32 startVertex, u32 numVerts )
		: mBuffer( allocator )
		, mStartVertex( startVertex )
		, mNumVerts( numVerts )
	{
	}

	namespace
	{
		static b32 gBufferLockingEnabled = true;
	}

	void tGeometryBufferVRamAllocator::fEnableBufferLocking( b32 enable )
	{
#ifdef platform_xbox360
		gBufferLockingEnabled = enable;
#else
		gBufferLockingEnabled = true;
#endif
	}

	b32 tGeometryBufferVRamAllocator::fBufferLockingEnabled( )
	{
		return gBufferLockingEnabled;
	}

	u32 tGeometryBufferVRamAllocator::fMultiBufferCount( )
	{
		return 2;
	}

	void tGeometryBufferVRamAllocator::fGlobalPreRender( )
	{
		const tGrowableArray<tGeometryBufferVRamAllocator*>& allocators = fGlobalList( );
		for( u32 i = 0; i < allocators.fCount( ); ++i )
			allocators[ i ]->fPreRender( );
	}

	void tGeometryBufferVRamAllocator::fGlobalPostRender( )
	{
		const tGrowableArray<tGeometryBufferVRamAllocator*>& allocators = fGlobalList( );
		for( u32 i = 0; i < allocators.fCount( ); ++i )
			allocators[ i ]->fPostRender( );
	}

	tGeometryBufferVRamAllocator::tGeometryBufferVRamAllocator( )
		: mTotalAllocated( 0 )
		, mDirty( false )
		, mCurrentFreedSlot( 0 )
	{
		mFreedSlices.fSetCount( tGeometryBufferVRamAllocator::fMultiBufferCount( ) );
	}

	tGeometryBufferVRamAllocator::~tGeometryBufferVRamAllocator( )
	{
		profile_geom_allocator( *this, 0, true );
	}

	void tGeometryBufferVRamAllocator::fAllocate( const tDevicePtr& device, const tVertexFormat& format, u32 numVerts, u32 allocFlags )
	{
		fDeallocate( );

		tGeometryBufferVRam::fAllocate( device, format, numVerts, allocFlags );

		fDeepLock( );

		mSlices.fSetCount( 1 );
		mSlices[ 0 ] = tGeometryBufferVRamSlice( 0u, numVerts );
	}

	void tGeometryBufferVRamAllocator::fDeallocate( )
	{
		tGeometryBufferVRamAllocatorPtr safe( this );
		for( u32 i = 0; i < mFreedSlices.fCount( ); ++i )
			fPostRender( );
		for( u32 i = 0; i < mSlices.fCount( ); ++i )
			mSlices[ i ].mBuffer.fRelease( );
		mSlices.fSetCount( 0 );
		tGeometryBufferVRam::fDeallocate( );
	}

	tGeometryBufferVRamSlice tGeometryBufferVRamAllocator::fGetSlice( u32 numVerts )
	{
		//fConsolidate( );

		tGeometryBufferVRamSlice o;

		// find the first N slices that fit
		const u32 maxN = 10;
		u32 n = 0;
		u32 best = 0;

		for( u32 i = 0; i < mSlices.fCount( ); ++i )
		{
			if( mSlices[ i ].mNumVerts == numVerts )
			{
				// incroyable, this slice fits perfectly, just return it immediately
				o = mSlices[ i ];
				o.mBuffer.fReset( this );
				mTotalAllocated += o.mNumVerts * mFormat.fVertexSize( );
				mSlices.fErase( i );
				mDirty = true;
				return o;
			}
			else if( mSlices[ i ].mNumVerts > numVerts )
			{
				// candidate
				if( n == 0 || mSlices[ i ].mNumVerts < mSlices[ best ].mNumVerts )
					best = i;
				if( ++n >= maxN )
					break; // found enough to consider
			}
		}

		if( n > 0 )
		{
			// we have a valid candidate, shave off the end and insert waste
			o = mSlices[ best ];
			mSlices[ best ] = tGeometryBufferVRamSlice( o.mStartVertex + numVerts, o.mNumVerts - numVerts );
			o.mNumVerts = numVerts;
			mDirty = true;
		}

		o.mBuffer.fReset( this );
		mTotalAllocated += o.mNumVerts * mFormat.fVertexSize( );
		return o;
	}

	void tGeometryBufferVRamAllocator::fReturnSlice( tGeometryBufferVRamSlice& slice )
	{
		if( fVertexCount( ) > 0 ) // otherwise we've already de-allocated
		{
			mFreedSlices[ mCurrentFreedSlot ].fPushBack( slice );

			// clear buffer slice
			slice = tGeometryBufferVRamSlice( this );
		}
		else
			slice = tGeometryBufferVRamSlice( );
	}

	void tGeometryBufferVRamAllocator::fReturnSliceInternal( tGeometryBufferVRamSlice& slice )
	{
		sigassert( slice.mBuffer.fGetRawPtr( ) == this );

		mTotalAllocated -= slice.mNumVerts * mFormat.fVertexSize( );
		sigassert( mTotalAllocated >= 0 );

		if( !slice.fEmpty( ) )
		{
			mSlices.fPushBack( slice );
			mSlices.fBack( ).mBuffer.fRelease( ); // avoid cyclical references
			mDirty = true;
		}

		// clear buffer slice
		slice = tGeometryBufferVRamSlice( this );
	}

	void tGeometryBufferVRamAllocator::fGetAndReturn( tGeometryBufferVRamSlice& slice, u32 numVerts )
	{
		fReturnSlice( slice );
		if( numVerts > 0 )
			slice = fGetSlice( numVerts );
	}

	void tGeometryBufferVRamAllocator::fPreRender( )
	{
		if( tGeometryBufferVRamAllocator::fBufferLockingEnabled( ) )
			fDeepUnlock( );
	}

	void tGeometryBufferVRamAllocator::fPostRender( )
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

	void tGeometryBufferVRamAllocator::fConsolidate( )
	{
		if( !mDirty || mSlices.fCount( ) <= 1 )
			return;

		// sort by "address"
		std::sort( 
			reinterpret_cast<tGeometryBufferVRamSliceFake*>( mSlices.fBegin( ) ), 
			reinterpret_cast<tGeometryBufferVRamSliceFake*>( mSlices.fEnd( ) ), 
			tSortGeometryBufferSliceByAddress( ) );

		// collapse adjacent
		for( u32 i = 0; i < mSlices.fCount( ) - 1; ++i )
		{
			if( mSlices[ i ].mStartVertex + mSlices[ i ].mNumVerts == mSlices[ i + 1 ].mStartVertex )
			{
				mSlices[ i ].mNumVerts += mSlices[ i + 1 ].mNumVerts;
				mSlices.fEraseOrdered( i + 1 );
				--i;
			}
		}

		mDirty = false;
	}

}}


