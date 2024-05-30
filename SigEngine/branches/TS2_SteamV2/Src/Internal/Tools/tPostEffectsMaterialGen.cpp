#include "ToolsPch.hpp"
#include "Gfx/tPostEffectsMaterial.hpp"
#include "tPostEffectsMaterialGen.hpp"

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

			ss << "tOutput main( float2 vInPos : POSITION )" << std::endl;
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

		std::string& fPsBlendUsingDepth( std::string& o, b32 useOverlay )
		{
			std::stringstream ss;

			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;
			ss << "float4 gTint1 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint1 << " );" << std::endl;
			ss << "float4 gDepthBlendValues : register( c" << Gfx::tPostEffectsMaterial::cPSTargetDepthValues << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;
			ss << "sampler2D gSource1 : register( s1 );" << std::endl;
			ss << "sampler2D gDepth : register( s2 );" << std::endl;

			if( useOverlay )
				ss << "sampler2D gOverlay : register( s3 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;

			ss << "   float middleDepth = gDepthBlendValues.x;" << std::endl;
			ss << "   float focalPlane = gDepthBlendValues.y;" << std::endl;
			ss << "   float blurScale = gDepthBlendValues.z;" << std::endl;
			ss << "   float minBlur = gDepthBlendValues.w;" << std::endl;

			ss << "   float depth = tex2D( gDepth, vInUv.xy ).r;" << std::endl;
			ss << "   depth = depth * depth * depth;" << std::endl;

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

			if( useOverlay )
			{
				ss << "  oColor.rgb *= overlay.rgb;" << std::endl;
			}

			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		std::string& fPsSaturation( std::string& o )
		{
			std::stringstream ss;

			ss << "float4 gTint0 : register( c" << Gfx::tPostEffectsMaterial::cPSRgbaTint0 << " );" << std::endl;
			ss << "float4 gSaturation : register( c" << Gfx::tPostEffectsMaterial::cPSSaturation << " );" << std::endl;
			ss << "float4 gContrast : register( c" << Gfx::tPostEffectsMaterial::cPSContrast << " );" << std::endl;

			ss << "sampler2D gSource0 : register( s0 );" << std::endl;

			ss << "void main( in float4 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   float4 sample = tex2D( gSource0, vInUv.xy );" << std::endl; 
			ss << "   float3 grayscale = dot( sample.rgb, float3( 0.2125f, 0.7154f, 0.0721f ) );" << std::endl;
			ss << "   oColor.rgb = lerp( grayscale.rgb, sample.rgb, gSaturation.rgb );" << std::endl;

			// contrast
			ss << "   oColor.rgb = gContrast.xyz * pow( oColor.rgb, gContrast.xyz );" << std::endl;

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
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderSample2x2, mtlFile, shaderBuffers, fPsSample2x2( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderGaussBlurH, mtlFile, shaderBuffers, fPsGaussBlurH( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderGaussBlurV, mtlFile, shaderBuffers, fPsGaussBlurV( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlend, mtlFile, shaderBuffers, fPsBlend( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingSource1Alpha, mtlFile, shaderBuffers, fPsBlendUsingSource1Alpha( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepth, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, false ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderSaturation, mtlFile, shaderBuffers, fPsSaturation( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderFilmGrain, mtlFile, shaderBuffers, fPsFilmGrain( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderTransform, mtlFile, shaderBuffers, fPsTransform( s ) );
		fAddPixelShader( pid, 1, Gfx::tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlay, mtlFile, shaderBuffers, fPsBlendUsingDepth( s, true ) );

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

