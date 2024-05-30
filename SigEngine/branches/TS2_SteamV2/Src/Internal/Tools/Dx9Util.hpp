#ifndef __Dx9Util__
#define __Dx9Util__
#include "Gfx/tDevice.hpp"

#define define_dxinterface_ptr( exportName, nameSpace, dxClass, simpleClassName ) \
	namespace Sig \
	{ \
	namespace nameSpace { typedef dxClass simpleClassName;  } \
	template<> inline void fRefCounterPtrAddRef( dxClass* p ) { p->AddRef( ); } \
	template<> inline void fRefCounterPtrDecRef( dxClass* p ) { p->Release( ); } \
	template<> inline u32 fRefCounterPtrRefCount( dxClass* p ) { p->AddRef( ); return p->Release( ); } \
	namespace nameSpace { define_smart_ptr( exportName, tRefCounterPtr, simpleClassName );  } \
	}

define_dxinterface_ptr( tools_export, Dx9Util, IDirect3DVertexDeclaration9, tVertexDecl )
define_dxinterface_ptr( tools_export, Dx9Util, IDirect3DVertexShader9, tVertexShader )
define_dxinterface_ptr( tools_export, Dx9Util, IDirect3DPixelShader9, tPixelShader )
define_dxinterface_ptr( tools_export, Dx9Util, IDirect3DBaseTexture9, tBaseTexture )
define_dxinterface_ptr( tools_export, Dx9Util, IDirect3DTexture9, tTexture2D )
define_dxinterface_ptr( tools_export, Dx9Util, IDirect3DCubeTexture9, tTextureCube )

namespace Sig { namespace Dx9Util
{
	tools_export b32 fCopyShaderBuffer( tDynamicBuffer& obuffer, ID3DXBuffer* shBuffer, ID3DXBuffer* shErrors, std::string* errorsOut=0 );
	tools_export b32 fCompileShader( const char* profileName, const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );
	tools_export b32 fCompileVertexShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );
	tools_export b32 fCompilePixelShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint=0, std::string* errorsOut=0 );
	tools_export tVertexShaderPtr fCreateVertexShader( const Gfx::tDevicePtr& device, const std::string& hlsl, const char* entryPoint=0, std::string* errorsOut=0 );
	tools_export tPixelShaderPtr fCreatePixelShader( const Gfx::tDevicePtr& device, const std::string& hlsl, const char* entryPoint=0, std::string* errorsOut=0 );


	class tools_export tTextureCache : public tRefCounter
	{
	public:
		struct tEntry
		{
			tBaseTexturePtr		mTexPtr;
			u64					mTimeStamp;
			b32					mCubeMap;
			tEntry( ) : mTimeStamp( 0 ), mCubeMap( false ) { }
			tEntry( const tBaseTexturePtr& texPtr, u64 ts, b32 cubeMap = false ) : mTexPtr( texPtr ), mTimeStamp( ts ), mCubeMap( cubeMap ) { }
		};
		typedef tHashTable<tFilePathPtr,tEntry> tEntryTable;
	private:
		Gfx::tDevicePtr mDevice;
		tEntryTable mEntries;
		tBaseTexturePtr mWhiteTexture, mBlackTexture;
		u32 mDirtyStamp;
	public:
		explicit tTextureCache( const Gfx::tDevicePtr& device );
		~tTextureCache( );
		tBaseTexturePtr fFindLoad2D( const tFilePathPtr& path );
		tBaseTexturePtr fFindLoadCube( const tFilePathPtr& path );
		const tBaseTexturePtr& fWhiteTexture( ) const { return mWhiteTexture; }
		const tBaseTexturePtr& fBlackTexture( ) const { return mBlackTexture; }
		void fRefresh( );
		u32 fDirtyStamp( ) const { return mDirtyStamp; }
	private:
		tBaseTexturePtr fLoad( const tFilePathPtr& path, b32 cubeMap ) const;
	};
	define_smart_ptr( tools_export, tRefCounterPtr, tTextureCache );
}}


#endif//__Dx9Util__
