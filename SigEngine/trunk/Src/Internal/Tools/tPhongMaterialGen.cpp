#include "ToolsPch.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "tTextureGen.hpp"
#include "Gfx/tTextureFile.hpp"
#include "Gfx/tPhongMaterial.hpp"
#include "tPhongMaterialGen.hpp"

namespace Sig
{
	using StringUtil::fAppend;

	register_rtti_factory( tPhongMaterialGen, false );

	namespace
	{
		static const char cVSInputName[] = "tVSInput";
		static const char cVSOutputName[] = "tVSOutput";
		static const char cPSInputName[] = "tPSInput";
	}

	struct tPhongLightingPassGen : public Gfx::tPhongMaterial::tLightingPass
	{
		void fVsGlobals( std::stringstream& ss, b32 skinned )
		{
			ss << "#pragma pack_matrix(row_major)" << std::endl;

			// local to world position matrix
			ss << "float3x4 gLocalToWorldPos : register( " << fAppend("c", cVSLocalToWorldPos) << " );" << std::endl;

			// local to world position matrix
			ss << "float3x4 gLocalToWorldNormal : register( " << fAppend("c", cVSLocalToWorldNorm) << " );" << std::endl;

			// combined view-projection matrix (world to projection)
			ss << "float4x4 gWorldToProj : register( " << fAppend("c", cVSWorldToProj) << " );" << std::endl;

			// eye position in world space
			ss << "float4 gWorldEyePos : register( " << fAppend("c", cVSWorldEyePos) << " );" << std::endl;

			// world to light space matrix
			ss << "float4x4 gWorldToLight : register( " << fAppend("c", cVSWorldToLight) << " );" << std::endl;

			// skinning matrix palette
			if( skinned )
			{
				ss << "float3x4 gMatrixPalette[" << Gfx::tMaterial::cMaxBoneCount << "] : register( " << fAppend("c", cVSMatrixPalette) << " );" << std::endl;
			}
		}

		void fVSInputStruct( std::stringstream& ss, b32 pos, b32 normal, b32 tangent, b32 color, u32 numUvs, b32 skinned, b32 instanced )
		{
			ss << "struct " << cVSInputName << " {" << std::endl;
			if( pos )
				ss << "  float3 vP : POSITION;" << std::endl;
			if( normal )
				ss << "  float3 vN : NORMAL;" << std::endl;
			if( tangent )
				ss << "  float4 vT : TANGENT;" << std::endl;
			if( color )
				ss << "  float4 vC : COLOR;" << std::endl;
			for( u32 i = 0; i < numUvs; ++i )
				ss << "  float2 " << fAppend("vUv", i) << " : " << fAppend("TEXCOORD", i) << ";" << std::endl;
			if( skinned )
			{
				ss << "  float4 vW : BLENDWEIGHT;" << std::endl;
				ss << "  int4	vI : BLENDINDICES;" << std::endl;
			}
			if( instanced )
			{
				ss << "  float4 vLocalToWorld0 : TEXCOORD4;" << std::endl;
				ss << "  float4 vLocalToWorld1 : TEXCOORD5;" << std::endl;
				ss << "  float4 vLocalToWorld2 : TEXCOORD6;" << std::endl;
				ss << "  float4 vInstanceTint : TEXCOORD7;" << std::endl;
			}
			ss << "};" << std::endl;
		}

		void fVSOutputPsInputStruct( std::stringstream& ss, b32 psIn, b32 pos, b32 normal, b32 tangent, b32 color, u32 numUvs, u32 lightCount = 0 )
		{
			if( psIn )
				ss << "struct " << cPSInputName << " {" << std::endl;
			else
				ss << "struct " << cVSOutputName << " {" << std::endl;

			if( !psIn ) // for vertex shader only, we write clip pos
				ss << "  float4 vCP : POSITION;" << std::endl;

			const b32 vsOrLights = !psIn || lightCount > 0;

			if( vsOrLights && pos )
				ss << "  float3 vP : TEXCOORD" << numUvs << ";" << std::endl;
			if( psIn ) // write face register for pixel shader only
				ss << "  float vF : VFACE;" << std::endl;

			if( vsOrLights && normal )
				ss << "  float3 vN : NORMAL;" << std::endl;
			if( vsOrLights && tangent )
				ss << "  float4 vT : TANGENT;" << std::endl;
			if( color )
				ss << "  float4 vC : COLOR;" << std::endl;
			if( !psIn || ( vsOrLights && normal ) )
				ss << "  float4 vLightPos : TEXCOORD" << numUvs + 1 << ";" << std::endl;
			if( !normal || vsOrLights )
			{
				for( u32 i = 0; i < numUvs; ++i )
					ss << "  float2 " << fAppend("vUv", i) << " : " << fAppend("TEXCOORD", i) << ";" << std::endl;
			}
			ss << "};" << std::endl;
		}

