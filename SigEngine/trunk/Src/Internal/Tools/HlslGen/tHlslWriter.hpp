#ifndef __tHlslWriter__
#define __tHlslWriter__
#include "tHlslGenTree.hpp"

namespace Sig { namespace HlslGen
{
	class tools_export tHlslWriter
	{
	private:
		const tHlslInput* mInput;
		tHlslOutput* mOutput;
		tPixelShaderOutput* mCurrentPixelShaderOutput;
		tGrowableArray<tHlslGenTreePtr> mShadeTreeRoots;
		std::stringstream mSs;			// holds the shader text as its built.
		std::stringstream mFunctions;	// Holds additional functions for the shader.
		u32 mFunctionLocation;			// Stores the character index in the shader where functions will be inserted after shader generation is complete.
		tWriteMode mWriteMode;
		b32 mDeferredShading;
		b32 mInstanced;
		b32 mRecvShadow;
		b32 mDualParaboloid;
		u32 mTabLevel;
		u32 mTempIndex;
		tGrowableArray<std::string> mDeclaredUserTypes;

	public:
		explicit tHlslWriter( const tGrowableArray<tHlslGenTreePtr>& shadeTreeRoots );
		const tHlslInput& fInput( ) const { return *mInput; }
		void fGenerateShaders( const tHlslInput& input, tHlslOutput& output );
		
		u32 fAddMaterialGlueVector( u32 registerIndex, s32 glueIndex );
		u32 fAddMaterialGlueSampler( u32 registerIndex, s32 glueIndex );
		u32 fAddMaterialGlueAtlasSampler( u32 samplerRegisterIndex, u32 infoRegisterIndex, s32 glueIndex );
		u32 fAddMaterialGlueString( u32 registerIndex, s32 glueIndex );
		void fAddMaterialGlueForTransitionObjects( u32 defIdRegIdx, u32 edgeColorRegIdx, u32 objsRegIdx );

		tHlslVariableConstPtr fMakeTempVector( const std::string& name, u32 dimensionX = 4, u32 arrayCount = 0 );
		std::stringstream& fBeginLine( b32 extraLineBreak = false );
		std::stringstream& fContinueLine( );
		std::stringstream& fFunctions( ) { return mFunctions; }
		void fPushTab( ) { ++mTabLevel; }
		void fPopTab( ) { sigassert( mTabLevel > 0 ); --mTabLevel; }
		tWriteMode fWriteMode( ) { return mWriteMode; }
		b32 fDeclareType( const std::string& userType ) { if( mDeclaredUserTypes.fFind( userType ) ) return true; mDeclaredUserTypes.fPushBack( userType ); return false; }
		b32 fRecvShadow( ) const { return mRecvShadow && !mDeferredShading && (fInput().mToolType != HlslGen::cToolTypeMaya); }
		b32 fDualParaboloid( ) const { return mDualParaboloid; }
		b32 fDeferredShading( ) const { return mDeferredShading; }
		b32 fInstanced( ) const { return mInstanced; }
	private:
		void fComputeVertexShaderRequirements_MeshModel( const tShaderRequirements& psReqs, tShaderRequirements& vsReqs, u32 numBoneWeights = 0 );
		void fComputeVertexShaderRequirements_FacingQuads( const tShaderRequirements& psReqs, tShaderRequirements& vsReqs );
		void fWriteVertexShader( const tShaderRequirements& vsReqs, u32 numBoneWeights, tVertexShaderOutput& output );
		void fWritePixelShader( const tShaderRequirements& psReqs, u32 numLights, tPixelShaderOutput& output );
		void fBeginShaderMain( const tShaderRequirements& reqs );
		void fEndShaderMain( const tShaderRequirements& reqs );
		void fVertexShaderMain_MeshModel( const tShaderRequirements& reqs, u32 numBoneWeights );
		void fVertexShaderMain_FacingQuads( const tShaderRequirements& reqs );
		void fPixelShaderMain( const tShaderRequirements& reqs, u32 numLights );
	};

}}

#endif//__tHlslWriter__


