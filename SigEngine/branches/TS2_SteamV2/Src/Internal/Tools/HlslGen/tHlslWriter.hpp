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
		std::stringstream mSs;
		tWriteMode mWriteMode;
		u32 mTabLevel;
		u32 mTempIndex;
		tGrowableArray<std::string> mDeclaredUserTypes;

	public:
		explicit tHlslWriter( const tGrowableArray<tHlslGenTreePtr>& shadeTreeRoots );
		const tHlslInput& fInput( ) const { return *mInput; }
		void fGenerateShaders( const tHlslInput& input, tHlslOutput& output );
		u32 fAddMaterialGlueVector( u32 registerIndex, s32 glueIndex );
		u32 fAddMaterialGlueSampler( u32 registerIndex, s32 glueIndex );
		u32 fAddMaterialGlueString( u32 registerIndex, s32 glueIndex );
		tHlslVariableConstPtr fMakeTempVector( const std::string& name, u32 dimensionX = 4, u32 arrayCount = 0 );
		std::stringstream& fBeginLine( b32 extraLineBreak = false );
		std::stringstream& fContinueLine( );
		void fPushTab( ) { ++mTabLevel; }
		void fPopTab( ) { sigassert( mTabLevel > 0 ); --mTabLevel; }
		tWriteMode fWriteMode( ) { return mWriteMode; }
		b32 fDeclareType( const std::string& userType ) { if( mDeclaredUserTypes.fFind( userType ) ) return true; mDeclaredUserTypes.fPushBack( userType ); return false; }
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


