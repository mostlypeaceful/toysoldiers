#include "ToolsPch.hpp"
#include "Gfx/tHeightFieldMaterial.hpp"
#include "Gfx/tTextureFile.hpp"
#include "tHeightFieldMaterialGen.hpp"
#include "tDeferredShadingMaterialGen.hpp"
#include "FileSystem.hpp"
#include "tPlatform.hpp"

namespace Sig
{
	using StringUtil::fAppend;

	register_rtti_factory( tHeightFieldMaterialGen, false );

	struct tHeightFieldLightingPassGen : public Gfx::tHeightFieldMaterial::tLightingPass
	{
		static void fVsGlobals( std::stringstream& ss )
		{
			ss << "#pragma pack_matrix(row_major)" << std::endl;

			// local to world matrix
			ss << "float3x4 gLocalToWorld : register( " << fAppend("c", cVSLocalToWorld) << " );" << std::endl;

			// combined view-projection matrix (world to projection)
			ss << "float4x4 gWorldToProj : register( " << fAppend("c", cVSWorldToProj) << " );" << std::endl;

			// eye position in world space
			ss << "float4 gWorldEyePos : register( " << fAppend("c", cVSWorldEyePos) << " );" << std::endl;

			// world to light space matrix
			ss << "float4x4 gWorldToLight : register( " << fAppend("c", cVSWorldToLight) << " );" << std::endl;
		}

		static void fAtlasSampler3D( std::stringstream& ss, b32 normalMap )
		{
			ss << "void fSampleTextureAtlas( out float4 sample0, " << ( normalMap ? "out float4 sample1, " : "" ) << "float mtlId, float2 nworlduv )" << std::endl;
			ss << "{" << std::endl;

			ss << "   float newTileFactor = gMaterialTilingFactors[ mtlId / 4 ][ mtlId % 4 ];" << std::endl;
			ss << "   float2 worldUv = nworlduv * ( newTileFactor * newTileFactor * 250.f );" << std::endl;

			ss << "   float layerCount = gTexAtlasDiffuseCount_NormalCount.x * gTexAtlasDiffuseCount_NormalCount.y;" << std::endl;
			ss << "   float sliceNorm = 1.0 / ( 2.0 * layerCount ) + ( mtlId / layerCount );" << std::endl;
			ss << "   sample0 = tex3D( gDiffuseMap, float3( worldUv, sliceNorm ) );" << std::endl;

			if( normalMap )
				ss << "   sample1 = tex3D( gNormalMap, float3( worldUv, sliceNorm ) );" << std::endl;

			ss << "}" << std::endl;
		}

		static void fAtlasSampler( std::stringstream& ss, b32 normalMap )
		{
			ss << "void fSampleTextureAtlas( out float4 sample0, " << ( normalMap ? "out float4 sample1, " : "" ) << "float mtlId, float2 nworlduv, float2 tileTexelDims, float2 invTexAtlasTexelDims, " << std::endl;
			ss << "                            float2 texAtlasDims, float2 invTexAtlasDims )" << std::endl;
			ss << "{" << std::endl;

			ss << "   float newTileFactor = gMaterialTilingFactors[ mtlId / 4 ][ mtlId % 4 ];" << std::endl;
			ss << "   float2 worldUv = nworlduv * ( newTileFactor * newTileFactor * 250.f );" << std::endl;

			ss << "   float2 gutter = 1.0f * invTexAtlasTexelDims;" << std::endl;
			ss << "   float2 dx = invTexAtlasTexelDims.x * ddx( worldUv * tileTexelDims.x );" << std::endl;
			ss << "   float2 dy = invTexAtlasTexelDims.x * ddy( worldUv * tileTexelDims.y );" << std::endl;
			ss << "   float2 tiledWorldUv = frac( worldUv );" << std::endl;
			ss << "   float2 uvOffset = float2( fmod( mtlId, texAtlasDims.x ), floor( mtlId * invTexAtlasDims.x ) );" << std::endl;
			ss << "   float2 uv = uvOffset * invTexAtlasDims + gutter + tiledWorldUv * ( invTexAtlasDims - 2.f * gutter );" << std::endl;
			ss << "   sample0 = tex2Dgrad( gDiffuseMap, uv, dx, dy );" << std::endl;

			if( normalMap )
				ss << "   sample1 = tex2Dgrad( gNormalMap, uv, dx, dy );" << std::endl;

			ss << "}" << std::endl;
		}

