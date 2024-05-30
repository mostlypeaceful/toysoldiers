//------------------------------------------------------------------------------
// \file tEditableTileCanvas.cpp - 02 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tEditableTileCanvas.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditableTileDb.hpp"
#include "tSigmlConverter.hpp"
#include "WxUtil.hpp"
#include "tTileCanvas.hpp"

namespace Sig { 

	class tModifyTileCanvasAction : public tEditorAction
	{
		tEditableTileCanvasPtr mRealCanvas;
		tTileCanvasState mStartCanvas, mEndCanvas;
	public:
		tModifyTileCanvasAction( tEditableTileCanvas* canvas, const tTileCanvasState& startCanvas, const tTileCanvasState& endCanvas )
			: mRealCanvas( canvas )
			, mStartCanvas( startCanvas )
			, mEndCanvas( endCanvas )
		{
			fSetIsLive( true );
		}
		virtual void fUndo( )
		{
			mRealCanvas->fClear( );
			mRealCanvas->fLoadState( mStartCanvas );
		}
		virtual void fRedo( )
		{
			mRealCanvas->fClear( );
			mRealCanvas->fLoadState( mEndCanvas );
		}
	};

namespace Sigml { 

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tTileCanvasObject& o )
	{
		s( "Tiles", o.mTiles );
		s( "Props", o.mProps );
	}

	register_rtti_factory( tTileCanvasObject, false );

	tTileCanvasObject::tTileCanvasObject( )
	{
	}

	void tTileCanvasObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject(s, *this);
	}

	void tTileCanvasObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject(s, *this);
	}

	tEntityDef* tTileCanvasObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tTileCanvasEntityDef* canvasDef = new tTileCanvasEntityDef();
		fConvertEntityDefBase( canvasDef, sigmlConverter );

		Math::tVec2i dims = mEditableProperties.fGetValue( tEditableTileCanvas::fEditablePropDimensions(), Math::tVec2i(1,1) );
		f32 size = mEditableProperties.fGetValue( tEditableTileCanvas::fEditablePropResolution(), 1.f );
		Math::tVec3f max( (dims.x/2)*size, size, (dims.y/2)*size );
		Math::tVec3f min( -max.x, -size, -max.z );
		canvasDef->mBounds = Math::tAabbf( min, max ); 

		tFilePathPtr tileSetPath = fTileSetPath();
		if( !tileSetPath.fExists() )
			log_warning( "No default tile set. This will make the editor fail to load this sigml.");

		// If there's no tile package, make one.
		if( !sigmlConverter.mTilePackagePath && tileSetPath.fExists() )
			sigmlConverter.mTilePackagePath = sigmlConverter.fAddLoadInPlaceResourceId( tResourceId::fMake< tTilePackage >( tileSetPath ) );

		const u32 numTiles =  mTiles.fCount();
		canvasDef->mTileDefs.fNewArray( numTiles + mProps.fCount() );
		for( u32 i = 0; i < mTiles.fCount(); ++i )
			canvasDef->mTileDefs[i] = (tTileEntityDef*)mTiles[i]->fCreateEntityDef(sigmlConverter);

		for( u32 i = 0; i < mProps.fCount(); ++i )
			canvasDef->mTileDefs[i+numTiles] = (tTileEntityDef*)mProps[i]->fCreateEntityDef(sigmlConverter);

		return canvasDef;
	}

	tEditableObject* tTileCanvasObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableTileCanvas( container, this );
	}

	tFilePathPtr tTileCanvasObject::fTileSetPath( ) const
	{
		tFilePathPtr tileSetPath =  tFilePathPtr( mEditableProperties.fGetValue( tEditableTileCanvas::fEditablePropTileSet(), std::string("") ) );

		// But also don't forget to check the old terrible property.
		if( tileSetPath.fExists() )
			return tileSetPath;

		tTileSetListObject tileSetList = mEditableProperties.fGetValue( tEditableTileCanvas::fEditablePropTileSetListDEFUNCT(), tTileSetListObject() );
		if( tileSetList.mTileSetDataList.fCount() == 0 )
			return tFilePathPtr();

		return tileSetList.mTileSetDataList[0].mFilePath;
	}
} }


namespace Sig
{
	//------------------------------------------------------------------------------
	// tTileTemplate
	//------------------------------------------------------------------------------
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

	static const u32 gDefaultSize = 20;
	static const f32 gDefaultTileSize = 50.f;

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

	//------------------------------------------------------------------------------
	// tEditablePropertyTileSetList
	//------------------------------------------------------------------------------
	register_rtti_factory( tEditablePropertyTileSetList, false )

	tEditablePropertyTileSetList::tEditablePropertyTileSetList( ) { }
	tEditablePropertyTileSetList::tEditablePropertyTileSetList( const std::string& name, const tTileSetListObject& data )
		: tRawDataEditableProperty< tTileSetListObject >( name, data )
	{ }

