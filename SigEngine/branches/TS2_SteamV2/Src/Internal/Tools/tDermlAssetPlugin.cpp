#include "ToolsPch.hpp"
#include "Derml.hpp"
#include "tDermlAssetPlugin.hpp"
#include "Gfx/tShadeMaterial.hpp"
#include "HlslGen/tHlslGenTree.hpp"
#include "tMaterialGenBase.hpp"
#include "Dx360Util.hpp"

namespace Sig
{

	///
	/// \section tDermlAssetGenPlugin
	///

	void tDermlAssetGenPlugin::fDetermineInputOutputs( 
		tInputOutputList& inputOutputsOut,
		const tFilePathPtrList& immediateInputFiles )
	{
		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Derml::fIsDermlFile( immediateInputFiles[ i ] ) )
			{
				inputOutputsOut.fGrowCount( 1 );
				inputOutputsOut.fBack( ).mOriginalInput = immediateInputFiles[i];
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[ i ] );
				inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Derml::fDermlPathToMtlb( immediateInputFiles[ i ] ) );
			}
		}
	}

	namespace
	{
		static b32 fCompileVertexShaderForPlatform( tPlatformId pid, const std::string& hlsl, tDynamicBuffer& obuffer, std::string& errorsOut )
		{
			if( hlsl.length( ) == 0 )
				return true; // no shader to compile

			switch( pid )
			{
			case cPlatformPcDx9:
				return Dx9Util::fCompileVertexShader( hlsl, obuffer, 0, &errorsOut );
			case cPlatformXbox360:
				return Dx360Util::fCompileVertexShader( hlsl, obuffer, 0, &errorsOut );
			default: log_warning( 0, "Invalid platform target for shader." ); break;
			}
			return false;
		}

		static b32 fCompilePixelShaderForPlatform( tPlatformId pid, const std::string& hlsl, tDynamicBuffer& obuffer, std::string& errorsOut )
		{
			switch( pid )
			{
			case cPlatformPcDx9:
				return Dx9Util::fCompilePixelShader( hlsl, obuffer, 0, &errorsOut );
			case cPlatformXbox360:
				return Dx360Util::fCompilePixelShader( hlsl, obuffer, 0, &errorsOut );
			default: log_warning( 0, "Invalid platform target for shader." ); break;
			}
			return false;
		}

		static void fConvertShaderGlue( const HlslGen::tShaderOutputBase& in, Gfx::tMaterialFile::tShaderPointer& out )
		{
			out.mGlueShared.fNewArray( in.mGlueShared.fCount( ) );
			out.mGlueInstance.fNewArray( in.mGlueInstance.fCount( ) );

			for( u32 i = 0; i < out.mGlueShared.fCount( ); ++i )
				out.mGlueShared[ i ] = in.mGlueShared[ i ].fGetRawPtr( );
			for( u32 i = 0; i < out.mGlueInstance.fCount( ); ++i )
				out.mGlueInstance[ i ] = in.mGlueInstance[ i ].fGetRawPtr( );
		}

		static b32 fConvertShadersForPlatform( tPlatformId pid, HlslGen::tHlslOutput& hlslOutput, Gfx::tMaterialFile& mtlFile, tMaterialGenBase::tShaderBufferSet& shaderBuffers )
		{
			mtlFile.mDiscardShaderBuffers = true; // TODO this may need to be platform dependent

			shaderBuffers.fNewArray( Gfx::tShadeMaterial::cShaderSlotCount );
			mtlFile.mShaderLists.fNewArray( shaderBuffers.fCount( ) );

			u32 curShBufferList = 0;
			u32 i = 0;

			curShBufferList = Gfx::tShadeMaterial::cShaderSlotStaticVs;
			shaderBuffers[ curShBufferList ].fNewArray( hlslOutput.mStaticVShaders.fCount( ) );
			mtlFile.mShaderLists[ curShBufferList ].fNewArray( shaderBuffers[ curShBufferList ].fCount( ) );
			for( i = 0; i < hlslOutput.mStaticVShaders.fCount( ); ++i )
			{
				if( !fCompileVertexShaderForPlatform( pid, hlslOutput.mStaticVShaders[ i ].mHlsl, shaderBuffers[ curShBufferList ][ i ], hlslOutput.mStaticVShaders[ i ].mErrors ) )
				{
					log_line( 0, hlslOutput.mStaticVShaders[ i ].mErrors );
					return false;
				}
				mtlFile.mShaderLists[ curShBufferList ][ i ].mType = Gfx::tMaterialFile::cShaderBufferTypeVertex;
				fConvertShaderGlue( hlslOutput.mStaticVShaders[ i ], mtlFile.mShaderLists[ curShBufferList ][ i ] );
			}

			curShBufferList = Gfx::tShadeMaterial::cShaderSlotSkinnedVs;
			shaderBuffers[ curShBufferList ].fNewArray( hlslOutput.mSkinnedVShaders.fCount( ) );
			mtlFile.mShaderLists[ curShBufferList ].fNewArray( shaderBuffers[ curShBufferList ].fCount( ) );
			for( i = 0; i < hlslOutput.mSkinnedVShaders.fCount( ); ++i )
			{
				if( !fCompileVertexShaderForPlatform( pid, hlslOutput.mSkinnedVShaders[ i ].mHlsl, shaderBuffers[ curShBufferList ][ i ], hlslOutput.mSkinnedVShaders[ i ].mErrors ) )
				{
					log_line( 0, hlslOutput.mSkinnedVShaders[ i ].mErrors );
					return false;
				}
				mtlFile.mShaderLists[ curShBufferList ][ i ].mType = Gfx::tMaterialFile::cShaderBufferTypeVertex;
				fConvertShaderGlue( hlslOutput.mSkinnedVShaders[ i ], mtlFile.mShaderLists[ curShBufferList ][ i ] );
			}

			curShBufferList = Gfx::tShadeMaterial::cShaderSlotColorPs;
			shaderBuffers[ curShBufferList ].fNewArray( hlslOutput.mColorPShaders.fCount( ) );
			mtlFile.mShaderLists[ curShBufferList ].fNewArray( shaderBuffers[ curShBufferList ].fCount( ) );
			for( i = 0; i < hlslOutput.mColorPShaders.fCount( ); ++i )
			{
				if( !fCompilePixelShaderForPlatform( pid, hlslOutput.mColorPShaders[ i ].mHlsl, shaderBuffers[ curShBufferList ][ i ], hlslOutput.mColorPShaders[ i ].mErrors ) )
				{
					log_line( 0, hlslOutput.mColorPShaders[ i ].mErrors );
					return false;
				}
				mtlFile.mShaderLists[ curShBufferList ][ i ].mType = Gfx::tMaterialFile::cShaderBufferTypePixel;
				fConvertShaderGlue( hlslOutput.mColorPShaders[ i ], mtlFile.mShaderLists[ curShBufferList ][ i ] );
			}

			curShBufferList = Gfx::tShadeMaterial::cShaderSlotDepthPs;
			shaderBuffers[ curShBufferList ].fNewArray( 1 );
			mtlFile.mShaderLists[ curShBufferList ].fNewArray( shaderBuffers[ curShBufferList ].fCount( ) );
			i = 0;
			{
				if( !fCompilePixelShaderForPlatform( pid, hlslOutput.mDepthOnlyPShader.mHlsl, shaderBuffers[ curShBufferList ][ i ], hlslOutput.mDepthOnlyPShader.mErrors ) )
				{
					log_line( 0, hlslOutput.mDepthOnlyPShader.mErrors );
					return false;
				}
				mtlFile.mShaderLists[ curShBufferList ][ i ].mType = Gfx::tMaterialFile::cShaderBufferTypePixel;
				fConvertShaderGlue( hlslOutput.mDepthOnlyPShader, mtlFile.mShaderLists[ curShBufferList ][ i ] );
			}

			curShBufferList = Gfx::tShadeMaterial::cShaderSlotDepthAlphaPs;
			shaderBuffers[ curShBufferList ].fNewArray( 1 );
			mtlFile.mShaderLists[ curShBufferList ].fNewArray( shaderBuffers[ curShBufferList ].fCount( ) );
			i = 0;
			{
				if( !fCompilePixelShaderForPlatform( pid, hlslOutput.mDepthWithAlphaPShader.mHlsl, shaderBuffers[ curShBufferList ][ i ], hlslOutput.mDepthWithAlphaPShader.mErrors ) )
				{
					log_line( 0, hlslOutput.mDepthWithAlphaPShader.mErrors );
					return false;
				}
				mtlFile.mShaderLists[ curShBufferList ][ i ].mType = Gfx::tMaterialFile::cShaderBufferTypePixel;
				fConvertShaderGlue( hlslOutput.mDepthWithAlphaPShader, mtlFile.mShaderLists[ curShBufferList ][ i ] );
			}

			return true;
		}
	}

	void tDermlAssetGenPlugin::fProcessInputOutputs(
		const tInputOutput& inOut,
		tFilePathPtrList& indirectGenFilesAddTo )
	{
		Derml::tFile dermlFile;

		if( !dermlFile.fLoadXml( inOut.mInputs.fFront( ) ) )
			return;

		// for each platform
		for( u32 i = 0; i < inOut.mPlatformsToConvert.fCount( ); ++i )
		{
			const tPlatformId pid = inOut.mPlatformsToConvert[ i ];

			HlslGen::tHlslPlatformId hlslPid = HlslGen::cPidDefault;
			if( dermlFile.mGeometryStyle == HlslGen::cVshMeshModel )
			{
				switch( pid )
				{
				case cPlatformPcDx9:	hlslPid = HlslGen::cPidPcDx9;	break;
				case cPlatformPcDx10:	hlslPid = HlslGen::cPidPcDx10;	break;
				case cPlatformXbox360:	hlslPid = HlslGen::cPidXbox360; break;
				case cPlatformPs3Ppu:	hlslPid = HlslGen::cPidPs3;		break;
				default: log_warning( 0, "Invalid platform target for shader." ); break;
				}
			}

			// generate hlsl shader code
			HlslGen::tHlslInput input = HlslGen::tHlslInput( hlslPid, dermlFile.mGeometryStyle, true, true, false );
			HlslGen::tHlslOutput output;
			HlslGen::tHlslGenTree::fGenerateShaders( dermlFile, input, output );

			Gfx::tMaterialFile mtlFile;
			tMaterialGenBase::tShaderBufferSet shaderBuffers;
			if( !fConvertShadersForPlatform( pid, output, mtlFile, shaderBuffers ) )
			{
				//return;
			}

			mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tShadeMaterial>( );

			// output the mtlb file
			tMaterialGenBase::fSaveMaterialFile( inOut.mOutput, mtlFile, shaderBuffers, pid );

// TODO dump shaders to temp folder so we can see the result
output.fWriteShadersToFile( );
		}
	}

	void tDermlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Derml::tFile& dermlFile )
	{
		// no dependencies
	}

}
