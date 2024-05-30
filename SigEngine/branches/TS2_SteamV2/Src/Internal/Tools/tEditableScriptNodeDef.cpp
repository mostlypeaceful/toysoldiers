//------------------------------------------------------------------------------
// \file tEditableScriptNodes.cpp - 15 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tEditableScriptNodeDef.hpp"
#include "Guid.hpp"
#include "Gfx/tTextureFile.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tEditableScriptNode
	//------------------------------------------------------------------------------
	tEditableScriptNodeDef::tEditableScriptNodeDef( tEditableScriptNodeDefSet* parent, std::string name, u32 guid, const Math::tVec4f& color )
		: mParent( parent )
		, mName( name )
		, mGuid( guid )
		, mColorRgba( color )
	{
	}

	tEditableScriptNodeDef::~tEditableScriptNodeDef( )
	{
		if( !mTexture.fNull( ) )
			mTexture->fUnload( this );
	}

	void tEditableScriptNodeDef::fSetTexture( const tFilePathPtr& texturePath, const tResourceDepotPtr& resourceDepot )
	{
		mTexturePath = texturePath;

		if( !resourceDepot.fNull( ) )
			mTexture = resourceDepot->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( mTexturePath ), this );
	}

	u32 tEditableScriptNodeDef::fColorU32( ) const
	{
		u32 flatColor = 0;
		flatColor += mColorRgba.x * 0x00FF0000;
		flatColor += mColorRgba.y * 0x0000FF00;
		flatColor += mColorRgba.z * 0x000000FF;
		flatColor += mColorRgba.w * 0xFF000000;

		return flatColor;
	}

	void tEditableScriptNodeDef::fSerialize( Tieml::tScriptNodeDef& file )
	{
		file.mName = mName;
		file.mGuid = mGuid;
		file.mColorRgba = mColorRgba;
		file.mScriptPath = mScriptPath;
		file.mTexture = mTexturePath;
	}

	void tEditableScriptNodeDef::fDeserialize( Tieml::tScriptNodeDef& file, const tResourceDepotPtr& resourceDepot )
	{
		mName = file.mName;
		mGuid = file.mGuid;
		mColorRgba = file.mColorRgba;
		mScriptPath = file.mScriptPath;
		fSetTexture( file.mTexture, resourceDepot );
	}

	//------------------------------------------------------------------------------
	// tEditableScriptNodes
	//------------------------------------------------------------------------------
	void tEditableScriptNodeDefSet::fClear( )
	{
		mScriptNodes.fDeleteArray( );
	}

	tEditableScriptNodeDef* tEditableScriptNodeDefSet::fAddEmptyNode( const tResourceDepotPtr& resourceDepot )
	{
		const u32 guid = fGetFreeGuid( );
		tEditableScriptNodeDef* node = new tEditableScriptNodeDef( this, "new node", guid, Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
		node->fSetTexture( tFilePathPtr( "_tools/SigTile/ScriptNode_d.tga" ), resourceDepot );
		mScriptNodes.fPushBack( tEditableScriptNodeDefPtr( node ) );

		return node;
	}

	void tEditableScriptNodeDefSet::fDeleteNode( tEditableScriptNodeDef* node )
	{
		const b32 success = mScriptNodes.fFindAndEraseOrdered( node );
		sigassert( success );
	}

	const tEditableScriptNodeDef* tEditableScriptNodeDefSet::fNodeByGuid( u32 guid ) const
	{
		for( u32 i = 0; i < mScriptNodes.fCount( ); ++i )
		{
			if( mScriptNodes[i]->fGuid( ) == guid )
				return mScriptNodes[i].fGetRawPtr( );
		}

		return NULL;
	}

	void tEditableScriptNodeDefSet::fSerialize( Tieml::tFile& file ) const
	{
		file.mScriptDefs.fSetCount( mScriptNodes.fCount( ) );
		for( u32 i = 0; i < mScriptNodes.fCount( ); ++i )
			mScriptNodes[i]->fSerialize( file.mScriptDefs[i] );
	}

	void tEditableScriptNodeDefSet::fDeserialize( Tieml::tFile& file, const tResourceDepotPtr& resourceDepot )
	{
		mScriptNodes.fSetCount( file.mScriptDefs.fCount( ) );
		for( u32 i = 0; i < mScriptNodes.fCount( ); ++i )
		{
			mScriptNodes[i] = tEditableScriptNodeDefPtr( new tEditableScriptNodeDef( this, "ERROR", 0, Math::tVec4f::cZeroVector ) );
			mScriptNodes[i]->fDeserialize( file.mScriptDefs[i], resourceDepot );
		}
	}

	u32 tEditableScriptNodeDefSet::fGetFreeGuid( )
	{
		b32 everythingIsOk = false;
		u32 potentialGuid = fGenerateGuid( );
		while( !everythingIsOk )
		{
			everythingIsOk = true;
			potentialGuid = fGenerateGuid( );

			for( u32 i = 0; i < mScriptNodes.fCount( ); ++i )
			{
				if( mScriptNodes[i]->fGuid( ) == potentialGuid )
				{
					everythingIsOk = false; // everything is not ok!
					break;
				}
			}
		}

		return potentialGuid;
	}
}
