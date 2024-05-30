#include "ToolsPch.hpp"
#include "tMaterialGenBase.hpp"
#include "tFileWriter.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "FileSystem.hpp"
#include "Dx9Util.hpp"
#include "Dx360Util.hpp"
#include "iAssetGenPlugin.hpp"
#include "tProjectFile.hpp"

namespace Sig
{
	const char tMaterialGenBase::gMaterialRootFolder[] = "Materials";

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tMaterialGenBase& o )
	{
		s( "TwoSided", o.mTwoSided );
		s( "FlipBackFaceNormal", o.mFlipBackFaceNormal );
		s( "Transparency", o.mTransparency );
		s( "AlphaCutOut", o.mAlphaCutOut );
		s( "Additive", o.mAdditive );
		s( "ZBufferTest", o.mZBufferTest );
		s( "ZBufferWrite", o.mZBufferWrite );
		s( "FaceX", o.mFaceX );
		s( "FaceY", o.mFaceY );
		s( "FaceZ", o.mFaceZ );
		s( "SortOffset", o.mSortOffset );
	}

	tMaterialGenBase::tMaterialGenBase( )
	{
	}

	void tMaterialGenBase::fSerialize( tXmlSerializer& s )		{ fSerializeXmlObject( s, *this ); }
	void tMaterialGenBase::fSerialize( tXmlDeserializer& s )	{ fSerializeXmlObject( s, *this ); }
	b32	 tMaterialGenBase::fIsFacing( ) const					{ return mFaceX || mFaceY || mFaceZ; }

	void tMaterialGenBase::fCreateOutputFile( const tFilePathPtr& resourcePath, tFileWriter& ofile, tPlatformId pid )
	{
		const tFilePathPtr ofileName = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, resourcePath );
		
		ofile.fOpen( ofileName );
		sigassert( ofile.fIsOpen( ) );
	}

	void tMaterialGenBase::fSaveMaterialFile( 
		Gfx::tMaterialFile& mtlFile, 
		const tShaderBufferSet& shaderBuffers, 
		tPlatformId pid )
	{
		fSaveMaterialFile( fMaterialFilePath( ), mtlFile, shaderBuffers, pid );
	}

	void tMaterialGenBase::fSaveMaterialFile( 
		const tFilePathPtr& resourcePath,
		Gfx::tMaterialFile& mtlFile, 
		const tShaderBufferSet& shaderBuffers, 
		tPlatformId pid )
	{
		// create output file
		tFileWriter ofile;
		fCreateOutputFile( resourcePath, ofile, pid );

		// to be on the safe-side, we need to zero-out some runtime values
		// before serializing, otherwise we'll be writing junk and crash
		for( u32 i = 0; i < mtlFile.mShaderLists.fCount( ); ++i )
		for( u32 j = 0; j < mtlFile.mShaderLists[ i ].fCount( ); ++j )
			mtlFile.mShaderLists[ i ][ j ].mPlatformHandle = 0;

		// make sure we set the signature prior to serializing
		mtlFile.fSetSignature( pid, Rtti::fGetClassId<Gfx::tMaterialFile>( ), Gfx::tMaterialFile::cVersion );

		// create load in place serializer, and serialize the file header (persistent portion of the file)
		tLoadInPlaceSerializer ser;
		mtlFile.mHeaderSize = ser.fSave( mtlFile, ofile, pid );

		// now continue saving shader buffers from the current location of the output file;
		// we'll store the offset of each buffer in the header
		u32 offsetFromStartOfFile = mtlFile.mHeaderSize;
		for( u32 i = 0; i < shaderBuffers.fCount( ); ++i )
		for( u32 j = 0; j < shaderBuffers[ i ].fCount( ); ++j )
		{
			// store current file offset
			mtlFile.mShaderLists[ i ][ j ].mBufferOffset = offsetFromStartOfFile;

			// store buffer size
			mtlFile.mShaderLists[ i ][ j ].mBufferSize = shaderBuffers[ i ][ j ].fCount( );

			// write shader buffer
			ofile( shaderBuffers[ i ][ j ].fBegin( ), shaderBuffers[ i ][ j ].fCount( ) );

			// increment current file offset
			offsetFromStartOfFile += shaderBuffers[ i ][ j ].fCount( );
		}

		// seek back to the start of the file
		ofile.fSeek( 0 );

		// re-write the table of header information now that it has proper offsets
		const u32 headerSizeVerify = ser.fSave( mtlFile, ofile, pid );

		// sanity check
		sigassert( mtlFile.mHeaderSize == headerSizeVerify );
	}

	void tMaterialGenBase::fConvertBase( Gfx::tMaterial* mtl )
	{
		// start with default based on transparency
		Gfx::tRenderState rs;
		fToRenderState( rs );

		mtl->fSetRenderState( rs );

		mtl->fSetFacingFlags( mFaceX, mFaceY, mFaceZ );
	}

	b32 tMaterialGenBase::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		// if not same type, no way we can be equivalent
		if( fClassId( ) != other.fClassId( ) )
			return false;

		const tMaterialGenBase& otherMyType = static_cast<const tMaterialGenBase&>( other );

		if( mTwoSided != otherMyType.mTwoSided )
			return false;
		if( mFlipBackFaceNormal != otherMyType.mFlipBackFaceNormal )
			return false;
		if( mTransparency != otherMyType.mTransparency )
			return false;
		if( mAlphaCutOut != otherMyType.mAlphaCutOut )
			return false;
		if( mAdditive != otherMyType.mAdditive )
			return false;
		if( mZBufferTest != otherMyType.mZBufferTest )
			return false;
		if( mZBufferWrite != otherMyType.mZBufferWrite )
			return false;
		if( mFaceX != otherMyType.mFaceX )
			return false;
		if( mFaceY != otherMyType.mFaceY )
			return false;
		if( mFaceZ != otherMyType.mFaceZ )
			return false;

		return true;
	}

	void tMaterialGenBase::fGenerateMaterialFile( const tFilePathPtr& exePath, b32 force )
	{
		for( tPlatformIdIterator i = cPlatformFirst; !i.fDone( ); i.fNext( ) )
		{
			if( !force )
			{
				const tFilePathPtr mtlFilePath = iAssetGenPlugin::fCreateAbsoluteOutputPath( (tPlatformId)i, fMaterialFilePath( ) );
				if( FileSystem::fIsAMoreRecentThanB( mtlFilePath, exePath ) )
				{
					log_line( 0, "  -> material is up to date for platform [" << fPlatformIdString( i ) << "]..." );
					continue;
				}
			}

			switch( i )	
			{
			case cPlatformWii:		fGenerateMaterialFileWii( i ); break;
			case cPlatformPcDx9:	fGenerateMaterialFilePcDx9( i ); break;
			case cPlatformPcDx10:	fGenerateMaterialFilePcDx10( i ); break;
			case cPlatformXbox360:	fGenerateMaterialFileXbox360( i ); break;
			case cPlatformPs3Ppu:	fGenerateMaterialFilePs3Ppu( i ); break;
			case cPlatformPs3Spu:	break;
			case cPlatformiOS:		/*TODO*/ break;
			default: log_warning( 0, "Unhandled platform in tMaterialGenBase::fGenerateMaterialFile!" ); break;
			}
		}
	}

	void tMaterialGenBase::fAddVertexShader( 
		tPlatformId pid,
		u32 shaderListIndex,
		u32 shaderIndex,
		Gfx::tMaterialFile& mtlFile, 
		tShaderBufferSet& shaderBuffers,
		const std::string& hlsl )
	{
		if( pid == cPlatformPcDx9 )
			fAddPcDx9VertexShader( shaderListIndex, shaderIndex, mtlFile, shaderBuffers, hlsl );
		else if( pid == cPlatformXbox360 )
			fAddXbox360VertexShader( shaderListIndex, shaderIndex, mtlFile, shaderBuffers, hlsl );
	}

	void tMaterialGenBase::fAddPixelShader( 
		tPlatformId pid,
		u32 shaderListIndex,
		u32 shaderIndex,
		Gfx::tMaterialFile& mtlFile, 
		tShaderBufferSet& shaderBuffers,
		const std::string& hlsl )
	{
		if( pid == cPlatformPcDx9 )
			fAddPcDx9PixelShader( shaderListIndex, shaderIndex, mtlFile, shaderBuffers, hlsl );
		else if( pid == cPlatformXbox360 )
			fAddXbox360PixelShader( shaderListIndex, shaderIndex, mtlFile, shaderBuffers, hlsl );
	}

	void tMaterialGenBase::fAddPcDx9VertexShader( 
		u32 shaderListIndex,
		u32 shaderIndex,
		Gfx::tMaterialFile& mtlFile, 
		tShaderBufferSet& shaderBuffers,
		const std::string& hlsl )
	{
		mtlFile.mShaderLists[ shaderListIndex ][ shaderIndex ].mType = Gfx::tMaterialFile::cShaderBufferTypeVertex;
		const b32 success = Dx9Util::fCompileVertexShader( hlsl, shaderBuffers[ shaderListIndex ][ shaderIndex ] );
		sigassert( success );
	}

	void tMaterialGenBase::fAddPcDx9PixelShader( 
		u32 shaderListIndex,
		u32 shaderIndex,
		Gfx::tMaterialFile& mtlFile, 
		tShaderBufferSet& shaderBuffers,
		const std::string& hlsl )
	{
		mtlFile.mShaderLists[ shaderListIndex ][ shaderIndex ].mType = Gfx::tMaterialFile::cShaderBufferTypePixel;
		const b32 success = Dx9Util::fCompilePixelShader( hlsl, shaderBuffers[ shaderListIndex ][ shaderIndex ] );
		sigassert( success );
	}

	void tMaterialGenBase::fAddXbox360VertexShader( 
		u32 shaderListIndex,
		u32 shaderIndex,
		Gfx::tMaterialFile& mtlFile, 
		tShaderBufferSet& shaderBuffers,
		const std::string& hlsl )
	{
		mtlFile.mShaderLists[ shaderListIndex ][ shaderIndex ].mType = Gfx::tMaterialFile::cShaderBufferTypeVertex;
		const b32 success = Dx360Util::fCompileVertexShader( hlsl, shaderBuffers[ shaderListIndex ][ shaderIndex ] );
		sigassert( success );
	}

	void tMaterialGenBase::fAddXbox360PixelShader( 
		u32 shaderListIndex,
		u32 shaderIndex,
		Gfx::tMaterialFile& mtlFile, 
		tShaderBufferSet& shaderBuffers,
		const std::string& hlsl )
	{
		mtlFile.mShaderLists[ shaderListIndex ][ shaderIndex ].mType = Gfx::tMaterialFile::cShaderBufferTypePixel;
		const b32 success = Dx360Util::fCompilePixelShader( hlsl, shaderBuffers[ shaderListIndex ][ shaderIndex ] );
		sigassert( success );
	}

	u32 tMaterialGenBase::fShadowMapLayerCount( tPlatformId pid )
	{
		if( pid == cPlatformXbox360 )
		{
			const tProjectFile& projFile = tProjectFile::fGetCurrentProjectFileCached( );
			return projFile.mShadowMapLayerCount;
		}
		return 1;
	}

	std::string tMaterialGenBase::fShadowMapSamplerName( tPlatformId pid )
	{
		const std::string shadowMapSampler = ( fShadowMapLayerCount( pid ) > 1 ) ? "sampler3D" : "sampler2D";
		return shadowMapSampler;
	}
	
	void tMaterialGenBase::fDeclareWorldToLightConstantsArray(
		tPlatformId pid,
		std::stringstream& ss, 
		u32 startRegister,
		const std::string& arrayName,
		u32 maxSlots )
	{
		ss << "float4x4 " << arrayName << "[" << maxSlots << "] : register( " << StringUtil::fAppend("c", startRegister) << " );" << std::endl;
	}

	void tMaterialGenBase::fDeclareLightConstantsArray(
		std::stringstream& ss, 
		u32 startRegister,
		const std::string& lightArrayName,
		u32 maxLights )
	{
		ss << "struct tLightConstants" << std::endl;
		ss << "{" << std::endl;
		ss << "   float4 mDirection;" << std::endl;
		ss << "   float4 mPosition;" << std::endl;
		ss << "   float4 mAttenuation;" << std::endl;
		ss << "   float4 mAngles;" << std::endl;
		ss << "   float4 mAmbient;" << std::endl;
		ss << "   float4 mColors[3];" << std::endl;
		ss << "};" << std::endl;

		ss << "tLightConstants " << lightArrayName << "[" << maxLights << "] : register( " << StringUtil::fAppend("c", startRegister) << " );" << std::endl;
	}

	void tMaterialGenBase::fCallFog( 
		std::stringstream& ss, 
		const std::string& pixelColorName,
		const std::string& positionName,
		const std::string& fogValuesName,
		const std::string& fogColorName,
		const std::string& resultColorName )
	{
		ss << "float pDist_ = length( " << positionName << " );" << std::endl;
		ss << "float tFog_ = saturate( ( pDist_ - " << fogValuesName << ".x ) / " << fogValuesName << ".y );" << std::endl;
		ss << "tFog_ = clamp( tFog_ * tFog_, " << fogValuesName << ".z, " << fogValuesName << ".w );" << std::endl;
		ss << "float3 " << resultColorName << " = lerp( " << pixelColorName << ", " << fogColorName << ", tFog_ );" << std::endl;
	}

	void tMaterialGenBase::fCallComputeShadowTerm(
		tPlatformId pid,
		std::stringstream& ss,
		const std::string& shadowMapSamplerName,
		const std::string& worldPosName,
		const std::string& shadowEpsilonName,
		const std::string& shadowMapTexelSizeName,
		const std::string& shadowAmountName,
		const std::string& shadowTermName,
		const std::string& shadowMapTargetName,
		const std::string& shadowSplitDistance0Name,
		const std::string& shadowSplitDistance1Name,
		const std::string& worldToLightArrayName )
	{
		std::string lightPosName;

		const u32 layerCount = fShadowMapLayerCount( pid );
		sigassert( layerCount <= Gfx::tMaterial::cMaxShadowLayers );
		switch( layerCount )
		{
		case 1: 
			lightPosName = worldPosName; // if no cascades, then this has already been transformed to light space pos
			break;
		case 2:
		case 3:
			{
				ss << "float numShadowLayers_ = " << layerCount << ";" << std::endl;

				ss << "float distToShadowTarget_ = length( " << shadowMapTargetName << " - " << worldPosName << " );" << std::endl;

				//ss << "float shadowLayer_ = 1;" << std::endl;
				if( layerCount == 2 )
					ss << "float shadowLayer_ = saturate( distToShadowTarget_  < " << shadowSplitDistance0Name << " );" << std::endl;
				else if( layerCount == 3 )
					ss << "float shadowLayer_ = ( distToShadowTarget_  < " << shadowSplitDistance1Name << " ) ? 2 : ( distToShadowTarget_ < " << shadowSplitDistance0Name << " ? 1 : 0 );" << std::endl;
				else
					sigassert( !"invalid number of shadow layers" );

				lightPosName = "lightSpacePos_";
				ss << "float4 " << lightPosName << " = mul( " << worldToLightArrayName << "[ ( int )shadowLayer_ ], " << worldPosName << " );" << std::endl;
			}
			break;
		default:
			sigassert( !"invalid number of shadow layers" );
			break;
		}


		sigassert( lightPosName.length( ) > 0 );

		// transform from render target space to texture space.
		ss << "float2 shadowTexCoord_ = 0.5 * " << lightPosName << ".xy + float2( 0.5, 0.5 );" << std::endl;
		ss << "shadowTexCoord_.y = 1.0f - shadowTexCoord_.y;" << std::endl;

		// transform to texel space
		ss << "float2 shadowTexelPos_ = " << shadowMapTexelSizeName << " * shadowTexCoord_;" << std::endl;

		// Determine the lerp amounts           
		ss << "float2 shadowLerps_ = frac( shadowTexelPos_ );" << std::endl;

		// compute texel offset
		ss << "float oneTexelOffset_ = 1.0f / " << shadowMapTexelSizeName << ";" << std::endl;

		// light-space depth (divide by dubya)
		ss << "float lightSpaceDepth_ = saturate( " << lightPosName << ".z );" << std::endl;

		//read in bilerp stamp, doing the shadow checks
		ss << "float shadowTaps_[4];" << std::endl;
		if( layerCount > 1 )
		{
			ss << "float shadowZcoord_ = 1.0f / ( 2.0f * numShadowLayers_ ) + shadowLayer_ / numShadowLayers_;" << std::endl;
			ss << "shadowTaps_[0] = (tex3D( " << shadowMapSamplerName << ", float3(shadowTexCoord_, shadowZcoord_) ) + " << shadowEpsilonName << " < lightSpaceDepth_) ? 0.0f : 1.0f;" << std::endl;
			ss << "shadowTaps_[1] = (tex3D( " << shadowMapSamplerName << ", float3(shadowTexCoord_ + float2(oneTexelOffset_, 0), shadowZcoord_) ) + " << shadowEpsilonName << " < lightSpaceDepth_) ? 0.0f : 1.0f;" << std::endl;
			ss << "shadowTaps_[2] = (tex3D( " << shadowMapSamplerName << ", float3(shadowTexCoord_ + float2(0, oneTexelOffset_), shadowZcoord_) ) + " << shadowEpsilonName << " < lightSpaceDepth_) ? 0.0f : 1.0f;" << std::endl;
			ss << "shadowTaps_[3] = (tex3D( " << shadowMapSamplerName << ", float3(shadowTexCoord_ + float2(oneTexelOffset_, oneTexelOffset_), shadowZcoord_) ) + " << shadowEpsilonName << " < lightSpaceDepth_) ? 0.0f : 1.0f;" << std::endl;
		}
		else
		{
			ss << "shadowTaps_[0] = (tex2D( " << shadowMapSamplerName << ", shadowTexCoord_ ) + " << shadowEpsilonName << " < lightSpaceDepth_) ? 0.0f : 1.0f;" << std::endl;
			ss << "shadowTaps_[1] = (tex2D( " << shadowMapSamplerName << ", shadowTexCoord_ + float2(oneTexelOffset_, 0) ) + " << shadowEpsilonName << " < lightSpaceDepth_) ? 0.0f : 1.0f;" << std::endl;
			ss << "shadowTaps_[2] = (tex2D( " << shadowMapSamplerName << ", shadowTexCoord_ + float2(0, oneTexelOffset_) ) + " << shadowEpsilonName << " < lightSpaceDepth_) ? 0.0f : 1.0f;" << std::endl;
			ss << "shadowTaps_[3] = (tex2D( " << shadowMapSamplerName << ", shadowTexCoord_ + float2(oneTexelOffset_, oneTexelOffset_) ) + " << shadowEpsilonName << " < lightSpaceDepth_) ? 0.0f : 1.0f;" << std::endl;
		}

		// lerp between the shadow values to calculate our light amount
		ss << "float " << shadowTermName << " = lerp( lerp( shadowTaps_[0], shadowTaps_[1], shadowLerps_.x ), lerp( shadowTaps_[2], shadowTaps_[3], shadowLerps_.x ), shadowLerps_.y );" << std::endl;
		ss << shadowTermName << " = lerp( 1.f, " << shadowTermName << ", " << shadowAmountName << " );" << std::endl;
	}

	void tMaterialGenBase::fCallAccumulateLight( 
		std::stringstream& ss, 
		b32 calcSpec,
		const std::string& lightName,
		const std::string& positionName,
		const std::string& normalName,
		const std::string& specSizeName,
		const std::string& diffAccumName,
		const std::string& specAccumName,
		const std::string& specMagAccumName,
		const std::string& shadowTermName )
	{
		ss << "float3 lc0_ = " << lightName << ".mColors[ 0 ].rgb * " << shadowTermName << ";" << std::endl;
		ss << "float3 lc1_ = " << lightName << ".mColors[ 1 ].rgb * " << shadowTermName << ";" << std::endl;
		ss << "float3 lc2_ = " << lightName << ".mColors[ 2 ].rgb;" << std::endl;
		if( calcSpec )
			ss << "float3 ls_ = lc0_;" << std::endl;
		ss << "float3 la_ = " << lightName << ".mAmbient.rgb;" << std::endl;
		ss << "float3 l2v_ = " << lightName << ".mPosition.xyz - " << positionName << ";" << std::endl;
		ss << "float lLen_ = length( l2v_ );" << std::endl;
		ss << "float3 l_ = " << lightName << ".mDirection.xyz * " << lightName << ".mDirection.w + ( l2v_ / lLen_ ) * " << lightName << ".mPosition.w;" << std::endl;

		ss << "float nDotL_ = dot( " << normalName << ", l_ );" << std::endl;
		ss << "float3 litResult_ = float3( 1.f, nDotL_, 0.f );" << std::endl;

		if( calcSpec )
		{
			// compute half vector
			ss << "float3 h_ = normalize( normalize( -" << positionName << " ) + l_ );" << std::endl;
			ss << "float nDotH_ = dot( " << normalName << ", h_ );" << std::endl;

			// compute spec term
			ss << "litResult_.z = lit( nDotL_, nDotH_, " << specSizeName << " ).z;" << std::endl;
		}

		ss << "float pointAtten_ = 1.f - min( 1.f, max( 0, lLen_ - " << lightName << ".mAttenuation.x ) / " << lightName << ".mAttenuation.y );" << std::endl;
		ss << "float atten_ = " << lightName << ".mAttenuation.z + " << lightName << ".mAttenuation.w * pointAtten_;" << std::endl;
		ss << "litResult_.z *= atten_;" << std::endl;

		ss << diffAccumName << " += la_ + atten_*((lc0_ * max(0,litResult_.y)) + (lc1_ * (1-abs(litResult_.y))) + (lc2_ * max(0,-litResult_.y)));" << std::endl;
		if( calcSpec )
		{
			ss << specAccumName << " += ls_ * litResult_.z;" << std::endl;
			ss << specMagAccumName << " += litResult_.z;" << std::endl;
		}
	}

}
