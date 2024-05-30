//------------------------------------------------------------------------------
// \file tTileEntity.hpp - 24 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTileEntity__
#define __tTileEntity__
#include "tSceneRefEntity.hpp"

namespace Sig
{

	///
	/// \class tTileEntityDef
	/// \brief 
	class base_export tTileEntityDef : public tSceneRefEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tTileEntityDef, 0xD07A0032 );

	public:
		tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceResourcePtr> > mTileScripts;
		u32 mPigmentGuid; //!< 0 if this tile is not random.
		u32 mTileType;

	public:
		tTileEntityDef( );
		tTileEntityDef( tNoOpTag );
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};

	///
	/// \class tSceneRefEntity
	/// \brief 
	class base_export tTileEntity : public tSceneRefEntity
	{
		define_dynamic_cast( tTileEntity, tSceneRefEntity );

	public:
		explicit tTileEntity( const tResourcePtr& sgResource, const tEntity* proxy = 0 );

		static void fExportScriptInterface( tScriptVm& vm );
	};
}

#endif //__tTileEntity__
