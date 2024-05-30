#include "ToolsPch.hpp"
#include "tHlslOutput.hpp"
#include "tFileWriter.hpp"

namespace Sig { namespace HlslGen
{

	void tShaderOutputBase::fAddMaterialGlue( const Gfx::tShadeMaterialGluePtr& glue )
	{
		Gfx::tShadeMaterialGluePtrArray& glueArray = glue->fIsShared( ) ? mGlueShared : mGlueInstance;
		u32 entryWithSameType = glue->fIsUnique( ) ? 0 : glueArray.fCount( );
		for( ; entryWithSameType < glueArray.fCount( ); ++entryWithSameType )
			if( glueArray[ entryWithSameType ]->fClassId( ) == glue->fClassId( ) )
				break;
		if( entryWithSameType < glueArray.fCount( ) )
			glueArray[ entryWithSameType ].fReset( glue->fClone( ) );
		else
			glueArray.fPushBack( Gfx::tShadeMaterialGluePtr( glue->fClone( ) ) );
	}

	void tShaderOutputBase::fWriteToFile( const tFilePathPtr& folder ) const
	{
		sigassert( !folder.fNull( ) );
		tFileWriter o( tFilePathPtr::fConstructPath( folder, tFilePathPtr( mName ) ) );
		o( mHlsl.c_str( ), ( u32 )mHlsl.length( ) );

		if( mErrors.length( ) > 0 )
		{
			std::stringstream ss;
			ss << std::endl << std::endl;
			ss << "/*" << std::endl << std::endl;
			ss << "!Compilation Errors!" << std::endl;
			ss << mErrors << std::endl;
			ss << "*/" << std::endl;
			const std::string errorsText = ss.str( );
			o( errorsText.c_str( ), ( u32 )errorsText.length( ) );
		}
		else
		{
			std::stringstream ss;
			ss << std::endl << std::endl;
			ss << "/*" << std::endl;
			ss << "Shader compiled successfully..." << std::endl;
			ss << "*/" << std::endl;
			const std::string successText = ss.str( );
			o( successText.c_str( ), ( u32 )successText.length( ) );
		}
	}

	tPlatformId tHlslInput::fRealPlatformId( ) const
	{
		switch( mPid )
		{
		case cPidDefault:
		case cPidPcDx9:
			return cPlatformPcDx9;
		case cPidPcDx10:
			return cPlatformPcDx10;
		case cPidXbox360:
			return cPlatformXbox360;
		case cPidPs3:
			return cPlatformPs3Ppu;
		}

		return cPlatformPcDx9;
	}

	b32 tHlslInput::fSupportsInstancing( ) const
	{
		return mPid == cPidXbox360;
	}

	tHlslOutput::tHlslOutput( u32 numColorPShaders )
		: mColorPShaders( numColorPShaders )
		, mColorShadowPShaders( numColorPShaders )
	{
		const std::string modeNames[]= { "color", "depthonly", "depthalpha" };
		static_assert( array_length( modeNames ) == cWriteModeCount );

		for( u32 i = 0; i < cWriteModeCount; ++i )
		{
			mStaticVShaders[ i ].mName = modeNames[ i ] + "_static.vsh";
			mStaticVShaders_Instanced[ i ].mName = modeNames[ i ] + "_static_instanced.vsh";
			mSkinnedVShaders[ i ].mName = modeNames[ i ] + "_skin.vsh";
			mSkinnedVShaders_Instanced[ i ].mName = modeNames[ i ] + "_skin_instanced.vsh";
		}
		for( u32 i = 0; i < mColorPShaders.fCount( ); ++i )
		{
			std::stringstream fileName;
			fileName << "color_" << i << "lights.psh";
			mColorPShaders[ i ].mName = fileName.str( );
		}
		for( u32 i = 0; i < mColorShadowPShaders.fCount( ); ++i )
		{
			std::stringstream fileName;
			fileName << "colorshadow_" << i << "lights.psh";
			mColorShadowPShaders[ i ].mName = fileName.str( );
		}
		{
			mDepthOnlyPShader.mName = "depthonly.psh";
		}
		{
			mDepthWithAlphaPShader.mName = "depthalpha.psh";
		}
		{
			mGBufferStaticVShader.mName = "gbuffer_static.vsh";
			mGBufferStaticVShader_Instanced.mName = "gbuffer_static_instanced.vsh";
			mGBufferSkinnedVShader.mName = "gbuffer_skin.vsh";
			mGBufferSkinnedVShader_Instanced.mName = "gbuffer_skin.vsh";
			mGBufferPShader.mName = "gbuffer.psh";
		}
		{
			for( u32 i = 0; i < cWriteModeCount; ++i )
			{
				mStaticVShaders_DP[ i ].mName = modeNames[ i ] + "_vshader_dp_static.vsh";
				mStaticVShaders_DP_Instanced[ i ].mName = modeNames[ i ] + "_vshader_dp_static_instanced.vsh";
				mSkinnedVShaders_DP[ i ].mName = modeNames[ i ] + "_vshader_dp_skinned.vsh";
				mSkinnedVShaders_DP_Instanced[ i ].mName = modeNames[ i ] + "_vshader_dp_skinned_instanced.vsh";
			}
			mDepthOnlyPShader_DP.mName = "depthonly_dp.psh";
			mDepthWithAlphaPShader_DP.mName = "depthalpha_dp.psh";
		}
	}

