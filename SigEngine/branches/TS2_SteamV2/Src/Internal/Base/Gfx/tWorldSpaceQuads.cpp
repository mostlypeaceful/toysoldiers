#include "BasePch.hpp"
#include "tWorldSpaceQuads.hpp"
#include "tFullBrightMaterial.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{
	tWorldSpaceQuads::tWorldSpaceQuads( )
		: mQuadCount( 0 )
	{
	}

	tWorldSpaceQuads::~tWorldSpaceQuads( )
	{		
		//destructor kept around to squash warning about cleaning up mSysMemVerts
	}

	void tWorldSpaceQuads::fOnDeviceLost( tDevice* device )
	{
		// do nothing
	}

	void tWorldSpaceQuads::fOnDeviceReset( tDevice* device )
	{
		if( device )
			fCreateGeometry( *device );
	}

	void tWorldSpaceQuads::fResetDeviceObjectsTexture( 
		const tDevicePtr& device,
		const tResourcePtr& colorMap,
		const tResourcePtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		sigassert( material->fLoaded( ) );

		fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
		mGeometry.fResetDeviceObjects( geometryAllocator, indexAllocator );

		fChangeColorMap( colorMap, material );

		fRegisterWithDevice( device.fGetRawPtr( ) );

		fCreateGeometry( *device );
	}

	void tWorldSpaceQuads::fChangeColorMap( const tResourcePtr& texResource, const tResourcePtr& materialFile )
	{
		mColorMap = texResource;

		Gfx::tFullBrightMaterial* fbMtl = 0;
		if( mMaterial )
			fbMtl = static_cast<Gfx::tFullBrightMaterial*>( mMaterial.fGetRawPtr( ) );
		else
		{
			fbMtl = NEW Gfx::tFullBrightMaterial( );
			mMaterial.fReset( fbMtl );
		}

		fbMtl->fSetMaterialFileResourcePtrOwned( materialFile );
		fbMtl->mColorMap.fSetDynamic( mColorMap.fGetRawPtr( ) );
		fbMtl->mColorMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );

		mGeometry.fChangeMaterial( *mMaterial );
		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	void tWorldSpaceQuads::fCreateGeometry( Gfx::tDevice& device )
	{
		const u32 numVerts	= mQuadCount * 4;
		const u32 numPrims	= mQuadCount * 2;
		const u32 numIds	= mQuadCount * 6;

		sigassert( numVerts == mSysMemVerts.fCount( ) );

		if( !mGeometry.fAllocateGeometry( *mMaterial, numVerts, numIds, numPrims ) )
		{
			log_warning( Log::cFlagGraphics, "Could not allocate geometry for tWorldSpaceQuads" );
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		// we're using a full bright material
		sigassert( mGeometry.fGetRenderBatch( )->fBatchData( ).mGeometryBuffer->fVertexFormat( ).fVertexSize( ) == sizeof( Gfx::tFullBrightRenderVertex ) );

		mGeometry.fCopyVertsToGpu( mSysMemVerts.fBegin( ), numVerts );

		// generate indices

		//quad diagram
		// 3----2
		// |\   |
		// | \  |
		// |  \ |
		// |   \|
		// 0----1

		tDynamicArray<u16> sysMemIndices( numIds ); u32 i = 0;
		for( u32 q = 0; q < mQuadCount; ++q )
		{
			u32 iStart = q * 4;

			sysMemIndices[ i++ ] = iStart + 0;
			sysMemIndices[ i++ ] = iStart + 3;
			sysMemIndices[ i++ ] = iStart + 1;
			sysMemIndices[ i++ ] = iStart + 1;
			sysMemIndices[ i++ ] = iStart + 3;
			sysMemIndices[ i++ ] = iStart + 2;
		}
		sigassert( i == numIds );
		mGeometry.fCopyIndicesToGpu( sysMemIndices.fBegin( ), numIds );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	void tWorldSpaceQuads::fSetQuadCount( u32 cnt )
	{
		mQuadCount = cnt;
		mSysMemVerts.fSetCount( mQuadCount * 4 );
	}

	u32 tWorldSpaceQuads::fQuadCount( ) const
	{
		return mQuadCount;
	}

	tFullBrightRenderVertex* tWorldSpaceQuads::fQuad( u32 index )
	{
		sigassert( index < mQuadCount );
		return &mSysMemVerts[ index * 4 ];
	}

	void tWorldSpaceQuads::fCreateDefaultQuad( )
	{
		fSetQuadCount( 1 );

		const Math::tVec3f corners[4] =
		{
			Math::tVec3f( -0.5f, 0.f, -0.5f ),
			Math::tVec3f( +0.5f, 0.f, -0.5f ),
			Math::tVec3f( +0.5f, 0.f, +0.5f ),
			Math::tVec3f( -0.5f, 0.f, +0.5f )
		};

		const Math::tVec2f uvOffset( +0.5f, +0.5f );

		tFullBrightRenderVertex* verts = fQuad( 0 );
		for( u32 i = 0; i < 4; ++i )
		{
			verts[ i ].mP = corners[ i ];
			verts[ i ].mColor = 0xffffffff;
			verts[ i ].mUv = corners[ i ].fXZ( ) + uvOffset;
		}

		Math::tAabbf box;
		box.fInvalidate( );
		for( u32 i = 0; i < 4; ++i )
			box |= corners[ i ];

		fSetObjectSpaceBox( box );
	}


}}

