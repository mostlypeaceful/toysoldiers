#ifndef __Dx360Util__
#define __Dx360Util__
#include "tFileWriter.hpp"
#include "Gfx/tTextureFile.hpp"

namespace Sig { namespace Dx360Util
{
	typedef tDynamicArray< tDynamicArray< tDynamicBuffer > > tRawFaceMipMapSet;

	toolsxdk_export u16 fConvertFloatToHalf( f32 f );
	toolsxdk_export Math::tVector2<u16> fConvertVec2fToHalf( const Math::tVec2f& f );
	toolsxdk_export Math::tVector4<u16> fConvertVec3fToHalf( const Math::tVec3f& f );
	toolsxdk_export Math::tVector4<u16> fConvertVec4fToHalf( const Math::tVec4f& f );

	toolsxdk_export b32 fCopyShaderBuffer( tDynamicBuffer& obuffer, ID3DXBuffer* shBuffer, ID3DXBuffer* shErrors, std::string* errorsOut=0 );
	toolsxdk_export b32 fCompileShader( const char* profileName, const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );
	toolsxdk_export b32 fCompileVertexShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );
	toolsxdk_export b32 fCompilePixelShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );

	toolsxdk_export b32  fTextureConversionWantsPreConvertedMips( );
	toolsxdk_export void fOutputMipChain( Gfx::tTextureFile& texFile, tRawFaceMipMapSet& rawFaceMips, tFileWriter& ofile );
}}

#endif//__Dx360Util__
