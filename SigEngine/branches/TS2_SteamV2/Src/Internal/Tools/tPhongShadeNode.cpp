#include "ToolsPch.hpp"
#include "tPhongShadeNode.hpp"
#include "DerivedShadeNodes.hpp"
#include "tMaterialGenBase.hpp"
#include "Editor/tEditablePropertyTypes.hpp"
#include "Editor/tEditablePropertyColor.hpp"
#include "HlslGen/tPixelShaderVars.hpp"
#include "HlslGen/tHlslGenTree.hpp"
#include "HlslGen/tHlslWriter.hpp"

namespace Sig
{


	namespace
	{
		const wxColour cLightingModelsBarColor( 0xdd, 0x44, 0x22 );
		const wxColour cLightingModelsTextColor( 0xee, 0xee, 0xaa );

		const char cPropNameReceiveShadow[]="Shader Properties.RcvShadow";
		const char cPropNameAddSpecToAlpha[]="Shader Properties.AlphaAddSpec";
	}

	register_rtti_factory( tPhongShadeNode, false );
	tPhongShadeNode::tPhongShadeNode( const wxPoint& p )
		: tShadeNode( "Phong", cLightingModelsBarColor, cLightingModelsTextColor, false, p )
	{
		fAddInput( "Diffuse" );
		fAddInput( "SpecColor" );
		fAddInput( "SpecFalloff" );
		fAddInput( "Normal" );
		fAddInput( "SelfIllum" );
		fAddInput( "Transmission" );
		fAddInput( "Ambient" );
		fAddInput( "Opacity" );
		fAddInput( "Rim" );
		fComputeDimensions( );
		fAddOutput( );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cPropNameReceiveShadow, true ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cPropNameAddSpecToAlpha, false ) ) );
	}
	b32 tPhongShadeNode::fInputNeedsWritingToHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tHlslGenTree& hlslGenTree, u32 ithInput )
	{
		using namespace HlslGen;
		switch( writer.fWriteMode( ) )
		{
		case cWriteModeColor: return true;
		case cWriteModeDepth: return false;
		case cWriteModeDepthWithAlpha: return( ithInput == cChannelOpacity );
		default: sigassert( !"invalid case" ); break;
		}
		return false;
	}
	void tPhongShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		// TODO handle case that no channels requiring lighting are specified

		const b32 isColor = ( writer.fWriteMode( ) == cWriteModeColor );

		if( isColor )
		{
			reqs.mNumLights = fMax( Gfx::tMaterial::cMaxLights, reqs.mNumLights );

			// add requirements for globals
			reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalLightParams, writer.fInput( ).mPid ) );
			if( hlslGenTree.fInputs( )[ cChannelRim ] )
				reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalRimLight, writer.fInput( ).mPid ) );

			// add requirements for inputs
			reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, writer.fInput( ).mPid ) );

			// add common temps
			if( hlslGenTree.fInputs( )[ cChannelSpecColor ] || hlslGenTree.fInputs( )[ cChannelRim ] )
				reqs.mCommon.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexDir, writer.fInput( ).mPid ) );

			// add requirements for shadow mapping only if specified
			const b32 receiveShadow = writer.fInput( ).mPreviewMode ? false : fShadeProps( ).fGetValue( cPropNameReceiveShadow, true );

			if( receiveShadow )
			{
				reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalShadowMapSampler, writer.fInput( ).mPid ) );
				reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobal_BackFace_ShadowMapEpsilon_TexelSize_Amount, writer.fInput( ).mPid ) );
				reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInLightPos, writer.fInput( ).mPid ) );

				const u32 shadowLayerCount = tMaterialGenBase::fShadowMapLayerCount( writer.fInput( ).fRealPlatformId( ) );
				if( shadowLayerCount > 1 )
				{
					reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobal_ShadowMapTargetPos, writer.fInput( ).mPid ) );
					reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobal_ShadowMapSplits, writer.fInput( ).mPid ) );
					reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalWorldToLightSpaceArray, writer.fInput( ).mPid ) );
				}
			}

			if( !hlslGenTree.fInputs( )[ cChannelNormal ] ) // add default normal if none was specified
				hlslGenTree.fForceSetInput( cChannelNormal, tHlslGenTreePtr( new tHlslGenTree( tShadeNodePtr( new tGeometryNormalShadeNode( ) ), &hlslGenTree ) ) );
		}
	}
	void tPhongShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		// TODO handle case that no channels requiring lighting are specified

		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		const b32 isColor = ( writer.fWriteMode( ) == cWriteModeColor );

		if( isColor )
		{
			tHlslVariableConstPtr effectiveNormal = inputResults[ cChannelNormal ];
			sigassert( effectiveNormal );

			const b32 receiveShadow = writer.fInput( ).mPreviewMode ? false : fShadeProps( ).fGetValue( cPropNameReceiveShadow, true );

			tHlslVariableConstPtr shadowTerm;
			if( receiveShadow && reqs.mNumLights > 0 )
				shadowTerm = fWriteShadowTerm( writer );

			tHlslVariableConstPtr colorResult;

			const b32 hasChannelsUsingLights = ( inputResults[ cChannelDiffuse ] || inputResults[ cChannelSpecColor ] || inputResults[ cChannelTransmission ] || inputResults[ cChannelRim ] );
			if( reqs.mNumLights > 0 && hasChannelsUsingLights )
				colorResult = fWriteLightLoop( writer, reqs, inputResults, effectiveNormal, shadowTerm );
			else
			{
				colorResult = writer.fMakeTempVector( "colorResult", 4 );
				writer.fBeginLine( true ) << "// without light, we get black" << std::endl;
				writer.fBeginLine( ) << colorResult->fDeclaration( )
					<< " = " << colorResult->fCastValueToType( 0.f ) << ";" << std::endl;
			}
			sigassert( colorResult && colorResult->fDimensionX( ) == 4 );

			// add emissive channel
			tHlslVariableConstPtr emissiveResult = inputResults[ cChannelEmissive ];
			if( emissiveResult )
			{
				writer.fBeginLine( true ) << "// add self illumination" << std::endl;
				writer.fBeginLine( ) << colorResult->fSwizzle( "rgb" ) << " += " << emissiveResult->fSwizzle( 3 ) << ";" << std::endl;
			}

			// add opacity channel
			tHlslVariableConstPtr opacityResult = inputResults[ cChannelOpacity ];
			writer.fBeginLine( true ) << "// add opacity" << std::endl;
			if( opacityResult )
				writer.fBeginLine( ) << colorResult->fSwizzle( "a" ) << " += " << opacityResult->fSwizzle( 1 ) << ";" << std::endl;
			else
				writer.fBeginLine( ) << colorResult->fSwizzle( "a" ) << " = 1.0;" << std::endl;

			hlslGenTree.fCacheShadeNodeOutput( colorResult );
		}
		else
		{
			// output opacity channel
			tHlslVariableConstPtr opacityResult = inputResults[ cChannelOpacity ];
			sigassert( opacityResult );

			tHlslVariableConstPtr opacityOutput = writer.fMakeTempVector( "opacity", 4 );
			writer.fBeginLine( true ) << "// output opacity" << std::endl;
			writer.fBeginLine( ) << opacityOutput->fDeclaration( )
				<< " = " << opacityOutput->fCastValueToType( 0.f ) << ";" << std::endl;
			writer.fBeginLine( ) << opacityOutput->fSwizzle( "a" ) << " = " << opacityResult->fSwizzle( 1 ) << ";" << std::endl;

			hlslGenTree.fCacheShadeNodeOutput( opacityOutput );
		}
	}

	HlslGen::tHlslVariableConstPtr tPhongShadeNode::fWriteShadowTerm( HlslGen::tHlslWriter& writer )
	{
		using namespace HlslGen;

		tHlslVariableConstPtr lightParams = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalLightParams, writer.fInput( ).mPid );
		tHlslVariableConstPtr lightPos = tPixelShaderVars::fVariable( tPixelShaderVars::cInLightPos, writer.fInput( ).mPid );
		tHlslVariableConstPtr backFace_epsilon_texelSize_amount = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobal_BackFace_ShadowMapEpsilon_TexelSize_Amount, writer.fInput( ).mPid );

		std::string lightPosName;
		const std::string worldPosName = lightPos->fName( );
		const std::string shadowEpsilonName = backFace_epsilon_texelSize_amount->fSwizzle( "y" );
		const std::string shadowTexelSizeName = backFace_epsilon_texelSize_amount->fSwizzle( "z" );
		const std::string shadowAmountName = backFace_epsilon_texelSize_amount->fSwizzle( "w" );

		writer.fBeginLine( true ) << "//" << std::endl;
		writer.fBeginLine( ) << "// compute shadow term..." << std::endl;
		writer.fBeginLine( ) << "//" << std::endl;

		const u32 shadowLayerCount = tMaterialGenBase::fShadowMapLayerCount( writer.fInput( ).fRealPlatformId( ) );
		if( shadowLayerCount > 1 )
		{
			tHlslVariableConstPtr shadowMapTargetPos = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobal_ShadowMapTargetPos, writer.fInput( ).mPid );
			tHlslVariableConstPtr shadowMapSplits = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobal_ShadowMapSplits, writer.fInput( ).mPid );
			tHlslVariableConstPtr worldToLightSpaceArray = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalWorldToLightSpaceArray, writer.fInput( ).mPid );

			sigassert( shadowLayerCount <= Gfx::tMaterial::cMaxShadowLayers );

			writer.fBeginLine( ) << "float numShadowLayers_ = " << shadowLayerCount << ";" << std::endl;
			writer.fBeginLine( ) << "float distToShadowTarget_ = length( " << shadowMapTargetPos->fSwizzle( "xyz" ) << " - " << worldPosName << " );" << std::endl;

			//ss << "float shadowLayer_ = 1;" << std::endl;
			if( shadowLayerCount == 2 )
				writer.fBeginLine( ) << "float shadowLayer_ = saturate( distToShadowTarget_  < " << shadowMapSplits->fSwizzle( "x" ) << " );" << std::endl;
			else if( shadowLayerCount == 3 )
				writer.fBeginLine( ) << "float shadowLayer_ = ( distToShadowTarget_  < " << shadowMapSplits->fSwizzle( "y" ) << " ) ? 2 : ( distToShadowTarget_ < " << shadowMapSplits->fSwizzle( "x" ) << " ? 1 : 0 );" << std::endl;
			else
				sigassert( !"invalid number of shadow layers" );


			lightPosName = "lightSpacePos_";
			writer.fBeginLine( ) << "float4 " << lightPosName << " = mul( " << worldToLightSpaceArray->fName( ) << "[ ( int )shadowLayer_ ], " << worldPosName << " );" << std::endl;
		}
		else
			lightPosName = worldPosName;

		sigassert( lightPosName.length( ) > 0 );


		tHlslVariableConstPtr shadowTexCoord = writer.fMakeTempVector( "shadowTexCoord", 2 );
		writer.fBeginLine( true ) << "// transform from render target space to texture space" << std::endl;
		writer.fBeginLine( ) << shadowTexCoord->fDeclaration( ) 
			<< " = 0.5 * " << lightPosName << ".xy + float2( 0.5, 0.5 );" << std::endl;
		writer.fBeginLine( ) << shadowTexCoord->fSwizzle( "y" ) << " = 1 - " << shadowTexCoord->fSwizzle( "y" ) << ";" << std::endl;

		tHlslVariableConstPtr shadowTexelPos = writer.fMakeTempVector( "shadowTexelPos", 2 );
		writer.fBeginLine( true ) << "// transform to texel space" << std::endl;
		writer.fBeginLine( ) << shadowTexelPos->fDeclaration( ) 
			<< " = " << shadowTexelSizeName << " * " << shadowTexCoord->fName( ) << ";" << std::endl;

		tHlslVariableConstPtr shadowLerps = writer.fMakeTempVector( "shadowLerps", 2 );
		writer.fBeginLine( true ) << "// determine lerp amounts" << std::endl;
		writer.fBeginLine( ) << shadowLerps->fDeclaration( ) 
			<< " = frac( " << shadowTexelPos->fName( ) << " );" << std::endl;

		tHlslVariableConstPtr texelOffset = writer.fMakeTempVector( "oneTexelOffset", 1 );
		writer.fBeginLine( true ) << "// compute texel offset" << std::endl;
		writer.fBeginLine( ) << texelOffset->fDeclaration( )
			<< " = 1.0 / " << shadowTexelSizeName << ";" << std::endl;

		tHlslVariableConstPtr lightSpaceDepth = writer.fMakeTempVector( "lightSpaceDepth", 1 );
		writer.fBeginLine( true ) << "// light-space depth (divide by dubya)" << std::endl;
		writer.fBeginLine( ) << lightSpaceDepth->fDeclaration( )
			<< " = saturate( " << lightPosName << ".z );" << std::endl;

		tHlslVariableConstPtr shadowMapSampler = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalShadowMapSampler, writer.fInput( ).mPid );
		tHlslVariableConstPtr shadowTaps = writer.fMakeTempVector( "shadowTaps", 1, 4 );
		writer.fBeginLine( true ) << "// read in bilerp stamp, doing the shadow checks" << std::endl;
		writer.fBeginLine( ) << shadowTaps->fDeclaration( ) << ";" << std::endl;

		if( shadowLayerCount > 1 )
		{
			writer.fBeginLine( ) << "float shadowZcoord_ = 1.0f / ( 2.0f * numShadowLayers_ ) + shadowLayer_ / numShadowLayers_;" << std::endl;
			writer.fBeginLine( ) << shadowTaps->fIndexArray( "0" )
				<< " = (tex3D( " << shadowMapSampler->fName( ) << ", float3(" << shadowTexCoord->fName( ) << ", shadowZcoord_) ) + " << shadowEpsilonName << " < " << lightSpaceDepth->fName( ) << ") ? 0.0f : 1.0f;" << std::endl;
			writer.fBeginLine( ) << shadowTaps->fIndexArray( "1" )
				<< " = (tex3D( " << shadowMapSampler->fName( ) << ", float3(" << shadowTexCoord->fName( ) << " + float2(" << texelOffset->fName( ) << ", 0), shadowZcoord_) ) + " << shadowEpsilonName << " < " << lightSpaceDepth->fName( ) << ") ? 0.0f : 1.0f;" << std::endl;
			writer.fBeginLine( ) << shadowTaps->fIndexArray( "2" )
				<< " = (tex3D( " << shadowMapSampler->fName( ) << ", float3(" << shadowTexCoord->fName( ) << " + float2(0, " << texelOffset->fName( ) << "), shadowZcoord_) ) + " << shadowEpsilonName << " < " << lightSpaceDepth->fName( ) << ") ? 0.0f : 1.0f;" << std::endl;
			writer.fBeginLine( ) << shadowTaps->fIndexArray( "3" )
				<< " = (tex3D( " << shadowMapSampler->fName( ) << ", float3(" << shadowTexCoord->fName( ) << " + float2(" << texelOffset->fName( ) << ", " << texelOffset->fName( ) << "), shadowZcoord_) ) + " << shadowEpsilonName << " < " << lightSpaceDepth->fName( ) << ") ? 0.0f : 1.0f;" << std::endl;
		}
		else
		{
			writer.fBeginLine( ) << shadowTaps->fIndexArray( "0" )
				<< " = (tex2D( " << shadowMapSampler->fName( ) << ", " << shadowTexCoord->fName( ) << " ) + " << shadowEpsilonName << " < " << lightSpaceDepth->fName( ) << ") ? 0.0f : 1.0f;" << std::endl;
			writer.fBeginLine( ) << shadowTaps->fIndexArray( "1" )
				<< " = (tex2D( " << shadowMapSampler->fName( ) << ", " << shadowTexCoord->fName( ) << " + float2(" << texelOffset->fName( ) << ", 0) ) + " << shadowEpsilonName << " < " << lightSpaceDepth->fName( ) << ") ? 0.0f : 1.0f;" << std::endl;
			writer.fBeginLine( ) << shadowTaps->fIndexArray( "2" )
				<< " = (tex2D( " << shadowMapSampler->fName( ) << ", " << shadowTexCoord->fName( ) << " + float2(0, " << texelOffset->fName( ) << ") ) + " << shadowEpsilonName << " < " << lightSpaceDepth->fName( ) << ") ? 0.0f : 1.0f;" << std::endl;
			writer.fBeginLine( ) << shadowTaps->fIndexArray( "3" )
				<< " = (tex2D( " << shadowMapSampler->fName( ) << ", " << shadowTexCoord->fName( ) << " + float2(" << texelOffset->fName( ) << ", " << texelOffset->fName( ) << ") ) + " << shadowEpsilonName << " < " << lightSpaceDepth->fName( ) << ") ? 0.0f : 1.0f;" << std::endl;
		}

		tHlslVariableConstPtr shadowTerm = writer.fMakeTempVector( "shadowTerm", 1 );
		writer.fBeginLine( true ) << "// lerp between the shadow values to calculate our light amount" << std::endl;
		writer.fBeginLine( ) << shadowTerm->fDeclaration( )
			<< " = lerp( lerp( " << shadowTaps->fIndexArray( "0" ) << ", " << shadowTaps->fIndexArray( "1" ) << ", " << shadowLerps->fSwizzle( "x" ) << " ), lerp( " << shadowTaps->fIndexArray( "2" ) << ", " << shadowTaps->fIndexArray( "3" ) << ", " << shadowLerps->fSwizzle( "x" ) << " ), " << shadowLerps->fSwizzle( "y" ) << " );" << std::endl;
		writer.fBeginLine( ) << shadowTerm->fName( ) << " = lerp( 1.f, " << shadowTerm->fName( ) << ", " << shadowAmountName << " );" << std::endl;
		return shadowTerm;
	}

	HlslGen::tHlslVariableConstPtr tPhongShadeNode::fWriteLightLoop( 
		HlslGen::tHlslWriter& writer, 
		const HlslGen::tShaderRequirements& reqs, 
		const HlslGen::tHlslGenTree::tInputResultArray& inputResults,
		const HlslGen::tHlslVariableConstPtr& effectiveNormal,
		const HlslGen::tHlslVariableConstPtr& shadowTerm )
	{
		using namespace HlslGen;

		tHlslVariableConstPtr diffuseResult = inputResults[ cChannelDiffuse ];
		tHlslVariableConstPtr transmissionResult = inputResults[ cChannelTransmission ];
		tHlslVariableConstPtr specColorResult = inputResults[ cChannelSpecColor ];
		tHlslVariableConstPtr ambientResult = inputResults[ cChannelAmbient ];
		tHlslVariableConstPtr rimResult = inputResults[ cChannelRim ];
		std::string specFalloffResultText;

		const b32 hasChannelsUsingLights = ( diffuseResult || specColorResult || transmissionResult || rimResult );
		sigassert( reqs.mNumLights > 0 && hasChannelsUsingLights );

		writer.fBeginLine( true ) << "//" << std::endl;
		writer.fBeginLine( ) << "// perform surface lighting calculations..." << std::endl;
		writer.fBeginLine( ) << "//" << std::endl;

		writer.fBeginLine( true ) << "// declare variables to accumulate light computations" << std::endl;

		// diffuse and spec accumulators for lights
		tHlslVariableConstPtr diffAccum, transAccum, specAccum, specMagAccum, rimAccum;
		if( diffuseResult )
		{
			diffAccum = writer.fMakeTempVector( "diffAccum", 3 );
			writer.fBeginLine( ) << diffAccum->fDeclaration( ) << " = " << diffAccum->fCastValueToType( 0.f ) << ";" << std::endl;
		}
		if( transmissionResult )
		{
			transAccum = writer.fMakeTempVector( "transAccum", 3 );
			writer.fBeginLine( ) << transAccum->fDeclaration( ) << " = " << transAccum->fCastValueToType( 0.f ) << ";" << std::endl;
		}
		if( specColorResult )
		{
			specAccum = writer.fMakeTempVector( "specAccum", 3 );
			specMagAccum = writer.fMakeTempVector( "specMagAccum", 1 );

			writer.fBeginLine( ) << specAccum->fDeclaration( ) << " = " << specAccum->fCastValueToType( 0.f ) << ";" << std::endl;
			writer.fBeginLine( ) << specMagAccum->fDeclaration( ) << " = " << specMagAccum->fCastValueToType( 0.f ) << ";" << std::endl;

			if( inputResults[ cChannelSpecFalloff ] )
				specFalloffResultText = inputResults[ cChannelSpecFalloff ]->fSwizzle( 1 );
			else
				specFalloffResultText = "0.25f";
		}
		if( rimResult )
		{
			rimAccum = writer.fMakeTempVector( "rimAccum", 3 );
			writer.fBeginLine( ) << rimAccum->fDeclaration( ) << " = " << rimAccum->fCastValueToType( 0.f ) << ";" << std::endl;
		}


		// call inner light loop
		sigassert( reqs.mNumLights >= 1 );
		fWriteLightLoopInner( writer, effectiveNormal, shadowTerm, ambientResult, diffAccum, transAccum, specAccum, specMagAccum, rimAccum, specFalloffResultText, 1, 0, false );
		if( reqs.mNumLights > 1 )
			fWriteLightLoopInner( writer, effectiveNormal, shadowTerm, ambientResult, diffAccum, transAccum, specAccum, specMagAccum, rimAccum, specFalloffResultText, reqs.mNumLights, 1, true );


		HlslGen::tHlslVariableConstPtr colorResult = writer.fMakeTempVector( "colorResult", 4 );
		writer.fBeginLine( true ) << "// combine channels by light accumulation values" << std::endl;
		writer.fBeginLine( ) << colorResult->fDeclaration( ) << " = " << colorResult->fCastValueToType( 0.f ) << ";" << std::endl;

		if( diffAccum )
		{
			sigassert( diffuseResult );
			writer.fBeginLine( ) << colorResult->fSwizzle( "rgb" ) 
				<< " += " << diffAccum->fSwizzle( "rgb" ) << " * " << diffuseResult->fSwizzle( 3 ) << ";" << std::endl;
		}
		if( transAccum )
		{
			sigassert( transmissionResult );
			writer.fBeginLine( ) << colorResult->fSwizzle( "rgb" ) 
				<< " += " << transAccum->fSwizzle( "rgb" ) << " * " << transmissionResult->fSwizzle( 3 ) << ";" << std::endl;
		}
		if( specAccum )
		{
			sigassert( specColorResult );
			writer.fBeginLine( ) << colorResult->fSwizzle( "rgb" ) 
				<< " += " << specAccum->fSwizzle( "rgb" ) << " * " << specColorResult->fSwizzle( 3 ) << ";" << std::endl;

			const b32 addSpecToAlpha = fShadeProps( ).fGetValue( cPropNameAddSpecToAlpha, false );
			if( addSpecToAlpha )
			{
				writer.fBeginLine( ) << colorResult->fSwizzle( "a" )
					<< " += " << specMagAccum->fName( ) << ";" << std::endl;
			}
		}
		if( rimAccum )
		{
			sigassert( rimResult );
			writer.fBeginLine( ) << colorResult->fSwizzle( "rgb" ) 
				<< " += " << rimAccum->fSwizzle( "rgb" ) << " * " << rimResult->fSwizzle( 3 ) << ";" << std::endl;
		}

		return colorResult;
	}

	void tPhongShadeNode::fWriteLightLoopInner( 
		HlslGen::tHlslWriter& writer, 
		const HlslGen::tHlslVariableConstPtr& effectiveNormal,
		const HlslGen::tHlslVariableConstPtr& shadowTerm,
		const HlslGen::tHlslVariableConstPtr& ambientResult,
		const HlslGen::tHlslVariableConstPtr& diffAccum,
		const HlslGen::tHlslVariableConstPtr& transAccum,
		const HlslGen::tHlslVariableConstPtr& specAccum,
		const HlslGen::tHlslVariableConstPtr& specMagAccum,
		const HlslGen::tHlslVariableConstPtr& rimAccum,
		const std::string& specFalloffResultText,
		u32 numLights,
		u32 startLight,
		b32 pointLights )
	{
		using namespace HlslGen;

		tHlslVariableConstPtr litSpacePos = tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, writer.fInput( ).mPid );
		tHlslVariableConstPtr eyeToVertex = tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexDir, writer.fInput( ).mPid );

		if( pointLights )
			writer.fBeginLine( true ) << "// for each point light, perform lighting calculation for " << fMatEdDisplayName( ) << std::endl;
		else
			writer.fBeginLine( true ) << "// perform directional lighting calculation for " << fMatEdDisplayName( ) << std::endl;

		if( numLights > 1 )
		{
			writer.fBeginLine( ) << "for( int ithLight = " << startLight << "; ithLight < " << numLights << "; ++ithLight )" << std::endl;
			writer.fBeginLine( ) << "{";
			writer.fPushTab( );
		}
		else
		{
			writer.fBeginLine( ) << "{" << std::endl;
			writer.fPushTab( );
			writer.fBeginLine( ) << "const int ithLight = " << startLight << ";" << std::endl;
		}