		static void fPsGlobals( std::stringstream& ss, b32 normalMap, b32 shadows, tPlatformId pid )
		{
			ss << "#pragma pack_matrix(row_major)" << std::endl;

			// eye position in world space
			ss << "float4 gWorldEyePos : register( " << 
				fAppend("c", cPSWorldEyePos) << " );" << std::endl;

			// world space dimensions (x-z only) and texture atlas dimension (num textures x, num textures y)
			ss << "float4 gWorldSpaceDims_TexAtlasMaxPixelDims : register( " << fAppend("c", cPSWorldSpaceDims_TextureAtlasMaxPixelDims) << " );" << std::endl;

			// number of textures across and down in each texture atlas
			ss << "float4 gTexAtlasDiffuseCount_NormalCount : register( " << fAppend("c", cPSDiffuseCount_NormalCount) << " );" << std::endl;

			// fog values
			ss << "float4 gFogValues : register( " << fAppend("c", cPSFogValues) << " );" << std::endl;

			// fog color
			ss << "float3 gFogColor : register( " << fAppend("c", cPSFogColor) << " );" << std::endl;

			// rgba tint value
			ss << "float4 gRgbaTint : register( " << fAppend("c", cPSRgbaTint) << " );" << std::endl;

			// misc. shadow map values
			if( shadows )
			{
				ss << "float4 gShadowMapEpsilon : register( " << fAppend("c", cPSShadowMapEpsilon) << " );" << std::endl;
				ss << "float4 gShadowMapEpsilon_TexelSize_Amount_Split : register( " << fAppend("c", cPSShadowMapEpsilon_TexelSize_Amount_Split) << " );" << std::endl;
				ss << "float4 gShadowMapTarget_Split : register( " << fAppend("c", cPSShadowMapTarget_Split) << " );" << std::endl;
			}

			// tiling factors
			ss << "float4 gMaterialTilingFactors[8] : register( " << fAppend("c", cPSTilingFactors ) << " );" << std::endl;

			ss << "float4 gSphericalHarmonics[" << Gfx::tSphericalHarmonics::cFactorCount << "] : register( c" << cPSSphericalHarmonics << " );" << std::endl;

			// declare world to light constants
			tHeightFieldMaterialGenBase::fDeclareWorldToLightConstantsArray( pid, ss, cPSWorldToLightArray );

			// declare light constants
			tHeightFieldMaterialGenBase::fDeclareLightConstantsArray( ss, cPSLightVectorFirst );

			const b32 supports3dTexture = tMaterialGenBase::fSupports3DTextures( pid );
			const std::string shadowMapSampler = tHeightFieldMaterialGenBase::fShadowMapSamplerName( pid );
			const std::string texAtlasSampler = supports3dTexture ? "sampler3D" : "sampler2D";

			if( shadows )
				ss << shadowMapSampler << " gShadowMap : register( s" << 0 << " );" << std::endl;
			ss << "sampler2D gMaskMap : register( s" << 1 << " );" << std::endl;
			ss << "sampler2D gMtlIdsMap : register( s" << 2 << " );" << std::endl;
			ss << texAtlasSampler << " gDiffuseMap : register( s" << 3 << " );" << std::endl;
			if( normalMap )
				ss << texAtlasSampler << " gNormalMap : register( s" << 4 << " );" << std::endl;

			ss << std::endl;

			if( supports3dTexture )
				fAtlasSampler3D( ss, normalMap );
			else
				fAtlasSampler( ss, normalMap );
		}

		static void fVSInputStruct( std::stringstream& ss, b32 useCompressedInputFormat )
		{
			ss << "struct tVSInput" << std::endl;
			ss << "{" << std::endl;
			if( useCompressedInputFormat )
			{
				ss << "   float4 vP : POSITION;" << std::endl;
				ss << "   float2 vN : NORMAL;" << std::endl;
			}
			else
			{
				ss << "   float3 vP : POSITION;" << std::endl;
				ss << "   float3 vN : NORMAL;" << std::endl;
			}
			//ss << "  float4 vC : COLOR;" << std::endl;
			ss << "};" << std::endl;
		}

