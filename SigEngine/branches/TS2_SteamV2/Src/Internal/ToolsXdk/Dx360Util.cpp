#include "ToolsXdkPch.hpp"
#include "Dx360Util.hpp"
#include "EndianUtil.hpp"
#include "d3d9.h"
#include "d3dx9.h"
#include "xgraphics.h"

#if _MSC_VER>=1900
#include "stdio.h" 
_ACRTIMP_ALT FILE* __cdecl __acrt_iob_func(unsigned);
#ifdef __cplusplus 
extern"C"
#endif 
FILE* __cdecl __iob_func(unsigned i) {
	return __acrt_iob_func(i);
}
#endif /* _MSC_VER>=1900 */


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

	b32 fCopyShaderBuffer( tDynamicBuffer& obuffer, ID3DXBuffer* shBuffer, ID3DXBuffer* shErrors, std::string* errorsOut )
	{
		if( shErrors )
		{
			const char* errmsg = ( const char* )shErrors->GetBufferPointer( );
			if( errorsOut )
				*errorsOut = errmsg;
			else
				log_warning( 0, "Shader compilation errors:" << std::endl << errmsg );
			shErrors->Release( );
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

#define ghetto_style 1

	b32 fTextureConversionWantsPreConvertedMips( )
	{
		return ghetto_style;
	}

#ifndef ghetto_style
	namespace
	{
		D3DFORMAT fConvertFormat( Gfx::tTextureFile::tFormat format )
		{
			D3DFORMAT o = ( D3DFORMAT )0;
			switch( format )
			{
			case Gfx::tTextureFile::cFormatA8R8G8B8:		o = D3DFMT_A8R8G8B8; break;
			case Gfx::tTextureFile::cFormatDXT1:			o = D3DFMT_DXT1; break;
			case Gfx::tTextureFile::cFormatDXT3:			o = D3DFMT_DXT3; break;
			case Gfx::tTextureFile::cFormatDXT5:			o = D3DFMT_DXT5; break;
			case Gfx::tTextureFile::cFormatR5G6B5:			o = D3DFMT_R5G6B5; break;
			case Gfx::tTextureFile::cFormatA8:				o = D3DFMT_A8; break;
			default:								sigassert( !"un-recognized texture format!" ); break;
			}
			return o;
		}
	}
#endif//ghetto_style

	void fOutputMipChain( Gfx::tTextureFile& texFile, tRawFaceMipMapSet& rawFaceMips, tFileWriter& ofile )
	{
#ifdef ghetto_style

		u32 offsetFromStartOfFile = texFile.mHeaderSize;
		for( u32 iface = 0; iface < rawFaceMips.fCount( ); ++iface )
		{
			for( u32 imip = 0; imip < rawFaceMips[ iface ].fCount( ); ++imip )
			{
				tDynamicBuffer& mipData = rawFaceMips[ iface ][ imip ];

				// store current file offset
				texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferOffset = offsetFromStartOfFile;

				// store buffer size (again, just in case)
				texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize = mipData.fCount( );

				switch( texFile.mFormat )
				{
				case Gfx::tTextureFile::cFormatA8R8G8B8:
					EndianUtil::fSwap32( mipData.fBegin( ), mipData.fCount( ) / 4 );
					break;
				case Gfx::tTextureFile::cFormatDXT1:
					EndianUtil::fSwap16( mipData.fBegin( ), mipData.fCount( ) / 2 );
					break;
				case Gfx::tTextureFile::cFormatDXT3:
					EndianUtil::fSwap16( mipData.fBegin( ), mipData.fCount( ) / 2 );
					break;
				case Gfx::tTextureFile::cFormatDXT5:
					EndianUtil::fSwap16( mipData.fBegin( ), mipData.fCount( ) / 2 );
					break;
				case Gfx::tTextureFile::cFormatR5G6B5:
					EndianUtil::fSwap16( mipData.fBegin( ), mipData.fCount( ) / 2 );
					break;
				case Gfx::tTextureFile::cFormatA8:
					EndianUtil::fSwap8( mipData.fBegin( ), mipData.fCount( ) / 1 );
					break;
				}

				// write mip buffer
				ofile( mipData.fBegin( ), mipData.fCount( ) );

				// increment current file offset
				offsetFromStartOfFile += mipData.fCount( );
			}
		}

#else//ghetto_style

		// This version is still under construction; need to get enough data to be able to call XGCopySurface,
		// and then beyond that we want to call XGTileTextureLevel, and do packed mip chains, etc.

		const D3DFORMAT srcFormat = D3DFMT_LIN_A8R8G8B8;
		const D3DFORMAT dstFormat = ( D3DFORMAT )( fConvertFormat( texFile.mFormat ) & ~D3DFORMAT_TILED_MASK );

		u32 offsetFromStartOfFile = texFile.mHeaderSize;
		for( u32 iface = 0; iface < rawFaceMips.fCount( ); ++iface )
		{
			u32 width	= texFile.mWidth;
			u32 height	= texFile.mHeight;

			for( u32 imip = 0; imip < rawFaceMips[ iface ].fCount( ); ++imip )
			{
				const tDynamicBuffer& srcMipData = rawFaceMips[ iface ][ imip ];

				tDynamicBuffer dstMipData;

				const u32 dstPitch  = 0;
				const u32 dstWidth  = width;
				const u32 dstHeight = height;
				const u32 srcPitch	= width;
				const u32 flags		= 0;
				const f32 alpha		= 0.f;
				XGCopySurface( dstMipData.fBegin( ), dstPitch, dstWidth, dstHeight, dstFormat, 0, srcMipData.fBegin( ), srcPitch, srcFormat, 0, flags, alpha );

				// store current file offset
				texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferOffset = offsetFromStartOfFile;

				// store buffer size (again, just in case)
				texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize = dstMipData.fCount( );

				// write mip buffer
				ofile( dstMipData.fBegin( ), dstMipData.fCount( ) );

				// increment current file offset
				offsetFromStartOfFile += dstMipData.fCount( );
			}

			width	= fMax( 1u, width  / 2 );
			height	= fMax( 1u, height / 2 );
		}
#endif//ghetto_style
	}

}}

