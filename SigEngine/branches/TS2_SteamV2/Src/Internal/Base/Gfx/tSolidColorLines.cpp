#include "BasePch.hpp"
#include "tSolidColorLines.hpp"

namespace Sig { namespace Gfx
{

	void tSolidColorLines::fResetDeviceObjects( 
		const tDevicePtr& device,
		const tMaterialPtr& material, 
		const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		mGeometry.fResetDeviceObjects( device, material, geometryAllocator, indexAllocator );
		fSetRenderBatch( tRenderBatchPtr( ) );
	}

	template<class tIndex>
	void fGenerateLineList( tIndex* ids, u32 numPrims, b32 strip )
	{
		sigassert( sizeof( tIndex ) == sizeof( u16 ) || sizeof( tIndex ) == sizeof( u32 ) );

		if( strip )
		{
			for( u32 iline = 0; iline < numPrims; ++iline )
			{
				ids[ iline * 2 + 0 ] = iline * 2 + 0 - ( iline & 1 );
				ids[ iline * 2 + 1 ] = iline * 2 + 1 - ( iline & 1 );
			}
		}
		else
		{
			for( u32 iline = 0; iline < numPrims; ++iline )
			{
				ids[ iline * 2 + 0 ] = iline * 2 + 0;
				ids[ iline * 2 + 1 ] = iline * 2 + 1;
			}
		}
	}

	void tSolidColorLines::fBake( const Sig::byte* verts, u32 numVerts, b32 strip )
	{
		const u32 numPrims	= strip ? ( numVerts - 1 ) : ( numVerts / 2 );
		const u32 numIds	= numPrims * 2;

		if( strip )
			mGeometry.fSetPrimTypeOverride( tIndexFormat::cPrimitiveLineStrip );
		else
			mGeometry.fSetPrimTypeOverride( tIndexFormat::cPrimitiveLineList );

		if( !mGeometry.fAllocateGeometry( *mGeometry.fMaterial( ), numVerts, numIds, numPrims ) )
		{
			log_warning( Log::cFlagGraphics, "Failed to allocate geometry for Solid Color Lines" );
			fSetRenderBatch( tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		// copy vert data to gpu
		mGeometry.fCopyVertsToGpu( verts, numVerts );

		// generate indices
		if( mGeometry.fIndexFormat( ).mStorageType == Gfx::tIndexFormat::cStorageU16 )
		{
			tDynamicArray<u16> ids( numIds );
			fGenerateLineList( ids.fBegin( ), numPrims, strip );
			mGeometry.fCopyIndicesToGpu( ids.fBegin( ), numIds );
		}
		else
		{
			tDynamicArray<u32> ids( numIds );
			fGenerateLineList( ids.fBegin( ), numPrims, strip );
			mGeometry.fCopyIndicesToGpu( ids.fBegin( ), numIds );
		}

		tRenderableEntity::fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

}}

