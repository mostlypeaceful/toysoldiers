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
#include "tPlatform.hpp"

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
		s( "TransparentDepthPrepass", o.mXparentDepthPrepass );
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
		mtlFile.fSetSignature<Gfx::tMaterialFile>( pid );

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
		mtl->fSetXparentDepthPrepassFlag( mXparentDepthPrepass );
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
		if( mXparentDepthPrepass != otherMyType.mXparentDepthPrepass )
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
			default: log_warning( "Unhandled platform in tMaterialGenBase::fGenerateMaterialFile!" ); break;
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
		if( !success )
		{
			log_line( 0, hlsl );
			sigassert( success );
		}
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
		if( !success )
		{
			log_line( 0, hlsl );
			sigassert( success );
		}
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
		if( !success )
		{
			log_line( 0, hlsl );
			sigassert( success );
		}
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
		if( !success )
		{
			log_line( 0, hlsl );
			sigassert( success );
		}
	}

	u32 tMaterialGenBase::fShadowMapLayerCount( tPlatformId pid )
	{
		const tProjectFile& projFile = tProjectFile::fInstance( );
		return projFile.mEngineConfig.mRendererSettings.mShadowShaderGenerationSettings.mShadowMapLayerCount;
	}

	std::string tMaterialGenBase::fShadowMapSamplerName( tPlatformId pid )
	{
		const std::string shadowMapSampler = ( fShadowMapLayerCount( pid ) > 1 ) ? "sampler3D" : "sampler2D";
		return shadowMapSampler;
	}

	b32 tMaterialGenBase::fSupports3DTextures( tPlatformId pid )
	{
		switch( pid )
		{
		case cPlatformXbox360: return true;
		}

		return false;
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
		ss << "// Fog is now applied through the 'Depth of field' post effect, for commonality between deferred and forward shading. -Matt K" << std::endl;
		ss << "   float3 " << resultColorName << " = " << pixelColorName << ";" << std::endl;

		//ss << "   float pDist_ = length( " << positionName << " );" << std::endl;
		//ss << "   float tFog_ = saturate( ( pDist_ - " << fogValuesName << ".x ) / " << fogValuesName << ".y );" << std::endl;
		//ss << "   tFog_ = clamp( tFog_ * tFog_, " << fogValuesName << ".z, " << fogValuesName << ".w );" << std::endl;
		//ss << "   float3 " << resultColorName << " = lerp( " << pixelColorName << ", " << fogColorName << ", tFog_ );" << std::endl;
	}

	namespace
	{
		b32 fInvertedZ( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			return params.mPid == cPlatformXbox360;
		}

		const tProjectFile::tShadowShaderGenerationSettings& fShadowGenSettings( )
		{
			return tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mShadowShaderGenerationSettings;
		}

		b32 fCheckEnable
			( const char*											description
			, const tMaterialGenBase::tComputeShadowParameters&		shadowParams
			, const tProjectFile::tNormalShadowParameters&			featureParams )
		{
			if( !featureParams.mEnable )
				return false;
			log_sigcheckfail( fAbs( featureParams.mMagnitude ) > 0.00001f, description << " enabled but 0 magnitude", return false );
			if( shadowParams.mNDotL.empty() )
				return false; // TODO: Warning / sigcheckfail?
			return true;
		}

		b32 fCheckEnable
			( const char*											description
			, const tMaterialGenBase::tComputeShadowParameters&		shadowParams
			, const tProjectFile::tSamplingShadowMapParameters&		featureParams )
		{
			if( !featureParams.mEnable )
				return false;
			log_sigcheckfail( fAbs( featureParams.mMagnitude ) > 0.00001f, description << " enabled but 0 magnitude", return false );
			// No normal check, sampling always works.
			return true;
		}

		void fComputeShadowTerm_Calculate_ShadowLayerDerivatives
			( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			const tProjectFile::tShadowShaderGenerationSettings& gen = fShadowGenSettings( );
			sigassert( tMaterialGenBase::fShadowMapLayerCount( params.mPid ) <= Gfx::tMaterial::cMaxShadowLayers );
			std::stringstream& ss = *params.mSs;

			const b32 layered
				= ( tMaterialGenBase::fShadowMapLayerCount( params.mPid ) > 1 || params.mForceLightSpaceConversion )
				&& params.mShadowMapSamplerName.size();

			if( layered )
			{
				ss << "float numShadowLayers_ = " << tMaterialGenBase::fShadowMapLayerCount( params.mPid ) << ";\n";
				ss << "// World pos expected to be a float 4, for the mul later,\n";
				ss << "// yet needs to be interpreted as a float3 here.\n";
				ss << "float distToShadowTarget_ = length( " << params.mShadowMapTargetName << " - "
					<< params.mWorldPosName << ".xyz );\n";

				ss << "float shadowLayer_ = ";
				if( tMaterialGenBase::fShadowMapLayerCount( params.mPid ) >= 3 )
					ss << "( distToShadowTarget_  < " << params.mShadowSplitDistance1Name << " ) ? 2 : ";
				if( tMaterialGenBase::fShadowMapLayerCount( params.mPid ) >= 2 )
					ss << "( distToShadowTarget_ < " << params.mShadowSplitDistance0Name << " ) ? 1 : ";
				ss << " 0;\n";

				ss << "float4 lightSpacePos_ = mul( "
					<< params.mWorldToLightArrayName << "[ ( int )shadowLayer_ ], "
					<< params.mWorldPosName << " );" << std::endl;
			}
			else
			{
				// if no cascades, then this has already been transformed to light space pos
				ss << "float4 lightSpacePos_ = " << params.mWorldPosName << ";\n";
			}

			const char* const shadowEpsilonComponent = layered ? "[ ( int )shadowLayer_ ]" : ".x";
			ss << "float shadowEpsilon_ = " << params.mShadowEpsilonName << shadowEpsilonComponent << ";" << std::endl;
		}


		void fComputeShadowTerm_Calculate_MiscDerivatives( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			std::stringstream& ss = *params.mSs;

			ss << "// transform from render target space to texture space." << std::endl;
			ss << "float2 shadowTexCoord_ = 0.5 * lightSpacePos_.xy + float2( 0.5, 0.5 );" << std::endl;
			ss << "shadowTexCoord_.y = 1.0f - shadowTexCoord_.y;" << std::endl;
			ss << std::endl;

			ss << "float oneTexelOffset_ = 1.0f / " << params.mShadowMapTexelSizeName << ";" << std::endl;
			ss << "float2 shadowTexelPos_ = " << params.mShadowMapTexelSizeName << " * shadowTexCoord_;" << std::endl;
			// light-space depth (divided by w) -- already inverted by fUnPackAllData if necessary
			ss << "float lightSpaceDepth_ = saturate( lightSpacePos_.z );" << std::endl;
		}

		void fComputeShadowTerm_Calculate_Noise( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			std::stringstream& ss = *params.mSs;

			// screenCoords doesn't continuously animate, shadow based ones do due to camera moving.
			ss << "float2 noise = frac(sin(dot( screenCoords,float2(12.9898,78.233)*2.0)) * 43758.5453);\n";
			//ss << "float2 noise = frac(sin(dot( shadowTexCoord_,float2(12.9898,78.233)*2.0)) * 43758.5453);\n";
			//ss << "float2 noise = frac(sin(dot( shadowTexelPos_,float2(12.9898,78.233)*2.0)) * 43758.5453);\n";
			ss << "noise = noise * 2 - float2( 1.0, 1.0 ); // remap to the [-1..+1] range\n";
		}

		void fComputeShadowTerm_SinStartEndAngles_FadeIn
			( const tMaterialGenBase::tComputeShadowParameters&		shadowParams
			, const tProjectFile::tNormalShadowParameters&			featureParams )
		{
			std::stringstream& ss = *shadowParams.mSs;
			const f32 startDeg = featureParams.mAngleStart;
			const f32 endDeg = featureParams.mAngleEnd;
			const f32 startSin = Math::fSin( Math::fToRadians( startDeg ) );
			const f32 endSin = Math::fSin( Math::fToRadians( endDeg ) );


			if( fEqual( startDeg, endDeg ) )
			{
				ss << "	float sinAngle_ = " << startSin << "; // Sin(" << startDeg << " degrees)\n";
				ss << "	float progress = " << shadowParams.mNDotL << " < sinAngle_;\n";
			}
			else
			{
				ss << "	float sinStartAngle_ = " << startSin << "; // Sin(" << startDeg << " degrees)\n";
				ss << "	float sinEndAngle_ = " << endSin << "; // Sin(" << endDeg << " degrees)\n";
				ss << "	float progress = smoothstep( sinStartAngle_, sinEndAngle_, " << shadowParams.mNDotL << " );\n";
			}
			if( fAbs( 1.0f - featureParams.mMagnitude ) > 0.00001f )
				ss << "	progress *= " << featureParams.mMagnitude << "; // Adjust for magnitude\n";
		}

		void fComputeShadowTerm_SinStartEndAngles_Edge
			( const tMaterialGenBase::tComputeShadowParameters&		shadowParams
			, const tProjectFile::tNormalShadowParameters&			featureParams )
		{
			std::stringstream& ss = *shadowParams.mSs;
			fComputeShadowTerm_SinStartEndAngles_FadeIn( shadowParams, featureParams );
			ss << "	progress = abs( " << featureParams.mMagnitude << " - 2.0*progress ); // Edge effect\n";
		}

		void fComputeShadowTerm_DebugProgress
			( const tMaterialGenBase::tComputeShadowParameters&		shadowParams
			, const tProjectFile::tCommonShadowParameters&			featureParams )
		{
			if( shadowParams.mDebugShadowTermName.empty() )
				return;
			if( !featureParams.mDebugChannel.fExists() )
				return;

			std::stringstream& ss = *shadowParams.mSs;
			ss << "	" << shadowParams.mDebugShadowTermName << "." << featureParams.mDebugChannel << " = progress;\n";
		}

		void fComputeShadowTerm_NormalShadowValue( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			std::stringstream& ss = *params.mSs;

			ss << "// Normal Shadow Value\n";
			ss << "{\n";
			fComputeShadowTerm_SinStartEndAngles_FadeIn	( params, fShadowGenSettings().mNormalShadowValue );
			fComputeShadowTerm_DebugProgress			( params, fShadowGenSettings().mNormalShadowValue );
			ss << "	shadowAmount_ = max( shadowAmount_, progress );\n";
			ss << "}\n\n";
		}

		void fComputeShadowTerm_NormalShadowSampleBlur( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			std::stringstream& ss = *params.mSs;

			ss << "// Normal Shadow Sampling Spread Blur\n";
			ss << "{\n";
			fComputeShadowTerm_SinStartEndAngles_Edge	( params, fShadowGenSettings().mNormalShadowBlurSample );
			fComputeShadowTerm_DebugProgress			( params, fShadowGenSettings().mNormalShadowBlurSample );
			ss << "	shadowTapOffsets_[0] += float2( -progress, -progress );\n";
			ss << "	shadowTapOffsets_[1] += float2( +progress, -progress );\n";
			ss << "	shadowTapOffsets_[2] += float2( -progress, +progress );\n";
			ss << "	shadowTapOffsets_[3] += float2( +progress, +progress );\n";
			ss << "}\n\n";
		}

		void fComputeShadowTerm_NormalShadowSampleMove( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			std::stringstream& ss = *params.mSs;

			fComputeShadowTerm_Calculate_Noise( params );
			ss << "// Normal Shadow Sampling Noise Blur\n";
			ss << "{\n";
			fComputeShadowTerm_SinStartEndAngles_Edge	( params, fShadowGenSettings().mNormalShadowMoveSample );
			fComputeShadowTerm_DebugProgress			( params, fShadowGenSettings().mNormalShadowMoveSample );
			ss << "	float2 dir = noise * progress * oneTexelOffset_;\n";
			ss << "	shadowTapOffsets_[0] += dir;\n";
			ss << "	shadowTapOffsets_[1] += dir;\n";
			ss << "	shadowTapOffsets_[2] += dir;\n";
			ss << "	shadowTapOffsets_[3] += dir;\n";
			ss << "}\n\n";
		}

		void fComputeShadowTerm_SampleShadowMap( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			std::stringstream& ss = *params.mSs;
			const b32 is3D = tMaterialGenBase::fShadowMapLayerCount( params.mPid ) > 1;

			ss << "// Sample the shadow map\n";
			ss << "float shadowDepthTaps_[4];\n";
			ss << "{\n";
			if( is3D )
			{
				ss << "	float shadowZcoord_ = 1.0 / ( 2.0 * numShadowLayers_ ) + shadowLayer_ / numShadowLayers_;\n";
			}
			for( u32 tap=0; tap<4; ++tap )
			{
				const std::string texCoord2d = "shadowTexCoord_ + shadowTapOffsets_["+StringUtil::fToString(tap)+"]";
				const std::string texCoord = is3D ? "float3( " + texCoord2d + ", shadowZcoord_ )" : texCoord2d;

				ss << "	shadowDepthTaps_[ " << tap << " ] = " << (fInvertedZ(params) ? "1.0 - " : "");

				// This will become more complicated when D3D11 merges, to support 2D texture arrays for shadows:
				if( tMaterialGenBase::fShadowMapLayerCount( params.mPid ) > 1 )
					ss << "tex3D( " << params.mShadowMapSamplerName << ", " << texCoord << " ).r;\n";
				else
					ss << "tex2D( " << params.mShadowMapSamplerName << ", " << texCoord << " ).r;\n";

				// Old binary shadow/no-shadow:
				//ss << sample.str() << " " << cShadowMapCompareFunc << " startShadowFadeIn_ ? 0.0 : 1.0;\n";
				//ss << sample.str() << " - startShadowFadeIn_ " << cShadowMapCompareFunc << "0.0 ? 0.0 : 1.0;\n";
				//ss << "smoothstep( endShadowFadeIn_, startShadowFadeIn_, " << sample.str() << " );\n";
				//ss << (invertedZ ? "1.0 - " : "") << sample.str( ) << ";\n";
			}
			ss << "}\n\n";
		}

		void fComputeShadowTerm_SummarizeShadowMapSamples( const tMaterialGenBase::tComputeShadowParameters& params )
		{
			std::stringstream& ss = *params.mSs;

			ss << "// Summarize the shadow map samples\n";
			ss << "float2 shadowLerps_ = frac( shadowTexelPos_ );" << std::endl;
			ss << "float maxShadowDepth = max( max( shadowDepthTaps_[0], shadowDepthTaps_[1] ), max( shadowDepthTaps_[2], shadowDepthTaps_[3] ) );\n";
			ss << "float minShadowDepth = min( min( shadowDepthTaps_[0], shadowDepthTaps_[1] ), min( shadowDepthTaps_[2], shadowDepthTaps_[3] ) );\n";
			ss << "float startShadowFadeIn_ = lightSpaceDepth_ - shadowEpsilon_;\n";
			ss << "float endShadowFadeIn_ = startShadowFadeIn_ - shadowEpsilon_;\n";
			ss << "\n";
		}

		void fComputeShadowTerm_OccludedBySamples
			( const tMaterialGenBase::tComputeShadowParameters&		shadowParams
			, const tProjectFile::tSamplingShadowMapParameters&		featureParams
			, u32													reqCount
			, u32													checkCount )
		{
			std::stringstream& ss = *shadowParams.mSs;

			ss << "// Is the pixel occluded by at least " << reqCount << " of " << checkCount << " samples?\n";
			ss << "{\n";
			ss << "	float occluders = 0.0;\n";
			for( u32 tap=0; tap<checkCount; ++tap )
				ss << "	occluders += shadowDepthTaps_["<<tap<<"] + shadowEpsilon_ < lightSpaceDepth_;\n";
			ss << "	float progress = occluders >= " << (checkCount-1) << ".99;\n";
			ss << "	progress *= " << featureParams.mMagnitude << ";\n";
			fComputeShadowTerm_DebugProgress( shadowParams, featureParams );
			ss << "	shadowAmount_ = max( shadowAmount_, progress );\n";
			ss << "}\n\n";
		}

		void fComputeShadowTerm_PercentageCloserFiltering
			( const tMaterialGenBase::tComputeShadowParameters&		shadowParams
			, const tProjectFile::tSamplingShadowMapParameters&		featureParams )
		{
			std::stringstream& ss = *shadowParams.mSs;

			ss << "// Percentage Closer Filtering Pass\n";
			ss << "{\n";
			ss << "	float pcfShadowSamples_[4];\n";
			for( u32 tap=0; tap<4; ++tap )
				ss << "	pcfShadowSamples_["<<tap<<"] = smoothstep( startShadowFadeIn_, endShadowFadeIn_, shadowDepthTaps_["<<tap<<"] );\n";
				//ss << "	pcfShadowSamples_["<<tap<<"] = shadowDepthTaps_["<<tap<<"] < lightSpaceDepth_ - shadowEpsilon_;\n";
			ss << "	float pcfShadowX1_ = lerp( pcfShadowSamples_[0], pcfShadowSamples_[1], shadowLerps_.x );\n";
			ss << "	float pcfShadowX2_ = lerp( pcfShadowSamples_[2], pcfShadowSamples_[3], shadowLerps_.x );\n";
			ss << "	float progress = lerp( pcfShadowX1_, pcfShadowX2_, shadowLerps_.y );\n";
			ss << "	progress *= " << featureParams.mMagnitude << ";\n";
			fComputeShadowTerm_DebugProgress( shadowParams, featureParams );
			ss << "	shadowAmount_ = max( shadowAmount_, progress );\n";
			ss << "}\n\n";
		}

		void fComputeShadowTerm_EstimatedNormalCheck
			( const tMaterialGenBase::tComputeShadowParameters&		shadowParams
			, const tProjectFile::tSamplingShadowMapParameters&		featureParams )
		{
			std::stringstream& ss = *shadowParams.mSs;

			ss << "// Estimated normal check\n";
			ss << "{\n";
			ss << "	float estimatedShadowDepth_ = lerp( lerp( shadowDepthTaps_[0], shadowDepthTaps_[1], shadowLerps_.x ), lerp( shadowDepthTaps_[2], shadowDepthTaps_[3], shadowLerps_.x ), shadowLerps_.y );" << std::endl;
			ss << "	estimatedShadowDepth_ = smoothstep( minShadowDepth, maxShadowDepth, estimatedShadowDepth_ );\n";
			ss << "	estimatedShadowDepth_ = estimatedShadowDepth_ * estimatedShadowDepth_;\n"; // square, biased towards near
			ss << "	estimatedShadowDepth_ = lerp( minShadowDepth, maxShadowDepth, estimatedShadowDepth_ );\n";
			if( !shadowParams.mNDotL.empty() )
				ss << "	estimatedShadowDepth_ += " << shadowParams.mNDotL << " * 0.5;\n";
			ss << "	float progress = smoothstep( startShadowFadeIn_, endShadowFadeIn_, estimatedShadowDepth_ );\n";
			ss << "	progress *= " << featureParams.mMagnitude << ";\n";
			fComputeShadowTerm_DebugProgress( shadowParams, featureParams );
			ss << "	shadowAmount_ = max( shadowAmount_, progress );\n";
			ss << "}\n\n";
		}
	}

	void tMaterialGenBase::fCallComputeShadowTerm( const tComputeShadowParameters& params )
	{
		sigcheckfail( params.mPid != cPlatformNone, return );
		sigcheckfail( params.mSs, return );

		// We should do cascaded shadow maps at some point. Right now we waste huge amounts of shadow texture space on
		// pointless overlaps and behind-the-player bullshit.
		//
		//		Reading material:
		// "Cascaded Shadow Maps"							http://msdn.microsoft.com/en-us/library/windows/desktop/ee416307(v=vs.85).aspx
		// "Common Techniques to Improve Shadow Depth Maps"	http://msdn.microsoft.com/en-us/library/windows/desktop/ee416324(v=vs.85).aspx

		std::stringstream& ss = *params.mSs;



		// Calculate additional temporaries based on our inputs.
		fComputeShadowTerm_Calculate_ShadowLayerDerivatives( params );
		fComputeShadowTerm_Calculate_MiscDerivatives( params );

		// Values for aggregating and modification by normal calcs.
		ss << "float shadowAmount_ = 0.0; // How much shadow applies to this pixel.\n";
		ss << "float2 shadowTapOffsets_[4]; // Where to sample the shadow map.\n";
		ss << "shadowTapOffsets_[0] = float2( 0.0, 0.0 );\n";
		ss << "shadowTapOffsets_[1] = float2( oneTexelOffset_, 0.0 );\n";
		ss << "shadowTapOffsets_[2] = float2( 0.0, oneTexelOffset_ );\n";
		ss << "shadowTapOffsets_[3] = float2( oneTexelOffset_, oneTexelOffset_ );\n";

		// Normal based shadow calcs + offset tweaks
		if( fCheckEnable( "Normal Shadow Value", params, fShadowGenSettings( ).mNormalShadowValue ) )
			fComputeShadowTerm_NormalShadowValue( params );
		if( fCheckEnable( "Normal Shadow Sample Blur", params, fShadowGenSettings( ).mNormalShadowBlurSample ) )
			fComputeShadowTerm_NormalShadowSampleBlur( params );
		if( fCheckEnable( "Normal Shadow Sample Move", params, fShadowGenSettings( ).mNormalShadowMoveSample ) )
			fComputeShadowTerm_NormalShadowSampleMove( params );

		if( params.mShadowMapSamplerName.size() )
		{
			// The expensive stuff: Sample the shadow map!
			fComputeShadowTerm_SampleShadowMap( params );
			fComputeShadowTerm_SummarizeShadowMapSamples( params );

			// Shadow map sample based calcs
			if( fCheckEnable( "Naive Single Range Check", params, fShadowGenSettings( ).mShadowMapNaieveSingleRangeCheck ) )
				fComputeShadowTerm_OccludedBySamples( params, fShadowGenSettings( ).mShadowMapNaieveSingleRangeCheck, 1, 1 );
			if( fCheckEnable( "Naive Any Range Check", params, fShadowGenSettings( ).mShadowMapNaieveBoundingRangesCheck ) )
				fComputeShadowTerm_OccludedBySamples( params, fShadowGenSettings( ).mShadowMapNaieveBoundingRangesCheck, 4, 4 );
			if( fCheckEnable( "Percentage Closer Filtering (PCF)", params, fShadowGenSettings( ).mShadowMapPercentageCloserFiltering ) )
				fComputeShadowTerm_PercentageCloserFiltering( params, fShadowGenSettings( ).mShadowMapPercentageCloserFiltering );
			if( fCheckEnable( "Estimated Map Normal Check", params, fShadowGenSettings( ).mShadowMapEstimatedNormal ) )
				fComputeShadowTerm_EstimatedNormalCheck( params, fShadowGenSettings( ).mShadowMapEstimatedNormal );
		}

		// lerp between the shadow values to calculate our light amount
		if( !params.mDebugShadowTermName.empty() )
			ss << params.mDebugShadowTermName << " = float4( 1.0, 1.0, 1.0, 1.0 ) - " << params.mDebugShadowTermName << ";\n";
		ss << "float " << params.mShadowTermName << " = 1.0 - shadowAmount_;\n";
		ss << params.mShadowTermName << " = lerp( 1.f, " << params.mShadowTermName << ", " << params.mShadowAmountName << " );" << std::endl;
	}

	void tMaterialGenBase::fCallLightAttenuate(
		std::stringstream&		ss,
		Gfx::tLight::tLightType	lightType,
		const char*				resultName,
		const char*				lightName,
		const char*				lightDistanceName )
	{
		switch( lightType )
		{
		case Gfx::tLight::cLightTypeDirection:
			ss << "		// Generated by tMaterialGenBase::fCallLightAttenuate (lightType == cLightTypeDirection)\n";
			ss << "		float " << resultName << " = 1.0;\n";
			ss << "\n";
			break;
		case Gfx::tLight::cLightTypePoint:
			ss << "		// Generated by tMaterialGenBase::fCallLightAttenuate (lightType == cLightTypePoint)\n";
			ss << "		float " << resultName << ";\n";
			ss << "		{\n";
			ss << "			float lightDist_					= " << lightDistanceName << ";\n";
			ss << "			float outerRadius_					= " << lightName << ".mAttenuation.x;\n";
			ss << "			float rcpInnerOuterRadiusDistance_	= " << lightName << ".mAttenuation.y;\n";
			ss << "\n";
			ss << "			float attenFactor_ = saturate( (outerRadius_ - lightDist_) * rcpInnerOuterRadiusDistance_ );\n";
			ss << "			attenFactor_ *= attenFactor_; // non linear falloff\n";
			ss << "			" << resultName << " = attenFactor_;\n";
			ss << "		}\n";
			ss << "\n";
			break;
		default:
			log_warning( "Invalid light type in tMaterialGenBase::fCallLightAttenuate" );
			break;
		}
	}

	void tMaterialGenBase::fCallAccumulateLight( 
		std::stringstream&		ss, 
		b32						calcSpec,
		Gfx::tLight::tLightType	lightType,
		const std::string&		lightName,
		const std::string&		positionName,
		const std::string&		normalName,
		const std::string&		specSizeName,
		const std::string&		diffAccumName,
		const std::string&		specAccumName,
		const std::string&		specMagAccumName,
		const std::string&		shadowTermName,
		b32						calcAmbient,
		const std::string&		ambientWeightsName )
	{
		ss << "      float3 lc0_ = " << lightName << ".mColors[ 0 ].rgb * " << shadowTermName << ";" << std::endl;
		ss << "      float3 lc1_ = " << lightName << ".mColors[ 1 ].rgb * " << shadowTermName << ";" << std::endl;
		ss << "      float3 lc2_ = " << lightName << ".mColors[ 2 ].rgb;" << std::endl;
		if( calcSpec )
			ss << "      float3 ls_ = lc0_;" << std::endl;

		//ss << "      float3 la_ = " << lightName << ".mAmbient.rgb;" << std::endl;
		if( calcAmbient )
		{
			ss << "      float4 la_;" << std::endl;
			fSphericalLookup( ss, normalName, "la_", ambientWeightsName );
		}
		else
			ss << "      float4 la_ = float4( 0,0,0,0 );" << std::endl;

		ss << "      float3 l2v_ = " << lightName << ".mPosition.xyz - " << positionName << ";" << std::endl;
		ss << "      float lLen_ = length( l2v_ );" << std::endl;
		ss << "      float3 l_ = " << lightName << ".mDirection.xyz * " << lightName << ".mDirection.w + ( l2v_ / lLen_ ) * " << lightName << ".mPosition.w;" << std::endl;

		ss << "      float nDotL_ = dot( " << normalName << ", l_ );" << std::endl;
		ss << "      float3 litResult_ = float3( 1.f, nDotL_, 0.f );" << std::endl;

		if( calcSpec )
		{
			// compute half vector
			ss << "      float3 h_ = normalize( normalize( -" << positionName << " ) + l_ );" << std::endl;
			ss << "      float nDotH_ = dot( " << normalName << ", h_ );" << std::endl;

			// compute spec term
			ss << "      litResult_.z = lit( nDotL_, nDotH_, " << specSizeName << " ).z;" << std::endl;
		}

		fCallLightAttenuate( ss, lightType, "atten_", lightName.c_str(), "lLen_" );
		ss << "      litResult_.z *= atten_;" << std::endl;

		ss << diffAccumName << " += la_.xyz + atten_*((lc0_ * max(0,litResult_.y)) + (lc1_ * (1-abs(litResult_.y))) + (lc2_ * max(0,-litResult_.y)));" << std::endl;
		if( calcSpec )
		{
			ss << specAccumName << " += ls_ * litResult_.z;" << std::endl;
			ss << specMagAccumName << " += litResult_.z;" << std::endl;
		}
	}


	void tMaterialGenBase::fSphericalLookup( std::stringstream& ss, const std::string& normal, const std::string& output, const std::string& weightsName )
	{
		ss << "float3 sh_dirn = " << normal << ".xyz;" << std::endl;
		ss << "\t" << output << " = " << weightsName << "[0];" << std::endl;
		ss << "\t" << output << " += " << weightsName << "[1] * sh_dirn.x;" << std::endl;
		ss << "\t" << output << " += " << weightsName << "[2] * sh_dirn.y;" << std::endl;
		ss << "\t" << output << " += " << weightsName << "[3] * sh_dirn.z;" << std::endl;
		ss << "\t" << output << " += " << weightsName << "[4] * (sh_dirn.x*sh_dirn.z);" << std::endl;
		ss << "\t" << output << " += " << weightsName << "[5] * (sh_dirn.z*sh_dirn.y);" << std::endl;
		ss << "\t" << output << " += " << weightsName << "[6] * (sh_dirn.y*sh_dirn.x);" << std::endl;
		ss << "\t" << output << " += " << weightsName << "[7] * (2.0f * sh_dirn.z*sh_dirn.z - sh_dirn.x*sh_dirn.x - sh_dirn.y+sh_dirn.y);" << std::endl;
		ss << "\t" << output << " += " << weightsName << "[8] * (sh_dirn.x*sh_dirn.x - sh_dirn.y+sh_dirn.y);" << std::endl;
	}

}