		static void fVSOutputPsInputStruct( std::stringstream& ss, b32 psIn, b32 normalMapped, b32 shadows, b32 deferred, u32 lightCount )
		{
			if( !psIn )
				ss << "struct tVSOutput" << std::endl;
			else
				ss << "struct tPSInput" << std::endl;
			ss << "{" << std::endl;
			if( !psIn )
				ss << "   float4 vCP : POSITION;" << std::endl;
			if( !deferred )
				ss << "   float3 vP : TEXCOORD0;" << std::endl;
			//ss << "  float4 vC : COLOR;" << std::endl;

			if( !psIn || lightCount > 0 )
			{
				ss << "   float3 vLP : TEXCOORD1;" << std::endl;

				if( !deferred )
				{
					if( !psIn || shadows )
						ss << "   float4 vLightPos : TEXCOORD2;" << std::endl;
				}
				ss << "   float3 vN : NORMAL;" << std::endl;
				if( normalMapped )
					ss << "   float3 vT : TANGENT;" << std::endl;
			}
			ss << "};" << std::endl;
		}

		static std::string& fVS( std::string& o, b32 normalMap, b32 useCompressedInputFormat, b32 deferred, tPlatformId pid )
		{
			std::stringstream ss;

			fVsGlobals( ss );

			fVSInputStruct( ss, useCompressedInputFormat );
			fVSOutputPsInputStruct( ss, false, normalMap, false, deferred, 0 );

			ss << "tVSOutput main( tVSInput i )" << std::endl;
			ss << "{" << std::endl;
			ss << "   tVSOutput o;" << std::endl;

			if( useCompressedInputFormat )
				ss << "   float3 inNormal = float3( i.vN.x, i.vN.y, i.vP.w );" << std::endl;
			else
				ss << "   float3 inNormal = i.vN.xyz;" << std::endl;

			// compute world pos
			ss << "   float3 pworld = mul( gLocalToWorld, float4( i.vP.xyz, 1.f ) );" << std::endl;
			// output clip pos
			ss << "   o.vCP = mul( gWorldToProj, float4( pworld, 1.f ) );" << std::endl;
			// output local pos
			ss << "   o.vLP = i.vP.xyz;" << std::endl;
			
			if( !deferred )
			{
				// output light pos
				if( tHeightFieldMaterialGenBase::fShadowMapLayerCount( pid ) == 1 )
					ss << "   o.vLightPos = mul( gWorldToLight, float4( pworld, 1.f ) );" << std::endl;
				else
					ss << "   o.vLightPos = float4( pworld, 1.f );" << std::endl;

				// output modified-eye-relative
				ss << "   o.vP.xyz = float3( pworld - gWorldEyePos.xyz );" << std::endl;
			}

			// transform normal to world
			ss << "   o.vN.xyz = mul( ( float3x3 )gLocalToWorld, inNormal );" << std::endl;

			if( normalMap )
			{
				// compute and output tangent
				ss << "   o.vT.xyz = mul( ( float3x3 )gLocalToWorld, normalize( cross( inNormal, float3( 0, 0, -1 ) ) ) );" << std::endl;
			}

			//// pass through color
			//ss << "   o.vC = i.vC;" << std::endl;

			// output
			ss << "   return o;" << std::endl;

			ss << "}" << std::endl;
			o = ss.str( );
			return o;
		}

