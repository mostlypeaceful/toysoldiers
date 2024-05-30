#ifndef __tHlslOutput__
#define __tHlslOutput__
#include "tStrongPtr.hpp"
#include "Dx9Util.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tShadeMaterial.hpp"
#include "Gfx/tVertexFormatVRam.hpp"

namespace Sig { namespace HlslGen
{
	enum tHlslPlatformId
	{
		cPidDefault,
		cPidPcDx9,
		cPidPcDx10,
		cPidXbox360,
		cPidPs3,
		// last
		cPidCount
	};
	enum tVshStyle
	{
		cVshMeshModel,
		cVshFacingQuads,
		// last
		cVshStyleCount
	};
	enum tWriteMode
	{
		cWriteModeColor,
		cWriteModeDepth,
		cWriteModeDepthWithAlpha,
		// last
		cWriteModeCount
	};
	enum tToolType
	{
		cToolTypeDefault,
		cToolTypeMaya,
	};
	struct tShaderOutputBase
	{
		std::string mName; // to be used for error reporting, hlsl output, etc.; nothing fancy, just gives a brief summary of the shader in the form of a filename (i.e., shader.vsh or shader.psh)
		std::string mHlsl;
		std::string mErrors;
		Gfx::tShadeMaterialGluePtrArray mGlueShared;
		Gfx::tShadeMaterialGluePtrArray mGlueInstance;
		void fAddMaterialGlue( const Gfx::tShadeMaterialGluePtr& glue );
		void fWriteToFile( const tFilePathPtr& folder ) const;
	};
	struct tVertexShaderOutput : public tShaderOutputBase
	{
		typedef tStrongPtr<Gfx::tVertexFormatVRam> tVertexFormatVRamPtr;
		tVertexFormatVRamPtr mVtxFormat;
		Dx9Util::tVertexShaderPtr mSh;
	};
	struct tPixelShaderOutput : public tShaderOutputBase
	{
		Dx9Util::tPixelShaderPtr mSh;
	};
	struct tHlslInput
	{
		tHlslPlatformId mPid;
		tVshStyle mVshStyle;
		b32 mGenDepthShaders;
		b32 mDXT5NNormalMaps;
		b32 mPreviewMode; // used in preview window and maya (i.e., material editor tools)
		tToolType mToolType;
		explicit tHlslInput( tHlslPlatformId pid = cPidDefault, tVshStyle vshStyle = cVshMeshModel, b32 genDepthShaders = true, b32 dxtn5 = true, b32 previewMode = false, tToolType toolType = cToolTypeDefault ) 
			: mPid( pid ), mVshStyle( vshStyle ), mGenDepthShaders( genDepthShaders ), mDXT5NNormalMaps( dxtn5 ), mPreviewMode( previewMode ), mToolType( toolType ) { }
		tPlatformId fRealPlatformId( ) const;
		b32 fSupportsInstancing( ) const;
	};
	struct tools_export tHlslOutput
	{
		Gfx::tShadeMaterialGlueValues						mMaterialGlueValues;

		//This list should match tShadeMaterial.hpp's tShadeMaterial::tShaderSlot
		tFixedArray<tVertexShaderOutput,cWriteModeCount>	mStaticVShaders;
		tFixedArray<tVertexShaderOutput,cWriteModeCount>	mStaticVShaders_Instanced;
		tFixedArray<tVertexShaderOutput,cWriteModeCount>	mSkinnedVShaders;
		tFixedArray<tVertexShaderOutput,cWriteModeCount>	mSkinnedVShaders_Instanced;
		tDynamicArray<tPixelShaderOutput>					mColorPShaders;
		tDynamicArray<tPixelShaderOutput>					mColorShadowPShaders;
		tPixelShaderOutput									mDepthOnlyPShader;
		tPixelShaderOutput									mDepthWithAlphaPShader;
		tVertexShaderOutput									mGBufferStaticVShader;
		tVertexShaderOutput									mGBufferStaticVShader_Instanced;
		tVertexShaderOutput									mGBufferSkinnedVShader;
		tVertexShaderOutput									mGBufferSkinnedVShader_Instanced;
		tPixelShaderOutput									mGBufferPShader;
		tFixedArray<tVertexShaderOutput,cWriteModeCount>	mStaticVShaders_DP;
		tFixedArray<tVertexShaderOutput,cWriteModeCount>	mStaticVShaders_DP_Instanced;
		tFixedArray<tVertexShaderOutput,cWriteModeCount>	mSkinnedVShaders_DP;
		tFixedArray<tVertexShaderOutput,cWriteModeCount>	mSkinnedVShaders_DP_Instanced;
		tPixelShaderOutput									mDepthOnlyPShader_DP;
		tPixelShaderOutput									mDepthWithAlphaPShader_DP;

		explicit tHlslOutput( u32 numColorPShaders = 0 );
		void fWriteShadersToFile( const tFilePathPtr& relDermlPath ) const;
		b32  fCreateDx9Shaders( const Gfx::tDevicePtr& device );
	};
}}

#endif//__tHlslOutput__
