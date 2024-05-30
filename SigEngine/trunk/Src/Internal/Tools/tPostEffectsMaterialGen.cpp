#include "ToolsPch.hpp"
#include "Gfx/tPostEffectsMaterial.hpp"
#include "tPostEffectsMaterialGen.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	register_rtti_factory( tPostEffectsMaterialGen, false );

	namespace
	{
		std::string& fVs30( std::string& o, b32 outputUv )
		{
			std::stringstream ss;

			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float4x4 gProjView  : register( c" << Gfx::tPostEffectsMaterial::cVSWorldToProj << " );" << std::endl;
			ss << "float4 gRenderTargetDims : register( c" << Gfx::tPostEffectsMaterial::cVSRenderTargetDims << " );" << std::endl;
			ss << "float4 gViewportTLBR : register( c" << Gfx::tPostEffectsMaterial::cVSViewportXform << " );" << std::endl;

			ss << "struct tOutput" << std::endl;
			ss << "{" << std::endl;
			ss << "  float4 vCP : POSITION;" << std::endl;
			if( outputUv )
				ss << "  float4 vOutUv : TEXCOORD0;" << std::endl;
			ss << "};" << std::endl;

			ss << "tOutput main( float3 vInPos : POSITION )" << std::endl;
			ss << "{" << std::endl;
			ss << "   tOutput o;" << std::endl;

			ss << "   o.vCP = mul( gProjView, float4( vInPos.x, vInPos.y, 0.f, 1.f ) );" << std::endl;

			if( outputUv )
			{
				ss << "   o.vOutUv.xy = gViewportTLBR.xy + vInPos * ( gViewportTLBR.zw - gViewportTLBR.xy );" << std::endl;
				ss << "   o.vOutUv.xy += float2( 0.5f/gRenderTargetDims.x, 0.5f/gRenderTargetDims.y );" << std::endl;
				ss << "   o.vOutUv.zw = vInPos.xy;" << std::endl;
			}

			ss << "   return o;" << std::endl;
			ss << "}" << std::endl;


			o = ss.str( );
			return o;
		}

		std::string& fPsSampleWithWeights( std::string& o, u32 numSamples, b32 useWeights )
		{
			std::stringstream ss;

			ss << "sampler2D gSource : register( s0 );" << std::endl;

			ss << "float4 gSampleOffsets[" << numSamples << "] : register( c" << Gfx::tPostEffectsMaterial::cPSSampleOffsets << " );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   oColor = 0;" << std::endl;
			ss << "   for( int i = 0; i < " << numSamples << "; ++i )" << std::endl;
			ss << "      oColor += tex2D( gSource, vInUv.xy + gSampleOffsets[ i ].xy )";
			if( useWeights )
				ss << " * gSampleOffsets[ i ].w" << std::endl;
			ss << ";" << std::endl;
			if( !useWeights )
				ss << "   oColor /= " << numSamples << ";" << std::endl;

			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		std::string& fPsCopy( std::string& o )
		{
			std::stringstream ss;

			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   oColor = gTint0 * tex2D( gSource0, vInUv.xy );" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		std::string& fPsAdd( std::string& o )
		{
			std::stringstream ss;

			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;
			ss << "float4 gTint1 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint1 << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;
			ss << "sampler2D gSource1 : register( s1 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   oColor = gTint0 * tex2D( gSource0, vInUv.xy ) + gTint1 * tex2D( gSource1, vInUv.xy );" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		std::string& fPsSwizzle( std::string& o, Gfx::tPostEffectsMaterial::tPShaders ps )
		{
			std::stringstream ss;

			const b32 threeD = ps == Gfx::tPostEffectsMaterial::cPShaderSwizzle3D;

			ss << "float4 gTintR : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;
			ss << "float4 gTintG : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint1 << " );" << std::endl;
			ss << "float4 gTintB : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint2 << " );" << std::endl;
			ss << "float4 gTintA : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint3 << " );" << std::endl;
			if( threeD )
			{
				ss << "float4 gTransformAdd : register( c" << Gfx::tPostEffectsMaterial::cPSTransformAdd << " );" << std::endl;
				ss << "sampler3D gSource0 : register( s0 );" << std::endl;
			}
			else
			{
				ss << "sampler2D gSource0 : register( s0 );" << std::endl;
			}

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			if( threeD )
				ss << "	float4 sample = tex3D( gSource0, float3( vInUv.xy, gTransformAdd.z ) );" << std::endl;
			else
				ss << "	float4 sample = tex2D( gSource0, vInUv.xy );" << std::endl;
			ss << "	oColor = gTintR * sample.r + gTintG * sample.g + gTintB * sample.b + gTintA * sample.a;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		std::string& fPsSample2x2( std::string& o )
		{
			return fPsSampleWithWeights( o, 4, false );
		}

		std::string& fPsGaussBlurH( std::string& o )
		{
			return fPsSampleWithWeights( o, 13, true );
		}

		std::string& fPsGaussBlurV( std::string& o )
		{
			return fPsSampleWithWeights( o, 13, true );
		}

		std::string& fPsBlend( std::string& o )
		{
			std::stringstream ss;

			ss << "float4 gBlend : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaBlend << " );" << std::endl;
			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;
			ss << "float4 gTint1 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint1 << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;
			ss << "sampler2D gSource1 : register( s1 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   oColor = lerp( gTint0 * tex2D( gSource0, vInUv.xy ), gTint1 * tex2D( gSource1, vInUv.xy ), gBlend );" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		std::string& fPsBlendUsingSource1Alpha( std::string& o )
		{
			std::stringstream ss;

			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;
			ss << "float4 gTint1 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint1 << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;
			ss << "sampler2D gSource1 : register( s1 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   float4 source1 = gTint1 * tex2D( gSource1, vInUv.zw );" << std::endl;
			ss << "   oColor = lerp( gTint0 * tex2D( gSource0, vInUv.xy ), source1, source1.a );" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		void fGenerate_SaturationConstants( std::stringstream& ss )
		{
			ss << "float4 gSaturation : register( c" << Gfx::tPostEffectsMaterial::cPSSaturation << " );" << std::endl;
			ss << "float4 gContrast : register( c" << Gfx::tPostEffectsMaterial::cPSContrast << " );" << std::endl;
		}

		void fGenerate_SaturationImplementation( std::stringstream& ss, const char* inSample, const char* outSample = NULL )
		{
			sigcheckfail( inSample, return );
			if( !outSample )
				outSample = inSample;

			ss << "   // fGenerate_SaturationImplementation\n";
			ss << "   {\n";
			ss << "      float3 grayscale = dot( " << inSample << ", float3( 0.2125f, 0.7154f, 0.0721f ) );\n";
			ss << "      float3 resaturated = lerp( grayscale.rgb, " << inSample << ", gSaturation.rgb );\n";
			// contrast
			ss << "      " << outSample << " = gContrast.xyz * pow( abs( resaturated ), gContrast.xyz );\n";
			ss << "   }\n";
		}

		std::string& fPsBlendUsingDepth( std::string& o, b32 useOverlay, b32 doFog, b32 doSaturation, tPlatformId pid )
		{
			std::stringstream ss;

			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;
			ss << "float4 gTint1 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint1 << " );" << std::endl;
			ss << "float4 gDepthBlendValues : register( c" << Gfx::tPostEffectsMaterial::cPSTargetDepthValues << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;
			ss << "sampler2D gSource1 : register( s1 );" << std::endl;
			ss << "sampler2D gDepth : register( s2 );" << std::endl;

			if( useOverlay )
				ss << "sampler2D gOverlay : register( s3 );" << std::endl;

			if( doFog )
			{
				ss << "float4x4 gProjToView : register( c" << Gfx::tPostEffectsMaterial::cPSProjToView << " );" << std::endl;
				ss << "float4x4 gProjToWorld : register( c" << Gfx::tPostEffectsMaterial::cPSProjToWorld << " );" << std::endl;
				ss << "float4 gFogParams : register( c" << Gfx::tPostEffectsMaterial::cPSFogParams << " );" << std::endl;
				ss << "float4 gFogColor : register( c" << Gfx::tPostEffectsMaterial::cPSFogColor << " );" << std::endl;
				ss << "float4 gVerticalFogParams : register( c" << Gfx::tPostEffectsMaterial::cPSVerticalFogParams << " );" << std::endl;
				ss << "float4 gVerticalFogColor : register( c" << Gfx::tPostEffectsMaterial::cPSVerticalFogColor << " );" << std::endl;
				ss << "float4 gNearFar : register( c" << Gfx::tPostEffectsMaterial::cPSNearFarPlanes << " );" << std::endl;
			}

			if( doSaturation )
			{
				ss << "float4 gTint2 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint2 << " );" << std::endl;
				fGenerate_SaturationConstants( ss );
			}

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;

			ss << "   float middleDepth = gDepthBlendValues.x;" << std::endl;
			ss << "   float focalPlane = gDepthBlendValues.y;" << std::endl;
			ss << "   float blurScale = gDepthBlendValues.z;" << std::endl;
			ss << "   float minBlur = gDepthBlendValues.w;" << std::endl;

			if( pid == cPlatformXbox360 )
				ss << "   float viewDepth = 1.f - tex2D( gDepth, vInUv.xy ).r;" << std::endl;
			else
				ss << "   float viewDepth = tex2D( gDepth, vInUv.xy ).r;" << std::endl;
			ss << "   float depth = viewDepth * viewDepth * viewDepth;" << std::endl;

			if( useOverlay )
			{
				ss << "float4 overlay = tex2D( gOverlay, vInUv.xy );" << std::endl;
				ss << "minBlur = saturate( minBlur + overlay.a );" << std::endl;
			}

			ss << "   float depthHi = 0.5f * ( max( 0.f, depth - middleDepth ) / ( 1.f - middleDepth ) );" << std::endl;
			ss << "   float depthLo = 0.5f - 0.5f * ( max( 0.f, middleDepth - depth ) / middleDepth );" << std::endl;
			ss << "   depth = depthLo + depthHi;" << std::endl;

			ss << "   float t = saturate( abs( depth - focalPlane ) * blurScale );" << std::endl;
			ss << "   t = max( minBlur, t * t );" << std::endl;
			//ss << "   oColor = float4( t, t, t, 1.f );" << std::endl;
			ss << "   oColor = lerp( gTint0 * tex2D( gSource0, vInUv.xy ), gTint1 * tex2D( gSource1, vInUv.xy ), t );" << std::endl;

			if( doFog )
			{
				ss << "	// compute and apply fog" << std::endl;
				ss << "	float x = vInUv.x * 2 - 1;" << std::endl;
				ss << "	float y = (1-vInUv.y) * 2 - 1;" << std::endl;
				ss << "	float4 vProjectedPos = float4(x, y, viewDepth, 1.0f);" << std::endl;  

				// World space Y depth.
				{
					ss << " {" << std::endl;
					ss << "	 float4 vPositionVS = mul( gProjToWorld, vProjectedPos );" << std::endl;
					ss << "  float worldDepth = vPositionVS.y / vPositionVS.w;" << std::endl;

					ss << "	 float fogginess = saturate( ( worldDepth - gVerticalFogParams.x ) / gVerticalFogParams.y );" << std::endl;
					ss << "	 fogginess = clamp( fogginess * fogginess, gVerticalFogParams.z, gVerticalFogParams.w );" << std::endl;
					ss << "	 oColor.rgb = lerp( oColor.rgb, gVerticalFogColor, fogginess );" << std::endl;
					ss << " }" << std::endl;
				}

				// Camera space depth
				{
					ss << " {" << std::endl;
					ss << "	 float4 vPositionVS = mul( gProjToView, vProjectedPos );" << std::endl;
					ss << "  float worldDepth = vPositionVS.z / vPositionVS.w + gNearFar.x; // + gNearFar.z * vPositionVS.z;" << std::endl;
					//ss << "	 float worldDepth = vPositionVS.z / vPositionVS.w;" << std::endl;

					ss << "	 float fogginess = saturate( ( worldDepth - gFogParams.x ) / gFogParams.y );" << std::endl;
					ss << "	 fogginess = clamp( fogginess * fogginess, gFogParams.z, gFogParams.w );" << std::endl;
					ss << "	 oColor.rgb = lerp( oColor.rgb, gFogColor, fogginess );" << std::endl;
					ss << " }" << std::endl;
				}
			}

			if( useOverlay )
			{
				ss << "  oColor.rgb *= overlay.rgb;" << std::endl;
			}

			if( doSaturation )
			{
				fGenerate_SaturationImplementation( ss, "oColor.rgb" );
				ss << "  oColor *= gTint2;" << std::endl;
			}

			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		std::string& fPsSaturation( std::string& o )
		{
			std::stringstream ss;

			fGenerate_SaturationConstants( ss );
			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   float4 sample = tex2D( gSource0, vInUv.xy );" << std::endl; 
			fGenerate_SaturationImplementation( ss, "sample.rgb", "oColor.rgb" );
			ss << "   oColor.a = sample.a;" << std::endl;
			ss << "   oColor *= gTint0;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}
		
		std::string& fPsFilmGrain( std::string& o )
		{
			std::stringstream ss;

			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;
			ss << "float4 gSaturation : register( c" << Gfx::tPostEffectsMaterial::cPSSaturation << " );" << std::endl;
			ss << "float4 gFilmGrainFlicker : register( c" << Gfx::tPostEffectsMaterial::cPSFilmGrainFlicker << " );" << std::endl;
			ss << "float4 gFilmGrainOffsets0 : register( c" << Gfx::tPostEffectsMaterial::cPSFilmGrainOffsets0 << " );" << std::endl;
			ss << "float4 gFilmGrainOffsets1 : register( c" << Gfx::tPostEffectsMaterial::cPSFilmGrainOffsets1 << " );" << std::endl;

			ss << "sampler2D gScene : register( s0 );" << std::endl;
			ss << "sampler2D gFilmGrain : register( s1 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   " << std::endl;
			ss << "   float4 scene = tex2D( gScene, vInUv.xy );" << std::endl;
			ss << "   float3 grayscale = dot( scene.rgb, float3( 0.2125f, 0.7154f, 0.0721f ) );" << std::endl;
			ss << "   scene.rgb = gTint0.rgb * lerp( grayscale.rgb, scene.rgb, gSaturation.rgb );" << std::endl;
			ss << "   " << std::endl;
			ss << "   float filmGrainR = tex2D( gFilmGrain, vInUv.xy + gFilmGrainOffsets0.xy ).r;" << std::endl;
			ss << "   float filmGrainG = tex2D( gFilmGrain, vInUv.xy + gFilmGrainOffsets0.zw ).g;" << std::endl;
			ss << "   float filmGrainB = tex2D( gFilmGrain, vInUv.xy + gFilmGrainOffsets1.xy ).b;" << std::endl;
			ss << "   float filmGrainA = tex2D( gFilmGrain, vInUv.xy + gFilmGrainOffsets1.zw ).a;" << std::endl;
			ss << "   " << std::endl;
			ss << "   oColor.rgb = scene.rgb + dot( gFilmGrainFlicker, float4( filmGrainR, filmGrainG, filmGrainB, filmGrainA ) );" << std::endl;
			ss << "   oColor.a = gTint0.a * scene.a;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		std::string& fPsTransform( std::string& o )
		{
			std::stringstream ss;

			ss << "float4 gTransformAdd : register( c" << Gfx::tPostEffectsMaterial::cPSTransformAdd << " );" << std::endl;
			ss << "float4 gTransformMul : register( c" << Gfx::tPostEffectsMaterial::cPSTransformMul << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   float4 sample = tex2D( gSource0, vInUv.xy );" << std::endl; 
			ss << "   oColor = ( sample + gTransformAdd ) * gTransformMul;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}


		std::string& fPsHighPass( std::string& o )
		{
			std::stringstream ss;

			ss << "float4 gCutoff : register( c" << Gfx::tPostEffectsMaterial::cPSTransformCutoff << " );" << std::endl;
			ss << "float4 gScale : register( c" << Gfx::tPostEffectsMaterial::cPSTransformMul << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   float4 sample = tex2D( gSource0, vInUv.xy );" << std::endl;
			ss << "   float4 clamped = max( float4( 0, 0, 0, 0 ), sample - gCutoff );" << std::endl;
			ss << "   clamped *= gScale;" << std::endl;
			ss << "   oColor = clamped + clamped * gCutoff;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}


		//http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
		//http://timothylottes.blogspot.com/2011/07/fxaa-311-released.html
		std::string& fPsFXAA( std::string& o )
		{
			tFilePathPtr cHeaderPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineRootFolder( ), tFilePathPtr( "Src/Internal/Tools/Resources/Shaders/" ), tFilePathPtr( "fxaa.txt" ) );
			tFilePathPtr cShaderPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineRootFolder( ), tFilePathPtr( "Src/Internal/Tools/Resources/Shaders/" ), tFilePathPtr( "fxaa_dx9.txt" ) );

			std::string header;
			FileSystem::fReadFileToString( header, cHeaderPath );

			std::string shader;
			FileSystem::fReadFileToString( shader, cShaderPath );

			std::stringstream ss;
			ss << "#define FXAA_PC 1" << std::endl;
			ss << "#define FXAA_HLSL_3 1" << std::endl;
			ss << "#define FXAA_GREEN_AS_LUMA 1" << std::endl;
			
			ss << header << std::endl;
			ss << shader << std::endl;

			o = ss.str( );
			return o;
		}
	


	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPostEffectsMaterialGen& o )
	{
	}

	void tPostEffectsMaterialGen::fSerialize( tXmlSerializer& s )		{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tPostEffectsMaterialGen::fSerialize( tXmlDeserializer& s )		{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tPostEffectsMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		Gfx::tPostEffectsMaterial* mtl = new Gfx::tPostEffectsMaterial( );

		fConvertBase( mtl );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tPostEffectsMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		return mtl;
	}

	b32 tPostEffectsMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		return tMaterialGenBase::fIsEquivalent( other );
	}

	tFilePathPtr tPostEffectsMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tPostEffectsMaterial::fMaterialFilePath( );
	}

	void tPostEffectsMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tPostEffectsMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tPostEffectsMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( 2 ); 
		tShaderBufferSet shaderBuffers( 2 ); 
		
		mtlFile.mShaderLists[ 0 ].fNewArray( Gfx::tPostEffectsMaterial::cVShaderCount );
		mtlFile.mShaderLists[ 1 ].fNewArray( Gfx::tPostEffectsMaterial::cPShaderCount );
		shaderBuffers[ 0 ].fNewArray( Gfx::tPostEffectsMaterial::cVShaderCount );
		shaderBuffers[ 1 ].fNewArray( Gfx::tPostEffectsMaterial::cPShaderCount );

		std::string s;

		// add vertex shaders
		fAddVertexShader( pid, 0, Gfx::tPostEffectsMaterial::cVShaderBasic, mtlFile, shaderBuffers, fVs30( s, false ) );
		fAddVertexShader( pid, 0, Gfx::tPostEffectsMaterial::cVShaderOutputUv, mtlFile, shaderBuffers, fVs30( s, true ) );

		// add pixel shaders
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderCopy, mtlFile, shaderBuffers, fPsCopy( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderAdd, mtlFile, shaderBuffers, fPsAdd( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderSwizzle, mtlFile, shaderBuffers, fPsSwizzle( s, Gfx::tPostEffectsMaterial::cPShaderSwizzle ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderSample2x2, mtlFile, shaderBuffers, fPsSample2x2( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderGaussBlurH, mtlFile, shaderBuffers, fPsGaussBlurH( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderGaussBlurV, mtlFile, shaderBuffers, fPsGaussBlurV( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlend, mtlFile, shaderBuffers, fPsBlend( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingSource1Alpha, mtlFile, shaderBuffers, fPsBlendUsingSource1Alpha( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepth, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, false, false, false, pid ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepthAndFog, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, false, true, false, pid ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderSaturation, mtlFile, shaderBuffers, fPsSaturation( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderFilmGrain, mtlFile, shaderBuffers, fPsFilmGrain( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderTransform, mtlFile, shaderBuffers, fPsTransform( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlay, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, true, false, false, pid ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlayAndFog, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, true, true, false, pid ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderFilterHighPass, mtlFile, shaderBuffers, fPsHighPass( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderFXAA, mtlFile, shaderBuffers, fPsFXAA( s ) );

		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepthAndSaturate, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, false, false, true, pid ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepthAndFogAndSaturate, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, false, true, true, pid ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlayAndSaturate, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, true, false, true, pid ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlayAndFogAndSaturate, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, true, true, true, pid ) );

		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderSwizzle3D, mtlFile, shaderBuffers, fPsSwizzle( s, Gfx::tPostEffectsMaterial::cPShaderSwizzle3D ) );

		// output material file
		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tPostEffectsMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tPostEffectsMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tPostEffectsMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

