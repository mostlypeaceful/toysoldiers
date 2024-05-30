//------------------------------------------------------------------------------
// \file tGroundCoverCloud.cpp - 13 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tGroundCoverCloud.hpp"
#include "tSceneRefEntity.hpp"
#include "tMersenneGenerator.hpp"
#include "tProfiler.hpp"
#include "tSceneGraph.hpp"
#include "tGameAppBase.hpp"
#include "tMeshEntity.hpp"
#include "tMesh.hpp"
#include "gui/tColoredQuad.hpp"

namespace Sig { namespace Gfx
{
	devvar( bool, GroundCover_RenderInvalidate, false );
	devvar( bool, GroundCover_Disable, false );
	devvar( bool, GroundCover_LockFrustum, false );
	devvar( bool, GroundCover_DrawInstanced, false );
	devvar( u32, GroundCover_AsyncReallocThreshhold, 100 );

	namespace
	{
		static const f32 cRoot2 = Math::fSqrt( 2.0f );

		static b32 gGloballyDisabled = false;

		static f32 gFarGatherRadii[ tGroundCoverCloudDef::cVisibilityCount ] = { 125.f, 250.f, 500.f, Math::cInfinity };
		static tGroundCoverCloudDef::tVisibility gGlobalFarVisibility = tGroundCoverCloudDef::cVisibilityCount;
		devvarptr_clamp( f32, GroundCover_EndFadeOut_0Near,		gFarGatherRadii[tGroundCoverCloudDef::cVisibilityNear], 1.f, 10000.f, 1 );
		devvarptr_clamp( f32, GroundCover_EndFadeOut_1Medium,	gFarGatherRadii[tGroundCoverCloudDef::cVisibilityMedium], 1.f, 10000.f, 1 );
		devvarptr_clamp( f32, GroundCover_EndFadeOut_2Far,		gFarGatherRadii[tGroundCoverCloudDef::cVisibilityFar], 1.f, 10000.f, 1 );

		static f32 gNearGatherRadii[ tGroundCoverCloudDef::cVisibilityCount ] = { 60.f, 125.f, 250.f, 0.f };
		static tGroundCoverCloudDef::tVisibility gGlobalNearVisibility = tGroundCoverCloudDef::cVisibilityCount;
		devvarptr_clamp( f32, GroundCover_BeginFadeOut_0Near,	gNearGatherRadii[tGroundCoverCloudDef::cVisibilityNear], 1.f, 10000.f, 1 );
		devvarptr_clamp( f32, GroundCover_BeginFadeOut_1Medium, gNearGatherRadii[tGroundCoverCloudDef::cVisibilityMedium], 1.f, 10000.f, 1 );
		devvarptr_clamp( f32, GroundCover_BeginFadeOut_2Far,	gNearGatherRadii[tGroundCoverCloudDef::cVisibilityFar], 1.f, 10000.f, 1 );
		
		static tGroundCoverCloud::tGroundCoverFilter gGroundCoverFilter;
	}
	namespace
	{
		//assumes outMats is pre-sized to max # of states filter could return
		void fInvalidateMatrices(
			const Math::tMat3f& otw,
			const tMat3fArray& mats, 
			tDynamicArray< tGrowableArray< Math::tMat3f > >& outMats,
			u32 (*filter)( f32 x, f32 z ) )
		{
			sigassert( outMats.fCount( ) > 0 && "outMats should be pre-alloc'ed to max # of renderable states" );
			if( filter )
			{
				for( u32 i = 0; i < mats.fCount( ); ++i )
				{
					const Math::tMat3f m = otw * mats[ i ];
					const Math::tVec3f p = m.fGetTranslation( );
					const u32 state = filter( p.x, p.z );
					sigassert( fInBounds<u32>( state, 0, 3 ) );
					outMats[ state ].fPushBack( m );
				}
			}
			else
			{
				sigassert( outMats.fCount( ) == 1 );
				outMats[ 0 ].fInsert( 0, mats.fBegin( ), mats.fCount( ) );
			}
		}
	}

	//------------------------------------------------------------------------------
	// tGroundCoverCloudDef
	//------------------------------------------------------------------------------
	tGroundCoverCloudDef::tVisibility tGroundCoverCloudDef::fFarVisibility( ) const
	{
		if( gGlobalFarVisibility == cVisibilityCount )
			return mFarVisibility;
		return gGlobalFarVisibility;
	}

	//------------------------------------------------------------------------------
	tGroundCoverCloudDef::tVisibility tGroundCoverCloudDef::fNearVisibility( ) const
	{
		if( gGlobalNearVisibility == cVisibilityCount )
			return mNearVisibility;
		return gGlobalNearVisibility;
	}

