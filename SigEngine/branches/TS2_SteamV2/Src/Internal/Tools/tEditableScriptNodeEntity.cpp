//------------------------------------------------------------------------------
// \file tEditableScriptNodeEntity.cpp - 16 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tEditableScriptNodeEntity.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "tEditableScriptNodeDef.hpp"
#include "Editor/tEditableTileDb.hpp"

namespace Sig
{
	tEditableScriptNodeEntity::tEditableScriptNodeEntity( 
		u32 scriptGuid, 
		const tEditableTileDb* tileDb )
	{	
		fCreateSquare( );

		fSetGuid( scriptGuid, tileDb );
	}

	tEditableScriptNodeEntity::~tEditableScriptNodeEntity( )
	{
		if( !mPanel.fNull( ) )
		{
			mPanel->fDelete( );
			mPanel.fRelease( );
		}
	}

	void tEditableScriptNodeEntity::fSetGuid( u32 newGuid, const tEditableTileDb* tileDb )
	{
		mScriptGuid = newGuid;

		const tEditableScriptNodeDef* thisNodeType = tileDb->fNodeByGuid( mScriptGuid );
		if( !thisNodeType )
		{
			// TODO: error texture
			return;
		}

		fRefresh( thisNodeType, mPanel->fObjectToWorld( ).fGetTranslation( ) );
	}

	void tEditableScriptNodeEntity::fRefresh( const tEditableScriptNodeDef* thisNodeType, const Math::tVec3f& newPos )
	{
		sigassert( thisNodeType );

		// Copy and set up everything needed for display.
		mTextureFile = thisNodeType->fTexture( );
		const u32 color = thisNodeType->fColorU32( );

		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );

		mPanel->fResetDeviceObjectsTexture( 
			Gfx::tDevice::fGetDefaultDevice( ), 
			mTextureFile, 
			allocator.mFullBrightMaterialFile, 
			allocator.mFullBrightGeomAllocator, 
			allocator.mIndexAllocator );

		Gfx::tFullBrightRenderVertex* verts = mPanel->fQuad( 0 );

		verts[ 0 ].mColor = color;
		verts[ 1 ].mColor = color;
		verts[ 2 ].mColor = color;
		verts[ 3 ].mColor = color;

		mPanel->fCreateGeometry( *Gfx::tDevice::fGetDefaultDevice( ) );

		fMoveTo( newPos );
	}

	void tEditableScriptNodeEntity::fMoveTo( const Math::tVec3f& newPos )
	{
		mPanel->fMoveTo( newPos );
	}

	void tEditableScriptNodeEntity::fShowPanel( tEntity& parent )
	{
		if( !mPanel->fSceneGraph( ) )
			mPanel->fSpawnImmediate( parent );
	}

	void tEditableScriptNodeEntity::fHidePanel( )
	{
		if( mPanel->fSceneGraph( ) )
			mPanel->fDeleteImmediate( );
	}

	void tEditableScriptNodeEntity::fMoveTo( const Math::tMat3f& newXform )
	{
		mPanel->fMoveTo( newXform );
	}

	void tEditableScriptNodeEntity::fCreateSquare( )
	{
		Gfx::tWorldSpaceQuads* quads = new Gfx::tWorldSpaceQuads( );
		mPanel.fReset( quads );
		quads->fCreateDefaultQuad( );
	}
}
