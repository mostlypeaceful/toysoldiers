#include "ToolsPch.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "tTextureGen.hpp"
#include "Gfx/tTextureFile.hpp"
#include "Gfx/tDecalMaterial.hpp"
#include "tDecalMaterialGen.hpp"
#include "tFullBrightMaterialGen.hpp"
#include "tFileWriter.hpp"

namespace Sig
{
	using StringUtil::fAppend;

	register_rtti_factory( tDecalMaterialGen, false );

	namespace
	{
		static const char cVSInputName[] = "tVSInput";
		static const char cVSOutputName[] = "tVSOutput";
		static const char cPSInputName[] = "tPSInput";
	}

	struct tDecalMaterialGenStruct : public Gfx::tDecalMaterial
	{
		void fGenerateFullBrightVs30( std::string& o )
		{
			std::stringstream ss;
			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float3x4 gLocalToWorldPos : register( c" << Gfx::tDecalMaterial::cVSLocalToWorldPos << " );" << std::endl;
			ss << "float4x4 gWorldToProj     : register( c" << Gfx::tDecalMaterial::cVSWorldToProj << " );" << std::endl;
			ss << "float4 gWorldEyePos       : register( c" << Gfx::tDecalMaterial::cVSWorldEyePos << " );" << std::endl;

			fVSInputStruct( ss );

			ss << "struct tVSOutput {" << std::endl;
			ss << "    float4 vCP : POSITION;" << std::endl;
			ss << "    float2 vOutUv : TEXCOORD0;" << std::endl;
			ss << "};" << std::endl;

			ss << "tVSOutput main( " << cVSInputName << " i ) {" << std::endl;
			ss << "    tVSOutput o;" << std::endl;
			ss << "    float3 vInPosWorld = mul( gLocalToWorldPos, float4( i.vP, 1.f ) );" << std::endl;
			ss << "    o.vCP = mul( gWorldToProj, float4( vInPosWorld, 1.f ) );" << std::endl;			
			ss << "    o.vOutUv.xy = i.vUv0;" << std::endl;
			ss << "    return o;" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}

		void fGenerateFullBrightPs30( std::string& o )
		{
			std::stringstream ss;
			ss << "sampler2D gColorMap : register( s" << 1 << " );" << std::endl;
			ss << "float4 gTint : register( c" << Gfx::tDecalMaterial::cPSRgbaTint << " );" << std::endl;
			ss << "void main( in float2 vInUv : TEXCOORD0, out float4 oColor : COLOR ) {" << std::endl;
			ss << "    oColor = gTint * tex2D( gColorMap, vInUv );" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}

		void fVsGlobals( std::stringstream& ss )
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
		}

		void fVSInputStruct( std::stringstream& ss )
		{
			ss << "struct " << cVSInputName << " {" << std::endl;
			ss << "    float3 vP : POSITION;" << std::endl;
			ss << "    float3 vN : NORMAL;" << std::endl;
			ss << "    float4 vT : TANGENT;" << std::endl;
			ss << "    float2 vUv0 : TEXCOORD0;" << std::endl;
			ss << "    float4 vColor : COLOR0;" << std::endl;
			ss << "};" << std::endl;
		}

		void fVSOutputPsInputStruct( std::stringstream& ss, b32 psIn, b32 tangent, u32 lightCount = 0 )
		{
			if( psIn )
				ss << "struct " << cPSInputName << " {" << std::endl;
			else
				ss << "struct " << cVSOutputName << " {" << std::endl;

			if( !psIn ) // for vertex shader only, we write clip pos
				ss << "    float4 vCP : POSITION;" << std::endl;

			const b32 vsOrLights = !psIn || lightCount > 0;

			if( vsOrLights )
				ss << "    float3 vP : TEXCOORD1;" << std::endl;
			if( psIn ) // write face register for pixel shader only
				ss << "    float vF : VFACE;" << std::endl;

			if( vsOrLights )
				ss << "    float3 vN : NORMAL;" << std::endl;
			if( vsOrLights && tangent )
				ss << "    float4 vT : TANGENT;" << std::endl;
			if( !psIn || vsOrLights )
				ss << "    float4 vLightPos : TEXCOORD2;" << std::endl;

			if( vsOrLights )
				ss << "    float2 vUv0 : TEXCOORD0;" << std::endl;

			ss << "    float4 vColor : COLOR0;" << std::endl;

			ss << "};" << std::endl;
		}

