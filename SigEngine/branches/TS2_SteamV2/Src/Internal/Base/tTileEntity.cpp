//------------------------------------------------------------------------------
// \file tTileEntity.cpp - 24 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTileEntity.hpp"
#include "Scripts/tScriptFile.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tTileEntityDef
	//------------------------------------------------------------------------------
	tTileEntityDef::tTileEntityDef( )
		: mPigmentGuid( 0 )
		, mTileType( -1 )
	{
	}

	tTileEntityDef::tTileEntityDef( tNoOpTag )
		: tSceneRefEntityDef( tNoOpTag( ) )
		, mTileScripts( cNoOpTag )
	{
	}

	void tTileEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		const tResourcePtr& resource = mReferenceFile->fGetResourcePtr( );
		const tSceneGraphFile* sgFile = resource->fCast<tSceneGraphFile>( );
		if( !sgFile ) 
			return;

		tEntity* realParent = &parent;
		tTileEntity* entity = NEW tTileEntity( resource );

		if( mBoneAttachment )
			realParent = &fInsertReferenceFrame( parent, mObjectToLocal );
		else
			entity->fMoveTo( mObjectToLocal );

		entity->fSpawn( *realParent );
		entity->fCollectEntities( creationFlags | fCreationFlags( ), this );

		// TODO: pick a random tile when randomizing. This may need to be done from tSceneGraphFile with a
		// method off the tTilePackage. May also need to override standard model business (at this location
		// or somewhere else, maybe from parent tSceneGraphPhile?) for inside of
		// this entity def.

		// Apply scripts to spawned tile piece.
		for( u32 i = 0; i < mTileScripts.fCount( ); ++i )
		{
			if( !mTileScripts[i] )
				continue;

			Sqrat::Function script = mTileScripts[i]->fGetResourcePtr( )->fCast< tScriptFile >( )->fIndexStandardExportedFunction( tScriptFile::cEntityOnCreated );
			if( !script.IsNull( ) ) 
				script.Execute( entity );
		}
	}

	//------------------------------------------------------------------------------
	// tTileEntity
	//------------------------------------------------------------------------------
	tTileEntity::tTileEntity( const tResourcePtr& sgResource, const tEntity* proxy )
		: tSceneRefEntity( sgResource, proxy )
	{
	}

	void tTileEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tTileEntity, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		vm.fRootTable( ).Bind(_SC("TileEntity"), classDesc);
	}
}
