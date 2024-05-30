//------------------------------------------------------------------------------
// \file tTileEntity.cpp - 02 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tEditableTileEntity.hpp"
#include "tEditableTileDb.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tDevice.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tTileEntity.hpp"
#include "tSigmlConverter.hpp"
#include "tEditableTileCanvas.hpp"

namespace Sig { 
namespace Sigml { 

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tTileObject& o )
	{
		s( "Type", o.mTileType );
		s( "IdString", o.mIdString );
		s( "SpecificTileSet", o.mSpecificTileSet );
		s( "TexturePath", o.mTexturePath );

		s( "Color", o.mColor );
		s( "Size", o.mTileWidth );
		s( "Height", o.mHeight );
		s( "GridCoords", o.mGridCoords );
		s( "Dims", o.mDims ); // TODO: if dims isn't parsed in, set a default??????????
		s( "Rotations", o.mNumRotations );
		s( "Pivot", o.mPivot );
	}

	register_rtti_factory( tTileObject, false );

	tTileObject::tTileObject( )
		: mTileType( -1 )
		, mSpecificTileSet( -1 )
		, mColor( 0 )
		, mTileWidth( 1.f )
		, mGridCoords( 0, 0 )
		, mDims( 1, 1 )
	{
	}

	void tTileObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject(s, *this);
	}

	void tTileObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject(s, *this);
	}

	tEntityDef* tTileObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tTileEntityDef* def = new tTileEntityDef( );
		fConvertEntityDefBase( def, sigmlConverter );

		def->mTileType = mTileType;
		if( mIdString.length() > 0 )
			def->mIdString = sigmlConverter.fAddLoadInPlaceStringPtr( mIdString.c_str() );

		Math::tVec3f max( mTileWidth/2.f, mTileWidth, mTileWidth/2.f );
		Math::tVec3f min( -max.x, -mTileWidth, -max.z );
		def->mBounds = Math::tAabbf( min, max ); 

		return def;
	}

	tEditableObject* tTileObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableTileEntity( container, this );
	}
} }


namespace Sig
{
	//------------------------------------------------------------------------------
	// tEditableTileEntity
	//------------------------------------------------------------------------------
	tEditableTileEntity::tEditableTileEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
		, mTileWidth( 1.f )
		, mGridCoords( -1, -1 )
		, mHeight( 0.f )
		, mNumRotations( 0 )
		, mAllowSerialize( false )
		, mPivot( Math::cInfinity )
	{
		fAddEditableProperties();
	}

	tEditableTileEntity::tEditableTileEntity( tEditableObjectContainer& container, const Sigml::tTileObject* to )
		: tEditableObject( container )
	{
		fAddEditableProperties();
		fDeserializeBaseObject( to );

		fConfigure(
			(tTileTypes)to->mTileType,
			container.fGetResourceDepot()->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( to->mTexturePath ), this ),
			to->mIdString,
			to->mColor,
			to->mDims
			);

		fSetHeight( to->mHeight );
		fSetGridCoords( to->mGridCoords );
		fSetNumRotations( to->mNumRotations );
		fSetSize( to->mTileWidth );
		fSetPivot( to->mPivot, !fIsProp() );

		// Pop out the panel since props don't recompute xforms.
		if( fIsProp() )
			fUpdatePanel();

