#include "BasePch.hpp"
#include "tTilePackage.hpp"

namespace Sig
{
	define_lip_version( tTilePackage, 1, 1, 1 );

	tTilePackage::tTileType::tTileType( )
		: mTotalWeight( 1.f )
	{
	}

	tTilePackage::tTileType::tTileType( tNoOpTag )
		: mTileDefs( cNoOpTag )
	{
		u32 i = 0;
		++i;
	}

	const tTilePackage::tTileDef* tTilePackage::tTileType::fGetRandomDef( tRandom& generator ) const
	{
		f32 targetRandom = generator.fFloatZeroToValue( mTotalWeight );
		for( u32 i = 0; i < mTileDefs.fCount(); ++i )
		{
			// We found it, gentlemen.
			if( targetRandom <= mTileDefs[i].mWeight )
				return &mTileDefs[i];

			targetRandom -= mTileDefs[i].mWeight;
		}

		return NULL;
	}

	const tTilePackage::tTileDef* tTilePackage::tTileType::fGetSpecificDef( const tStringPtr& idString ) const
	{
		for( u32 i = 0; i < mTileDefs.fCount(); ++i )
		{
			// We found it, gentlemen.
			if( mTileDefs[i].mIdString && idString == mTileDefs[i].mIdString->fGetStringPtr() )
				return &mTileDefs[i];
		}

		return NULL;
	}


	const char* tTilePackage::fGetFileExtension( )
	{
		return ".tiledb";
	}

	tTilePackage::tTilePackage( )
		: mGuid( 0 )
		, mUseSpecificGen( false )
	{
	}

	tTilePackage::tTilePackage( tNoOpTag ) 
		: tLoadInPlaceFileBase( cNoOpTag )
		, mTileDefsByType( cNoOpTag )
		, mGenerator( cNoOpTag )

	{
		u32 i = 0;
		++i;
	}
}