	void tEditablePropertyTileSetList::fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
	{
		sigassert( size == sizeof( mRawData ) );
		*( tTileSetListObject* )dst = mRawData;
	}

	void tEditablePropertyTileSetList::fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
	{
		sigassert( size == sizeof( mRawData ) );
		mRawData = *( tTileSetListObject* )src;
	}

	tEditableProperty* tEditablePropertyTileSetList::fClone( ) const
	{
		return new tEditablePropertyTileSetList( fGetName(), mRawData );
	}

	void tEditablePropertyTileSetList::fCreateNewGui( const tTileSetDataObject& data )
	{
	}

	//------------------------------------------------------------------------------
	// tEditableTileCanvas
	//------------------------------------------------------------------------------
	tEditableTileCanvas::tEditableTileCanvas( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fAddEditableProperties();
		fCommonCtor();
	}

	tEditableTileCanvas::tEditableTileCanvas( tEditableObjectContainer& container, const Sigml::tTileCanvasObject* tco )
		: tEditableObject( container )
	{
		fAddEditableProperties();
		fDeserializeBaseObject( tco );
		fCommonCtor();

		// Remove defunct property.
		if( mEditableProperties.fFind( fEditablePropTileSetListDEFUNCT() ) )
		{
			// Copy over old version
			tTileSetListObject tileSetList = mEditableProperties.fGetValue( tEditableTileCanvas::fEditablePropTileSetListDEFUNCT(), tTileSetListObject() );
			if( tileSetList.mTileSetDataList.fCount() )
			{
				tFilePathPtr tileset = tileSetList.mTileSetDataList[0].mFilePath;
				mEditableProperties.fSetDataNoNotify( fEditablePropTileSet(), std::string( tileset.fCStr() ) );
			}
			mEditableProperties.fRemove( fEditablePropTileSetListDEFUNCT() );
		}

		for( u32 i = 0; i < tco->mTiles.fCount( ); ++i )
		{
			Sigml::tTileObject* thisTile = tco->mTiles[i]->fDynamicCast<Sigml::tTileObject>();

			// Important that the tiles are stored relative to this guy. Which means we have to translate them
			// back to the world to get the proper paint pos. But better than getting the wrong one.
			Math::tVec3f pos = fObjectToWorld().fXformPoint( thisTile->mXform.fGetTranslation() );
			tEditableTileEntityPtr tile = tEditableTileEntityPtr( (tEditableTileEntity*)thisTile->fCreateEditableObject(mContainer) );
			if( !tile.fNull( ) )
			{
				Math::tVec2u gridCoords = tile->fGetGridCoords();
				fFillPixel( tile, gridCoords.x, gridCoords.y, tile->fGetNumRotations() );
			}
			else
			{
				// TODO: remap
			}
		}

		for( u32 i = 0; i < tco->mProps.fCount(); ++i )
		{
			Sigml::tTileObject* thisProp = tco->mProps[i]->fDynamicCast<Sigml::tTileObject>();

			Math::tVec3f pos = fObjectToWorld().fXformPoint( thisProp->mXform.fGetTranslation() );
			tEditableTileEntityPtr prop = tEditableTileEntityPtr( (tEditableTileEntity*)thisProp->fCreateEditableObject(mContainer) );
			if( !prop.fNull( ) )
				fFillPixel( prop, prop->mGridCoords.x, prop->mGridCoords.y, 0 );
		}
	}

	void tEditableTileCanvas::fCommonCtor()
	{
		mDummyBox.fReset( new tCanvasDummyEntity( mContainer.fGetDummyBoxTemplate().fGetRenderBatch(), mContainer.fGetDummyBoxTemplate().fGetBounds() ) );
		mDummyBox->fSpawnImmediate( *this );
		mDummyBox->fSetInvisible( false );

		mGrid.fReset( new Gfx::tSolidColorGrid() );
		mGrid->fResetDeviceObjects( 
			Gfx::tDevice::fGetDefaultDevice( ),
			mContainer.fGetSolidColorMaterial( ), 
			mContainer.fGetSolidColorGeometryAllocator( ), 
			mContainer.fGetSolidColorIndexAllocator( )  );
		mGrid->fSpawnImmediate( *this );

		const Math::tVec2i dims = fDimensions();
		mOldDims = dims;
		fSetSize( dims.x, dims.y );

		mViewModels = false;
		mHidingProps = false;

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

		fUpdateBoxTint();
	}

	tEditableTileCanvas::~tEditableTileCanvas( )
	{
	}

