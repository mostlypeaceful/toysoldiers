//------------------------------------------------------------------------------
// \file Tiledb.hpp - 27 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __Tiledb__
#define __Tiledb__
#include "tXmlSerializer.hpp"
#include "tXmlDeserializer.hpp"


namespace Sig
{
	class tTiledbConverter;
}

namespace Sig { namespace TileDb
{

	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsTiledbFile( const tFilePathPtr& path );

	///
	/// \class tTileData
	/// \brief 
	class tools_export tTileData : public Rtti::tSerializableBaseClass
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tTileData, 0xFD74126E );

	public:
		tFilePathPtr mTileFilePath;
		tFilePathPtr mModelFilePath; //!< stored relative to the containing tile set's dir

		virtual void fSerialize( tXmlSerializer& s );
		virtual void fDeserialize( tXmlDeserializer& s );
	};

	///
	/// \class tTileTypeList
	/// \brief 
	class tools_export tTileTypeList : public Rtti::tSerializableBaseClass
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tTileTypeList, 0x417EA460 );

	public:
		u32 mTypeEnum;
		std::string mTypeName;
		tDynamicArray< tTileData > mTileData;

		virtual void fSerialize( tXmlSerializer& s );
		virtual void fDeserialize( tXmlDeserializer& s );
	};

	///
	/// \class tFile
	/// \brief 
	class tools_export tFile
	{
	public:
		u32 mVersion;
		u32 mGuid;
		std::string mFamily;
		std::string mName;
		tFilePathPtr mResDir;
		tFilePathPtr mDiscoveredDir; //!< Only used to check for a conflicting/bad tile set
		tDynamicArray< tTileTypeList > mTileTypesList;

		/// 
		/// Returns true if files were successfully loaded.
		static b32 fLoadTileDbs( tGrowableArray< tFile >& loadedFiles );

	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
	};

}}

#endif // Tiledb
