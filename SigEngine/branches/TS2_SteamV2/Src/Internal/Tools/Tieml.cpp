//------------------------------------------------------------------------------
// \file Tieml.cpp - 08 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "Tieml.hpp"
#include "tTileEntity.hpp"
#include "tTiemlConverter.hpp"
#include "Scripts/tScriptFile.hpp"

namespace Sig { namespace Tieml
{
	static const f32 gHackTilePosScale = 20.f;

	const char* fGetFileExtension( )
	{
		return ".tieml";
	}

	b32 fIsTiemlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	tFilePathPtr fTiemlPathToTieb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, ".tieb" );
	}

	tFilePathPtr fTiebPathToTieml( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, ".tieml" );
	}

	//------------------------------------------------------------------------------
	// tTile
	//------------------------------------------------------------------------------
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tTile& o )
	{
		s( "WorldPos", o.mXform );
		s( "TileType", o.mTileType );
		s( "RandType", o.mRandType );
		s( "SpecTileSet", o.mSpecificTileSet );
		s( "SpecModel", o.mSpecificModel );
		s( "BrushGuid", o.mPigmentGuid );
		s( "Model", o.mModelPath );
		s( "AttachedScriptGuids", o.mAttachedScriptGuids );
	}

	tTile::tTile( )
		: mTileType( ~0 )
		, mPigmentGuid( 0 )
		, mRandType( true )
	{
	}

	void tTile::fSerialize( tXmlSerializer& s ) { fSerializeXmlObject( s, *this ); }
	void tTile::fDeserialize( tXmlDeserializer& s ) { fSerializeXmlObject( s, *this ); }

	tEntityDef* tTile::fCreateEntityDef( tTiemlConverter& tiemlConverter ) const
	{
		tTileEntityDef* entityDef = new tTileEntityDef( );

		// spatial shite
		entityDef->mBounds.fInvalidate( );
		Math::tMat3f xform = mXform;
		Math::tVec3f pos = xform.fGetTranslation( ) * gHackTilePosScale;
		xform.fSetTranslation( pos );
		entityDef->mObjectToLocal = xform;
		entityDef->mLocalToObject = xform.fInverse( );

		// If this was a tile placed specifically, load that one model.
		if( mRandType == cNotRandom || mRandType == cRandomizeAtAssetGen )
		{
			tResourceId rid;

			const tFilePathPtr sigbPath = Sigml::fSigmlPathToSigb( tiemlConverter.fConstructModelFilePath( *this ) );
			rid = tResourceId::fMake<tSceneGraphFile>( sigbPath );
			entityDef->mReferenceFile = tiemlConverter.fAddLoadInPlaceResourcePtr( rid );
		}
		else
		{
			// Pass down the tile set group to use when this tile is randomized.
			entityDef->mPigmentGuid = mPigmentGuid;
		}

		// Add script resources.
		entityDef->mTileScripts.fNewArray( mAttachedScriptGuids.fCount( ) );
		for( u32 i = 0; i < mAttachedScriptGuids.fCount( ); ++i )
		{
			tFilePathPtr scriptPath = tiemlConverter.fGetScriptPath( mAttachedScriptGuids[i] );
			entityDef->mTileScripts[i] = tiemlConverter.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tScriptFile>( scriptPath ) );
		}

		return entityDef;
	}

	//------------------------------------------------------------------------------
	// tBrush
	//------------------------------------------------------------------------------
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tTilePigmentDef& o )
	{
		s( "Name", o.mName );
		s( "Guid", o.mGuid );
		s( "TileSetGuids", o.mTileSetGuids );
		s( "Color", o.mColorRgba );
		s( "Height", o.mHeight );
		s( "Size", o.mSize );
	}

	tTilePigmentDef::tTilePigmentDef( )
		: mGuid( 0 )
		, mSize( 20.f )
		, mHeight( 0.f )
	{
	}

	void tTilePigmentDef::fSerialize( tXmlSerializer& s ) { fSerializeXmlObject( s, *this ); }
	void tTilePigmentDef::fDeserialize( tXmlDeserializer& s ) { fSerializeXmlObject( s, *this ); }

	//------------------------------------------------------------------------------
	// tScriptNodePaletteElement
	//------------------------------------------------------------------------------
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tScriptNodeDef& o )
	{
		s( "Name", o.mName );
		s( "Guid", o.mGuid );
		s( "Color", o.mColorRgba );
		s( "ScriptPath", o.mScriptPath );
		s( "TexturePath", o.mTexture );
	}

	tScriptNodeDef::tScriptNodeDef( )
	{
	}

	void tScriptNodeDef::fSerialize( tXmlSerializer& s ) { fSerializeXmlObject( s, *this ); }
	void tScriptNodeDef::fDeserialize( tXmlDeserializer& s ) { fSerializeXmlObject( s, *this ); }

	//------------------------------------------------------------------------------
	// tFile
	//------------------------------------------------------------------------------
	namespace
	{
		const u32 gTiemlVersion = 2;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		if( o.mVersion != gTiemlVersion )
		{
			log_warning( 0, "Tieml file format is out of date -> Please re-export." );
			return;
		}

		s( "Tiles", o.mTiles );
		s( "PigmentDefs", o.mTilePigmentDefs );
		s( "ScriptDefs", o.mScriptDefs );
		s( "GridSize", o.mGridSize );
	}

	tFile::tFile( )
		: mVersion( gTiemlVersion )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Tieml", *this, promptToCheckout ) )
		{
			log_warning( 0, "Couldn't save Tieml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Tieml", *this ) )
		{
			log_warning( 0, "Couldn't load Tieml file [" << path << "]" );
			return false;
		}

		return true;
	}
}}
