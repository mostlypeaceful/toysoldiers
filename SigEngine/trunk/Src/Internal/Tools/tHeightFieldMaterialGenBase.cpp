#include "ToolsPch.hpp"
#include "tHeightFieldMaterialGenBase.hpp"

namespace Sig
{
	tHeightFieldMaterialGenBase::tHeightFieldMaterialGenBase( )
		: mWorldSpaceDims( 256.f, 256.f )
		, mSubDiffuseRectDims( 512.f )
		, mSubNormalRectDims( 512.f )
		, mDiffuseCount_NormalCount( 1.f )
	{
		for( u32 i = 0; i < mTileFactors.fCount( ); ++i )
			mTileFactors[ i ] = Math::tVec4f( 0.5f );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tHeightFieldMaterialGenBase& o )
	{
		s( "WorldSpaceDims", o.mWorldSpaceDims );
	}

	void tHeightFieldMaterialGenBase::fSerialize( tXmlSerializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tHeightFieldMaterialGenBase::fSerialize( tXmlDeserializer& s ) { tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	b32 tHeightFieldMaterialGenBase::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		if( !tMaterialGenBase::fIsEquivalent( other ) )
			return false;

		const tHeightFieldMaterialGenBase& otherMyType = static_cast<const tHeightFieldMaterialGenBase&>( other );

		if( mWorldSpaceDims != otherMyType.mWorldSpaceDims )
			return false;
		if( mSubDiffuseRectDims != otherMyType.mSubDiffuseRectDims )
			return false;
		if( mSubNormalRectDims != otherMyType.mSubNormalRectDims )
			return false;
		if( mDiffuseCount_NormalCount != otherMyType.mDiffuseCount_NormalCount )
			return false;
		for( u32 i = 0; i < mTileFactors.fCount( ); ++i )
			if( mTileFactors[ i ] != otherMyType.mTileFactors[ i ] )
				return false;


		return true;
	}

}//Sig
