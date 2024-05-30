#include "ToolsPch.hpp"
#include "tHlslWriter.hpp"
#include "tPixelShaderVars.hpp"
#include "tVertexShaderVars.hpp"
#include "tMaterialGenBase.hpp"
#include "Gfx/DerivedShadeMaterialGlue.hpp"

namespace Sig { namespace HlslGen
{
	namespace
	{
		static const char* cTabs[] = 
		{
			"",
			"\t",
			"\t\t",
			"\t\t\t",
			"\t\t\t\t",
			"\t\t\t\t\t",
			"\t\t\t\t\t\t",
		};
		static const u32 cTabMax = array_length( cTabs );
		static inline const char* fTab( u32 tabCount ) { return cTabs[ fMin( tabCount, cTabMax-1 ) ]; }
	}

	tHlslWriter::tHlslWriter( const tGrowableArray<tHlslGenTreePtr>& shadeTreeRoots )
		: mInput( 0 )
		, mOutput( 0 )
		, mCurrentPixelShaderOutput( 0 )
		, mShadeTreeRoots( shadeTreeRoots )
		, mTabLevel( 0 )
		, mTempIndex( 0 )
		, mWriteMode( cWriteModeColor )
	{
	}

	void tHlslWriter::fGenerateShaders( const tHlslInput& input, tHlslOutput& output )
	{
		mInput = &input;
		mOutput = &output;
		output = tHlslOutput( ); // clear output

		tShaderRequirements psReqs;

		//
		// write color shaders...
		//
		mWriteMode = cWriteModeColor;
		psReqs = tShaderRequirements( );
		tHlslGenTree::fDeterminePixelShaderRequirements( mShadeTreeRoots, *this, psReqs );
		output = tHlslOutput( psReqs.mNumLights + 1 );

		fWriteVertexShader( psReqs, 0, output.mStaticVShaders[ mWriteMode ] );
		fWriteVertexShader( psReqs, 4, output.mSkinnedVShaders[ mWriteMode ] );
		for( u32 i = 0; i <= psReqs.mNumLights; ++i )
			fWritePixelShader( psReqs, i, output.mColorPShaders[ i ] );

		if( input.mGenDepthShaders )
		{
			//
			// write depth-only shaders
			//
			mWriteMode = cWriteModeDepth;
			psReqs = tShaderRequirements( );
			tHlslGenTree::fDeterminePixelShaderRequirements( mShadeTreeRoots, *this, psReqs );

			fWriteVertexShader( psReqs, 0, output.mStaticVShaders[ mWriteMode ] );
			fWriteVertexShader( psReqs, 4, output.mSkinnedVShaders[ mWriteMode ] );
			fWritePixelShader( psReqs, 0, output.mDepthOnlyPShader );

			//
			// write depth-with-alph shaders
			//
			mWriteMode = cWriteModeDepthWithAlpha;
			psReqs = tShaderRequirements( );
			tHlslGenTree::fDeterminePixelShaderRequirements( mShadeTreeRoots, *this, psReqs );
			if( true ) // if( we even need depth with alpha ) // TODO?
			{
				fWriteVertexShader( psReqs, 0, output.mStaticVShaders[ mWriteMode ] );
				fWriteVertexShader( psReqs, 4, output.mSkinnedVShaders[ mWriteMode ] );
				fWritePixelShader( psReqs, 0, output.mDepthWithAlphaPShader );
			}
		}

		mInput = 0;
		mOutput = 0;
	}

	u32 tHlslWriter::fAddMaterialGlueVector( u32 registerIndex, s32 glueIndex )
	{
		sigassert( mOutput && mCurrentPixelShaderOutput );
		u32 o = 0;
		if( glueIndex >= 0 )	o = glueIndex;
		else					o = mOutput->mMaterialGlueValues.fAddVector( );
		mCurrentPixelShaderOutput->fAddMaterialGlue( Gfx::tShadeMaterialGluePtr( new Gfx::tPsGlueMaterialVector( registerIndex, o ) ) );
		return o;
	}

	u32 tHlslWriter::fAddMaterialGlueSampler( u32 registerIndex, s32 glueIndex )
	{
		sigassert( mOutput && mCurrentPixelShaderOutput );
		u32 o = 0;
		if( glueIndex >= 0 )	o = glueIndex;
		else					o = mOutput->mMaterialGlueValues.fAddSampler( );
		mCurrentPixelShaderOutput->fAddMaterialGlue( Gfx::tShadeMaterialGluePtr( new Gfx::tPsGlueMaterialSampler( registerIndex, o ) ) );
		return o;
	}

	u32 tHlslWriter::fAddMaterialGlueString( u32 registerIndex, s32 glueIndex )
	{
		sigassert( mOutput && mCurrentPixelShaderOutput );
		u32 o = 0;
		if( glueIndex >= 0 )	o = glueIndex;
		else					o = mOutput->mMaterialGlueValues.fAddString( );
		mCurrentPixelShaderOutput->fAddMaterialGlue( Gfx::tShadeMaterialGluePtr( new Gfx::tPsGlueDynamicVec4( registerIndex, o ) ) );
		return o;
	}

