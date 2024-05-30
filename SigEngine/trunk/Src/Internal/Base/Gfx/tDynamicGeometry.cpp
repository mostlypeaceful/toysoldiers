#include "BasePch.hpp"
#include "tDynamicGeometry.hpp"
#include "tMaterial.hpp"

namespace Sig { namespace Gfx
{
	void tDynamicGeometry::tBufferedState::fReleaseResources( )
	{
		if( mGeometry.mBuffer )
			mGeometry.mBuffer->fReturnSlice( mGeometry );

		if( mIndices.mBuffer )
			mIndices.mBuffer->fReturnSlice( mIndices );

		mRenderBatch.fRelease( );
	}

	tDynamicGeometry::tDynamicGeometry( u32 batchBehaviorFlags )
		: mRenderStateOverride( 0 )
		, mPrimTypeOverride( tIndexFormat::cPrimitiveInvalid )
		, mBatchBehaviorFlags( batchBehaviorFlags )
	{
	}
	tDynamicGeometry::~tDynamicGeometry( )
	{
		fFreeSlice( );
	}

	void tDynamicGeometry::fResetDeviceObjects( 
			const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		fFreeSlice( );
		mBufferedState.mGeometry = tGeometryBufferVRamSlice( geometryAllocator.fGetRawPtr( ) );
		mBufferedState.mIndices = tIndexBufferVRamSlice( indexAllocator.fGetRawPtr( ) );
	}

	void tDynamicGeometry::fFreeSlice( )
	{
		mBufferedState.fReleaseResources( );
	}

	void tDynamicGeometry::fChangeMaterial( const tMaterial& mtl )
	{
		if( mBufferedState.mRenderBatch )
			fSetupRenderBatch( mtl, mBufferedState.mRenderBatch->fBatchData( ).mVertexCount, mBufferedState.mRenderBatch->fBatchData( ).mPrimitiveCount );
	}

	//------------------------------------------------------------------------------
	void tDynamicGeometry::fSetBatchBehaviorFlags( u32 behaviorFlags )
	{
		if( mBatchBehaviorFlags == behaviorFlags )
			return;

		mBatchBehaviorFlags = behaviorFlags;
		if( mBufferedState.mRenderBatch )
		{
			fSetupRenderBatch( 
				*mBufferedState.mRenderBatch->fBatchData( ).mMaterial, 
				mBufferedState.mRenderBatch->fBatchData( ).mVertexCount, 
				mBufferedState.mRenderBatch->fBatchData( ).mPrimitiveCount );
		}
	}

	void tDynamicGeometry::fSetupRenderBatch( const tMaterial& mtl, u32 numVerts, u32 numPrims )
	{
		tBufferedState& state = fCurrentState( );

		tRenderBatchData batchData;
		batchData.mRenderState			= mRenderStateOverride ? mRenderStateOverride : &mtl.fGetRenderState( );
		batchData.mMaterial				= &mtl;
		batchData.mVertexFormat			= &state.mGeometry.mBuffer->fVertexFormat( );
		batchData.mGeometryBuffer		= state.mGeometry.mBuffer.fGetRawPtr( );
		batchData.mIndexBuffer			= state.mIndices.mBuffer.fGetRawPtr( );
		batchData.mVertexCount			= numVerts;
		batchData.mBaseVertexIndex		= state.mGeometry.mStartVertex;
		batchData.mPrimitiveCount		= numPrims;
		batchData.mBaseIndexIndex		= state.mIndices.mStartIndex;
		if( mPrimTypeOverride == tIndexFormat::cPrimitiveInvalid )
			batchData.mPrimitiveType	= state.mIndices.mBuffer->fIndexFormat( ).mPrimitiveType;
		else
			batchData.mPrimitiveType	= mPrimTypeOverride;
		batchData.mBehaviorFlags		= mBatchBehaviorFlags;

		state.mRenderBatch				= tRenderBatch::fCreate( batchData );
	}