// until we get shadows on multiple lights, we continue to use the shadow term from the first light
//if( shadowTerm )
//{
//	writer.fBeginLine( true ) << "// only shadow the first light" << std::endl;
//	writer.fBeginLine( ) << shadowTerm->fName( ) << " = min( 1.0, " << shadowTerm->fName( ) << " + ithLight );" << std::endl;
//}

		tHlslVariableConstPtr lightParams = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalLightParams, writer.fInput( ).mPid );
		tHlslVariableConstPtr lightPos = lightParams->fMember( tPixelShaderVars::cLightParamPosition );
		tHlslVariableConstPtr lightDirRaw = lightParams->fMember( tPixelShaderVars::cLightParamDirection );
		tHlslVariableConstPtr lightAtten = lightParams->fMember( tPixelShaderVars::cLightParamAttenuation );
		tHlslVariableConstPtr lightAngles = lightParams->fMember( tPixelShaderVars::cLightParamAngles );
		tHlslVariableConstPtr lightAmbient = lightParams->fMember( tPixelShaderVars::cLightParamColorAmbient );
		tHlslVariableConstPtr lightFront = lightParams->fMember( tPixelShaderVars::cLightParamColorFront );
		tHlslVariableConstPtr lightSurround = lightParams->fMember( tPixelShaderVars::cLightParamColorSurround );
		tHlslVariableConstPtr lightBack = lightParams->fMember( tPixelShaderVars::cLightParamColorBack );

		const std::string lightPosName = lightParams->fIndexArrayAndSwizzle( "ithLight", lightPos->fName( ) );
		const std::string lightDirName = lightParams->fIndexArrayAndSwizzle( "ithLight", lightDirRaw->fName( ) );
		const std::string lightAttenName = lightParams->fIndexArrayAndSwizzle( "ithLight", lightAtten->fName( ) );
		const std::string lightAnglesName = lightParams->fIndexArrayAndSwizzle( "ithLight", lightAngles->fName( ) );
		const std::string lightAmbientName = lightParams->fIndexArrayAndSwizzle( "ithLight", lightAmbient->fName( ) );
		const std::string lightFrontName = lightParams->fIndexArrayAndSwizzle( "ithLight", lightFront->fName( ) );
		const std::string lightSurroundName = lightParams->fIndexArrayAndSwizzle( "ithLight", lightSurround->fName( ) );
		const std::string lightBackName = lightParams->fIndexArrayAndSwizzle( "ithLight", lightBack->fName( ) );

		tHlslVariableConstPtr rimLightParams = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalRimLight, writer.fInput( ).mPid );
		const std::string rimLightDirText = rimLightParams->fSwizzle( rimLightParams->fMember( 0 )->fName( ), ".xyz" );
		const std::string rimLightColorText = rimLightParams->fSwizzle( rimLightParams->fMember( 1 )->fName( ), ".xyz" );

		writer.fBeginLine( true ) << "// compute assorted light-related values" << std::endl;

		tHlslVariableConstPtr frontColor = writer.fMakeTempVector( "frontColor", 3 );
		writer.fBeginLine( ) << frontColor->fDeclaration( ) << " = " << lightFrontName << ".rgb";
		if( shadowTerm ) writer.fContinueLine( ) << " * " << shadowTerm->fSwizzle( 1 );
		writer.fContinueLine( ) << ";" << std::endl;

		tHlslVariableConstPtr surroundColor, backColor;
		if( !pointLights )
		{
			surroundColor = writer.fMakeTempVector( "surroundColor", 3 );
			writer.fBeginLine( ) << surroundColor->fDeclaration( ) << " = " << lightSurroundName << ".rgb";
			if( shadowTerm ) writer.fContinueLine( ) << " * " << shadowTerm->fSwizzle( 1 );
			writer.fContinueLine( ) << ";" << std::endl;

			backColor = writer.fMakeTempVector( "backColor", 3 );
			writer.fBeginLine( ) << backColor->fDeclaration( ) << " = " << lightBackName << ".rgb" << ";" << std::endl;
		}

		tHlslVariableConstPtr lightToVertex, lightDist;
		if( pointLights )
		{
			lightToVertex = writer.fMakeTempVector( "lightToVertex", 3 );
			writer.fBeginLine( ) << lightToVertex->fDeclaration( )
				<< " = " << lightPosName << ".xyz - " << litSpacePos->fSwizzle( "xyz" ) << ";" << std::endl;

			lightDist = writer.fMakeTempVector( "lightDist", 1 );
			writer.fBeginLine( ) << lightDist->fDeclaration( )
				<< " = length( " << lightToVertex->fName( ) << " );" << std::endl;
		}

		tHlslVariableConstPtr lightDir = writer.fMakeTempVector( "lightDir", 3 );
		if( pointLights )
			writer.fBeginLine( ) << lightDir->fDeclaration( ) << " = " << "( " << lightToVertex->fName( ) << " / " << lightDist->fName( ) << " );" << std::endl;
		else
			writer.fBeginLine( ) << lightDir->fDeclaration( ) << " = " << lightDirName << ".xyz;" << std::endl;

		tHlslVariableConstPtr nDotL = writer.fMakeTempVector( "nDotL", 1 );
		writer.fBeginLine( ) << nDotL->fDeclaration( )
			<< " = dot( " << effectiveNormal->fSwizzle( "xyz" ) << ", " << lightDir->fSwizzle( "xyz" ) << " );" << std::endl;

		tHlslVariableConstPtr litResult = writer.fMakeTempVector( "litResult", 3 );
		writer.fBeginLine( ) << litResult->fDeclaration( )
			<< " = float3( 1.f, " << nDotL->fName( ) << ", 0.f );" << std::endl;

		if( specAccum )
		{
			tHlslVariableConstPtr halfVector = writer.fMakeTempVector( "halfVector", 3 );
			writer.fBeginLine( true ) << "// compute half-way vector and normal-dot-half" << std::endl;
			writer.fBeginLine( ) << halfVector->fDeclaration( )
				<< " = normalize( -" << eyeToVertex->fSwizzle( 3 ) << " + " << lightDir->fSwizzle( "xyz" ) << " );" << std::endl;

			tHlslVariableConstPtr nDotH = writer.fMakeTempVector( "nDotH", 1 );
			writer.fBeginLine( ) << nDotH->fDeclaration( )
				<< " = dot( " << effectiveNormal->fSwizzle( "xyz" ) << ", " << halfVector->fSwizzle( "xyz" ) << " );" << std::endl;

			tHlslVariableConstPtr specPower = writer.fMakeTempVector( "specPower", 1 );
			writer.fBeginLine( true ) << "// scale spec falloff from [0,1] to actual mathematically-appropriate range" << std::endl;
			writer.fBeginLine( ) << specPower->fDeclaration( )
				<< " = 1.0f + ( " << specFalloffResultText << " * " << specFalloffResultText << " ) * 250.f;" << std::endl;

			writer.fBeginLine( true ) << "// compute spec term" << std::endl;
			if( pointLights )
			{
				writer.fBeginLine( ) << litResult->fSwizzle( "xyz" )
					<< " = lit( " << nDotL->fName( ) << ", " << nDotH->fName( ) << ", " << specPower->fName( ) << " );" << std::endl;
			}
			else
			{
				writer.fBeginLine( ) << litResult->fSwizzle( "z" )
					<< " = lit( " << nDotL->fName( ) << ", " << nDotH->fName( ) << ", " << specPower->fName( ) << " ).z;" << std::endl;
			}
		}

		tHlslVariableConstPtr atten;
		if( pointLights )
		{
			atten = writer.fMakeTempVector( "attenuation", 1 );
			writer.fBeginLine( true ) << "// compute and apply attenuation" << std::endl;
			writer.fBeginLine( ) << atten->fDeclaration( ) << " = 1.0 - " << std::endl;
			writer.fPushTab( );
			writer.fBeginLine( ) << "min( 1.0, max( 0.0, " << lightDist->fName( ) << " - " << lightAttenName << ".x ) / " << lightAttenName << ".y );" << std::endl;
			writer.fPopTab( );
			writer.fBeginLine( ) << atten->fName( )
				<< " = " << lightAttenName << ".z + " << lightAttenName << ".w * " << atten->fName( ) << ";" << std::endl;
			if( specAccum )
			{
				writer.fBeginLine( ) << litResult->fSwizzle( "z" )
					<< " *= " << atten->fName( ) << ";" << std::endl;
			}
		}

		writer.fBeginLine( true ) << "// finally, we accumulate our diffuse and/or spec terms" << std::endl;
		if( diffAccum )
		{
			if( pointLights )
				writer.fBeginLine( ) << diffAccum->fName( ) << " += " << std::endl;
			else
			{
				if( ambientResult )
				{
					if( ambientResult->fDimensionX( ) >= 3 )
						writer.fBeginLine( ) << diffAccum->fName( ) << " += " << lightAmbientName << ".rgb * " << ambientResult->fSwizzle( 3 ) << " + " << std::endl;
					else
						writer.fBeginLine( ) << diffAccum->fName( ) << " += " << lightAmbientName << ".rgb * " << ambientResult->fSwizzle( 1 ) << " + " << std::endl;
				}
				else
					writer.fBeginLine( ) << diffAccum->fName( ) << " += " << lightAmbientName << ".rgb + " << std::endl;
			}

			writer.fPushTab( );
			if( pointLights )
				writer.fBeginLine( ) << atten->fName( ) << " * " << std::endl;
			writer.fBeginLine( ) << "(" << std::endl;
			writer.fPushTab( );
			if( pointLights )
				writer.fBeginLine( ) << "  (" << frontColor->fName( ) << " * max(0.0," << litResult->fSwizzle( "y" ) << "))" << std::endl;
			else
			{
				writer.fBeginLine( ) << "  (" << frontColor->fName( ) << " * max(0.0," << litResult->fSwizzle( "y" ) << "))" << std::endl;
				writer.fBeginLine( ) << "+ (" << surroundColor->fName( ) << " * (1.0-abs(" << litResult->fSwizzle( "y" ) << ")))" << std::endl;
				writer.fBeginLine( ) << "+ (" << backColor->fName( ) << " * max(0.0, -" << litResult->fSwizzle( "y" ) << "))" << std::endl;
			}
			writer.fPopTab( );
			writer.fBeginLine( ) << ");" << std::endl;
			writer.fPopTab( );
		}
		if( rimAccum && !pointLights ) // only compute once for the directional light
		{
			tHlslVariableConstPtr rimFactor = writer.fMakeTempVector( "rimAtten", 1 );
			writer.fBeginLine( true ) << "// compute amount of rim light to apply based on how much the vertex is between the eye and the light" << std::endl;
			writer.fBeginLine( ) << rimFactor->fDeclaration( )
				<< " = max( 0.5, dot( " << eyeToVertex->fSwizzle( 3 ) << ", " << rimLightDirText << " ) );" << std::endl;

			writer.fBeginLine( ) << rimAccum->fName( ) << " += " << rimLightColorText << " * "  << rimFactor->fName( ) << " * pow(1-abs( dot(" << rimLightDirText << ", " << effectiveNormal->fSwizzle( "xyz" ) << " )),2);" << std::endl;
		}
		if( transAccum )
		{
			writer.fBeginLine( true ) << "// accumulate transmission" << std::endl;
			writer.fBeginLine( ) << transAccum->fName( ) << " += " << std::endl;
			writer.fPushTab( );
			if( pointLights )
				writer.fBeginLine( ) << atten->fName( ) << " * " << std::endl;
			writer.fBeginLine( ) << "(" << std::endl;
			writer.fPushTab( );
			if( pointLights )
				writer.fBeginLine( ) << "(" << frontColor->fName( ) << " * -" << litResult->fSwizzle( "y" ) << ")" << std::endl;
			else
			{
				if( !diffAccum ) // only include rim light if no diffuse, otherwise it would get included twice
					writer.fBeginLine( ) << "+ (" << surroundColor->fName( ) << " * (1.0-abs(" << litResult->fSwizzle( "y" ) << ")))" << std::endl;
				writer.fBeginLine( ) << "+ (" << backColor->fName( ) << " * max(0.0, +" << litResult->fSwizzle( "y" ) << "))" << std::endl;
			}
			writer.fPopTab( );
			writer.fBeginLine( ) << ");" << std::endl;
			writer.fPopTab( );
		}
		if( specAccum )
		{
			writer.fBeginLine( true ) << "// accumulate spec" << std::endl;
			writer.fBeginLine( ) << specAccum->fName( )
				<< " += " << frontColor->fName( ) << " * " << litResult->fSwizzle( "z" ) << ";" << std::endl;
			writer.fBeginLine( ) << specMagAccum->fName( )
				<< " += " << litResult->fSwizzle( "z" ) << ";" << std::endl;
		}

		writer.fPopTab( );
		writer.fBeginLine( ) << "}" << std::endl;
	}

}
