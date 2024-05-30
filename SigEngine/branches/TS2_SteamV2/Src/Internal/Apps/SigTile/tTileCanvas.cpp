//------------------------------------------------------------------------------
// \file tTileCanvas.cpp - 02 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tTileCanvas.hpp"
#include "tSigTileMainWindow.hpp"
#include "Tieml.hpp"
#include "tTilePaintPanel.hpp"
#include "tTileDbPanel.hpp"

namespace Sig
{
	enum tDirections
	{
		cNW,
		cN,
		cNE,
		cE,
		cSE,
		cS,
		cSW,
		cW,

		cNumDirections,
	};

	class tTileTemplate
	{
		struct tTileRestrictions
		{
			// These are treated as "or".
			tGrowableArray< tTileTypes > mTypesRequired;
			tGrowableArray< tTileTypes > mTypesDisallowed;

			b32 fMatch( tTileTypes type ) const
			{
				if( mTypesRequired.fCount( ) > 0 && !mTypesRequired.fFind( type ) )
					return false;

				if( mTypesDisallowed.fCount( ) > 0 && mTypesDisallowed.fFind( type ) )
					return false;

				return true;
			}
		};

		tTileRestrictions mRestrictions[cNumDirections];
		u32 mRotations;
	public:
		tTileTemplate( ) : mRotations( 0 ) { }

		void fSetRestriction( tDirections dir, tTileTypes tile, b32 required )
		{
			if( required )
				mRestrictions[dir].mTypesRequired.fPushBack( tile );
			else
				mRestrictions[dir].mTypesDisallowed.fPushBack( tile );
		}

		void fRotateCW( )
		{
			mRotations = ++mRotations % 4;
		}

		u32 fRotations( ) const { return mRotations; }

		void fResetRotation( )
		{
			mRotations = 0;
		}

		b32 fMatch( tDirections dir, tTileTypes testedType )
		{
			tDirections plus = (tDirections)(dir - mRotations*2);
			u32 clamped = (dir - mRotations*2) % cNumDirections;
			return mRestrictions[ clamped ].fMatch( testedType );
		}
	};

	static const u32 gDefaultSize = 500;

	static b32 gSetUp = true;
	tTileTemplate gTileTemplates[ cNumBasicTypes ];


	//////////////////////////////////////////////////////////////////////////
	//	
	//		-1,-1	0,-1	1,-1
	//			-------------
	//			|   |   |   |
	//			|---+---+---|
	//     -1,0 |   |   |   | 1,0
	//			|---+---+---|
	//			|   |   |   |
	//			-------------
	//		-1,1	0,1		1,1
	//
	//////////////////////////////////////////////////////////////////////////
	void fDirectionToIndices( tDirections dir, s32& xOffset, s32& yOffset )
	{
		switch( dir )
		{
		case cNW:
			xOffset = -1; yOffset = -1; break;
		case cN:
			xOffset = 0; yOffset = -1; break;
		case cNE:
			xOffset = 1; yOffset = -1; break;
		case cW:
			xOffset = -1; yOffset = 0; break;
		case cE:
			xOffset = 1; yOffset = 0; break;
		case cSW:
			xOffset = -1; yOffset = 1; break;
		case cS:
			xOffset = 0; yOffset = 1; break;
		case cSE:
			xOffset = 1; yOffset = 1; break;

		default: break;
		}
	}

	tTileCanvas::tTileCanvas( tSigTileMainWindow* parent, u32 width, u32 height )
		: mMainWindow( parent )
		, mWidth( 0 )
		, mHeight( 0 )
		, mResolution( 1.f )
		, mActionMask( 0 )
	{
		fSetSize( width, height );

		if( gSetUp )
		{
			gTileTemplates[ cCorner ].fSetRestriction( cE, cFloor, true );
			//gTileTemplates[ cCorner ].fSetRestriction( cNW, cFloor, true );
			gTileTemplates[ cCorner ].fSetRestriction( cS, cFloor, true );
			gTileTemplates[ cCorner ].fSetRestriction( cW, cFloor, false );
			gTileTemplates[ cCorner ].fSetRestriction( cNW, cFloor, false );
			gTileTemplates[ cCorner ].fSetRestriction( cN, cFloor, false );

			gTileTemplates[ cNiche ].fSetRestriction( cSE, cFloor, true );
			gTileTemplates[ cNiche ].fSetRestriction( cS, cFloor, false );
			gTileTemplates[ cNiche ].fSetRestriction( cSW, cFloor, false );
			gTileTemplates[ cNiche ].fSetRestriction( cW, cFloor, false );
			gTileTemplates[ cNiche ].fSetRestriction( cNW, cFloor, false );
			gTileTemplates[ cNiche ].fSetRestriction( cN, cFloor, false );
			gTileTemplates[ cNiche ].fSetRestriction( cNE, cFloor, false );
			gTileTemplates[ cNiche ].fSetRestriction( cE, cFloor, false );

			gTileTemplates[ cWall ].fSetRestriction( cS, cFloor, true );
			gTileTemplates[ cWall ].fSetRestriction( cW, cFloor, false );
			gTileTemplates[ cWall ].fSetRestriction( cNW, cFloor, false );
			gTileTemplates[ cWall ].fSetRestriction( cN, cFloor, false );
			gTileTemplates[ cWall ].fSetRestriction( cNE, cFloor, false );
			gTileTemplates[ cWall ].fSetRestriction( cE, cFloor, false );
			
			gSetUp = false;
		}
	}

