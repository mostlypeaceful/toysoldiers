#ifndef __tGeometryBufferVRamSlice__
#define __tGeometryBufferVRamSlice__
#include "tGloballyAccessible.hpp"
#include "tGeometryBufferVRam.hpp"

namespace Sig { namespace Gfx
{

	class tGeometryBufferVRamAllocator;

	define_smart_ptr( base_export, tRefCounterPtr, tGeometryBufferVRamAllocator );

	class base_export tGeometryBufferVRamSlice
	{
		define_class_pool_new_delete( tGeometryBufferVRamSlice, 128 );

	public:
		tGeometryBufferVRamAllocatorPtr mBuffer;
		u32 mStartVertex;
		u32 mNumVerts;

		tGeometryBufferVRamSlice( );
		tGeometryBufferVRamSlice( u32 startVertex, u32 numVerts );
		explicit tGeometryBufferVRamSlice( tGeometryBufferVRamAllocator* allocator, u32 startVertex = 0, u32 numVerts = 0 );

		inline b32 fEmpty( ) const { return mNumVerts == 0; }
	};

	class base_export tGeometryBufferVRamAllocator
		: public tGloballyAccessible<tGeometryBufferVRamAllocator,4>
		, public tUncopyable
		, public tGeometryBufferVRam
		, public tRefCounter
	{
		typedef tGrowableArray< tGeometryBufferVRamSlice > tSliceArray;
		tSliceArray mSlices;
		s32 mTotalAllocated;
		b32 mDirty;

		tGrowableArray<tSliceArray> mFreedSlices;
		u32 mCurrentFreedSlot;

	public:
		static void fEnableBufferLocking( b32 enable );
		static b32  fBufferLockingEnabled( );
		static u32 fMultiBufferCount( );
		static void fGlobalPreRender( );
		static void fGlobalPostRender( );

	public:
		tGeometryBufferVRamAllocator( );
		~tGeometryBufferVRamAllocator( );
		void fAllocate( const tDevicePtr& device, const tVertexFormat& format, u32 numVerts, u32 allocFlags );
		void fDeallocate( );
		s32 fTotalAllocated( ) const { return mTotalAllocated; }
		tGeometryBufferVRamSlice fGetSlice( u32 numVerts );
		void fReturnSlice( tGeometryBufferVRamSlice& slice );
		void fGetAndReturn( tGeometryBufferVRamSlice& slice, u32 numVerts );
		void fPreRender( );
		void fPostRender( );

	private:
		void fReturnSliceInternal( tGeometryBufferVRamSlice& slice );
		void fConsolidate( );
	};

}}


#endif//__tGeometryBufferVRamSlice__