		static std::string& fPS( std::string& o, b32 normalMap, b32 shadows, b32 deferred, u32 lightCount, tPlatformId pid )
		{
			// TODO see if this makes any difference
			const b32 renormalizeTangentSpace = true;

			std::stringstream ss;

			fPsGlobals( ss, normalMap, shadows, pid );

			fVSOutputPsInputStruct( ss, true, normalMap, shadows, deferred, lightCount );

			if( deferred )
			{
				tDeferredShadingMaterialGen::fPackFunctions( ss );
				ss << "void main( tPSInput i, out float4 oColor : COLOR0, out float4 oNormal : COLOR1, out float4 oEmission : COLOR2, out float4 oAmbient : COLOR3 )" << std::endl;
			}
			else
				ss << "void main( tPSInput i, out float4 oColor : COLOR0 )" << std::endl;

			ss << "{" << std::endl;

			// copy diffuse vertex color
			//ss << "   float4 diffuse = i.vC;" << std::endl;
			ss << "   float4 diffuse = float4(1,1,1,1);" << std::endl;

			// declare diffuse accumulator
			if( deferred )
				ss << "   float3 diffuseAccum = 1.f;" << std::endl;
			else
				ss << "   float3 diffuseAccum = 0.f;" << std::endl;

			if( lightCount > 0 || deferred )
			{
				if( renormalizeTangentSpace )
				{
					ss << "   i.vN.xyz = normalize( i.vN.xyz );" << std::endl;

					if( normalMap )
						ss << "   i.vT.xyz = normalize( i.vT.xyz );" << std::endl;
				}

				// copy normal
				ss << "   float3 n = i.vN.xyz;" << std::endl;

				// position in height field local space
				ss << "   float3 plocal = i.vLP;" << std::endl;
				// create a normalized uv value
				ss << "   float2 xzDims = gWorldSpaceDims_TexAtlasMaxPixelDims.xy;" << std::endl;
				ss << "   float2 nworlduv  = ( plocal.xz + 0.5f * xzDims ) / xzDims;" << std::endl;
				// we want the base tiling factor for the diffuse/normal maps to remain constant in world-space, so we just pick an arbitrary
				// size as our "default" texture patch size, in this case 512 meters
				ss << "   float2 nworlduv2 = ( plocal.xz + 256.f ) / 512.f;" << std::endl;
				// sample mask texture
				ss << "   float3 materialMasks = tex2D( gMaskMap, nworlduv ).rgb;" << std::endl;
				// sample mtlids texture
				ss << "   float3 materialIds = tex2D( gMtlIdsMap, nworlduv ).rgb;" << std::endl;

				// compute actual material ids (expand to integer values in [0, num material ids)
				ss << "   float2 textureAtlasDims = gTexAtlasDiffuseCount_NormalCount.xy;" << std::endl;
				ss << "   float2 invTextureAtlasDims = 1.f / textureAtlasDims;" << std::endl;
				ss << "   float3 materialIdsExpanded = floor( materialIds * float3( 31.f, 63.f, 31.f ) + 0.49f );" << std::endl;

				// dimensions of a tile
				ss << "   float2 tileTexelDims = gWorldSpaceDims_TexAtlasMaxPixelDims.zw;" << std::endl;

				// inverse of the entire atlas's texel dimensions
				ss << "   float2 invTexAtlasTexelDims = 1.f / ( tileTexelDims * textureAtlasDims );" << std::endl;

				// sample each material's diffuse texture in the big texture atlas
				ss << "   float4 diffuseMtl0,diffuseMtl1,diffuseMtl2;" << std::endl;
				if( normalMap )
					ss << "   float4 normalMapMtl0,normalMapMtl1,normalMapMtl2;" << std::endl;

				if( tMaterialGenBase::fSupports3DTextures( pid ) )
				{
					ss << "   fSampleTextureAtlas( diffuseMtl0, " << ( normalMap ? "normalMapMtl0, " : "" ) << "materialIdsExpanded.x, nworlduv2 );" << std::endl;
					ss << "   fSampleTextureAtlas( diffuseMtl1, " << ( normalMap ? "normalMapMtl1, " : "" ) << "materialIdsExpanded.y, nworlduv2 );" << std::endl;
					ss << "   fSampleTextureAtlas( diffuseMtl2, " << ( normalMap ? "normalMapMtl2, " : "" ) << "materialIdsExpanded.z, nworlduv2 );" << std::endl;
				}
				else
				{
					ss << "   fSampleTextureAtlas( diffuseMtl0, " << ( normalMap ? "normalMapMtl0, " : "" ) << "materialIdsExpanded.x, nworlduv2, tileTexelDims, invTexAtlasTexelDims, " << std::endl;
					ss << "            textureAtlasDims, invTextureAtlasDims );" << std::endl;
					ss << "   fSampleTextureAtlas( diffuseMtl1, " << ( normalMap ? "normalMapMtl1, " : "" ) << "materialIdsExpanded.y, nworlduv2, tileTexelDims, invTexAtlasTexelDims, " << std::endl;
					ss << "            textureAtlasDims, invTextureAtlasDims );" << std::endl;
					ss << "   fSampleTextureAtlas( diffuseMtl2, " << ( normalMap ? "normalMapMtl2, " : "" ) << "materialIdsExpanded.z, nworlduv2, tileTexelDims, invTexAtlasTexelDims, " << std::endl;
					ss << "            textureAtlasDims, invTextureAtlasDims );" << std::endl;
				}

				if( normalMap )
				{
					ss << "   float3 tangentSpaceNormal = materialMasks.x * ( 2*normalMapMtl0.aag-1 ) + materialMasks.y * ( 2*normalMapMtl1.aag-1 ) + materialMasks.z * ( 2*normalMapMtl2.aag-1 );" << std::endl;
					ss << "   tangentSpaceNormal.y = sqrt( 1 - tangentSpaceNormal.x * tangentSpaceNormal.x - tangentSpaceNormal.z * tangentSpaceNormal.z );" << std::endl;
					const f32 bumpScale = 1.f; // TODOHACK
					ss << "   tangentSpaceNormal.xz *= " << bumpScale << ";" << std::endl;

					//ss << "float3 tangent = normalize( cross( n, float3( 0, 0, 1 ) ) );" << std::endl;
					ss << "   float3 bitangent = normalize( cross( i.vT.xyz, n ) );" << std::endl;

					ss << "   n = normalize( tangentSpaceNormal.x * i.vT.xyz + tangentSpaceNormal.y * n + tangentSpaceNormal.z * bitangent );" << std::endl;
				}

				// mix the materials' diffuse samples using material mas
				ss << "   float4 diffuseMaterialColor = materialMasks.x * diffuseMtl0 + materialMasks.y * diffuseMtl1 + materialMasks.z * diffuseMtl2;" << std::endl;

				// modulate incoming vertex diffuse by material diffuse
				ss << "   diffuse.rgb *= diffuseMaterialColor.rgb;" << std::endl;

				if( !deferred )
				{
					if( shadows )
					{
						tMaterialGenBase::tComputeShadowParameters shadowParams;
						shadowParams.mPid						= pid;
						shadowParams.mSs						= &ss;
						shadowParams.mWorldPosName				= "i.vLightPos";
						//shadowParams.mNDotL					= "nDotL"; // dot( n, gLights[ 0 ].vN??? )
						tMaterialGenBase::fCallComputeShadowTerm( shadowParams );
					}

					// for each light, accumulate diffuse
					ss << "   int iLight = 0;" << std::endl;
					tHeightFieldMaterialGenBase::fCallAccumulateLight
						( ss,
						false,
						Gfx::tLight::cLightTypeDirection,
						"gLights[ iLight ]",
						"i.vP.xyz",
						"n",
						"0.f",
						"diffuseAccum",
						"specAccum",
						"specMagAccum",
						shadows ? "shadowTerm" : "1",
						true,
						"gSphericalHarmonics" );

					if( lightCount > 1 )
					{
						// These lights are treated differently because they dont support shadowing

						if( lightCount > 2 )
							ss << "   for( iLight = 1; iLight < " << lightCount << "; ++iLight )" << std::endl;
						else
							ss << "   iLight = 1;" << std::endl;

						ss << "   {" << std::endl;

						tHeightFieldMaterialGenBase::fCallAccumulateLight(
							ss,
							false,
							Gfx::tLight::cLightTypePoint,
							"gLights[ iLight ]",
							"i.vP.xyz",
							"n",
							"0.f",
							"diffuseAccum",
							"specAccum",
							"specMagAccum",
							shadows ? "1" : "1" );

						// end light loop
						ss << "   }" << std::endl;
					}
				}
			}

			// modulate d by diffuse texture samples
			ss << "   diffuseAccum *= diffuse.rgb;" << std::endl;

			// modulate by rgba tint
			ss << "   float4 preFog = float4( diffuseAccum.rgb * gRgbaTint.rgb, 1.f );" << std::endl;

			if( deferred  )
			{
				ss << "   oColor = float4( preFog.rgb, 0.f );" << std::endl;
				ss << "   oNormal = float4( fPackNormal( n ), 0 );" << std::endl;
				ss << "   oEmission = float4( 0, 0, 0, 0 );" << std::endl;

				ss << "   float4 ambientAccum;" << std::endl;
				tMaterialGenBase::fSphericalLookup( ss, "n", "ambientAccum", "gSphericalHarmonics" );

				ss << "   oAmbient = ambientAccum;" << std::endl;
			}
			else
			{
				tHeightFieldMaterialGenBase::fCallFog( ss, "preFog.rgb", "i.vP.xyz", "gFogValues", "gFogColor", "fogResult" );
				ss << "   oColor = float4( fogResult, preFog.a );" << std::endl;
			}

			// DEBUG NOTE - If you want to render normals as colors comment this code back in.
			//{
			//	if( lightCount > 0 )
			//	{
			//		// For absolute direction ( 0 - 1 )
			//		//ss << "   oColor = abs( float4( n.xyz, preFog.a ) );" << std::endl;
			//		// For direct direction ( -1 - 1 )
			//		ss << "   oColor = float4( n.xyz, preFog.a );" << std::endl;
			//	}
			//}

			ss << "}" << std::endl;
			o = ss.str( );
			return o;
		}