	//------------------------------------------------------------------------------
	u32 tGroundCoverCloudDef::fCalculateSize( ) const
	{
		return sizeof( *this ) + mElements.fTotalSizeOf( ) + mMask.fTotalSizeOf( ) + mHeights.fTotalSizeOf( );
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloudDef::fAnyElementsCastShadow( ) const
	{
		for( u32 i = 0; i < mElements.fCount( ); ++i )
			if( mElements[ i ].fCastsShadow( ) )
				return true;
		return false;
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloudDef::fConvertMask( 
		u32 dimx, u32 dimz, const f32 inMask[], 
		u32 paintUnits, 
		u32 & maskDimX, u32 & maskDimZ, tDynamicArray<byte> & outMask )
	{
		maskDimX = ( dimx / paintUnits ) + ( ( dimx % paintUnits ) ? 1 : 0 );
		maskDimZ = ( dimz / paintUnits ) + ( ( dimz % paintUnits ) ? 1 : 0 );

		const f32 numSamples = (f32)( paintUnits * paintUnits );

		const u32 outCount = maskDimX * maskDimZ;
		if( outMask.fCount( ) != outCount )
		{
			outMask.fNewArray( outCount );
			sigassert( !outCount || outMask.fBegin( ) );
		}

		for( u32 zM = 0; zM < maskDimZ; ++zM )
		{
			for( u32 xM = 0; xM < maskDimX; ++xM )	
			{
				f32 value = 0;
				for( u32 zP = 0; zP < paintUnits; ++zP )
				{
					const u32 zS = fClamp( zM * paintUnits + zP, 0u, dimz - 1 );
					for( u32 xP = 0; xP < paintUnits; ++xP )
					{
						const u32 xS = fClamp( xM * paintUnits + xP, 0u, dimx - 1 );
						value += inMask[ zS * dimx + xS ];
					}
				}

				outMask[ zM * maskDimX + xM ] = ( byte )( ( value / numSamples ) * 255 );
			}
		}
	}

	//------------------------------------------------------------------------------
	// tRenderableGatherer
	//------------------------------------------------------------------------------
	namespace
	{
		struct tRenderableGatherer
		{
			b32 operator( )( const tEntity & e ) const
			{

				Gfx::tRenderableEntity * renderable = e.fDynamicCast<Gfx::tRenderableEntity>( );
				if( !renderable )
					return false;

				//HACKHACK to prevent rendering all the different states of ground cover
				// in SigEd for Rise. This shouldn't effect any other game (including TS3)
				// unless that game wants to do something fancy with state masks for
				// their ground cover.
	#ifdef target_tools
				if( !renderable->fStateEnabled( 0 ) )
					return false;
	#endif//target_tools
				//END HACKHACK

				mRenderables.fPushBack( renderable );

				return false;
			}

			mutable tGrowableArray<tRenderableEntity*> mRenderables;
		};
	}


	//------------------------------------------------------------------------------
	// tGroundCoverCloud
	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fSetGlobalDisable( b32 disable )
	{
		gGloballyDisabled = disable;
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::fGetGlobalDisable( )
	{
		return GroundCover_Disable || gGloballyDisabled;
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fSetGatherRadius( tGroundCoverCloudDef::tVisibility visibility, f32 radius )
	{
		sigassert( fInBounds<u32>( visibility, 0, tGroundCoverCloudDef::cVisibilityCount - 1 ) );
		gFarGatherRadii[ visibility ] = radius;
	}

	//------------------------------------------------------------------------------
	f32 tGroundCoverCloud::fGetGatherRadius( tGroundCoverCloudDef::tVisibility visibility )
	{
		sigassert( fInBounds<u32>( visibility, 0, tGroundCoverCloudDef::cVisibilityCount - 1 ) );
		return gFarGatherRadii[ visibility ];
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fSetForcedVisibility( tGroundCoverCloudDef::tVisibility visibility  )
	{
		sigassert( fInBounds<u32>( visibility, 0, tGroundCoverCloudDef::cVisibilityCount ) );
		gGlobalFarVisibility = visibility;
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::fGetForcedVisibility( )
	{
		return gGlobalFarVisibility;
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fSetGroundCoverFilter( const tGroundCoverFilter & filter )
	{
		gGroundCoverFilter = filter;
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::fGetRotation( 
		const tGroundCoverCloudDef * def, tMersenneGenerator & random, Math::tEulerAnglesf & angles )
	{
		if( !def->fRotation( ) )
			return false;

		switch( def->fRotation( ) )
		{
		case tGroundCoverCloudDef::cRotationRandomXYZ:
			angles.x = random.fFloatInRange( -def->fMaxXZRotation( ), def->fMaxXZRotation( ) );
			angles.z = random.fFloatInRange( -def->fMaxXZRotation( ), def->fMaxXZRotation( ) );
		case tGroundCoverCloudDef::cRotationRandomY:
			angles.y = random.fFloatInRange( -def->fMaxYRotation( ), def->fMaxYRotation( ) );
			break;
		}

		return !angles.fIsZero( );
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::fGetTranslation( 
		const tGroundCoverCloudDef * def, tMersenneGenerator & random, Math::tVec3f & offset )
	{
		if( !def->fTranslation( ) )
			return false;

		switch( def->fTranslation( ) )
		{
		case tGroundCoverCloudDef::cTranslationXYZ:
			offset.y = random.fFloatInRange( -def->fMaxYTranslation( ), def->fMaxYTranslation( ) );
		case tGroundCoverCloudDef::cTranslationXZ:
			offset.x = random.fFloatInRange( -def->fMaxXZTranslation( ), def->fMaxXZTranslation( ) );
			offset.z = random.fFloatInRange( -def->fMaxXZTranslation( ), def->fMaxXZTranslation( ) );
			break;
		}

		return !offset.fIsZero( );
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::fGetScale( 
		const tGroundCoverCloudDef * def, tMersenneGenerator & random, f32 & scale )
	{
		if( !def->fHasScaleRange( ) )
			return false;

		f32 range = def->fScaleRange( );
		scale = random.fFloatInRange( 1.f - range, 1.f + range );
		return scale != 1;
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fGenSpawnInfo(
		const tGroundCoverCloudDef * def,
		u32 cX, u32 cZ, f32 pX, f32 pZ,
		tGrowableArray<tSpawnInfo> & spawns, 
		b32 skipHeights )
	{
		const tGroundCoverCloudDef::tElement * elements = def->fElements( );
		const u32 elementCount = def->fElementCount( );

		// Account for floating point error
		f32 pXr = s32(pX * 1000.f) / 1000.f;
		f32 pZr = s32(pZ * 1000.f) / 1000.f;

		f32 a = tMersenneGenerator( *(u32*)&pXr ).fFloatInRange( -pZr, pZr );
		f32 b = tMersenneGenerator( *(u32*)&pZr ).fFloatInRange( -pXr, pXr );

		// Account for floating point error
		a = s32(a * 1000.f) / 1000.f;
		b = s32(b * 1000.f) / 1000.f;

		u32 seed = *(u32*)&a ^ *(u32*)&b;

		tMersenneGenerator spawnRand( seed );
		tMersenneGenerator rotRand( seed );
		tMersenneGenerator transRand( seed );
		tMersenneGenerator scaleRand( seed );

		for( u32 p = 0; p < elementCount; ++p )
		{
			const tGroundCoverCloudDef::tElement & element = elements[ p ];

			const u32 spawnCount = element.fSpawnCount( );
			for( u32 s = 0; s < spawnCount; ++s )
			{
				tSpawnInfo& info = spawns.fPushBack( );

				info.mElement = &element;
				info.mLocalXform = Math::tMat3f::cIdentity;
				info.mLocalXform( 0, 3 ) = pX; 
				info.mLocalXform( 2, 3 ) = pZ;

				// We do the spawn if the rand test passes the mask test and the frequency test
				info.mDoSpawn = ( ( byte )( spawnRand.fFloatZeroToOne( )  * 255 ) >= def->fMask( cX, cZ ) );
				info.mDoSpawn = info.mDoSpawn && ( spawnRand.fFloatZeroToOne( ) <= element.fFrequency( ) );

				if( !skipHeights )
					info.mLocalXform( 1, 3 ) = def->fHeight( cX, cZ, spawns.fCount( ) - 1 );

				Math::tVec3f translation( 0 );
				if( fGetTranslation( def, transRand, translation ) )
					info.mLocalXform.fTranslateGlobal( translation );

				Math::tEulerAnglesf angles( 0 );
				if( fGetRotation( def, rotRand, angles ) )
					info.mLocalXform *= Math::tMat3f( Math::tQuatf( angles ) );

				f32 scale;
				if( fGetScale( def, scaleRand, scale ) )
					info.mLocalXform.fScaleLocal( Math::tVec3f( scale ) );
			}
		}
	}

	//------------------------------------------------------------------------------
	tGroundCoverCloud::tGroundCoverCloud( const tGroundCoverCloudDef * def )
		: mDef( def )
		, mFrustumDirty( true )
	{
		mGatherRect.fInvalidate( );
	}

	//------------------------------------------------------------------------------
	tGroundCoverCloud::~tGroundCoverCloud( )
	{
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::fShouldRender( ) const
	{
		return !tGroundCoverCloud::fGetGlobalDisable( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fPrepareRenderables( const Gfx::tCamera & camera )
	{
		profile( cProfilePerfPrepareRenderables );
		profile_pix( "GC fPrepareRenderables" );

		if( !fShouldRender( ) )
			return;

		if( fInstanced( ) )
		{
			profile_pix( "mInstancedGC.fUpdate()" );
			mInstancedGC.fUpdate( camera );
			return;
		}

		// See forever
		if( mDef->fFarVisibility( ) == tGroundCoverCloudDef::cVisibilityNoFade )
		{
			fUpdateGatherRect( 0, 0, mDef->fDimX( ), mDef->fDimZ( ) );
			return;
		}

		//update gc frustum
		if( !GroundCover_LockFrustum )
		{
			const Math::tFrustumf & camFrust = camera.fGetWorldSpaceFrustum( );
			if( !mFrustumDirty )
			{
				for( u32 i = 0; i < Math::tFrustumf::cPlaneCount; ++i )
				{
					if( mFrustum[ i ].a != camFrust[ i ].a ||
						mFrustum[ i ].b != camFrust[ i ].b ||
						mFrustum[ i ].c != camFrust[ i ].c ||
						mFrustum[ i ].d != camFrust[ i ].d )
					{
						mFrustumDirty = true;
						break;
					}
				}
			}
			mFrustum = camFrust;
		}

		const u32 dimx = mDef->fDimX( );
		const u32 dimz = mDef->fDimZ( );

		const f32 gatherRadius = gFarGatherRadii[ mDef->fFarVisibility( ) ];
		const f32 unitSize = mDef->fUnitSize( );

		const Math::tVec3f worldEye = camera.fGetTripod( ).mLookAt;
		
		const Math::tMat3f & worldToObject = fWorldToObject( );
		const Math::tVec3f offset( fMaxX( ), 0, fMaxZ( ) );
		const Math::tVec3f localEye = worldToObject.fXformPoint( worldEye );

		const u32 minX = (u32)fClamp<s32>( fRoundDown<s32>( ( localEye.x + offset.x - gatherRadius ) / unitSize ), 0, dimx );
		const u32 maxX = (u32)fClamp<s32>( fRoundUp<s32>( ( localEye.x + offset.x + gatherRadius ) / unitSize ), 0, dimx );
		const u32 minZ = (u32)fClamp<s32>( fRoundDown<s32>( ( localEye.z + offset.z - gatherRadius ) / unitSize ), 0, dimz );
		const u32 maxZ = (u32)fClamp<s32>( fRoundUp<s32>( ( localEye.z + offset.z + gatherRadius ) / unitSize ), 0, dimz );

		fUpdateGatherRect( minX, minZ, maxX, maxZ );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fGatherRenderables( tGatherCb * cb, b32 forShadows )
	{
		if( !fShouldRender( ) )
			return;

		if( forShadows && !mDef->fAnyElementsCastShadow( ) )
			return;

		fGatherRenderables( 
			mGatherRect.mL, mGatherRect.mT, 
			mGatherRect.mR, mGatherRect.mB, 
			cb, forShadows );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fCleanRenderables( )
	{
		if( !mUnused.fGetItemCount( ) )
			return;

		// How many do we have?
		u32 total = 0;
		for( tUnusedTable::tIterator itr = mUnused.fBegin( ); itr != mUnused.fEnd( ); ++itr )
		{
			if( itr->fNullOrRemoved( ) ) continue;
			
			total += itr->mValue.fCount( );
		}

		// Free less from active clouds than from inactive
		const f32 removeRatio = mGatherRect.fHasArea( ) ? 0.01f : 0.05f;
		u32 toRemove = fRoundUp<u32>( total * removeRatio );

		// Try to distribute across the different types
		while( toRemove )
		{
			for( tUnusedTable::tIterator itr = mUnused.fBegin( ); toRemove && itr != mUnused.fEnd( ); ++itr )
			{
				if( itr->fNullOrRemoved( ) ) continue;

				if( itr->mValue.fCount( ) )
				{
					itr->mValue.fSetCount( itr->mValue.fCount( ) - 1 );
					--toRemove;
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fInitGCInstancing( )
	{
		mInstancedRenderables.fSetCount( 0 );
		mInstancedGC.fInit( this );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fInvalidateRenderables( f32 x, f32 z, f32 minRadius, f32 maxRadius )
	{
		profile_pix( "GC fInvalidateRenderables" );

		sigassert( minRadius >= 0.f );
		sigassert( maxRadius >= 0.f );
		sigassert( minRadius <= maxRadius );

		if( !fShouldRender( ) )
			return;

		if( fInstanced( ) )
		{
			profile_pix( "mInstancedGC.fInvalidate()" );
			mInstancedGC.fInvalidate( x, z, minRadius, maxRadius );
			return;
		}

		const f32 unitSize = mDef->fUnitSize( );
		const u32 dimx = mDef->fDimX( );

		//pump up radius change if less than unit size
		if( maxRadius - minRadius < unitSize )
			minRadius = maxRadius - unitSize;

		//get invalidate rect
		const Math::tRectu invalidRect = fGenInvalidateRect( x, z, maxRadius );

		//vars for 2d dist check loop
		const f32 halfUnitSize = mDef->fUnitSize( ) / 2.0f;
		const f32 radiusCheck = halfUnitSize * cRoot2; //distance from center of box to corner
		const f32 minMult = fMax( 0.f, minRadius - radiusCheck );
		const f32 minCheckSq = minMult * minMult;
		const f32 maxCheckSq = ( maxRadius + radiusCheck ) * ( maxRadius + radiusCheck );

		const Math::tMat3f & worldToObject = fWorldToObject( );
		const Math::tVec3f center = worldToObject.fXformPoint( Math::tVec3f( x, 0, z ) );

		const f32 xpStart = fXPos( invalidRect.mL ) - center.x;
		f32 zp = fZPos( invalidRect.mT ) - center.z;

		//dist check every renderable
		for( u32 zi = invalidRect.mT; zi < invalidRect.mB; ++zi )
		{
			f32 xp = xpStart;
			for( u32 xi = invalidRect.mL; xi < invalidRect.mR; ++xi )
			{
				const f32 distSq = xp * xp + zp * zp;
				if( distSq >= minCheckSq && distSq <= maxCheckSq )
				{
					const u32 idx = xi + zi * dimx;
					sigassert( idx < mCells.fCount( ) );

					//invalidate cell
					tCell& c = mCells[ idx ];
					if( c.fIsInstantiated( ) )
						c.fUpdateCulling( mCuller );

					//debug render
					if( GroundCover_RenderInvalidate )
					{
						const Math::tMat3f otw = fObjectToWorld( );
						const Math::tAabbf aabb( Math::tVec3f( fXPos( xi ), mDef->fHeight( xi, zi, 0 ), fZPos( zi ) ), Math::tVec3f( fXPos( xi + 1 ), mDef->fHeight( xi + 1, zi + 1, 0 ), fZPos( zi + 1 ) ) );
						Math::tObbf obb( aabb, otw );
						obb.mCenter.y += 0.5f;
						fSceneGraph( )->fDebugGeometry( ).fRenderOnce( obb, Math::tVec4f( 1, 0, 0, 0.25f ) );
					}
				}
				xp += unitSize;
			}
			zp += unitSize;
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fUpdateAllCellBounds( )
	{
		for( u32 z = 0; z < mDef->mDimZ; ++z )
		{
			for( u32 x = 0; x < mDef->mDimX; ++x )
			{
				fUpdateCellBounds( x, z );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fOnSpawn( )
	{
		profile_pix( "GC fOnSpawn" );
		tEntityCloud::fOnSpawn( );

		if( fInstanced( ) )
		{
			fInitGCInstancing( );
			return;
		}

		fAllocateCells( mDef->fDimX( ) * mDef->fDimZ( ) );
		
		//setup bounds for all cells
		const u32 dimx = mDef->fDimX( );
		const u32 dimz = mDef->fDimZ( );
		const f32 unitSize = mDef->fUnitSize( );
		const f32 hus = unitSize / 2.0f;
		for( u32 z = 0; z < dimz; ++z )
		{
			for( u32 x = 0; x < dimx; ++x )
			{
				fUpdateCellBounds( x, z );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fOnDelete( )
	{
		const u32 cellCount = mCells.fCount( );
		for( u32 c = 0; c < cellCount; ++c )
			mCells[ c ].fDestroy( mUnused, mCuller );

		mUnused.fClear( );

		mInstancedGC.fDeleteEverything( );

		tEntityCloud::fOnDelete( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fOnMoved( b32 recomputeParentRelative )
	{
		tEntityCloud::fOnMoved( recomputeParentRelative );
		if( !mCells.fCount( ) )
			return;

		const f32 unitSize = mDef->fUnitSize( );
		const u32 dimx = mDef->fDimX( );
		const u32 dimz = mDef->fDimZ( );

		const Math::tMat3f & objectToWorld = fObjectToWorld( );

		f32 zP = fMinZ( );
		for( u32 z = 0; z < dimz; ++z, zP += unitSize  )
		{
			f32 xP = fMinX( );
			for( u32 x = 0; x < dimx; ++x, xP += unitSize )
			{
				tCell & cell = mCells[ z * dimx + x ];

				//move renderables (if spawned)
				if( cell.fIsDeadOrEmpty( ) )
					continue;
				cell.fRenderList( ).fMoveTo( fDef( ), mCuller, objectToWorld, x, z, xP, zP );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fAllocateCells( u32 newCount )
	{
		mGatherRect.fInvalidate( );
		const u32 oldCount = mCells.fCount( );
		for( u32 c = 0; c < oldCount; ++c )
			fDestroyCell( mCells[ c ] );

		mCells.fResize( newCount );
		sigassert( !newCount || mCells.fBegin( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fGatherRenderables( 
		u32 minX, u32 minZ, u32 maxX, u32 maxZ, tGatherCb * cb, b32 forShadows )
	{
		profile( cProfilePerfGatherGroundCover );

		sigassert( minX <= mDef->mDimX );
		sigassert( minZ <= mDef->mDimZ );
		sigassert( maxX <= mDef->mDimX );
		sigassert( maxZ <= mDef->mDimZ );

		tFixedArray<tEntityBVH::tObjectPtr,256> kickBuffer;
		u32 kickCount = 0;

		const u32 dimx = mDef->fDimX( );
		for( u32 z = minZ; z < maxZ; ++z)
		{
			for( u32 x = minX; x < maxX; ++x )
			{
				const tRenderList & rl = mCells[ z * dimx + x ].fRenderList( );
				const u32 eCount = rl.fCount( );
				for( u32 e = 0; e < eCount; ++e )
				{
					tRenderableEntity * entity = rl[ e ];

					if( kickCount >= kickBuffer.fCount( ) )
					{
						(*cb)( kickBuffer.fBegin( ), kickCount );
						kickCount = 0;
					}

					kickBuffer[ kickCount++ ] = entity->fToSpatialSetObject( );
				}
			}
		}

		if( kickCount > 0 )
		{
			(*cb)( kickBuffer.fBegin( ), kickCount );
			kickCount = 0;
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fInstantiateCells( u32 minX, u32 minZ, u32 maxX, u32 maxZ, b32 force )
	{
		profile_pix( "GC fInstantiateCells" );
		sigassert( minX <= mDef->fDimX( ) );
		sigassert( maxX <= mDef->fDimX( ) );
		sigassert( minZ <= mDef->fDimZ( ) );
		sigassert( maxZ <= mDef->fDimZ( ) );

		const f32 unitSize = mDef->fUnitSize( );
		const u32 totalDimX = mDef->fDimX( );

		f32 pZ = fZPos( minZ );
		for( u32 z = minZ; z < maxZ; ++z, pZ += unitSize  )
		{
			f32 pX = fXPos( minX );
			for( u32 x = minX; x < maxX; ++x, pX += unitSize )
			{
				tCell & cell = mCells[ z * totalDimX + x ];
				if( !force && cell.fIsInstantiated( ) )
					continue;

				b32 contains;
				if( !force && !mFrustum.fIntersectsOrContains( cell.mBounds, contains ) )
					continue;

				//destroy and instantiate cell
				fDestroyCell( cell );
				cell.mList.fSpawn( mUnused, fDef( ), mCuller, fObjectToWorld( ), x, z, pX, pZ );
				cell.fInstantiate( );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fUpdateCellBounds( u32 x, u32 z )
	{
		sigassert( mDef->fUnitSize( ) > 0 );
		const u32 dimx = mDef->fDimX( );
		const f32 hus = mDef->fUnitSize( ) / 2.0f; //half unit size

		//NOTE: height calculation will be incorrect if there is a large enough variance across the cell 
		const Math::tVec3f pos( fXPos( x ), mDef->fHeight( x, z, 0 ), fZPos( z ) );
		const f32 radius = hus * cRoot2; //*cRoot2 so we fully encompass all 4 corners
		mCells[ z * dimx + x ].mBounds = Math::tSpheref( pos, radius ).fTransform( fObjectToWorld( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fDestroyCell( tCell & cell )
	{
		if( cell.fIsInstantiated( ) )
			cell.fDestroy( mUnused, mCuller );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fUpdateGatherRect( u32 minX, u32 minZ, u32 maxX, u32 maxZ )
	{
		profile_pix( "GC fUpdateGatherRect" );
		sigassert( minX <= mDef->fDimX( ) );
		sigassert( maxX <= mDef->fDimX( ) );
		sigassert( minZ <= mDef->fDimZ( ) );
		sigassert( maxZ <= mDef->fDimZ( ) );

		if( !mFrustumDirty &&
			mGatherRect.mL == minX &&
			mGatherRect.mR == maxX &&
			mGatherRect.mT == minZ &&
			mGatherRect.mB == maxZ )
			return;

		mFrustumDirty = false;

		const u32 dimx = mDef->fDimX( );

		// destroy any cells not in frustum
		for( u32 z = mGatherRect.mT; z < mGatherRect.mB; ++z )
		{
			for( u32 x = mGatherRect.mL; x < mGatherRect.mR; ++x )
			{
				tCell & cell = mCells[ z * dimx + x ];
				if( !cell.fIsInstantiated( ) )
					continue;
				b32 contains = false;
				if( !mFrustum.fIntersectsOrContains( cell.mBounds, contains ) )
					fDestroyCell( cell );
			}
		}

		// Drop cells to the left
		for( u32 z = mGatherRect.mT; z < mGatherRect.mB; ++z )
		{
			for( u32 x = mGatherRect.mL; x < minX; ++x )
			{
				fDestroyCell( mCells[ z * dimx + x ] );
			}
		}

		// Drop cells to the right
		for( u32 z = mGatherRect.mT; z < mGatherRect.mB; ++z )
		{
			for( u32 x = maxX; x < mGatherRect.mR; ++x )
			{
				fDestroyCell( mCells[ z * dimx + x ] );
			}
		}

		// Drop cells above
		for( u32 x = mGatherRect.mL; x < mGatherRect.mR; ++x )
		{
			for( u32 z = mGatherRect.mT; z < minZ; ++z )
			{
				fDestroyCell( mCells[ z * dimx + x ] );
			}
		}

		//Drop cells below
		for( u32 x = mGatherRect.mL; x < mGatherRect.mR; ++x )
		{
			for( u32 z = maxZ; z < mGatherRect.mB; ++z )
			{
				fDestroyCell( mCells[ z * dimx + x ] );
			}
		}

		mGatherRect.mL = minX;
		mGatherRect.mR = maxX;
		mGatherRect.mT = minZ;
		mGatherRect.mB = maxZ;

		fInstantiateCells( minX, minZ, maxX, maxZ, false );
	}
	
	//------------------------------------------------------------------------------
	Math::tRectu tGroundCoverCloud::fGenInvalidateRect( f32 worldX, f32 worldZ, f32 radius ) const
	{
		Math::tRectu r;

		const Math::tMat3f & worldToObject = fWorldToObject( );
		const Math::tVec3f pt = worldToObject.fXformPoint( Math::tVec3f( worldX, 0, worldZ ) );

		const f32 offsetX = fMaxX( );
		const f32 offsetZ = fMaxZ( );
		const f32 xmin = pt.x - radius + offsetX;
		const f32 xmax = pt.x + radius + offsetX;
		const f32 zmin = pt.z - radius + offsetZ;
		const f32 zmax = pt.z + radius + offsetZ;

		const u32 dimx = mDef->fDimX( );
		const u32 dimz = mDef->fDimZ( );
		const f32 unitSize = mDef->fUnitSize( );

		r.mL = (u32)fClamp<s32>( fRoundDown<s32>( xmin / unitSize - 1 ), 0, dimx );
		r.mR = (u32)fClamp<s32>( fRoundUp<s32>( xmax / unitSize ), 0, dimx );
		r.mT = (u32)fClamp<s32>( fRoundDown<s32>( zmin / unitSize - 1 ), 0, dimz );
		r.mB = (u32)fClamp<s32>( fRoundUp<s32>( zmax / unitSize ), 0, dimz );

		return r;
	}

	//------------------------------------------------------------------------------
	// tRenderList
	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fSpawn( 
		tUnusedTable & table,
		const tGroundCoverCloudDef * def,
		tLinearFrustumCulling & lfc,
		const Math::tMat3f & ownerWorld,
		u32 cX, u32 cZ, f32 pX, f32 pZ )
	{
		sigassert( mParents.fCount( ) == 0 );
		sigassert( mRenderables.fCount( ) == 0 );

		// Keep some often spawned arrays around
		static tGrowableArray<tSpawnInfo> spawns; 
		static tRenderableGatherer gatherer; 
		static tGrowableArray<tParent> newParents;

		fGenSpawnInfo( def, cX, cZ, pX, pZ, spawns );

		const u32 spawnCount = spawns.fCount( );
		for( u32 s = 0; s < spawnCount; ++s )
		{
			const tSpawnInfo & info = spawns[ s ];
			if( !info.mDoSpawn )
				continue;

			tParent& parent = newParents.fPushBack( );

			tUnusedArray * unused = table.fFind( info.mElement->fSgResourcePtr( )->fGetPath( ) );

			if( unused && unused->fCount( ) )
			{
				parent.mEntity = unused->fPopBack( );
				parent.mEntity->fMoveTo( ownerWorld * info.mLocalXform );
			}
			else
			{
				parent.mEntity.fReset( NEW tSceneRefEntity( info.mElement->fSgResourcePtr( ) ) );
				parent.mEntity->fMoveTo( ownerWorld * info.mLocalXform );
				parent.mEntity->fCollectEntities( tEntityCreationFlags( ) );
			}

			// If the entry is now empty, remove it
			if( unused && !unused->fCount( ) )
				table.fRemove( unused );

			parent.mRenderableStart = gatherer.mRenderables.fCount( );
			parent.mEntity->fForEachDescendent( gatherer );
			parent.mRenderableEnd = gatherer.mRenderables.fCount( );

			fApplyProperties( def, lfc, parent, *info.mElement, gatherer.mRenderables.fBegin( ) );
		}

		mParents = newParents;
		mRenderables = gatherer.mRenderables;

		// Zero out static arrays
		spawns.fSetCount( 0 );
		gatherer.mRenderables.fSetCount( 0 );
		newParents.fSetCount( 0 );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fMoveTo( 
		const tGroundCoverCloudDef * def,
		tLinearFrustumCulling & lfc,
		const Math::tMat3f & ownerWorld, 
		u32 cX, u32 cZ, f32 pX, f32 pZ )
	{
		tGrowableArray<tSpawnInfo> spawns;

		fGenSpawnInfo( def, cX, cZ, pX, pZ, spawns );

		u32 p = 0;
		const u32 spawnCount = spawns.fCount( );
		for( u32 s = 0; s < spawnCount; ++s )
		{
			const tSpawnInfo & info = spawns[ s ];
			if( !info.mDoSpawn )
				continue;

			tParent& parent = mParents[ p ];
			sigassert( parent.mEntity->fSgResource( ) == info.mElement->fSgResourcePtr( ) );
			parent.mEntity->fMoveTo( ownerWorld * info.mLocalXform );

			for( u32 r = parent.mRenderableStart; r < parent.mRenderableEnd; ++r )
			{
				tRenderableEntity * renderable = mRenderables[ r ];
				if( renderable->fInRenderCullingList( ) )
					renderable->fUpdateRenderCulling( lfc );
			}

			++p;
		}

		sigassert( p == mParents.fCount( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fApplyProperties( 
		const tGroundCoverCloudDef * def,
		tLinearFrustumCulling & lfc,
		const tGroundCoverCloudDef::tElement & element )
	{
		const u32 parentCount = mParents.fCount( );
		for( u32 p = 0; p < parentCount; ++p )
		{
			tParent & parent = mParents[ p ];
			if( parent.mEntity->fSgResource( ) != element.fSgResourcePtr( ) )
				continue;

			fApplyProperties( def, lfc, parent, element, mRenderables.fBegin( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fUpdateCulling( tLinearFrustumCulling& lfc )
	{
		const b32 validFilter = !gGroundCoverFilter.fNull( );
		for( u32 i = 0; i < mRenderables.fCount( ); ++i )
		{
			Gfx::tRenderableEntity* re = mRenderables[ i ];
			const b32 inCullList = re->fInRenderCullingList( );

			const b32 failed = validFilter && !gGroundCoverFilter( re );

			if( inCullList && failed )
				re->fRemoveFromRenderCulling( lfc );
			else if( !inCullList && !failed )
				re->fAddToRenderCulling( lfc );
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fDestroy(
		tUnusedTable & table,
		tLinearFrustumCulling & lfc )
	{ 
		// Add our parents to the unused table
		const u32 parentCount = mParents.fCount( );
		for( u32 p = 0; p < parentCount; ++p )
		{
			const tParent & parent = mParents[ p ];
			const tFilePathPtr & path = parent.mEntity->fSgResource( )->fGetPath( );

			tUnusedArray * unused = table.fFind( path );
			if( !unused )
				unused = table.fInsert( path, tUnusedArray( ) );

			unused->fPushBack( parent.mEntity );

			for( u32 r = parent.mRenderableStart; r < parent.mRenderableEnd; ++r )
			{
				tRenderableEntity * renderable = mRenderables[ r ];
				if( renderable->fInRenderCullingList( ) )
					renderable->fRemoveFromRenderCulling( lfc );
			}
		}

		mParents.fDeleteArray( );
		mRenderables.fDeleteArray( );
	}

	//------------------------------------------------------------------------------
	// tGroundCoverInstance
	//------------------------------------------------------------------------------
	u32 tInstanceGrid::fDimX( const tGroundCoverCloudDef * def )
	{
		return (u32)ceil( def->fDimX( ) / (f32)def->fInstancedCellDiff( ) );
	}
	u32 tInstanceGrid::fDimZ( const tGroundCoverCloudDef * def )
	{
		return (u32)ceil( def->fDimZ( ) / (f32)def->fInstancedCellDiff( ) );
	}
	u32 tInstanceGrid::fConvert( const tGroundCoverCloudDef * def, u32 x, u32 z )
	{
		const u32 w = fDimX( def );
		x /= def->fInstancedCellDiff( );
		z /= def->fInstancedCellDiff( );
		return x + z * w;
	}
	void tInstanceGrid::fConvert( const tGroundCoverCloudDef * def, Math::tRectu& r )
	{
		r.mL = fRoundDown<u32>( (f32)r.mL / def->fInstancedCellDiff( ) );
		r.mR = fRoundUp<u32>( (f32)r.mR / def->fInstancedCellDiff( ) );
		r.mT = fRoundDown<u32>( (f32)r.mT / def->fInstancedCellDiff( ) );
		r.mB = fRoundUp<u32>( (f32)r.mB / def->fInstancedCellDiff( ) );
	}
	void tInstanceGrid::fGetIntersectingCells( u32 minX, u32 maxX, u32 minZ, u32 maxZ, const tCamera& camera, tGrowableArray<u32>& cells, const tDynamicArray< tDynamicArray< Math::tMat3f > >& cellMats ) const
	{
		const Math::tFrustumf& frust = camera.fGetWorldSpaceFrustum( );
		const tGroundCoverCloudDef* def = mCloud->fDef( );
		const Math::tVec3f eye = mCloud->fWorldToObject( ).fXformPoint( camera.fGetTripod( ).mEye );
		const u32 dimX = tInstanceGrid::fDimX( mCloud->fDef( ) );
		const f32 minDistSq = Math::fSquare( gNearGatherRadii[ def->mNearVisibility ] );
		const f32 maxDistSq = Math::fSquare( gFarGatherRadii[ def->mFarVisibility ] );

		const f32 unitSize = def->fUnitSize( ) * def->fInstancedCellDiff( );
		const f32 halfUnitSize = unitSize / 2.0f;
		const f32 firstXPos = def->fXPos( minX * def->fInstancedCellDiff( ) );
		f32 zPos = def->fZPos( minZ * def->fInstancedCellDiff( ) );

		for( u32 z = minZ; z < maxZ; ++z )
		{
			f32 xPos = firstXPos;
			for( u32 x = minX; x < maxX; ++x )
			{
				const u32 i = x + z * dimX;
				if( cellMats[ i ].fCount( ) )
				{
					const f32 clampedX = fClamp( eye.x, xPos, xPos + unitSize );
					const f32 clampedZ = fClamp( eye.z, zPos, zPos + unitSize );
					const f32 xDiff = clampedX - eye.x;
					const f32 zDiff = clampedZ - eye.z;
					const f32 distSq = xDiff * xDiff + zDiff * zDiff;
					if( fInBounds( distSq, minDistSq, maxDistSq ) )
					{
						const f32 hdiff = mHeights[ i ].mB - mHeights[ i ].mA; //pre-calculated in world-space
						const f32 hmid = mHeights[ i ].mA + hdiff / 2.0f;
						Math::tSpheref s( Math::tVec3f( xPos + halfUnitSize, 0, zPos + halfUnitSize ), fMax( hdiff, unitSize ) );
						s = s.fTransform( mCloud->fObjectToWorld( ) );
						s.mCenter.y	= hmid;

						if( frust.fIntersects( s ) )
							cells.fPushBack( i );
					}
				}
				xPos += unitSize;
			}
			zPos += unitSize;
		}
	}
	tInstanceGrid::tInstanceGrid( )
		: mCloud( NULL )
	{
	}
	void tInstanceGrid::fInit( const tGroundCoverCloud* cloud, const tDynamicArray< tInstanceMaster >& masters )
	{
		mCloud = cloud;
		const u32 totalCells = tInstanceGrid::fDimX( mCloud->fDef( ) ) * tInstanceGrid::fDimZ( mCloud->fDef( ) );

		mHeights.fNewArray( totalCells );
		mCells.fNewArray( totalCells );
		for( u32 i = 0; i < totalCells; ++i )
			mCells[ i ].fInit( cloud->fDef( ), masters );

#ifdef gc_debug_cells
		mSetup = false;
#endif//gc_debug_cells
	}
	void tInstanceGrid::fAppendRenderables( tGrowableArray< tRenderableEntity* >& out, const tGrowableArray<u32>& cells )
	{
		for( u32 i = 0; i < cells.fCount( ); ++i )
		{
			tInstancedCell& cell = mCells[ cells[ i ] ];
			for( u32 iR = 0; iR < cell.mRenderables.fCount( ); ++iR )
			{
				if( cell.mRenderables[ iR ].mEntity && cell.mRenderables[ iR ].mEntity->fRenderBatch( )->fBatchData( ).mGeometryBuffer2->fNumInstances( ) )
					out.fPushBack( cell.mRenderables[ iR ].mEntity.fGetRawPtr( ) );
			}
		}
	}
	void tInstanceGrid::fInvalidate( const tGrowableArray<u32>& cells, const tDynamicArray< tMat3fArray >& mats )
	{
		const b32 asyncReallocCells = cells.fCount( ) < GroundCover_AsyncReallocThreshhold;
		for( u32 i = 0; i < cells.fCount( ); ++i )
			mCells[ cells[ i ] ].fInvalidate( mCloud->fObjectToWorld( ), mats[ cells[ i ] ], asyncReallocCells );
	}
	void tInstanceGrid::fInvalidateAll( const tDynamicArray< tMat3fArray >& mats )
	{
		for( u32 i = 0; i < mCells.fCount( ); ++i )
			mCells[ i ].fInvalidate( mCloud->fObjectToWorld( ), mats[ i ], false );

		for( u32 i = 0; i < mHeights.fCount( ); ++i )
		{
			f32 minH = Math::cInfinity;
			f32 maxH = -Math::cInfinity;
			for( u32 j = 0; j < mats[ i ].fCount( ); ++j )
			{
				f32 h = mats[ i ][ j ].fGetTranslation( ).y;
				if( h < minH )
					minH = h;
				if( h > maxH )
					maxH = h;
			}
			if( mats[ i ].fCount( ) == 0 )
			{
				minH = 0.f;
				maxH = 0.f;
			}
			mHeights[ i ].mA = minH;
			mHeights[ i ].mB = maxH;
		}
	}
	void tInstanceGrid::fDeleteEverything( )
	{
		mCloud = NULL;
		mCells.fDeleteArray( );
	}
#ifdef gc_debug_cells
	namespace
	{
		Math::tVec4f gInstanceDebugColorHit( 1, 0, 0, 0.25f );
		Math::tVec4f gInstanceDebugColorNoHit( 0, 0, 0, 0.25f );
		Math::tVec4f gInstanceDebugColorFrust( 1, 1, 1, 0.1f );
		f32 gInstanceDebugXOffset;
		f32 gInstanceDebugYOffset;
	}
	void tInstanceGrid::fSetupDebugDraw( )
	{
		const u32 dimX = tInstanceGrid::fDimX( mCloud->fDef( ) );
		const u32 dimZ = tInstanceGrid::fDimZ( mCloud->fDef( ) );
		const f32 w = mCloud->fDef( )->fInstancedCellDiff( ) * 1.0f;
		gInstanceDebugXOffset = 1270.0f - dimX * ( w + 1 );
		gInstanceDebugYOffset = 500.0f - dimZ * ( w + 1 );
		mDebugQuads.fNewArray( dimX * dimZ );
		for( u32 z = 0; z < dimZ; ++z )
		{
			for( u32 x = 0; x < dimX; ++x )
			{
				const u32 idx = x + z * dimX;
				sigassert( mDebugQuads[ idx ].fGetRawPtr( ) == NULL );
				mDebugQuads[ idx ].fReset( NEW_TYPED( Gui::tColoredQuad )( ) );

				const Math::tVec4f init( 1, 1, 0, 0.25 );
				mDebugQuads[ idx ]->fSetVertColors( init, init, init, init );
				const f32 left = gInstanceDebugXOffset + x * ( w + 1 );
				const f32 top = gInstanceDebugYOffset + z * ( w + 1 );
				mDebugQuads[ idx ]->fSetRect( Math::tRect( top, left, top + w, left + w ) );
				tGameAppBase::fInstance( ).fRootCanvas( ).fAddChild( Gui::tCanvasPtr( mDebugQuads[ idx ].fGetRawPtr( ) ) );
			}
		}

		const Math::tVec4f frustClr = gInstanceDebugColorFrust;
		mDebugFrust.fReset( NEW_TYPED( Gui::tColoredQuad )( ) );
		mDebugFrust->fSetVertColors( frustClr, frustClr, frustClr, frustClr );
		tGameAppBase::fInstance( ).fRootCanvas( ).fAddChild( Gui::tCanvasPtr( mDebugFrust.fGetRawPtr( ) ) );
	}
	void tInstanceGrid::fUpdateDebugDraw( const tGrowableArray<u32>& cells, const Gfx::tCamera& camera )
	{
		if( !mSetup && GroundCover_DrawInstanced )
		{
			mSetup = true;
			fSetupDebugDraw( );
		}

		if( !mDebugFrust )
			return;

		const b32 updateVisibility = ( GroundCover_DrawInstanced && mDebugFrust->fInvisible( ) ) || ( !GroundCover_DrawInstanced && !mDebugFrust->fInvisible( ) );
		if( updateVisibility )
		{
			mDebugFrust->fSetInvisible( !GroundCover_DrawInstanced );
			for( u32 i = 0; i < mDebugQuads.fCount( ); ++i )
				mDebugQuads[ i ]->fSetInvisible( !GroundCover_DrawInstanced );
		}

		if( !GroundCover_DrawInstanced )
			return;

		const Math::tVec4f nohit = gInstanceDebugColorNoHit;
		const Math::tVec4f hit = gInstanceDebugColorHit;
		for( u32 i = 0; i < mDebugQuads.fCount( ); ++i )
		{
			if( cells.fFind( i ) )
				mDebugQuads[ i ]->fSetVertColors( hit, hit, hit, hit );
			else
				mDebugQuads[ i ]->fSetVertColors( nohit, nohit, nohit, nohit );
		}

		const Math::tVec3f& p = camera.fGetTripod( ).mLookAt;
		Math::tRectu ru = mCloud->fGenInvalidateRect( p.x, p.z, gFarGatherRadii[ mCloud->fDef( )->mFarVisibility ] );
		fConvert( mCloud->fDef( ), ru );
		Math::tRectf r( (f32)ru.mT, (f32)ru.mL, (f32)ru.mB, (f32)ru.mR );
		r *= (f32)( mCloud->fDef( )->fInstancedCellDiff( ) + 1 );
		r += Math::tVec2f( gInstanceDebugXOffset, gInstanceDebugYOffset );
		mDebugFrust->fSetRect( r );
	}
#endif//gc_debug_cells
	//------------------------------------------------------------------------------
	namespace
	{
		const tVertexElement cGCInstanceElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_4, 1, tVertexFormat::cStreamIndex_Instanced ),
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_4, 2, tVertexFormat::cStreamIndex_Instanced ),
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_4, 3, tVertexFormat::cStreamIndex_Instanced ),
		};
		const tVertexFormat cGCInstanceFormat( cGCInstanceElements, array_length( cGCInstanceElements ) );
	}
	template<class tSourceType> void fDuplicateIndexBufferAndConvertToU32Indicies( u32* dst, const tSourceType* src, u32 numIndicies, u32 instances )
	{
		for( u32 i = 0; i < instances; ++i )
		{
			for( u32 j = 0; j < numIndicies; ++j )
			{
				dst[ j + i * numIndicies ] = src[ j ] + i * numIndicies;
			}
		}
	}
	//------------------------------------------------------------------------------
	tInstanceMaster::tInstanceMaster( )
		: mFormat( cGCInstanceFormat )
	{
	}
	//------------------------------------------------------------------------------
	void tInstanceMaster::fInit( const tGroundCoverCloud* cloud, const tRenderableEntity* src, u32 maxInstances )
	{
		mEntity.fReset( src );
		mFormat.fCombine( *src->fRenderBatch( )->fBatchData( ).mVertexFormat );

#ifdef platform_xbox360
		//allocate index buffer
		tIndexBufferVRam& sourceIb = const_cast<tIndexBufferVRam&>( *mEntity->fRenderBatch( )->fBatchData( ).mIndexBuffer ); //if we can figure out a better way of handling fDeepLock() then we can get rid of this const_cast
		const tDevicePtr& device = tGameAppBase::fInstance( ).fDevice( );
		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextGroundCover ) );
		{
			mFormat.fAllocateInPlace( device );
			mIndicies.fAllocate( device,
				tIndexFormat( tIndexFormat::cStorageU32, sourceIb.fIndexFormat( ).mPrimitiveType ),
				sourceIb.fIndexCount( ) * maxInstances,
				sourceIb.fPrimitiveCount( ) * maxInstances,
				0,
				sourceIb.fIndexCount( ) );
		}
		Memory::tHeap::fResetVramContext( );

		//prep index buffer
		const u32 indexCount = sourceIb.fIndexCount( );
		const u32 ibSize = sourceIb.fBufferSize( );
		const byte* from = sourceIb.fDeepLock( ); //this may or may not be u32
		u32* to = (u32*)mIndicies.fDeepLock( );
		{
			static_assert( tIndexFormat::cStorageTypeCount == 2 );
			if( sourceIb.fIndexFormat( ).mStorageType == tIndexFormat::cStorageU32 )
				fDuplicateIndexBufferAndConvertToU32Indicies( to, (const u32*)from, indexCount, maxInstances );
			else
				fDuplicateIndexBufferAndConvertToU32Indicies( to, (const u16*)from, indexCount, maxInstances );
		}
		mIndicies.fDeepUnlock( );
		sourceIb.fDeepUnlock( );
#endif//platform_xbox360
	}
	//------------------------------------------------------------------------------
	tInstanceRenderable::tInstanceRenderable( )
		: mGpuMats( tVertexFormat::cStreamIndex_Instanced )
		, mState( ~0 )
	{
	}
	//------------------------------------------------------------------------------
	void tInstanceRenderable::fCreateRenderable( const tGroundCoverCloudDef* def, const tInstanceMaster& master )
	{
		//create batch based off renderable
		tRenderBatchData data = master.mEntity->fRenderBatch( )->fBatchData( );
		data.mBehaviorFlags |= tRenderBatchData::cBehaviorUseInstancing;
		data.mVertexFormat = &master.mFormat;
#ifdef platform_xbox360
		data.mIndexBuffer = &master.mIndicies;
#endif//platform_xbox360
		data.mGeometryBuffer2 = &mGpuMats;

		//create root entity that will draw all instances
		mEntity.fReset( NEW_TYPED( tRenderableEntity )( tRenderBatch::fCreate( data ) ) );
		mEntity->fSetCastsShadow( master.mEntity->fCastsShadow( ) );
		mEntity->fSetFadeSettingsOnThis( 
			NULL, //<-- doesn't matter, we should be ignoring this anyways. We do a setfadesetting so we can pass the fade values to our shader and calculate fade there compared to camera pos
			Gfx::tRenderableEntity::cFadeNever,
			Gfx::tRenderableEntity::cFadeNever, 
			gFarGatherRadii[ def->fFarVisibility( ) ],
			0.f ); //<-- also completely ignored by instancing system

		//get state
		for( u32 i = 0; i < 4; ++i )
		{
			if( master.mEntity->fStateEnabled( i ) )
			{
				mState = i;
				break;
			}
		}
		sigassert( mState != ~0 );
	}
	void tInstanceRenderable::fCopyMatsToGpu( const Math::tMat3f* mats, const u32 count )
	{
		//allocate vertex buffer if not enough space
		if( mGpuMats.fVertexCount( ) < count )
		{
			const tDevicePtr& device = tGameAppBase::fInstance( ).fDevice( );
			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextGroundCover ) );
			{
				mGpuMats.fAllocate( device, cGCInstanceFormat, count, 0 );
			}
			Memory::tHeap::fResetVramContext( );
		}

		//copy all mats to gpu
		const u32 totalBytes = count * sizeof(mats[0]);
		sigassert( mGpuMats.fBufferSize( ) >= totalBytes );
		byte* const gpuMats = mGpuMats.fDeepLock( );
		{
			fMemCpyToGpu( gpuMats, mats, totalBytes );
		}
		mGpuMats.fDeepUnlock( );
		mGpuMats.fSetNumInstances( count );
	}
	tBakedElement::tBakedElement( )
		: mCloud( NULL )
		, mMats( NULL )
	{
	}
	void tBakedElement::fInit( tGroundCoverCloud* cloud, tSceneRefEntity* sre, const tDynamicArray< tMat3fArray >* mats, const u32 maxInstances )
	{
		mCloud = cloud;
		mMats = mats;
		sigassert( mCloud );
		sigassert( mMats );
		const tGroundCoverCloudDef* def = mCloud->fDef( );

		//spawn sigml
		tRenderableEntity::fSetInvisible( *sre, true ); //hide source sigml
		mSigml.fReset( sre );		

		const f32 farFadeOverride = gFarGatherRadii[ def->fFarVisibility( ) ];
		const f32 nearFadeOverride = gNearGatherRadii[ def->fNearVisibility( ) ];

		//gather/create renderable masters
		tRenderableGatherer gatherer;
		mSigml->fForEachDescendent( gatherer );
		const u32 numRenderables = gatherer.mRenderables.fCount( );
		if( numRenderables )
		{
			mRenderableMasters.fNewArray( numRenderables );
			for( u32 i = 0; i < numRenderables; ++i )
			{
				if( tRenderableEntity::fProjectLODEnable( ) )
				{
					//force entity to have a render batch
					gatherer.mRenderables[ i ]->fSetForceHighLOD( true ); //must force high-lod because all meshes must have the same vertex/index buffer
					gatherer.mRenderables[ i ]->fUpdateLOD( Math::tVec3f::cZeroVector );
				}

				//init
				mRenderableMasters[ i ].fInit( cloud, gatherer.mRenderables[ i ], maxInstances );
			}
		}

		//init grid
		mGrid.fInit( mCloud, mRenderableMasters );
	}
	void tBakedElement::fAppendRenderables( tGrowableArray< tRenderableEntity* >& out, const tCamera& camera )
	{
		const Math::tVec3f& eye = camera.fGetTripod( ).mEye;
		Math::tRectu r = mCloud->fGenInvalidateRect( eye.x, eye.z, gFarGatherRadii[ mCloud->fDef( )->mFarVisibility ] );
		tInstanceGrid::fConvert( mCloud->fDef( ), r );

		tGrowableArray<u32> cells;
		mGrid.fGetIntersectingCells( r.mL, r.mR, r.mT, r.mB, camera, cells, *mMats );

		//add all cells specified
		mGrid.fAppendRenderables( out, cells );

#ifdef gc_debug_cells
		mGrid.fUpdateDebugDraw( cells, camera );
#endif//gc_debug_cells
	}
	//------------------------------------------------------------------------------
	// tInstancedGroundCoverCloud
	//------------------------------------------------------------------------------
	u32 (*tInstancedGroundCoverCloud::gGetAllowedRenderableState)( f32 x, f32 z ) = 0;
	//------------------------------------------------------------------------------
	tInstancedGroundCoverCloud::tInstancedGroundCoverCloud( )
		: mCloud( NULL )
	{
	}
	tInstancedGroundCoverCloud::~tInstancedGroundCoverCloud( )
	{
		fDeleteEverything( );
	}
	//------------------------------------------------------------------------------
	void tInstancedGroundCoverCloud::fInit( tGroundCoverCloud* cloud )
	{
		Time::tStopWatch sw;

		fDeleteEverything( );
		mCloud = cloud;

		const tGroundCoverCloudDef* def = mCloud->fDef( );
		const u32 numElements = def->fElementCount( );

		//calculate best number of cells to plop together
		u32 maxInstances = 300;
		{
			u32 largestCellCount = 0;
			for( u32 iElement = 0; iElement < numElements; ++iElement )
			{
				const u32 cellCount = def->fElements( )[ iElement ].mSpawnCount;
				if( cellCount > largestCellCount )
					largestCellCount = cellCount;
			}
			maxInstances = largestCellCount * Math::fSquare( def->fInstancedCellDiff( ) );
		}


		//create sigmls for each element
		tDynamicArray<tSceneRefEntity*> elementSigmls;
		elementSigmls.fNewArray( numElements );
		for( u32 iElement = 0; iElement < numElements; ++iElement )
		{
			//spawn entity
			const tGroundCoverCloudDef::tElement& element = def->fElements( )[ iElement ];
			const tFilePathPtr& sigmlPath = element.mSgFile->fGetFilePathPtr( );
			elementSigmls[ iElement ] = mCloud->fSpawnChildImmediate( sigmlPath )->fDynamicCast<tSceneRefEntity>( );			
		}
		
		//setup all elements
		mBakedElements.fNewArray( numElements );
		for( u32 iElement = 0; iElement < numElements; ++iElement )
		{
			tBakedElement& baked = mBakedElements[ iElement ];
			baked.fInit( mCloud, elementSigmls[ iElement ], &mCloud->fDef( )->mCells[ iElement ], maxInstances );
		}

		//invalidate all cells to force copying src mats to gpu
		for( u32 i = 0; i < mBakedElements.fCount( ); ++i )
			mBakedElements[ i ].mGrid.fInvalidateAll( *mBakedElements[ i ].mMats );

		log_line( 0, "Ground Cover Init Took: " << sw.fGetElapsedMs( ) << "ms." );
	}
	//------------------------------------------------------------------------------
	void tInstancedGroundCoverCloud::fUpdate( const tCamera& camera )
	{
		if( !mCloud )
			return; //need to init!

		//nuke all renderable entities, we'll re-add each frame
		mCloud->mInstancedRenderables.fSetCount( 0 );
		
		//add all renderables to be drawn
		const u32 totalBaked = mBakedElements.fCount( );
		for( u32 iBaked = 0; iBaked < totalBaked; ++iBaked )
		{
			tBakedElement& baked = mBakedElements[ iBaked ];
			baked.fAppendRenderables( mCloud->mInstancedRenderables, camera );
		}
	}
	//------------------------------------------------------------------------------
	void tInstancedGroundCoverCloud::fInvalidate( f32 x, f32 z, f32 minRadius, f32 maxRadius )
	{
		if( !mCloud )
			return;

		Math::tRectu r = mCloud->fGenInvalidateRect( x, z, maxRadius );
		tInstanceGrid::fConvert( mCloud->fDef( ), r );
		const u32 dimX = tInstanceGrid::fDimX( mCloud->fDef( ) );

		tGrowableArray<u32> cells;
		cells.fReserve( r.fWidth( ) * r.fHeight( ) );
		for( u32 zi = r.mT; zi < r.mB; ++zi )
		{
			for( u32 xi = r.mL; xi < r.mR; ++xi )
			{
				cells.fPushBack( xi + zi * dimX );
			}
		}

		for( u32 i = 0; i < mBakedElements.fCount( ); ++i )
			mBakedElements[ i ].mGrid.fInvalidate( cells, mCloud->fDef( )->mCells[ i ] );
	}
	void tInstancedCell::fInit( const tGroundCoverCloudDef* def, const tDynamicArray< tInstanceMaster >& masters )
	{
		const u32 numMasters = masters.fCount( );
		mRenderables.fNewArray( numMasters );
		for( u32 i = 0; i < numMasters; ++i )
			mRenderables[ i ].fCreateRenderable( def, masters[ i ] );
	}
	void tInstancedCell::fInvalidate( const Math::tMat3f& otw, const tMat3fArray& mats, b32 asyncRealloc )
	{
		if( tInstancedGroundCoverCloud::gGetAllowedRenderableState )
		{
			//separate matrices by filter
			tDynamicArray< tGrowableArray< Math::tMat3f > > outMats;
			outMats.fNewArray( 4 );
			fInvalidateMatrices( otw, mats, outMats, tInstancedGroundCoverCloud::gGetAllowedRenderableState );

			//update each renderable
			for( u32 i = 0; i < mRenderables.fCount( ); ++i )
			{
				const u32 state = mRenderables[ i ].mState;
				if( asyncRealloc ) // Lets us fix 5-20ms CPU stall on lock/release of geometry buffer in use by GPU
					mRenderables[ i ].mGpuMats.fDeallocate( Gfx::tGeometryBufferVRam::cDeallocAsync );
				mRenderables[ i ].fCopyMatsToGpu( outMats[ state ].fBegin( ), outMats[ state ].fCount( ) );
			}
		}
		else
		{
			//separate matrices by filter
			tDynamicArray< tGrowableArray< Math::tMat3f > > outMats;
			outMats.fNewArray( 1 );
			fInvalidateMatrices( otw, mats, outMats, NULL );

			//update renderable
			if( asyncRealloc ) // Lets us fix 5-20ms CPU stall on lock/release of geometry buffer in use by GPU
				mRenderables[ 0 ].mGpuMats.fDeallocate( Gfx::tGeometryBufferVRam::cDeallocAsync );
			mRenderables[ 0 ].fCopyMatsToGpu( outMats[ 0 ].fBegin( ), outMats[ 0 ].fCount( ) );
		}
	}
	//------------------------------------------------------------------------------
	void tInstancedGroundCoverCloud::fDeleteEverything( )
	{
		mCloud = NULL;
		mBakedElements.fDeleteArray( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fApplyProperties( 
		const tGroundCoverCloudDef * def,
		tLinearFrustumCulling & lfc,
		tParent & parent, 
		const tGroundCoverCloudDef::tElement & element,
		tRenderableEntity * renderables[] )
	{
		// Apply element properties to each renderable entity
		const b32 noFilter = gGroundCoverFilter.fNull( );
		for( u32 r = parent.mRenderableStart; r < parent.mRenderableEnd; ++r )
		{
			tRenderableEntity * renderable = renderables[ r ];
			renderable->fSetCastsShadow( element.fCastsShadow( ) );
			renderable->fSetFadeSettingsOnThis( 
				parent.mEntity.fGetRawPtr( ), 
				Gfx::tRenderableEntity::cFadeNever,
				Gfx::tRenderableEntity::cFadeNever, 
				def ? gFarGatherRadii[ def->fFarVisibility( ) ] : 0.f,
				def ? gNearGatherRadii[ def->fNearVisibility( ) ] : 0.f );

			if( renderable->fInRenderCullingList( ) )
					continue;
			if( noFilter || gGroundCoverFilter( renderable ) )
				renderable->fAddToRenderCulling( lfc );
		}
	}

	//------------------------------------------------------------------------------
	// tCell
	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tCell::fInstantiate( )
	{
		sigassert( !mInstantiated );
		mInstantiated = true; 
	}
	
}}//Sig::Gfx
