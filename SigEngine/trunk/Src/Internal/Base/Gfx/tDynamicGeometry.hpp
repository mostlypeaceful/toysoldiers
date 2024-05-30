#ifndef __tDynamicGeometry__
#define __tDynamicGeometry__
#include "Gfx/tRenderBatch.hpp"
#include "Gfx/tGeometryBufferVRamSlice.hpp"
#include "Gfx/tIndexBufferVRamSlice.hpp"


namespace Sig { namespace Gfx
{

	class base_export tDynamicGeometry : public tUncopyable, public tRefCounter
	{
	public:
		struct tBufferedState
		{
			tRenderBatchPtr				mRenderBatch;
			tGeometryBufferVRamSlice	mGeometry;
			tIndexBufferVRamSlice		mIndices;
			void						fReleaseResources( );
		};

	public:
		explicit tDynamicGeometry( u32 batchBehaviorFlags = 0 );
		virtual ~tDynamicGeometry( );

		void fResetDeviceObjects( 
			const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const tIndexBufferVRamAllocatorPtr& indexAllocator );

		b32  fAllocateGeometry( const tMaterial& mtl, u32 numVerts, u32 numIds, u32 numPrims );
		b32  fAllocateVertices( const tMaterial& mtl, u32 numVerts, b32 updateRenderBatch = true );
		b32  fAllocateIndices( const tMaterial& mtl, u32 numIds, u32 numPrims, b32 updateRenderBatch = true );

		void fCopyVertsToGpu( const void* vertBuffer, u32 numVerts );
		void fCopyIndicesToGpu( const void* idBuffer, u32 numIds );

		Sig::byte* fLockBuffer( ) const { return fCurrentState( ).mGeometry.mBuffer->fQuickLock( fCurrentState( ).mGeometry.mStartVertex, fCurrentState( ).mGeometry.mNumVerts ); }
		void fUnlockBuffer( Sig::byte* p ) { fCurrentState( ).mGeometry.mBuffer->fQuickUnlock( p ); }
		Sig::byte* fLockIndices( ) const { return fCurrentState( ).mIndices.mBuffer->fQuickLock( fCurrentState( ).mIndices.mStartIndex, fCurrentState( ).mIndices.mNumIds ); }
		void fUnlockIndices( Sig::byte* p ) { fCurrentState( ).mIndices.mBuffer->fQuickUnlock( p ); }

		void									fSetRenderStateOverride( const tRenderState* rs ) { mRenderStateOverride = rs; }
		void									fSetPrimTypeOverride( tIndexFormat::tPrimitiveType primType ) { mPrimTypeOverride = primType; }
		void									fChangeMaterial( const tMaterial& mtl );
		const tRenderBatchPtr&					fGetRenderBatch( ) const { return fCurrentState( ).mRenderBatch; }
		tRenderBatchPtr							fGetModifiedRenderBatch( const tRenderState* rsOverride ) const;
		const tGeometryBufferVRamAllocatorPtr&	fGetGeometryAllocator( ) const { return fCurrentState( ).mGeometry.mBuffer; }
		const tIndexBufferVRamAllocatorPtr&		fGetIndexAllocator( ) const { return fCurrentState( ).mIndices.mBuffer; }
		const tIndexFormat&						fIndexFormat( ) const { return fIndices( ).mBuffer->fIndexFormat( ); }

		void									fSetBatchBehaviorFlags( u32 behaviorFlags );
		u32										fGetBatchBehaviorFlags( ) { return mBatchBehaviorFlags; }

		const tBufferedState&	fCurrentState( ) const	{ return mBufferedState; }
		tBufferedState&			fCurrentState( )		{ return mBufferedState; }

		void fFreeSlice( );

	protected:

		const tGeometryBufferVRamSlice& fGeometry( ) const { return fCurrentState( ).mGeometry; }
		const tIndexBufferVRamSlice& fIndices( ) const { return fCurrentState( ).mIndices; }

		void fSetupRenderBatch( const tMaterial& mtl, u32 numVerts, u32 numPrims );

	private:
		tBufferedState					mBufferedState;
		const tRenderState*				mRenderStateOverride;
		tIndexFormat::tPrimitiveType	mPrimTypeOverride;
		u32								mBatchBehaviorFlags;
	};

	typedef tRefCounterPtr< tDynamicGeometry > tDynamicGeometryPtr;

}}

#endif//__tDynamicGeometry__
