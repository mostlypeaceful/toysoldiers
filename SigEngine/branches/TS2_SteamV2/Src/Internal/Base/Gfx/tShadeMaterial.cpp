#include "BasePch.hpp"
#include "tShadeMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tRenderContext.hpp"
#include "tRenderBatch.hpp"

namespace Sig { namespace Gfx
{

	tShadeMaterialGlueValues::tShadeMaterialGlueValues( )
		: mBackFaceFlip( 0.f )
	{
	}

	tShadeMaterialGlueValues::tShadeMaterialGlueValues( tNoOpTag )
		: mSamplers( cNoOpTag )
		, mVectors( cNoOpTag )
		, mStrings( cNoOpTag )
	{
	}

	u32 tShadeMaterialGlueValues::fAddSampler( const tTextureReference& tr )
	{
		mSamplers.fPushBack( tr );
		return mSamplers.fCount( ) - 1;
	}

	u32 tShadeMaterialGlueValues::fAddVector( const Math::tVec4f& v )
	{
		mVectors.fPushBack( v );
		return mVectors.fCount( ) - 1;
	}

	u32	tShadeMaterialGlueValues::fAddString( )
	{
		mStrings.fPushBack( 0 );
		return mStrings.fCount( ) - 1;
	}

	b32 tShadeMaterialGlueValues::fUpdateSampler( const tTextureReference& tr, u32 idx )
	{
		if( idx >= mSamplers.fCount( ) )
			return false;
		mSamplers[ idx ].mTexSrc = cTexSourceFromFile;
		mSamplers[ idx ].mTexRef = tr;
		return true;
	}

	b32 tShadeMaterialGlueValues::fUpdateSampler( tTextureSource texSrc, u32 idx )
	{
		if( idx >= mSamplers.fCount( ) )
			return false;
		mSamplers[ idx ].mTexSrc = texSrc;
		mSamplers[ idx ].mTexRef = tTextureReference( );
		return true;
	}

	b32 tShadeMaterialGlueValues::fUpdateVector( const Math::tVec4f& v, u32 idx )
	{
		Math::tVec4f* toUpdate = ( idx < mVectors.fCount( ) ) ? &mVectors[ idx ] : 0;
		if( toUpdate )
		{
			*toUpdate = v;
			return true;
		}
		return false;
	}

	b32 tShadeMaterialGlueValues::fUpdateString( const char* s, tLoadInPlaceFileBase& lipFileCreator, u32 idx )
	{
#ifdef target_tools
		if( idx >= mStrings.fCount( ) )
			return false;
		mStrings[ idx ] = lipFileCreator.fAddLoadInPlaceStringPtr( s );
		return true;
#else
		return false;
#endif
	}

	const tTextureReference& tShadeMaterialGlueValues::fFindSampler( u32 idx, const tRenderContext& context ) const
	{
		if( idx < mSamplers.fCount( ) )
		{
			switch( mSamplers[ idx ].mTexSrc ) {
				case cTexSourceFromFile:	return mSamplers[ idx ].mTexRef;
				case cTexSourceWhite:		return context.mWhiteTexture;
				case cTexSourceBlack:		return context.mBlackTexture;
				case cTexSourceNoise:		return context.mWhiteTexture; // TODO noise texture
			}
		}
		return context.mWhiteTexture; // default
	}

	const Math::tVec4f* tShadeMaterialGlueValues::fFindVector( u32 idx ) const
	{
		return ( idx < mVectors.fCount( ) ) ? &mVectors[ idx ] : 0;
	}

	namespace { static const tStringPtr cNullString(""); }
	const tStringPtr& tShadeMaterialGlueValues::fFindString( u32 idx ) const
	{
		return ( idx < mStrings.fCount( ) && mStrings[ idx ] ) ? mStrings[ idx ]->fGetStringPtr( ) : cNullString;
	}

