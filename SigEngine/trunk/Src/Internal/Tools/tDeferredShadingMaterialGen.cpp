#include "ToolsPch.hpp"
#include "Gfx/tDeferredShadingMaterial.hpp"
#include "tDeferredShadingMaterialGen.hpp"
#include "FileSystem.hpp"
#include "tProjectFile.hpp"

namespace Sig
{
	using StringUtil::fAppend;

	register_rtti_factory( tDeferredShadingMaterialGen, false );

	namespace
	{
		b32 fShadowed( Gfx::tDeferredShadingMaterial::tPSShaders ps )
		{
			switch( ps )
			{
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight:
			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight:
				return false;

			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight_NormalShadowed:
			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight_Shadowed:
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight_Shadowed:
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight_ShadowedPCF:
				return true;

			default:
				sig_nodefault( );
				return false;
			}
		}

		b32 fShadowMapped( Gfx::tDeferredShadingMaterial::tPSShaders ps )
		{
			switch( ps )
			{
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight:
			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight:
			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight_NormalShadowed:
				return false;

			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight_Shadowed:
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight_Shadowed:
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight_ShadowedPCF:
				return true;

			default:
				sig_nodefault( );
				return false;
			}
		}

		b32 fWritesDepth( Gfx::tDeferredShadingMaterial::tPSShaders type )
		{
			return type == Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight
				|| type == Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight_NormalShadowed
				|| type == Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight_Shadowed;
		}

		b32 fOutputEmission( Gfx::tDeferredShadingMaterial::tPSShaders type )
		{
			return fWritesDepth( type );
		}

		b32 fFrom3DS( Gfx::tDeferredShadingMaterial::tPSShaders type )
		{
			switch( type )
			{
			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight:
			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight_NormalShadowed:
			case Gfx::tDeferredShadingMaterial::cPSShadersDirectionalLight_Shadowed:
				return false;
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight:
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight_Shadowed:
			case Gfx::tDeferredShadingMaterial::cPSShadersPointLight_ShadowedPCF:
				return true;
			default:
				sig_nodefault( );
				return false;
			}
		}

		b32 fDebug( Gfx::tDeferredShadingMaterial::tPSShaders type )
		{
			return false;
		}
	}

	void tDeferredShadingMaterialGen::fPackFunctions( std::stringstream& ss )
	{
		ss << std::endl << "float3 fPackNormal( float3 normal ) { return normal * 0.5 + 0.5; }" << std::endl << std::endl;
	}
	void tDeferredShadingMaterialGen::fUnPackFunctions( std::stringstream& ss )
	{
		ss << std::endl << "float3 fUnPackNormal( float3 normal ) { return normalize( (normal - 0.5) * 2.0 ); }" << std::endl << std::endl;
	}

	static void fUnPackAllDataFunction( std::stringstream& ss, tPlatformId pid )
	{
		tDeferredShadingMaterialGen::fUnPackFunctions( ss );

		ss << "void fUnPackAllData( float2 screenCoords, out float3 position, out float3 normal, out float3 diffuse, out float depth, out float specPower, out float specValue )" << std::endl;
		ss << "{" << std::endl;
		ss << "		float4 normalTap = tex2D( gGBufferNormal, screenCoords );" << std::endl;
		ss << "		normal = fUnPackNormal( normalTap.rgb );" << std::endl;
		ss << "		float4 gbuffDiffuseTap = tex2D( gGBufferDiffuse, screenCoords );" << std::endl;
		ss << "		diffuse = gbuffDiffuseTap.rgb;" << std::endl;
		ss << "		specPower = gbuffDiffuseTap.a;" << std::endl;
		ss << "		specValue = normalTap.a;" << std::endl;
		// reconstruct world position from depth
		ss << "		depth = tex2D( gDepthBuffer, screenCoords ).r;" << std::endl;

		if( pid == cPlatformXbox360 )
			ss << "		float z = 1-depth;" << std::endl;
		else
			ss << "		float z = depth;" << std::endl;

		ss << "		float x = screenCoords.x * 2 - 1;" << std::endl;
		ss << "		float y = (1-screenCoords.y) * 2 - 1;" << std::endl;
		ss << "		float4 vProjectedPos = float4(x, y, z, 1.0f);" << std::endl;  
		ss << "		float4 vPositionVS = mul( gProjToWorld, vProjectedPos );" << std::endl;
		ss << "		position = vPositionVS.xyz / vPositionVS.w;" << std::endl;
		ss << "}" << std::endl << std::endl;
	}

