#include "ToolsPch.hpp"
#include "tHlslGenTree.hpp"
#include "Derml.hpp"
#include "tHlslWriter.hpp"
#include "tPixelShaderVars.hpp"

namespace Sig { namespace HlslGen
{


	tHlslGenTree::tHlslGenTree( const tShadeNodePtr& shadeNode, tHlslGenTree* output )
		: mShadeNode( shadeNode )
		, mVisited( false )
	{
		sigassert( mShadeNode );

		mShadeNode->fSetMaterialGlueIndex( ); // clear material glue index

		mInputs.fSetCount( mShadeNode->fInputCount( ) );
		if( output )
			fAddOutput( output );
	}

	tHlslGenTree::~tHlslGenTree( )
	{
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			if( mInputs[ i ] )
				mInputs[ i ]->mOutputs.fFindAndErase( this );
		}
	}

	void tHlslGenTree::fForceSetInput( u32 ithInput, const tHlslGenTreePtr& input )
	{
		sigassert( !mInputs[ ithInput ] );
		sigassert( input->fOutputs( ).fFind( this ) );
		mInputs[ ithInput ] = input;
	}

	void tHlslGenTree::fGenerateShaders( const Derml::tFile& dermlFile, const tHlslInput& input, tHlslOutput& output )
	{
		// get outputs
		tShadeNode::tOutputArray outputs;
		dermlFile.fCollectOutputs( outputs );

		// construct corresponding shade trees
		tGrowableArray<tHlslGenTreePtr> shadeTreeRoots;
		for( u32 i = 0; i < outputs.fCount( ); ++i )
			if( outputs[ i ] )
				shadeTreeRoots.fPushBack( tHlslGenTreePtr( new tHlslGenTree( outputs[ i ] ) ) );
		for( u32 i = 0; i < shadeTreeRoots.fCount( ); ++i )
			shadeTreeRoots[ i ]->fConstructTree( dermlFile );

		tHlslWriter hlslWriter( shadeTreeRoots );
		hlslWriter.fGenerateShaders( input, output );
	}

	namespace
	{
		static b32 fShouldWriteOutput( tHlslWriter& writer, const tHlslGenTreePtr& node )
		{
			if(	writer.fWriteMode( ) == cWriteModeColor )
				return true; // in color mode, all nodes get written
			if( ( writer.fWriteMode( ) == cWriteModeDepth || writer.fWriteMode( ) == cWriteModeDepthWithAlpha ) &&
				node->fShadeNode( )->fOutputSemantic( ) == tShadeNode::cOutputColor0 )
				return true; // in depth modes, only color0 gets written
			return false;
		}
	}

	void tHlslGenTree::fDeterminePixelShaderRequirements( const tGrowableArray<tHlslGenTreePtr>& shadeTreeRoots, tHlslWriter& writer, tShaderRequirements& reqs )
	{
		reqs.mNextConstantRegister = tPixelShaderVars::fBaseConstantRegister( );
		reqs.mNextSamplerRegister = tPixelShaderVars::fBaseSamplerRegister( );

		// determine the pixel shader requirements
		for( u32 i = 0; i < shadeTreeRoots.fCount( ); ++i )
		{
			if(	fShouldWriteOutput( writer, shadeTreeRoots[ i ] ) )
				shadeTreeRoots[ i ]->fAddPixelShaderRequirements( writer, reqs );
		}

		// cleanup
		for( u32 i = 0; i < shadeTreeRoots.fCount( ); ++i )
			shadeTreeRoots[ i ]->fCleanupAfterAddingRequirements( );

		reqs.fSortGlobals( );
	}

	void tHlslGenTree::fWriteNodesToPixelShader( const tGrowableArray<tHlslGenTreePtr>& shadeTreeRoots, tHlslWriter& writer, const tShaderRequirements& reqs )
	{
		// write all nodes to shader
		for( u32 i = 0; i < shadeTreeRoots.fCount( ); ++i )
		{
			if(	fShouldWriteOutput( writer, shadeTreeRoots[ i ] ) )
				shadeTreeRoots[ i ]->fWriteToPixelShader( writer, reqs );
		}

		// cleanup
		for( u32 i = 0; i < shadeTreeRoots.fCount( ); ++i )
			shadeTreeRoots[ i ]->fCleanupAfterWritingToPixelShader( );
	}

