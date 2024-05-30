#include "BasePch.hpp"
#include "tPathDecalEntityDef.hpp"
#include "Gfx/tStaticDecalTris.hpp"
#include "Gfx/tDecalMaterial.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tDefaultAllocators.hpp"

namespace Sig
{
	tPathDecalEntityDef::tPathDecalEntityDef( )
		: mCameraDepthOffset( 0.f )
		, mAcceptsLights( true )
		, mDepthBias( -1 )
		, mSlopeScaleDepthBias( 0 )
		, pad0( false )
		, mDiffuseTexture( NULL )
		, mNormalMap( NULL )
	{
	}

	tPathDecalEntityDef::tPathDecalEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mVerts( cNoOpTag )
		, mTriIndices( cNoOpTag )
	{
	}

	tPathDecalEntityDef::~tPathDecalEntityDef( )
	{
	}

	void tPathDecalEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		Gfx::tStaticDecalGeometry* entity = NEW Gfx::tStaticDecalGeometry( );
		fApplyPropsAndSpawnWithScript( *entity, params );

		entity->fSetCameraDepthOffset( mCameraDepthOffset );
		
		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );
		
		Gfx::tDecalMaterial* decalMat = NEW Gfx::tDecalMaterial( );

		// Keep a reference to the decal material file.
		decalMat->fSetMaterialFileResourcePtrOwned( allocator.mDecalMaterialFile );

		// Set textures and sampling.
		decalMat->mDiffuseMap.fSetLoadInPlace( mDiffuseTexture );
		decalMat->mDiffuseMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );

		if( mNormalMap )
		{
			decalMat->mNormalMap.fSetLoadInPlace( mNormalMap );
			decalMat->mNormalMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );
		}

		decalMat->fSetAcceptsLights( mAcceptsLights );
		
		// Set material.
		entity->fResetDeviceObjectsMaterial(
			Gfx::tDevice::fGetDefaultDevice( ),
			mTriIndices.fBegin( ), mTriIndices.fCount( ),
			mVerts.fBegin( ), mVerts.fCount( ),
			decalMat,
			allocator.mDecalGeomAllocator,
			allocator.mIndexAllocator );

		// Depth bias.
		Gfx::tRenderState newBias = entity->fRenderState( );
		newBias.fSetDepthBias( mDepthBias );
		newBias.fSetSlopeScaleBias( mSlopeScaleDepthBias );
		entity->fSetRenderState( newBias );

		entity->fSetObjectSpaceBox( mBounds );
	}
}

