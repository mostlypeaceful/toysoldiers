//------------------------------------------------------------------------------
// \file Tieml.hpp - 08 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __Tieml__
#define __Tieml__
#include "tXmlSerializer.hpp"
#include "tXmlDeserializer.hpp"
#include "Sigml.hpp"


namespace Sig
{
	class tTiemlConverter;
}

namespace Sig { namespace Tieml
{

	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsTiemlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fTiemlPathToTieb( const tFilePathPtr& path );
	tools_export tFilePathPtr fTiebPathToTieml( const tFilePathPtr& path );

	///
	/// \class tTile
	/// \brief Holds all information related to a tile of data.
	class tools_export tTile : public Rtti::tSerializableBaseClass
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tTile, 0x6632A20E );

	public:
		tTile( );

		Math::tMat3f	mXform;
		u32				mPigmentGuid;

		tFilePathPtr	mModelPath; //!< Relative to the tile set's base directory.
		u32				mTileType;
		u32				mRandType;
		b32				mSpecificTileSet;
		b32				mSpecificModel;
		tGrowableArray<u32> mAttachedScriptGuids;

	public:
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fDeserialize( tXmlDeserializer& s );

		virtual tEntityDef* fCreateEntityDef( tTiemlConverter& tiemlConverter ) const;
	};
	typedef tGrowableArray< tTile >  tTileList;


	///
	/// \class tPaletteElement
	/// \brief 
	class tools_export tTilePigmentDef : public Rtti::tSerializableBaseClass
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tTilePigmentDef, 0x259A99E0 );

	public:
		tTilePigmentDef( );

		std::string				mName;
		u32						mGuid;
		tGrowableArray< u32 >	mTileSetGuids;
		Math::tVec4f			mColorRgba;
		f32						mHeight;
		f32						mSize;

	public:
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fDeserialize( tXmlDeserializer& s );
	};
	typedef tGrowableArray< tTilePigmentDef >  tTilePigmentDefList;

	///
	/// \class tScriptNodeDef
	/// \brief 
	class tools_export tScriptNodeDef : public Rtti::tSerializableBaseClass
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tScriptNodeDef, 0x259A99E0 );

	public:
		tScriptNodeDef( );

		std::string				mName;
		u32						mGuid;
		Math::tVec4f			mColorRgba;
		tFilePathPtr			mScriptPath;
		tFilePathPtr			mTexture;

	public:
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fDeserialize( tXmlDeserializer& s );
	};
	typedef tGrowableArray< tScriptNodeDef >  tScriptNodeDefList;

	///
	/// \class tFile
	/// \brief Holds a grid of tiles and size of grid. That's all.
	class tools_export tFile
	{
	public:
		u32					mVersion;
		tTileList			mTiles;
		tTilePigmentDefList	mTilePigmentDefs;
		tScriptNodeDefList	mScriptDefs;
		Math::tVec2u		mGridSize;

	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
	};

}}

#endif // Tieml