	void tHlslGenTree::fResolveInputResults( tHlslWriter& writer, const tShaderRequirements& reqs ) const
	{
		mInputResults.fSetCount( mInputs.fCount( ) );
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			const tHlslGenTreePtr& input = mInputs[ i ];
			if( !input )
				continue;
			if( !mShadeNode->fInputNeedsWritingToHlsl( writer, *this, i ) )
				continue;
			if( !input->fShadeNodeOutput( ) )
			{
				input->fShadeNode( )->fWriteHlsl( writer, reqs, *input );
				//sigassert( input->fShadeNodeOutput( ) );
			}
			mInputResults[ i ] = input->fShadeNodeOutput( );
		}
	}

	void tHlslGenTree::fConstructTree( const Derml::tFile& dermlFile )
	{
		// cache root
		tHlslGenTree* root = fRoot( );

		// find connections to 'this'
		Derml::tConnectionList connex;
		dermlFile.fGetConnectionList( mShadeNode, connex );

		// instantiate/find input shade trees
		for( u32 i = 0; i < connex.fCount( ); ++i )
		{
			tShadeNodePtr child = dermlFile.mNodes[ connex[ i ].mOutput ];
			tHlslGenTree* find = root->fFindShadeTree( child );
			if( !find )
				find = new tHlslGenTree( child, this );

			tHlslGenTreePtr& input = mInputs[ connex[ i ].mIndex ];
			if( input != find )
				input.fReset( find );
		}

		// now recurse on inputs
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			if( mInputs[ i ] )
				mInputs[ i ]->fConstructTree( dermlFile );
		}
	}

	void tHlslGenTree::fAddPixelShaderRequirements( tHlslWriter& writer, tShaderRequirements& reqs )
	{
		if( mVisited )
			return;
		mVisited = true;

		// query the requirements of mShadeNode
		mShadeNode->fAddHlslRequirements( writer, reqs, *this );

		// recurse on inputs
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			if( mInputs[ i ] && mShadeNode->fInputNeedsWritingToHlsl( writer, *this, i ) )
				mInputs[ i ]->fAddPixelShaderRequirements( writer, reqs );
		}
	}

	void tHlslGenTree::fCleanupAfterAddingRequirements( )
	{
		mVisited = false;

		// recurse on inputs
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			if( mInputs[ i ] )
				mInputs[ i ]->fCleanupAfterAddingRequirements( );
		}
	}

	void tHlslGenTree::fCleanupAfterWritingToPixelShader( )
	{
		mShadeNodeOutput.fRelease( );
		mInputResults.fSetCount( 0 );

		// recurse on inputs
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			if( mInputs[ i ] )
				mInputs[ i ]->fCleanupAfterWritingToPixelShader( );
		}
	}

	void tHlslGenTree::fWriteToPixelShader( tHlslWriter& writer, const tShaderRequirements& reqs )
	{
		// write to hlsl
		mShadeNode->fWriteHlsl( writer, reqs, *this );
	}

	tHlslGenTree* tHlslGenTree::fRoot( )
	{
		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
		{
			tHlslGenTree* root = mOutputs[ i ]->fRoot( );
			if( root )
				return root;
		}

		return this;
	}

	void tHlslGenTree::fAddOutput( tHlslGenTree* output )
	{
		sigassert( output );
		mOutputs.fFindOrAdd( output );
	}

	tHlslGenTree* tHlslGenTree::fFindShadeTree( const tShadeNodePtr& shadeNode )
	{
		if( shadeNode == mShadeNode )
			return this;
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			if( mInputs[ i ] )
			{
				tHlslGenTree* find = mInputs[ i ]->fFindShadeTree( shadeNode );
				if( find )
					return find;
			}
		}
		return 0;
	}

}}

