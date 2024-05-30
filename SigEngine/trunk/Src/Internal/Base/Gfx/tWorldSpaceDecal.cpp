#include "BasePch.hpp"
#include "tWorldSpaceDecal.hpp"
#include "tSceneGraph.hpp"
#include "tDefaultAllocators.hpp"
#include "tFullBrightMaterial.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{

	devvar( bool, Debug_Decals_DrawProjectionBox, false );
	devvar_clamp( s32, Gameplay_Decals_DepthBias, -2, -128, 128, 0 );
	devvar( f32, Gameplay_Decals_LifeTime, 30 );
	devvar( f32, Gameplay_Decals_FadeOutTimeRatio, 0.5f );
	devrgba( Gameplay_Decals_Tint, Math::tVec4f( 1.f, 1.f, 1.f, 0.5f ) ); 


	namespace
	{
		template<class tVolume>
		struct tCollectTrisIntersectCallback : public tEntityBVH::tIntersectVolumeCallback<tVolume>
		{
			tGrowableArray<Math::tTrianglef>& mTris;
			const tEntityTagMask	mFlags;

			explicit tCollectTrisIntersectCallback( tGrowableArray<Math::tTrianglef>& tris, u32 hitFlags ) : mTris( tris ), mFlags( hitFlags ) { }
			void operator()( const tVolume& v, tEntityBVH::tObjectPtr octreeObject, b32 aabbWhollyContained ) const
			{
				tSpatialEntity* test = static_cast< tSpatialEntity* >( octreeObject->fOwner( ) );

				if( !test->fHasGameTagsAny( mFlags ) )
					return;

				//if( !this->fQuickAabbTest( v, octreeObject, aabbWhollyContained ) )
				//	return;

				test->fCollectTris( v, mTris );
			}
		};
	}


	tWorldSpaceDecal::tWorldSpaceDecal( const tResourcePtr& colorMap, const Math::tObbf& projectionBox, Gfx::tDevice& device, tEntityTagMask hitFlags )
		: mProjectionBox( projectionBox )
		, mColorMap( colorMap )
		, mDevice( &device )
		, mBoundingBox( -0.1f, 0.1f )
		, mGeometryCreated( false )
		, mHitFlags( hitFlags )
		, mAge( 0.0f )
	{
		Gfx::tDefaultAllocators& defAllocators = Gfx::tDefaultAllocators::fInstance( );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
		mGeometry.fResetDeviceObjects( defAllocators.mFullBrightGeomAllocator, defAllocators.mIndexAllocator );

		fChangeColorMap( colorMap, defAllocators.mFullBrightMaterialFile );

		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mRenderState.fSetDepthBias( Gameplay_Decals_DepthBias );
		//mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );
		//mRenderState.fEnableDisable( Gfx::tRenderState::cFillWireFrame, true );
		mGeometry.fSetRenderStateOverride( &mRenderState );

		fRegisterWithDevice( &device );

		fSetRgbaTint( *this, Gameplay_Decals_Tint );
	}

	tWorldSpaceDecal::~tWorldSpaceDecal( )
	{
	}

	void tWorldSpaceDecal::fChangeColorMap( const tResourcePtr& texResource, const tResourcePtr& materialFile )
	{
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
		fbMtl->mColorMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );

		mGeometry.fChangeMaterial( *mMaterial );
		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	void tWorldSpaceDecal::fOnDeviceLost( tDevice* device )
	{
	}

	void tWorldSpaceDecal::fOnDeviceReset( tDevice* device )
	{
		if( device )
			fCreateGeometry( *device );
	}

	void tWorldSpaceDecal::fOnSpawn( )
	{
		tRenderableEntity::fOnSpawn( );

		fRunListInsert( cRunListActST );
		fRunListInsert( cRunListCoRenderMT );
	}

	void tWorldSpaceDecal::fActST( f32 dt )
	{
		if( Debug_Decals_DrawProjectionBox )
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mProjectionBox, Math::tVec4f( 0.f, 1.f, 0.f, 1.f ) );

		if( !mGeometryCreated )
			fCreateGeometry( *mDevice );

		//handle fading and deleting
		mAge += dt;

		f32 fadeOutTime = Gameplay_Decals_LifeTime * Gameplay_Decals_FadeOutTimeRatio;

		if( mAge > Gameplay_Decals_LifeTime )
		{
			fDelete( );
		}
		else if( mAge >= fadeOutTime )
		{
			sigassert( fadeOutTime > 0.0f );
			f32 fade = fClamp( (Gameplay_Decals_LifeTime - mAge) / fadeOutTime, 0.0f, 1.0f );

			fSetRgbaTint( *this, Math::tVec4f( 1.0f, 1.0f, 1.0f, fade ) * ( Math::tVec4f )Gameplay_Decals_Tint );
		}
	}

	void tWorldSpaceDecal::fCoRenderMT( f32 dt )
	{
		if( mTris.fCount( ) == 0 )
		{
			tCollectTrisIntersectCallback<Math::tObbf> collector( mTris, mHitFlags );
			fSceneGraph( )->fCollectTris( mProjectionBox, collector );
			fBuildGeometryMT( );
		}
	}

	Math::tVec2f tWorldSpaceDecal::fComputeTexCoord( const Math::tVec3f& pos )
	{
		Math::tVec3f localP = mProjectionBox.fPointToLocalVector( pos );
		localP /= mProjectionBox.fExtents( );

		// reproject [-1, 1] to [0, 1]
		localP *= 0.5f;
		localP += Math::tVec3f( 0.5f );

		return Math::tVec2f( localP.x, localP.z );
	}

	void tWorldSpaceDecal::fBuildGeometryMT( )
	{
		mSysMemVerts.fSetCount( mTris.fCount( ) * 3 );
		mBoundingBox.fInvalidate( );

		const u32 color = 0xffffffff;

		for( u32 t = 0; t < mTris.fCount( ); ++t )
		{
			Math::tTrianglef &tri = mTris[ t ];

			tFullBrightRenderVertex *verts = &mSysMemVerts[ t * 3 ];

			verts[ 0 ] = tFullBrightRenderVertex( tri.mA, color, fComputeTexCoord( tri.mA ) );
			verts[ 1 ] = tFullBrightRenderVertex( tri.mB, color, fComputeTexCoord( tri.mB ) );
			verts[ 2 ] = tFullBrightRenderVertex( tri.mC, color, fComputeTexCoord( tri.mC ) );

			mBoundingBox |= verts[ 0 ].mP;
			mBoundingBox |= verts[ 1 ].mP;
			mBoundingBox |= verts[ 2 ].mP;
		}
	}

	void tWorldSpaceDecal::fCreateGeometry( Gfx::tDevice& device )
	{
		u32 triCnt = mTris.fCount( );
		const u32 numVerts	= triCnt * 3;
		const u32 numPrims	= triCnt * 1;
		const u32 numIds	= triCnt * 3;

		sigassert( numVerts == mSysMemVerts.fCount( ) );

		if( !mGeometry.fAllocateGeometry( *mMaterial, numVerts, numIds, numPrims ) )
		{
			log_warning( "Could not allocate geometry for tWorldSpaceDecal" );
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		// we're using a full bright material
		sigassert( mGeometry.fGetRenderBatch( )->fBatchData( ).mGeometryBuffer->fVertexFormat( ).fVertexSize( ) == sizeof( Gfx::tFullBrightRenderVertex ) );

		mGeometry.fCopyVertsToGpu( mSysMemVerts.fBegin( ), numVerts );

		// generate indices
		tDynamicArray<u16> sysMemIndices( numIds );
		for( u32 i = 0; i < numIds; ++i )
			sysMemIndices[ i ] = i;
		mGeometry.fCopyIndicesToGpu( sysMemIndices.fBegin( ), numIds );
		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );

		fSetObjectSpaceBox( mBoundingBox );

		mGeometryCreated = true;
	}

}}
