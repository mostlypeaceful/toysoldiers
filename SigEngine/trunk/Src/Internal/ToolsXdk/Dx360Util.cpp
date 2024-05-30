#include "ToolsXdkPch.hpp"
#include "Dx360Util.hpp"
#include "EndianUtil.hpp"
#include "Gfx/tDevice.hpp"
#include "d3d9.h"
#include "d3dx9.h"
#include "xgraphics.h"

namespace Sig { namespace Dx360Util
{
	u16 fConvertFloatToHalf( f32 f )
	{
		return XMConvertFloatToHalf( f );
	}
	Math::tVector2<u16> fConvertVec2fToHalf( const Math::tVec2f& f )
	{
		return Math::tVector2<u16>( 
			XMConvertFloatToHalf( f.x ),
			XMConvertFloatToHalf( f.y ) );
	}
	Math::tVector4<u16> fConvertVec3fToHalf( const Math::tVec3f& f )
	{
		return Math::tVector4<u16>( 
			XMConvertFloatToHalf( f.x ),
			XMConvertFloatToHalf( f.y ),
			XMConvertFloatToHalf( f.z ),
			0 );
	}
	Math::tVector4<u16> fConvertVec4fToHalf( const Math::tVec4f& f )
	{
		return Math::tVector4<u16>( 
			XMConvertFloatToHalf( f.x ),
			XMConvertFloatToHalf( f.y ),
			XMConvertFloatToHalf( f.z ),
			XMConvertFloatToHalf( f.w ) );
	}

	f32 fConvertHalfToFloat( u16 h )
	{
		return XMConvertHalfToFloat( h );
	}
	Math::tVec2f fConvertHalfToVec2f( const Math::tVector2<u16>& h )
	{
		return Math::tVec2f( 
			XMConvertHalfToFloat( h.x ), 
			XMConvertHalfToFloat( h.y ) );
	}
	Math::tVec3f fConvertHalfToVec3f( const Math::tVector4<u16>& h )
	{
		return Math::tVec3f( 
			XMConvertHalfToFloat( h.x ), 
			XMConvertHalfToFloat( h.y ),
			XMConvertHalfToFloat( h.z ) );
	}
	Math::tVec4f fConvertHalfToVec4f( const Math::tVector4<u16>& h )
	{
		return Math::tVec4f( 
			XMConvertHalfToFloat( h.x ), 
			XMConvertHalfToFloat( h.y ),
			XMConvertHalfToFloat( h.z ),
			XMConvertHalfToFloat( h.w ) );
	}

	b32 fCopyShaderBuffer( tDynamicBuffer& obuffer, ID3DXBuffer* shBuffer, ID3DXBuffer* shErrors, std::string* errorsOut )
	{
		if( shErrors )
		{
			const char* errmsg = ( const char* )shErrors->GetBufferPointer( );
			b32 failed = (strstr( errmsg, "error" ) != NULL);

			if( errorsOut )
				*errorsOut = errmsg;
			else
				log_warning( "Shader compilation errors:" << std::endl << errmsg );
			shErrors->Release( );

			if( failed )
				return false;
		}

		if( shBuffer )
		{
			obuffer.fNewArray( shBuffer->GetBufferSize( ) );
			fMemCpy( obuffer.fBegin( ), shBuffer->GetBufferPointer( ), obuffer.fCount( ) );
			shBuffer->Release( );
			return true;
		}

		return false;
	}

	b32 fCompileShader( const char* profileName, const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint, std::string* errorsOut )
	{
		if( !entryPoint )
			entryPoint = "main";

		ID3DXBuffer* buffer = 0;
		ID3DXBuffer* errors = 0;
		D3DXCompileShader( 
			hlsl.c_str( ),
			( UINT )hlsl.length( ),
			0,
			0,
			entryPoint,
			profileName,
			0,
			&buffer,
			&errors,
			0 );

		return fCopyShaderBuffer( obuffer, buffer, errors, errorsOut );
	}

	b32 fCompileVertexShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint, std::string* errorsOut )
	{
		const char* profile = "vs_3_0";
		return fCompileShader( profile, hlsl, obuffer, entryPoint, errorsOut );
	}