		std::string& fVS( std::string& o, b32 tangent, tPlatformId pid )
		{
			std::stringstream ss;

			fVsGlobals( ss );

			fVSInputStruct( ss );
			fVSOutputPsInputStruct( ss, false, tangent );

			ss << cVSOutputName << " main( " << cVSInputName << " i ) {" << std::endl;

			ss << "    " << cVSOutputName << " o;" << std::endl;

			// compute world pos
			std::string localToWorldPosName, localToWorldNormalName;
			localToWorldPosName = "gLocalToWorldPos";
			localToWorldNormalName = "gLocalToWorldNormal";

			ss << "    float3 pworld = mul( " << localToWorldPosName << ", float4( i.vP.xyz, 1.f ) );" << std::endl;
			// output clip pos
			ss << "    o.vCP = mul( gWorldToProj, float4( pworld, 1.f ) );" << std::endl;
			// output light pos
			if( tMaterialGenBase::fShadowMapLayerCount( pid ) == 1 )
				ss << "   o.vLightPos = mul( gWorldToLight, float4( pworld, 1.f ) );" << std::endl;
			else
				ss << "   o.vLightPos = float4( pworld, 1.f );" << std::endl;
			// output modified-eye-relative
			ss << "    o.vP.xyz = pworld - gWorldEyePos.xyz;" << std::endl;

			ss << "    o.vN.xyz = mul( ( float3x3 )" << localToWorldNormalName << ", i.vN );" << std::endl;

			if( tangent ) // transform tangent to world, but preserve w (sign for binormal)
			{
				ss << "    o.vT.xyz = mul( ( float3x3 )" << localToWorldNormalName << ", i.vT.xyz );" << std::endl;
				ss << "    o.vT.w = i.vT.w;" << std::endl;
			}

			ss << "    o.vUv0 = i.vUv0;" << std::endl;
			ss << "    o.vColor = i.vColor;" << std::endl;

			ss << "    return o;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		void fPsGlobals( tPlatformId pid, std::stringstream& ss, b32 normalMap )
		{
			ss << "#pragma pack_matrix(row_major)" << std::endl;

			u32 ithSampler = 0;

			const std::string shadowMapSampler = tMaterialGenBase::fShadowMapSamplerName( pid );
			ss << shadowMapSampler << " gShadowMap : register( s" << ithSampler++ << " );" << std::endl;

			ss << "sampler2D gDiffuseMap : register( s" << ithSampler++ << " );" << std::endl;
			if( normalMap )
				ss << "sampler2D gNormalMap : register( s" << ithSampler++ << " );" << std::endl;

			ss << "float4 gRgbaTint : register( " << fAppend("c", cPSRgbaTint) << " );" << std::endl;
			ss << "float4 gDiffuseUvXform : register( " << fAppend("c", cPSDiffuseUvXform) << " );" << std::endl;
			ss << "float4 gNormalUvXform : register( " << fAppend("c", cPSNormalUvXform) << " );" << std::endl;

			ss << "float4 gBumpDepth_SpecSize_Opacity_BackFaceFlip : register( " << fAppend("c", cPSBumpDepth_SpecSize_Opacity_BackFaceFlip) << " );" << std::endl;
			ss << "float4 gFogValues : register( " << fAppend("c", cPSFogValues) << " );" << std::endl;
			ss << "float3 gFogColor : register( " << fAppend("c", cPSFogColor) << " );" << std::endl;

			// misc. shadow map values
			ss << "float4 gShadowMapEpsilon_TexelSize_Amount_Split : register( " << fAppend("c", cPSShadowMapEpsilon_TexelSize_Amount_Split) << " );" << std::endl;
			ss << "float4 gShadowMapTarget_Split : register( " << fAppend("c", cPSShadowMapTarget_Split) << " );" << std::endl;

			// eye position in world space
			ss << "float4 gWorldEyePos : register( " << fAppend("c", cPSWorldEyePos) << " );" << std::endl;

			// declare world to light constants
			tMaterialGenBase::fDeclareWorldToLightConstantsArray( pid, ss, cPSWorldToLightArray );

			// declare light constants
			tMaterialGenBase::fDeclareLightConstantsArray( ss, cPSLightVectorFirst );
		}

		std::string& fPS( std::string& o, b32 tangent, u32 numUvs, u32 diffuseUvSet, u32 normalUvSet, u32 lightCount, tPlatformId pid )
		{
			// TODO see if this makes any difference
			const b32 renormalizeTangentSpace = true;

			std::stringstream ss;

			fPsGlobals( pid, ss, normalUvSet < numUvs );

			fVSOutputPsInputStruct( ss, true, tangent, lightCount );

			ss << "void main( " << cPSInputName << " i, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;

			if( lightCount == 0 )
			{
				// early out, as we don't want to do any lighting
				ss << "oColor = gRgbaTint * i.vColor;" << std::endl;
				ss << "}" << std::endl;
				o = ss.str( );
				return o;
			}

			if( tangent )
			{
				// we should only have a tangent if we have a normal map
				sigassert( normalUvSet < numUvs );

				// transform uv
				ss << "float2 normalUv = " << fAppend("i.vUv", normalUvSet) << ";" << std::endl;

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
			else
			{
				// normal, but no normal mapping
				if( renormalizeTangentSpace )
					ss << "i.vN.xyz = normalize( i.vN.xyz );" << std::endl;
				// just copy normal
				ss << "float3 n = i.vN.xyz;" << std::endl;
			}

			// by now we safely have a variable called 'n' representing the world space normal;
			// multiply it by the sign of the face register to achieve proper double-sided lighting;
			// note that we negate bcz our idea of 'back-face' is opposite from D3D.
			ss << "n *= min( 1.f, gBumpDepth_SpecSize_Opacity_BackFaceFlip.w " << ( pid == cPlatformXbox360 ? "+" : "-" ) << " sign( i.vF ) );" << std::endl;

			// declare diffuse variable, this may get modulated by map
			ss << "float4 diffuse = float4( 1, 1, 1, gBumpDepth_SpecSize_Opacity_BackFaceFlip.z );" << std::endl;
			if( diffuseUvSet < numUvs )
			{
				// transform uv
				ss << "float2 diffuseUv = " << fAppend("i.vUv", diffuseUvSet) << ";" << std::endl;
				// sample diffuse and modulate
				ss << "diffuse *= tex2D( gDiffuseMap, diffuseUv );" << std::endl;
			}

			tMaterialGenBase::fCallComputeShadowTerm( pid, ss, "gShadowMap", "i.vLightPos", "gShadowMapEpsilon_TexelSize_Amount_Split.x", "gShadowMapEpsilon_TexelSize_Amount_Split.y", "gShadowMapEpsilon_TexelSize_Amount_Split.z", "shadowTerm", "gShadowMapTarget_Split.xyz", "gShadowMapTarget_Split.w", "gShadowMapEpsilon_TexelSize_Amount_Split.w" );

			// declare diffuse light accumulator
			ss << "float3 diffAccum = 0.f;" << std::endl;

			if( lightCount > 0 )
			{
				// begin light loop
				if( lightCount > 1 )
					ss << "for( int iLight = 0; iLight < " << lightCount << "; ++iLight )" << std::endl;
				else
					ss << "int iLight = 0;" << std::endl;

				ss << "{" << std::endl;

				// until we get shadows on multiple lights, we continue to use the shadow term from the first light
				//// only shadow the first light
				//ss << "shadowTerm = min( 1.f, shadowTerm + iLight );" << std::endl;

				tMaterialGenBase::fCallAccumulateLight( 
					ss, false, "gLights[ iLight ]", "i.vP.xyz", "n", "gBumpDepth_SpecSize_Opacity_BackFaceFlip.y", "diffAccum", "specAccum", "specMagAccum", "shadowTerm" );
				ss << "}" << std::endl;
			}

			// pre-fog pixel color (modulate final combined light values with material colors)
			ss << "float4 preFog = float4( diffAccum * diffuse.rgb, diffuse.a );" << std::endl;

			// tint final pre-fog color
			ss << "preFog *= gRgbaTint * i.vColor;" << std::endl;

			tMaterialGenBase::fCallFog( ss, "preFog.rgb", "i.vP.xyz", "gFogValues", "gFogColor", "fogResult" );

			ss << "oColor = float4( fogResult.rgb, preFog.a );" << std::endl;

			ss << "}" << std::endl;

			o = ss.str( );
			return o;
		}

		void fWriteCompiledShader( const tFilePathPtr& filePath, std::string shaderText )
		{
			sigassert( !filePath.fNull( ) );
			tFileWriter o( filePath );
			o( shaderText.c_str( ), ( u32 )shaderText.length( ) );
		}

		void fGenerate( Gfx::tMaterialFile& mtlFile, tMaterialGenBase::tShaderBufferSet& shaderBuffers, tPlatformId pid )
		{
			shaderBuffers[ Gfx::tDecalMaterial::cShaderCategoryVS ].fNewArray( Gfx::tDecalMaterial::cVSCount );
			mtlFile.mShaderLists[ Gfx::tDecalMaterial::cShaderCategoryVS ].fNewArray( Gfx::tDecalMaterial::cVSCount );
			shaderBuffers[ Gfx::tDecalMaterial::cShaderCategoryPS ].fNewArray( Gfx::tDecalMaterial::cPSNumLit * Gfx::tMaterial::cLightSlotCount + Gfx::tDecalMaterial::cPSNumUnlit );
			mtlFile.mShaderLists[ Gfx::tDecalMaterial::cShaderCategoryPS ].fNewArray( Gfx::tDecalMaterial::cPSNumLit * Gfx::tMaterial::cLightSlotCount + Gfx::tDecalMaterial::cPSNumUnlit );

			// Generate fullbright
			std::string vs, ps;
			tDecalMaterialGenStruct::fGenerateFullBrightVs30( vs );
			tDecalMaterialGenStruct::fGenerateFullBrightPs30( ps );

			tMaterialGenBase::fAddVertexShader( pid, Gfx::tDecalMaterial::cShaderCategoryVS, Gfx::tDecalMaterial::cVSFullBright, mtlFile, shaderBuffers, vs );
			tMaterialGenBase::fAddPixelShader( pid, Gfx::tDecalMaterial::cShaderCategoryPS, Gfx::tDecalMaterial::cPSFullBright, mtlFile, shaderBuffers, ps );

			fWriteCompiledShader( tFilePathPtr( "C:\\SignalStudios\\TS2\\SigEngine\\tmp\\matgen\\Fullbright.vsh" ), vs );
			fWriteCompiledShader( tFilePathPtr( "C:\\SignalStudios\\TS2\\SigEngine\\tmp\\matgen\\Fullbright.psh" ), ps );

			std::string s;

			// add vertex shaders
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tDecalMaterial::cShaderCategoryVS, cVSDiffuse, mtlFile, shaderBuffers,
				fVS( s, false, pid ) );
			tMaterialGenBase::fAddVertexShader( pid, Gfx::tDecalMaterial::cShaderCategoryVS, cVSDiffuseNormal, mtlFile, shaderBuffers,
				fVS( s, true, pid ) );

			fWriteCompiledShader( tFilePathPtr( "C:\\SignalStudios\\TS2\\SigEngine\\tmp\\matgen\\DiffuseOnly.vsh" ), s );
			fWriteCompiledShader( tFilePathPtr( "C:\\SignalStudios\\TS2\\SigEngine\\tmp\\matgen\\DiffuseNormal.vsh" ), s );

			for( u32 iLightCount = 0; iLightCount < Gfx::tMaterial::cLightSlotCount; ++iLightCount )
			{
				// add non normal-mapped pixel shaders
				tMaterialGenBase::fAddPixelShader( pid, Gfx::tDecalMaterial::cShaderCategoryPS, 
					fPixelShaderSlot( cPSDiffuse, iLightCount ), mtlFile, shaderBuffers,
					fPS( s, false,  1,  0, ~0, iLightCount, pid ) );

				// add normal-mapped shaders
				tMaterialGenBase::fAddPixelShader( pid, Gfx::tDecalMaterial::cShaderCategoryPS, 
					fPixelShaderSlot( cPSDiffuseNormal, iLightCount ), mtlFile, shaderBuffers,
					fPS( s, true,  1,  0,  0, iLightCount, pid ) );
			}
		}

		void fAssignVertexAndPixelShaderSlots( Gfx::tDecalMaterial* mtl, b32 hasTangent )
		{
			// select vertex format and vertex shader (currently one and the same thing)
			if( hasTangent )
			{
				// normal mapped versions
				mtl->mVS = cVSDiffuseNormal;
				mtl->mPS = cPSDiffuseNormal;
			}
			else
			{
				// non normal mapped versions
				mtl->mVS = cVSDiffuse;
				mtl->mPS = cPSDiffuse;
			}
			// TODO: Fullbright
		}
	};

