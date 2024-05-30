//------------------------------------------------------------------------------
// \file tEditableScriptNodeDef.hpp - 15 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEditableScriptNodeDef__
#define __tEditableScriptNodeDef__
#include "Tieml.hpp"
#include "tResourceDepot.hpp"

namespace Sig
{
	class tEditableTileDb;
	class tEditableScriptNodeDefSet;

	///
	/// \class tEditableScriptNodeDef
	/// \brief 
	class tools_export tEditableScriptNodeDef : public tRefCounter, public tUncopyable
	{
		tEditableScriptNodeDefSet*	mParent;

		std::string				mName;
		u32						mGuid;
		Math::tVec4f			mColorRgba;
		tFilePathPtr			mScriptPath;
		tResourcePtr			mTexture;
		tFilePathPtr			mTexturePath;

	public:
		tEditableScriptNodeDef( tEditableScriptNodeDefSet* parent, std::string name, u32 guid, const Math::tVec4f& color );
		~tEditableScriptNodeDef( );

		void fSetScriptPath( const tFilePathPtr& path ) { mScriptPath = path; }
		void fSetName( std::string newName ) { mName = newName; }
		void fSetColor( const Math::tVec4f& newColor ) { mColorRgba = newColor; }
		void fSetTexture( const tFilePathPtr& texturePath, const tResourceDepotPtr& resourceDepot );

		std::string				fName( ) const { return mName; }
		u32						fGuid( ) const { return mGuid; }
		const Math::tVec4f&		fColor( ) const { return mColorRgba; }
		u32						fColorU32( ) const;
		tFilePathPtr			fScriptPath( ) const { return mScriptPath; }
		tResourcePtr			fTexture( ) const { return mTexture; }

		void fSerialize( Tieml::tScriptNodeDef& file );
		void fDeserialize( Tieml::tScriptNodeDef& file, const tResourceDepotPtr& resourceDepot );

		//void fUpdatePlacedTile( tEditableTilePtr& tile ) const;

	private:
	};

	typedef tRefCounterPtr< tEditableScriptNodeDef > tEditableScriptNodeDefPtr;

	///
	/// \class tScriptNodeDefSet
	/// \brief 
	class tools_export tEditableScriptNodeDefSet : public tRefCounter, public tUncopyable
	{
		tEditableTileDb* mDatabase;
		tGrowableArray< tEditableScriptNodeDefPtr > mScriptNodes;

	public:
		tEditableScriptNodeDefSet( tEditableTileDb* database )
			: mDatabase( database )
		{ }

		void fClear( );

		tEditableTileDb* fDatabase( ) const { return mDatabase; }

		tEditableScriptNodeDef*			fAddEmptyNode( const tResourceDepotPtr& resourceDepot );
		void							fDeleteNode( tEditableScriptNodeDef* node );
		const tEditableScriptNodeDef*	fNodeByGuid( u32 guid ) const;
		u32								fNumNodes( ) const { return mScriptNodes.fCount( ); }
		tEditableScriptNodeDef*			fNodeByIdx( u32 idx ) const { return mScriptNodes[idx].fGetRawPtr( ); }

		void fSerialize( Tieml::tFile& file ) const;
		void fDeserialize( Tieml::tFile& file, const tResourceDepotPtr& resourceDepot );

	private:
		u32 fGetFreeGuid( );
	};
}

#endif //__tEditableScriptNodeDef__
