#include "BasePch.hpp"
#include "tStaticDecalTris.hpp"
#include "tDecalMaterial.hpp"
#include "tScreen.hpp"
#include "tMaterialFile.hpp"


namespace Sig { namespace Gfx
{
	tStaticDecalGeometry::tStaticDecalGeometry( )
		: mRenderState( tRenderState::cDefaultColorTransparent )
		, mGeometry( tRenderBatchData::cBehaviorRecieveShadow ) // by default
	{
		mGeometry.fSetRenderStateOverride( &mRenderState );
	}

	tStaticDecalGeometry::~tStaticDecalGeometry( )
	{		
		//destructor kept around to squash warning about cleaning up mSysMemVerts
	}

	devvar_clamp( f32, Lod_DecalNormalDrop, 100.f, -0.1f, 10000.f, 1 );
	devvar_clamp( f32, Lod_DecalShadowDrop, 100.f, -0.1f, 10000.f, 1 );

	//------------------------------------------------------------------------------
	void tStaticDecalGeometry::fUpdateLOD( const Math::tVec3f & eye )
	{
		const f32 d = fWorldSpaceBox( ).fDistanceToPoint( eye );

		u32 behaviorFlags = 0;

		if( d <= Lod_DecalShadowDrop )
			behaviorFlags |= Gfx::tRenderBatchData::cBehaviorRecieveShadow;

		if( d > Lod_DecalNormalDrop )
			behaviorFlags |= Gfx::tRenderBatchData::cBehaviorNoNormalMaps;

		mGeometry.fSetBatchBehaviorFlags( behaviorFlags );
		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	void tStaticDecalGeometry::fOnDeviceLost( tDevice* device )
	{
		// do nothing
	}

	void tStaticDecalGeometry::fOnDeviceReset( tDevice* device )
	{
		//if( device )
		//	fCreateGeometry( *device );
	}

	void tStaticDecalGeometry::fResetDeviceObjectsMaterial( 
		const tDevicePtr& device,
		const u16* indices, u32 indexCount,
		const Gfx::tDecalRenderVertex* verts, u32 vertCount,
		Gfx::tDecalMaterial* newMat,
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		// TODO: what to do
		//sigassert( newMat->mDiffuseMap.fGetTextureFile );

		fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
		mGeometry.fResetDeviceObjects( geometryAllocator, indexAllocator );

		fChangeMaterial( newMat );

		fRegisterWithDevice( device.fGetRawPtr( ) );

		fCreateGeometry( *device, indices, indexCount, verts, vertCount );
	}

	void tStaticDecalGeometry::fChangeMaterial( Gfx::tDecalMaterial* newMat )
	{
		mMaterial.fReset( newMat );

		mGeometry.fChangeMaterial( *mMaterial );
		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	void tStaticDecalGeometry::fCreateGeometry( 
		Gfx::tDevice& device,
		const u16* indices, u32 indexCount,
		const Gfx::tDecalRenderVertex* verts, u32 vertCount )
	{
		const u32 primtCount	= indexCount / 3;

		if( !mGeometry.fAllocateGeometry( *mMaterial, vertCount, indexCount, primtCount ) )
		{
			log_warning( "Could not allocate geometry for tWorldSpaceTris" );
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		// we're using a decal
		const u32 vertSize = mGeometry.fGetRenderBatch( )->fBatchData( ).mGeometryBuffer->fVertexFormat( ).fVertexSize( );
		const u32 expectedSize = sizeof( tDecalRenderVertex );
		sigassert( vertSize == expectedSize );

		mGeometry.fCopyVertsToGpu( verts, vertCount );		
		mGeometry.fCopyIndicesToGpu( indices, indexCount );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

}}

