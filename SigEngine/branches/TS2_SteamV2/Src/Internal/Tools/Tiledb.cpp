//------------------------------------------------------------------------------
// \file Tiledb.cpp - 27 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "Tiledb.hpp"
#include "FileSystem.hpp"

namespace Sig { namespace TileDb
{
	const char* fGetFileExtension( )
	{
		return ".tiledb";
	}

	b32 fIsTiledbFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	//------------------------------------------------------------------------------
	// tTileData
	//------------------------------------------------------------------------------
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tTileData& o )
	{
		s( "TilePath", o.mTileFilePath );
		s( "ModelPath", o.mModelFilePath );
	}

	void tTileData::fSerialize( tXmlSerializer& s ) { fSerializeXmlObject( s, *this ); }
	void tTileData::fDeserialize( tXmlDeserializer& s ) { fSerializeXmlObject( s, *this ); }

	//------------------------------------------------------------------------------
	// tTileTypeList
	//------------------------------------------------------------------------------
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tTileTypeList& o )
	{
		s( "TypeEnum", o.mTypeEnum );
		s( "TypeName", o.mTypeName );
		s( "TileData", o.mTileData );
	}

	void tTileTypeList::fSerialize( tXmlSerializer& s ) { fSerializeXmlObject( s, *this ); }
	void tTileTypeList::fDeserialize( tXmlDeserializer& s ) { fSerializeXmlObject( s, *this ); }

	//------------------------------------------------------------------------------
	// tFile
	//------------------------------------------------------------------------------

	namespace
	{
		static u32 gTiledbVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		if( o.mVersion != gTiledbVersion )
		{
			log_warning( 0, "Tiledb file format is out of date -> Please re-export." );
			return;
		}

		s( "Guid", o.mGuid );
		s( "Family", o.mFamily );
		s( "Name", o.mName );
		s( "ResDir", o.mResDir );
		s( "TileTypesData", o.mTileTypesList );
	}

	tFile::tFile( )
		: mVersion( gTiledbVersion )
		, mGuid( 0 )
	{
	}

	b32 tFile::fLoadTileDbs( tGrowableArray< tFile >& loadedFiles )
	{
		const tFilePathPtr currentRootFolder = ToolsPaths::fGetCurrentProjectResFolder( );

		// Grab all names.
		tFilePathPtrList files;
		FileSystem::fGetFileNamesInFolder( files, currentRootFolder, true, true, tFilePathPtr( ".tiledb" ) );

		if( files.fCount( ) == 0 )
			return false;

		for( u32 i = 0; i < files.fCount( ); ++i )
		{
			loadedFiles.fPushBack( TileDb::tFile( ) );
			loadedFiles.fBack( ).mDiscoveredDir = ToolsPaths::fMakeResRelative( tFilePathPtr( StringUtil::fDirectoryFromPath( files[i].fCStr( ) ) ) );
			if( !loadedFiles.fBack( ).fLoadXml( files[i] ) )
				loadedFiles.fPopBack( );
		}

		return loadedFiles.fCount( ) != 0;
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Tiledb", *this, promptToCheckout ) )
		{
			log_warning( 0, "Couldn't save Tiledb file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Tiledb", *this ) )
		{
			log_warning( 0, "Couldn't load Tiledb file [" << path << "]" );
			return false;
		}

		return true;
	}
}}
