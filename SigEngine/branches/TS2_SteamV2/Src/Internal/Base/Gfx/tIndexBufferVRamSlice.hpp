#ifndef __tIndexBufferVRamSlice__
#define __tIndexBufferVRamSlice__
#include "tGloballyAccessible.hpp"
#include "tIndexBufferVRam.hpp"

namespace Sig { namespace Gfx
{

	class tIndexBufferVRamAllocator;

	define_smart_ptr( base_export, tRefCounterPtr, tIndexBufferVRamAllocator );

	class base_export tIndexBufferVRamSlice
	{
		define_class_pool_new_delete( tIndexBufferVRamSlice, 128 );

	public:
		tIndexBufferVRamAllocatorPtr mBuffer;
		u32 mStartIndex;
		u32 mNumIds;

		tIndexBufferVRamSlice( );
		tIndexBufferVRamSlice( u32 startIndex, u32 numIds );
		explicit tIndexBufferVRamSlice( tIndexBufferVRamAllocator* buffer, u32 startIndex = 0, u32 numIds = 0 );

		inline b32 fEmpty( ) const { return mNumIds == 0; }
	};

	class base_export tIndexBufferVRamAllocator
		: public tGloballyAccessible<tIndexBufferVRamAllocator,4>
		, public tUncopyable
		, public tIndexBufferVRam
		, public tRefCounter
	{
		typedef tGrowableArray< tIndexBufferVRamSlice > tSliceArray;
		tSliceArray mSlices;
		s32 mTotalAllocated;
		b32 mDirty;

		tGrowableArray<tSliceArray> mFreedSlices;
		u32 mCurrentFreedSlot;

	public:
		static void fGlobalPreRender( );
		static void fGlobalPostRender( );

	public:
		tIndexBufferVRamAllocator( );
		~tIndexBufferVRamAllocator( );
		void fAllocate( const tDevicePtr& device, const tIndexFormat& format, u32 numIds, u32 numPrims, u32 allocFlags );
		void fDeallocate( );
		s32 fTotalAllocated( ) const { return mTotalAllocated; }
		tIndexBufferVRamSlice fGetSlice( u32 numIds );
		void fReturnSlice( tIndexBufferVRamSlice& slice );
		void fGetAndReturn( tIndexBufferVRamSlice& slice, u32 numIds );
		void fPreRender( );
		void fPostRender( );

	private:
		void fReturnSliceInternal( tIndexBufferVRamSlice& slice );
		void fConsolidate( );
	};

}}


#endif//__tIndexBufferVRamSlice__

