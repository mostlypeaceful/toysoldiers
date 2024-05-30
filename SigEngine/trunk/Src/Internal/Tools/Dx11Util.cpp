#include "ToolsPch.hpp"
#include "Dx9Util.hpp"
#include "Dx11Util.hpp"
#include <d3d11.h>

namespace Sig { namespace Dx11Util
{

	b32 fCopyShaderBuffer( tDynamicBuffer& obuffer, ID3DXBuffer* shBuffer, ID3DXBuffer* shErrors, std::string* errorsOut )
	{
		return Dx9Util::fCopyShaderBuffer( obuffer, shBuffer, shErrors, errorsOut );
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
			D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, // allow our 3.0 shaders to compile against DX11 4.0 targets.
			&buffer,
			&errors,
			0 );

		return fCopyShaderBuffer( obuffer, buffer, errors, errorsOut );
	}

	b32 fCompileVertexShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint, std::string* errorsOut )
	{
		const char* profile = "vs_4_0";
		return fCompileShader( profile, hlsl, obuffer, entryPoint, errorsOut );
	}

	b32 fCompilePixelShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint, std::string* errorsOut )
	{
		const char* profile = "ps_4_0";
		return fCompileShader( profile, hlsl, obuffer, entryPoint, errorsOut );
	}
}}