	tTileCanvas::~tTileCanvas( )
	{
	}

	void tTileCanvas::fClear( )
	{
		for( s32 x = 0; x < mWidth; ++x )
		{
			for( s32 y = 0; y < mHeight; ++y )
			{
				fErasePixel( x, y );
				fEraseScriptIdx( x, y );
			}
		}

		fSetSize( gDefaultSize, gDefaultSize );
	}

	void tTileCanvas::fSerialize( Tieml::tFile& file )
	{
		file.mGridSize[0] = mWidth;
		file.mGridSize[1] = mHeight;

		++mActionMask;

		for( s32 x = 0; x < mWidth; ++x )
		{
			for( s32 y = 0; y < mHeight; ++y )
				fSerializeTile( file, x, y );
		}
	}

	void tTileCanvas::fDeserialize( Tieml::tFile& file )
	{
		fSetSize( file.mGridSize[0], file.mGridSize[1] );

		mMainWindow->fDataBase( )->fSeedRandomTiles( );

		for( u32 i = 0; i < file.mTiles.fCount( ); ++i )
		{
			const Tieml::tTile& thisTile = file.mTiles[i];
			Math::tVec3f pos = thisTile.mXform.fGetTranslation( );
			tEditableTileEntityPtr tile = mMainWindow->fDataBase( )->fDeserializeTile( thisTile );
			if( !tile.fNull( ) )
			{
				fPaintTilePos( tile, pos.x, pos.z );
			}
			else
			{
				// TODO: remap
			}
		}
	}

	void tTileCanvas::fPaintTilePos( tEditableTileEntityPtr& paintTile, f32 x, f32 y )
	{
		sigassert( !paintTile.fNull( ) );

		s32 gridX = 0, gridY = 0;
		fWorldToGrid( x, y, gridX, gridY );
		fFillPixel( paintTile, gridX, gridY );
	}

	void tTileCanvas::fAutopaintTilePos( f32 x, f32 y )
	{
		s32 gridX = 0, gridY = 0;
		fWorldToGrid( x, y, gridX, gridY );

		for( u32 currX = 0; currX < 3; ++currX )
			for( u32 currY = 0; currY < 3; ++currY )
				mExistingTiles[currX][currY].fRelease( );

		mExistingTiles[1][1] = mTiles[gridX][gridY];

		fFillPixel( mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cFloor, x, y ), gridX, gridY );


		for( u32 i = 0; i < cNumDirections; ++i )
		{
			s32 xOffset = 0, yOffset = 0;
			fDirectionToIndices( (tDirections)i, xOffset, yOffset );

			// Skip any tiles that are already filled.
			if( !mTiles[ gridX+xOffset ][ gridY+yOffset ].fNull( ) )
			{
				// Record this for later so we can tell which tiles used to be null.
				mExistingTiles[ xOffset+1 ][ yOffset+1 ] = mTiles[ gridX+xOffset ][ gridY+yOffset ];
				continue;
			}

			fFillPixel( fGetNullSpaceTileByDirection( (tDirections)i, x+xOffset, y+yOffset ), gridX+xOffset, gridY+yOffset );
		}