	b32 fCompilePixelShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint, std::string* errorsOut )
	{
		const char* profile = "ps_3_0";
		return fCompileShader( profile, hlsl, obuffer, entryPoint, errorsOut );
	}
	namespace
	{
		// XDK Docs - 32 tile size
		static const u32 cTileSize = 32;

		D3DFORMAT fConvertFormat( Gfx::tTextureFile::tFormat format )
		{
			D3DFORMAT o = ( D3DFORMAT )0;

			switch( format )
			{
			case Gfx::tTextureFile::cFormatA8R8G8B8: 	o = D3DFMT_A8R8G8B8; break;
			case Gfx::tTextureFile::cFormatDXT1:		o = D3DFMT_DXT1; break;
			case Gfx::tTextureFile::cFormatDXT3:		o = D3DFMT_DXT3;break;
			case Gfx::tTextureFile::cFormatDXT5:		o = D3DFMT_DXT5; break;
			case Gfx::tTextureFile::cFormatR5G6B5:		o = D3DFMT_R5G6B5; break;
			case Gfx::tTextureFile::cFormatA8:			o = D3DFMT_A8; break;
			default:									sigassert( !"un-recognized texture format!" ); break;
			}

			return o;
		}

		inline u32 fMipSize( u32 size, u32 imip )
		{
			return fMax( 1u, size >> imip );
		}

		inline u32 fSourcePitch( u32 bufferSize, u32 baseHeight, u32 imip, D3DFORMAT d3dformat )
		{
			b32 isDxt = false;
			switch( d3dformat ) {
				case D3DFMT_DXT1:
				case D3DFMT_DXT3:
				case D3DFMT_DXT5:
				case D3DFMT_LIN_DXT1:
				case D3DFMT_LIN_DXT3:
				case D3DFMT_LIN_DXT5: isDxt = true;
			}

			u32 mipHeight = fMipSize( baseHeight, imip );
			if( isDxt )
				mipHeight = XGNextMultiple( mipHeight, 4 );

			u32 srcPitch = bufferSize / mipHeight;
			if( isDxt )
				srcPitch *= 4;
			
			return srcPitch;
		}

		struct tTextureDesc
		{
			u32 mWidth;
			u32 mHeight;
			u32 mRowPitch;
			u32 mSlicePitch;
			u32 mWidthInBlocks;
			u32 mHeightInBlocks;
			u32 mBytesPerBlock;
			u32 mWidthAsAllocated;
			u32 mHeightAsAllocated;
		};

		void fGetTextureDesc( 
			u32 mip, u32 width, u32 height, D3DFORMAT d3dFormat, tTextureDesc* desc )
		{
			UINT blockX, blockY;
			XGGetBlockDimensions( XGGetGpuFormat( d3dFormat ), &blockX, &blockY );

			desc->mWidth = fMipSize( width, mip );
			desc->mHeight = fMipSize( height, mip );

			desc->mBytesPerBlock = ( XGBitsPerPixelFromFormat( d3dFormat ) * blockX * blockY ) / 8;
			desc->mWidthInBlocks = XGNextMultiple( desc->mWidth, blockX ) / blockX;
			desc->mHeightInBlocks = XGNextMultiple( desc->mHeight, blockY ) / blockY;

			u32 alignedBlockWidth, alignedBlockHeight;
			if( mip )
			{
				alignedBlockWidth = fMipSize( Math::fCeilingPow2( width ), mip ) / blockX;
				alignedBlockHeight = fMipSize( Math::fCeilingPow2( height ), mip ) / blockY;

				alignedBlockWidth = XGNextMultiple( alignedBlockWidth, cTileSize );
				alignedBlockHeight = XGNextMultiple( alignedBlockHeight, cTileSize );
			}
			else
			{
				alignedBlockWidth = XGNextMultiple( desc->mWidthInBlocks, cTileSize );
				alignedBlockHeight = XGNextMultiple( desc->mHeightInBlocks, cTileSize );
			}

			desc->mWidthAsAllocated = alignedBlockWidth * blockX;
			desc->mHeightAsAllocated = alignedBlockHeight * blockY;

			const u32 cPageSize = 4096; // XDK Docs allocations for textures are minimum 4096
			desc->mSlicePitch = XGNextMultiple
				( desc->mBytesPerBlock * alignedBlockWidth * alignedBlockHeight, cPageSize );
			desc->mRowPitch = desc->mBytesPerBlock * alignedBlockWidth;
		}
	}

	b32 fTextureConversionWantsPreConvertedMips( )
	{
		return true;
	}

	void fOutputMipChain( Gfx::tTextureFile& texFile, tRawFaceMipMapSet& rawFaceMips, tFileWriter& ofile )
	{
		const D3DFORMAT tiledD3dFormat = fConvertFormat( texFile.mFormat );
		const D3DFORMAT untiledD3dFormat = (D3DFORMAT)(tiledD3dFormat & ~D3DFORMAT_TILED_MASK);

		const DWORD tiledGpuFormat = XGGetGpuFormat( tiledD3dFormat );
		const DWORD untiledGpuFormat = XGGetGpuFormat( untiledD3dFormat );

		u32 offsetFromStartOfFile = texFile.mHeaderSize;
		for( u32 iface = 0; iface < rawFaceMips.fCount( ); ++iface )
		{
			const u32 mipCount = rawFaceMips[ iface ].fCount( );
			const u32 packMipStart = XGGetMipTailBaseLevel( texFile.mWidth, texFile.mHeight, FALSE );
			const u32 unpackedMipCount = fMin( packMipStart, mipCount );

			for( u32 imip = 0; imip < unpackedMipCount; ++imip )
			{
				// Compute pitch for src data
				const tDynamicBuffer& srcMipData = rawFaceMips[ iface ][ imip ];
				const u32 srcPitchInBytes = fSourcePitch
					( srcMipData.fCount( ), texFile.mHeight, imip, untiledD3dFormat );

				// Gather xbox storage info
				tTextureDesc desc;
				fGetTextureDesc
					( imip
					, texFile.mWidth
					, texFile.mHeight
					, tiledD3dFormat
					, &desc );

				// Allocate our buffer
				tDynamicBuffer mipData( desc.mSlicePitch );

				// Tile the source data into the final buffer
				XGTileTextureLevel
					( texFile.mWidth
					, texFile.mHeight
					, imip
					, tiledGpuFormat
					, 0
					, mipData.fBegin( )
					, NULL
					, srcMipData.fBegin( )
					, srcPitchInBytes
					, NULL );

				// Endian swap the surface
				XGEndianSwapSurface
					( mipData.fBegin( )
					, desc.mRowPitch
					, mipData.fBegin( )
					, desc.mRowPitch
					, desc.mWidthAsAllocated
					, desc.mHeightAsAllocated
					, tiledD3dFormat );


				// store current file offset
				texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferOffset = offsetFromStartOfFile;

				// store buffer size
				texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize = mipData.fCount( );

				// write mip buffer
				ofile( mipData.fBegin( ), mipData.fCount( ) );

				// increment current file offset
				offsetFromStartOfFile += mipData.fCount( );
			}

			// Handle the packed mip tail
			if( packMipStart < mipCount )
			{
				tTextureDesc desc;
				fGetTextureDesc
					( packMipStart
					, texFile.mWidth
					, texFile.mHeight
					, tiledD3dFormat
					, &desc );

				tDynamicBuffer mipData( desc.mSlicePitch );

				for( u32 imip = packMipStart; imip < mipCount; ++imip )
				{

					// Set all packed mip levels to clearly invalid values
					texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferOffset = 0;
					texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize = 0;

					const u32 byteOffsetToMip = XGGetMipTailLevelOffset
						( texFile.mWidth
						, texFile.mHeight
						, 0
						, imip
						, tiledGpuFormat
						, TRUE
						, FALSE );

					const tDynamicBuffer& srcMipData = rawFaceMips[ iface ][ packMipStart ];

					// Compute pitch for src data
					const u32 srcPitchInBytes = fSourcePitch
						( srcMipData.fCount( ), texFile.mHeight, imip, untiledD3dFormat );

					XGTileTextureLevel
						( texFile.mWidth
						, texFile.mHeight
						, imip
						, tiledGpuFormat
						, 0
						, mipData.fBegin( ) + byteOffsetToMip
						, NULL
						, srcMipData.fBegin( )
						, srcPitchInBytes
						, NULL );
				}
				
				XGEndianSwapSurface
					( mipData.fBegin( )
					, desc.mRowPitch
					, mipData.fBegin( )
					, desc.mRowPitch
					, desc.mWidthAsAllocated
					, desc.mHeightAsAllocated
					, tiledD3dFormat );

				// store current file offset in the packed mip start entry
				texFile.mImages[ iface ].mMipMapBuffers[ packMipStart ].mBufferOffset = offsetFromStartOfFile;

				// store buffer size in the packed mip start entry
				texFile.mImages[ iface ].mMipMapBuffers[ packMipStart ].mBufferSize = mipData.fCount( );

				// write mip buffer
				ofile( mipData.fBegin( ), mipData.fCount( ) );

				// increment current file offset
				offsetFromStartOfFile += mipData.fCount( );
			}
		}

	}

}}

