#include "BasePch.hpp"
#include "tHeightFieldMaterialBase.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f32_3 ),
			//tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tVertexElement gCompressedVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f16_4 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f16_2 ),
			//tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
	}
	const tVertexFormat tHeightFieldMaterialBase::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );
	const tVertexFormat tHeightFieldMaterialBase::cCompressedVertexFormat( gCompressedVertexFormatElements, array_length( gCompressedVertexFormatElements ) );

	devvar( bool, Lod_TerrainNormals, true );
	devvar( bool, Lod_TerrainShadows, true );

	b32 tHeightFieldMaterialBase::fLod_TerrainNormals( )
	{
		return Lod_TerrainNormals;
	}
	b32 tHeightFieldMaterialBase::fLod_TerrainShadows( )
	{
		return Lod_TerrainShadows;
	}


	//------------------------------------------------------------------------------
	// tHeightFieldMaterialBase
	//------------------------------------------------------------------------------
	tHeightFieldMaterialBase::tHeightFieldMaterialBase( )
		: mWorldSpaceDims( 256.f )
		, mSubDiffuseRectDims( 512.f )
		, mSubNormalRectDims( 512.f )
		, mDiffuseCount_NormalCount( 1.f )
	{
		for( u32 i = 0; i < mTileFactors.fCount( ); ++i )
			mTileFactors[ i ] = Math::tVec4f( 0.5f );
	}
	//------------------------------------------------------------------------------
	tHeightFieldMaterialBase::tHeightFieldMaterialBase( tNoOpTag )
		: tMaterial( cNoOpTag )
		, mMaskMap( cNoOpTag )
		, mMtlIdsMap( cNoOpTag )
		, mDiffuseMap( cNoOpTag )
		, mNormalMap( cNoOpTag )
		, mWorldSpaceDims( cNoOpTag )
		, mSubDiffuseRectDims( cNoOpTag )
		, mSubNormalRectDims( cNoOpTag )
		, mDiffuseCount_NormalCount( cNoOpTag )
		, mTileFactors( cNoOpTag )
	{
	}
	//------------------------------------------------------------------------------

}}//Sig::Gfx