	struct tDeferredShadingLightPassGen : public Gfx::tDeferredShadingMaterial
	{
		// This shader simply renders a full screen quad, computing texture coordinates.
		//  This is about the most complicated way to do this but it was hijacked from post effects to keep the ball rolling.
		static std::string& fVSQuad( std::string& o )
		{
			std::stringstream ss;

			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float4x4 gProjView  : register( c" << cVSScreenCamera << " );" << std::endl;
			ss << "float4 gRenderTargetDims : register( c" << cVSRenderTargetDims << " );" << std::endl;
			ss << "float4 gViewportTLBR : register( c" << cVSViewportXform << " );" << std::endl;

			ss << "struct tOutput" << std::endl;
			ss << "{" << std::endl;
			ss << "  float4 vCP : POSITION;" << std::endl;
			ss << "  float2 vOutUv : TEXCOORD0;" << std::endl;
			ss << "};" << std::endl;

			ss << "tOutput main( float3 vInPos : POSITION )" << std::endl;
			ss << "{" << std::endl;
			ss << "   tOutput o;" << std::endl;
			ss << "   o.vCP = mul( gProjView, float4( vInPos.x, vInPos.y, 0.f, 1.f ) );" << std::endl;
			ss << "   o.vOutUv.xy = gViewportTLBR.xy + vInPos.xy * ( gViewportTLBR.zw - gViewportTLBR.xy );" << std::endl;
			ss << "   o.vOutUv.xy += float2( 0.5f/gRenderTargetDims.x, 0.5f/gRenderTargetDims.y );" << std::endl;
			ss << "   return o;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		// Render 3d geometry from the scene, generating screen coordinates.
		static std::string& fVS3D( std::string& o, b32 depthOnly )
		{
			std::stringstream ss;

			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float4x4 gProjView  : register( c" << cVSSceneCamera << " );" << std::endl;
			ss << "float4 gRenderTargetDims : register( c" << cVSRenderTargetDims << " );" << std::endl;
			ss << "float4 gViewportTLBR : register( c" << cVSViewportXform << " );" << std::endl;

			ss << "struct tOutput" << std::endl;
			ss << "{" << std::endl;
			ss << "  float4 vCP : POSITION;" << std::endl;
			if( !depthOnly )
			{
				ss << "  float4 vOutUv : TEXCOORD0;" << std::endl;
				ss << "  float2 halfPx : COLOR1;" << std::endl;
			}
			ss << "};" << std::endl;

			ss << "tOutput main( float3 vInPos : POSITION )" << std::endl;
			ss << "{" << std::endl;
			ss << "   tOutput o;" << std::endl;
			ss << "	  float4 screenP = mul( gProjView, float4( vInPos, 1.f ) );" << std::endl;
			ss << "   o.vCP = screenP;" << std::endl;
			if( !depthOnly )
			{
				ss << "   o.vOutUv = screenP;" << std::endl;
				ss << "   o.halfPx.xy = float2( 0.5f/gRenderTargetDims.x, 0.5f/gRenderTargetDims.y );" << std::endl;
			}
			ss << "   return o;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		static void fSpecTerm( std::stringstream& ss )
		{
			ss << "		float realSpecPower	= 1.0f + ( specPower * specPower ) * 250.f;" << std::endl;
			ss << "		float3 eyeToPx		= worldP - gWorldEyePos;" << std::endl;
			ss << "		float  distToEye	= length( eyeToPx );" << std::endl;
			ss << "		eyeToPx				/= distToEye;" << std::endl;
			ss << "		float3 halfVec		= normalize( -eyeToPx + lightDir );" << std::endl;
			ss << "		float nDotH			= dot( worldN, halfVec );" << std::endl;
			ss << "		float specTerm		= lit( nDotL, nDotH, realSpecPower ).z;" << std::endl;
			ss << "		float3 specLight	= frontColor * specTerm * specValue;" << std::endl;
		}

		static void fDirectionalLight( std::stringstream& ss, tPlatformId pid, tDeferredShadingMaterial::tPSShaders type )
		{
			const b32 debug = fDebug( type );

			// grab vars
			ss << "		float3 ambLight		= float3( 0,0,0 );" << std::endl; //ambient light is computed in gbuffer pass and stored in emissive component.
			ss << "		float3 lightDir		= gLights[ 0 ].mDirection.xyz;" << std::endl;
			ss << "		float  nDotL		= dot( lightDir, worldN );" << std::endl;
			ss << "		float4 iLightPos = float4( worldP, 1 );" << std::endl;

			tMaterialGenBase::tComputeShadowParameters shadowParams;
			shadowParams.mPid						= pid;
			shadowParams.mSs						= &ss;
			shadowParams.mForceLightSpaceConversion	= true;
			shadowParams.mNDotL						= "nDotL";
			if( !fShadowMapped( type ) )
				shadowParams.mShadowMapSamplerName = "";

			if( debug && fShadowMapped( type ) )
			{
				// Replace diffuseLight with debug shit
				ss << "float4 shadowDebug = float4( 0.0, 0.0, 0.0, 0.0 );\n";
				shadowParams.mDebugShadowTermName = "shadowDebug";
				tMaterialGenBase::fCallComputeShadowTerm( shadowParams );
				ss << "float3 diffuseLight = shadowDebug.rgb;\n";
				ss << "float3 specLight = float3( 0.0, 0.0, 0.0 );\n";
			}
			else
			{
				if( fShadowed( type ) )
					tMaterialGenBase::fCallComputeShadowTerm( shadowParams );
				else
					ss << "		float shadowTerm = 1.0;" << std::endl;
					
				ss << std::endl;

				ss << "		float3 frontColor	= gLights[ 0 ].mColors[ 0 ].xyz * shadowTerm;" << std::endl;
				ss << "		float3 backColor	= gLights[ 0 ].mColors[ 2 ].xyz";
				if( tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mShadowShaderGenerationSettings.mShadowBack )
					ss << " * shadowTerm";
				ss << ";" << std::endl;
			
				// diffuse light
				ss << "		float3 diffuseLight = ambLight + frontColor * max(0, fHalfLambert( nDotL )) + backColor * max(0, fHalfLambert( -nDotL ));" << std::endl;

				fSpecTerm( ss );
			}
		}

		static void fPointLight( std::stringstream& ss, tPlatformId pid, tDeferredShadingMaterial::tPSShaders ps )
		{
			ss << "		float shadowTerm = 1.0;" << std::endl;

			ss << "		float3 lightDir		= gLights[ 0 ].mPosition.xyz;" << std::endl;
			ss << "		lightDir			-= worldP.xyz;" << std::endl;
			ss << "		float lightDist		= length( lightDir );" << std::endl;
			ss << "		lightDir			/= lightDist;" << std::endl;
			ss << "		float  nDotL		= dot( lightDir, worldN );" << std::endl;

			if( fShadowMapped( ps ) )
				fDualParaboloidShadows( ss, pid, ps );

			// grab vars
			ss << "		float3 frontColor	= gLights[ 0 ].mColors[ 0 ].xyz * shadowTerm;" << std::endl;
			ss << "		float3 backColor	= gLights[ 0 ].mColors[ 2 ].xyz";
			if( tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mShadowShaderGenerationSettings.mShadowBack )
				ss << " * shadowTerm";
			ss << ";" << std::endl;

			tMaterialGenBase::fCallLightAttenuate( ss, Gfx::tLight::cLightTypePoint, "attenFactor", "gLights[ 0 ]", "lightDist" );

			// diffuse light
			ss << "		float3 diffuseLight = frontColor * max(0, fHalfLambert( nDotL )) + backColor * max(0, fHalfLambert( -nDotL ));" << std::endl;
			ss << "		diffuseLight		*= attenFactor;" << std::endl;

			fSpecTerm( ss );

			ss << "		specLight			*= attenFactor;" << std::endl;
		}

		static const tProjectFile::tShadowShaderGenerationSettings& fShadowGenSettings( )
		{
			return tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mShadowShaderGenerationSettings;
		}

		static void fDualParaboloidShadows_CalculatevPosDPAndfLength( std::stringstream& ss, u32 layer )
		{
			ss << "				// fCalculatevPosDPAndLength( ss, layer=" << layer << " )\n";
			ss << "				{\n";
			ss << "					// transform into lightspace\n";
			ss << "					vPosDP = mul( gWorldToLightArray[ " << layer << " ], float4(worldP, 1.0f) ).xyz;\n";
			ss << "					fLength = length(vPosDP);\n";
			ss << "					vPosDP /= fLength;\n";
			ss << "				}\n\n";
		}

		static void fDualParaboloidShadows_CalculateTexCoord( std::stringstream& ss, const char* z )
		{
			ss << "				// fCalculateTexCoord( ss, z=\"" << z << "\" )\n";
			ss << "				{\n";
			ss << "					float vposScale = 0.5 / (1.0 + vPosDP.z);\n";
			ss << "					vTexCoord.xy = vPosDP.xy * vposScale + float2( 0.5, 0.5 );\n";
			ss << "					vTexCoord.y = 1.0 - vTexCoord.y;\n";
			ss << "					vTexCoord.z = " << z << ";\n";
			ss << "				}\n\n";
		}

		static void fDualParaboloidShadows_SampleShadowMap( std::stringstream& ss, tPlatformId pid, tDeferredShadingMaterial::tPSShaders ps )
		{
			const b32 supportsLayers = tMaterialGenBase::fShadowMapLayerCount( pid ) > 1;
			const char* const tex23D = supportsLayers ? "tex3D" : "tex2D";
			const char* const swiz23 = supportsLayers ? ".xyz" : ".xy";

			ss << "				// fDualParaboloidShadows_ShadowMap( ss, pid="<<pid<<", ps="<<ps<<" )\n";
			ss << "				{\n";

			switch( ps )
			{
			case tDeferredShadingMaterial::cPSShadersPointLight_Shadowed:
				ss << "				float fDPDepth = " << tex23D << "( gShadowMap, vTexCoord" << swiz23 << " ).r;		" << std::endl;	

				if( pid == cPlatformXbox360 )
					ss << "				fDPDepth = 1.0 - fDPDepth; // Handle 360 inverted depth buffer\n";

				ss << "				mapShadowing_ = smoothstep( startShadowFadeIn_, endShadowFadeIn_, fDPDepth );\n";
				break;
			case tDeferredShadingMaterial::cPSShadersPointLight_ShadowedPCF:
				{

					ss << "				float2 shadowTexel_ = vTexCoord.xy * g_shadowResolution_ - float2( 0.5, 0.5 );\n";
					ss << "				float2 shadowLerps_ = frac( shadowTexel_ );\n";
					ss << "				vTexCoord.xy = (shadowTexel_ - shadowLerps_) * g_shadowResolutionRcp_;\n";

					ss << "				float3 dx = float3( g_shadowResolutionRcp_, 0.0, 0.0 );\n";
					ss << "				float3 dy = float3( 0.0, g_shadowResolutionRcp_, 0.0 );\n";
					ss << "				float sample00 = " << tex23D << "( gShadowMap, ( vTexCoord           )" << swiz23 << " ).r;\n";
					ss << "				float sample01 = " << tex23D << "( gShadowMap, ( vTexCoord      + dy )" << swiz23 << " ).r;\n";
					ss << "				float sample10 = " << tex23D << "( gShadowMap, ( vTexCoord + dx      )" << swiz23 << " ).r;\n";
					ss << "				float sample11 = " << tex23D << "( gShadowMap, ( vTexCoord + dx + dy )" << swiz23 << " ).r;\n";
					ss << "				float4 samples = float4( sample00, sample01, sample10, sample11 );\n";
					if( pid == cPlatformXbox360 )
						ss << "				samples = float4( 1.0, 1.0, 1.0, 1.0 ) - samples; // Handle 360 inverted depth buffer\n";
					ss << "				samples = smoothstep( startShadowFadeIn_, endShadowFadeIn_, samples );\n";
					ss << "\n";
					ss << "				float2 sampleN_ = lerp( samples.xz, samples.yw, shadowLerps_.y );\n";
					ss << "				mapShadowing_ = lerp( sampleN_.x, sampleN_.y, shadowLerps_.x );\n";
				}
				break;
			default:
				sig_nodefault( );
				break;
			}

			ss << "				}\n\n";
		}

		static void fDualParaboloidShadows( std::stringstream& ss, tPlatformId pid, tDeferredShadingMaterial::tPSShaders ps )
		{
			sigassert( fShadowMapped( ps ) );

			b32 supportsLayers = tMaterialGenBase::fShadowMapLayerCount( pid ) > 1;

			{
				// texcoord-calculation is the same calculation as in the Depth-VS,
				// but texcoords have to be in range [0, 1]

				ss << "		// Calculate dual paraboloid shadows\n";
				ss << "		{\n";
				ss << "			float g_fNear = gShadowMapTarget_Split.y;" << std::endl;					// first split subs as near plane
				ss << "			float g_fFar = gShadowMapEpsilon_TexelSize_Amount_Split.w;" << std::endl;	// second split subs as far plane
				ss << "			float g_shadowResolution_ = gShadowMapEpsilon_TexelSize_Amount_Split.y;\n";
				ss << "			float g_shadowResolutionRcp_ = 1.0 / g_shadowResolution_;\n";
					
				
				ss << "			float fLength;\n";
				ss << "			float3 vPosDP;\n";
				ss << "			float3 vTexCoord;\n";

				ss << "			// How much shadow applies to this pixel\n";
				ss << "			float shadowAmount = 0.0;\n";

				ss << "			// Shadow Map Shadow Value\n";
				ss << "			{\n";
				// 0.6s: arbitrary reduction of the epsilon to avoid peter panning.  Feel free to move into e.g. tProjectFile or something later.
				fDualParaboloidShadows_CalculatevPosDPAndfLength( ss, 0 );
				ss << "				float fSceneDepth = (fLength - g_fNear) / (g_fFar - g_fNear);\n";
				ss << "				float startShadowFadeIn_ = fSceneDepth - gShadowMapEpsilon_TexelSize_Amount_Split.x * 0.6;\n";
				ss << "				float endShadowFadeIn_ = startShadowFadeIn_ - gShadowMapEpsilon_TexelSize_Amount_Split.x * 0.6;\n";
				ss << "				float mapShadowing_ = 0.0;\n";
				ss << "\n";
				ss << "				if(vPosDP.z >= 0.0f)" << std::endl;
				ss << "				{					" << std::endl;
				fDualParaboloidShadows_CalculateTexCoord( ss, "0.25" );
				fDualParaboloidShadows_SampleShadowMap( ss, pid, ps );
				ss << "				}					" << std::endl;

				if( supportsLayers )
				{
					ss << "				else				" << std::endl;
					ss << "				{					" << std::endl;
					fDualParaboloidShadows_CalculatevPosDPAndfLength( ss, 1 );
					fDualParaboloidShadows_CalculateTexCoord( ss, "0.75" );
					fDualParaboloidShadows_SampleShadowMap( ss, pid, ps );
					ss << "				}					" << std::endl;
				}

				ss << "				shadowAmount = max( shadowAmount, mapShadowing_ );\n";
				ss << "			}\n";
				ss << "\n";

				if( fShadowGenSettings().mNormalShadowValue.mEnable )
				{
					const f32 startDeg = fShadowGenSettings().mNormalShadowValue.mAngleStart;
					const f32 endDeg = fShadowGenSettings().mNormalShadowValue.mAngleEnd;
					const f32 startSin = Math::fSin( Math::fToRadians( startDeg ) );
					const f32 endSin = Math::fSin( Math::fToRadians( endDeg ) );

					ss << "			// Normal Shadow Value\n";
					ss << "			{\n";
					ss << "				float sinStartAngle_ = " << startSin << "; // Sin("<<startDeg<<" degrees)\n";
					ss << "				float sinEndAngle_ = " << endSin << "; // Sin("<<endDeg<<" degrees)\n";
					ss << "				float normalShadowing_ = smoothstep( sinStartAngle_, sinEndAngle_, nDotL );\n";
					ss << "				normalShadowing_ *= " << fShadowGenSettings().mNormalShadowValue.mMagnitude << ";\n";
					ss << "				shadowAmount = max( shadowAmount, normalShadowing_ );\n";
					ss << "			}\n\n";
					// shadowTerm is the final value which gets directly multiplied against the lighting contribution on the pixel.
					// shadowAmount is roughly the inverse: how much shadow to apply to the pixel.  Hence the 1 - shadowAmount ...
					ss << "			shadowTerm = 1.0 - shadowAmount * gShadowMapEpsilon_TexelSize_Amount_Split.z;\n";
					ss << "		}\n";
				}
			}
		}

		static std::string& fPS( std::string& o, tPlatformId pid, tPSShaders type )
		{
			std::stringstream ss;

			const b32 writeDepth		= fWritesDepth( type );
			const b32 from3DVS			= fFrom3DS( type );
			const b32 outputEmission	= fOutputEmission( type );
			const b32 debug				= fDebug( type );

			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float4x4 gProjToWorld : register( c" << cPSProjToWorld << " );" << std::endl;
			ss << "float3   gWorldEyePos : register( c" << cPSWorldEyePos << " );" << std::endl;
			ss << "float3   gHalfLambert : register( c" << cPSHalfLambert << " );" << std::endl;

			ss << "sampler2D gDepthBuffer : register( s" << cTexUnitDepth << " );" << std::endl;
			ss << "sampler2D gGBufferDiffuse : register( s" << cTexUnitGbuffer0 << " );" << std::endl;
			ss << "sampler2D gGBufferNormal : register( s" << cTexUnitGbuffer1 << " );" << std::endl;
			
			if( outputEmission )
			{
				ss << "sampler2D gGBufferEmissive : register( s" << cTexUnitGbuffer2 << " );" << std::endl;
				ss << "sampler2D gGBufferAmbient : register( s" << cTexUnitGbuffer3 << " );" << std::endl;
			}

			tMaterialGenBase::fDeclareWorldToLightConstantsArray( pid, ss, cPSWorldToLightArray );
			tMaterialGenBase::fDeclareLightConstantsArray( ss, cPSLightVectorFirst );

			if( fShadowMapped( type ) )
			{
				const std::string shadowMapSampler = tMaterialGenBase::fShadowMapSamplerName( pid );
				ss << shadowMapSampler << " gShadowMap : register( s" << cTexUnitShadow << " );" << std::endl;
			}

			if( fShadowed( type ) )
			{
				ss << "float4 gShadowMapEpsilon : register( " << fAppend("c", cPSShadowMapEpsilon) << " );" << std::endl;
				ss << "float4 gShadowMapEpsilon_TexelSize_Amount_Split : register( " << fAppend("c", cPSShadowMapEpsilon_TexelSize_Amount_Split) << " );" << std::endl;
				ss << "float4 gShadowMapTarget_Split : register( " << fAppend("c", cPSShadowMapTarget_Split) << " );" << std::endl;
			}

			ss << std::endl;

			fUnPackAllDataFunction( ss, pid );
			ss << "float fHalfLambert( float input ) { float v = input * 0.5 + 0.5f; return v*v*sign(v); }" << std::endl;


			// Input output structures
			{
				ss << "struct PS_OUTPUT" << std::endl;
				ss << "{" << std::endl;
				ss << "	float4 oColor : COLOR0;" << std::endl;
				if( writeDepth )
					ss << " float  oDepth : DEPTH;" << std::endl;
				ss << "};" << std::endl;

				ss << "struct PS_INPUT" << std::endl;
				ss << "{" << std::endl;
				if( from3DVS )
				{
					ss << "  float4 vInUv : TEXCOORD0;" << std::endl;
					ss << "  float2 halfPx : COLOR1;" << std::endl; //todo make this a ps constant
				}
				else
				{
					ss << "  float2 vInUv : TEXCOORD0;" << std::endl;
				}
				ss << "};" << std::endl;
			}


			ss << "PS_OUTPUT main( in PS_INPUT input )" << std::endl;
			ss << "{" << std::endl;

			// unpack screen coordinates.
			if( from3DVS )
			{
				ss << "	float2 screenCoords = input.vInUv.xy / input.vInUv.w;" << std::endl;
				ss << "	screenCoords = screenCoords * 0.5f + 0.5f;" << std::endl;
				ss << "	screenCoords.y = 1-screenCoords.y;" << std::endl;
				ss << "	screenCoords += input.halfPx;" << std::endl;
			}
			else
			{
				ss << "	  float2 screenCoords = input.vInUv.xy;" << std::endl;
			}

			// unpack scene data
			ss << "		float3 worldP;" << std::endl;
			ss << "		float3 worldN;" << std::endl;
			ss << "		float3 diffuse;" << std::endl;
			ss << "		float  depth;" << std::endl;
			ss << "		float  specPower;" << std::endl; //todo
			ss << "		float  specValue;" << std::endl;  //todo
			ss << "		fUnPackAllData( screenCoords, worldP, worldN, diffuse, depth, specPower, specValue );" << std::endl;
			ss << std::endl;

			// compute lighting
			switch( type )
			{
			case cPSShadersDirectionalLight:
			case cPSShadersDirectionalLight_Shadowed:
			case cPSShadersDirectionalLight_NormalShadowed:
				fDirectionalLight( ss, pid, type );
				break;
			case cPSShadersPointLight:
			case cPSShadersPointLight_Shadowed:
			case cPSShadersPointLight_ShadowedPCF:
				fPointLight( ss, pid, type );
				break;
			default:
				sig_nodefault( );
				break;
			}

			// output result
			ss << "		PS_OUTPUT outp;" << std::endl;

			if( outputEmission )
			{
				ss << "		float3 ambientLight = tex2D( gGBufferAmbient, screenCoords ).rgb;" << std::endl;
				ss << "		float3 emissiveAccum = tex2D( gGBufferEmissive, screenCoords ).rgb;" << std::endl;

				ss << "		float3 lightAccum = diffuse * (ambientLight + diffuseLight) + specLight;" << std::endl;
				ss << "		outp.oColor = float4( emissiveAccum + lightAccum, 1 );" << std::endl;

				//// render depth
				//ss << "   outp.oColor = float4( depth, depth, depth, 1 );" << std::endl;
			}
			else
			{
				ss << "		float3 lightAccum = diffuse * diffuseLight + specLight;" << std::endl;
				ss << "		outp.oColor = float4( lightAccum, 1 );" << std::endl;

				////testing
				//ss << "   outp.oColor = float4( 0, 0, 0, 1 );" << std::endl;
			}

			if( writeDepth )
				ss << "		outp.oDepth = depth;" << std::endl;

			// render texture coordinates
			//ss << "   outp.oColor = float4( vInUv.xy, 0, 0 );" << std::endl;

			ss << "		return outp;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );

			// to dump shader to debug output.
			log_line( 0, o );

			return o;
		}

		void fGenerate( Gfx::tMaterialFile& mtlFile, tMaterialGenBase::tShaderBufferSet& shaderBuffers, tPlatformId pid )
		{
			shaderBuffers[ cShaderCategoryVS ].fNewArray( cVSShadersCount );
			mtlFile.mShaderLists[ cShaderCategoryVS ].fNewArray( cVSShadersCount );
			shaderBuffers[ cShaderCategoryPS ].fNewArray( cPSShadersCount );
			mtlFile.mShaderLists[ cShaderCategoryPS ].fNewArray( cPSShadersCount );

			std::string s;

			tMaterialGenBase::fAddVertexShader( pid, cShaderCategoryVS, cVSShaderQuad,								mtlFile, shaderBuffers, fVSQuad( s ) );
			tMaterialGenBase::fAddVertexShader( pid, cShaderCategoryVS, cVSShader3D,								mtlFile, shaderBuffers, fVS3D( s, false ) );
			tMaterialGenBase::fAddVertexShader( pid, cShaderCategoryVS, cVSShader3DDepthOnly,						mtlFile, shaderBuffers, fVS3D( s, true ) );

			tMaterialGenBase::fAddPixelShader( pid, cShaderCategoryPS, cPSShadersDirectionalLight_Shadowed,			mtlFile, shaderBuffers, fPS( s, pid, cPSShadersDirectionalLight_Shadowed ) );
			tMaterialGenBase::fAddPixelShader( pid, cShaderCategoryPS, cPSShadersPointLight,						mtlFile, shaderBuffers, fPS( s, pid, cPSShadersPointLight ) );
			tMaterialGenBase::fAddPixelShader( pid, cShaderCategoryPS, cPSShadersPointLight_Shadowed,				mtlFile, shaderBuffers, fPS( s, pid, cPSShadersPointLight_Shadowed ) );
			tMaterialGenBase::fAddPixelShader( pid, cShaderCategoryPS, cPSShadersPointLight_ShadowedPCF,			mtlFile, shaderBuffers, fPS( s, pid, cPSShadersPointLight_ShadowedPCF ) );
			tMaterialGenBase::fAddPixelShader( pid, cShaderCategoryPS, cPSShadersDirectionalLight_NormalShadowed,	mtlFile, shaderBuffers, fPS( s, pid, cPSShadersDirectionalLight_NormalShadowed ) );
			tMaterialGenBase::fAddPixelShader( pid, cShaderCategoryPS, cPSShadersDirectionalLight,					mtlFile, shaderBuffers, fPS( s, pid, cPSShadersDirectionalLight ) );
		}
	};


	tDeferredShadingMaterialGen::tDeferredShadingMaterialGen( )
	{
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tDeferredShadingMaterialGen& o )
	{
	}

	void tDeferredShadingMaterialGen::fSerialize( tXmlSerializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tDeferredShadingMaterialGen::fSerialize( tXmlDeserializer& s ) { tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tDeferredShadingMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned, b32 compressedVerts )
	{
		Gfx::tDeferredShadingMaterial* mtl = new Gfx::tDeferredShadingMaterial( );

		fConvertBase( mtl );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tDeferredShadingMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		return mtl;
	}

	b32 tDeferredShadingMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		if( !tMaterialGenBase::fIsEquivalent( other ) )
			return false;

		return true;
	}

	tFilePathPtr tDeferredShadingMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tDeferredShadingMaterial::fMaterialFilePath( );
	}

	void tDeferredShadingMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tDeferredShadingMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tDeferredShadingMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( Gfx::tDeferredShadingMaterial::cShaderCategoryCount );
		tShaderBufferSet shaderBuffers( Gfx::tDeferredShadingMaterial::cShaderCategoryCount );

		tDeferredShadingLightPassGen lightingPassGen;
		lightingPassGen.fGenerate( mtlFile, shaderBuffers, pid );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tDeferredShadingMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tDeferredShadingMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tDeferredShadingMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

