#ifndef __Dx9Util__
#define __Dx9Util__
#include "Gfx/tDevice.hpp"
#include "Gfx/tSphericalHarmonics.hpp"

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
	tools_export void fAssembleCubeMapFromTextures( const tFixedArray<IDirect3DTexture9*,6>& textures, IDirect3DTexture9*& output );

	class tools_export tTextureCache : public tRefCounter
	{
	private:
		struct tEntry
		{
			tBaseTexturePtr		mTexPtr;
			u64					mTimeStamp;
			b32					mCubeMap;
			Math::tVec4f		mAtlasInfo; // num tex X, num tex Y, tile dim X, tile dim Y

			tEntry( ) : mTimeStamp( 0 ), mCubeMap( false ) { }
			tEntry( const tBaseTexturePtr& texPtr, u64 ts, b32 cubeMap = false ) : mTexPtr( texPtr ), mTimeStamp( ts ), mCubeMap( cubeMap ) { }
		};

		typedef tHashTable<tFilePathPtr,tEntry> tEntryTable;

	private:
		Gfx::tDevicePtr mDevice;
		tEntryTable mEntries;
		tBaseTexturePtr mWhiteTexture;
		tBaseTexturePtr mBlackTexture;
		tBaseTexturePtr mBlankNormals;
		u32 mDirtyStamp;

	public:
		explicit tTextureCache( const Gfx::tDevicePtr& device );
		~tTextureCache( );

		tBaseTexturePtr fFindLoad2D( const tFilePathPtr& path );
		tBaseTexturePtr fFindLoadCube( const tFilePathPtr& path );
		tBaseTexturePtr fFindLoadAtlas( const tFilePathPtr& path, Math::tVec4f & info );

		static Math::tVec2f fTextureDims2D( tBaseTexture& tex );

		const tBaseTexturePtr& fWhiteTexture( ) const { return mWhiteTexture; }
		const tBaseTexturePtr& fBlackTexture( ) const { return mBlackTexture; }
		const tBaseTexturePtr& fBlankNormalsTexture( ) const { return mBlankNormals; }

		void fRefresh( );
		u32 fDirtyStamp( ) const { return mDirtyStamp; }

		static void fComputeSphericalHarmonics( tBaseTexturePtr& input, Gfx::tSphericalHarmonics& output, const Gfx::tShBasisWeights& weights );

	private:
		tBaseTexturePtr fLoad( const tFilePathPtr& path, b32 cubeMap ) const;
		tBaseTexturePtr fLoadAtlas( const tFilePathPtr& path, Math::tVec4f & info ) const;

	};

	define_smart_ptr( tools_export, tRefCounterPtr, tTextureCache );
}}


#endif//__Dx9Util__
