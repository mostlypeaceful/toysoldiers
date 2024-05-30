#include "BasePch.hpp"
#include "tMaterialFile.hpp"
#include "tDevice.hpp"


// HAXOR/WORKAROUND linker won't link anything from DerivedShadeMaterialGlue.hpp unless we do this
#include "DerivedShadeMaterialGlue.hpp"
namespace Sig { namespace Gfx
{
	register_rtti_factory( tVsGlueModelToWorld, true ); // HAXOR/WORKAROUND linker won't link anything from DerivedShadeMaterialGlue.hpp unless we do this
}}

namespace Sig { namespace Gfx
{

	void tShadeMaterialGlue::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		sigassert( !"shouldn't be here (tShadeMaterialGlue::fApplyShared)" );
	}
	void tShadeMaterialGlue::fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		sigassert( !"shouldn't be here (tShadeMaterialGlue::fApplyInstance)" );
	}

	const u32 tMaterialFile::cVersion = 2;

	const char* tMaterialFile::fGetFileExtension( )
	{
		return ".mtlb";
	}

	tMaterialFile::tMaterialFile( )
		: mMaterialCid( Rtti::cInvalidClassId )
		, mHeaderSize( 0 )
		, mDiscardShaderBuffers( false )
	{
	}

	tMaterialFile::tMaterialFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mShaderLists( cNoOpTag )
	{
	}

	void tMaterialFile::fOnFileLoaded( const tResource& ownerResource )
	{
		tDevicePtr device = tDevice::fGetDefaultDevice( );

		for( u32 i = 0; i < mShaderLists.fCount( ); ++i )
		{
			for( u32 j = 0; j < mShaderLists[ i ].fCount( ); ++j )
			{
				tShaderPointer& shPtr = mShaderLists[ i ][ j ];
				if( shPtr.mBufferSize == 0 )
					continue;
				const Sig::byte* shaderBuffer = ( const Sig::byte* )this + shPtr.mBufferOffset;
				fCreateShaderPlatformSpecific( device, shPtr, shaderBuffer );
			}
		}
	}

	void tMaterialFile::fResizeAfterLoad( tGenericBuffer* fileBuffer )
	{
		// this is pretty ghetto and not for the faint of heart; ideally
		// the load-in-place serialization system could generalize a solution
		// to relocating memory; since we know we only have a few pointers
		// that needs fixing up, this is pretty safe and easy...
		if( mDiscardShaderBuffers )
		{
			const Sig::byte* initialLocation = fileBuffer->fGetBuffer( );

			fileBuffer->fResize( mHeaderSize );

			const Sig::byte* newLocation = fileBuffer->fGetBuffer( );
			tMaterialFile* newThis = ( tMaterialFile* )newLocation;

			const ptrdiff_t delta = newLocation - initialLocation;

			newThis->fRelocateLoadInPlaceTables( delta );
			newThis->mShaderLists.fRelocateInPlace( delta );
			for( u32 i = 0; i < newThis->mShaderLists.fCount( ); ++i )
			{
				tShaderList& shaderList = newThis->mShaderLists[ i ];
				shaderList.fRelocateInPlace( delta );

				for( u32 j = 0; j < shaderList.fCount( ); ++j )
				{
					tShadeMaterialGlueArray& glueShared   = shaderList[ j ].mGlueShared;
					tShadeMaterialGlueArray& glueInstance = shaderList[ j ].mGlueInstance;

					glueShared.  fRelocateInPlace( delta );
					glueInstance.fRelocateInPlace( delta );

					for( u32 k = 0; k < glueShared.fCount( ); ++k )
						glueShared[ k ].fRelocateInPlace( delta );
					for( u32 k = 0; k < glueInstance.fCount( ); ++k )
						glueInstance[ k ].fRelocateInPlace( delta );
				}
			}
		}
	}

	void tMaterialFile::fOnFileUnloading( )
	{
		for( u32 i = 0; i < mShaderLists.fCount( ); ++i )
			for( u32 j = 0; j < mShaderLists[ i ].fCount( ); ++j )
				fDestroyShaderPlatformSpecific( mShaderLists[ i ][ j ] );
	}


}}