		void fGenerate( Gfx::tMaterialFile& mtlFile, tHeightFieldMaterialGenBase::tShaderBufferSet& shaderBuffers, tPlatformId pid )
		{
			shaderBuffers[ Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS ].fNewArray( cVSCount );
			mtlFile.mShaderLists[ Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS ].fNewArray( cVSCount );
			shaderBuffers[ Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingPS ].fNewArray( cPSCount * Gfx::tMaterial::cLightSlotCount );
			mtlFile.mShaderLists[ Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingPS ].fNewArray( cPSCount * Gfx::tMaterial::cLightSlotCount );

			const tFilePathPtr tmpFolder = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePathPtr( "heightfield/" ), tFilePathPtr( fPlatformIdString(pid) ) );

			std::string s;

			fVS( s, false, false, false, pid );
			FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse.vsh" ) ) );
			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS, cVSDiffuse, mtlFile, shaderBuffers, s );

			fVS( s, true, false, false, pid );
			FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_nmap.vsh" ) ) );
			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS, cVSDiffuseNormalMap, mtlFile, shaderBuffers, s );

			fVS( s, false, true, false, pid );
			FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_compressed.vsh" ) ) );
			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS, cVSDiffuseCompressed, mtlFile, shaderBuffers, s );

			fVS( s, true, true, false, pid );
			FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_nmap_compressed.vsh" ) ) );
			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS, cVSDiffuseNormalMapCompressed, mtlFile, shaderBuffers, s );



			fVS( s, false, false, true, pid );
			FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_gbuf.vsh" ) ) );
			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS, cVSDiffuseGBuffer, mtlFile, shaderBuffers, s );

			fVS( s, true, false, true, pid );
			FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_gbuf_nmap.vsh" ) ) );
			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS, cVSDiffuseNormalMapGBuffer, mtlFile, shaderBuffers, s );

			fVS( s, false, true, true, pid );
			FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_gbuf_compressed.vsh" ) ) );
			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS, cVSDiffuseGBufferCompressed, mtlFile, shaderBuffers, s );

			fVS( s, true, true, true, pid );
			FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_gbuf_nmap_compressed.vsh" ) ) );
			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingVS, cVSDiffuseNormalMapGBufferCompressed, mtlFile, shaderBuffers, s );


			for( u32 iLightCount = 0; iLightCount < Gfx::tMaterial::cLightSlotCount; ++iLightCount )
			{
				fPS( s, false, false, false, iLightCount, pid );
				FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse.psh" ) ) );
				tHeightFieldMaterialGenBase::fAddPixelShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingPS,
					fPixelShaderSlot( cPSDiffuse, iLightCount ), mtlFile, shaderBuffers, s );


				fPS( s, false, true, false, iLightCount, pid );
				FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_shadow.psh" ) ) );
				tHeightFieldMaterialGenBase::fAddPixelShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingPS,
					fPixelShaderSlot( cPSDiffuseShadow, iLightCount ), mtlFile, shaderBuffers, s );


				fPS( s, true, false, false, iLightCount, pid );
				FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_nmap.psh" ) ) );
				tHeightFieldMaterialGenBase::fAddPixelShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingPS,
					fPixelShaderSlot( cPSDiffuseNormalMap, iLightCount ), mtlFile, shaderBuffers, s );


				fPS( s, true, true, false, iLightCount, pid );
				FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuse_nmap_shadow.psh" ) ) );
				tHeightFieldMaterialGenBase::fAddPixelShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingPS,
					fPixelShaderSlot( cPSDiffuseNormalMapShadow, iLightCount ), mtlFile, shaderBuffers, s );

				fPS( s, false, false, true, 1, pid );
				FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuseGBuff.psh" ) ) );
				tHeightFieldMaterialGenBase::fAddPixelShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingPS,
					fPixelShaderSlot( cPSDiffuseGBuffer, iLightCount ), mtlFile, shaderBuffers, s );

				fPS( s, true, false, true, 1, pid );
				FileSystem::fWriteBufferToFile( s.c_str( ), s.length( ), tFilePathPtr::fConstructPath( tmpFolder, tFilePathPtr( "diffuseGBuff_nmap.psh" ) ) );
				tHeightFieldMaterialGenBase::fAddPixelShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryLightingPS,
					fPixelShaderSlot( cPSDiffuseNormalMapGBuffer, iLightCount ), mtlFile, shaderBuffers, s );
			}
		}
	};


	struct tHeightFieldShadowMapPassGen : public Gfx::tHeightFieldMaterial::tShadowMapPass
	{
		static void fVsGlobals( std::stringstream& ss )
		{
			ss << "#pragma pack_matrix(row_major)" << std::endl;

			// local to world matrix
			ss << "float3x4 gLocalToWorld : register( " << 
				fAppend("c", cVSLocalToWorld) << " );" << std::endl;

			// combined view-projection matrix (world to projection)
			ss << "float4x4 gWorldToProj : register( " << 
				fAppend("c", cVSWorldToProj) << " );" << std::endl;
		}

		static void fPsGlobals( std::stringstream& ss, tPlatformId pid )
		{
		}

		static void fVSInputStruct( std::stringstream& ss )
		{
			ss << "struct tVSInput" << std::endl;
			ss << "{" << std::endl;
			ss << "  float4 vP : POSITION;" << std::endl;
			ss << "};" << std::endl;
		}

		static void fVSOutputPsInputStruct( std::stringstream& ss, b32 psIn )
		{
			if( !psIn )
				ss << "struct tVSOutput" << std::endl;
			else
				ss << "struct tPSInput" << std::endl;
			ss << "{" << std::endl;
			if( !psIn )
				ss << "  float4 vCP : POSITION;" << std::endl;
			ss << "float2 vDepth : TEXCOORD0;" << std::endl;
			ss << "};" << std::endl;
		}

		static std::string& fVS( std::string& o )
		{
			std::stringstream ss;

			fVsGlobals( ss );

			fVSInputStruct( ss );
			fVSOutputPsInputStruct( ss, false );

			ss << "tVSOutput main( tVSInput i )" << std::endl;
			ss << "{" << std::endl;
			ss << "   tVSOutput o;" << std::endl;

			// compute world pos
			ss << "   float3 pworld = mul( gLocalToWorld, float4( i.vP.xyz, 1.f ) );" << std::endl;
			// output clip pos
			ss << "   o.vCP = mul( gWorldToProj, float4( pworld, 1.f ) );" << std::endl;
			// store depth
			ss << "   o.vDepth = o.vCP.zw;" << std::endl;
			// output
			ss << "   return o;" << std::endl;

			ss << "}" << std::endl;
			o = ss.str( );
			return o;
		}

		static std::string& fPS( std::string& o, tPlatformId pid )
		{
			std::stringstream ss;

			fPsGlobals( ss, pid );

			fVSOutputPsInputStruct( ss, true );

			ss << "void main( tPSInput i, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "  oColor = i.vDepth.x / i.vDepth.y;" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
			return o;
		}

		void fGenerate( Gfx::tMaterialFile& mtlFile, tHeightFieldMaterialGenBase::tShaderBufferSet& shaderBuffers, tPlatformId pid )
		{
			shaderBuffers[ Gfx::tHeightFieldMaterialBase::cShaderCategoryShadowMapVS ].fNewArray( cVSCount );
			mtlFile.mShaderLists[ Gfx::tHeightFieldMaterialBase::cShaderCategoryShadowMapVS ].fNewArray( cVSCount );
			shaderBuffers[ Gfx::tHeightFieldMaterialBase::cShaderCategoryShadowMapPS ].fNewArray( cPSCount );
			mtlFile.mShaderLists[ Gfx::tHeightFieldMaterialBase::cShaderCategoryShadowMapPS ].fNewArray( cPSCount );

			std::string s;

			tHeightFieldMaterialGenBase::fAddVertexShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryShadowMapVS, cVSDepth, mtlFile, shaderBuffers, fVS( s ) );
			tHeightFieldMaterialGenBase::fAddPixelShader( pid, Gfx::tHeightFieldMaterialBase::cShaderCategoryShadowMapPS, cPSDepth, mtlFile, shaderBuffers, fPS( s, pid ) );
		}
	};


	tHeightFieldMaterialGen::tHeightFieldMaterialGen( )
	{
	}

	Gfx::tMaterial* tHeightFieldMaterialGen::fCreateHeightFieldGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned, b32 compressedVerts )
	{
		Gfx::tHeightFieldMaterial* mtl = new Gfx::tHeightFieldMaterial( );

		fConvertBase( mtl );

		mtl->mWorldSpaceDims = mWorldSpaceDims;
		mtl->mSubDiffuseRectDims = mSubDiffuseRectDims;
		mtl->mSubNormalRectDims = mSubNormalRectDims;
		mtl->mDiffuseCount_NormalCount = mDiffuseCount_NormalCount;
		mtl->mTileFactors = mTileFactors;

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tHeightFieldMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		const b32 normalMapped = mNormalMapTextureName.fLength( ) > 0;

		const tResourceId maskMapFileRid = tResourceId::fMake<Gfx::tTextureFile>( mMaskTextureName );
		const tResourceId mtlidsMapFileRid = tResourceId::fMake<Gfx::tTextureFile>( mMtlIdsTextureName );
		const tResourceId diffuseMapFileRid = tResourceId::fMake<Gfx::tTextureFile>( mDiffuseTextureName );
		const tResourceId normalMapFileRid = tResourceId::fMake<Gfx::tTextureFile>( mNormalMapTextureName );
		mtl->mMaskMap.fSetLoadInPlace( lipFileCreator.fAddLoadInPlaceResourcePtr( maskMapFileRid ) );
		mtl->mMtlIdsMap.fSetLoadInPlace( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlidsMapFileRid ) );
		mtl->mDiffuseMap.fSetLoadInPlace( lipFileCreator.fAddLoadInPlaceResourcePtr( diffuseMapFileRid ) );
		if( normalMapped )
			mtl->mNormalMap.fSetLoadInPlace( lipFileCreator.fAddLoadInPlaceResourcePtr( normalMapFileRid ) );

		// set up proper sampling mode for material ids map: no filter, clamp addressing
		mtl->mMtlIdsMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeNone, Gfx::tTextureFile::cAddressModeClamp );
		// set up proper sampling mode for mask map: filter, clamp addressing
		mtl->mMaskMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );
		// set up proper sampling mode for diffuse and normal maps: filter, wrap addressing
		mtl->mDiffuseMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );
		if( normalMapped )
			mtl->mNormalMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );

		// handle the different possibilities (normal mapped, etc)
		if( normalMapped )
		{
			if( compressedVerts )
				mtl->mLightingPass.mVS = Gfx::tHeightFieldMaterial::tLightingPass::cVSDiffuseNormalMapCompressed;
			else
				mtl->mLightingPass.mVS = Gfx::tHeightFieldMaterial::tLightingPass::cVSDiffuseNormalMap;
			mtl->mLightingPass.mPS = Gfx::tHeightFieldMaterial::tLightingPass::cPSDiffuseNormalMapShadow;
		}
		else
		{
			if( compressedVerts )
				mtl->mLightingPass.mVS = Gfx::tHeightFieldMaterial::tLightingPass::cVSDiffuseCompressed;
			else
				mtl->mLightingPass.mVS = Gfx::tHeightFieldMaterial::tLightingPass::cVSDiffuse;
			mtl->mLightingPass.mPS = Gfx::tHeightFieldMaterial::tLightingPass::cPSDiffuseShadow;
		}
		mtl->mShadowMapPass.mVS = Gfx::tHeightFieldMaterial::tShadowMapPass::cVSDepth;
		mtl->mShadowMapPass.mPS = Gfx::tHeightFieldMaterial::tShadowMapPass::cPSDepth;

		return mtl;
	}

	tFilePathPtr tHeightFieldMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tHeightFieldMaterial::fMaterialFilePath( );
	}

	void tHeightFieldMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tHeightFieldMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tHeightFieldMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( Gfx::tHeightFieldMaterialBase::cShaderCategoryCount );
		tShaderBufferSet shaderBuffers( Gfx::tHeightFieldMaterialBase::cShaderCategoryCount );

		tHeightFieldLightingPassGen lightingPassGen;
		lightingPassGen.fGenerate( mtlFile, shaderBuffers, pid );

		tHeightFieldShadowMapPassGen shadowMapPassGen;
		shadowMapPassGen.fGenerate( mtlFile, shaderBuffers, pid );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tHeightFieldMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tHeightFieldMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tHeightFieldMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