	tHlslVariableConstPtr tHlslWriter::fMakeTempVector( const std::string& name, u32 dimensionX, u32 arrayCount )
	{
		return tHlslVariableConstPtr( tHlslVariable::fMakeTempVector( name, mTempIndex++, dimensionX, arrayCount ) );
	}

	std::stringstream& tHlslWriter::fBeginLine( b32 extraLineBreak )
	{
		if( extraLineBreak )
			mSs << std::endl;
		mSs << fTab( mTabLevel );
		return mSs;
	}

	std::stringstream& tHlslWriter::fContinueLine( )
	{
		return mSs;
	}

	void tHlslWriter::fComputeVertexShaderRequirements_MeshModel( const tShaderRequirements& psReqs, tShaderRequirements& vsReqs, u32 numBoneWeights )
	{
		// we always require a model space position as input
		vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpacePos, fInput( ).mPid ) );
		vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutProjSpacePos, fInput( ).mPid ) );

		// requirements for position
		vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalModelPosToWorld, fInput( ).mPid ) );
		vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalWorldToProjection, fInput( ).mPid ) );

		// add ouput depth if required for pixel shader
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInProjSpaceDepth, fInput( ).mPid ) ) )
		{
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutProjSpaceDepth, fInput( ).mPid ) );
		}

		// add requirements for lit-space position if necessary
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, fInput( ).mPid ) ) )
		{
			// globals
			vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalWorldEyePos, fInput( ).mPid ) );

			// outputs
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpacePos, fInput( ).mPid ) );
		}

		// add requirements for lit-space normal/tangent if necessary
		const b32 psLitSpaceNormal = psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceNormal ) ) != 0;
		const b32 psLitSpaceTangent = psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceTangent ) ) != 0;
		if( psLitSpaceNormal || psLitSpaceTangent )
		{
			// globals
			vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalModelNormalToWorld, fInput( ).mPid ) );

			// inputs/outputs
			if( psLitSpaceNormal )
			{
				vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpaceNormal, fInput( ).mPid ) );
				vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpaceNormal, fInput( ).mPid ) );
			}
			if( psLitSpaceTangent )
			{
				if( fInput( ).mPreviewMode )
					vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpaceTangent3, fInput( ).mPid ) );
				else
					vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpaceTangent, fInput( ).mPid ) );
				vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpaceTangent, fInput( ).mPid ) );
			}
		}

		// vertex color
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInVertexColor, fInput( ).mPid ) ) )
		{
			// inputs
			vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInVertexColor, fInput( ).mPid ) );
			// outputs
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutVertexColor, fInput( ).mPid ) );
		}

		// texcoords
		for( u32 i = 0; i < 4; ++i )
		{
			if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( ( tPixelShaderVars::tVarId )( tPixelShaderVars::cInUv0 + i ), fInput( ).mPid ) ) )
			{
				// inputs
				vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( ( tVertexShaderVars::tVarId )( tVertexShaderVars::cInUv0 + i ), fInput( ).mPid ) );
				// outputs
				vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( ( tVertexShaderVars::tVarId )( tVertexShaderVars::cOutUv0 + i ), fInput( ).mPid ) );
			}
		}
		for( u32 i = 0; i < 2; ++i )
		{
			if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( ( tPixelShaderVars::tVarId )( tPixelShaderVars::cInUv01 + i ), fInput( ).mPid ) ) )
			{
				if( fInput( ).mPreviewMode )
				{
					// inputs
					vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( ( tVertexShaderVars::tVarId )( tVertexShaderVars::cInUv0 + 2*i ), fInput( ).mPid ) );
					vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( ( tVertexShaderVars::tVarId )( tVertexShaderVars::cInUv1 + 2*i ), fInput( ).mPid ) );
				}
				else
				{
					// inputs
					vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( ( tVertexShaderVars::tVarId )( tVertexShaderVars::cInUv01 + i ), fInput( ).mPid ) );
				}
				// outputs
				vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( ( tVertexShaderVars::tVarId )( tVertexShaderVars::cOutUv01 +i ), fInput( ).mPid ) );
			}
		}

		// add requirements for light position (for shadow mapping) if necessary
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInLightPos, fInput( ).mPid ) ) )
		{
			// globals
			vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalWorldToLight, fInput( ).mPid ) );

			// outputs
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutLightPos, fInput( ).mPid ) );
		}

		// add skinning requirements if necessary
		if( numBoneWeights > 0 )
		{
			// globals
			vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalSkinningPalette, fInput( ).mPid ) );

			// inputs to the vertex shader
			vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInBlendWeights, fInput( ).mPid ) );
			vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInBlendIndices, fInput( ).mPid ) );
		}
	}

	void tHlslWriter::fComputeVertexShaderRequirements_FacingQuads( const tShaderRequirements& psReqs, tShaderRequirements& vsReqs )
	{
		// for particles (facing quads), we need to maintain a consistent vertex format for quick updating on the cpu (i.e., we need
		// to always assume a specific vertex layout), so we always add these inputs:
		vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpacePos, fInput( ).mPid ) );
		vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInVertexColor, fInput( ).mPid ) );
		vsReqs.mInputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv01, fInput( ).mPid ) );

		// add position output (projection space)
		vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutProjSpacePos, fInput( ).mPid ) );

		// requirements for position
		vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalModelToView, fInput( ).mPid ) );
		vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalViewToProjection, fInput( ).mPid ) );

		// vertex color
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInVertexColor, fInput( ).mPid ) ) )
		{
			// outputs
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutVertexColor, fInput( ).mPid ) );
		}

		// uv 0/1
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInUv01, fInput( ).mPid ) ) )
		{
			// outputs
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutUv01, fInput( ).mPid ) );
		}
		// uv 2/3
		sigassert( !psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInUv23, fInput( ).mPid ) ) );

		// add ouput depth if required for pixel shader
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInProjSpaceDepth, fInput( ).mPid ) ) )
		{
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutProjSpaceDepth, fInput( ).mPid ) );
		}

		// add requirements for lit-space position if necessary
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, fInput( ).mPid ) ) )
		{
			// globals
			vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalViewToWorld, fInput( ).mPid ) );

			// outputs
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpacePos, fInput( ).mPid ) );
		}

		// add requirements for lit-space normal/tangent if necessary
		const b32 psLitSpaceNormal = psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceNormal ) ) != 0;
		const b32 psLitSpaceTangent = psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceTangent ) ) != 0;
		if( psLitSpaceNormal || psLitSpaceTangent )
		{
			// globals
			vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalViewToWorld, fInput( ).mPid ) );

			// outputs
			if( psLitSpaceNormal )
				vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpaceNormal, fInput( ).mPid ) );
			if( psLitSpaceTangent )
				vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpaceTangent, fInput( ).mPid ) );
		}

		// add requirements for light position (for shadow mapping) if necessary
		if( psReqs.mInputs.fFind( tPixelShaderVars::fVariable( tPixelShaderVars::cInLightPos, fInput( ).mPid ) ) )
		{
			// globals
			vsReqs.mGlobals.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalViewToLight, fInput( ).mPid ) );

			// outputs
			vsReqs.mOutputs.fFindOrAdd( tVertexShaderVars::fVariable( tVertexShaderVars::cOutLightPos, fInput( ).mPid ) );
		}
	}

	void tHlslWriter::fWriteVertexShader( const tShaderRequirements& psReqs, u32 numBoneWeights, tVertexShaderOutput& output )
	{
		if( numBoneWeights > 0 && 
			fInput( ).mVshStyle == cVshFacingQuads )
		{
			// facing quads doesn't support bone weights
			return;
		}

		// given pixel shader requirements, compute vertex shader requirements
		tShaderRequirements vsReqs;

		switch( fInput( ).mVshStyle )
		{
		case cVshMeshModel:		fComputeVertexShaderRequirements_MeshModel( psReqs, vsReqs, numBoneWeights ); break;
		case cVshFacingQuads:	fComputeVertexShaderRequirements_FacingQuads( psReqs, vsReqs ); break;
		default: sigassert( !"unrecognized vertex shader style passed to tHlslWriter::fWriteVertexShader" ); break;
		}

		vsReqs.fSortGlobals( );
		vsReqs.fComputeMaterialGlue( output );

		fBeginShaderMain( vsReqs );

		switch( fInput( ).mVshStyle )
		{
		case cVshMeshModel:		fVertexShaderMain_MeshModel( vsReqs, numBoneWeights ); break;
		case cVshFacingQuads:	fVertexShaderMain_FacingQuads( vsReqs ); break;
		default: sigassert( !"unrecognized vertex shader style passed to tHlslWriter::fWriteVertexShader" ); break;
		}

		fEndShaderMain( vsReqs );

		Gfx::tVertexFormat vtxFormat;
		vsReqs.fComputeVertexFormat( vtxFormat );
		output.mVtxFormat.fReset( new Gfx::tVertexFormatVRam( vtxFormat ) );
		output.mHlsl = mSs.str( );
	}

	void tHlslWriter::fWritePixelShader( const tShaderRequirements& psReqsOG, u32 numLights, tPixelShaderOutput& output )
	{
		mCurrentPixelShaderOutput = &output;

		tShaderRequirements psReqs = psReqsOG;
		psReqs.mNumLights = numLights;
		psReqs.fComputeMaterialGlue( output );

		fBeginShaderMain( psReqs );

		fPixelShaderMain( psReqs, numLights );

		fEndShaderMain( psReqs );

		output.mHlsl = mSs.str( );

		mCurrentPixelShaderOutput = 0;
	}

	void tHlslWriter::fBeginShaderMain( const tShaderRequirements& reqs )
	{
		mSs.str( std::string( ) ); // clear stream
		mTabLevel = 0;
		mTempIndex = 0;
		mDeclaredUserTypes.fSetCount( 0 );

		fBeginLine( true ) << "#pragma pack_matrix( row_major )" << std::endl;

		for( u32 i = 0; i < reqs.mGlobals.fCount( ); ++i )
			reqs.mGlobals[ i ]->fDeclareType( *this );

		for( u32 i = 0; i < reqs.mGlobals.fCount( ); ++i )
			fBeginLine( i == 0 ) << reqs.mGlobals[ i ]->fDeclaration( ) << ";" << std::endl;

		// TODO write functions?

		fBeginLine( true ) << "void main( " << std::endl;

		fPushTab( );

		for( u32 i = 0; i < reqs.mInputs.fCount( ); ++i )
			fBeginLine( ) << reqs.mInputs[ i ]->fDeclaration( ) << "," << std::endl;

		const u32 lastOutput = reqs.mOutputs.fCount( ) - 1;
		for( u32 i = 0; i < reqs.mOutputs.fCount( ); ++i )
			fBeginLine( ) << reqs.mOutputs[ i ]->fDeclaration( ) << ( ( i == lastOutput ) ? " )" : "," ) << std::endl;

		fPopTab( );

		fBeginLine( ) << "{"/* << std::endl*/;

		fPushTab( );
	}

	void tHlslWriter::fEndShaderMain( const tShaderRequirements& reqs )
	{
		fPopTab( );

		mSs << "}" << std::endl << std::endl;
	}

	void tHlslWriter::fVertexShaderMain_MeshModel( const tShaderRequirements& reqs, u32 numBoneWeights )
	{
		const b32 skinned = numBoneWeights > 0;

		// cache variables we'll be using
		tHlslVariableConstPtr modelSpacePos = tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpacePos, fInput( ).mPid );
		tHlslVariableConstPtr projSpacePos = tVertexShaderVars::fVariable( tVertexShaderVars::cOutProjSpacePos, fInput( ).mPid );
		tHlslVariableConstPtr skinPalette = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalSkinningPalette, fInput( ).mPid );
		tHlslVariableConstPtr blendIndices = tVertexShaderVars::fVariable( tVertexShaderVars::cInBlendIndices, fInput( ).mPid );
		tHlslVariableConstPtr blendWeights = tVertexShaderVars::fVariable( tVertexShaderVars::cInBlendWeights, fInput( ).mPid );
		tHlslVariableConstPtr modelPosToWorld = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalModelPosToWorld, fInput( ).mPid );
		tHlslVariableConstPtr modelNormalToWorld = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalModelNormalToWorld, fInput( ).mPid );
		tHlslVariableConstPtr worldToProj = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalWorldToProjection, fInput( ).mPid );
		tHlslVariableConstPtr worldEyePos = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalWorldEyePos, fInput( ).mPid );

		// compute skinned position (which is just the model space pos if not skinned)
		tHlslVariableConstPtr skinMatrix;
		tHlslVariableConstPtr skinnedPos;
		if( skinned )
		{
			skinMatrix.fReset( tHlslVariable::fMakeMatrixFloat( "tempSkinMatrix", tHlslVariable::cTemp, 4, 3 ) );

			fBeginLine( true ) << "// compute skinning matrix" << std::endl;
			fBeginLine( ) << skinMatrix->fDeclaration( ) 
				<< " = " << skinPalette->fIndexArray( blendIndices->fSwizzle( "x" ) ) << " * " << blendWeights->fSwizzle( "x" ) << ";" << std::endl;
			fBeginLine( ) << skinMatrix->fName( ) 
				<< " += " << skinPalette->fIndexArray( blendIndices->fSwizzle( "y" ) ) << " * " << blendWeights->fSwizzle( "y" ) << ";" << std::endl;
			fBeginLine( ) << skinMatrix->fName( ) 
				<< " += " << skinPalette->fIndexArray( blendIndices->fSwizzle( "z" ) ) << " * " << blendWeights->fSwizzle( "z" ) << ";" << std::endl;
			fBeginLine( ) << skinMatrix->fName( ) 
				<< " += " << skinPalette->fIndexArray( blendIndices->fSwizzle( "w" ) ) << " * " << blendWeights->fSwizzle( "w" ) << ";" << std::endl;

			skinnedPos.fReset( tHlslVariable::fMakeVectorFloat( "tempSkinnedPos", tHlslVariable::cTemp, 3 ) );
			fBeginLine( true ) << "// compute skinned position" << std::endl;
			fBeginLine( ) << skinnedPos->fDeclaration( )
				<< " = mul( " << skinMatrix->fName( ) << ", float4( " << modelSpacePos->fSwizzle( "xyz" ) << ", 1.f ) );" << std::endl;
		}
		else
			skinnedPos = modelSpacePos;

		// declare/compute world position temporary variable
		fBeginLine( true ) << "// compute world space position" << std::endl;
		tHlslVariableConstPtr worldPos( tHlslVariable::fMakeVectorFloat( "worldPos", tHlslVariable::cTemp, 3 ) );
		fBeginLine( ) << worldPos->fDeclaration( )
			<< " = mul( " << modelPosToWorld->fName( ) << ", float4( " << skinnedPos->fSwizzle( "xyz" ) << ", 1.f ) );" << std::endl;

		// output projection space pos
		fBeginLine( true ) << "// output projection space position" << std::endl;
		fBeginLine( ) << projSpacePos->fName( ) 
			<< " = mul( " << worldToProj->fName( ) << ", float4( " << worldPos->fSwizzle( "xyz" ) << ", 1.f ) );" << std::endl;

		//
		// now we handle outputting optional parameters (normals, tangents, colors, uvs, etc)...
		//

		tHlslVariableConstPtr projSpaceDepth = tVertexShaderVars::fVariable( tVertexShaderVars::cOutProjSpaceDepth, fInput( ).mPid );
		tHlslVariableConstPtr litSpacePos = tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpacePos, fInput( ).mPid );
		tHlslVariableConstPtr litSpaceNormal = tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpaceNormal, fInput( ).mPid );
		tHlslVariableConstPtr litSpaceTangent = tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpaceTangent, fInput( ).mPid );
		tHlslVariableConstPtr vertexColor = tVertexShaderVars::fVariable( tVertexShaderVars::cOutVertexColor, fInput( ).mPid );
		tHlslVariableConstPtr uv0 = tVertexShaderVars::fVariable( tVertexShaderVars::cOutUv0, fInput( ).mPid );
		tHlslVariableConstPtr uv1 = tVertexShaderVars::fVariable( tVertexShaderVars::cOutUv1, fInput( ).mPid );
		tHlslVariableConstPtr uv2 = tVertexShaderVars::fVariable( tVertexShaderVars::cOutUv2, fInput( ).mPid );
		tHlslVariableConstPtr uv3 = tVertexShaderVars::fVariable( tVertexShaderVars::cOutUv3, fInput( ).mPid );
		tHlslVariableConstPtr uv01 = tVertexShaderVars::fVariable( tVertexShaderVars::cOutUv01, fInput( ).mPid );
		tHlslVariableConstPtr uv23 = tVertexShaderVars::fVariable( tVertexShaderVars::cOutUv23, fInput( ).mPid );
		tHlslVariableConstPtr lightPos = tVertexShaderVars::fVariable( tVertexShaderVars::cOutLightPos, fInput( ).mPid );

		// proj-space depth
		if( reqs.mOutputs.fFind( projSpaceDepth ) )
		{
			fBeginLine( true ) << "// output projection space depth" << std::endl;
			fBeginLine( ) << projSpaceDepth->fName( )
				<< " = " << projSpacePos->fSwizzle( "zw" ) << ";" << std::endl;
		}

		// lit-space pos
		if( reqs.mOutputs.fFind( litSpacePos ) )
		{
			fBeginLine( true ) << "// output position to lit space" << std::endl;

			sigassert( reqs.mGlobals.fFind( worldEyePos ) );
			fBeginLine( ) << litSpacePos->fName( )
				<< " = " << worldPos->fSwizzle( "xyz" ) << " - " << worldEyePos->fSwizzle( "xyz" ) << ";" << std::endl;
		}

		// lit-space normal
		if( reqs.mOutputs.fFind( litSpaceNormal ) )
		{
			fBeginLine( true ) << "// output normal to lit space" << std::endl;

			tHlslVariableConstPtr modelSpaceNormal = tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpaceNormal, fInput( ).mPid );
			sigassert( reqs.mInputs.fFind( modelSpaceNormal ) );

			tHlslVariableConstPtr inputNormal;
			if( skinMatrix )
			{
				fBeginLine( ) << litSpaceNormal->fName( )
					<< " = mul( ( float3x3 )" << skinMatrix->fName( ) << ", " << modelSpaceNormal->fSwizzle( "xyz" ) << " );" << std::endl;
				inputNormal = litSpaceNormal;
			}
			else
				inputNormal = modelSpaceNormal;

			fBeginLine( ) << litSpaceNormal->fName( )
				<< " = mul( " << modelNormalToWorld->fName( ) << ", " << inputNormal->fSwizzle( "xyz" ) << " );" << std::endl;
		}

		// lit-space tangent
		if( reqs.mOutputs.fFind( litSpaceTangent ) )
		{
			fBeginLine( true ) << "// output tangent to lit space" << std::endl;

			tHlslVariableConstPtr modelSpaceTangent;
			if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpaceTangent3, fInput( ).mPid ) ) )
				modelSpaceTangent = tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpaceTangent3, fInput( ).mPid );
			else
				modelSpaceTangent = tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpaceTangent, fInput( ).mPid );
			sigassert( reqs.mInputs.fFind( modelSpaceTangent ) );

			tHlslVariableConstPtr inputTangent;
			if( skinMatrix )
			{
				fBeginLine( ) << litSpaceTangent->fSwizzle( "xyz" )
					<< " = mul( ( float3x3 )" << skinMatrix->fName( ) << ", " << modelSpaceTangent->fSwizzle( "xyz" ) << " );" << std::endl;
				inputTangent = litSpaceTangent;
			}
			else
				inputTangent = modelSpaceTangent;

			fBeginLine( ) << litSpaceTangent->fSwizzle( "xyz" )
				<< " = mul( " << modelNormalToWorld->fName( ) << ", " << inputTangent->fSwizzle( "xyz" ) << " );" << std::endl;

			if( modelSpaceTangent->fDimensionX( ) == 4 )
				fBeginLine( ) << litSpaceTangent->fSwizzle( "w" ) << " = " << modelSpaceTangent->fSwizzle( "w" ) << ";" << std::endl;
			else
				fBeginLine( ) << litSpaceTangent->fSwizzle( "w" ) << " = 1.f;" << std::endl;
		}

		// vertex color
		if( reqs.mOutputs.fFind( vertexColor ) )
		{
			fBeginLine( true ) << "// pass through vertex color" << std::endl;
			fBeginLine( ) << vertexColor->fName( ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInVertexColor, fInput( ).mPid )->fName( ) << ";" << std::endl;
		}

		// uv sets 0 and/or 1
		if( reqs.mOutputs.fFind( uv0 ) )
		{
			fBeginLine( true ) << "// pass through UVs 0" << std::endl;
			if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv0, fInput( ).mPid ) ) )
				fBeginLine( ) << uv0->fSwizzle( "xy" ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv0, fInput( ).mPid )->fName( ) << ";" << std::endl;
		}
		if( reqs.mOutputs.fFind( uv1 ) )
		{
			fBeginLine( true ) << "// pass through UVs 1" << std::endl;
			if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv1, fInput( ).mPid ) ) )
				fBeginLine( ) << uv1->fSwizzle( "xy" ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv1, fInput( ).mPid )->fName( ) << ";" << std::endl;
		}
		if( reqs.mOutputs.fFind( uv01 ) )
		{
			fBeginLine( true ) << "// pass through UVs 0 and/or 1" << std::endl;
			if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv01, fInput( ).mPid ) ) )
				fBeginLine( ) << uv01->fName( ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv01, fInput( ).mPid )->fName( ) << ";" << std::endl;
			else
			{
				if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv0, fInput( ).mPid ) ) )
					fBeginLine( ) << uv01->fSwizzle( "xy" ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv0, fInput( ).mPid )->fName( ) << ";" << std::endl;
				if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv1, fInput( ).mPid ) ) )
					fBeginLine( ) << uv01->fSwizzle( "zw" ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv1, fInput( ).mPid )->fName( ) << ";" << std::endl;
			}
		}

		// uv sets 2 and/or 3
		if( reqs.mOutputs.fFind( uv2 ) )
		{
			fBeginLine( true ) << "// pass through UVs 2" << std::endl;
			if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv2, fInput( ).mPid ) ) )
				fBeginLine( ) << uv2->fSwizzle( "xy" ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv2, fInput( ).mPid )->fName( ) << ";" << std::endl;
		}
		if( reqs.mOutputs.fFind( uv3 ) )
		{
			fBeginLine( true ) << "// pass through UVs 3" << std::endl;
			if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv3, fInput( ).mPid ) ) )
				fBeginLine( ) << uv3->fSwizzle( "xy" ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv3, fInput( ).mPid )->fName( ) << ";" << std::endl;
		}
		if( reqs.mOutputs.fFind( uv23 ) )
		{
			fBeginLine( true ) << "// pass through UVs 2 and/or 3" << std::endl;
			if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv23, fInput( ).mPid ) ) )
				fBeginLine( ) << uv23->fName( ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv23, fInput( ).mPid )->fName( ) << ";" << std::endl;
			else
			{
				if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv2, fInput( ).mPid ) ) )
					fBeginLine( ) << uv23->fSwizzle( "xy" ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv2, fInput( ).mPid )->fName( ) << ";" << std::endl;
				if( reqs.mInputs.fFind( tVertexShaderVars::fVariable( tVertexShaderVars::cInUv3, fInput( ).mPid ) ) )
					fBeginLine( ) << uv23->fSwizzle( "zw" ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInUv3, fInput( ).mPid )->fName( ) << ";" << std::endl;
			}
		}

		// light position
		if( reqs.mOutputs.fFind( lightPos ) )
		{
			fBeginLine( true ) << "// light position for shadow mapping" << std::endl;

			const u32 shadowLayerCount = tMaterialGenBase::fShadowMapLayerCount( fInput( ).fRealPlatformId( ) );
			if( shadowLayerCount > 1 )
			{
				fBeginLine( ) << lightPos->fName( )
					<< " = float4( " << worldPos->fSwizzle( "xyz" ) << ", 1.f );" << std::endl;
			}
			else
			{
				tHlslVariableConstPtr worldToLight = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalWorldToLight, fInput( ).mPid );
				fBeginLine( ) << lightPos->fName( )
					<< " = mul( " << worldToLight->fName( ) << ", float4( " << worldPos->fSwizzle( "xyz" ) << ", 1.f ) );" << std::endl;
			}
		}
	}

	void tHlslWriter::fVertexShaderMain_FacingQuads( const tShaderRequirements& reqs )
	{
		// cache variables we'll be using
		tHlslVariableConstPtr modelSpacePos = tVertexShaderVars::fVariable( tVertexShaderVars::cInModelSpacePos, fInput( ).mPid );
		tHlslVariableConstPtr projSpacePos = tVertexShaderVars::fVariable( tVertexShaderVars::cOutProjSpacePos, fInput( ).mPid );
		tHlslVariableConstPtr modelToView = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalModelToView, fInput( ).mPid );
		tHlslVariableConstPtr viewToProj = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalViewToProjection, fInput( ).mPid );
		tHlslVariableConstPtr vertexColor = tVertexShaderVars::fVariable( tVertexShaderVars::cOutVertexColor, fInput( ).mPid );
		tHlslVariableConstPtr iUv01 = tVertexShaderVars::fVariable( tVertexShaderVars::cInUv01, fInput( ).mPid );
		tHlslVariableConstPtr oUv01 = tVertexShaderVars::fVariable( tVertexShaderVars::cOutUv01, fInput( ).mPid );

		// vertex color
		if( reqs.mOutputs.fFind( vertexColor ) )
		{
			fBeginLine( true ) << "// pass through vertex color" << std::endl;
			fBeginLine( ) << vertexColor->fName( ) << " = " << tVertexShaderVars::fVariable( tVertexShaderVars::cInVertexColor, fInput( ).mPid )->fName( ) << ";" << std::endl;
		}

		tHlslVariableConstPtr xSign( tHlslVariable::fMakeVectorFloat( "xSign", tHlslVariable::cTemp, 1 ) );
		tHlslVariableConstPtr ySign( tHlslVariable::fMakeVectorFloat( "ySign", tHlslVariable::cTemp, 1 ) );
		tHlslVariableConstPtr roll( tHlslVariable::fMakeVectorFloat( "roll", tHlslVariable::cTemp, 1 ) );

		fBeginLine( true ) << "// extract transform related values" << std::endl;
		fBeginLine( ) << xSign->fDeclaration( ) << " = sign( " << iUv01->fSwizzle( "x" ) << " );" << std::endl;
		fBeginLine( ) << ySign->fDeclaration( ) << " = sign( " << iUv01->fSwizzle( "y" ) << " );" << std::endl;
		fBeginLine( ) << roll->fDeclaration( ) << " =  " << iUv01->fSwizzle( "z" ) << ";" << std::endl;

		tHlslVariableConstPtr sinRoll( tHlslVariable::fMakeVectorFloat( "sinRoll", tHlslVariable::cTemp, 1 ) );
		tHlslVariableConstPtr cosRoll( tHlslVariable::fMakeVectorFloat( "cosRoll", tHlslVariable::cTemp, 1 ) );
		fBeginLine( ) << sinRoll->fDeclaration( ) << ";" << std::endl;
		fBeginLine( ) << cosRoll->fDeclaration( ) << ";" << std::endl;
		fBeginLine( ) << "sincos( " << roll->fName( ) << ", " << sinRoll->fName( ) << ", " << cosRoll->fName( ) << " );" << std::endl;

		if( reqs.mOutputs.fFind( oUv01 ) )
		{
			tHlslVariableConstPtr uv( tHlslVariable::fMakeVectorFloat( "uv", tHlslVariable::cTemp, 2 ) );
			fBeginLine( true ) << "// compute uv value and output" << std::endl;
			fBeginLine( ) << uv->fDeclaration( ) << " = float2( " << xSign->fName( ) << " * 0.5f + 0.5f, 1.0f - ( " << ySign->fName( ) << " * 0.5f + 0.5f ) );" << std::endl;
			fBeginLine( ) << oUv01->fName( ) << " = float4( " << uv->fName( ) << ", " << uv->fName( ) << " );" << std::endl;
		}


		tHlslVariableConstPtr viewSpacePos( tHlslVariable::fMakeVectorFloat( "viewSpacePos", tHlslVariable::cTemp, 3 ) );
		tHlslVariableConstPtr viewSpaceFacingPos( tHlslVariable::fMakeVectorFloat( "viewSpaceFacingPos", tHlslVariable::cTemp, 3 ) );
		tHlslVariableConstPtr cameraX( tHlslVariable::fMakeVectorFloat( "cameraX", tHlslVariable::cTemp, 3 ) );
		tHlslVariableConstPtr cameraY( tHlslVariable::fMakeVectorFloat( "cameraY", tHlslVariable::cTemp, 3 ) );
		fBeginLine( true ) << "// compute view space position of particle center" << std::endl;
		fBeginLine( ) << viewSpacePos->fDeclaration( ) << " = mul( " << modelToView->fName( ) << ", float4( " << modelSpacePos->fSwizzle( 3 ) << ", 1.f ) );" << std::endl;

		fBeginLine( true ) << "// compute facing view space position of particle corner" << std::endl;
		fBeginLine( ) << cameraX->fDeclaration( ) << " = " << iUv01->fSwizzle( "x" ) << " * float3( " << cosRoll->fName( ) << ", -" << sinRoll->fName( ) << ", 0 );" << std::endl;
		fBeginLine( ) << cameraY->fDeclaration( ) << " = " << iUv01->fSwizzle( "y" ) << " * float3( " << sinRoll->fName( ) << ",  " << cosRoll->fName( ) << ", 0 );" << std::endl;
		fBeginLine( ) << viewSpaceFacingPos->fDeclaration( ) << " = " << viewSpacePos->fSwizzle( "xyz" ) << " + " << cameraX->fSwizzle( "xyz" ) << " + " << cameraY->fSwizzle( "xyz" ) << ";" << std::endl;

		fBeginLine( true ) << "// output position to projection space" << std::endl;
		fBeginLine( ) << projSpacePos->fName( ) << " = mul( " << viewToProj->fName( ) << ", float4( " << viewSpaceFacingPos->fSwizzle( "xyz" ) << ", 1.f ) );" << std::endl;

		//
		// now we handle outputting optional parameters for lighting/shadowing...
		//

		tHlslVariableConstPtr projSpaceDepth = tVertexShaderVars::fVariable( tVertexShaderVars::cOutProjSpaceDepth, fInput( ).mPid );
		tHlslVariableConstPtr viewToWorld = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalViewToWorld, fInput( ).mPid );
		tHlslVariableConstPtr litSpacePos = tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpacePos, fInput( ).mPid );
		tHlslVariableConstPtr litSpaceNormal = tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpaceNormal, fInput( ).mPid );
		tHlslVariableConstPtr litSpaceTangent = tVertexShaderVars::fVariable( tVertexShaderVars::cOutLitSpaceTangent, fInput( ).mPid );
		tHlslVariableConstPtr lightPos = tVertexShaderVars::fVariable( tVertexShaderVars::cOutLightPos, fInput( ).mPid );

		// proj-space depth
		if( reqs.mOutputs.fFind( projSpaceDepth ) )
		{
			fBeginLine( true ) << "// output projection space depth" << std::endl;
			fBeginLine( ) << projSpaceDepth->fName( )
				<< " = " << projSpacePos->fSwizzle( "zw" ) << ";" << std::endl;
		}

		// lit-space pos
		if( reqs.mOutputs.fFind( litSpacePos ) )
		{
			fBeginLine( true ) << "// output position to lit space" << std::endl;
			fBeginLine( ) << litSpacePos->fName( )
				<< " = float4( mul( ( float3x3 )" << viewToWorld->fName( ) << ", " << viewSpaceFacingPos->fSwizzle( "xyz" ) << " ), 1.f );" << std::endl;
		}

		// lit-space normal
		if( reqs.mOutputs.fFind( litSpaceNormal ) )
		{
			fBeginLine( true ) << "// output normal to lit space" << std::endl;
			fBeginLine( ) << litSpaceNormal->fName( )
				<< " = mul( ( float3x3 )" << viewToWorld->fName( ) << ", float3( 0, 0, 1 ) );" << std::endl;
		}

		// lit-space tangent
		if( reqs.mOutputs.fFind( litSpaceTangent ) )
		{
			fBeginLine( true ) << "// output tangent to lit space" << std::endl;
			fBeginLine( ) << litSpaceTangent->fName( )
				<< " = float4( mul( ( float3x3 )" << viewToWorld->fName( ) << ", float3( 1, 0, 0 ) ), 1.f );" << std::endl;
		}

		// light position
		if( reqs.mOutputs.fFind( lightPos ) )
		{
			fBeginLine( true ) << "// light position for shadow mapping" << std::endl;
			tHlslVariableConstPtr viewToLight = tVertexShaderVars::fVariable( tVertexShaderVars::cGlobalViewToLight, fInput( ).mPid );
			fBeginLine( ) << lightPos->fName( )
				<< " = mul( " << viewToLight->fName( ) << ", float4( " << viewSpaceFacingPos->fSwizzle( "xyz" ) << ", 1.f ) );" << std::endl;
		}
	}

	void tHlslWriter::fPixelShaderMain( const tShaderRequirements& reqs, u32 numLights )
	{
		// cache variables we'll be using
		tHlslVariableConstPtr litSpacePos = tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos, fInput( ).mPid );
		tHlslVariableConstPtr litSpaceNormal = tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceNormal, fInput( ).mPid );
		tHlslVariableConstPtr litSpaceTangent = tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceTangent, fInput( ).mPid );

		tHlslVariableConstPtr eyeToVertexDir = tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexDir, fInput( ).mPid );
		tHlslVariableConstPtr eyeToVertexLen = tPixelShaderVars::fVariable( tPixelShaderVars::cTempEyeToVertexLen, fInput( ).mPid );

		// normalize input normal
		if( reqs.mInputs.fFind( litSpaceNormal ) )
		{
			fBeginLine( true ) << "// re-normalize normal to account for interpolation" << std::endl;
			fBeginLine( ) << litSpaceNormal->fSwizzle( "xyz" ) << " = normalize( " << litSpaceNormal->fSwizzle( "xyz" ) << " );" << std::endl;
		}

		// normalize input tangent
		if( reqs.mInputs.fFind( litSpaceTangent ) )
		{
			fBeginLine( true ) << "// re-normalize tangent to account for interpolation" << std::endl;
			fBeginLine( ) << litSpaceTangent->fSwizzle( "xyz" ) << " = normalize( " << litSpaceTangent->fSwizzle( "xyz" ) << " );" << std::endl;
		}

		const b32 hasEyeToVertexDir = reqs.mCommon.fFind( eyeToVertexDir ) != 0;
		const b32 hasEyeToVertexLen = reqs.mCommon.fFind( eyeToVertexLen ) != 0;
		if( hasEyeToVertexDir || hasEyeToVertexLen )
		{
			sigassert( reqs.mInputs.fFind( litSpacePos ) );

			fBeginLine( true ) << "// cache length of eye-to-vertex vector" << std::endl;
			fBeginLine( ) << eyeToVertexLen->fDeclaration( ) << " = length( " << litSpacePos->fSwizzle( "xyz" ) << " );" << std::endl;

			if( hasEyeToVertexDir )
			{
				fBeginLine( true ) << "// cache normalized eye-to-vertex direction vector" << std::endl;
				fBeginLine( ) << eyeToVertexDir->fDeclaration( ) << " = " << litSpacePos->fSwizzle( "xyz" ) << " / " << eyeToVertexLen->fName( ) << ";" << std::endl;
			}
		}


		tHlslGenTree::fWriteNodesToPixelShader( mShadeTreeRoots, *this, reqs );
	}

}}

