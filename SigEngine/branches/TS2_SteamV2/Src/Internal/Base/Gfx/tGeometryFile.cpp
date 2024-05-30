#include "BasePch.hpp"
#include "tGeometryFile.hpp"
#include "tDevice.hpp"
#include "tProfiler.hpp"

namespace Sig { namespace Gfx
{
	const u32 tGeometryFile::cVersion = 0;

	const char* tGeometryFile::fGetFileExtension( )
	{
		return ".geob";
	}

	tGeometryFile::tGeometryFile( )
		: mHeaderSize( 0 )
		, mDiscardSysRamBuffers( 0 )
	{
	}

	tGeometryFile::tGeometryFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mGeometryPointers( cNoOpTag )
		, mIndexListPointers( cNoOpTag )
	{
	}

	tGeometryFile::~tGeometryFile( )
	{
	}

	void tGeometryFile::fOnFileLoaded( const tResource& ownerResource )
	{
		tDevicePtr device = tDevice::fGetDefaultDevice( );

		for( u32 i = 0; i < mGeometryPointers.fCount( ); ++i )
		{
			tGeometryPointer& bufferPtr = mGeometryPointers[ i ];

			// compute sys ram buffer pointer
			const Sig::byte* sysRamBuffer = ( const Sig::byte* )this + bufferPtr.mBufferOffset;

			// allocate vram buffer
			bufferPtr.mVRamBuffer.fAllocateInPlace( device, sysRamBuffer );
		}
		for( u32 i = 0; i < mIndexListPointers.fCount( ); ++i )
		{
			tIndexListPointer& bufferPtr = mIndexListPointers[ i ];

			// compute sys ram buffer pointer
			const Sig::byte* sysRamBuffer = ( const Sig::byte* )this + bufferPtr.mBufferOffset;

			// allocate vram buffer
			bufferPtr.mVRamBuffer.fAllocateInPlace( device, sysRamBuffer );
		}

		profile_mem(cProfileMemGeometry, +fComputeStorage( ));
	}

	void tGeometryFile::fResizeAfterLoad( tGenericBuffer* fileBuffer )
	{
		// this is pretty ghetto and not for the faint of heart; ideally
		// the load-in-place serialization system could generalize a solution
		// to relocating memory; since we know we only have a few pointers
		// that need fixing up, this is pretty safe and easy...
		if( mDiscardSysRamBuffers )
		{
			const Sig::byte* initialLocation = fileBuffer->fGetBuffer( );

			fileBuffer->fResize( mHeaderSize );

			const Sig::byte* newLocation = fileBuffer->fGetBuffer( );
			tGeometryFile* newThis = ( tGeometryFile* )newLocation;

			const ptrdiff_t delta = newLocation - initialLocation;

			newThis->fRelocateLoadInPlaceTables( delta );
			newThis->mGeometryPointers.fRelocateInPlace( delta );
			newThis->mIndexListPointers.fRelocateInPlace( delta );

			for( u32 i = 0; i < newThis->mGeometryPointers.fCount( ); ++i )
				newThis->mGeometryPointers[ i ].mVRamBuffer.fRelocateInPlace( delta );
		}
	}

	void tGeometryFile::fOnFileUnloading( )
	{
		profile_mem(cProfileMemGeometry, -fComputeStorage( ));

		for( u32 i = 0; i < mGeometryPointers.fCount( ); ++i )
			mGeometryPointers[ i ].mVRamBuffer.fDeallocateInPlace( );
		for( u32 i = 0; i < mIndexListPointers.fCount( ); ++i )
			mIndexListPointers[ i ].mVRamBuffer.fDeallocateInPlace( );
	}
	s32 tGeometryFile::fComputeStorage( ) const
	{
		u32 total = 0;
		for( u32 i = 0; i < mGeometryPointers.fCount( ); ++i )
			total += mGeometryPointers[ i ].mVRamBuffer.fBufferSize( );
		for( u32 i = 0; i < mIndexListPointers.fCount( ); ++i )
			total += mIndexListPointers[ i ].mVRamBuffer.fBufferSize( );
		return total;
	}
	f32 tGeometryFile::fComputeStorageInMB( ) const
	{
		return fComputeStorage( ) / ( 1024.f * 1024.f );
	}

}}