	tShadeMaterial::tPass::tPass( )
		: mVsType( cInvalidShaderIndex )
		, mVsIndex( cInvalidShaderIndex )
		, mPsType( cInvalidShaderIndex )
		, mPsBaseIndex( cInvalidShaderIndex )
		, mPsMaxLights( 0 )
		, pad0( 0 ), pad1( 0 ), pad2( 0 )
	{
	}
	tShadeMaterial::tPass::tPass( tNoOpTag )
		: mVertexFormat( cNoOpTag )
	{
	}
	void tShadeMaterial::tPass::fApplyShared( const tShadeMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const tMaterialFile* mtlFile = mtl.mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		sigassert( mtlFile );

		// apply vertex and pixel shaders
		sigassert( mVsIndex != cInvalidShaderIndex );
		sigassert( mPsBaseIndex != cInvalidShaderIndex );
		const u32 psIndex = mPsBaseIndex + fMin<u32>( mPsMaxLights, context.mLightShaderConstants.mLightCount );
		mtlFile->fApplyShader( device, mVsType, mVsIndex );
		mtlFile->fApplyShader( device, mPsType, psIndex  );

		// apply shared glue
		const tMaterialFile::tShaderPointer& vs = mtlFile->mShaderLists[ mVsType ][ mVsIndex ];
		const tMaterialFile::tShaderPointer& ps = mtlFile->mShaderLists[ mPsType ][ psIndex  ];
		const tShadeMaterialGlueArray& vsGlue = vs.mGlueShared;
		const tShadeMaterialGlueArray& psGlue = ps.mGlueShared;
		for( u32 i = 0; i < vsGlue.fCount( ); ++i )
			vsGlue[ i ]->fApplyShared( mtl, mtl.mGlueValues, device, context );
		for( u32 i = 0; i < psGlue.fCount( ); ++i )
			psGlue[ i ]->fApplyShared( mtl, mtl.mGlueValues, device, context );
	}
	void tShadeMaterial::tPass::fApplyInstance( const tShadeMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		const tMaterialFile* mtlFile = mtl.mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		sigassert( mtlFile );

		// get vertex and pixel shader indices
		sigassert( mVsIndex != cInvalidShaderIndex );
		sigassert( mPsBaseIndex != cInvalidShaderIndex );
		const u32 psIndex = mPsBaseIndex + fMin<u32>( mPsMaxLights, context.mLightShaderConstants.mLightCount );

		// apply shared glue
		const tMaterialFile::tShaderPointer& vs = mtlFile->mShaderLists[ mVsType ][ mVsIndex ];
		const tMaterialFile::tShaderPointer& ps = mtlFile->mShaderLists[ mPsType ][ psIndex  ];
		const tShadeMaterialGlueArray& vsGlue = vs.mGlueInstance;
		const tShadeMaterialGlueArray& psGlue = ps.mGlueInstance;
		for( u32 i = 0; i < vsGlue.fCount( ); ++i )
			vsGlue[ i ]->fApplyInstance( mtl, mtl.mGlueValues, device, context, drawCall );
		for( u32 i = 0; i < psGlue.fCount( ); ++i )
			psGlue[ i ]->fApplyInstance( mtl, mtl.mGlueValues, device, context, drawCall );
	}

	tShadeMaterial::tShadeMaterial( )
	{
	}
	tShadeMaterial::tShadeMaterial( tNoOpTag )
		: tMaterial( cNoOpTag )
		, mColorPass( cNoOpTag )
		, mDepthPass( cNoOpTag )
		, mGlueValues( cNoOpTag )
	{
	}
	const tVertexFormat& tShadeMaterial::fVertexFormat( ) const
	{
		return mColorPass.mVertexFormat;
	}
	b32 tShadeMaterial::fIsLit( ) const
	{
		return mColorPass.mPsMaxLights > 0;
	}
	b32 tShadeMaterial::fRendersDepth( ) const
	{
		return mDepthPass.mPsBaseIndex != tPass::cInvalidShaderIndex;
	}
	b32 tShadeMaterial::fSupportsInstancing( ) const
	{
		return false;
	}
	void tShadeMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		const tPass* pass = 0;
		switch( context.mRenderPassMode ) {
			case tRenderState::cRenderPassLighting:		pass = &mColorPass; break;
			case tRenderState::cRenderPassShadowMap:	pass = &mDepthPass; break;
			case tRenderState::cRenderPassDepth:		pass = &mDepthPass; break;
		}
		sigassert( pass );
		pass->fApplyShared( *this, device, context );
	}
	void tShadeMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		const tPass* pass = 0;
		switch( context.mRenderPassMode ) {
			case tRenderState::cRenderPassLighting:		pass = &mColorPass; break;
			case tRenderState::cRenderPassShadowMap:	pass = &mDepthPass; break;
			case tRenderState::cRenderPassDepth:		pass = &mDepthPass; break;
		}
		sigassert( pass );
		pass->fApplyInstance( *this, device, context, drawCall );

		renderBatch.fRenderInstance( device );
	}
}}