	tDecalMaterialGen::tDecalMaterialGen( )
		: mBumpDepth( 0.f )
		, mSpecSize( 0.5f )
	{
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tDecalMaterialGen::tUvParameters& o )
	{
		s( "MirrorUv", o.mMirrorUv );
		s( "WrapUv", o.mWrapUv );
		s( "RepeatUv", o.mRepeatUv );
		s( "OffsetUv", o.mOffsetUv );
		s( "UvSetName", o.mUvSetName );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tDecalMaterialGen& o )
	{
		s( "DiffuseMapPath", o.mDiffuseMapPath );
		s( "DiffuseUvParams", o.mDiffuseUvParams );

		s( "NormalMapPath", o.mNormalMapPath );
		s( "NormalUvParams", o.mNormalUvParams );
		s( "BumpDepth", o.mBumpDepth );

		s( "SpecSize", o.mSpecSize );
	}

	void tDecalMaterialGen::fSerialize( tXmlSerializer& s )		{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tDecalMaterialGen::fSerialize( tXmlDeserializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tDecalMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		Gfx::tDecalMaterial* mtl = new Gfx::tDecalMaterial( );

		fConvertBase( mtl );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tDecalMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		Math::tVec4f dummyColor(1.f);

		tLoadInPlaceResourcePtr* difLip = mtl->mDiffuseMap.fGetLip( );
		tLoadInPlaceResourcePtr* normLip = mtl->mNormalMap.fGetLip( );

		const b32 diffuseMap	= fAddTexture( 
			dummyColor, difLip, mtl->mDiffuseUvXform, dummyColor, mDiffuseMapPath, mDiffuseUvParams, lipFileCreator );
		const b32 normalMap		= fAddTexture( 
			dummyColor, normLip, mtl->mNormalUvXform, dummyColor, mNormalMapPath, mNormalUvParams, lipFileCreator );

		sigassert( diffuseMap );

		if( normalMap )
			mtl->mBumpDepth_SpecSize_Opacity_BackFaceFlip.x = mBumpDepth;
		if( mFlipBackFaceNormal )
			mtl->mBumpDepth_SpecSize_Opacity_BackFaceFlip.w = 2.f;

		tDecalMaterialGenStruct lightingPassGen;
		lightingPassGen.fAssignVertexAndPixelShaderSlots( mtl, normalMap );

		return mtl;
	}

	b32 tDecalMaterialGen::fAddTexture( 
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

	b32 tDecalMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		if( !tMaterialGenBase::fIsEquivalent( other ) )
			return false;

		const tDecalMaterialGen& otherDecal = static_cast<const tDecalMaterialGen&>( other );

		if( mDiffuseMapPath != otherDecal.mDiffuseMapPath )
			return false;
		if( mNormalMapPath != otherDecal.mNormalMapPath )
			return false;

		if( mDiffuseMapPath.fLength( ) > 0 )
			if( mDiffuseUvParams != otherDecal.mDiffuseUvParams )
				return false;
		if( mNormalMapPath.fLength( ) > 0 )
		{
			if( mNormalUvParams != otherDecal.mNormalUvParams )
				return false;
			if( !fEqual( mBumpDepth, otherDecal.mBumpDepth ) )
				return false;
		}


		// if we made it past all the early outs, then we must be equivalent
		return true;
	}

	void tDecalMaterialGen::fGetTextureResourcePaths( tFilePathPtrList& resourcePathsOut )
	{
		if( mDiffuseMapPath.fLength( ) > 0 )
			resourcePathsOut.fFindOrAdd( mDiffuseMapPath );
		if( mNormalMapPath.fLength( ) > 0 )
			resourcePathsOut.fFindOrAdd( mNormalMapPath );
	}

	tFilePathPtr tDecalMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tDecalMaterial::fMaterialFilePath( );
	}

	void tDecalMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tDecalMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tDecalMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( Gfx::tDecalMaterial::cShaderCategoryCount );
		tShaderBufferSet shaderBuffers( Gfx::tDecalMaterial::cShaderCategoryCount );

		tDecalMaterialGenStruct lightingPassGen;
		lightingPassGen.fGenerate( mtlFile, shaderBuffers, pid );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tDecalMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tDecalMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tDecalMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

