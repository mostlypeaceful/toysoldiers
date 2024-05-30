#ifndef __tTilePackage__
#define __tTilePackage__
#include "tLoadInPlaceFileBase.hpp"

namespace Sig
{
	///
	/// \class tTilePackage
	/// \brief A tile set. Sawesome.
	class base_export tTilePackage : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tTilePackage, 0x1B32C6E1);
	public:
		static const char* fGetFileExtension( );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }

	public:
		///
		/// \class tTileDef
		/// \brief An individual model definition in a type.
		struct base_export tTileDef
		{
			declare_reflector( );
		public:
			tLoadInPlaceResourcePtr* mSigml;
			tLoadInPlaceResourcePtr* mTexture;
			tLoadInPlaceStringPtr* mIdString;
			f32 mWeight;
			Math::tVec2u mDims;

			tTileDef( ) : mSigml( NULL ), mTexture( NULL ), mIdString( NULL ), mWeight( 1.f ), mDims( Math::tVec2u::cZeroVector ) { }
			tTileDef( tNoOpTag ) { }
		};

		///
		/// \class tTileType
		/// \brief A list of all models possible in a given type.
		struct base_export tTileType
		{
			declare_reflector( );
		public:
			f32 mTotalWeight; // Needs to be filled out in AssetGen.
			tDynamicArray< tTileDef > mTileDefs;

			tTileType( );
			tTileType( tNoOpTag );

			const tTilePackage::tTileDef* fGetRandomDef( tRandom& generator ) const;
			const tTilePackage::tTileDef* fGetSpecificDef( const tStringPtr& idString ) const;
		};

	public:
		u32 mGuid;
		tDynamicArray< tTileType > mTileDefsByType; // Size should == the number of tile types
		tLoadInPlaceRuntimeObject< tRandom > mGenerator;
		b32 mUseSpecificGen;

	public:
		tTilePackage( );
		tTilePackage( tNoOpTag );

		const tTileDef* fFindRandomTileDefByTileType( u32 tileType )
		{
			if( mUseSpecificGen )
				return mTileDefsByType[tileType].fGetRandomDef( mGenerator.fTreatAsObject( ) );
			else
				return mTileDefsByType[tileType].fGetRandomDef( tRandom::fObjectiveRand( ) );
		}

		const tTileDef* fFindSpecificTileDefByTileType( u32 tileType, const tStringPtr& idString )
		{
			return mTileDefsByType[tileType].fGetSpecificDef( idString );
		}

	};
}

#endif // __tTilePackage__