	b32 tDynamicGeometry::fAllocateGeometry( const tMaterial& mtl, u32 numVerts, u32 numIds, u32 numPrims )
	{
		b32 success = true;
		success = fAllocateVertices( mtl, numVerts, false ) && success;
		success = fAllocateIndices( mtl, numIds, numPrims, false ) && success;
		fSetupRenderBatch( mtl, numVerts, numPrims );
		return success;
	}
	b32 tDynamicGeometry::fAllocateVertices( const tMaterial& mtl, u32 numVerts, b32 updateRenderBatch )
	{
		tBufferedState& state = fCurrentState( );

		//const f32 overAllocationFactors[] = { 4.f, 2.f, 1.5f };
		//const u32 numOverAllocationFactors = array_length( overAllocationFactors );
		//const u32 ithOverAllocationFactor = fMin<u32>( numVerts / 256, numOverAllocationFactors - 1 );
		//const f32 overAllocationFactor = overAllocationFactors[ ithOverAllocationFactor ];

		//b32 geometryChanged = state.mRenderBatch.fNull( ) || ( numVerts != state.mRenderBatch->fBatchData( ).mVertexCount );

		// allocate geometry if necessary
		//if( state.mGeometry.mNumVerts < numVerts ||
		//	state.mGeometry.mNumVerts > fRound<u32>( numVerts * overAllocationFactor ) )
		//{
			// not enough geometry, or else way too much, re-allocate
			state.mGeometry.mBuffer->fGetAndReturn( state.mGeometry, numVerts );
		//	geometryChanged = true;
		//}

		if( state.mGeometry.mNumVerts < numVerts )
		{
			state.fReleaseResources( );
			return false; // couldn't allocate enough verts
		}

		if( /*geometryChanged && */updateRenderBatch )
			fSetupRenderBatch( mtl, numVerts, state.mRenderBatch ? state.mRenderBatch->fBatchData( ).mPrimitiveCount : 0 );

		return true;
	}
	b32 tDynamicGeometry::fAllocateIndices( const tMaterial& mtl, u32 numIds, u32 numPrims, b32 updateRenderBatch )
	{
		tBufferedState& state = fCurrentState( );

		//const f32 overAllocationFactors[] = { 4.f, 2.f, 1.5f };
		//const u32 numOverAllocationFactors = array_length( overAllocationFactors );
		//const u32 ithOverAllocationFactor = fMin<u32>( numIds / 256, numOverAllocationFactors - 1 );
		//const f32 overAllocationFactor = overAllocationFactors[ ithOverAllocationFactor ];

		//b32 indicesChanged	= state.mRenderBatch.fNull( ) || ( numPrims != state.mRenderBatch->fBatchData( ).mPrimitiveCount );

		//// allocate index buffer if necessary
		//if( state.mIndices.mNumIds < numIds ||
		//	state.mIndices.mNumIds > fRound<u32>( numIds * overAllocationFactor ) )
		//{
			// not enough indices, or else way too much, re-allocate
			state.mIndices.mBuffer->fGetAndReturn( state.mIndices, numIds );
		//	indicesChanged = true;
		//}

		if( state.mIndices.mNumIds < numIds )
		{
			state.fReleaseResources( );
			return false; // couldn't allocate enough verts/indices
		}

		if( /*indicesChanged && */updateRenderBatch )
			fSetupRenderBatch( mtl, state.mRenderBatch ? state.mRenderBatch->fBatchData( ).mVertexCount : 0, numPrims );

		return true;
	}
	void tDynamicGeometry::fCopyVertsToGpu( const void* vertBuffer, u32 numVerts )
	{
		Sig::byte* gpuVerts = fGeometry( ).mBuffer->fQuickLock( fGeometry( ).mStartVertex, fGeometry( ).mNumVerts );
		sigassert( gpuVerts );
		fMemCpyToGpu( gpuVerts, vertBuffer, numVerts * fGeometry( ).mBuffer->fVertexFormat( ).fVertexSize( ) );
		fGeometry( ).mBuffer->fQuickUnlock( gpuVerts );
	}
	void tDynamicGeometry::fCopyIndicesToGpu( const void* idBuffer, u32 numIds )
	{
		Sig::byte* gpuIds = fIndices( ).mBuffer->fQuickLock( fIndices( ).mStartIndex, fIndices( ).mNumIds );
		sigassert( gpuIds );
		fMemCpyToGpu( gpuIds, idBuffer, numIds * fIndices( ).mBuffer->fIndexFormat( ).mSize );
		fIndices( ).mBuffer->fQuickUnlock( gpuIds );
	}
	tRenderBatchPtr tDynamicGeometry::fGetModifiedRenderBatch( const tRenderState* rsOverride ) const
	{
		sigassert( !fCurrentState( ).mRenderBatch.fNull( ) );
		tRenderBatchData batchData = fCurrentState( ).mRenderBatch->fBatchData( );
		if( rsOverride )
			batchData.mRenderState = rsOverride;
		return tRenderBatch::fCreate( batchData );
	}

}}

