//------------------------------------------------------------------------------
// \file Tiledb.cpp - 27 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "Tiledb.hpp"
#include "FileSystem.hpp"
#include "tTilePackage.hpp"

namespace Sig { namespace Tiledml
{
	const char* fGetFileExtension( )
	{
		return ".tiledml";
	}

	b32 fIsTiledmlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	tFilePathPtr fTiledmlPathToTiledb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, tTilePackage::fGetFileExtension( ) );
	}

	tFilePathPtr fTiledbPathToTiledml( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, fGetFileExtension( ) );
	}

	//------------------------------------------------------------------------------
	// tTileData
	//------------------------------------------------------------------------------
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tTileData& o )
	{
		s( "TilePath", o.mTileFilePath );
		s( "ModelPath", o.mModelFilePath );
		s( "IdString", o.mIdString );
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
			log_warning( "Tiledb file format is out of date -> Please re-export." );
			return;
		}

		s( "Guid", o.mGuid );
		s( "FileName", o.mFileName );

		s( "ResDir", o.mResDir );
		s( "TileTypesData", o.mTileTypesList );
	}

	tFile::tFile( )
		: mVersion( gTiledbVersion )
		, mGuid( 0 )
	{
	}

	b32 tFile::fLoadTiledmls( tGrowableArray< tFile >& loadedFiles )
	{
		const tFilePathPtr currentRootFolder = ToolsPaths::fGetCurrentProjectResFolder( );

		// Grab all names.
		tFilePathPtrList files;
		FileSystem::fGetFileNamesInFolder( files, currentRootFolder, true, true, tFilePathPtr( ".tiledml" ) );

		if( files.fCount( ) == 0 )
			return false;

		for( u32 i = 0; i < files.fCount( ); ++i )
		{
			loadedFiles.fPushBack( Tiledml::tFile( ) );
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
			log_warning( "Couldn't save Tiledb file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Tiledb", *this ) )
		{
			log_warning( "Couldn't load Tiledb file [" << path << "]" );
			return false;
		}

		return true;
	}
}}
