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

namespace Sig
{
	tEditableTileEntity::tEditableTileEntity( 
		tTileTypes type,
		u32 guid, 
		tResourcePtr& texture, 
		tSgFileRefEntityPtr& model, 
		const tFilePathPtr& modelPath, 
		const Math::tMat3f& objXform, 
		u32 color, 
		f32 tileWidth,
		const Math::tVec2u& dims,
		tRandStage randType,
		b32 specTileSet, 
		b32 specModel )
		: mModelPath( modelPath )
		, mType( type )
		, mPigmentGuid( guid )
		, mTileView( true )
		, mColor( color )
		, mTileWidth( tileWidth )
		, mShrinkScale( 1.f / mTileWidth )
		, mDims( dims )
		, mActionMask( 0 )
		, mRandomType( randType )
		, mSpecificTileSet( specTileSet )
		, mSpecificModel( specModel )
		, mShowingModels( false )
	{
		if( !model.fNull( ) && !modelPath.fNull( ) )
			mModel = tSgFileRefEntityPtr( new tSgFileRefEntity( model->fSgResource( ) ) );

		mTextureFile = texture;
		Math::tVec3f translation = objXform.fGetTranslation( );
		fCreateSquare( translation.x, translation.z, color );
		mPanel->fMoveTo( objXform );

		if( !mModel.fNull( ) )
		{
			Math::tMat3f tinyXform = objXform;
			tinyXform.fScaleLocal( Math::tVec3f( mShrinkScale, mShrinkScale, mShrinkScale ) );
			mModel->fMoveTo( tinyXform );
		}
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

	tEditableTileEntityPtr tEditableTileEntity::fClone( )
	{
		tEditableTileEntity* tileCopy = new tEditableTileEntity( mType, mPigmentGuid, mTextureFile, mModel, mModelPath, mPanel->fObjectToWorld( ), mColor, mTileWidth, mDims, mRandomType, mSpecificTileSet, mSpecificModel );
		return tEditableTileEntityPtr( tileCopy );
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

	void tEditableTileEntity::fSetHeight( f32 height )
	{
		Math::tVec3f translation = mPanel->fObjectToWorld( ).fGetTranslation( );
		translation.y = height;
		mPanel->fMoveTo( translation );

		if( !mModel.fNull( ) )
			mModel->fMoveTo( translation );
	}

	void tEditableTileEntity::fSetSize( f32 size )
	{
		mTileWidth = size;
		mShrinkScale = 1.f / mTileWidth;

		if( !mModel.fNull( ) )
		{
			Math::tMat3f tinyXform = mPanel->fObjectToWorld( );
			tinyXform.fScaleLocal( Math::tVec3f( mShrinkScale, mShrinkScale, mShrinkScale ) );
			mModel->fMoveTo( tinyXform );
		}
	}

	void tEditableTileEntity::fBakeRandomizedTile( const tFilePathPtr& modelPath, const tSgFileRefEntityPtr& model )
	{
		// A bizarre check. If we are showing models but the panel is out, we need to hide the panel before
		// setting up the new randomized model.
		if( mShowingModels && mPanel->fSceneGraph( ) )
			fHidePanel( );

		mModelPath = modelPath;
		mModel = tSgFileRefEntityPtr( new tSgFileRefEntity( model->fSgResource( ) ) );

		Math::tMat3f tinyXform = mPanel->fObjectToWorld( );
		tinyXform.fScaleLocal( Math::tVec3f( mShrinkScale, mShrinkScale, mShrinkScale ) );
		mModel->fMoveTo( tinyXform );
	}

	void tEditableTileEntity::fShow( const tSceneGraphPtr& sceneGraph )
	{
		if( mTileView )
			fShowPanel( sceneGraph );
		else
			fShowModel( sceneGraph );

		for( u32 i = 0; i < mScripts.fCount( ); ++i )
			mScripts[i]->fShowPanel( sceneGraph->fRootEntity( ) );
	}

	void tEditableTileEntity::fHide( )
	{
		for( u32 i = 0; i < mScripts.fCount( ); ++i )
			mScripts[i]->fHidePanel( );

		if( mTileView )
			fHidePanel( );
		else
			fHideModel( );
	}

	void tEditableTileEntity::fAddScript( const tEditableScriptNodeEntityPtr& script )
	{
		// TODO: allow script layers
		fDeleteAllScripts( );
		mScripts.fPushBack( script );
		script->fShowPanel( *mPanel );
	}

	void tEditableTileEntity::fDeleteAllScripts( )
	{
		for( u32 i = 0; i < mScripts.fCount( ); ++i )
			mScripts[i]->fHidePanel( );
		mScripts.fDeleteArray( );
	}

	void tEditableTileEntity::fCopyScripts( const tEditableTileEntityPtr& otherTile )
	{
		for( u32 i = 0; i < otherTile->mScripts.fCount( ); ++i )
			mScripts.fFindOrAdd( otherTile->mScripts[i] );

		otherTile->mScripts.fDeleteArray( );
	}

	tGrowableArray<u32> tEditableTileEntity::fSerializeScripts( ) const
	{
		tGrowableArray<u32> ret( mScripts.fCount( ) );
		for( u32 i = 0; i < mScripts.fCount( ); ++i )
			ret[i] = mScripts[i]->fGuid( );

		return ret;
	}

	void tEditableTileEntity::fDeserializeScripts( const tGrowableArray<u32>& guids, const tEditableTileDb* tileDb )
	{
		const Math::tVec3f translation = mPanel->fObjectToWorld( ).fGetTranslation( ) + Math::tVec3f::cYAxis * 0.5f;

		for( u32 i = 0; i < guids.fCount( ); ++i )
		{
			tEditableScriptNodeEntityPtr newScript( new tEditableScriptNodeEntity( guids[i], tileDb ) );
			newScript->fMoveTo( translation );
			mScripts.fPushBack( newScript );
		}
	}

	void tEditableTileEntity::fUpdateScripts( const tEditableTileDb* tileDb )
	{
		for( u32 i = 0; i < mScripts.fCount( ); ++i )
		{
			const tEditableScriptNodeDef* thisNodeType = tileDb->fNodeByGuid( mScripts[i]->fGuid( ) );
			if( thisNodeType )
				mScripts[i]->fRefresh( thisNodeType, mPanel->fObjectToWorld( ).fGetTranslation( ) + Math::tVec3f::cYAxis * 0.5f );
		}
	}

	void tEditableTileEntity::fDeleteScriptsByGuid( u32 scriptGuid )
	{
		for( u32 i = 0; i < mScripts.fCount( ); ++i )
		{
			if( mScripts[i]->fGuid( ) == scriptGuid )
				mScripts.fEraseOrdered( i-- );
		}
	}

	void tEditableTileEntity::fSetViewMode( b32 showModels, const tSceneGraphPtr& sceneGraph )
	{
		mShowingModels = showModels;

 		if( mShowingModels )
		{
			if( mModel.fNull( ) )
				return;

			fHidePanel( );
			fShowModel( sceneGraph );
		}
		else
		{
			fShowPanel( sceneGraph );
			fHideModel( );
		}
	}

	void tEditableTileEntity::fMoveTo( const Math::tVec3f& newPos )
	{
		mPanel->fMoveTo( newPos );

		if( !mModel.fNull( ) )
			mModel->fMoveTo( newPos );
	}

	void tEditableTileEntity::fMoveTo( const Math::tMat3f& newXform )
	{
		mPanel->fMoveTo( newXform );

		if( !mModel.fNull( ) )
			mModel->fMoveTo( newXform );
	}

	void tEditableTileEntity::fRotate( b32 ccw )
	{
		Math::tMat3f obj = mPanel->fObjectToWorld( );
		Math::tQuatf rotation( obj );

		Math::tQuatf addedRotation( Math::tAxisAnglef( Math::tVec3f::cYAxis, (ccw) ? Math::cPiOver2 : -Math::cPiOver2 ) );
		rotation *= addedRotation;

		Math::tMat3f newRot( rotation );
		newRot.fSetTranslation( obj.fGetTranslation( ) );

		mPanel->fMoveTo( newRot );

		if( !mModel.fNull( ) )
		{
			newRot.fScaleLocal( Math::tVec3f( mShrinkScale, mShrinkScale, mShrinkScale ) );
			mModel->fMoveTo( newRot );
		}
	}

	Math::tAabbf tEditableTileEntity::fGetBounding( ) const
	{
		return mPanel->fWorldSpaceBox( );
	}

	void tEditableTileEntity::fCreateSquare( f32 xPos, f32 yPos, u32 color )
	{
		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );
		Gfx::tWorldSpaceQuads* quads = new Gfx::tWorldSpaceQuads( );

		quads->fResetDeviceObjectsTexture( 
			Gfx::tDevice::fGetDefaultDevice( ), 
			mTextureFile, 
			allocator.mFullBrightMaterialFile, 
			allocator.mFullBrightGeomAllocator, 
			allocator.mIndexAllocator );
		mPanel.fReset( quads );

		quads->fCreateDefaultQuad( );

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

		quads->fCreateGeometry( *Gfx::tDevice::fGetDefaultDevice( ) );
		quads->fMoveTo( Math::tVec3f( xPos, 0.05f, yPos ) );
	}

	void tEditableTileEntity::fShowPanel( const tSceneGraphPtr& sceneGraph )
	{
		if( !mPanel->fSceneGraph( ) )
			mPanel->fSpawnImmediate( sceneGraph->fRootEntity( ) );
	}

	void tEditableTileEntity::fHidePanel( )
	{
		if( mPanel->fSceneGraph( ) )
			mPanel->fDeleteImmediate( );
	}

	void tEditableTileEntity::fShowModel( const tSceneGraphPtr& sceneGraph )
	{
		if( !mModel.fNull( ) && !mModel->fSceneGraph( ) )
			mModel->fSpawnImmediate( sceneGraph->fRootEntity( ) );
	}

	void tEditableTileEntity::fHideModel( )
	{
		if( !mModel.fNull( ) && mModel->fSceneGraph( ) )
			mModel->fDeleteImmediate( );
	}
}
