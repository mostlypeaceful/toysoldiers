#include "ToolsPch.hpp"
#include "DerivedShadeNodes.hpp"
#include "tTextureSysRam.hpp"
#include "Tatml.hpp"

#include "Editor/tEditablePropertyTypes.hpp"
#include "Editor/tEditablePropertyColor.hpp"

#include "HlslGen/tPixelShaderVars.hpp"
#include "HlslGen/tHlslGenTree.hpp"
#include "HlslGen/tHlslWriter.hpp"

#include "tDeferredShadingMaterialGen.hpp"
#include "Gfx/tTransitionObjectMaterialHelper.hpp"

namespace Sig
{

	namespace
	{
		const wxColour cFinalOutputBarColor( 0x00, 0x00, 0x00 );
		const wxColour cFinalOutputTextColor( 0xff, 0xff, 0xff );
		const char cPropNameFog[]="Shader Properties.Fog";
		const char cPropNameTint[]="Shader Properties.Tint";
	}

	register_rtti_factory( tDefaultOutputShadeNode, false );
	tDefaultOutputShadeNode::tDefaultOutputShadeNode( const wxPoint& p )
		: tShadeNode( "DefaultOutput", cFinalOutputBarColor, cFinalOutputTextColor, false, p )
	{
		fAddInput( "Color0" );
		fAddInput( "TintControl" );
		fComputeDimensions( );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cPropNameTint, true ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cPropNameFog, true ) ) );
	}
	void tDefaultOutputShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		reqs.mOutputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cOutColor0, writer.fInput( ).mPid ) );

		b32 deferredShading = writer.fDeferredShading( );
		if( deferredShading )
		{
			reqs.mOutputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cOutColor1, writer.fInput( ).mPid ) );
			reqs.mOutputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cOutColor2, writer.fInput( ).mPid ) );
			reqs.mOutputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cOutColor3, writer.fInput( ).mPid ) );
			reqs.mCommon.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cTempEmissionResult, writer.fInput( ).mPid ) );
			reqs.mCommon.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cTempAmbientResult, writer.fInput( ).mPid ) );
		}

		if( writer.fWriteMode( ) == cWriteModeColor )
		{
			// add fog requirements if specified
			const b32 tint = fShadeProps( ).fGetValue( cPropNameTint, true );
			//const b32 fog = fShadeProps( ).fGetValue( cPropNameFog, true );
			if( tint )
				reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalInstanceTint, writer.fInput( ).mPid ) );
			
			// fog is always applied as a post processor step now.
			//if( fog )
			//{
			//	reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalFogParams, writer.fInput( ).mPid ) );
			//	reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalFogColor, writer.fInput( ).mPid ) );
			//	reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, writer.fInput( ).mPid ) );
			//	reqs.mCommon.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexLen, writer.fInput( ).mPid ) );
			//}

			if( deferredShading )
				reqs.mCommon.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cTempEffectiveNormal, writer.fInput( ).mPid ) );
		}
		else
		{
			reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInProjSpaceDepth, writer.fInput( ).mPid ) );
		}
	}
	void tDefaultOutputShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		b32 deferredShader = writer.fDeferredShading( );
		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		tHlslVariableConstPtr output = tPixelShaderVars::fVariable( tPixelShaderVars::cOutColor0, writer.fInput( ).mPid );
		hlslGenTree.fCacheShadeNodeOutput( output );

		if( writer.fDualParaboloid( ) )
		{
			writer.fBeginLine( true ) << "// Clipping for dual paraboloid shadows." << std::endl;
			tHlslVariableConstPtr projSpaceDepth = tPixelShaderVars::fVariable( tPixelShaderVars::cInProjSpaceDepth, writer.fInput( ).mPid );
			writer.fBeginLine( ) << "clip( " << projSpaceDepth->fSwizzle( "z" ) << ");" << std::endl;
		}

		if( writer.fWriteMode( ) == cWriteModeColor )
		{
			const b32 tint = fShadeProps( ).fGetValue( cPropNameTint, true );
			//const b32 fog = fShadeProps( ).fGetValue( cPropNameFog, true );

			tHlslVariableConstPtr instanceTint = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalInstanceTint, writer.fInput( ).mPid );
			tHlslVariableConstPtr noTintBlend = writer.fMakeTempVector( "noTintBlend" );
			writer.fBeginLine( true ) << "// write color output 0" << std::endl;
			
			if( tint && inputResults[ 1 ] )
				writer.fBeginLine( ) << noTintBlend->fDeclaration( ) << " = float4( 1, 1, 1, 1 );" << std::endl;

			if( inputResults[ 0 ] )
			{
				writer.fBeginLine( ) << output->fName( ) << " = " << inputResults[ 0 ]->fSwizzle( *output );
				
				if( tint )
				{
					writer.fContinueLine( ) << " * ";
					
					if( inputResults[ 1 ] )
						 writer.fContinueLine( ) << "lerp( " << noTintBlend->fSwizzle( *output ) << ", " ;

					writer.fContinueLine( ) << instanceTint->fSwizzle( *output );

					if( inputResults[ 1 ] )
						writer.fContinueLine( ) << ", " << inputResults[ 1 ]->fSwizzle( *output ) << " )";
				}
				writer.fContinueLine( ) << ";" << std::endl;
			}
			else
			{
				if( tint )
				{
					writer.fBeginLine( ) << output->fName( ) << " = ";

					if( inputResults[ 1 ] )
						writer.fContinueLine( ) << "lerp( " << noTintBlend->fSwizzle( *output ) << ", ";

					writer.fContinueLine( ) << instanceTint->fSwizzle( *output );

					if( inputResults[ 1 ] )
						writer.fContinueLine( ) << ", " << inputResults[ 1 ]->fSwizzle( *output ) << " )";

					writer.fContinueLine( ) << ";" << std::endl;
				}
				else
					writer.fBeginLine( ) << output->fName( ) << " = " << output->fCastValueToType( 1.f ) << ";" << std::endl;
			}

			// fog is always applied as a post processor step now.
			//// apply fog if specified
			//if( fog && !deferredShader )
			//{
			//	//writer.fBeginLine( true ) << "// Fog is now applied through the 'Depth of field' post effect, for commonality between deferred and forward shading. -Matt K" << std::endl;
			//	//tHlslVariableConstPtr fogParams = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalFogParams, writer.fInput( ).mPid );
			//	//tHlslVariableConstPtr fogColor = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalFogColor, writer.fInput( ).mPid );
			//	//tHlslVariableConstPtr eyeToVertexLen = tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexLen, writer.fInput( ).mPid );
			//	//tHlslVariableConstPtr fogginess = writer.fMakeTempVector( "fogginess", 1 );

			//	//writer.fBeginLine( true ) << "// compute and apply fog" << std::endl;
			//	//writer.fBeginLine( ) << fogginess->fDeclaration( )
			//	//	<< " = saturate( ( " << eyeToVertexLen->fName( ) << " - " << fogParams->fSwizzle( "x" ) << " ) / " << fogParams->fSwizzle( "y" ) << " );" << std::endl;
			//	//writer.fBeginLine( ) << fogginess->fName( )
			//	//	<< " = clamp( " << fogginess->fName( ) << " * " << fogginess->fName( ) << ", " << fogParams->fSwizzle( "z" ) << ", " << fogParams->fSwizzle( "w" ) << " );" << std::endl;
			//	//writer.fBeginLine( ) << output->fSwizzle( "rgb" )
			//	//	<< " = lerp( " << output->fSwizzle( "rgb" ) << ", " << fogColor->fSwizzle( "rgb" ) << ", " << fogginess->fName( ) << " );" << std::endl;
			//}

			if( deferredShader )
			{
				tDeferredShadingMaterialGen::fPackFunctions( writer.fFunctions( ) );

				tHlslVariableConstPtr normalOutput = tPixelShaderVars::fVariable( tPixelShaderVars::cOutColor1, writer.fInput( ).mPid );
				tHlslVariableConstPtr emissionOutput = tPixelShaderVars::fVariable( tPixelShaderVars::cOutColor2, writer.fInput( ).mPid );
				tHlslVariableConstPtr ambientOutput = tPixelShaderVars::fVariable( tPixelShaderVars::cOutColor3, writer.fInput( ).mPid );

				tHlslVariableConstPtr effectiveNormal = tPixelShaderVars::fVariable( tPixelShaderVars::cTempEffectiveNormal, writer.fInput( ).mPid );
				tHlslVariableConstPtr emissionResult = tPixelShaderVars::fVariable( tPixelShaderVars::cTempEmissionResult, writer.fInput( ).mPid );
				tHlslVariableConstPtr ambientResult = tPixelShaderVars::fVariable( tPixelShaderVars::cTempAmbientResult, writer.fInput( ).mPid );
				
				writer.fBeginLine( true ) << "// write normal output 1" << std::endl;
				writer.fBeginLine( ) << normalOutput->fName( ) << " = float4( fPackNormal( " << effectiveNormal->fSwizzle( "xyz" ) << " ), " << effectiveNormal->fSwizzle( "w" ) << " );" << std::endl;

				writer.fBeginLine( true ) << "// write emission output" << std::endl;
				writer.fBeginLine( ) << emissionOutput->fName( ) << " = " << emissionResult->fSwizzle( emissionOutput ) << ";" << std::endl;

				writer.fBeginLine( true ) << "// write ambient output" << std::endl;
				writer.fBeginLine( ) << ambientOutput->fName( ) << " = " << ambientResult->fSwizzle( ambientOutput ) << ";" << std::endl;
			}
		}
		else
		{
			b32 handled = false;
			tHlslVariableConstPtr projSpaceDepth = tPixelShaderVars::fVariable( tPixelShaderVars::cInProjSpaceDepth, writer.fInput( ).mPid );
			writer.fBeginLine( true ) << "// write color output 0" << std::endl;

			if( writer.fWriteMode( ) == cWriteModeDepthWithAlpha )
			{
				// write alpha from input
				tHlslVariableConstPtr input = inputResults[ 0 ];
				if( input && input->fDimensionX( ) == 4 )
				{
					handled = true;

					writer.fBeginLine( ) << output->fSwizzle( "rgb" ) << " = " << projSpaceDepth->fSwizzle( "x" ) << " / " << projSpaceDepth->fSwizzle( "y" ) << ";" << std::endl;
					writer.fBeginLine( ) << output->fSwizzle( "a" ) << " = " << input->fSwizzle( "a" ) << ";" << std::endl;
				}
			}

			if( !handled )
			{
				writer.fBeginLine( ) << output->fName( ) << " = " << projSpaceDepth->fSwizzle( "x" ) << " / " << projSpaceDepth->fSwizzle( "y" ) << ";" << std::endl;
			}
		}
	}



	namespace
	{
		const wxColour cLightingModelsBarColor( 0xdd, 0x44, 0x22 );
		const wxColour cLightingModelsTextColor( 0xee, 0xee, 0xaa );
	}

	register_rtti_factory( tFlatParticleShadeNode, false );
	tFlatParticleShadeNode::tFlatParticleShadeNode( const wxPoint& p )
		: tShadeNode( "FlatParticle", cLightingModelsBarColor, cLightingModelsTextColor, false, p )
	{
		fAddInput( "BaseColor" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tFlatParticleShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalFlatParticleColor, writer.fInput( ).mPid ) );

		tHlslVariableConstPtr req = tPixelShaderVars::fVariable( tPixelShaderVars::cInUv01, writer.fInput( ).mPid ); 
		reqs.mInputs.fFindOrAdd( req );
		hlslGenTree.fCacheShadeNodeReq( req );
	}
	void tFlatParticleShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		tHlslVariableConstPtr flatParticleColor = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalFlatParticleColor, writer.fInput( ).mPid );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "flatParticleColor", 4 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// multiply input particle color by global flat particle color (" << fMatEdDisplayName( ) << ")" << std::endl;


		if( inputResults.fCount( ) == 1 && inputResults[ 0 ] )
			writer.fBeginLine( ) << output->fDeclaration( ) << " = " << inputResults[ 0 ]->fSwizzle( 4 ) << " * " << flatParticleColor->fName( ) << ";" << std::endl;
		else
			writer.fBeginLine( ) << output->fDeclaration( ) << " = " << flatParticleColor->fName( ) << ";" << std::endl;

		writer.fBeginLine( ) << output->fSwizzle( "w" ) << " = step( " << hlslGenTree.fShadeNodeReq( )->fSwizzle( "z" ) << ", " << output->fSwizzle( "w" ) << " ) * " << output->fSwizzle( "w" ) << ";" << std::endl;

	}

	namespace
	{
		const wxColour cTextureMappingBarColor( 0xff, 0xff, 0x22 );
		const wxColour cTextureMappingTextColor( 0x11, 0x11, 0x44 );
		const char cPropNameChannelIndex[]  = "Shader Properties.Channel";
		const char cPropNameTexturePath[]   = ".TexturePath";
		const char cPropNameTextureSource[] = ".TextureSource";
		const char cPropNameTexAtlasPath[]   = ".TexAtlasPath";
		const char cPropNameAtlasTexCount[] = "Shader Properties.TexCount";

		static std::string fGetTextureMaterialPropertyValue( tShadeNode& src )
		{
			std::string value;
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( cPropNameTexturePath );
			if( existingValueProp )
				existingValueProp->fGetData( value );
			return value;
		}

		static std::string fGetTexAtlasMaterialPropertyValue( tShadeNode& src )
		{
			std::string value;
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( cPropNameTexAtlasPath );
			if( existingValueProp )
				existingValueProp->fGetData( value );
			return value;
		}

		static u32 fGetTextureSourceMaterialPropertyValue( tShadeNode& src )
		{
			u32 source = 0;
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( cPropNameTextureSource );
			if( existingValueProp )
				existingValueProp->fGetData( source );
			return source;
		}

		static void fUpdateTextureMaterialGlueValues( tShadeNode& src, b32 cubeMap, Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
		{
			const Gfx::tShadeMaterialGlueValues::tTextureSource texSource = ( Gfx::tShadeMaterialGlueValues::tTextureSource )fGetTextureSourceMaterialPropertyValue( src );
			const tFilePathPtr srcFileName = tFilePathPtr( fGetTextureMaterialPropertyValue( src ) );
			
			Dx9Util::tBaseTexturePtr texPtr;
			switch( texSource ) 
			{
				case Gfx::tShadeMaterialGlueValues::cTexSourceFromFile: texPtr = cubeMap ? textureCache.fFindLoadCube( srcFileName ) : textureCache.fFindLoad2D( srcFileName ); break;
				case Gfx::tShadeMaterialGlueValues::cTexSourceWhite: texPtr = textureCache.fWhiteTexture( ); break;
				case Gfx::tShadeMaterialGlueValues::cTexSourceBlack: texPtr = textureCache.fBlackTexture( ); break;
				case Gfx::tShadeMaterialGlueValues::cTexSourceNoise: texPtr = textureCache.fWhiteTexture( ); break; // TODO noise 
				case Gfx::tShadeMaterialGlueValues::cTexSourceBlankNormals: texPtr = textureCache.fBlankNormalsTexture( ); break;
			}
			
			// fall backs.
			if( !texPtr ) 
			{
				if( dynamic_cast<tNormalMapShadeNode*>( &src ) )
					texPtr = textureCache.fBlankNormalsTexture( );
				else
					texPtr = textureCache.fWhiteTexture( );
			}

			Gfx::tTextureReference tr;
			tr.fSetRaw( ( Gfx::tTextureReference::tPlatformHandle )texPtr.fGetRawPtr( ) );
			// TODO get sampling modes from user
			tr.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, cubeMap ? Gfx::tTextureFile::cAddressModeClamp : Gfx::tTextureFile::cAddressModeWrap );
			glueVals.fUpdateSampler( tr, src.fMaterialGlueIndex( ) );
		}

		static void fAcquireTextureMaterialGlueValuesForAssetGen( tShadeNode& src, b32 cubeMap, Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
		{
			Gfx::tShadeMaterialGlueValues::tTextureSource texSource = ( Gfx::tShadeMaterialGlueValues::tTextureSource )fGetTextureSourceMaterialPropertyValue( src );
			const tFilePathPtr srcFileName = tFilePathPtr( fGetTextureMaterialPropertyValue( src ) );

			if( srcFileName.fLength( ) == 0 )
				texSource = Gfx::tShadeMaterialGlueValues::cTexSourceWhite;

			switch( texSource )
			{
			case Gfx::tShadeMaterialGlueValues::cTexSourceFromFile:
				{
					resourcePathsOut.fFindOrAdd( srcFileName );
					tLoadInPlaceResourcePtr* lip = lipFileCreator.fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tTextureFile>( tTextureSysRam::fCreateBinaryPath( srcFileName ) ) );
					Gfx::tTextureReference tr;
					tr.fSetLoadInPlace( lip );
					// TODO get sampling modes from user
					tr.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, cubeMap ? Gfx::tTextureFile::cAddressModeClamp : Gfx::tTextureFile::cAddressModeWrap );
					glueVals.fUpdateSampler( tr, src.fMaterialGlueIndex( ) );
				}
				break;
			case Gfx::tShadeMaterialGlueValues::cTexSourceWhite: glueVals.fUpdateSampler( Gfx::tShadeMaterialGlueValues::cTexSourceWhite, src.fMaterialGlueIndex( ) ); break;
			case Gfx::tShadeMaterialGlueValues::cTexSourceBlack: glueVals.fUpdateSampler( Gfx::tShadeMaterialGlueValues::cTexSourceBlack, src.fMaterialGlueIndex( ) ); break;
			case Gfx::tShadeMaterialGlueValues::cTexSourceNoise: glueVals.fUpdateSampler( Gfx::tShadeMaterialGlueValues::cTexSourceNoise, src.fMaterialGlueIndex( ) ); break;
			case Gfx::tShadeMaterialGlueValues::cTexSourceBlankNormals: glueVals.fUpdateSampler( Gfx::tShadeMaterialGlueValues::cTexSourceBlankNormals, src.fMaterialGlueIndex( ) ); break;
				
			}
		}

		static b32 fRefreshTextureMaterialProperties( tShadeNode* shadeNode, tShadeNode& src, u32 index )
		{
			const std::string title = shadeNode->fMatEdDisplayName( index );

			tEditablePropertyPtr existingValueProp;

			std::string value = shadeNode->fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesDefault, std::string( ) );

			existingValueProp = src.fMatProps( ).fFindPartial( cPropNameTexturePath );
			if( existingValueProp )
				existingValueProp->fGetData( value );

			tDynamicArray<std::string> choices(Gfx::tShadeMaterialGlueValues::cTexSourceCount);
			choices[Gfx::tShadeMaterialGlueValues::cTexSourceFromFile] = "From File";
			choices[Gfx::tShadeMaterialGlueValues::cTexSourceWhite]	= "White";
			choices[Gfx::tShadeMaterialGlueValues::cTexSourceBlack]	= "Black";
			choices[Gfx::tShadeMaterialGlueValues::cTexSourceNoise]	= "Noise";
			choices[Gfx::tShadeMaterialGlueValues::cTexSourceBlankNormals] = "Blank Normals";
			u32 choice = 0;
			existingValueProp = src.fMatProps( ).fFindPartial( cPropNameTextureSource );
			if( existingValueProp )
				existingValueProp->fGetData( choice );

			shadeNode->fMatProps( ).fClear( 32 );
			shadeNode->fMatProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( title + cPropNameTexturePath, value ) ) );
			shadeNode->fMatProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( title + cPropNameTextureSource, choices, choice ) ) );
			return true;
		}

		static void fUpdateTexturePathBrowseFilters( const std::string& name )
		{
			const std::string realName = name + cPropNameTexturePath;
			tEditablePropertyFileNameString::fAddFilter( realName, "*.tga" );
			tEditablePropertyFileNameString::fAddFilter( realName, "*.png" );
			tEditablePropertyFileNameString::fAddFilter( realName, "*.bmp" );
			tEditablePropertyFileNameString::fAddFilter( realName, "*.jpg" );
			tEditablePropertyFileNameString::fAddFilter( realName, "*.dds" );
			tEditablePropertyFileNameString::fAddFilter( realName, "*.tatml" );
		}
	}

	register_rtti_factory( tUvChannelShadeNode, false );
	tUvChannelShadeNode::tUvChannelShadeNode( const wxPoint& p )
		: tShadeNode( "UvChannel", cTextureMappingBarColor, cTextureMappingTextColor, false, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cPropNameChannelIndex, 0.f, 0.f, 3.f, 1.f, 0 ) ) );
	}
	void tUvChannelShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		const b32 facingQuads = ( writer.fInput( ).mVshStyle == cVshFacingQuads );
		const u32 uvChannel = fUvChannel( facingQuads ? 0 : 3 );

		tHlslVariableConstPtr req;

		if( facingQuads )
			req = tPixelShaderVars::fVariable( tPixelShaderVars::cInUv01, writer.fInput( ).mPid );
		else
			req = tPixelShaderVars::fVariable( ( tPixelShaderVars::tVarId )( tPixelShaderVars::cInUv0 + uvChannel ), writer.fInput( ).mPid );

		//if( uvChannel <= 1 )
		//	req = tPixelShaderVars::fVariable( tPixelShaderVars::cInUv01, writer.fInput( ).mPid );
		//else if( uvChannel <= 3 )
		//	req = tPixelShaderVars::fVariable( tPixelShaderVars::cInUv23, writer.fInput( ).mPid );

		sigassert( req );
		reqs.mInputs.fFindOrAdd( req );
		hlslGenTree.fCacheShadeNodeReq( req );
	}
	void tUvChannelShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		const u32 uvChannel = fUvChannel( ( writer.fInput( ).mVshStyle == cVshFacingQuads ) ? 0 : 3 );

		std::stringstream ss;
		ss << "tempUv" << uvChannel;
		tHlslVariableConstPtr output = writer.fMakeTempVector( ss.str( ), 2 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// swizzle and store uv value from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = " << hlslGenTree.fShadeNodeReq( )->fSwizzle( /*( uvChannel & 1 ) ? "zw" :*/ "xy" ) << ";" << std::endl;
	}
	u32 tUvChannelShadeNode::fUvChannel( u32 maxChannel )
	{
		const f32 value = fShadeProps( ).fGetValue( cPropNameChannelIndex, 0.f );
		return fClamp( fRound<u32>( value ), 0u, maxChannel );
	}

	//------------------------------------------------------------------------------
	// tColorMapShadeNode
	//------------------------------------------------------------------------------
	register_rtti_factory( tColorMapShadeNode, false );
	tColorMapShadeNode::tColorMapShadeNode( const wxPoint& p )
		: tShadeNode( "ColorMap", cTextureMappingBarColor, cTextureMappingTextColor, true, p )
	{
		fAddInput( "Uv" );
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( cNamePropertiesDefault ) ) );
	}
	b32 tColorMapShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		fUpdateTexturePathBrowseFilters( fMatEdDisplayName( index ) );
		return fRefreshTextureMaterialProperties( this, src, index );
	}
	void tColorMapShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		tHlslVariableConstPtr sampler( tHlslVariable::fMakeSampler2D( "gColorMap", reqs.mNextSamplerRegister++ ) );
		hlslGenTree.fCacheShadeNodeReq( sampler );
		reqs.mGlobals.fFindOrAdd( sampler );

		if( !hlslGenTree.fInputs( )[ 0 ] ) // add default uv input if none was specified
			hlslGenTree.fForceSetInput( 0, tHlslGenTreePtr( new tHlslGenTree( tShadeNodePtr( new tUvChannelShadeNode( ) ), &hlslGenTree ) ) );
	}
	void tColorMapShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );
		sigassert( inputResults[ 0 ] );

		fSetMaterialGlueIndex( writer.fAddMaterialGlueSampler( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "colorMap", 4 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// sample color map from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = tex2D( " << hlslGenTree.fShadeNodeReq( )->fName( ) << ", " << inputResults[ 0 ]->fSwizzle( 2 ) << " );" << std::endl;
	}
	void tColorMapShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		fUpdateTextureMaterialGlueValues( *this, false, glueVals, textureCache );
	}
	void tColorMapShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		fAcquireTextureMaterialGlueValuesForAssetGen( *this, false, glueVals, lipFileCreator, resourcePathsOut );
	}

	//------------------------------------------------------------------------------
	// tNormalMapShadeNode
	//------------------------------------------------------------------------------
	register_rtti_factory( tNormalMapShadeNode, false );
	tNormalMapShadeNode::tNormalMapShadeNode( const wxPoint& p )
		: tShadeNode( "NormalMap", cTextureMappingBarColor, cTextureMappingTextColor, true, p )
	{
		fAddInput( "Uv" );
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( cNamePropertiesDefault ) ) );
	}
	b32 tNormalMapShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		fUpdateTexturePathBrowseFilters( fMatEdDisplayName( index ) );
		return fRefreshTextureMaterialProperties( this, src, index );
	}
	void tNormalMapShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		tHlslVariableConstPtr sampler( tHlslVariable::fMakeSampler2D( "gNormalMap", reqs.mNextSamplerRegister++ ) );
		hlslGenTree.fCacheShadeNodeReq( sampler );
		reqs.mGlobals.fFindOrAdd( sampler );

		if( !hlslGenTree.fInputs( )[ 0 ] ) // add default uv input if none was specified
			hlslGenTree.fForceSetInput( 0, tHlslGenTreePtr( new tHlslGenTree( tShadeNodePtr( new tUvChannelShadeNode( ) ), &hlslGenTree ) ) );
	}
	void tNormalMapShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );
		sigassert( inputResults[ 0 ] );

		fSetMaterialGlueIndex( writer.fAddMaterialGlueSampler( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "tangSpaceNorm", 3 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// sample normal map from " << fMatEdDisplayName( ) << std::endl;

		const b32 dxtn5 = writer.fInput( ).mDXT5NNormalMaps && !writer.fInput( ).mPreviewMode;

		if( dxtn5 )
		{
			writer.fBeginLine( ) << "// we assume DXT5 using DXTN compression, where x is in alpha, z is in green, and we reconstruct y." << std::endl;
			writer.fBeginLine( ) << "// tangent space is different from what you might expect, we swap tangent space y <=> z bcz we're in 'maya' space, or open-gl space." << std::endl;
			writer.fBeginLine( ) << output->fDeclaration( ) << " = tex2D( " << hlslGenTree.fShadeNodeReq( )->fName( ) << ", " << inputResults[ 0 ]->fSwizzle( 2 ) << " ).gaa * 2 - 1;" << std::endl;

			const std::string green = output->fSwizzle( "x" );
			const std::string alpha = output->fSwizzle( "y" );
			writer.fBeginLine( ) << output->fName( ) << " = float3( " << std::endl;
			writer.fPushTab( );
			writer.fBeginLine( ) << alpha << ", " << std::endl;
			writer.fBeginLine( ) << "sqrt( 1.f - " << alpha << " * " << alpha << " - " << green << " * " << green << " ), " << std::endl;
			writer.fBeginLine( ) << green << " );" << std::endl;
			writer.fPopTab( );
		}
		else
		{
			writer.fBeginLine( ) << output->fDeclaration( ) << " = tex2D( " << hlslGenTree.fShadeNodeReq( )->fName( ) << ", " << inputResults[ 0 ]->fSwizzle( 2 ) << " ).xzy * 2 - 1;" << std::endl;
		}
	}
	void tNormalMapShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		fUpdateTextureMaterialGlueValues( *this, false, glueVals, textureCache );
	}
	void tNormalMapShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		fAcquireTextureMaterialGlueValuesForAssetGen( *this, false, glueVals, lipFileCreator, resourcePathsOut );
	}

	//------------------------------------------------------------------------------
	// tColorMapAtlasShadeNode
	//------------------------------------------------------------------------------
	register_rtti_factory( tColorMapAtlasShadeNode, false );
	tColorMapAtlasShadeNode::tColorMapAtlasShadeNode( const wxPoint& p )
		: tShadeNode( "ColorMapAtlas", cTextureMappingBarColor, cTextureMappingTextColor, true, p )
	{
		fAddInput( "Uv" );
		fAddInput( "Index" );
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( cNamePropertiesDefault ) ) );
	}
	b32 tColorMapAtlasShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		const std::string title = fMatEdDisplayName( index );
		const std::string textureName = title + cPropNameTexAtlasPath;
		tEditablePropertyFileNameString::fAddFilter( textureName, "*.tatml" );

		// Update the material properties
		{
			std::string value = fShadeProps( ).fGetValue( cNamePropertiesDefault, std::string( ) );

			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( cPropNameTexAtlasPath );
			if( existingValueProp )
				existingValueProp->fGetData( value );

			fMatProps( ).fClear( 32 );
			fMatProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( textureName, value ) ) );
		}

		return true;
	}
	void tColorMapAtlasShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		// Sampler
		{
			tHlslVariableConstPtr sampler;
			if( writer.fInput( ).mPid == HlslGen::cPidXbox360 )
				sampler.fReset( tHlslVariable::fMakeSampler3D( "gColorMapAtlas", reqs.mNextSamplerRegister++ ) );
			else
				sampler.fReset( tHlslVariable::fMakeSampler2D( "gColorMapAtlas", reqs.mNextSamplerRegister++ ) );
			hlslGenTree.fCacheShadeNodeReq( sampler, 0 );
			reqs.mGlobals.fFindOrAdd( sampler );
		}

		// Tex atlas info
		{
			// num Textures X, num Textures Y, tile Dim X, tile Dim Y
			tHlslVariableConstPtr atlasInfo( tHlslVariable::fMakeGlobalVector( "gColorMapAtlasInfo", reqs.mNextConstantRegister++ ) );
			hlslGenTree.fCacheShadeNodeReq( atlasInfo, 1 );
			reqs.mGlobals.fFindOrAdd( atlasInfo );
		}

		if( !hlslGenTree.fInputs( )[ 0 ] ) // add default uv input if none was specified
			hlslGenTree.fForceSetInput( 0, tHlslGenTreePtr( new tHlslGenTree( tShadeNodePtr( new tUvChannelShadeNode( ) ), &hlslGenTree ) ) );
		if( !hlslGenTree.fInputs( )[ 1 ] ) // add default index input if none was specified
			hlslGenTree.fForceSetInput( 1, tHlslGenTreePtr( new tHlslGenTree( tShadeNodePtr( new tNumberShadeNode( ) ), &hlslGenTree ) ) );

	}
	void tColorMapAtlasShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );
		sigassert( inputResults[ 0 ] && inputResults[ 1 ] );

		const tHlslVariableConstPtr & uvResult = inputResults[ 0 ];
		const tHlslVariableConstPtr & indexResult = inputResults[ 1 ];

		const tHlslVariableConstPtr& sampler = hlslGenTree.fShadeNodeReq( 0 );
		const tHlslVariableConstPtr& atlasInfo = hlslGenTree.fShadeNodeReq( 1 );

		// Material glue setup
		{
			fSetMaterialGlueIndex( 
				writer.fAddMaterialGlueAtlasSampler( 
					sampler->fSemantic( ).mRegisterIndex, 
					atlasInfo->fSemantic( ).mRegisterIndex,
					fMaterialGlueIndex( ) ) );
		}

		tHlslVariableConstPtr output = writer.fMakeTempVector( "colorMapAtlas", 4 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// sample color map atlas from " << fMatEdDisplayName( ) << std::endl;
		if( writer.fInput( ).mPid == HlslGen::cPidXbox360 )
		{	
			tHlslVariableConstPtr layerCount = writer.fMakeTempVector( "layerCount", 1 );
			tHlslVariableConstPtr sliceNorm = writer.fMakeTempVector( "sliceNorm", 1 );

			writer.fBeginLine( ) << layerCount->fDeclaration( ) << " = " << atlasInfo->fName( ) << ".x * " << atlasInfo->fName( ) << ".y;" << std::endl;
			writer.fBeginLine( ) << sliceNorm->fDeclaration( ) 
				<< " = 1.0 / ( 2.0 * " << layerCount->fName( ) << " ) + " 
				<< "( " << indexResult->fSwizzle( 1 ) << " / " << layerCount->fName( ) << " );" << std::endl;

			writer.fBeginLine( ) 
				<< output->fDeclaration( ) 
				<< " = tex3D( " << sampler->fName( ) << ", float3( " << uvResult->fSwizzle( 2 ) << ", " << sliceNorm->fName( ) << " ) );" << std::endl;

			/*
				float layerCount = gColorMapAtlasInfo.x * gColorMapAtlasInfo.y;
				float sliceNorm = 1.0 / ( 2.0 * layerCount ) + ( index / layerCount );
				float4 colorMapAtlas = tex3d( gColorMapAtlas, float3( uv, sliceNorm ) );
			*/
		}
		else
		{
			tHlslVariableConstPtr texAtlasDims = writer.fMakeTempVector( "texAtlasDims", 2 );
			tHlslVariableConstPtr tileTexelDims = writer.fMakeTempVector( "tileTexelDims", 2 );
			tHlslVariableConstPtr invTexAtlasDims = writer.fMakeTempVector( "invTexAtlasDims", 2 );
			tHlslVariableConstPtr invTexAtlasTexelDims = writer.fMakeTempVector( "invTexAtlasTexelDims", 2 );
			tHlslVariableConstPtr dx = writer.fMakeTempVector( "dx", 2 );
			tHlslVariableConstPtr dy = writer.fMakeTempVector( "dy", 2 );

			writer.fBeginLine( ) << texAtlasDims->fDeclaration( ) << " = " << hlslGenTree.fShadeNodeReq( 1 )->fSwizzle( 2 ) << ";" << std::endl;
			writer.fBeginLine( ) << tileTexelDims->fDeclaration( ) << " = " << hlslGenTree.fShadeNodeReq( 1 )->fSwizzle( "zw" ) << ";" << std::endl;
			writer.fBeginLine( ) << invTexAtlasDims->fDeclaration( ) << " = 1.0f / " << texAtlasDims->fName( ) << ";" << std::endl;
			writer.fBeginLine( ) << invTexAtlasTexelDims->fDeclaration( ) << " = 1.0f / ( " << tileTexelDims->fName( ) << " * " << texAtlasDims->fName( ) << " );" << std::endl;
			writer.fBeginLine( ) << dx->fDeclaration( ) << " = " << invTexAtlasTexelDims->fSwizzle( "x" ) << " * ddx( " << inputResults[ 0 ]->fName( ) << " * " << tileTexelDims->fSwizzle( "x" ) << " );" << std::endl;
			writer.fBeginLine( ) << dy->fDeclaration( ) << " = " << invTexAtlasTexelDims->fSwizzle( "x" ) << " * ddy( " << inputResults[ 0 ]->fName( ) << " * " << tileTexelDims->fSwizzle( "y" ) << " );" << std::endl;

			/*
				float2 texAtlasDims = gColorMapAtlasInfo.xy;
				float2 tileTexelDims = gColorMapAtlasInfo.zw;
				float2 invTexAtlasDims = 1.f / textureAtlasDims;
				float2 invTexAtlasTexelDims = 1.f / ( tileTexelDims * textureAtlasDims );
				float2 dx = invTexAtlasTexelDims.x * ddx( uv * tileTexelDims.x );
				float2 dy = invTexAtlasTexelDims.y * ddy( uv * tileTexelDims.y );
			*/

			tHlslVariableConstPtr uvOffset = writer.fMakeTempVector( "uvOffset", 2 );
			tHlslVariableConstPtr uvFinal = writer.fMakeTempVector( "uvFinal", 2 );

			writer.fBeginLine( ) 
				<< uvOffset->fDeclaration( ) << " = float2( " 
				<< "fmod( " << inputResults[ 1 ]->fSwizzle( 1 ) << ", " << texAtlasDims->fSwizzle( 1 ) << " ), " 
				<< "floor( " << inputResults[ 1 ]->fSwizzle( 1 ) << " * " << invTexAtlasDims->fSwizzle( 1 ) << " ) );" << std::endl;
			writer.fBeginLine( ) 
				<< uvFinal->fDeclaration( ) << " = " 
				<< uvOffset->fName( ) << " * " << invTexAtlasDims->fName( ) << " + " << invTexAtlasTexelDims->fName( ) << " + "
				<< "frac( " << inputResults[ 0 ]->fName( ) << " ) * ( " << invTexAtlasDims->fName( ) << " - 2.f * " << invTexAtlasTexelDims->fName( ) << " );" << std::endl; 
			
			writer.fBeginLine( ) << output->fDeclaration( ) << " = tex2Dgrad( " 
				<< hlslGenTree.fShadeNodeReq( 0 )->fName( ) << ", " 
				<< uvFinal->fName( ) << ", " 
				<< dx->fName( ) << ", " 
				<< dy->fName( ) <<  " );" << std::endl;

			//float2 uvOffset = float2( fmod( index, texAtlasDims.x ), floor( index * invTexAtlasDims.x ) );
			//float2 uvFinal = uvOffset * invTexAtlasDims + invTexAtlasTexelDims + uv * ( invTexAtlasDims - 2.f * invTexAtlasTexelDims );
			//float4 colorMapAtlas = tex2Dgrad( gColorMapAtlas, uvFinal, dx, dy );
		}
	}
	void tColorMapAtlasShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		const tFilePathPtr srcFileName = tFilePathPtr( fGetTexAtlasMaterialPropertyValue( *this ) );

		Math::tVec4f atlasInfo;
		Dx9Util::tBaseTexturePtr texPtr = textureCache.fFindLoadAtlas( srcFileName, atlasInfo );
		if( !texPtr )
		{
			texPtr = textureCache.fWhiteTexture( );
			atlasInfo = Math::tVec4f( 1 );
		}

		Gfx::tTextureReference tr;
		tr.fSetRaw( ( Gfx::tTextureReference::tPlatformHandle )texPtr.fGetRawPtr( ) );
		
		// TODO get sampling modes from user
		tr.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );
		glueVals.fUpdateAtlasSampler( tr, atlasInfo, fMaterialGlueIndex( ) );

	}
	void tColorMapAtlasShadeNode::fAcquireMaterialGlueValuesForAssetGen( 
		Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		const tFilePathPtr srcFileName = tFilePathPtr( fGetTexAtlasMaterialPropertyValue( *this ) );

		if( srcFileName.fLength( ) )
		{
			resourcePathsOut.fFindOrAdd( srcFileName );

			tLoadInPlaceResourcePtr* lip = lipFileCreator.fAddLoadInPlaceResourcePtr( 
				tResourceId::fMake<Gfx::tTextureFile>( Tatml::fTatmlPathToTatb( srcFileName ) ) );
			
			Gfx::tTextureReference tr;
			tr.fSetLoadInPlace( lip );

			// TODO get sampling modes from user
			tr.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );
			glueVals.fUpdateSampler( tr, fMaterialGlueIndex( ) );
		}

		else
		{
			glueVals.fUpdateSampler( 
				Gfx::tShadeMaterialGlueValues::cTexSourceWhite, fMaterialGlueIndex( ) );
		}
	}

	//------------------------------------------------------------------------------
	// tNormalMapAtlasShadeNode
	//------------------------------------------------------------------------------

	register_rtti_factory( tCubeMapShadeNode, false );
	tCubeMapShadeNode::tCubeMapShadeNode( const wxPoint& p )
		: tShadeNode( "CubeMap", cTextureMappingBarColor, cTextureMappingTextColor, true, p )
	{
		fAddInput( "InputVector" );
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( cNamePropertiesDefault ) ) );
	}
	b32 tCubeMapShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		fUpdateTexturePathBrowseFilters( fMatEdDisplayName( index ) );
		return fRefreshTextureMaterialProperties( this, src, index );
	}
	void tCubeMapShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		tHlslVariableConstPtr sampler( tHlslVariable::fMakeSamplerCube( "gCubeMap", reqs.mNextSamplerRegister++ ) );
		hlslGenTree.fCacheShadeNodeReq( sampler );
		reqs.mGlobals.fFindOrAdd( sampler );

		if( !hlslGenTree.fInputs( )[ 0 ] ) // add default reflection vector input if none was specified
			hlslGenTree.fForceSetInput( 0, tHlslGenTreePtr( new tHlslGenTree( tShadeNodePtr( new tReflectionVectorShadeNode( ) ), &hlslGenTree ) ) );
	}
	void tCubeMapShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );
		sigassert( inputResults[ 0 ] );

		fSetMaterialGlueIndex( writer.fAddMaterialGlueSampler( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "cubeMap", 4 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// sample cube map from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = texCUBE( " << hlslGenTree.fShadeNodeReq( )->fName( ) << ", " << inputResults[ 0 ]->fSwizzle( 3 ) << " );" << std::endl;
	}
	void tCubeMapShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		fUpdateTextureMaterialGlueValues( *this, true, glueVals, textureCache );
	}
	void tCubeMapShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		fAcquireTextureMaterialGlueValuesForAssetGen( *this, true, glueVals, lipFileCreator, resourcePathsOut );
	}


	namespace
	{
		const wxColour cBlendOpsBarColor( 0x22, 0x44, 0xdd );
		const wxColour cBlendOpsTextColor( 0xff, 0xff, 0xaa );

		static void fResolveBinaryOpInputResults( const HlslGen::tHlslGenTree::tInputResultArray& inputResults, std::string& a, std::string& b, u32& resultDimension )
		{
			if( inputResults[ 0 ] && inputResults[ 1 ] )
			{
				resultDimension = fMax( inputResults[ 0 ]->fDimensionX( ), inputResults[ 1 ]->fDimensionX( ) );
				a = inputResults[ 0 ]->fSwizzle( resultDimension );
				b = inputResults[ 1 ]->fSwizzle( resultDimension );
			}
			else if( inputResults[ 0 ] )
			{
				a = inputResults[ 0 ]->fName( );
				b = inputResults[ 0 ]->fCastValueToType( 1.f );
				resultDimension = inputResults[ 0 ]->fDimensionX( );
			}
			else if( inputResults[ 1 ] )
			{
				a = inputResults[ 1 ]->fCastValueToType( 1.f );
				b = inputResults[ 1 ]->fName( );
				resultDimension = inputResults[ 1 ]->fDimensionX( );
			}
			else
			{
				a = "float4(0,0,0,0)";
				b = "float4(1,1,1,1)";
				resultDimension = 4;
			}
		}
	}

	register_rtti_factory( tAddShadeNode, false );
	tAddShadeNode::tAddShadeNode( const wxPoint& p )
		: tShadeNode( "Add", cBlendOpsBarColor, cBlendOpsTextColor, false, p )
	{
		fAddInput( "First" );
		fAddInput( "Second" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tAddShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string a, b;
		u32 resultDimension = 4;
		fResolveBinaryOpInputResults( inputResults, a, b, resultDimension );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "addResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = " << a << " + " << b << ";" << std::endl;
	}

	register_rtti_factory( tSubtractShadeNode, false );
	tSubtractShadeNode::tSubtractShadeNode( const wxPoint& p )
		: tShadeNode( "Subtract", cBlendOpsBarColor, cBlendOpsTextColor, false, p )
	{
		fAddInput( "First" );
		fAddInput( "Second" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tSubtractShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string a, b;
		u32 resultDimension = 4;
		fResolveBinaryOpInputResults( inputResults, a, b, resultDimension );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "subtractResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = " << a << " - " << b << ";" << std::endl;
	}

	register_rtti_factory( tMultiplyShadeNode, false );
	tMultiplyShadeNode::tMultiplyShadeNode( const wxPoint& p )
		: tShadeNode( "Multiply", cBlendOpsBarColor, cBlendOpsTextColor, false, p )
	{
		fAddInput( "First" );
		fAddInput( "Second" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tMultiplyShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string a, b;
		u32 resultDimension = 4;
		fResolveBinaryOpInputResults( inputResults, a, b, resultDimension );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "multiplyResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = " << a << " * " << b << ";" << std::endl;
	}


	register_rtti_factory( tDivideShadeNode, false );
	tDivideShadeNode::tDivideShadeNode( const wxPoint& p )
		: tShadeNode( "Divide", cBlendOpsBarColor, cBlendOpsTextColor, false, p )
	{
		fAddInput( "First" );
		fAddInput( "Second" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tDivideShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string a, b;
		u32 resultDimension = 4;
		fResolveBinaryOpInputResults( inputResults, a, b, resultDimension );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "divideResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = " << a << " / " << b << ";" << std::endl;
	}


	register_rtti_factory( tPowShadeNode, false );
	tPowShadeNode::tPowShadeNode( const wxPoint& p )
		: tShadeNode( "Pow", cBlendOpsBarColor, cBlendOpsTextColor, false, p )
	{
		fAddInput( "Value" );
		fAddInput( "Power" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tPowShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string a, b;
		u32 resultDimension = 4;
		fResolveBinaryOpInputResults( inputResults, a, b, resultDimension );

		tHlslVariableConstPtr output = writer.fMakeTempVector( "powResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = pow( " << a << ", " << b << " );" << std::endl;
	}

	register_rtti_factory( tOneMinusShadeNode, false );
	tOneMinusShadeNode::tOneMinusShadeNode( const wxPoint& p )
		: tShadeNode( "OneMinus", cBlendOpsBarColor, cBlendOpsTextColor, false, p )
	{
		fAddInput( "Value" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tOneMinusShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string value;
		u32 resultDimension = 1;

		if( inputResults[ 0 ] )
		{
			value = inputResults[ 0 ]->fSwizzle( resultDimension );
			resultDimension = inputResults[ 0 ]->fDimensionX( );
		}
		else
			value = "0.5";

		tHlslVariableConstPtr output = writer.fMakeTempVector( "oneMinusResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = 1.0 - " << value << ";" << std::endl;
	}

	register_rtti_factory( tBlendShadeNode, false );
	tBlendShadeNode::tBlendShadeNode( const wxPoint& p )
		: tShadeNode( "Blend", cBlendOpsBarColor, cBlendOpsTextColor, false, p )
	{
		fAddInput( "First" );
		fAddInput( "Second" );
		fAddInput( "BlendFactor" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tBlendShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string a, b, blend;
		u32 resultDimension = 4;
		fResolveBinaryOpInputResults( inputResults, a, b, resultDimension );

		if( inputResults[ 2 ] )
			blend = inputResults[ 2 ]->fSwizzle( resultDimension );
		else
			blend = "0.5";

		tHlslVariableConstPtr output = writer.fMakeTempVector( "blendResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = lerp( " << a << ", " << b << ", " << blend << " );" << std::endl;
	}

	//------------------------------------------------------------------------------
	// tDotShadeNode
	//------------------------------------------------------------------------------
	register_rtti_factory( tDotShadeNode, false );
	tDotShadeNode::tDotShadeNode( const wxPoint& p )
		: tShadeNode( "Dot", cBlendOpsBarColor, cBlendOpsTextColor, false, p )
	{
		fAddInput( "First" );
		fAddInput( "Second" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tDotShadeNode::fWriteHlsl( 
		HlslGen::tHlslWriter& writer, 
		const HlslGen::tShaderRequirements& reqs, 
		HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string a, b;
		u32 resultDimension = 4;
		fResolveBinaryOpInputResults( inputResults, a, b, resultDimension );
		
		tHlslVariableConstPtr output = writer.fMakeTempVector( "dotResult", 1 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = dot( " << a << ", " << b << " );" << std::endl;
	}



	namespace
	{
		const wxColour cColorValuesBarColor( 0xaa, 0x11, 0x88 );
		const wxColour cColorValuesTextColor( 0xff, 0xff, 0x99 );
		const char cPropNameColorValue[]=".Color";

		static Math::tVec4f fGetColorMaterialPropertyValue( tShadeNode& src )
		{
			tColorPickerData value = src.fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesDefault, tColorPickerData( ) );
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( cPropNameColorValue );
			if( existingValueProp )
				existingValueProp->fGetData( value );
			return value.fExpandRgba( );
		}
		static b32 fRefreshColorMaterialProperties( tShadeNode* shadeNode, tShadeNode& src, u32 index, b32 alpha )
		{
			const std::string title = shadeNode->fMatEdDisplayName( index );
			const tColorPickerData min = shadeNode->fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesMin, tColorPickerData( ) );
			const tColorPickerData max = shadeNode->fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesMax, tColorPickerData( ) );
			
			tColorPickerData value = shadeNode->fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesDefault, tColorPickerData( ) );
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( cPropNameColorValue );
			if( existingValueProp )
				existingValueProp->fGetData( value );
			value = fClamp( value, min, max );
			shadeNode->fMatProps( ).fClear( 32 );
			shadeNode->fMatProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyColor( title + cPropNameColorValue, value, min, max ) ) );
			return true;
		}
	}

	register_rtti_factory( tColorRGBShadeNode, false );
	tColorRGBShadeNode::tColorRGBShadeNode( const wxPoint& p )
		: tShadeNode( "ColorRGB", cColorValuesBarColor, cColorValuesTextColor, true, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyColor( cNamePropertiesDefault ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyColor( cNamePropertiesMin, tColorPickerData( 0.f, 1.f, 1.f ) ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyColor( cNamePropertiesMax, tColorPickerData( 1.f, 1.f, 1.f ) ) ) );
	}
	b32 tColorRGBShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		return fRefreshColorMaterialProperties( this, src, index, false );
	}
	void tColorRGBShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		tHlslVariableConstPtr var( tHlslVariable::fMakeGlobalVector( "gRgb" + fMatEdName( ), reqs.mNextConstantRegister++, 3 ) );
		hlslGenTree.fCacheShadeNodeReq( var );
		reqs.mGlobals.fFindOrAdd( var );
	}
	void tColorRGBShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		fSetMaterialGlueIndex( writer.fAddMaterialGlueVector( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );
		hlslGenTree.fCacheShadeNodeOutput( hlslGenTree.fShadeNodeReq( ) );
	}
	void tColorRGBShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		const Math::tVec4f v = fGetColorMaterialPropertyValue( *this );
		glueVals.fUpdateVector( v, fMaterialGlueIndex( ) );
	}
	void tColorRGBShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		const Math::tVec4f v = fGetColorMaterialPropertyValue( *this );
		glueVals.fUpdateVector( v, fMaterialGlueIndex( ) );
	}

	register_rtti_factory( tColorRGBAShadeNode, false );
	tColorRGBAShadeNode::tColorRGBAShadeNode( const wxPoint& p )
		: tShadeNode( "ColorRGBA", cColorValuesBarColor, cColorValuesTextColor, true, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyColor( cNamePropertiesDefault ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyColor( cNamePropertiesMin, tColorPickerData( 0.f, 0.f, 1.f ) ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyColor( cNamePropertiesMax, tColorPickerData( 1.f, 1.f, 1.f ) ) ) );
	}
	b32 tColorRGBAShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		return fRefreshColorMaterialProperties( this, src, index, true );
	}
	void tColorRGBAShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		tHlslVariableConstPtr var( tHlslVariable::fMakeGlobalVector( "gRgba" + fMatEdName( ), reqs.mNextConstantRegister++, 4 ) );
		hlslGenTree.fCacheShadeNodeReq( var );
		reqs.mGlobals.fFindOrAdd( var );
	}
	void tColorRGBAShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		fSetMaterialGlueIndex( writer.fAddMaterialGlueVector( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );
		hlslGenTree.fCacheShadeNodeOutput( hlslGenTree.fShadeNodeReq( ) );
	}
	void tColorRGBAShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		const Math::tVec4f v = fGetColorMaterialPropertyValue( *this );
		glueVals.fUpdateVector( v, fMaterialGlueIndex( ) );
	}
	void tColorRGBAShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		const Math::tVec4f v = fGetColorMaterialPropertyValue( *this );
		glueVals.fUpdateVector( v, fMaterialGlueIndex( ) );
	}


	register_rtti_factory( tVertexColorShadeNode, false );
	tVertexColorShadeNode::tVertexColorShadeNode( const wxPoint& p )
		: tShadeNode( "VertexColor", cColorValuesBarColor, cColorValuesTextColor, false, p )
	{
		fComputeDimensions( );
		fAddOutput( );
	}
	void tVertexColorShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		if( writer.fInput( ).mToolType != HlslGen::cToolTypeMaya )
		{
			using namespace HlslGen;
			tHlslVariableConstPtr var = tPixelShaderVars::fVariable( tPixelShaderVars::cInVertexColor, writer.fInput( ).mPid );
			hlslGenTree.fCacheShadeNodeReq( var );
			reqs.mInputs.fFindOrAdd( var );
		}
	}
	void tVertexColorShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		if( writer.fInput( ).mToolType != HlslGen::cToolTypeMaya )
		{
			hlslGenTree.fCacheShadeNodeOutput( hlslGenTree.fShadeNodeReq( ) );
		}
	}







	namespace
	{
		const wxColour cNumberValuesBarColor( 0x66, 0x66, 0x66 );
		const wxColour cNumberValuesTextColor( 0xff, 0xff, 0xff );
		const char cPropNameNumericValue[]=".Value";

		template<class tNumericType, class tEditablePropertyType>
		static tNumericType fGetNumericMaterialPropertyValue( tShadeNode& src, const char* matPropName )
		{
			tNumericType value = src.fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesDefault, tNumericType( 0.f ) );
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( matPropName );
			if( existingValueProp )
				existingValueProp->fGetData( value );
			return value;
		}

		template<class tEditablePropertyType>
		static std::string fGetStringMaterialPropertyValue( tShadeNode& src, const char* matPropName )
		{
			std::string value = src.fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesDefault, std::string( "" ) );
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( matPropName );
			if( existingValueProp )
				existingValueProp->fGetData( value );
			return value;
		}

		template<class tNumericType, class tEditablePropertyType>
		static b32 fRefreshNumericMaterialProperties( tShadeNode* shadeNode, tShadeNode& src, u32 index, const char* matPropName )
		{
			const std::string title = shadeNode->fMatEdDisplayName( index );
			const f32 min = shadeNode->fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesMin, 0.f );
			const f32 max = shadeNode->fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesMax, 0.f );
			tNumericType value = shadeNode->fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesDefault, tNumericType( 0.f ) );
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( matPropName );
			if( existingValueProp )
				existingValueProp->fGetData( value );
			value = fClamp<tNumericType>( value, min, max );
			shadeNode->fMatProps( ).fClear( 32 );
			shadeNode->fMatProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyType( title + matPropName, value, min, max, 0.01f, 2 ) ) );
			return true;
		}

		template<class tEditablePropertyType>
		static b32 fRefreshStringMaterialProperties( tShadeNode* shadeNode, tShadeNode& src, u32 index, const char* matPropName )
		{
			const std::string title = shadeNode->fMatEdDisplayName( index );
			std::string value = shadeNode->fShadeProps( ).fGetValue( tShadeNode::cNamePropertiesDefault, std::string( "" ) );
			tEditablePropertyPtr existingValueProp = src.fMatProps( ).fFindPartial( matPropName );
			if( existingValueProp )
				existingValueProp->fGetData( value );
			shadeNode->fMatProps( ).fClear( 32 );
			shadeNode->fMatProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyType( title + matPropName, value ) ) );
			return true;
		}
	}


	register_rtti_factory( tNumberShadeNode, false );
	tNumberShadeNode::tNumberShadeNode( const wxPoint& p )
		: tShadeNode( "Number", cNumberValuesBarColor, cNumberValuesTextColor, true, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesDefault, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMin, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMax, 1.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
	}
	b32 tNumberShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		return fRefreshNumericMaterialProperties<f32,tEditablePropertyFloat>( this, src, index, cPropNameNumericValue );
	}
	void tNumberShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		tHlslVariableConstPtr var( tHlslVariable::fMakeGlobalVector( "gNumber" + fMatEdName( ), reqs.mNextConstantRegister++, 1 ) );
		hlslGenTree.fCacheShadeNodeReq( var );
		reqs.mGlobals.fFindOrAdd( var );
	}
	void tNumberShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		fSetMaterialGlueIndex( writer.fAddMaterialGlueVector( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );
		hlslGenTree.fCacheShadeNodeOutput( hlslGenTree.fShadeNodeReq( ) );
	}
	void tNumberShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		const f32 v = fGetNumericMaterialPropertyValue<f32,tEditablePropertyFloat>( *this, cPropNameNumericValue );
		glueVals.fUpdateVector( v, fMaterialGlueIndex( ) );
	}
	void tNumberShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		const f32 v = fGetNumericMaterialPropertyValue<f32,tEditablePropertyFloat>( *this, cPropNameNumericValue );
		glueVals.fUpdateVector( v, fMaterialGlueIndex( ) );
	}

	register_rtti_factory( tVector2ShadeNode, false );
	tVector2ShadeNode::tVector2ShadeNode( const wxPoint& p )
		: tShadeNode( "Vector2", cNumberValuesBarColor, cNumberValuesTextColor, true, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyVec2f( cNamePropertiesDefault, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMin, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMax, 1.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
	}
	b32 tVector2ShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		return fRefreshNumericMaterialProperties<Math::tVec2f,tEditablePropertyVec2f>( this, src, index, cPropNameNumericValue );
	}
	void tVector2ShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		tHlslVariableConstPtr var( tHlslVariable::fMakeGlobalVector( "gVec2" + fMatEdName( ), reqs.mNextConstantRegister++, 2 ) );
		hlslGenTree.fCacheShadeNodeReq( var );
		reqs.mGlobals.fFindOrAdd( var );
	}
	void tVector2ShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		fSetMaterialGlueIndex( writer.fAddMaterialGlueVector( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );
		hlslGenTree.fCacheShadeNodeOutput( hlslGenTree.fShadeNodeReq( ) );
	}
	void tVector2ShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		const Math::tVec2f v = fGetNumericMaterialPropertyValue<Math::tVec2f,tEditablePropertyVec2f>( *this, cPropNameNumericValue );
		glueVals.fUpdateVector( Math::tVec4f( v.x, v.y, 0.f, 1.f ), fMaterialGlueIndex( ) );
	}
	void tVector2ShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		const Math::tVec2f v = fGetNumericMaterialPropertyValue<Math::tVec2f,tEditablePropertyVec2f>( *this, cPropNameNumericValue );
		glueVals.fUpdateVector( Math::tVec4f( v.x, v.y, 0.f, 1.f ), fMaterialGlueIndex( ) );
	}

	register_rtti_factory( tVector3ShadeNode, false );
	tVector3ShadeNode::tVector3ShadeNode( const wxPoint& p )
		: tShadeNode( "Vector3", cNumberValuesBarColor, cNumberValuesTextColor, true, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyVec3f( cNamePropertiesDefault, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMin, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMax, 1.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
	}
	b32 tVector3ShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		return fRefreshNumericMaterialProperties<Math::tVec3f,tEditablePropertyVec3f>( this, src, index, cPropNameNumericValue );
	}
	void tVector3ShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		tHlslVariableConstPtr var( tHlslVariable::fMakeGlobalVector( "gVec3" + fMatEdName( ), reqs.mNextConstantRegister++, 3 ) );
		hlslGenTree.fCacheShadeNodeReq( var );
		reqs.mGlobals.fFindOrAdd( var );
	}
	void tVector3ShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		fSetMaterialGlueIndex( writer.fAddMaterialGlueVector( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );
		hlslGenTree.fCacheShadeNodeOutput( hlslGenTree.fShadeNodeReq( ) );
	}
	void tVector3ShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		const Math::tVec3f v = fGetNumericMaterialPropertyValue<Math::tVec3f,tEditablePropertyVec3f>( *this, cPropNameNumericValue );
		glueVals.fUpdateVector( Math::tVec4f( v, 1.f ), fMaterialGlueIndex( ) );
	}
	void tVector3ShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		const Math::tVec3f v = fGetNumericMaterialPropertyValue<Math::tVec3f,tEditablePropertyVec3f>( *this, cPropNameNumericValue );
		glueVals.fUpdateVector( Math::tVec4f( v, 1.f ), fMaterialGlueIndex( ) );
	}

	register_rtti_factory( tVector4ShadeNode, false );
	tVector4ShadeNode::tVector4ShadeNode( const wxPoint& p )
		: tShadeNode( "Vector4", cNumberValuesBarColor, cNumberValuesTextColor, true, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyVec4f( cNamePropertiesDefault, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMin, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMax, 1.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
	}
	b32 tVector4ShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		return fRefreshNumericMaterialProperties<Math::tVec4f,tEditablePropertyVec4f>( this, src, index, cPropNameNumericValue );
	}
	void tVector4ShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		tHlslVariableConstPtr var( tHlslVariable::fMakeGlobalVector( "gVec4" + fMatEdName( ), reqs.mNextConstantRegister++, 4 ) );
		hlslGenTree.fCacheShadeNodeReq( var );
		reqs.mGlobals.fFindOrAdd( var );
	}
	void tVector4ShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		fSetMaterialGlueIndex( writer.fAddMaterialGlueVector( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );
		hlslGenTree.fCacheShadeNodeOutput( hlslGenTree.fShadeNodeReq( ) );
	}
	void tVector4ShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		const Math::tVec4f v = fGetNumericMaterialPropertyValue<Math::tVec4f,tEditablePropertyVec4f>( *this, cPropNameNumericValue );
		glueVals.fUpdateVector( v, fMaterialGlueIndex( ) );
	}
	void tVector4ShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		const Math::tVec4f v = fGetNumericMaterialPropertyValue<Math::tVec4f,tEditablePropertyVec4f>( *this, cPropNameNumericValue );
		glueVals.fUpdateVector( v, fMaterialGlueIndex( ) );
	}

	register_rtti_factory( tDynamicVec4ShadeNode, false );
	tDynamicVec4ShadeNode::tDynamicVec4ShadeNode( const wxPoint& p )
		: tShadeNode( "DynamicValue", cNumberValuesBarColor, cNumberValuesTextColor, true, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyString( cNamePropertiesDefault ) ) );

		//fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyVec4f( cNamePropertiesDefault, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		//fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMin, 0.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
		//fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cNamePropertiesMax, 1.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
	}
	b32 tDynamicVec4ShadeNode::fRefreshMaterialProperties( tShadeNode& src, u32 index )
	{
		return fRefreshStringMaterialProperties<tEditablePropertyString>( this, src, index, ".PropertyName" );
	}
	void tDynamicVec4ShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		tHlslVariableConstPtr var( tHlslVariable::fMakeGlobalVector( "gDynamicVec4" + fMatEdName( ), reqs.mNextConstantRegister++, 4 ) );
		hlslGenTree.fCacheShadeNodeReq( var );
		reqs.mGlobals.fFindOrAdd( var );
	}
	void tDynamicVec4ShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		fSetMaterialGlueIndex( writer.fAddMaterialGlueString( hlslGenTree.fShadeNodeReq( )->fSemantic( ).mRegisterIndex, fMaterialGlueIndex( ) ) );
		hlslGenTree.fCacheShadeNodeOutput( hlslGenTree.fShadeNodeReq( ) );
	}
	void tDynamicVec4ShadeNode::fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache )
	{
		//std::string s = fGetStringMaterialPropertyValue<tEditablePropertyString>( *this, ".PropertyName" );
		//glueVals.fUpdateString( s.c_str( ), lipFileCreator, fMaterialGlueIndex( ) );
	}
	void tDynamicVec4ShadeNode::fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut )
	{
		std::string s = fGetStringMaterialPropertyValue<tEditablePropertyString>( *this, ".PropertyName" );
		glueVals.fUpdateString( s.c_str( ), lipFileCreator, fMaterialGlueIndex( ) );
	}



	namespace
	{
		const wxColour cNormalBarColor( 0x11, 0x11, 0x88 );
		const wxColour cNormalTextColor( 0x44, 0xff, 0x44 );

		static void fAddCommonNormalHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
		{
			using namespace HlslGen;
			reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceNormal, writer.fInput( ).mPid ) );
			reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobal_BackFace_ShadowMapEpsilon_TexelSize_Amount, writer.fInput( ).mPid ) );
			reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInFaceSign, writer.fInput( ).mPid ) );
		}

		static void fWriteNormalDoubleSidedHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree, const HlslGen::tHlslVariableConstPtr& output )
		{
			using namespace HlslGen;
			tHlslVariableConstPtr backFace_epsilon_texelSize_amount = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobal_BackFace_ShadowMapEpsilon_TexelSize_Amount, writer.fInput( ).mPid );
			tHlslVariableConstPtr faceSign = tPixelShaderVars::fVariable( tPixelShaderVars::cInFaceSign, writer.fInput( ).mPid );
			writer.fBeginLine( true ) << "// multiply effective normal by the sign of the face register to achieve proper double-sided lighting;" << std::endl;
			writer.fBeginLine( ) << "// note that we negate bcz our idea of 'back-face' is opposite from D3D." << std::endl;
			const char* signSymbol = ( ( writer.fInput( ).mPid == HlslGen::cPidXbox360 ) ? "+" : "-" );
			writer.fBeginLine( ) << output->fSwizzle( "xyz" ) << " *= min( 1.0, " << backFace_epsilon_texelSize_amount->fSwizzle( "x" ) << " " << signSymbol << " sign( " << faceSign->fName( ) << " ) );" << std::endl;
		}
	}



	register_rtti_factory( tGeometryNormalShadeNode, false );
	tGeometryNormalShadeNode::tGeometryNormalShadeNode( const wxPoint& p )
		: tShadeNode( "GeometryNormal", cNormalBarColor, cNormalTextColor, false, p )
	{
		fComputeDimensions( );
		fAddOutput( );
	}
	void tGeometryNormalShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		fAddCommonNormalHlslRequirements( writer, reqs, hlslGenTree );
	}
	void tGeometryNormalShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		tHlslVariableConstPtr output = writer.fMakeTempVector( "geometryNormal", 3 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		tHlslVariableConstPtr litSpaceNormal = tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceNormal, writer.fInput( ).mPid );
		writer.fBeginLine( true ) << "// copy input litSpaceNormal" << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( )
			<< " = " << litSpaceNormal->fSwizzle( "xyz" ) << ";" << std::endl;

		fWriteNormalDoubleSidedHlsl( writer, reqs, hlslGenTree, output );
	}



	register_rtti_factory( tPerPixelNormalShadeNode, false );
	tPerPixelNormalShadeNode::tPerPixelNormalShadeNode( const wxPoint& p )
		: tShadeNode( "PerPixelNormal", cNormalBarColor, cNormalTextColor, false, p )
	{
		fAddInput( "NormalMap" );
		fAddInput( "BumpScale" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tPerPixelNormalShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		fAddCommonNormalHlslRequirements( writer, reqs, hlslGenTree );

		if( hlslGenTree.fInputs( )[ 0 ] )
			reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceTangent, writer.fInput( ).mPid ) );
	}
	void tPerPixelNormalShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		tHlslVariableConstPtr tangSpaceNorm = inputResults[ 0 ];

		if( !tangSpaceNorm )
		{
			// no input was specified, so just output the regular geometry normal
			tGeometryNormalShadeNode defaultNode;
			defaultNode.fWriteHlsl( writer, reqs, hlslGenTree );
			return;
		}

		tHlslVariableConstPtr output = writer.fMakeTempVector( "perPixelNormal", 3 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		tHlslVariableConstPtr litSpaceNormal = tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceNormal, writer.fInput( ).mPid );
		tHlslVariableConstPtr litSpaceTangent = tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceTangent, writer.fInput( ).mPid );

		if( inputResults[ 1 ] && tangSpaceNorm->fDimensionX( ) >= 3 )
		{
			writer.fBeginLine( true ) << "// scale normal map by bump strength" << std::endl;
			writer.fBeginLine( ) << tangSpaceNorm->fSwizzle( "xz" ) << " *= " << inputResults[ 1 ]->fSwizzle( 1 ) << ";" << std::endl;
		}

		writer.fBeginLine( true ) << "// compute binormal/bitangent" << std::endl;
		tHlslVariableConstPtr litSpaceBinormal = writer.fMakeTempVector( "litSpaceBinormal", 3 );
		writer.fBeginLine( ) << litSpaceBinormal->fDeclaration( ) 
			<< " = cross( " << litSpaceTangent->fSwizzle( "xyz" ) << ", " << litSpaceNormal->fSwizzle( "xyz" ) << " )";
		if( writer.fInput( ).mPreviewMode )
			writer.fContinueLine( ) << " * -1";
		else
			writer.fContinueLine( ) << " * " << litSpaceTangent->fSwizzle( "w" );
		writer.fContinueLine( ) << ";" << std::endl;

		writer.fBeginLine( true ) << "// transform tangent space normal to world space" << std::endl;
		const b32 xOnly = ( tangSpaceNorm->fDimensionX( ) < 3 );
		writer.fBeginLine( ) << output->fDeclaration( ) << " = normalize( " << std::endl;
		writer.fPushTab( );
		writer.fBeginLine( ) << tangSpaceNorm->fSwizzle( xOnly ? "x" : "x" ) << " * " << litSpaceTangent->fSwizzle( "xyz" ) << " + " << std::endl;
		writer.fBeginLine( ) << tangSpaceNorm->fSwizzle( xOnly ? "x" : "y" ) << " * " << litSpaceNormal->fSwizzle( "xyz" ) << " + " << std::endl;
		writer.fBeginLine( ) << tangSpaceNorm->fSwizzle( xOnly ? "x" : "z" ) << " * " << litSpaceBinormal->fSwizzle( "xyz" ) << " );" << std::endl;
		writer.fPopTab( );

		fWriteNormalDoubleSidedHlsl( writer, reqs, hlslGenTree, output );
	}

	register_rtti_factory( tReflectionVectorShadeNode, false );
	tReflectionVectorShadeNode::tReflectionVectorShadeNode( const wxPoint& p )
		: tShadeNode( "ReflectionVec", cNormalBarColor, cNormalTextColor, false, p )
	{
		fAddInput( "Normal" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tReflectionVectorShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, writer.fInput( ).mPid ) );
		reqs.mCommon.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexDir, writer.fInput( ).mPid ) );
		if( !hlslGenTree.fInputs( )[ 0 ] ) // add default normal if none was specified
			hlslGenTree.fForceSetInput( 0, tHlslGenTreePtr( new tHlslGenTree( tShadeNodePtr( new tGeometryNormalShadeNode( ) ), &hlslGenTree ) ) );
	}
	void tReflectionVectorShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		tHlslVariableConstPtr eyeToVertex = tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexDir, writer.fInput( ).mPid );
		tHlslVariableConstPtr effectiveNormal = inputResults[ 0 ];
		sigassert( effectiveNormal );

		tHlslVariableConstPtr reflectionVec = writer.fMakeTempVector( "reflectionVec", 3 );
		hlslGenTree.fCacheShadeNodeOutput( reflectionVec );

		writer.fBeginLine( true ) << "// reflect eye-relative position across normal" << std::endl;
		writer.fBeginLine( ) << reflectionVec->fDeclaration( )
			<< " = reflect( " << eyeToVertex->fSwizzle( 3 ) << ", " << effectiveNormal->fSwizzle( 3 ) << " );" << std::endl;
	}

	namespace
	{
		static const char cWorldSpaceScaleName[] = "Shader Properties.Scale";
	}

	register_rtti_factory( tWorldSpacePosShadeNode, false );
	tWorldSpacePosShadeNode::tWorldSpacePosShadeNode( const wxPoint& p )
		: tShadeNode( "WorldSpacePos", cNormalBarColor, cNormalTextColor, false, p )
	{
		fComputeDimensions( );
		fAddOutput( );

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyVec3f( cWorldSpaceScaleName, 1.f, -9999.f, +9999.f, 0.001f, 3 ) ) );
	}
	void tWorldSpacePosShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, writer.fInput( ).mPid ) );
		reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalWorldEyePos, writer.fInput( ).mPid ) );
	}
	void tWorldSpacePosShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		tHlslVariableConstPtr output = writer.fMakeTempVector( "worldSpacePos", 3 );
		hlslGenTree.fCacheShadeNodeOutput( output );

		tHlslVariableConstPtr litSpacePos = tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, writer.fInput( ).mPid );
		tHlslVariableConstPtr eyePos = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalWorldEyePos, writer.fInput( ).mPid );

		const Math::tVec3f worldSpaceScale = fShadeProps( ).fGetValue( cWorldSpaceScaleName, Math::tVec3f::cOnesVector );
		std::stringstream ss; ss << "float3( " << worldSpaceScale.x << ", " << worldSpaceScale.y << ", " << worldSpaceScale.z << " )";
		const std::string worldSpaceScaleString = ss.str( );

		writer.fBeginLine( true ) << "// compute world space position" << std::endl;
		if( writer.fInput( ).mPreviewMode && writer.fInput( ).mToolType == cToolTypeMaya )
		{
			writer.fBeginLine( ) << output->fDeclaration( )
				<< " = " << worldSpaceScaleString << " * ( " << litSpacePos->fSwizzle( 3 ) << " + " << eyePos->fSwizzle( 3 ) << " ) / 100.0;" << std::endl;
		}
		else
		{
			writer.fBeginLine( ) << output->fDeclaration( )
				<< " = " << worldSpaceScaleString << " * ( " << litSpacePos->fSwizzle( 3 ) << " + " << eyePos->fSwizzle( 3 ) << " );" << std::endl;
		}
	}


	register_rtti_factory( tEdgeShadeNode, false );
	tEdgeShadeNode::tEdgeShadeNode( const wxPoint& p )
		: tShadeNode( "EdgeDetect", cNormalBarColor, cNormalTextColor, false, p )
	{
		fAddInput( "Normal" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tEdgeShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, writer.fInput( ).mPid ) );
		reqs.mCommon.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexDir, writer.fInput( ).mPid ) );
		if( !hlslGenTree.fInputs( )[ 0 ] ) // add default normal if none was specified
			hlslGenTree.fForceSetInput( 0, tHlslGenTreePtr( new tHlslGenTree( tShadeNodePtr( new tGeometryNormalShadeNode( ) ), &hlslGenTree ) ) );
	}
	void tEdgeShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		tHlslVariableConstPtr eyeToVertex = tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexDir, writer.fInput( ).mPid );
		tHlslVariableConstPtr effectiveNormal = inputResults[ 0 ];
		sigassert( effectiveNormal );

		tHlslVariableConstPtr edgeValue = writer.fMakeTempVector( "edgeValue", 1 );
		hlslGenTree.fCacheShadeNodeOutput( edgeValue );

		writer.fBeginLine( true ) << "// compute dot-product of normal with view vector" << std::endl;
		writer.fBeginLine( ) << edgeValue->fDeclaration( )
			<< " = 1.0 - abs( dot( " << eyeToVertex->fSwizzle( 3 ) << ", " << effectiveNormal->fSwizzle( 3 ) << " ) );" << std::endl;
	}



	namespace
	{
		const wxColour cUtilityBarColor( 0x55, 0xff, 0xff );
		const wxColour cUtilityTextColor( 0x22, 0x00, 0x00 );

		const char cPropNameChannelCount[]="Shader Properties.ChannelCount";
		const char cPropNameChannelR[]="Shader Properties.Channel1";
		const char cPropNameChannelG[]="Shader Properties.Channel2";
		const char cPropNameChannelB[]="Shader Properties.Channel3";
		const char cPropNameChannelA[]="Shader Properties.Channel4";

		const char cPropNameStopOnPause[]="Shader Properties.StopOnPause";
	}

	register_rtti_factory( tSwizzleShadeNode, false );
	tSwizzleShadeNode::tSwizzleShadeNode( const wxPoint& p )
		: tShadeNode( "Swizzle", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Input" );
		fComputeDimensions( );
		fAddOutput( );

		tDynamicArray<std::string> choices(6);
		choices[0] = "R";
		choices[1] = "G";
		choices[2] = "B";
		choices[3] = "A";
		choices[4] = "0.0";
		choices[5] = "1.0";

		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cPropNameChannelCount, 4.f, 1.f, 4.f, 1.f, 0 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( cPropNameChannelR, choices, 0 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( cPropNameChannelG, choices, 1 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( cPropNameChannelB, choices, 2 ) ) );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( cPropNameChannelA, choices, 3 ) ) );
	}
	void tSwizzleShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		const u32 channelCount = fChannelCount( );
		tHlslVariableConstPtr output = writer.fMakeTempVector( "swizzleResult", channelCount );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		if( inputResults[ 0 ] )
		{
			const std::string swizzleText = fSwizzleText( inputResults[ 0 ] );
			writer.fBeginLine( ) << output->fDeclaration( ) << " = " << swizzleText << ";" << std::endl;
		}
		else
			writer.fBeginLine( ) << output->fDeclaration( ) << " = " << output->fCastValueToType( 1.f ) << ";" << std::endl;
	}
	u32 tSwizzleShadeNode::fChannelCount( )
	{
		const f32 value = fShadeProps( ).fGetValue( cPropNameChannelCount, 0.f );
		return fClamp( fRound<u32>( value ), 1u, 4u );
	}
	std::string tSwizzleShadeNode::fSwizzleText( const HlslGen::tHlslVariableConstPtr& input )
	{
		using namespace HlslGen;

		std::stringstream ss;

		u32 channels[4] = {
			fShadeProps( ).fGetValue( cPropNameChannelR, 0u ),
			fShadeProps( ).fGetValue( cPropNameChannelG, 1u ),
			fShadeProps( ).fGetValue( cPropNameChannelB, 2u ),
			fShadeProps( ).fGetValue( cPropNameChannelA, 3u ),
		};
		const char* channelsText[6]={"r","g","b","a","0.0","1.0"};

		const u32 channelCount = fChannelCount( );
		const u32 maxDimension = input->fDimensionX( );

		tHlslVariableConstPtr dummyVar( tHlslVariable::fMakeVectorFloat( "dummy", tHlslVariable::cTemp, channelCount ) );

		if( channelCount > 1 )
			ss << dummyVar->fTypeString( ) << "( ";

		sigassert( channelCount <= array_length( channels ) );
		for( u32 i = 0; i < channelCount; ++i )
		{
			sigassert( channels[ i ] < array_length( channelsText ) );

			if( channels[ i ] <= 3 )
			{
				if( input->fDimensionX( ) > 1 )
					ss << input->fSwizzle( channelsText[ fMin( maxDimension - 1, channels[ i ] ) ] );
				else
					ss << input->fName( );
			}
			else
				ss << channelsText[ channels[ i ] ];

			if( i != channelCount - 1 )
				ss << ", ";
		}

		if( channelCount > 1 )
			ss << " )";

		return ss.str( );
	}

	register_rtti_factory( tSaturateShadeNode, false );
	tSaturateShadeNode::tSaturateShadeNode( const wxPoint& p )
		: tShadeNode( "Saturate", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Value" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tSaturateShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string value;
		u32 resultDimension = 1;

		if( inputResults[ 0 ] )
		{
			value = inputResults[ 0 ]->fName( );
			resultDimension = inputResults[ 0 ]->fDimensionX( );
		}
		else
			value = "0.5";

		tHlslVariableConstPtr output = writer.fMakeTempVector( "saturateResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = saturate( " << value << " );" << std::endl;
	}

	register_rtti_factory( tClampShadeNode, false );
	tClampShadeNode::tClampShadeNode( const wxPoint& p )
		: tShadeNode( "Clamp", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Value" );
		fAddInput( "Min" );
		fAddInput( "Max" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tClampShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string value;
		u32 resultDimension = 1;

		if( inputResults[ 0 ] )
		{
			value = inputResults[ 0 ]->fName( );
			resultDimension = inputResults[ 0 ]->fDimensionX( );
		}
		else
			value = "0.5";


		std::string min;
		if( inputResults[ 1 ] )
			min = inputResults[ 1 ]->fSwizzle( resultDimension );
		else
			min = "0.0";

		std::string max;
		if( inputResults[ 2 ] )
			max = inputResults[ 2 ]->fSwizzle( resultDimension );
		else
			max = "1.0";

		tHlslVariableConstPtr output = writer.fMakeTempVector( "clampResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = clamp( " << value << ", " << min << ", " << max << " );" << std::endl;
	}

	register_rtti_factory( tThresholdShadeNode, false );
	tThresholdShadeNode::tThresholdShadeNode( const wxPoint& p )
		: tShadeNode( "Threshold", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Value" );
		fAddInput( "Threshold" );
		fComputeDimensions( );
		fAddOutput( );
	}
	void tThresholdShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		std::string value;
		u32 resultDimension = 1;

		if( inputResults[ 0 ] )
		{
			value = inputResults[ 0 ]->fName( );
			resultDimension = inputResults[ 0 ]->fDimensionX( );
		}
		else
			value = "0.5";


		std::string threshold;
		if( inputResults[ 1 ] )
			threshold = inputResults[ 1 ]->fSwizzle( resultDimension );
		else
			threshold = "0.5";

		tHlslVariableConstPtr output = writer.fMakeTempVector( "thresholdResult", resultDimension );
		hlslGenTree.fCacheShadeNodeOutput( output );

		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		writer.fBeginLine( ) << output->fDeclaration( ) << " = " << value << " > " << threshold << " ? 1.0 : 0.0;" << std::endl;
	}


	namespace { enum tPulseIndices { cPulseSpeed, cPulseOffset, cPulseMin, cPulseMax }; }
	register_rtti_factory( tPulseShadeNode, false );
	tPulseShadeNode::tPulseShadeNode( const wxPoint& p )
		: tShadeNode( "Pulse", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Speed" );
		fAddInput( "Offset" );
		fAddInput( "Min" );
		fAddInput( "Max" );
		fComputeDimensions( );
		fAddOutput( );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cPropNameStopOnPause, true ) ) );
	}
	void tPulseShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalTime, writer.fInput( ).mPid ) );
	}
	void tPulseShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		const b32 remapRange = ( inputResults[ cPulseMin ] || inputResults[ cPulseMax ] );
		const b32 stopOnPause = fShadeProps( ).fGetValue( cPropNameStopOnPause, true );

		tHlslVariableConstPtr time = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalTime, writer.fInput( ).mPid );
		tHlslVariableConstPtr timeScale = inputResults[ cPulseSpeed ];
		tHlslVariableConstPtr timeOffset = inputResults[ cPulseOffset ];
		tHlslVariableConstPtr pulsedTime = writer.fMakeTempVector( "pulsedTime", 1 );

		writer.fBeginLine( true ) << "// pulse time" << std::endl;
		writer.fBeginLine( ) << pulsedTime->fDeclaration( ) << " = ";

		std::string timeExpression = time->fSwizzle( stopOnPause ? "y" : "x" );
		if( timeOffset )
			timeExpression += " + " + timeOffset->fSwizzle( 1 );

		if( timeScale )
			writer.fContinueLine( ) << "abs( sin( " << timeScale->fSwizzle( 1 ) << " * " << timeExpression << " ) );" << std::endl;
		else
			writer.fContinueLine( ) << "abs( sin( " << timeExpression << " ) );" << std::endl;

		if( remapRange )
		{
			std::string min,max;
			if( inputResults[ cPulseMin ] ) min = inputResults[ cPulseMin ]->fSwizzle( 1 );
			else					min = "0.0";
			if( inputResults[ cPulseMax ] ) max = inputResults[ cPulseMax ]->fSwizzle( 1 );
			else					max = "1.0";

			tHlslVariableConstPtr remappedPulsedTime = writer.fMakeTempVector( "remappedPulsedTime", 1 );

			writer.fBeginLine( true ) << "// remap pulsed time to specified range" << std::endl;
			writer.fBeginLine( ) << remappedPulsedTime->fDeclaration( )
				<< " = lerp( " << min << ", " << max << ", " << pulsedTime->fName( ) << " );" << std::endl;

			hlslGenTree.fCacheShadeNodeOutput( remappedPulsedTime );
		}
		else
			hlslGenTree.fCacheShadeNodeOutput( pulsedTime );
	}

	namespace { enum tScrollIndices { cScrollSpeed, cScrollOffset }; }
	register_rtti_factory( tScrollShadeNode, false );
	tScrollShadeNode::tScrollShadeNode( const wxPoint& p )
		: tShadeNode( "Scroll", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Speed" );
		fAddInput( "Offset" );
		fComputeDimensions( );
		fAddOutput( );
		fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cPropNameStopOnPause, true ) ) );
	}
	void tScrollShadeNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalTime, writer.fInput( ).mPid ) );
	}
	void tScrollShadeNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		const b32 stopOnPause = fShadeProps( ).fGetValue( cPropNameStopOnPause, true );

		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );

		tHlslVariableConstPtr time = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalTime, writer.fInput( ).mPid );
		tHlslVariableConstPtr timeScale = inputResults[ cScrollSpeed ];
		tHlslVariableConstPtr timeOffset = inputResults[ cScrollOffset ];
		tHlslVariableConstPtr scrolledTime = writer.fMakeTempVector( "scrolledTime", 1 );
		hlslGenTree.fCacheShadeNodeOutput( scrolledTime );

		writer.fBeginLine( true ) << "// scroll time" << std::endl;
		writer.fBeginLine( ) << scrolledTime->fDeclaration( ) << " = ";

		std::string timeExpression = time->fSwizzle( stopOnPause ? "y" : "x" );
		if( timeOffset )
			timeExpression += " + " + timeOffset->fSwizzle( 1 );

		if( timeScale )
			writer.fContinueLine( ) << timeScale->fSwizzle( 1 ) << " * " << timeExpression << ";" << std::endl;
		else
			writer.fContinueLine( ) << timeExpression << ";" << std::endl;
	}

	//------------------------------------------------------------------------------
	// tFracShadeNode
	//------------------------------------------------------------------------------
	register_rtti_factory( tFracShadeNode, false );
	tFracShadeNode::tFracShadeNode( const wxPoint& p )
		: tShadeNode( "Frac", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Value" );
		fComputeDimensions( );
		fAddOutput( );
	}

	//------------------------------------------------------------------------------
	void tFracShadeNode::fWriteHlsl( 
		HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );
		const tHlslVariableConstPtr & input = inputResults[ 0 ];

		tHlslVariableConstPtr result;
		if( inputResults.fCount( ) && inputResults[ 0 ] )
		{	
			result = writer.fMakeTempVector( "fracResult", input->fDimensionX( ) );
			writer.fBeginLine( ) << result->fDeclaration( ) << " = frac( " << input->fName( ) << " );" << std::endl;
		}
		else
		{
			result = writer.fMakeTempVector( "fracResult" );
			writer.fBeginLine( ) << result->fDeclaration( ) << " = float4( 0 );" << std::endl;
		}

		hlslGenTree.fCacheShadeNodeOutput( result );
	}

	//------------------------------------------------------------------------------
	// tTruncShadeNode
	//------------------------------------------------------------------------------	
	register_rtti_factory( tTruncShadeNode, false );
	tTruncShadeNode::tTruncShadeNode( const wxPoint& p )
		: tShadeNode( "Trunc", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Value" );
		fComputeDimensions( );
		fAddOutput( );
	}

	//------------------------------------------------------------------------------
	void tTruncShadeNode::fWriteHlsl( 
		HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		hlslGenTree.fResolveInputResults( writer, reqs );
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );
		const tHlslVariableConstPtr & input = inputResults[ 0 ];

		tHlslVariableConstPtr result;
		if( input )
		{
			result = writer.fMakeTempVector( "truncResult", input->fDimensionX( ) );
			writer.fBeginLine( ) << result->fDeclaration( ) << " = trunc( " << input->fName( ) << " );" << std::endl;
		}
		else
		{
			result = writer.fMakeTempVector( "truncResult" );
			writer.fBeginLine( ) << result->fDeclaration( ) << " = float4( 0 );" << std::endl;
		}

		hlslGenTree.fCacheShadeNodeOutput( result );
	}

	//------------------------------------------------------------------------------
	// tTransitionObjectsNode
	//------------------------------------------------------------------------------
	register_rtti_factory( tTransitionObjectsNode, false );
	tTransitionObjectsNode::tTransitionObjectsNode( const wxPoint& p )
		: tShadeNode( "TransitionObjects", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fComputeDimensions( );
		fAddOutput( );
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectsNode::fAddHlslRequirements( 
		HlslGen::tHlslWriter& writer, 
		HlslGen::tShaderRequirements& reqs, 
		HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		// World space pos
		{
			reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInWorldSpacePos, writer.fInput( ).mPid ) );
		}

		// Time
		{
			reqs.mGlobals.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalTime, writer.fInput( ).mPid ) );
		}

		// Default Id
		{
			const char * cDefaultIdName = Gfx::tTransitionObjectMaterialHelper::cDefaultIdName;

			tHlslVariableConstPtr defaultId( tHlslVariable::fMakeGlobalVector( cDefaultIdName, reqs.mNextConstantRegister++ ) );
			hlslGenTree.fCacheShadeNodeReq( defaultId, 0 );
			reqs.mGlobals.fFindOrAdd( defaultId );
		}

		// Edge Colors
		{
			const u32 cNumEdgeColors = Gfx::tTransitionObjectMaterialHelper::cMaxAlignments;
			const char * cEdgeColorName = Gfx::tTransitionObjectMaterialHelper::cEdgeColorName;

			tHlslVariableConstPtr edgeColors( tHlslVariable::fMakeGlobalVector( 
				cEdgeColorName, reqs.mNextConstantRegister, 4, cNumEdgeColors ) );
			reqs.mNextConstantRegister += cNumEdgeColors;

			hlslGenTree.fCacheShadeNodeReq( edgeColors, 1 );
			reqs.mGlobals.fFindOrAdd( edgeColors );
		}

		// Objects
		{
			const u32 cNumObjects = Gfx::tTransitionObjectMaterialHelper::cMaxTransitionObjects;
			const char * cObjectsName = Gfx::tTransitionObjectMaterialHelper::cObjectsName;

			tHlslVariableConstPtr objects( tHlslVariable::fMakeGlobalVector(
				cObjectsName, reqs.mNextConstantRegister, 4, cNumObjects ) );
			reqs.mNextConstantRegister += cNumObjects;

			hlslGenTree.fCacheShadeNodeReq( objects, 2 );
			reqs.mGlobals.fFindOrAdd( objects );
		}
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectsNode::fWriteHlsl( 
		HlslGen::tHlslWriter& writer, 
		const HlslGen::tShaderRequirements& reqs, 
		HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;
		const tHlslVariableConstPtr time = tPixelShaderVars::fVariable( tPixelShaderVars::cGlobalTime, writer.fInput( ).mPid );
		const tHlslVariableConstPtr worldSpacePos = tPixelShaderVars::fVariable( tPixelShaderVars::cInWorldSpacePos, writer.fInput( ).mPid );
		const tHlslVariableConstPtr& defaultId = hlslGenTree.fShadeNodeReq( 0 );
		const tHlslVariableConstPtr& edgeColors = hlslGenTree.fShadeNodeReq( 1 );
		const tHlslVariableConstPtr& objects = hlslGenTree.fShadeNodeReq( 2 );

		// Material glue setup
		{
			writer.fAddMaterialGlueForTransitionObjects(
				defaultId->fSemantic( ).mRegisterIndex,
				edgeColors->fSemantic( ).mRegisterIndex,
				objects->fSemantic( ).mRegisterIndex );
		}

		Gfx::tTransitionObjectMaterialHelper::fAddFunctionDefinition( 
			writer.fFunctions( ),
			time->fName( ).c_str( ),
			defaultId->fName( ).c_str( ),
			edgeColors->fName( ).c_str( ),
			objects->fName( ).c_str( ) );

		tHlslVariableConstPtr alignInfo = writer.fMakeTempVector( "alignInfo" );

		std::stringstream & ss = writer.fBeginLine( );
		ss << alignInfo->fDeclaration( ) << " = ";
		Gfx::tTransitionObjectMaterialHelper::fAddFunctionCall( ss, worldSpacePos->fSwizzle( 3 ).c_str( ) );

		hlslGenTree.fCacheShadeNodeOutput( alignInfo );
	}

	//------------------------------------------------------------------------------
	// tSplitPlaneNode
	//------------------------------------------------------------------------------
	register_rtti_factory( tSplitPlaneNode, false );
	tSplitPlaneNode::tSplitPlaneNode( const wxPoint& p )
		: tShadeNode( "SplitPlane", cUtilityBarColor, cUtilityTextColor, false, p )
	{
		fAddInput( "Plane" );
		fComputeDimensions( );
		fAddOutput( );
	}

	//------------------------------------------------------------------------------
	void tSplitPlaneNode::fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		// World space pos
		{
			reqs.mInputs.fFindOrAdd( tPixelShaderVars::fVariable( tPixelShaderVars::cInWorldSpacePos, writer.fInput( ).mPid ) );
		}
	}

	//------------------------------------------------------------------------------
	void tSplitPlaneNode::fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree )
	{
		using namespace HlslGen;

		hlslGenTree.fResolveInputResults( writer, reqs );
		
		const tHlslGenTree::tInputResultArray& inputResults = hlslGenTree.fInputResults( );
		const tHlslVariableConstPtr & input = inputResults[ 0 ];
		const tHlslVariableConstPtr worldSpacePos = tPixelShaderVars::fVariable( tPixelShaderVars::cInWorldSpacePos, writer.fInput( ).mPid );


		writer.fBeginLine( true ) << "// write result from " << fMatEdDisplayName( ) << std::endl;
		tHlslVariableConstPtr result = writer.fMakeTempVector( "splitPlaneResult", 1 );
		if( input )
			writer.fBeginLine( ) << result->fDeclaration( ) << " = step( -" << input->fSwizzle( "w" ) << ", dot( " << input->fSwizzle( 3 ) << ", " << worldSpacePos->fSwizzle( 3 ) <<  " ) );" << std::endl;
		else
			writer.fBeginLine( ) << result->fDeclaration( ) << " = 0;" << std::endl;

		hlslGenTree.fCacheShadeNodeOutput( result );
	}

}