		std::string& fVS( std::string& o, b32 pos, b32 normal, b32 tangent, b32 color, u32 numUvs, b32 skinned, b32 instanced, b32 deferred, tPlatformId pid )
		{
			std::stringstream ss;

			fVsGlobals( ss, skinned );

			fVSInputStruct( ss, pos, normal, tangent, color, numUvs, skinned, instanced );
			fVSOutputPsInputStruct( ss, false, pos, normal, tangent, color, numUvs );

			ss << cVSOutputName << " main( " << cVSInputName << " i ) {" << std::endl;

			ss << cVSOutputName << " o;" << std::endl;

			sigassert( pos );

			// compute skinned pos
			if( skinned )
			{
				ss << "float3x4 skinMatrix = gMatrixPalette[ i.vI.x ] * i.vW.x;" << std::endl;
				ss << "skinMatrix += gMatrixPalette[ i.vI.y ] * i.vW.y;" << std::endl;
				ss << "skinMatrix += gMatrixPalette[ i.vI.z ] * i.vW.z;" << std::endl;
				ss << "skinMatrix += gMatrixPalette[ i.vI.w ] * i.vW.w;" << std::endl;
				ss << "float3 pSkinned = mul( skinMatrix, float4( i.vP.xyz, 1.f ) );" << std::endl;
			}
			else
			{
				ss << "float3 pSkinned = i.vP.xyz;" << std::endl;
			}

			// compute world pos
			std::string localToWorldPosName, localToWorldNormalName;
			if( instanced )
			{
				localToWorldPosName = "localToWorldPos";
				localToWorldNormalName = localToWorldPosName;
				ss << "float3x4 " << localToWorldPosName << " = float3x4( i.vLocalToWorld0, i.vLocalToWorld1, i.vLocalToWorld2 );" << std::endl;
			}
			else
			{
				localToWorldPosName = "gLocalToWorldPos";
				localToWorldNormalName = "gLocalToWorldNormal";
			}

			ss << "float3 pworld = mul( " << localToWorldPosName << ", float4( pSkinned, 1.f ) );" << std::endl;
			// output clip pos
			ss << "o.vCP = mul( gWorldToProj, float4( pworld, 1.f ) );" << std::endl;
			// output light pos
			if( tMaterialGenBase::fShadowMapLayerCount( pid ) == 1 )
				ss << "   o.vLightPos = mul( gWorldToLight, float4( pworld, 1.f ) );" << std::endl;
			else
				ss << "   o.vLightPos = float4( pworld, 1.f );" << std::endl;
			// output modified-eye-relative
			ss << "o.vP.xyz = pworld - gWorldEyePos.xyz;" << std::endl;

			if( normal ) // transform normal to world
			{
				if( skinned )
				{
					ss << "o.vN.xyz = mul( ( float3x3 )skinMatrix, i.vN );" << std::endl;
					ss << "o.vN.xyz = mul( ( float3x3 )" << localToWorldNormalName << ", o.vN.xyz );" << std::endl;
				}
				else
					ss << "o.vN.xyz = mul( ( float3x3 )" << localToWorldNormalName << ", i.vN );" << std::endl;
			}

			if( tangent ) // transform tangent to world, but preserve w (sign for binormal)
			{
				if( skinned )
				{
					ss << "o.vT.xyz = mul( ( float3x3 )skinMatrix, i.vT.xyz );" << std::endl;
					ss << "o.vT.xyz = mul( ( float3x3 )" << localToWorldNormalName << ", o.vT.xyz );" << std::endl;
				}
				else
					ss << "o.vT.xyz = mul( ( float3x3 )" << localToWorldNormalName << ", i.vT.xyz );" << std::endl;
				ss << "o.vT.w = i.vT.w;" << std::endl;
			}

			sigassert( color ); // for now we require a vertex color
			if( color ) // pass through color
			{
				if( instanced )
					ss << "o.vC = i.vC * i.vInstanceTint;" << std::endl;
				else
					ss << "o.vC = i.vC;" << std::endl;
			}

			for( u32 i = 0; i < numUvs; ++i ) // pass through uv
				ss << fAppend("o.vUv", i) << " = " << fAppend("i.vUv", i) << ";" << std::endl;

			ss << "return o;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		void fPsGlobals( tPlatformId pid, std::stringstream& ss, b32 diffuseMap, b32 specColorMap, b32 emissiveMap, b32 normalMap )
		{
			ss << "#pragma pack_matrix(row_major)" << std::endl;

			u32 ithSampler = 0;

			const std::string shadowMapSampler = tMaterialGenBase::fShadowMapSamplerName( pid );
			ss << shadowMapSampler << " gShadowMap : register( s" << ithSampler++ << " );" << std::endl;

			if( diffuseMap )
				ss << "sampler2D gDiffuseMap : register( s" << ithSampler++ << " );" << std::endl;
			if( specColorMap )
				ss << "sampler2D gSpecColorMap : register( s" << ithSampler++ << " );" << std::endl;
			if( emissiveMap )
				ss << "sampler2D gEmissiveMap : register( s" << ithSampler++ << " );" << std::endl;
			if( normalMap )
				ss << "sampler2D gNormalMap : register( s" << ithSampler++ << " );" << std::endl;

			ss << "float4 gRgbaTint : register( " << fAppend("c", cPSRgbaTint) << " );" << std::endl;
			ss << "float4 gDiffuseUvXform : register( " << fAppend("c", cPSDiffuseUvXform) << " );" << std::endl;
			ss << "float4 gSpecColorUvXform : register( " << fAppend("c", cPSSpecColorUvXform) << " );" << std::endl;
			ss << "float4 gEmissiveUvXform : register( " << fAppend("c", cPSEmissiveUvXform) << " );" << std::endl;
			ss << "float4 gNormalUvXform : register( " << fAppend("c", cPSNormalUvXform) << " );" << std::endl;

			ss << "float3 gDiffuseColor : register( " << fAppend("c", cPSDiffuseColor) << " );" << std::endl;
			ss << "float4 gSpecColor : register( " << fAppend("c", cPSSpecColor) << " );" << std::endl;
			ss << "float3 gEmissiveColor : register( " << fAppend("c", cPSEmissiveColor) << " );" << std::endl;
			ss << "float4 gBumpDepth_SpecSize_Opacity_BackFaceFlip : register( " << fAppend("c", cPSBumpDepth_SpecSize_Opacity_BackFaceFlip) << " );" << std::endl;
			ss << "float4 gFogValues : register( " << fAppend("c", cPSFogValues) << " );" << std::endl;
			ss << "float3 gFogColor : register( " << fAppend("c", cPSFogColor) << " );" << std::endl;

			// eye position in world space
			ss << "float4 gWorldEyePos : register( " << fAppend("c", cPSWorldEyePos) << " );" << std::endl;

			// misc. shadow map values
			ss << "float4 gShadowMapEpsilon : register( " << fAppend("c", cPSShadowMapEpsilon) << " );" << std::endl;
			ss << "float4 gShadowMapEpsilon_TexelSize_Amount_Split : register( " << fAppend("c", cPSShadowMapEpsilon_TexelSize_Amount_Split) << " );" << std::endl;
			ss << "float4 gShadowMapTarget_Split : register( " << fAppend("c", cPSShadowMapTarget_Split) << " );" << std::endl;
			
			ss << "float4 gSphericalHarmonics[" << Gfx::tSphericalHarmonics::cFactorCount << "] : register( c" << cPSSphericalHarmonics << " );" << std::endl;


			// declare world to light constants
			tMaterialGenBase::fDeclareWorldToLightConstantsArray( pid, ss, cPSWorldToLightArray );

			// declare light constants
			tMaterialGenBase::fDeclareLightConstantsArray( ss, cPSLightVectorFirst );
		}

		std::string& fPS( std::string& o, b32 pos, b32 normal, b32 tangent, b32 color, u32 numUvs,
					u32 diffuseUvSet, u32 specColorUvSet, u32 emissiveUvSet, u32 normalUvSet, b32 doSpec, u32 lightCount, b32 deferred, tPlatformId pid )
		{
			// TODO see if this makes any difference
			const b32 renormalizeTangentSpace = true;

			std::stringstream ss;

			fPsGlobals( pid, ss, diffuseUvSet < numUvs, specColorUvSet < numUvs, emissiveUvSet < numUvs, normalUvSet < numUvs );

			fVSOutputPsInputStruct( ss, true, pos, normal, tangent, color, numUvs, lightCount );

			const std::string deferredOutputs = deferred ? ", out float4 oNormal : COLOR1, out float4 oEmissive : COLOR2, out float4 oAmbient : COLOR3" : "";

			ss << "void main( " << cPSInputName << " i, out float4 oColor : COLOR0" << deferredOutputs << " )" << std::endl;
			ss << "{" << std::endl;

			if( lightCount > 0 && tangent )
			{
				// we should only have a tangent if we have a normal map
				sigassert( normalUvSet < numUvs );

				// transform uv
				ss << "float2 normalUv = " << fAppend("i.vUv", normalUvSet) << " * gNormalUvXform.xy + gNormalUvXform.zw;" << std::endl;

				// sample normal map
				ss << "float4 nsample = tex2D( gNormalMap, normalUv ) * 2 - 1;" << std::endl;

				// for now we assume DXT5 using DXTN compression, where x is in alpha, z is in green, and we reconstruct y.
				// Our tangent space is different from what you might expect, 
				// we swap tangent space y <=> z bcz we're in "maya" space, or open-gl space)
				ss << "float3 ntspace = float3( nsample.a, sqrt( 1.f - nsample.a * nsample.a - nsample.g * nsample.g ), nsample.g );" << std::endl;

				if( renormalizeTangentSpace )
				{
					ss << "i.vN.xyz = normalize( i.vN.xyz );" << std::endl;
					ss << "i.vT.xyz = normalize( i.vT.xyz );" << std::endl;
				}

				// construct binormal (using sign from tangent)
				ss << "float3 b = cross( i.vT.xyz, i.vN.xyz ) * i.vT.w;" << std::endl;
				// scale by bump strength
				ss << "ntspace.xz *= gBumpDepth_SpecSize_Opacity_BackFaceFlip.x;" << std::endl;
				// transform normal to world space
				ss << "float3 n = normalize( ntspace.x * i.vT.xyz + ntspace.y * i.vN.xyz + ntspace.z * b );" << std::endl;
			}
			else if( lightCount > 0 && normal )
			{
				// normal, but no normal mapping

				if( renormalizeTangentSpace )
					ss << "i.vN.xyz = normalize( i.vN.xyz );" << std::endl;
				// just copy normal
				ss << "float3 n = i.vN.xyz;" << std::endl;
			}
			else
			{
				// no normal, this means we're doing emissive only

				// declare emissive variable, this may get modulated by map
				ss << "float4 emissive = i.vC * float4( gEmissiveColor, gBumpDepth_SpecSize_Opacity_BackFaceFlip.z );" << std::endl;
				if( emissiveUvSet < numUvs && !normal )
				{
					// transform uv
					ss << "float2 emissiveUv = " << fAppend("i.vUv", emissiveUvSet) << " * gEmissiveUvXform.xy + gEmissiveUvXform.zw;" << std::endl;
					// sample emissive and modulate
					ss << "emissive *= tex2D( gEmissiveMap, emissiveUv );" << std::endl;
				}

				// early out, as we don't want to do any lighting
				if( deferred )
				{
					ss << "oColor = float4( 0,0,0,0 );" << std::endl;
					ss << "oNormal = float4( 0,0,0,0 );" << std::endl;
					ss << "oEmissive = gRgbaTint * emissive;" << std::endl;
					ss << "oAmbient = float4( 0,0,0,0 );" << std::endl;

					ss << "}" << std::endl;
				}
				else
				{					
					ss << "oColor = gRgbaTint * emissive;" << std::endl;
					ss << "}" << std::endl;
				}

				o = ss.str( );
				return o;
			}

			// by now we safely have a variable called 'n' representing the world space normal;
			// multiply it by the sign of the face register to achieve proper double-sided lighting;
			// note that we negate bcz our idea of 'back-face' is opposite from D3D.
			ss << "n *= min( 1.f, gBumpDepth_SpecSize_Opacity_BackFaceFlip.w " << ( pid == cPlatformXbox360 ? "+" : "-" ) << " sign( i.vF ) );" << std::endl;

			// declare emissive variable, this may get modulated by map
			ss << "float3 emissive = gEmissiveColor;" << std::endl;
			if( emissiveUvSet < numUvs )
			{
				// transform uv
				ss << "float2 emissiveUv = " << fAppend("i.vUv", emissiveUvSet) << " * gEmissiveUvXform.xy + gEmissiveUvXform.zw;" << std::endl;
				// sample emissive and modulate
				ss << "emissive *= tex2D( gEmissiveMap, emissiveUv ).xyz;" << std::endl;
			}

			// declare diffuse variable, this may get modulated by map
			ss << "float4 diffuse = i.vC * float4( gDiffuseColor, gBumpDepth_SpecSize_Opacity_BackFaceFlip.z );" << std::endl;
			if( diffuseUvSet < numUvs )
			{
				// transform uv
				ss << "float2 diffuseUv = " << fAppend("i.vUv", diffuseUvSet) << " * gDiffuseUvXform.xy + gDiffuseUvXform.zw;" << std::endl;
				// sample diffuse and modulate
				ss << "diffuse *= tex2D( gDiffuseMap, diffuseUv );" << std::endl;
			}

			if( doSpec )
			{
				// declare spec color
				ss << "float3 specular = gSpecColor.rgb;" << std::endl;
				if( specColorUvSet < numUvs )
				{
					// transform uv
					ss << "float2 specColorUv = " << fAppend("i.vUv", specColorUvSet) << " * gSpecColorUvXform.xy + gSpecColorUvXform.zw;" << std::endl;
					// sample spec color and modulate
					ss << "specular *= tex2D( gSpecColorMap, specColorUv ).rgb;" << std::endl;
				}
			}

			if( !deferred )
			{
				tMaterialGenBase::tComputeShadowParameters shadowParams;
				shadowParams.mPid						= pid;
				shadowParams.mSs						= &ss;
				shadowParams.mWorldPosName				= "i.vLightPos";
				//shadowParams.mNDotL					= "nDotL"; // dot( n, gLights[ 0 ].vN??? )
				tMaterialGenBase::fCallComputeShadowTerm( shadowParams );

				// declare diffuse light accumulator
				ss << "float3 diffAccum = 0.f;" << std::endl;
				if( doSpec ) // declare spec light accumulator
				{
					ss << "float3 specAccum = 0.f;" << std::endl;
					ss << "float specMagAccum = 0.f;" << std::endl;
				}

				if( lightCount > 0 )
				{
					// first light
					ss << "{" << std::endl;
					ss << "int iLight = 0;" << std::endl;
					tMaterialGenBase::fCallAccumulateLight(
						ss,
						doSpec,
						Gfx::tLight::cLightTypeDirection,
						"gLights[ iLight ]",
						"i.vP.xyz",
						"n",
						"gBumpDepth_SpecSize_Opacity_BackFaceFlip.y",
						"diffAccum",
						"specAccum",
						"specMagAccum",
						"1",
						true,
						"gSphericalHarmonics" );
					ss << "}" << std::endl;

					// begin light loop
					if( lightCount > 1 )
					{
						ss << "for( int iLight = 0; iLight < " << lightCount << "; ++iLight )" << std::endl;
						ss << "{" << std::endl;
						tMaterialGenBase::fCallAccumulateLight(
							ss,
							doSpec,
							Gfx::tLight::cLightTypePoint,
							"gLights[ iLight ]",
							"i.vP.xyz",
							"n",
							"gBumpDepth_SpecSize_Opacity_BackFaceFlip.y",
							"diffAccum",
							"specAccum",
							"specMagAccum",
							"1" );
						ss << "}" << std::endl;
					}
				}

				if( doSpec )
				{
					// pre-fog pixel color (modulate final combined light values with material colors)
					ss << "float4 preFog = float4( emissive + diffAccum * diffuse.rgb + specAccum * specular, diffuse.a + gSpecColor.w * specMagAccum );" << std::endl;
				}
				else
				{
					// pre-fog pixel color (modulate final combined light values with material colors)
					ss << "float4 preFog = float4( emissive + diffAccum * diffuse.rgb, diffuse.a );" << std::endl;
				}

				// tint final pre-fog color
				ss << "preFog *= gRgbaTint;" << std::endl;

				// fog is done in post process
				//tMaterialGenBase::fCallFog( ss, "preFog.rgb", "i.vP.xyz", "gFogValues", "gFogColor", "fogResult" );

				ss << "oColor = float4( preFog.rgb, preFog.a );" << std::endl;
			}
			else
			{
				ss << "oColor = float4( diffuse.rgb, 0 );" << std::endl;
				ss << "oNormal = float4( n, 0 );" << std::endl;
				ss << "oEmissive = float4( emissive, 0 );" << std::endl;

				ss << "float4 ambientAccum;" << std::endl;
				tMaterialGenBase::fSphericalLookup( ss, "n", "ambientAccum", "gSphericalHarmonics" );
				ss << "oAmbient = ambientAccum;" << std::endl;
			}

			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		void fGenerate( Gfx::tMaterialFile& mtlFile, tMaterialGenBase::tShaderBufferSet& shaderBuffers, tPlatformId pid )
		{
			shaderBuffers[ Gfx::tPhongMaterial::cShaderCategoryLightingVS ].fNewArray( cVSCount );
			mtlFile.mShaderLists[ Gfx::tPhongMaterial::cShaderCategoryLightingVS ].fNewArray( cVSCount );
			shaderBuffers[ Gfx::tPhongMaterial::cShaderCategoryLightingPS ].fNewArray( cPSCount * Gfx::tMaterial::cLightSlotCount );
			mtlFile.mShaderLists[ Gfx::tPhongMaterial::cShaderCategoryLightingPS ].fNewArray( cPSCount * Gfx::tMaterial::cLightSlotCount );
			shaderBuffers[ Gfx::tPhongMaterial::cShaderCategoryGBufferVS ].fNewArray( cVSCount );
			mtlFile.mShaderLists[ Gfx::tPhongMaterial::cShaderCategoryGBufferVS ].fNewArray( cVSCount );
			shaderBuffers[ Gfx::tPhongMaterial::cShaderCategoryGBufferPS ].fNewArray( cPSCount * Gfx::tMaterial::cLightSlotCount );
			mtlFile.mShaderLists[ Gfx::tPhongMaterial::cShaderCategoryGBufferPS ].fNewArray( cPSCount * Gfx::tMaterial::cLightSlotCount );

			std::string s;

			for( u32 i = 0; i < 2; ++i )
			{
				b32 deferred = (i == 1);
				u32 vs = deferred ? Gfx::tPhongMaterial::cShaderCategoryGBufferVS : Gfx::tPhongMaterial::cShaderCategoryLightingVS;
				u32 ps = deferred ? Gfx::tPhongMaterial::cShaderCategoryGBufferPS : Gfx::tPhongMaterial::cShaderCategoryLightingPS;

				// add "normal" (non-skinned, non-instanced) vertex shaders
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalColor, mtlFile, shaderBuffers,
					fVS( s, true, true, false, true,  0, false, false, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosColorUv0, mtlFile, shaderBuffers,
					fVS( s, true, false, false, true, 1, false, false, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalColorUv0, mtlFile, shaderBuffers,
					fVS( s, true, true, false, true,  1, false, false, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalTangentColorUv0, mtlFile, shaderBuffers,
					fVS( s, true, true, true, true,   1, false, false, deferred, pid ) );

				// add skinned vertex shaders
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalColor_Skinned, mtlFile, shaderBuffers,
					fVS( s, true, true, false, true,  0, true, false, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosColorUv0_Skinned, mtlFile, shaderBuffers,
					fVS( s, true, false, false, true, 1, true, false, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalColorUv0_Skinned, mtlFile, shaderBuffers,
					fVS( s, true, true, false, true,  1, true, false, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalTangentColorUv0_Skinned, mtlFile, shaderBuffers,
					fVS( s, true, true, true, true,   1, true, false, deferred, pid ) );

				// add instanced vertex shaders
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalColor_Instanced, mtlFile, shaderBuffers,
					fVS( s, true, true, false, true,  0, false, true, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosColorUv0_Instanced, mtlFile, shaderBuffers,
					fVS( s, true, false, false, true, 1, false, true, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalColorUv0_Instanced, mtlFile, shaderBuffers,
					fVS( s, true, true, false, true,  1, false, true, deferred, pid ) );
				tMaterialGenBase::fAddVertexShader( pid, vs, cVSPosNormalTangentColorUv0_Instanced, mtlFile, shaderBuffers,
					fVS( s, true, true, true, true,   1, false, true, deferred, pid ) );

				for( u32 iLightCount = 0; iLightCount < Gfx::tMaterial::cLightSlotCount; ++iLightCount )
				{
					// add non normal-mapped pixel shaders
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalColor, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, false, true,  0, ~0, ~0, ~0, ~0, false, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosColor_EmissiveMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, false, false, false, true, 1, ~0, ~0, 0, ~0, false, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalColor_DiffuseMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, false, true,  1,  0, ~0, ~0, ~0, false, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalColor_DiffuseMapUv0_SpecColorMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, false, true,  1,  0,  0, ~0, ~0, true, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalColor_DiffuseMapUv0_EmissiveMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, false, true,  1,  0, ~0,  0, ~0, false, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalColor_DiffuseMapUv0_SpecColorMapUv0_EmissiveMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, false, true,  1,  0,  0,  0, ~0, true, iLightCount, deferred, pid ) );

					// add normal-mapped shaders
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalTangentColor_NormalMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, true, true, 1, ~0, ~0, ~0,  0, false, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalTangentColor_DiffuseMapUv0_NormalMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, true, true, 1,  0, ~0, ~0,  0, false, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalTangentColor_DiffuseMapUv0_SpecColorMapUv0_NormalMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, true, true, 1,  0,  0, ~0,  0, true, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalTangentColor_DiffuseMapUv0_EmissiveMapUv0_NormalMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, true, true, 1,  0, ~0,  0,  0, false, iLightCount, deferred, pid ) );
					tMaterialGenBase::fAddPixelShader( pid, ps, 
						fPixelShaderSlot( cPSPosNormalTangentColor_DiffuseMapUv0_SpecColorMapUv0_EmissiveMapUv0_NormalMapUv0, iLightCount ), mtlFile, shaderBuffers,
						fPS( s, true, true, true, true, 1,  0,  0,  0,  0, true, iLightCount, deferred, pid ) );
				}
			}
		}

		void fAssignVertexAndPixelShaderSlots( Gfx::tPhongMaterial* mtl, b32 hasDiffuse, b32 hasUv0, b32 hasTangent, b32 diffuseMap, b32 normalMap, b32 specColorMap, b32 emissiveMap )
		{
			// select vertex format and vertex shader (currently one and the same thing)
			if( hasUv0 && hasTangent )
				mtl->mLightingPass.mVS = cVSPosNormalTangentColorUv0;
			else if( hasUv0 && !hasDiffuse && emissiveMap )
				mtl->mLightingPass.mVS = cVSPosColorUv0;
			else if( hasUv0 )
				mtl->mLightingPass.mVS = cVSPosNormalColorUv0;
			else
				mtl->mLightingPass.mVS = cVSPosNormalColor;
			mtl->mVertexFormatSlot = ( u32 )mtl->mLightingPass.mVS;

			// select pixel shader
			if( normalMap )
			{
				// normal mapped versions

				if( diffuseMap && specColorMap && emissiveMap )
					mtl->mLightingPass.mPS = cPSPosNormalTangentColor_DiffuseMapUv0_SpecColorMapUv0_EmissiveMapUv0_NormalMapUv0;
				else if( diffuseMap && specColorMap )
					mtl->mLightingPass.mPS = cPSPosNormalTangentColor_DiffuseMapUv0_SpecColorMapUv0_NormalMapUv0;
				else if( diffuseMap && emissiveMap )
					mtl->mLightingPass.mPS = cPSPosNormalTangentColor_DiffuseMapUv0_EmissiveMapUv0_NormalMapUv0;
				else if( diffuseMap )
					mtl->mLightingPass.mPS = cPSPosNormalTangentColor_DiffuseMapUv0_NormalMapUv0;
				else
					mtl->mLightingPass.mPS = cPSPosNormalTangentColor_NormalMapUv0;
			}
			else
			{
				// non normal mapped versions

				if( diffuseMap && specColorMap && emissiveMap )
					mtl->mLightingPass.mPS = cPSPosNormalColor_DiffuseMapUv0_SpecColorMapUv0_EmissiveMapUv0;
				else if( diffuseMap && specColorMap )
					mtl->mLightingPass.mPS = cPSPosNormalColor_DiffuseMapUv0_SpecColorMapUv0;
				else if( diffuseMap && emissiveMap )
					mtl->mLightingPass.mPS = cPSPosNormalColor_DiffuseMapUv0_EmissiveMapUv0;
				else if( diffuseMap )
					mtl->mLightingPass.mPS = cPSPosNormalColor_DiffuseMapUv0;
				else if( !hasDiffuse && emissiveMap )
					mtl->mLightingPass.mPS = cPSPosColor_EmissiveMapUv0;
				else
					mtl->mLightingPass.mPS = cPSPosNormalColor;
			}
		}
	};


	struct tPhongShadowMapPassGen : public Gfx::tPhongMaterial::tShadowMapPass
	{
		void fVsGlobals( std::stringstream& ss, b32 skinned, b32 dualParaboloid )
		{
			ss << "#pragma pack_matrix(row_major)" << std::endl;

			// local to world position matrix
			ss << "float3x4 gLocalToWorldPos : register( " << fAppend("c", cVSLocalToWorldPos) << " );" << std::endl;

			// local to world position matrix
			ss << "float3x4 gLocalToWorldNormal : register( " << fAppend("c", cVSLocalToWorldNorm) << " );" << std::endl;

			// combined view-projection matrix (world to projection)
			ss << "float4x4 gWorldToProj : register( " << fAppend("c", cVSWorldToProj) << " );" << std::endl;

			if( dualParaboloid )
			{
				ss << "float4 gShadowMapSplits : register( " << fAppend("c", cVSShadowMapSplits) << " );" << std::endl;
			}

			// skinning matrix palette
			if( skinned )
			{
				ss << "float3x4 gMatrixPalette[" << Gfx::tMaterial::cMaxBoneCount << "] : register( " << fAppend("c", cVSMatrixPalette) << " );" << std::endl;
			}
		}

		void fVSInputStruct( std::stringstream& ss, b32 hasCutout, b32 skinned, b32 instanced )
		{
			ss << "struct " << cVSInputName << " {" << std::endl;
			ss << "  float3 vP : POSITION;" << std::endl;
			if( skinned )
			{
				ss << "  float4 vW : BLENDWEIGHT;" << std::endl;
				ss << "  int4	vI : BLENDINDICES;" << std::endl;
			}
			if( hasCutout )
				ss << " float2 vUv : TEXCOORD0;" << std::endl;
			if( instanced )
			{
				ss << "  float4 vLocalToWorld0 : TEXCOORD4;" << std::endl;
				ss << "  float4 vLocalToWorld1 : TEXCOORD5;" << std::endl;
				ss << "  float4 vLocalToWorld2 : TEXCOORD6;" << std::endl;
				ss << "  float4 vInstanceTint : TEXCOORD7;" << std::endl;
			}
			ss << "};" << std::endl;
		}

		void fVSOutputPsInputStruct( std::stringstream& ss, b32 psIn, b32 hasCutout )
		{
			if( psIn )
				ss << "struct " << cPSInputName << " {" << std::endl;
			else
				ss << "struct " << cVSOutputName << " {" << std::endl;

			if( !psIn ) // for vertex shader only, we write clip pos
				ss << "  float4 vCP : POSITION;" << std::endl;

			ss << "float3 vDepth : TEXCOORD0;" << std::endl;


			if( hasCutout )
				ss << " float2 vUv : TEXCOORD1;" << std::endl;

			ss << "};" << std::endl;
		}

		std::string& fVS( std::string& o, b32 doCutout, b32 skinned, b32 instanced, b32 dualParaboloid )
		{
			std::stringstream ss;

			fVsGlobals( ss, skinned, dualParaboloid );
			fVSInputStruct( ss, doCutout, skinned, instanced );
			fVSOutputPsInputStruct( ss, false, doCutout );

			ss << cVSOutputName << " main( " << cVSInputName << " i ) {" << std::endl;

			ss << cVSOutputName << " o;" << std::endl;

			// compute skinned pos
			if( skinned )
			{
				ss << "float3x4 skinMatrix = gMatrixPalette[ i.vI.x ] * i.vW.x;" << std::endl;
				ss << "skinMatrix += gMatrixPalette[ i.vI.y ] * i.vW.y;" << std::endl;
				ss << "skinMatrix += gMatrixPalette[ i.vI.z ] * i.vW.z;" << std::endl;
				ss << "skinMatrix += gMatrixPalette[ i.vI.w ] * i.vW.w;" << std::endl;
				ss << "float3 pSkinned = mul( skinMatrix, float4( i.vP.xyz, 1.f ) );" << std::endl;
			}
			else
			{
				ss << "float3 pSkinned = i.vP.xyz;" << std::endl;
			}

			// compute world pos
			if( instanced )
				ss << "float3 pworld = mul( float3x4( i.vLocalToWorld0, i.vLocalToWorld1, i.vLocalToWorld2 ), float4( pSkinned, 1.f ) );" << std::endl;
			else
				ss << "float3 pworld = mul( gLocalToWorldPos, float4( pSkinned, 1.f ) );" << std::endl;
			// output clip pos
			ss << "o.vCP = mul( gWorldToProj, float4( pworld, 1.f ) );" << std::endl;

			// store depth
			if( dualParaboloid )
			{
				//http://gamedevelop.eu/en/tutorials/dual-paraboloid-shadow-mapping.htm

				ss << "o.vCP /= o.vCP.w;" << std::endl;
				ss << "float fLength = length(o.vCP.xyz);" << std::endl;
				ss << "o.vCP /= fLength;" << std::endl;
				ss << "o.vCP.x /= o.vCP.z + 1.0;" << std::endl;
				ss << "o.vCP.y /= o.vCP.z + 1.0;" << std::endl;

				ss << "float nearP = gShadowMapSplits.x;" << std::endl;	// first split subs as near plane
				ss << "float farP = gShadowMapSplits.y;" << std::endl;	// second split subs as far plane

				ss << "float clipDepth = o.vCP.z;" << std::endl;
				ss << "o.vCP.w = 1.0;" << std::endl;
				ss << "o.vCP.z = (fLength - nearP) / (farP - nearP);" << std::endl;
				ss << "o.vDepth = float3( o.vCP.z, 1.0, clipDepth );" << std::endl;
			}
			else
			{
				ss << "o.vDepth = o.vCP.zwz;" << std::endl;
			}

			if( doCutout )
				ss << "o.vUv = i.vUv;" << std::endl;

			ss << "return o;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		void fPsGlobals( tPlatformId pid, std::stringstream& ss, b32 doCutout )
		{
			if( doCutout )
			{
				ss << "sampler2D gDiffuseMap : register( s" << 0 << " );" << std::endl;
			}
		}

		std::string& fPS( std::string& o, b32 doCutout, tPlatformId pid, b32 dualParaboloid )
		{
			// TODO see if this makes any difference
			const b32 renormalizeTangentSpace = true;

			std::stringstream ss;

			fPsGlobals( pid, ss, doCutout );
			fVSOutputPsInputStruct( ss, true, doCutout );

			ss << "void main( " << cPSInputName << " i, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;

			if( dualParaboloid )
				ss << "	clip( i.vDepth.z );" << std::endl;

			if( doCutout )
			{
				ss << "  oColor.rgb = i.vDepth.x / i.vDepth.y;" << std::endl;
				ss << "  oColor.a = tex2D( gDiffuseMap, i.vUv ).a;" << std::endl;
			}
			else
			{
				ss << "  oColor = i.vDepth.x / i.vDepth.y;" << std::endl;
			}

			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		void fGenerate( Gfx::tMaterialFile& mtlFile, tMaterialGenBase::tShaderBufferSet& shaderBuffers, tPlatformId pid )
		{
			shaderBuffers[ Gfx::tPhongMaterial::cShaderCategoryShadowMapVS ].fNewArray( cVSCount );
			mtlFile.mShaderLists[ Gfx::tPhongMaterial::cShaderCategoryShadowMapVS ].fNewArray( cVSCount );
			shaderBuffers[ Gfx::tPhongMaterial::cShaderCategoryShadowMapPS ].fNewArray( cPSCount );
			mtlFile.mShaderLists[ Gfx::tPhongMaterial::cShaderCategoryShadowMapPS ].fNewArray( cPSCount );

			std::string s;

			// add normal vertex shaders
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepth, mtlFile, shaderBuffers,
				fVS( s, false, false, false, false ) );
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepthCutOut, mtlFile, shaderBuffers,
				fVS( s, true, false, false, false ) );
			// add skinned vertex shaders
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepth_Skinned, mtlFile, shaderBuffers,
				fVS( s, false, true, false, false ) );
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepthCutOut_Skinned, mtlFile, shaderBuffers,
				fVS( s, true, true, false, false ) );
			// Dual Paraboloid variants
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepth_DP, mtlFile, shaderBuffers,
				fVS( s, false, false, false, true ) );
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepthCutOut_DP, mtlFile, shaderBuffers,
				fVS( s, true, false, false, true ) );
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepth_Skinned_DP, mtlFile, shaderBuffers,
				fVS( s, false, true, false, true ) );
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepthCutOut_Skinned_DP, mtlFile, shaderBuffers,
				fVS( s, true, true, false, true ) );


			// add instanced vertex shaders
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepth_Instanced, mtlFile, shaderBuffers,
				fVS( s, false, false, true, false ) );
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapVS, cVSDepthCutOut_Instanced, mtlFile, shaderBuffers,
				fVS( s, true, false, true, false ) );


			// add pixel shaders
			tMaterialGenBase::fAddPixelShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapPS, 
				cPSDepth, mtlFile, shaderBuffers,
				fPS( s, false, pid, false ) );
			tMaterialGenBase::fAddPixelShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapPS, 
				cPSDepthCutOut, mtlFile, shaderBuffers,
				fPS( s, true, pid, false ) );
			// Dual Paraboloid variants
			tMaterialGenBase::fAddPixelShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapPS, 
				cPSDepth_DP, mtlFile, shaderBuffers,
				fPS( s, false, pid, true ) );
			tMaterialGenBase::fAddPixelShader( pid, Gfx::tPhongMaterial::cShaderCategoryShadowMapPS, 
				cPSDepthCutOut_DP, mtlFile, shaderBuffers,
				fPS( s, true, pid, true ) );
		}

		void fAssignVertexAndPixelShaderSlots( Gfx::tPhongMaterial* mtl, b32 hasDiffuse )
		{
			if( hasDiffuse && mtl->fGetRenderState( ).fGetCutOutThreshold( ) > 0 )
			{
				mtl->mShadowMapPass.mVS = cVSDepthCutOut;
				mtl->mShadowMapPass.mPS = cPSDepthCutOut;
			}
			else
			{
				mtl->mShadowMapPass.mVS = cVSDepth;
				mtl->mShadowMapPass.mPS = cPSDepth;
			}
		}
	};


	tPhongMaterialGen::tPhongMaterialGen( )
		: mDiffuseColor( 0.f )
		, mSpecColor( 0.f )
		, mEmissiveColor( 0.f )
		, mOpacityColor( 1.f )
		, mBumpDepth( 0.f )
		, mSpecSize( 0.5f )
	{
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPhongMaterialGen::tUvParameters& o )
	{
		s( "MirrorUv", o.mMirrorUv );
		s( "WrapUv", o.mWrapUv );
		s( "RepeatUv", o.mRepeatUv );
		s( "OffsetUv", o.mOffsetUv );
		s( "UvSetName", o.mUvSetName );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPhongMaterialGen& o )
	{
		s( "DiffuseColor", o.mDiffuseColor );
		s( "DiffuseMapPath", o.mDiffuseMapPath );
		s( "DiffuseUvParams", o.mDiffuseUvParams );

		s( "SpecColor", o.mSpecColor );
		s( "SpecColorMapPath", o.mSpecColorMapPath );
		s( "SpecColorUvParams", o.mSpecColorUvParams );

		s( "EmissiveColor", o.mEmissiveColor );
		s( "EmissiveMapPath", o.mEmissiveMapPath );
		s( "EmissiveUvParams", o.mEmissiveUvParams );

		s( "OpacityColor", o.mOpacityColor );
		s( "OpacityeMapPath", o.mOpacityMapPath );
		s( "OpacityUvParams", o.mOpacityUvParams );

		s( "NormalMapPath", o.mNormalMapPath );
		s( "NormalUvParams", o.mNormalUvParams );
		s( "BumpDepth", o.mBumpDepth );

		s( "SpecSize", o.mSpecSize );
	}

	void tPhongMaterialGen::fSerialize( tXmlSerializer& s )		{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tPhongMaterialGen::fSerialize( tXmlDeserializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tPhongMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		Gfx::tPhongMaterial* mtl = new Gfx::tPhongMaterial( );

		fConvertBase( mtl );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tPhongMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		Math::tVec4f dummyColor(1.f);

		const b32 diffuseMap	= fAddTexture( 
			mtl->mDiffuseColor, mtl->mDiffuseMap, mtl->mDiffuseUvXform, mDiffuseColor, mDiffuseMapPath, mDiffuseUvParams, lipFileCreator );
		const b32 specColorMap	= fAddTexture( 
			mtl->mSpecColor, mtl->mSpecColorMap, mtl->mSpecColorUvXform, mSpecColor, mSpecColorMapPath, mSpecColorUvParams, lipFileCreator );
		const b32 emissiveMap	= fAddTexture( 
			mtl->mEmissiveColor, mtl->mEmissiveMap, mtl->mEmissiveUvXform, mEmissiveColor, mEmissiveMapPath, mEmissiveUvParams, lipFileCreator );
		const b32 normalMap		= fAddTexture( 
			dummyColor, mtl->mNormalMap, mtl->mNormalUvXform, dummyColor, mNormalMapPath, mNormalUvParams, lipFileCreator );

		if( mTransparency )
			mtl->mSpecColor.w = 1.f; // for transparency, we add spec to alpha
		else
			mtl->mSpecColor.w = 0.f; // for cut-out, we DON'T add spec to alpha

		if( normalMap )
			mtl->mBumpDepth_SpecSize_Opacity_BackFaceFlip.x = mBumpDepth;
		if( specColorMap )
			mtl->mBumpDepth_SpecSize_Opacity_BackFaceFlip.y = 4.f * ( mSpecSize * 98.f + 2 );
		if( mFlipBackFaceNormal )
			mtl->mBumpDepth_SpecSize_Opacity_BackFaceFlip.w = 2.f;

		const b32 hasDiffuse = !Math::tVec3f( mDiffuseColor.x, mDiffuseColor.y, mDiffuseColor.z ).fIsZero( );
		const b32 hasUv0 = diffuseMap || specColorMap || emissiveMap || normalMap;
		const b32 hasTangent = normalMap;

		tPhongLightingPassGen lightingPassGen;
		lightingPassGen.fAssignVertexAndPixelShaderSlots( mtl, hasDiffuse, hasUv0, hasTangent, diffuseMap, normalMap, specColorMap, emissiveMap );

		tPhongShadowMapPassGen shadowMapPassGen;
		shadowMapPassGen.fAssignVertexAndPixelShaderSlots( mtl, hasDiffuse );

		mtl->fSetSkinned( skinned );
		return mtl;
	}

	b32 tPhongMaterialGen::fAddTexture( 
		Math::tVec4f& color, 
		tLoadInPlaceResourcePtr*& resourcePtr, 
		Math::tVec4f& uvXform, 
		const Math::tVec4f& inColor,
		const tFilePathPtr& path,
		const tUvParameters& uvParams, 
		tLoadInPlaceFileBase& lipFileCreator )
	{
		// copy color no matter what
		color = inColor;

		if( path.fLength( ) > 0 )
		{
			// convert uvxform
			uvXform = Math::tVec4f( 
				uvParams.mRepeatUv.x, 
				uvParams.mRepeatUv.y, 
				uvParams.mOffsetUv.x / ( fEqual(uvParams.mRepeatUv.x,0.f) ? 1.f : uvParams.mRepeatUv.x ),
				uvParams.mOffsetUv.y / ( fEqual(uvParams.mRepeatUv.y,0.f) ? 1.f : uvParams.mRepeatUv.y ) );

			// add texture resource pointer
			const tFilePathPtr binaryPath = tTextureGen::fCreateResourceNameFromInputPath( path );
			const tResourceId rid = tResourceId::fMake<Gfx::tTextureFile>( binaryPath );
			resourcePtr = lipFileCreator.fAddLoadInPlaceResourcePtr( rid );
			return true;
		}

		return false;
	}

	b32 tPhongMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		if( !tMaterialGenBase::fIsEquivalent( other ) )
			return false;

		const tPhongMaterialGen& otherPhong = static_cast<const tPhongMaterialGen&>( other );

		if( mDiffuseMapPath != otherPhong.mDiffuseMapPath )
			return false;
		if( mSpecColorMapPath != otherPhong.mSpecColorMapPath )
			return false;
		if( mEmissiveMapPath != otherPhong.mEmissiveMapPath )
			return false;
		if( mOpacityMapPath != otherPhong.mOpacityMapPath )
			return false;
		if( mNormalMapPath != otherPhong.mNormalMapPath )
			return false;

		if( !mDiffuseColor.fEqual( otherPhong.mDiffuseColor ) )
			return false;
		if( !mSpecColor.fEqual( otherPhong.mSpecColor ) )
			return false;
		if( !mEmissiveColor.fEqual( otherPhong.mEmissiveColor ) )
			return false;
		if( !mOpacityColor.fEqual( otherPhong.mOpacityColor ) )
			return false;

		if( mDiffuseMapPath.fLength( ) > 0 )
			if( mDiffuseUvParams != otherPhong.mDiffuseUvParams )
				return false;
		if( mSpecColorMapPath.fLength( ) > 0 )
			if( mSpecColorUvParams != otherPhong.mSpecColorUvParams )
				return false;
		if( mEmissiveMapPath.fLength( ) > 0 )
			if( mEmissiveUvParams != otherPhong.mEmissiveUvParams )
				return false;
		if( mOpacityMapPath.fLength( ) > 0 )
			if( mOpacityUvParams != otherPhong.mOpacityUvParams )
				return false;
		if( mNormalMapPath.fLength( ) > 0 )
		{
			if( mNormalUvParams != otherPhong.mNormalUvParams )
				return false;
			if( !fEqual( mBumpDepth, otherPhong.mBumpDepth ) )
				return false;
		}

		if( !mSpecColor.fIsZero( ) )
			if( !fEqual( mSpecSize, otherPhong.mSpecSize ) )
				return false;

		// if we made it past all the early outs, then we must be equivalent
		return true;
	}

	void tPhongMaterialGen::fGetTextureResourcePaths( tFilePathPtrList& resourcePathsOut )
	{
		if( mDiffuseMapPath.fLength( ) > 0 )
			resourcePathsOut.fFindOrAdd( mDiffuseMapPath );
		if( mSpecColorMapPath.fLength( ) > 0 )
			resourcePathsOut.fFindOrAdd( mSpecColorMapPath );
		if( mEmissiveMapPath.fLength( ) > 0 )
			resourcePathsOut.fFindOrAdd( mEmissiveMapPath );
		if( mOpacityMapPath.fLength( ) > 0 )
			resourcePathsOut.fFindOrAdd( mOpacityMapPath );
		if( mNormalMapPath.fLength( ) > 0 )
			resourcePathsOut.fFindOrAdd( mNormalMapPath );
	}

	tFilePathPtr tPhongMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tPhongMaterial::fMaterialFilePath( );
	}

	void tPhongMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tPhongMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tPhongMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( Gfx::tPhongMaterial::cShaderCategoryCount );
		tShaderBufferSet shaderBuffers( Gfx::tPhongMaterial::cShaderCategoryCount );

		tPhongLightingPassGen lightingPassGen;
		lightingPassGen.fGenerate( mtlFile, shaderBuffers, pid );

		tPhongShadowMapPassGen shadowMapPassGen;
		shadowMapPassGen.fGenerate( mtlFile, shaderBuffers, pid );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tPhongMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tPhongMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tPhongMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