		mAllowSerialize = false;
	}

	tEditableTileEntity::~tEditableTileEntity( )
	{
		if( !mPanel.fNull( ) )
		{
			mPanel->fDelete( );
			mPanel.fRelease( );
		}

		if( !mModel.fNull( ) )
		{
			mModel->fDelete( );
			mModel.fRelease( );
		}
	}

	void tEditableTileEntity::fAddEditableProperties( )
	{
		tGrowableArray< tEditablePropertyPtr > propertiesToKeep;
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyString( Sigml::tObject::fEditablePropObjectName() ) ) );

		mEditableProperties.fAssignPreferExisting( propertiesToKeep );
	}

	void tEditableTileEntity::fConfigure(
		tTileTypes type,
		tResourcePtr& texture,
		const std::string& idString,
		u32 color,
		const Math::tVec2u& dims
		)
	{
		mType = type;
		mColor = color;
		mTileWidth = 1.f;
		mDims = dims;
		mSpecificTileSet = ~0u;
		mIdString = idString;
		mShowingModels = false;

		fSetTexture( texture, false );
	}

	Sigml::tObjectPtr tEditableTileEntity::fSerialize( b32 clone ) const
	{
		// Don't serialize unless this is a clone operation or the canvas parent
		// tells us that it's ok to serialize.
		if( !mAllowSerialize && !clone )
			return Sigml::tObjectPtr();

		Sigml::tTileObject* o = new Sigml::tTileObject( );
		fSerializeBaseObject( o, clone );

		o->mXform = fParentRelative(); // Needs to remain relative to the canvas
		o->mTileType = mType;
		o->mSpecificTileSet = mSpecificTileSet;
		o->mIdString = mIdString;

		o->mTexturePath = mTextureFile->fGetPath();

		o->mColor = mColor;
		o->mHeight = mHeight;
		o->mGridCoords = mGridCoords; // Needs to remain relative to the canvas
		o->mDims = mDims;
		o->mNumRotations = mNumRotations;
		o->mTileWidth = mTileWidth;
		o->mPivot = mPivot;

		return Sigml::tObjectPtr( o );
	}

	tEntityPtr tEditableTileEntity::fClone( )
	{
		// Tiles don't get to exist in the object container. 
		// To prevent them from getting serialized outside a TileCanvas.
		tEntityPtr clone = tEditableObject::fClone();
		tEditableTileEntity* cloneTile = clone->fDynamicCast<tEditableTileEntity>();

		if( mParentCanvas )
			cloneTile->fSetCanvas( mParentCanvas );

		if( (fIsProp() || (mType == cUniques && fIsSpecificModel())) && mModel )
		{
			cloneTile->fPreviewTileModel( mModel );
			cloneTile->fSetViewMode( true );

			if( mParentCanvas )
				mParentCanvas->fPaintTilePos( tEditableTileEntityPtr( cloneTile ), cloneTile->fObjectToWorld().fGetTranslation(), 0 );
		}

		return clone;
	}

	void tEditableTileEntity::fAddToWorld()
	{
		if( mParentCanvas && mParentCanvas->fIsHidden() )
		{
			if( fIsProp() )
				mParentCanvas->fRecordReaddProp( tEditableTileEntityPtr(this) );
			return;
		}

		tEditableObject::fAddToWorld();

		// I hate this.
		b32 hid = fIsHidden();
		if( mParentCanvas && !hid)
			mParentCanvas->fPaintTilePos( tEditableTileEntityPtr( this ), mParentCanvas->fObjectToWorld().fXformPoint( fObjectToWorld().fGetTranslation() ), 0 );
	}

	void tEditableTileEntity::fRemoveFromWorld()
	{
		if( mParentCanvas && fSceneGraph() )
			mParentCanvas->fEraseProp( tEditableTileEntityPtr(this) );

		tEditableObject::fRemoveFromWorld();
	}

	tTileState tEditableTileEntity::fSaveState( ) const
	{
		tTileState tileState;

		tileState.mHeight = mHeight;
		tileState.mGridCoords = mGridCoords; // Needs to remain relative to the canvas
		tileState.mDims = mDims;
		tileState.mNumRotations = mNumRotations;
		tileState.mTileWidth = mTileWidth;
		tileState.mPivot = mPivot;

		tileState.mColor = mColor;
		tileState.mIdString = mIdString; //!< Relative to the tile set's base directory.
		tileState.mTileType = mType;
		tileState.mTexturePath = mTextureFile->fGetPath();
		tileState.mSpecificTileSet = mSpecificTileSet;

		return tileState;
	}

	void tEditableTileEntity::fRestoreState( const tTileState& newState )
	{
		// TODO
		tSgFileRefEntityPtr modelEnt;
		//if( !newState.mModelPath.fNull() )
		//	modelEnt = tSgFileRefEntityPtr( new tSgFileRefEntity( mContainer.fGetResourceDepot()->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>(newState.mModelPath), this ) ) );

		fConfigure(
			(tTileTypes)newState.mTileType,
			mContainer.fGetResourceDepot()->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( newState.mTexturePath ), this ),
			newState.mIdString,
			newState.mColor,
			newState.mDims
			);

		fSetHeight( newState.mHeight );
		fSetGridCoords( newState.mGridCoords );
		fSetNumRotations( newState.mNumRotations );
		fSetSize( newState.mTileWidth );
		fSetPivot( newState.mPivot, true );
	}

	std::string tEditableTileEntity::fGetToolTip( ) const
	{
		if( fIsSpecificModel() )
		{
			std::string retTip = mIdString;
			if( mModel )
			{
				retTip += ": ";
				retTip += mModel->fResourcePath().fCStr();
			}

			return retTip;
		}

		if( fIsProp() )
			return "Non-Specific Prop";

		return "Non-Specific Tile";
	}

	Math::tVec2u tEditableTileEntity::fRotatedDims( ) const
	{		
		Math::tMat3f currentRotation = fObjectToWorld( );

		Math::tVec3f dims( mDims.x, 0.f, mDims.y );
		dims = currentRotation.fXformVector( dims );

		return Math::tVec2u( (u32)fRound<f32>( fAbs( dims.x ) ), (u32)fRound<f32>( fAbs( dims.z ) ) );
	}

	void tEditableTileEntity::fSetColor( u32 color )
	{
		Gfx::tFullBrightRenderVertex* verts = mPanel->fQuad( 0 );

		verts[ 0 ].mColor = color;
		verts[ 1 ].mColor = color;
		verts[ 2 ].mColor = color;
		verts[ 3 ].mColor = color;

		mPanel->fCreateGeometry( *Gfx::tDevice::fGetDefaultDevice( ) );
	}

	void tEditableTileEntity::fSetHeight( f32 height, b32 andComputeXform )
	{
		mHeight = height;

		if( andComputeXform )
			fRebuildXform();
	}

	void tEditableTileEntity::fSetSize( f32 size, b32 andComputeXform )
	{
		mTileWidth = size;

		if( andComputeXform )
			fRebuildXform();
	}

	void tEditableTileEntity::fSetGridCoords( const Math::tVec2u& gridCoords, b32 andComputeXform )
	{
		mGridCoords = gridCoords;
		
		if( andComputeXform )
			fRebuildXform();
	}

	void tEditableTileEntity::fSetNumRotations( u32 numRotations, b32 andComputeXform )
	{
		mNumRotations = numRotations;

		if( andComputeXform )
			fRebuildXform();
	}

	void tEditableTileEntity::fSetPivot( const Math::tVec3f& pivot, b32 andComputeXform )
	{
		mPivot = pivot;

		if( andComputeXform )
			fRebuildXform();
	}

	void tEditableTileEntity::fSetTexture( const tResourcePtr& texture, b32 andComputeXform )
	{
		mTextureFile = texture;
		fCreateSquare( mColor );

		if( andComputeXform )
			fRebuildXform();
	}

	Math::tMat3f tEditableTileEntity::fComputeXform( ) const
	{
		const Math::tVec2u rotatedDims = fRotatedDims();
		const f32 xHalfOffset = fIsEven(rotatedDims.x) ? 0.f : mTileWidth/2.f;
		const f32 zHalfOffset = fIsEven(rotatedDims.y) ? 0.f : mTileWidth/2.f;

		Math::tVec3f gridPos;
		if( fIsProp() )
			gridPos = mObjectToWorld.fGetTranslation();
		else if( mGridCoords.x != -1 && mGridCoords.y != -1 && mPivot != Math::tVec3f( Math::cInfinity ) )
			gridPos = Math::tVec3f( mGridCoords.x*mTileWidth + xHalfOffset - mPivot.x - 0.5f, mHeight, mGridCoords.y*mTileWidth + zHalfOffset - mPivot.z );
		else
			gridPos = Math::tVec3f::cZeroVector;

		Math::tQuatf quatRot( Math::tAxisAnglef(Math::tVec3f::cYAxis, mNumRotations * -Math::cPiOver2) );

		Math::tPRSXform<f32> prs( gridPos, quatRot, Math::tVec3f(1.f, 1.f, 1.f) );
		Math::tMat3f newXform( prs );

		return newXform;
	}

	void tEditableTileEntity::fUpdatePanel( )
	{
		Math::tMat3f tinyXform( Math::tMat3f::cIdentity );
		tinyXform.fScaleGlobal( Math::tVec3f( mTileWidth, mTileWidth, mTileWidth ) );
		tinyXform.fTranslateGlobal( Math::tVec3f(0.f, 0.01f, 0.f) );
		mPanel->fSetParentRelativeXform( tinyXform );
	}

	void tEditableTileEntity::fRebuildXform( )
	{
		// Update panel scale.
		fUpdatePanel();

		// Update position/rotation.
		fSetParentRelativeXform( fComputeXform() );
	}

	namespace
	{
		static u32 fRenderableEntityCount( tEntity& root )
		{
			u32 o = 0;
			if( root.fDynamicCast< Gfx::tRenderableEntity >( ) )
				++o;
			for( u32 i = 0; i < root.fChildCount( ); ++i )
				o += fRenderableEntityCount( *root.fChild( i ) );
			return o;
		}
	}

	void tEditableTileEntity::fPreviewTileModel( const tSgFileRefEntityPtr& model )
	{
		if( mShowingModels )
			fHideModel();

		// A bizarre check. If we are showing models but the panel is out, we need to hide the panel before
		// setting up the new randomized model.
		if( mShowingModels && mPanel->fSceneGraph( ) )
			fHidePanel( );

		const tSceneGraphFile* sgFile = model->fSgResource( )->fCast< tSceneGraphFile >( );
		if( sgFile )
		{
			mModel = tSgFileRefEntityPtr( new tSgFileRefEntity( model->fSgResource( ) ) );

			// Properly account for scene files with no renderable entities in it.
			if( fRenderableEntityCount( *model ) > 0 )
			{
				fSetLocalSpaceMinMax( sgFile->mBounds.mMin, sgFile->mBounds.mMax );
				mDummyBox->fSetInvisible( true );
			}
			else
			{
				const Math::tAabbf objSpaceBox = model->fCombinedObjectSpaceBox( );
				fSetLocalSpaceMinMax( objSpaceBox.mMin, objSpaceBox.mMax );
				mDummyBox->fSetParentRelativeXform( objSpaceBox.fAdjustMatrix( Math::tMat3f::cIdentity, 0.0f ) );
				mDummyBox->fSetInvisible( false );
				mDummyBox->fSetRgbaTint( Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.3f ) );
			}
		}
		else
		{
			log_warning( "fPreviewTileModel: failed to cast model's sg resource" );
		}

		if( mShowingModels )
			fShowModel();
	}

	void tEditableTileEntity::fShow()
	{
		if( !mShowingModels )
			fShowPanel();
		else
			fShowModel();
	}

	void tEditableTileEntity::fHide( )
	{
		if( !mShowingModels )
			fHidePanel( );
		else
			fHideModel( );
	}

	void tEditableTileEntity::fSetViewMode( b32 showModels, b32 explicitOverride )
	{
		// Check first for props.
		if( !explicitOverride && fIsProp() && mShowingModels && !showModels && !mModel.fNull() )
			return;

		// Leave specific uniques on too
		if( !showModels && mType == cUniques && fIsSpecificModel() )
			return;

		mShowingModels = showModels;

		if( mShowingModels )
		{
			if( mModel.fNull() )
				return;

			fHidePanel();
			fShowModel();
		}
		else
		{
			fShowPanel();
			fHideModel();
		}
	}

	Math::tAabbf tEditableTileEntity::fGetBounding( ) const
	{
		return mPanel->fWorldSpaceBox( );
	}

	void tEditableTileEntity::fCreateSquare( u32 color )
	{
		if( mPanel )
		{
			mPanel->fDeleteImmediate();
			mPanel.fRelease();
		}

		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );
		Gfx::tWorldSpaceQuads* quads = new Gfx::tWorldSpaceQuads( );

		quads->fResetDeviceObjectsTexture( 
			Gfx::tDevice::fGetDefaultDevice( ), 
			mTextureFile, 
			allocator.mFullBrightMaterialFile, 
			allocator.mFullBrightGeomAllocator, 
			allocator.mIndexAllocator );
		mPanel.fReset( quads );

		mPanel->fCreateDefaultQuad( );

		fSetColor( color );

		Gfx::tFullBrightRenderVertex* verts = quads->fQuad( 0 );
		verts[ 0 ].mP.x *= mDims.x;
		verts[ 0 ].mP.z *= mDims.y;
		verts[ 1 ].mP.x *= mDims.x;
		verts[ 1 ].mP.z *= mDims.y;
		verts[ 2 ].mP.x *= mDims.x;
		verts[ 2 ].mP.z *= mDims.y;
		verts[ 3 ].mP.x *= mDims.x;
		verts[ 3 ].mP.z *= mDims.y;

		mPanel->fCreateGeometry( *Gfx::tDevice::fGetDefaultDevice( ) );
		mPanel->fSpawnImmediate( *this );
	}

	void tEditableTileEntity::fShowPanel( )
	{
		if( !mPanel->fSceneGraph() )
			mPanel->fSpawnImmediate( *this );
		mPanel->fSetRgbaTint( Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
	}

	void tEditableTileEntity::fHidePanel( )
	{
		mPanel->fSetRgbaTint( Math::tVec4f( 1.f, 1.f, 1.f, 0.15f ) );
	}

	void tEditableTileEntity::fShowModel( )
	{
		if( !mModel.fNull() && !mModel->fSceneGraph() && mModel->fSgResource()->fLoaded() )
			mModel->fSpawnImmediate( *this );

		fHidePanel();
	}

	void tEditableTileEntity::fHideModel( )
	{
		if( !mModel.fNull( ) && mModel->fSceneGraph( ) )
			mModel->fDeleteImmediate( );

		fShowPanel();
	}
}
