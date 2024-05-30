//------------------------------------------------------------------------------
// \file tTileEntity.cpp - 24 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTileEntity.hpp"
#include "Scripts/tScriptFile.hpp"
#include "tApplication.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tTileEntityDef
	//------------------------------------------------------------------------------
	tTileEntityDef::tTileEntityDef( )
		: mTileType( -1 )
		, mIdString( NULL )
	{
	}

	tTileEntityDef::tTileEntityDef( tNoOpTag )
		: tSceneRefEntityDef( cNoOpTag )
		, mTileScripts( cNoOpTag )
		, mTileSet( cNoOpTag )
	{
	}

	void tTileEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		// Pick a random resource to use!
		// Ensure that resource exists in many ways.
		tTilePackage* tileSet = mTileSet.fTreatAsObject();
		const tTilePackage::tTileDef* tileDef;
		if( mIdString )
			tileDef = tileSet->fFindSpecificTileDefByTileType( mTileType, mIdString->fGetStringPtr( ) );
		else
			tileDef = tileSet->fFindRandomTileDefByTileType( mTileType );
		if( !tileDef )
			return;

		const tResourcePtr& resource = tileDef->mSigml->fGetResourcePtr( );
		const tSceneGraphFile* sgFile = resource->fCast<tSceneGraphFile>( );
		if( !sgFile ) 
			return;

		tEntity* realParent = &params.mParent;
		tTileEntity* entity = NEW tTileEntity( this, resource, tileDef );

		entity->fMoveTo( mObjectToLocal );

		entity->fSpawn( *realParent );
		entity->fCollectEntities( params.mCreationFlags | fCreationFlags( ), this );

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
	tTileEntity::tTileEntity( const tTileEntityDef* entityDef, const tResourcePtr& sgResource, const tTilePackage::tTileDef* tileDef, const tEntity* proxy /* = 0 */ )
		: tSceneRefEntity( sgResource, proxy )
		, mEntityDef( entityDef )
		, mTileDef( tileDef )
	{
	}

	Math::tMat3f tTileEntity::fGetAttachmentXform( b32 flipIt ) const
	{
		Math::tMat3f ptMat = fParentRelative( );
		ptMat.fTranslateGlobal( -ptMat.fZAxis( ) * mEntityDef->mBounds.fDepth( ) / 2.f );
		if( flipIt )
			ptMat.fOrientZAxis( ptMat.fZAxis( ) * -1.f );

		const tEntity* parent = fParent( );
		while( parent )
		{
			ptMat = parent->fParentRelative( ) * ptMat;
			parent = parent->fParent( );
		}
				
		return ptMat;
	}

	void tTileEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tTileEntity, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		vm.fRootTable( ).Bind(_SC("TileEntity"), classDesc);
	}
}