	void tHlslOutput::fWriteShadersToFile( const tFilePathPtr& relDermlPath ) const
	{
		const tFilePathPtr folder = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), relDermlPath );

		for( u32 i = 0; i < cWriteModeCount; ++i )
		{
			mStaticVShaders[ i ].fWriteToFile( folder );
			mStaticVShaders_Instanced[ i ].fWriteToFile( folder );
			mSkinnedVShaders[ i ].fWriteToFile( folder );
			mSkinnedVShaders_Instanced[ i ].fWriteToFile( folder );
		}
		for( u32 i = 0; i < mColorPShaders.fCount( ); ++i )
			mColorPShaders[ i ].fWriteToFile( folder );
		for( u32 i = 0; i < mColorShadowPShaders.fCount( ); ++i )
			mColorShadowPShaders[ i ].fWriteToFile( folder );
		mDepthOnlyPShader.fWriteToFile( folder );
		mDepthWithAlphaPShader.fWriteToFile( folder );

		mGBufferStaticVShader.fWriteToFile( folder );
		mGBufferStaticVShader_Instanced.fWriteToFile( folder );
		mGBufferSkinnedVShader.fWriteToFile( folder );
		mGBufferSkinnedVShader_Instanced.fWriteToFile( folder );
		mGBufferPShader.fWriteToFile( folder );

		for( u32 i = 0; i < cWriteModeCount; ++i )
		{
			mStaticVShaders_DP[ i ].fWriteToFile( folder );
			mStaticVShaders_DP_Instanced[ i ].fWriteToFile( folder );
			mSkinnedVShaders_DP[ i ].fWriteToFile( folder );
			mSkinnedVShaders_DP_Instanced[ i ].fWriteToFile( folder );
		}
		mDepthOnlyPShader_DP.fWriteToFile( folder );
		mDepthWithAlphaPShader_DP.fWriteToFile( folder );
	}
	namespace
	{
		u32 fCreateDx9VertexShader( const Gfx::tDevicePtr& device, tVertexShaderOutput& shader )
		{
			if( shader.mHlsl.length( ) > 0 )
			{
				shader.mSh = Dx9Util::fCreateVertexShader( device, shader.mHlsl, 0, &shader.mErrors );
				if( shader.mSh.fNull( ) )
					return 1;
				shader.mVtxFormat->fAllocateInPlace( device );
			}
			return 0;
		}
		u32 fCreateDx9VertexShaderGroup( const Gfx::tDevicePtr& device, tFixedArray<tVertexShaderOutput,cWriteModeCount>& shaderGroup )
		{
			u32 error = 0;
			for( u32 i = 0; i < cWriteModeCount; ++i )
			{
				error |= fCreateDx9VertexShader( device, shaderGroup[ i ] );
			}
			return error;
		}
		u32 fCreateDx9PixelShader( const Gfx::tDevicePtr& device, tPixelShaderOutput& shader )
		{
			if( shader.mHlsl.length( ) > 0 )
			{
				shader.mSh = Dx9Util::fCreatePixelShader( device, shader.mHlsl, 0, &shader.mErrors );
				return shader.mSh.fNull( );
			}
			return 0;
		}
		u32 fCreateDx9PixelShaderGroup( const Gfx::tDevicePtr& device, tDynamicArray<tPixelShaderOutput>& shaderGroup )
		{
			u32 error = 0;
			for( u32 i = 0; i < shaderGroup.fCount( ); ++i )
			{
				fCreateDx9PixelShader( device, shaderGroup[ i ] );
			}
			return error;
		}
	}
	b32 tHlslOutput::fCreateDx9Shaders( const Gfx::tDevicePtr& device )
	{
		u32 error = false;
		error |= fCreateDx9VertexShaderGroup( device, mStaticVShaders );
		error |= fCreateDx9VertexShaderGroup( device, mStaticVShaders_Instanced );
		error |= fCreateDx9VertexShaderGroup( device, mSkinnedVShaders );
		error |= fCreateDx9VertexShaderGroup( device, mSkinnedVShaders_Instanced );
		error |= fCreateDx9PixelShaderGroup( device, mColorPShaders );
		error |= fCreateDx9PixelShaderGroup( device, mColorShadowPShaders );
		error |= fCreateDx9PixelShader( device, mDepthOnlyPShader );
		error |= fCreateDx9PixelShader( device, mDepthWithAlphaPShader );
		error |= fCreateDx9VertexShader( device, mGBufferStaticVShader );
		error |= fCreateDx9VertexShader( device, mGBufferStaticVShader_Instanced );
		error |= fCreateDx9VertexShader( device, mGBufferSkinnedVShader );
		error |= fCreateDx9VertexShader( device, mGBufferSkinnedVShader_Instanced );
		error |= fCreateDx9PixelShader( device, mGBufferPShader );
		error |= fCreateDx9VertexShaderGroup( device, mStaticVShaders_DP );
		error |= fCreateDx9VertexShaderGroup( device, mStaticVShaders_DP_Instanced );
		error |= fCreateDx9VertexShaderGroup( device, mSkinnedVShaders_DP );
		error |= fCreateDx9VertexShaderGroup( device, mSkinnedVShaders_DP_Instanced );
		error |= fCreateDx9PixelShader( device, mDepthOnlyPShader_DP );
		error |= fCreateDx9PixelShader( device, mDepthWithAlphaPShader_DP );
		return !error;
	}

}}

