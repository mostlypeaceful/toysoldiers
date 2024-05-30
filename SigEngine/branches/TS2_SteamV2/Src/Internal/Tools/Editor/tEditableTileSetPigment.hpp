//------------------------------------------------------------------------------
// \file tEditableTileSetPigment.hpp - 03 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEditableTileSetPigment__
#define __tEditableTileSetPigment__
#include "Tieml.hpp"
#include "tEditableTileEntity.hpp"

namespace Sig
{
	class tEditableTileSetPigmentSet;
	class tEditableTileDb;
	class tEditableTileSet;
	enum tTileTypes;

	typedef tGrowableArray<tFilePathPtr> tModelList;

	///
	/// \class tTileSetPalette
	/// \brief Contains painting information for a collection of tile sets.
	class tools_export tEditableTileSetPigment : public tRefCounter, public tUncopyable
	{
		// Holds a reference to a tile set and the relative frequency associated with it.
		struct tTileSetRef
		{
			tTileSetRef( ) : mRandomWeight( 1.f ) { }

			b32 operator==( const tTileSetRef& other ) { return mGuid == other.mGuid; }

			u32 mGuid;
			f32 mRandomWeight;
		};

		tEditableTileSetPigmentSet* mParent;

		std::string						mName;
		u32								mGuid;
		tGrowableArray< tTileSetRef >	mTileSetRefs;
		Math::tVec4f					mColorRgba;
		f32								mTileHeight;
		f32								mTileSize;
		s32								mSelectedTileSet;

		tDynamicArray<b32>				mHasRandom; // Holds whether a category has random tiles available for it.
		tGrowableArray< tModelList >	mSpecificModels; // Holds full lists of the specific models in a category.

	public:
		tEditableTileSetPigment( tEditableTileSetPigmentSet* parent, std::string name, u32 guid, const Math::tVec4f& color );

		void fSetName( std::string newName ) { mName = newName; }
		void fSetColor( const Math::tVec4f& newColor ) { mColorRgba = newColor; }
		void fSetHeight( f32 newHeight ) { mTileHeight = newHeight; }
		void fSetSize( f32 newSize ) { mTileSize = newSize; }

		std::string				fName( ) const { return mName; }
		u32						fGuid( ) const { return mGuid; }
		const Math::tVec4f&		fColor( ) const { return mColorRgba; }
		u32						fColorU32( ) const;
		f32						fTileHeight( ) const { return mTileHeight; }
		f32						fTileSize( ) const { return mTileSize; }

		void	fAddTileSetGuid( u32 tileSetGuid );
		u32		fNumTileSetGuids( ) const { return mTileSetRefs.fCount( ); }
		u32		fTileSetGuid( u32 idx ) const { return mTileSetRefs[idx].mGuid; }
		f32		fTileSetChance( u32 idx ) const { return mTileSetRefs[idx].mRandomWeight; }
		void	fDeleteTileSetGuid( u32 idx );

		b32 fIsSpecificTileSet( ) const { return mTileSetRefs.fCount( ) == 1; }
		void fSeedRandom( );
		u32 fGetTileSetGuidAuto( ); // Gets whatever tile set is necessary, random or spec. Will set random seed if it is unset.

		b32 fHasRandomTiles( tTileTypes forThisType ) { return mHasRandom[ forThisType ]; }
		const tModelList& fSpecificModelList( tTileTypes forThisType ) { return mSpecificModels[ forThisType ]; }
		void fOnTileDbChanged( );

		void fSerialize( Tieml::tTilePigmentDef& file );
		void fDeserialize( Tieml::tTilePigmentDef& file );

		tEditableTileEntityPtr fDeserializeTile( const Tieml::tTile& tile ) const;

		void fUpdatePlacedTile( tEditableTileEntityPtr& tile ) const;

	private:
		void fAddFirstSpecificModels( tEditableTileSet* tileSet );
		void fAdditiveCheckForValidSpecificModels( u32 newTileSetGuid );
		void fFullRebuildValidSpecificModels( );

		u32 fGetSpecificTileSetGuid( ) const { return mTileSetRefs[0].mGuid; }
		u32 fGetRandomTileSetGuid( );
		s32 fSelectRandomTileSet( ) const;
	};

	typedef tRefCounterPtr< tEditableTileSetPigment > tTileSetPigmentPtr;

	///
	/// \class tTileSetPalettes
	/// \brief 
	class tools_export tEditableTileSetPigmentSet : public tRefCounter, public tUncopyable
	{
		tEditableTileDb* mDatabase;
		tGrowableArray< tTileSetPigmentPtr > mPigments;

	public:
		tEditableTileSetPigmentSet( tEditableTileDb* database )
			: mDatabase( database )
		{ }

		void fClear( );

		tEditableTileDb* fDatabase( ) const { return mDatabase; }

		tEditableTileSetPigment*	fAddEmptyPigment( );
		void						fDeletePigment( tEditableTileSetPigment* pigment );
		tEditableTileSetPigment*	fTileSetPigmentByGuid( u32 guid ) const;
		u32							fNumPigments( ) const { return mPigments.fCount( ); }
		tEditableTileSetPigment*	fPigmentByIdx( u32 idx ) const { return mPigments[idx].fGetRawPtr( ); }

		void fOnTileDbChanged( );

		void fSerialize( Tieml::tFile& file ) const;
		void fDeserialize( Tieml::tFile& file );

	private:
		u32 fGetFreeGuid( );
	};
}

#endif // __tEditableTileSetPigment__
