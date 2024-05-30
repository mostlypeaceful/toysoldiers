#include "ToolsPch.hpp"
#include "Derml.hpp"
#include "tDermlAssetPlugin.hpp"
#include "Gfx/tShadeMaterial.hpp"
#include "HlslGen/tHlslGenTree.hpp"
#include "tMaterialGenBase.hpp"
#include "Dx360Util.hpp"

namespace Sig
{
	using namespace Gfx;

	///
	/// \section tDermlAssetGenPlugin
	///

    tLoadInPlaceResourcePtr* tDermlAssetGenPlugin::fAddDependencyInternal( 
        tLoadInPlaceFileBase& outputFileObject, 
        const tFilePathPtr& dependencyPath 
    )
    {
        tLoadInPlaceResourcePtr* o = 0;
        return o;
    }

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
			default: log_warning( "Invalid platform target for shader." ); break;
			}
			return false;
		}

		static b32 fCompilePixelShaderForPlatform( tPlatformId pid, const std::string& hlsl, tDynamicBuffer& obuffer, std::string& errorsOut )
		{
			if( hlsl.length( ) == 0 )
				return true; // no shader to compile

			switch( pid )
			{
			case cPlatformPcDx9:
				return Dx9Util::fCompilePixelShader( hlsl, obuffer, 0, &errorsOut );
			case cPlatformXbox360:
				return Dx360Util::fCompilePixelShader( hlsl, obuffer, 0, &errorsOut );
			default: log_warning( "Invalid platform target for shader." ); break;
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

		struct tPlatformConvertData
		{
			const tPlatformId mPid;
			tMaterialFile& mMtlFile;
			tMaterialGenBase::tShaderBufferSet& mShaderBuffers;
			tPlatformConvertData( tPlatformId pid, tMaterialFile& mtlFile, tMaterialGenBase::tShaderBufferSet& shaderBuffers )
				: mPid( pid )
				, mMtlFile( mtlFile )
				, mShaderBuffers( shaderBuffers )
			{}
		};

		b32 fConvertVertexShaderGroup( tPlatformConvertData& data, const u32 groupId, tFixedArray<HlslGen::tVertexShaderOutput,HlslGen::cWriteModeCount>& shaderGroup )
		{
			data.mShaderBuffers[ groupId ].fNewArray( shaderGroup.fCount( ) );
			data.mMtlFile.mShaderLists[ groupId ].fNewArray( data.mShaderBuffers[ groupId ].fCount( ) );
			for( u32 i = 0; i < shaderGroup.fCount( ); ++i )
			{
				if( !fCompileVertexShaderForPlatform( data.mPid, shaderGroup[ i ].mHlsl, data.mShaderBuffers[ groupId ][ i ], shaderGroup[ i ].mErrors ) )
				{
					log_line( 0, shaderGroup[ i ].mName << " ERRORS:\n" << shaderGroup[ i ].mErrors << "\nSHADER:\n" << shaderGroup[ i ].mHlsl );
					return false;
				}
				tMaterialFile::tShaderPointer& shaderPtr = data.mMtlFile.mShaderLists[ groupId ][ i ];
				shaderPtr.mType = tMaterialFile::cShaderBufferTypeVertex;
				fConvertShaderGlue( shaderGroup[ i ], shaderPtr );
			}
			return true;
		}
		b32 fConvertPixelShaderGroup( tPlatformConvertData& data, const u32 groupId, tDynamicArray<HlslGen::tPixelShaderOutput>& shaderGroup )
		{
			data.mShaderBuffers[ groupId ].fNewArray( shaderGroup.fCount( ) );
			data.mMtlFile.mShaderLists[ groupId ].fNewArray( data.mShaderBuffers[ groupId ].fCount( ) );
			for( u32 i = 0; i < shaderGroup.fCount( ); ++i )
			{
				if( !fCompilePixelShaderForPlatform( data.mPid, shaderGroup[ i ].mHlsl, data.mShaderBuffers[ groupId ][ i ], shaderGroup[ i ].mErrors ) )
				{
					log_line( 0, shaderGroup[ i ].mErrors );
					return false;
				}
				tMaterialFile::tShaderPointer& shaderPtr = data.mMtlFile.mShaderLists[ groupId ][ i ];
				shaderPtr.mType = tMaterialFile::cShaderBufferTypePixel;
				fConvertShaderGlue( shaderGroup[ i ], shaderPtr );
			}
			return true;
		}
		b32 fConvertVertexShader( tPlatformConvertData& data, const u32 groupId, HlslGen::tVertexShaderOutput& shader )
		{
			data.mShaderBuffers[ groupId ].fNewArray( 1 );
			data.mMtlFile.mShaderLists[ groupId ].fNewArray( data.mShaderBuffers[ groupId ].fCount( ) );
			u32 i = 0;
			{
				if( !fCompileVertexShaderForPlatform( data.mPid, shader.mHlsl, data.mShaderBuffers[ groupId ][ i ], shader.mErrors ) )
				{
					log_line( 0, shader.mName << " ERRORS:\n" << shader.mErrors << "\nSHADER:\n" << shader.mHlsl );
					return false;
				}
				tMaterialFile::tShaderPointer& shaderPtr = data.mMtlFile.mShaderLists[ groupId ][ i ];
				shaderPtr.mType = tMaterialFile::cShaderBufferTypeVertex;
				fConvertShaderGlue( shader, shaderPtr );
			}
			return true;
		}
		b32 fConvertPixelShader( tPlatformConvertData& data, const u32 groupId, HlslGen::tPixelShaderOutput& shader )
		{
			data.mShaderBuffers[ groupId ].fNewArray( 1 );
			data.mMtlFile.mShaderLists[ groupId ].fNewArray( data.mShaderBuffers[ groupId ].fCount( ) );
			u32 i = 0;
			{
				if( !fCompilePixelShaderForPlatform( data.mPid, shader.mHlsl, data.mShaderBuffers[ groupId ][ i ], shader.mErrors ) )
				{
					log_line( 0, shader.mErrors );
					return false;
				}
				tMaterialFile::tShaderPointer& shaderPtr = data.mMtlFile.mShaderLists[ groupId ][ i ];
				shaderPtr.mType = tMaterialFile::cShaderBufferTypePixel;
				fConvertShaderGlue( shader, shaderPtr );
			}
			return true;
		}

		static b32 fConvertShadersForPlatform( tPlatformId pid, HlslGen::tHlslOutput& hlslOutput, Gfx::tMaterialFile& mtlFile, tMaterialGenBase::tShaderBufferSet& shaderBuffers )
		{
			mtlFile.mDiscardShaderBuffers = true; // TODO this may need to be platform dependent

			shaderBuffers.fNewArray( Gfx::tShadeMaterial::cShaderSlotCount );
			mtlFile.mShaderLists.fNewArray( shaderBuffers.fCount( ) );
			
			tPlatformConvertData data( pid, mtlFile, shaderBuffers );

			//standard shaders
			if( !fConvertVertexShaderGroup( data,	tShadeMaterial::cShaderSlotStaticVs,					hlslOutput.mStaticVShaders ) ) return false;
			if( !fConvertVertexShaderGroup( data,	tShadeMaterial::cShaderSlotStaticVs_Instanced,			hlslOutput.mStaticVShaders_Instanced ) ) return false;
			if( !fConvertVertexShaderGroup( data,	tShadeMaterial::cShaderSlotSkinnedVs,					hlslOutput.mSkinnedVShaders ) ) return false;
			if( !fConvertVertexShaderGroup( data,	tShadeMaterial::cShaderSlotSkinnedVs_Instanced,			hlslOutput.mSkinnedVShaders_Instanced ) ) return false;
			if( !fConvertPixelShaderGroup( data,	tShadeMaterial::cShaderSlotColorPs,						hlslOutput.mColorPShaders ) ) return false;
			if( !fConvertPixelShaderGroup( data,	tShadeMaterial::cShaderSlotColorShadowPs,				hlslOutput.mColorShadowPShaders ) ) return false;
			if( !fConvertPixelShader( data,			tShadeMaterial::cShaderSlotDepthPs,						hlslOutput.mDepthOnlyPShader ) ) return false;
			if( !fConvertPixelShader( data,			tShadeMaterial::cShaderSlotDepthAlphaPs,				hlslOutput.mDepthWithAlphaPShader ) ) return false;
			
			//gbuffer shaders
			if( !fConvertVertexShader( data,		tShadeMaterial::cShaderSlotGBufferStaticVs,				hlslOutput.mGBufferStaticVShader ) ) return false;
			if( !fConvertVertexShader( data,		tShadeMaterial::cShaderSlotGBufferStaticVs_Instanced,	hlslOutput.mGBufferStaticVShader_Instanced ) ) return false;
			if( !fConvertVertexShader( data,		tShadeMaterial::cShaderSlotGBufferSkinnedVs,			hlslOutput.mGBufferSkinnedVShader ) ) return false;
			if( !fConvertVertexShader( data,		tShadeMaterial::cShaderSlotGBufferSkinnedVs_Instanced,	hlslOutput.mGBufferSkinnedVShader_Instanced ) ) return false;
			if( !fConvertPixelShader( data,			tShadeMaterial::cShaderSlotGBufferPs,					hlslOutput.mGBufferPShader ) ) return false;

			// dual paraboloid shaders
			if( !fConvertVertexShaderGroup( data,	tShadeMaterial::cShaderSlotStaticVs_DP,					hlslOutput.mStaticVShaders_DP ) ) return false;
			if( !fConvertVertexShaderGroup( data,	tShadeMaterial::cShaderSlotSkinnedVs_DP,				hlslOutput.mSkinnedVShaders_DP ) ) return false;
			if( !fConvertPixelShader( data,			tShadeMaterial::cShaderSlotDepthPs_DP,					hlslOutput.mDepthOnlyPShader_DP ) ) return false;
			if( !fConvertPixelShader( data,			tShadeMaterial::cShaderSlotDepthAlphaPs_DP,				hlslOutput.mDepthWithAlphaPShader_DP ) ) return false;

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
				default: log_warning( "Invalid platform target for shader." ); break;
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

			// dump shaders to temp folder so we can see the result
			output.fWriteShadersToFile( inOut.mOutput );
		}
	}

	void tDermlAssetGenPlugin::fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Derml::tFile& dermlFile )
	{
		// no dependencies
	}

}