		for( u32 i = 0; i < cNumDirections; ++i )
		{
			s32 xOffset = 0, yOffset = 0;
			fDirectionToIndices( (tDirections)i, xOffset, yOffset );

			// Catch only tiles that existed prior to sticking in the null space tiles.
			if( mExistingTiles[ xOffset+1 ][ yOffset+1 ].fNull( ) )
				continue;

			// Also ignore any floors, those have been intentionally painted.
			if( !mTiles[ gridX+xOffset ][ gridY+yOffset ].fNull( ) && mTiles[ gridX+xOffset ][ gridY+yOffset ]->fTileType( ) == cFloor )
				continue;

			// Analyze what tile shape should be here.
			tEditableTileEntityPtr neededTile;
			if( !fAnalyzeTile( neededTile, gridX+xOffset, gridY+yOffset ) )
				continue;

			fFillPixel( neededTile, gridX+xOffset, gridY+yOffset );
		}
	}

	void tTileCanvas::fEraseTile( f32 x, f32 y )
	{
		s32 gridX = 0, gridY = 0;
		fWorldToGrid( x, y, gridX, gridY );
		fErasePixel( gridX, gridY );
	}

	void tTileCanvas::fPaintScriptPos( tEditableScriptNodeEntityPtr& script, f32 x, f32 y )
	{
		if( script.fNull( ) )
			return;

		s32 gridX = 0, gridY = 0;
		fWorldToGrid( x, y, gridX, gridY );
		fPaintScriptIdx( script, gridX, gridY );
	}

	void tTileCanvas::fEraseScript( f32 x, f32 y )
	{
		s32 gridX = 0, gridY = 0;
		fWorldToGrid( x, y, gridX, gridY );
		fEraseScriptIdx( gridX, gridY );
	}

	void tTileCanvas::fBakeRandomized( )
	{
		++mActionMask;

		const b32 viewMode = mMainWindow->fViewMode( );
		mMainWindow->fDataBase( )->fSeedRandomTiles( );

		for( s32 x = 0; x < mWidth; ++x )
		{
			for( s32 y = 0; y < mHeight; ++y )
			{
				if( mTiles[x][y].fNull( ) || mTiles[x][y]->fActionMask( ) == mActionMask )
					continue;

				if( mTiles[x][y]->fIsRandomTile( ) )
				{
					if( viewMode == cModels )
						mTiles[x][y]->fHideModel( );

					mMainWindow->fDataBase( )->fBakeRandomTileForEditor( mTiles[x][y] );

					if( viewMode == cModels )
						mTiles[x][y]->fShowModel( mMainWindow->fGuiApp( ).fSceneGraph( ) );
				}

				mTiles[x][y]->fSetActionMask( mActionMask );
			}
		}
	}

	void tTileCanvas::fShowTile( tEditableTileEntity* tile )
	{
		tile->fShow( mMainWindow->fGuiApp( ).fSceneGraph( ) );
	}

	void tTileCanvas::fHideTile( tEditableTileEntity* tile )
	{
		tile->fHide( );
	}

	f32 tTileCanvas::fTileHeight( f32 x, f32 y )
	{
		s32 gridX = 0, gridY = 0;
		fWorldToGrid( x, y, gridX, gridY );

		if( mTiles[gridX][gridY].fNull( ) )
			return 0.f;

		return mTiles[gridX][gridY]->fObjectToWorld( ).fGetTranslation( ).y;
	}

	void tTileCanvas::fRefreshTiles( )
	{
		++mActionMask;

		for( u32 x = 0; x < mTiles.fCount( ); ++x )
		{
			for( u32 y = 0; y < mTiles[x].fCount( ); ++y )
			{
				if( mTiles[x][y].fNull( ) || mActionMask == mTiles[x][y]->fActionMask( ) )
					continue;

				const tEditableTileSetPigment* pigment = mMainWindow->fDataBase( )->fPigmentByGuid( mTiles[x][y]->fPigmentGuid( ) );
				if( pigment )
					pigment->fUpdatePlacedTile( mTiles[x][y] );

				mTiles[x][y]->fUpdateScripts( mMainWindow->fDataBase( ) );
				
				mTiles[x][y]->fSetActionMask( mActionMask );
			}
		}
	}

	void tTileCanvas::fDeleteTilesWithGuid( b32 pigmentGuid )
	{
		// Don't need to track an action for this one because any affected
		// tiles will be deleted immediately and can't be hit multiple times.
		for( u32 x = 0; x < mTiles.fCount( ); ++x )
		{
			for( u32 y = 0; y < mTiles[x].fCount( ); ++y )
			{
				if( mTiles[x][y].fNull( ) )
					continue;

				if( mTiles[x][y]->fPigmentGuid( ) == pigmentGuid )
					fErasePixel( x, y );
			}
		}
	}

	void tTileCanvas::fDeleteScriptNodesWithGuid( b32 nodeGuid )
	{
		++mActionMask;

		for( u32 x = 0; x < mTiles.fCount( ); ++x )
		{
			for( u32 y = 0; y < mTiles[x].fCount( ); ++y )
			{
				if( mTiles[x][y].fNull( ) || mActionMask == mTiles[x][y]->fActionMask( ) )
					continue;

				mTiles[x][y]->fDeleteScriptsByGuid( nodeGuid );

				mTiles[x][y]->fSetActionMask( mActionMask );
			}
		}
	}

	void tTileCanvas::fSetViewMode( b32 showModels )
	{
		const tSceneGraphPtr& sceneGraph = mMainWindow->fGuiApp( ).fSceneGraph( );

		++mActionMask;

		for( u32 x = 0; x < mTiles.fCount( ); ++x )
		{
			for( u32 y = 0; y < mTiles[x].fCount( ); ++y )
			{
				if( !mTiles[x][y].fNull( ) && mActionMask != mTiles[x][y]->fActionMask( ) )
				{
					mTiles[x][y]->fSetViewMode( showModels, sceneGraph );
					mTiles[x][y]->fSetActionMask( mActionMask );
				}
			}
		}
	}

	void tTileCanvas::fSnapToGrid( Math::tVec3f& pos, const Math::tVec2u& dims )
	{
		if( fIsEven( dims.x ) ) pos.x += mResolution*0.5f;
		if( fIsEven( dims.y ) ) pos.z += mResolution*0.5f;

		s32 gridX = 0, gridY = 0;
		fWorldToGrid( pos.x, pos.z, gridX, gridY );
		fGridToWorld( gridX, gridY, pos.x, pos.z );

		if( fIsEven( dims.x ) ) pos.x -= mResolution*0.5f;
		if( fIsEven( dims.y ) ) pos.z -= mResolution*0.5f;
	}

	Math::tAabbf tTileCanvas::fComputeBounding( )
	{
		Math::tAabbf o;
		o.fInvalidate( );
		for( u32 x = 0; x < mTiles.fCount( ); ++x )
		{
			for( u32 y = 0; y < mTiles[x].fCount( ); ++y )
			{
				if( !mTiles[x][y].fNull( ) )
					o |= mTiles[x][y]->fGetBounding( );
			}
		}
		return o;
	}

	void tTileCanvas::fSerializeTile( Tieml::tFile& file, s32 x, s32 y )
	{
		if( mTiles[x][y].fNull( ) || mTiles[x][y]->fActionMask( ) == mActionMask )
			return;

		tEditableTileEntity* theTile = mTiles[x][y].fGetRawPtr( );

		Tieml::tTile tile;
		tile.mXform				= theTile->fObjectToWorld( );
		tile.mTileType			= theTile->fTileType( );
		tile.mPigmentGuid		= theTile->fPigmentGuid( );
		tile.mSpecificModel		= theTile->fSpecModel( );
		tile.mSpecificTileSet	= theTile->fSpecTileSet( );
		tile.mRandType			= theTile->fRandType( );

		// This will discard the model path for non-specific tiles.
		if( tile.mRandType == cNotRandom || tile.mSpecificModel )
			tile.mModelPath	= theTile->fModelPath( );

		// TODO: save scripts
		tile.mAttachedScriptGuids = theTile->fSerializeScripts( );

		file.mTiles.fPushBack( tile );

		theTile->fSetActionMask( mActionMask );
	}

	void tTileCanvas::fSetSize( u32 width, u32 height )
	{
		mWidth = width;
		mHeight = height;
		mTiles.fSetCount( mWidth );
		for( s32 i = 0; i < mWidth; ++i )
			mTiles[i].fSetCount( mHeight );
	}

	void tTileCanvas::fWorldToGrid( const f32 worldX, const f32 worldY, s32& gridX, s32& gridY )
	{
		gridX = worldX/mResolution + mWidth/2;
		gridY = worldY/mResolution + mHeight/2;
	}

	void tTileCanvas::fGridToWorld( const s32 gridX, const s32 gridY, f32& worldX, f32& worldY )
	{
		worldX = (gridX - mWidth/2) * mResolution + mResolution*0.5f;
		worldY = (gridY - mHeight/2) * mResolution + mResolution*0.5f;
	}

	void tTileCanvas::fFillPixel( tEditableTileEntityPtr& paintTile, s32 gridX, s32 gridY )
	{
		f32 worldX = 0.f, worldY = 0.f;
		fGridToWorld( gridX, gridY, worldX, worldY );

		Math::tVec2u dims = paintTile->fRotatedDims( );
		const u32 startX = gridX - dims.x/2;
		const u32 endX = startX + dims.x;
		for( u32 x = startX; x < endX; ++x )
		{
			const u32 startY = gridY - dims.y/2;
			const u32 endY = startY + dims.y;
			for( u32 y = startY; y < endY; ++y )
			{
				if( !mTiles[ x ][ y ].fNull( ) )
				{
					paintTile->fCopyScripts( mTiles[x][y] );
					fErasePixel( x, y );
				}

				mTiles[ x ][ y ] = paintTile;
				paintTile->fOccupyCell( Math::tVec2u( x, y ) );
			}
		}

		paintTile->fShow( mMainWindow->fGuiApp( ).fSceneGraph( ) );
	}

	void tTileCanvas::fErasePixel( s32 gridX, s32 gridY )
	{
		if( !mTiles[ gridX ][ gridY ].fNull( ) )
		{
			mTiles[ gridX ][ gridY ]->fHide( );

			tGrowableArray<Math::tVec2u> occCells = mTiles[ gridX ][ gridY ]->fOccupiedCells( );
			for( u32 i = 0; i < occCells.fCount( ); ++i )
			{
				const Math::tVec2u& thisOcc = occCells[ i ];
				mTiles[ thisOcc.x ][ thisOcc.y ].fRelease( );
			}
		}
	}

	void tTileCanvas::fPaintScriptIdx( tEditableScriptNodeEntityPtr& script, s32 gridX, s32 gridY )
	{
		if( mTiles[ gridX ][ gridY ].fNull( ) )
			return;

		mTiles[ gridX ][ gridY ]->fAddScript ( script );
	}

	void tTileCanvas::fEraseScriptIdx( s32 gridX, s32 gridY )
	{
		if( mTiles[ gridX ][ gridY ].fNull( ) )
			return;

		mTiles[ gridX ][ gridY ]->fDeleteAllScripts( );
	}

	tEditableTileEntityPtr tTileCanvas::fGetNullSpaceTileByDirection( tDirections dir, f32 x, f32 y )
	{
		switch( dir )
		{
		case cNW:
			return mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cNiche, x, y, 0 );
		case cN:
			return mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cWall, x, y, 0 );
		case cNE:
			return mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cNiche, x, y, 1 );
		case cW:
			return mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cWall, x, y, 3 );
		case cE:
			return mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cWall, x, y, 1 );
		case cSW:
			return mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cNiche, x, y, 3 );
		case cS:
			return mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cWall, x, y, 2 );
		case cSE:
			return mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( cNiche, x, y, 2 );

		default: break;
		}

		sigassert( 0 );
		return tEditableTileEntityPtr( );
	}

	b32 tTileCanvas::fAnalyzeTile( tEditableTileEntityPtr& neededTile, s32 gridX, s32 gridY )
	{
		// Loop through all templates.
		for( u32 tileType = cWall; tileType < cNumBasicTypes; ++tileType )
		{
			gTileTemplates[ tileType ].fResetRotation( );

			// Loop through all rotations of all templates.
			for( u32 rotations = 0; rotations < 4; ++rotations )
			{
				// Match the existing config against all template directions.
				b32 matchesAll = true;
				for( u32 dir = 0; dir < cNumDirections; ++dir )
				{
					s32 xOffset = 0, yOffset = 0;
					fDirectionToIndices( (tDirections)dir, xOffset, yOffset );

					// Grab the examined tile's type.
					tTileTypes foundType = cNumTileTypes;
					if( !mTiles[ gridX+xOffset ][ gridY+yOffset ].fNull( ) )
						foundType = mTiles[ gridX+xOffset ][ gridY+yOffset ]->fTileType( );

					// Check this template matches this tile.
					if( !gTileTemplates[ tileType ].fMatch( (tDirections)dir, foundType ) )
					{
						matchesAll = false;
						break;
					}
				}

				if( matchesAll )
				{
					f32 worldX = 0.f, worldY = 0.f;
					fGridToWorld( gridX, gridY, worldX, worldY );
					neededTile = mMainWindow->fTilePaintPanel( )->fGetRandomizedTile( (tTileTypes)tileType, worldX, worldY, gTileTemplates[ tileType ].fRotations( ) );
					return true;
				}

				gTileTemplates[ tileType ].fRotateCW( );
			}
		}

		return false;
	}

	void tTileCanvas::fPutExistingBack( u32 gridX, u32 gridY )	
	{
		for( u32 x = 0; x < 3; ++x )
		{
			for( u32 y = 0; y < 3; ++y )
			{
				mTiles[ gridX + x - 1 ][ gridY + y - 1 ] = mExistingTiles[x][y];
				mExistingTiles[x][y].fRelease( );
			}
		}
	}
}