	void tEditableTileCanvas::fAddEditableProperties( )
	{
		tGrowableArray< tEditablePropertyPtr > propertiesToKeep;
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyString( Sigml::tObject::fEditablePropObjectName() ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropLockTranslation( ), false ) ) );

		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyVec2i( fEditablePropDimensions(), Math::tVec2i(gDefaultSize, gDefaultSize), 0, 100000, 1 ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropResolution(), gDefaultTileSize, 0.001f, 100000.f, 0.01f, 2 ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyFileNameString( fEditablePropTileSet() ) ) );

		mEditableProperties.fAssignPreferExisting( propertiesToKeep );

		tEditablePropertyFileNameString::fAddFilter( fEditablePropTileSet(), "*.tiledml" );
	}

	void tEditableTileCanvas::fClear( )
	{
		const Math::tVec2i dims = fDimensions();

		for( u32 x = 0; x < mTiles.fCount(); ++x )
		{
			for( u32 y = 0; y < mTiles[x].fCount(); ++y )
			{
				fErasePixel( x, y );
			}
		}
	}

	Sigml::tObjectPtr tEditableTileCanvas::fSerialize( b32 clone ) const
	{
		Sigml::tTileCanvasObject* o = new Sigml::tTileCanvasObject( );
		fSerializeBaseObject( o, clone );

		const Math::tVec2i dims = fDimensions();
		tGrowableArray< tEditableTileEntityPtr > visited;
		for( s32 x = 0; x < dims.x; ++x )
		{
			for( s32 y = 0; y < dims.y; ++y )
			{
				if( mTiles[x][y].fNull() )
					continue;

				// Some tile slots may reference a large 2x2 or bigger piece.
				if( visited.fFind( mTiles[x][y] ) )
					continue;

				// Only canvases are allowed to serialize tiles.
				mTiles[x][y]->fSetAllowSerialize( true );
				o->mTiles.fPushBack( mTiles[x][y]->fSerialize(clone) );
				mTiles[x][y]->fSetAllowSerialize( false );
				visited.fPushBack( mTiles[x][y] );
			}
		}

		for( u32 i = 0; i < mProps.fCount(); ++i )
		{
			// Only canvases are allowed to serialize tiles.
			mProps[i]->fSetAllowSerialize( true );
			o->mProps.fPushBack( mProps[i]->fSerialize(clone) );
			mProps[i]->fSetAllowSerialize( false );
		}

		return Sigml::tObjectPtr( o );
	}

	void tEditableTileCanvas::fAddToWorld()
	{
		tEditableObject::fAddToWorld();

		// Props can be hidden and need to be re-added but tiles cannot.
		for( u32 i = 0; i < mReaddProps.fCount(); ++i )
		{
			if( !mReaddProps[i]->fSceneGraph() )
				mReaddProps[i]->fAddToWorld();
		}

		mReaddProps.fSetCount( 0 );
	}

	void tEditableTileCanvas::fRemoveFromWorld()
	{
		tEditableObject::fRemoveFromWorld();
	}

	tGrowableArray< tEntityPtr > tEditableTileCanvas::fGetContents( const b32 propsOnly )
	{
		tGrowableArray< tEntityPtr > returnGuys;

		for( u32 i = 0; i < mProps.fCount(); ++i )
			returnGuys.fPushBack( tEntityPtr( mProps[i].fGetRawPtr() ) );

		const Math::tVec2i dims = fDimensions();
		for( s32 x = 0; x < dims.x; ++x )
		{
			for( s32 y = 0; y < dims.y; ++y )
			{
				if( mTiles[x][y].fNull() )
					continue;

				if( propsOnly && !mTiles[x][y]->fIsProp() )
					continue;

				returnGuys.fFindOrAdd( tEntityPtr( mTiles[x][y].fGetRawPtr() ) );
			}
		}

		return returnGuys;
	}

	tTileCanvasState tEditableTileCanvas::fSaveState( const Math::tVec2i* dimOverride ) const
	{
		tTileCanvasState newState;

		const Math::tVec2i dims = (dimOverride) ? *dimOverride : fDimensions();
		newState.mDims = dims;
		for( s32 x = 0; x < dims.x; ++x )
		{
			for( s32 y = 0; y < dims.y; ++y )
			{
				if( mTiles[x][y].fNull() )
					continue;

				newState.mTileStates.fPushBack( mTiles[x][y]->fSaveState() );
			}
		}

		return newState;
	}

	void tEditableTileCanvas::fLoadState( const tTileCanvasState& state )
	{
		fClear();

		fSetSize( state.mDims.x, state.mDims.y );
		for( u32 i = 0; i < state.mTileStates.fCount(); ++i )
		{
			const tTileState& tileState = state.mTileStates[i];

			tTileTypes type = (tTileTypes)tileState.mTileType;

			tEditableTileEntityPtr newTile( new tEditableTileEntity( mContainer ) );

			newTile->fRestoreState( tileState );
			fFillPixelNoPosCompute( tEditableTileEntityPtr(newTile), tileState.mGridCoords.x, tileState.mGridCoords.y );
		}
	}

	void tEditableTileCanvas::fPaintTilePos( tEditableTileEntityPtr& paintTile, const Math::tVec3f& worldPos, u32 numRotations )
	{
		sigassert( !paintTile.fNull( ) );

		// Need to translate the prop position into the canvas space since props don't rebuild their xforms.
		if( paintTile->fIsProp() )
			paintTile->fMoveTo( fWorldToObject() * paintTile->fObjectToWorld() );

		s32 gridX = 0, gridY = 0;
		fWorldToGrid( worldPos, gridX, gridY );
		fFillPixel( paintTile, gridX, gridY, numRotations );
	}

	void tEditableTileCanvas::fAutopaintTilePos( tEditableTiledml* tileDb, const Math::tVec3f& worldPos )
	{
		s32 gridX = 0, gridY = 0;
		fWorldToGrid( worldPos, gridX, gridY );
		Math::tVec3f local = fWorldToObject().fXformPoint( worldPos );

		for( u32 currX = 0; currX < 3; ++currX )
			for( u32 currY = 0; currY < 3; ++currY )
				mExistingTiles[currX][currY].fRelease( );

		mExistingTiles[1][1] = mTiles[gridX][gridY];

		fFillPixel( tileDb->fGetRandomizedTile( mContainer, cFloor ), gridX, gridY, 0 );

		// Draw out into null tiles.
		for( u32 i = 0; i < cNumDirections; ++i )
		{
			s32 xOffset = 0, yOffset = 0;
			fDirectionToIndices( (tDirections)i, xOffset, yOffset );

			// Ensure we don't index anything crazy
			if( fOutOfBounds( gridX+xOffset, gridY+yOffset ) )
				continue;

			// Skip any tiles that are already filled.
			if( !mTiles[ gridX+xOffset ][ gridY+yOffset ].fNull( ) )
			{
				// Record this for later so we can tell which tiles used to be null.
				mExistingTiles[ xOffset+1 ][ yOffset+1 ] = mTiles[ gridX+xOffset ][ gridY+yOffset ];
				continue;
			}

			Math::tVec3f neighborLocal = fGridToPosition( gridX+xOffset, gridY+yOffset );
			u32 numRotations = 0;
			tEditableTileEntityPtr tile = fGetNullSpaceTileByDirection( numRotations, tileDb, (tDirections)i );
			fFillPixel( tile, gridX+xOffset, gridY+yOffset, numRotations );
		}

		// Do repairs on previous tiles.
		for( u32 i = 0; i < cNumDirections; ++i )
		{
			s32 xOffset = 0, yOffset = 0;
			fDirectionToIndices( (tDirections)i, xOffset, yOffset );

			// Catch only tiles that existed prior to sticking in the null space tiles.
			if( mExistingTiles[ xOffset+1 ][ yOffset+1 ].fNull( ) )
				continue;

			// Ensure we don't index anything crazy
			if( fOutOfBounds( gridX+xOffset, gridY+yOffset ) )
				continue;

			// Also ignore any floors, those have been intentionally painted.
			if( !mTiles[ gridX+xOffset ][ gridY+yOffset ].fNull( ) && mTiles[ gridX+xOffset ][ gridY+yOffset ]->fTileType( ) == cFloor )
				continue;

			// Analyze what tile shape should be here.
			tEditableTileEntityPtr neededTile;
			u32 neededRotations = 0;
			if( !fAnalyzeTile( neededTile, neededRotations, tileDb, gridX+xOffset, gridY+yOffset ) )
				continue;

			fFillPixel( neededTile, gridX+xOffset, gridY+yOffset, neededRotations );
		}
	}

	void tEditableTileCanvas::fEraseTile( const Math::tVec3f& worldPos )
	{
		s32 gridX = 0, gridY = 0;
		fWorldToGrid( worldPos, gridX, gridY );
		fErasePixel( gridX, gridY );
	}

	void tEditableTileCanvas::fEraseProp( tEditableTileEntityPtr& prop )
	{
		for( u32 i = 0; i < mProps.fCount(); ++i )
		{
			if( prop == mProps[i] )
			{
				mProps[i]->fHide( );
				mProps[i]->fDeleteImmediate();

				mProps.fErase(i);
			}
		}
	}

	void tEditableTileCanvas::fRecordReaddProp( tEditableTileEntityPtr& prop )
	{
		mReaddProps.fPushBack( prop );
	}

	void tEditableTileCanvas::fPreviewRandomized( tEditableTiledml* tileDb )
	{
		// Show models when previewing
		fSetViewModel( true );

		tEditableTileSet* tileSet = fPickTileSet( tileDb );
		if( !tileSet )
		{
			log_warning("Somehow failed to find the right guid in the tileset DB");
			return;
		}
		else if( !tileSet->fLoaded() )
		{
			wxMessageBox( "Tile canvas' tile set isn't loaded: " + tileSet->fFileName(), "Unloaded Tile Set Detected" );
			return;
		}

		// Loop through tiles and apply a random model.
		const Math::tVec2i dims = fDimensions();
		for( s32 x = 0; x < dims.x; ++x )
		{
			for( s32 y = 0; y < dims.y; ++y )
			{
				tEditableTileEntityPtr tile = mTiles[x][y];
				fPreview( tile, tileSet );
			}
		}

		// Loop through props
		for( u32 i = 0; i < mProps.fCount(); ++i )
			fPreview( mProps[i], tileSet );
	}

	void tEditableTileCanvas::fPickPropAppareance( tEditableTiledml* tileDb, tEditableTileEntityPtr& object )
	{
		tEditableTileSet* tileSet = fPickTileSet( tileDb );
		if( !tileSet )
		{
			object->fSetViewMode( false, true );
			log_warning("Tile set absent.");
			return;
		}

		fPreview( object, tileSet );
	}

	void tEditableTileCanvas::fPickTileIcon( tEditableTiledml* tileDb, tEditableTileEntityPtr& tile )
	{
		tEditableTileSet* tileSet = fPickTileSet( tileDb );
		if( !tileSet )
		{
			log_warning("tEditableTileCanvas::fPickTileIcon: Tile set absent.");
			return;
		}

		const tEditableTileDef* tileDef = tileSet->fGetSpecificTileDef( tile->mType, tile->mIdString );
		if( !tileDef )
		{
			log_warning( "tEditableTileCanvas::fPickTileIcon: no tile def found for id str: " << tile->mIdString );
			return;
		}

		tile->fSetTexture( tileDef->mTileResource, true );
	}

	void tEditableTileCanvas::fSetViewModel( b32 showModel )
	{
		mViewModels = showModel;

		fUpdateBoxTint();

		// Tiles.
		for( u32 x = 0; x < mTiles.fCount( ); ++x )
		{
			for( u32 y = 0; y < mTiles[x].fCount( ); ++y )
			{
				if( !mTiles[x][y].fNull() )
					mTiles[x][y]->fSetViewMode( mViewModels );
			}
		}

		// Props.
		for( u32 i = 0; i < mProps.fCount(); ++i )
			mProps[i]->fSetViewMode( mViewModels );
	}

	void tEditableTileCanvas::fSetHideProps( b32 hideProps )
	{
		mHidingProps = hideProps;

		for( u32 i = 0; i < mProps.fCount(); ++i )
		{
			if( mHidingProps )
				mProps[i]->fHide();
			else
				mProps[i]->fShow();
		}
	}

	b32 tEditableTileCanvas::fOutOfBounds( u32 x, u32 y ) const
	{
		if( x < 0 || y < 0 )
			return true;

		Math::tVec2i dims = fDimensions();
		if( x >= (u32)dims.x || y >= (u32)dims.y )
			return true;

		return false;
	}

	Math::tVec2i tEditableTileCanvas::fDimensions() const
	{
		return mEditableProperties.fGetValue( fEditablePropDimensions(), Math::tVec2i(gDefaultSize, gDefaultSize) );
	}

	f32 tEditableTileCanvas::fResolution() const
	{
		return mEditableProperties.fGetValue( fEditablePropResolution(), gDefaultTileSize );
	}

	tFilePathPtr tEditableTileCanvas::fTileSetPath() const
	{
		return tFilePathPtr( mEditableProperties.fGetValue( fEditablePropTileSet(), std::string("") ) );
	}

	f32 tEditableTileCanvas::fTileHeight( const Math::tVec3f& worldPos )
	{
		s32 gridX = 0, gridY = 0;
		fWorldToGrid( worldPos, gridX, gridY );

		if( fOutOfBounds(gridX, gridY) )
			return 0.f;

		if( mTiles[gridX][gridY].fNull( ) )
			return 0.f;

		return mTiles[gridX][gridY]->fGetHeight();
	}

	void tEditableTileCanvas::fSnapToGrid( Math::tVec3f& pos, const Math::tVec2u& dims )
	{
		const Math::tVec2i canDims = fDimensions();
		if( dims.x > (u32)canDims.x || dims.y > (u32)canDims.y )
			return;

		const f32 res = fResolution();

		if( fIsEven( dims.x ) ) pos.x += res*0.5f;
		if( fIsEven( dims.y ) ) pos.z += res*0.5f;

		s32 gridX = 0, gridY = 0;
		fWorldToGrid( pos, gridX, gridY, dims );
		pos = fGridToPosition( gridX, gridY, true );

		if( fIsEven( dims.x ) ) pos.x -= res*0.5f;
		if( fIsEven( dims.y ) ) pos.z -= res*0.5f;
	}

	Math::tVec2u tEditableTileCanvas::fGetGridCoords( Math::tVec3f& worldPos )
	{
		s32 gridX = 0, gridY = 0;
		fWorldToGrid( worldPos, gridX, gridY );
		return Math::tVec2u( gridX, gridY );
	}

	b32 tEditableTileCanvas::fShiftHorizontal( s32 shiftAmount, b32 moveProps )
	{
		const Math::tVec2i dims = fDimensions();
		const s32 start =	(shiftAmount > 0) ?	dims.x-1	: 0;
		const s32 end =		(shiftAmount > 0) ?	-1		: dims.x;
		const s32 incDir =	(shiftAmount > 0) ?	-1		: 1;

		// Start in a direction and go to the end.
		for( s32 x = start; x != end; x += incDir )
		{
			if( dims.x <= x || x < 0 )
			{
				log_warning( "out of bounds" );
				return false;
			}

			tGrowableArray< tEditableTileEntityPtr >& thisColumn = mTiles[x];
			for( s32 y = 0; y < dims.y; ++y )
			{
				if( x == start && !thisColumn[y].fNull() )
				{
					log_warning( "moving outside bounds" );
					return false;
				}

				if( mTiles[x][y].fNull() )
					continue;

				// Copy it over.
				const s32 newX = x - incDir;
				mTiles[newX][y] = mTiles[x][y];
				mTiles[newX][y]->fSetGridCoords( Math::tVec2u(newX, y), true );
				mTiles[x][y].fRelease();
			}
		}

		if( !moveProps )
			return true;

		fShiftProps( -shiftAmount, 0 );

		return true;
	}

	b32 tEditableTileCanvas::fShiftVertical( s32 shiftAmount, b32 moveProps )
	{
		// Check out this sweet copypaste job
		const Math::tVec2i dims = fDimensions();
		const s32 start =	(shiftAmount > 0) ?	dims.y-1	: 0;
		const s32 end =		(shiftAmount > 0) ?	-1		: dims.y;
		const s32 incDir =	(shiftAmount > 0) ?	-1		: 1;

		// Start in a direction and go to the end.
		for( s32 y = start; y != end; y += incDir )
		{
			for( s32 x = 0; x < dims.x; ++x )
			{
				tGrowableArray< tEditableTileEntityPtr >& thisColumn = mTiles[x];

				if( dims.y <= y || y < 0 )
				{
					log_warning( "out of bounds" );
					return false;
				}

				if( y == start && !thisColumn[y].fNull() )
				{
					log_warning( "moving outside bounds" );
					return false;
				}

				if( mTiles[x][y].fNull() )
					continue;

				// Copy it over.
				const s32 newY = y - incDir;
				mTiles[x][newY] = mTiles[x][y];
				mTiles[x][newY]->fSetGridCoords( Math::tVec2u(x, newY), true );
				mTiles[x][y].fRelease();
			}
		}

		if( !moveProps )
			return true;

		fShiftProps( 0, -shiftAmount );

		return true;
	}

	Math::tAabbf tEditableTileCanvas::fComputeBounding( )
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

	void tEditableTileCanvas::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		if( property.fGetName() == fEditablePropDimensions() )
		{
			// TODO: expand nicely
			Math::tVec2i newDims;
			property.fGetData<Math::tVec2i>( newDims );

			// Ensure the size is even since things get weird on odd dimensions
			if( fIsOdd(newDims.x) )
				newDims.x += 1;
			if( fIsOdd(newDims.y) )
				newDims.y += 1;

			tTileCanvasState startCanvas = fSaveState( &mOldDims );

			fSetSize( newDims.x, newDims.y );

			tTileCanvasState endCanvas = fSaveState();

			mContainer.fGetActionStack( ).fAddAction( tEditorActionPtr( new tModifyTileCanvasAction( this, startCanvas, endCanvas ) ) );
		}
		else if( property.fGetName() == fEditablePropResolution() )
		{
			fUpdateVisualSpacing();
		}
	}

	void tEditableTileCanvas::fSetSize( u32 width, u32 height, b32 clearTiles )
	{
		mEditableProperties.fSetDataNoNotify( fEditablePropDimensions(), Math::tVec2i(width, height) );

		if( clearTiles )
			fClear();
		else
		{
			// Check to clean out any discards
			if( width < (u32)mOldDims.x )
			{
				for( u32 x = width; x < mTiles.fCount(); ++x )
				{
					tGrowableArray< tEditableTileEntityPtr > thisColumn = mTiles[x];
					for( u32 y = 0; y < thisColumn.fCount(); ++y )
						fErasePixel( x, y );
				}
			}
			if( height < (u32)mOldDims.y )
			{
				for( u32 x = 0; x < mTiles.fCount(); ++x )
				{
					tGrowableArray< tEditableTileEntityPtr > thisColumn = mTiles[x];
					for( u32 y = height; y < thisColumn.fCount(); ++y )
						fErasePixel( x, y );
				}
			}
		}

		fShiftProps( ((s32)width - mOldDims.x)/2, ((s32)height - mOldDims.y)/2 );

		mTiles.fSetCount( width );
		for( u32 i = 0; i < width; ++i )
			mTiles[i].fSetCount( height );

		fUpdateVisualSpacing();

		mOldDims = Math::tVec2i( width, height );
	}

	void tEditableTileCanvas::fWorldToGrid( const Math::tVec3f& worldPos, s32& gridX, s32& gridY, const Math::tVec2u& tileDims )
	{
		const f32 res = fResolution();
		const Math::tVec2i dims = fDimensions();

		Math::tVec3f local = fWorldToObject().fXformPoint( worldPos );

		gridX = local.x/res + dims.x/2;
		gridY = local.z/res + dims.y/2;

		if( tileDims.x != 1 || tileDims.y != 1 )
		{
			// Fit X
			s32 startX = gridX - tileDims.x/2;
			if( startX < 0 )
			{
				gridX += -startX;

				// Reset this even though it will probably be bad news if this actually gets
				// modified and then used again. (ie the canvas is smaller than the tile piece)
				startX = 0; 
			}

			s32 endX = startX + tileDims.x;
			if( endX >= dims.x )
				gridX += dims.x - endX;

			// Fit Y
			s32 startY = gridY - tileDims.y/2;
			if( startY < 0 )
			{
				gridY += -startY;
				startY = 0; 
			}

			s32 endY = startY + tileDims.y;
			if( endY >= dims.y )
				gridY += dims.y - endY;
		}
	}

	Math::tVec3f tEditableTileCanvas::fGridToPosition( const s32 gridX, const s32 gridY, b32 worldSpace )
	{
		const f32 res = fResolution();
		const Math::tVec2i dims = fDimensions();

		Math::tVec3f ret;
		ret.x = (gridX - dims.x/2) * res + res*0.5f;
		ret.y = 0.f;
		ret.z = (gridY - dims.y/2) * res + res*0.5f;

		if( worldSpace )
			ret = fObjectToWorld().fXformPoint( ret );

		return ret;
	}

	void tEditableTileCanvas::fShiftProps( s32 shiftX, s32 shiftY )
	{
		if( shiftX == 0 && shiftY == 0 )
			return;

		const f32 size = fResolution();
		for( u32 i = 0; i < mProps.fCount(); ++i )
		{
			Math::tVec3f newPos = mProps[i]->fObjectToWorld().fGetTranslation();
			newPos.x -= size * shiftX;
			newPos.z -= size * shiftY;
			mProps[i]->fMoveTo( newPos );
		}
	}

	void tEditableTileCanvas::fFillPixel( tEditableTileEntityPtr& paintTile, s32 gridX, s32 gridY, u32 numRotations )
	{
		if( !paintTile->fIsProp() && fOutOfBounds(gridX, gridY) )
			return;

		Math::tVec2i canDims = fDimensions();
		const f32 res = fResolution();
		Math::tVec3f pivot( (canDims.x*res)/2.f, 0.f, (canDims.y*res)/2.f );
		paintTile->fSetPivot( pivot );
		paintTile->fSetSize( fResolution() );

		paintTile->fSetCanvas( tEditableTileCanvasPtr(this) );

		if( paintTile->fIsProp() )
		{
			// Needs to get called to pop out the panel since we don't call a RebuildXform function.
			paintTile->fUpdatePanel();

			paintTile->fSpawnImmediate( *this );
			paintTile->fShow();
			mProps.fPushBack( paintTile );
		}
		else
		{
			// Only for tiles since props can currently be moved around in the editor and it doesn't
			// update the coords where they they exist.
			paintTile->fSetGridCoords( Math::tVec2u(gridX, gridY) );

			// Hide models when painting tiles.
			if( mViewModels )
				fSetViewModel( false );

			fFillPixelNoPosCompute( paintTile, gridX, gridY );

			paintTile->fSetNumRotations( numRotations );
			paintTile->fSetHeight( 0.f, true ); // TODO: height
		}
	}

	void tEditableTileCanvas::fFillPixelNoPosCompute( tEditableTileEntityPtr& paintTile, s32 gridX, s32 gridY )
	{
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
					fErasePixel( x, y );

				mTiles[ x ][ y ] = paintTile;
				paintTile->fOccupyCell( Math::tVec2u( x, y ) );
			}
		}

		paintTile->fSetSelectable( false );
		paintTile->fSpawnImmediate( *this );
		paintTile->fShow();
	}

	void tEditableTileCanvas::fErasePixel( s32 gridX, s32 gridY )
	{
		if( !mTiles[ gridX ][ gridY ].fNull( ) )
		{
			mTiles[ gridX ][ gridY ]->fHide( );
			mTiles[ gridX ][ gridY ]->fDeleteImmediate();

			tGrowableArray<Math::tVec2u> occCells = mTiles[ gridX ][ gridY ]->fOccupiedCells( );
			for( u32 i = 0; i < occCells.fCount( ); ++i )
			{
				const Math::tVec2u& thisOcc = occCells[ i ];
				mTiles[ thisOcc.x ][ thisOcc.y ].fRelease( );
			}
		}
	}

	void tEditableTileCanvas::fUpdateBoxTint( )
	{
		if( mViewModels )
			mDummyBox->fSetRgbaTint( Math::tVec4f( 0.f, 0.8f, 0.f, 0.f ) );
		else
			mDummyBox->fSetRgbaTint( Math::tVec4f( 0.f, 0.8f, 0.f, 0.3f ) );
	}

	void tEditableTileCanvas::fUpdateVisualSpacing()
	{
		const Math::tVec2i dims = fDimensions();
		const f32 res = fResolution();
		const f32 halfW = (dims.x * res) * 0.5f;
		const f32 halfH = (dims.y * res) * 0.5f;

		fSetLocalSpaceMinMax( Math::tVec3f( -halfW, 0.f, -halfH ), Math::tVec3f( halfW, 0.f, halfH ) );
		Math::tMat3f box( Math::tMat3f::cIdentity );
		box.fScaleGlobal( Math::tVec3f( halfW, 0.1f, halfH ) );
		box.fTranslateGlobal( Math::tVec3f( 0.f, -0.05f, 0.f ) );
		mDummyBox->fSetParentRelativeXform( box );

		mGrid->fGenerate( (u32)fMin(dims.x/2, dims.y/2), res, 10.f * res, Math::tVec3f::cXAxis, Math::tVec3f::cZAxis );

		// Update the panels' sizes to match the new grid size
		for( u32 x = 0; x < (u32)dims.x; ++x )
		{
			for( u32 y = 0; y < (u32)dims.y; ++y )
			{
				tEditableTileEntityPtr tile = mTiles[x][y];
				if( tile.fNull() )
					continue;

				Math::tVec3f pivot( (dims.x*res)/2.f, 0.f, (dims.y*res)/2.f );
				tile->fSetPivot( pivot );
				tile->fSetSize( res, true );
			}
		}
	}

	void tEditableTileCanvas::fPreview( tEditableTileEntityPtr& object, tEditableTileSet* tileSet )
	{
		// Processes a single tile/prop object with a given tileset and picks either a random or specific asset.

		if( object.fNull() )
			return;

		const tEditableTileDef* previewTile = NULL;

		if( object->fIsRandomTile( ) )
		{
			// Pick a random model to preview.
			previewTile = tileSet->fGetRandomTileDef( object->fTileType() );
		}
		else
		{
			// Show the specific model for this tile set.
			previewTile = tileSet->fGetSpecificTileDef( object->fTileType(), object->mIdString );
		}

		if( !previewTile || !previewTile->mModelEntity )
		{
			if( !previewTile )
			{
				log_warning( "Specific tile ID [\"" << object->mIdString << "\"] missing from tile set [\"" << tileSet->fFileName() << "\"]" );
			}
			else if( !previewTile->mModelEntity )
			{
				log_warning( "Specific tile ID [\"" << object->mIdString << "\"] missing a model somehow in tile set [\"" << tileSet->fFileName() << "\"]" );
			}

			object->fSetViewMode( false );
			return;
		}

		object->fPreviewTileModel( previewTile->mModelEntity );
		object->fSetViewMode( true );
	}

	tEditableTileSet* tEditableTileCanvas::fPickTileSet( tEditableTiledml* db )
	{
		tFilePathPtr tileSetPath = fTileSetPath();

		return db->fTileSetByFilePath( tileSetPath );
	}

	b32 tEditableTileCanvas::fAnalyzeTile( tEditableTileEntityPtr& outNeededTile, u32& outNumRotations, tEditableTiledml* tileDb, s32 gridX, s32 gridY )
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

					// Ensure we don't index anything crazy
					if( fOutOfBounds( gridX+xOffset, gridY+yOffset ) )
						continue;

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
					Math::tVec3f localPos = fGridToPosition( gridX, gridY );
					outNumRotations = gTileTemplates[ tileType ].fRotations();
					outNeededTile = tileDb->fGetRandomizedTile( mContainer, (tTileTypes)tileType );
					return true;
				}

				gTileTemplates[ tileType ].fRotateCW( );
			}
		}

		return false;
	}

	tEditableTileEntityPtr tEditableTileCanvas::fGetNullSpaceTileByDirection( u32& outNumRotations, tEditableTiledml* tileDb, tDirections dir )
	{
		switch( dir )
		{
		case cNW:
			outNumRotations = 0;
			return tileDb->fGetRandomizedTile( mContainer, cNiche );
		case cN:
			outNumRotations = 0;
			return tileDb->fGetRandomizedTile( mContainer, cWall );
		case cNE:
			outNumRotations = 1;
			return tileDb->fGetRandomizedTile( mContainer, cNiche );
		case cW:
			outNumRotations = 3;
			return tileDb->fGetRandomizedTile( mContainer, cWall );
		case cE:
			outNumRotations = 1;
			return tileDb->fGetRandomizedTile( mContainer, cWall );
		case cSW:
			outNumRotations = 3;
			return tileDb->fGetRandomizedTile( mContainer, cNiche );
		case cS:
			outNumRotations = 2;
			return tileDb->fGetRandomizedTile( mContainer, cWall );
		case cSE:
			outNumRotations = 2;
			return tileDb->fGetRandomizedTile( mContainer, cNiche );

		default: break;
		}

		sigassert( 0 );
		return tEditableTileEntityPtr( );
	}
}
