#ifndef __tHlslGenTree__
#define __tHlslGenTree__
#include "tShadeNode.hpp"

namespace Sig { namespace Derml { class tFile; } }

namespace Sig { namespace HlslGen
{
	class tHlslWriter;

	class tHlslGenTree;
	typedef tRefCounterPtr<tHlslGenTree> tHlslGenTreePtr;

	class tools_export tHlslGenTree : public tUncopyable, public tRefCounter
	{
	public:
		typedef tGrowableArray<tHlslGenTree*> tOutputArray;
		typedef tGrowableArray<tHlslGenTreePtr> tInputArray;
		typedef tGrowableArray<tHlslVariableConstPtr> tInputResultArray;

	private:
		tShadeNodePtr	mShadeNode;
		tOutputArray	mOutputs;
		tInputArray		mInputs;
		b32				mVisited;

		mutable tFixedArray<tHlslVariableConstPtr, 3>	mShadeNodeReq;
		mutable tHlslVariableConstPtr	mShadeNodeOutput;
		mutable tInputResultArray		mInputResults;

	public:
		static void fGenerateShaders( const Derml::tFile& dermlFile, const tHlslInput& input, tHlslOutput& output );
		static void fDeterminePixelShaderRequirements( const tGrowableArray<tHlslGenTreePtr>& shadeTreeRoots, tHlslWriter& writer, tShaderRequirements& reqs );
		static void fWriteNodesToPixelShader( const tGrowableArray<tHlslGenTreePtr>& shadeTreeRoots, tHlslWriter& writer, const tShaderRequirements& reqs );

	public:
		explicit tHlslGenTree( const tShadeNodePtr& shadeNode, tHlslGenTree* output = 0 );
		~tHlslGenTree( );

		void fForceSetInput( u32 ithInput, const tHlslGenTreePtr& input );

		const tShadeNodePtr& fShadeNode( ) const { return mShadeNode; }
		const tOutputArray& fOutputs( ) const { return mOutputs; }
		const tInputArray& fInputs( ) const { return mInputs; }

		void fCacheShadeNodeReq( const tHlslVariableConstPtr& var, u32 idx = 0 ) const { mShadeNodeReq[ idx ] = var; }
		const tHlslVariableConstPtr& fShadeNodeReq( u32 idx = 0 ) const { return mShadeNodeReq[ idx ]; }

		void fCacheShadeNodeOutput( const tHlslVariableConstPtr& var ) const { mShadeNodeOutput = var; }
		const tHlslVariableConstPtr& fShadeNodeOutput( ) const { return mShadeNodeOutput; }

		void fResolveInputResults( tHlslWriter& writer, const tShaderRequirements& reqs ) const;
		const tInputResultArray& fInputResults( ) const { return mInputResults; }

	private:
		void fConstructTree( const Derml::tFile& dermlFile );
		void fAddPixelShaderRequirements( tHlslWriter& writer, tShaderRequirements& reqs );
		void fCleanupAfterAddingRequirements( );
		void fCleanupAfterWritingToPixelShader( );
		void fWriteToPixelShader( tHlslWriter& writer, const tShaderRequirements& reqs );
		tHlslGenTree* fRoot( );
		void fAddOutput( tHlslGenTree* output );
		tHlslGenTree* fFindShadeTree( const tShadeNodePtr& shadeNode );
	};

}}

#endif//__tHlslGenTree__
