#include "ToolsPch.hpp"
#include "tShadeMaterialGen.hpp"
#include "Gfx/tShadeMaterial.hpp"
#include "hlslGen/tHlslGenTree.hpp"

namespace Sig
{
	register_rtti_factory( tShadeMaterialGen, false );

	tShadeMaterialGen::tShadeMaterialGen( )
	{
	}

	b32 tShadeMaterialGen::fFromDermlMtlFile( const Derml::tMtlFile& dermlMtlFile )
	{
		mMtlFile = dermlMtlFile;
		return true;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tShadeMaterialGen& o )
	{
		o.mMtlFile.fSerialize( s );
	}

	void tShadeMaterialGen::fSerialize( tXmlSerializer& s )		{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tShadeMaterialGen::fSerialize( tXmlDeserializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tShadeMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		// TODO technically, we need this process to happen for each platform (i.e., a separate call to fCreateGfxMaterial);
		// right now we're just hoping all platforms have the same material glue properties

		Derml::tFile dermlFile;
		if( !dermlFile.fLoadXml( ToolsPaths::fMakeResAbsolute( mMtlFile.mShaderPath ) ) )
			return 0;
		mMtlFile.fUpdateAgainstShaderFile( dermlFile );

		HlslGen::tHlslPlatformId hlslPid = HlslGen::cPidDefault;
		if( dermlFile.mGeometryStyle == HlslGen::cVshMeshModel )
			hlslPid = HlslGen::cPidXbox360;

		// generate hlsl shader code
		HlslGen::tHlslInput input = HlslGen::tHlslInput( hlslPid, dermlFile.mGeometryStyle );
		HlslGen::tHlslOutput output;
		HlslGen::tHlslGenTree::fGenerateShaders( dermlFile, input, output );

		Gfx::tShadeMaterial* mtl = new Gfx::tShadeMaterial( );
		fConvertBase( mtl );

		mtl->mGlueValues = output.mMaterialGlueValues;
		mtl->mGlueValues.fSetBackFaceFlip( mFlipBackFaceNormal ? 2.f : 0.f );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Derml::fDermlPathToMtlb( mMtlFile.mShaderPath ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		// add shader as a dependency
		mTextureDependencies.fFindOrAdd( mMtlFile.mShaderPath );

		// get material glue values (take as output an extra array of the texture paths as dependencies)
		for( u32 i = 0; i < mMtlFile.mNodes.fCount( ); ++i )
			mMtlFile.mNodes[ i ]->fAcquireMaterialGlueValuesForAssetGen( mtl->mGlueValues, lipFileCreator, mTextureDependencies );

		const b32 useAlpha = ( mtl->fGetRenderState( ).fHasTransparency( ) || mtl->fGetRenderState( ).fQuery( Gfx::tRenderState::cCutOut ) );
		const u32 depthVsIndex = useAlpha ? HlslGen::cWriteModeDepthWithAlpha : HlslGen::cWriteModeDepth;

		const Gfx::tShadeMaterial::tShaderSlot vsSlot = skinned ? Gfx::tShadeMaterial::cShaderSlotSkinnedVs : Gfx::tShadeMaterial::cShaderSlotStaticVs;
		mtl->mColorPass.mVsType = vsSlot;
		mtl->mColorPass.mVsIndex = HlslGen::cWriteModeColor;
		mtl->mDepthPass.mVsType = vsSlot;
		mtl->mDepthPass.mVsIndex = depthVsIndex;
		mtl->mColorPass.mPsType = Gfx::tShadeMaterial::cShaderSlotColorPs;
		mtl->mColorPass.mPsBaseIndex = 0;
		mtl->mColorPass.mPsMaxLights = fMax<int>( 0, output.mColorPShaders.fCount( ) - 1 );
		mtl->mDepthPass.mPsType = useAlpha ? Gfx::tShadeMaterial::cShaderSlotDepthAlphaPs : Gfx::tShadeMaterial::cShaderSlotDepthPs;
		mtl->mDepthPass.mPsBaseIndex = 0;
		mtl->mDepthPass.mPsMaxLights = 0;

		// copy vertex formats
		tFixedArray<HlslGen::tVertexShaderOutput,HlslGen::cWriteModeCount>& vtxShaders = skinned ? output.mSkinnedVShaders : output.mStaticVShaders;
		if( vtxShaders[ HlslGen::cWriteModeColor ].mVtxFormat )
			mtl->mColorPass.mVertexFormat = *vtxShaders[ HlslGen::cWriteModeColor ].mVtxFormat;
		if( vtxShaders[ depthVsIndex ].mVtxFormat )
			mtl->mDepthPass.mVertexFormat = *vtxShaders[ depthVsIndex ].mVtxFormat;

		return mtl;
	}

	b32	 tShadeMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		return false;
	}

	void tShadeMaterialGen::fGetTextureResourcePaths( tFilePathPtrList& resourcePathsOut )
	{
		// pass texture resource paths acquired from material glue values
		for( u32 i = 0; i < mTextureDependencies.fCount( ); ++i )
			resourcePathsOut.fFindOrAdd( mTextureDependencies[ i ] );
	}

}


