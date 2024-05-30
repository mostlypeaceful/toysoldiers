#ifndef __Dx11Util__
#define __Dx11Util__
#include "Gfx/tDevice.hpp"
//#include <d3d11.h>

#define define_dxinterface_ptr( exportName, nameSpace, dxClass, simpleClassName ) \
	namespace Sig \
	{ \
	namespace nameSpace { typedef dxClass simpleClassName;  } \
	template<> inline void fRefCounterPtrAddRef( dxClass* p ) { p->AddRef( ); } \
	template<> inline void fRefCounterPtrDecRef( dxClass* p ) { p->Release( ); } \
	template<> inline u32 fRefCounterPtrRefCount( dxClass* p ) { p->AddRef( ); return p->Release( ); } \
	namespace nameSpace { define_smart_ptr( exportName, tRefCounterPtr, simpleClassName );  } \
	}

//define_dxinterface_ptr( tools_export, Dx11Util, IDirect3DVertexDeclaration9, tVertexDecl )
//define_dxinterface_ptr( tools_export, Dx11Util, ID3D11VertexShader, tVertexShader )
//define_dxinterface_ptr( tools_export, Dx11Util, ID3D11PixelShader, tPixelShader )
//define_dxinterface_ptr( tools_export, Dx11Util, IDirect3DBaseTexture9, tBaseTexture )
//define_dxinterface_ptr( tools_export, Dx11Util, ID3D11Texture2D, tTexture2D )
//define_dxinterface_ptr( tools_export, Dx11Util, IDirect3DCubeTexture9, tTextureCube )

namespace Sig { namespace Dx11Util
{
	tools_export b32 fCopyShaderBuffer( tDynamicBuffer& obuffer, ID3DXBuffer* shBuffer, ID3DXBuffer* shErrors, std::string* errorsOut=0 );
	tools_export b32 fCompileShader( const char* profileName, const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );
	tools_export b32 fCompileVertexShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );
	tools_export b32 fCompilePixelShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );
}}


#endif//__Dx9Util__
