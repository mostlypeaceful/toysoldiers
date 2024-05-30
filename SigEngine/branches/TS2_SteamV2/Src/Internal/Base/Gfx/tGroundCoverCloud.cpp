//------------------------------------------------------------------------------
// \file tGroundCoverCloud.cpp - 13 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tGroundCoverCloud.hpp"
#include "tSceneRefEntity.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "tMersenneGenerator.hpp"
#include "tProfiler.hpp"


namespace Sig { namespace Gfx
{
	namespace
	{
		f32 gGatherRadii[ tGroundCoverCloudDef::cVisibilityCount ] = { 125.f, 250.f, 500.f, Math::cInfinity };
		b32 gGloballyDisabled = false;
		tGroundCoverCloudDef::tVisibility gGlobalVisibility = tGroundCoverCloudDef::cVisibilityCount;
	}

	//------------------------------------------------------------------------------
	// tGroundCoverCloudDef
	//------------------------------------------------------------------------------
	tGroundCoverCloudDef::tVisibility tGroundCoverCloudDef::fVisibility( ) const
	{
		if( gGlobalVisibility == cVisibilityCount )
			return mVisibility;
		return gGlobalVisibility;
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

	struct tRenderableGatherer
	{
		b32 operator( )( const tEntity & e ) const {
			Gfx::tRenderableEntity * renderable = e.fDynamicCast<Gfx::tRenderableEntity>( );
			if( renderable )
				mRenderables.fPushBack( renderable );

			return false;
		}

		mutable tDynamicArray<tRenderableEntity*> mRenderables;
	};


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
		return gGloballyDisabled;
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fSetGatherRadius( tGroundCoverCloudDef::tVisibility visibility, f32 radius )
	{
		sigassert( fInBounds<u32>( visibility, 0, tGroundCoverCloudDef::cVisibilityCount - 1 ) );
		gGatherRadii[ visibility ] = radius;
	}

	//------------------------------------------------------------------------------
	f32 tGroundCoverCloud::fGetGatherRadius( tGroundCoverCloudDef::tVisibility visibility )
	{
		sigassert( fInBounds<u32>( visibility, 0, tGroundCoverCloudDef::cVisibilityCount - 1 ) );
		return gGatherRadii[ visibility ];
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fSetForcedVisibility( tGroundCoverCloudDef::tVisibility visibility  )
	{
		sigassert( fInBounds<u32>( visibility, 0, tGroundCoverCloudDef::cVisibilityCount ) );
		gGlobalVisibility = visibility;
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::fGetForcedVisibility( )
	{
		return gGlobalVisibility;
	}

	//------------------------------------------------------------------------------
	tGroundCoverCloud::tGroundCoverCloud( const tGroundCoverCloudDef * def )
		: mDef( def )
	{
		mGatherRect.fInvalidate( );
	}

	//------------------------------------------------------------------------------
	tGroundCoverCloud::~tGroundCoverCloud( )
	{
		mUnused.fClear( );

		const u32 cellCount = mCells.fCount( );
		for( u32 c = 0; c < cellCount; ++c )
			mCells[ c ].fDestroy( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fPrepareRenderables( const Gfx::tCamera & camera )
	{
		profile( cProfilePerfPrepareRenderables );

		if( gGloballyDisabled )
			return;

		// See forever
		if( mDef->fVisibility( ) == tGroundCoverCloudDef::cVisibilityForever )
		{
			fUpdateGatherRect( 0, 0, mDef->fDimX( ), mDef->fDimZ( ) );
			return;
		}

		const u32 dimx = mDef->fDimX( );
		const u32 dimz = mDef->fDimZ( );

		const f32 gatherRadius = gGatherRadii[ mDef->fVisibility( ) ];
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
		if( gGloballyDisabled )
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

#if target_tools

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fSpawnCell(
		const tGroundCoverCloudDef * def,
		u32 cX, u32 cZ, f32 pX, f32 pZ,
		tGrowableArray<tSpawnInfo> & spawns,
		b32 skipHeights)
	{
		tRenderList::fSpawn( def, cX, cZ, pX, pZ, spawns, skipHeights );
	}

#endif

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fOnSpawn( )
	{
		tEntityCloud::fOnSpawn( );
		fAllocateCells( mDef->fDimX( ) * mDef->fDimZ( ) );
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
				if( cell.fIsDeadOrEmpty( ) )
					continue;

				cell.fRenderList( ).fMoveTo( mDef, objectToWorld, x, z, xP, zP );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fAllocateCells( u32 newCount )
	{
		mGatherRect.fInvalidate( );
		const u32 oldCount = mCells.fCount( );
		for( u32 c = 0; c < oldCount; ++c )
			mCells[ c ].fDestroy( );

		mCells.fNewArray( newCount );
		sigassert( !newCount || mCells.fBegin( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fAllocateCells( u32 newCount, tUnusedTable & unused )
	{
		mGatherRect.fInvalidate( );
		const u32 oldCount = mCells.fCount( );
		for( u32 c = 0; c < oldCount; ++c )
			mCells[ c ].fDestroy( unused );

		mCells.fNewArray( newCount );
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

				fInstantiateCell( cell, x, z, pX, pZ );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fInstantiateCell( tCell & cell, u32 cX, u32 cZ, f32 pX, f32 pZ )
	{
		if( cell.fIsInstantiated( ) )
			fDestroyCell( cell );

		tRenderList list;
		list.fSpawn( mUnused, mDef, fObjectToWorld( ), cX, cZ, pX, pZ );

		cell.fInstantiate( list );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fDestroyCell( tCell & cell )
	{
		sigassert( cell.fIsInstantiated( ) );

		cell.fDestroy( mUnused );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::fUpdateGatherRect( u32 minX, u32 minZ, u32 maxX, u32 maxZ )
	{
		sigassert( minX <= mDef->fDimX( ) );
		sigassert( maxX <= mDef->fDimX( ) );
		sigassert( minZ <= mDef->fDimZ( ) );
		sigassert( maxZ <= mDef->fDimZ( ) );

		if( mGatherRect.mL == minX &&
			mGatherRect.mR == maxX &&
			mGatherRect.mT == minZ &&
			mGatherRect.mB == maxZ )
			return;

		const u32 dimx = mDef->fDimX( );

		// Drop cells to the left
		for( u32 z = mGatherRect.mT; z < mGatherRect.mB; ++z )
		{
			for( u32 x = mGatherRect.mL; x < minX; ++x )
			{
				tCell & cell = mCells[ z * dimx + x ];
				if( cell.fIsInstantiated( ) )
					fDestroyCell( cell );
			}
		}

		// Drop cells to the right
		for( u32 z = mGatherRect.mT; z < mGatherRect.mB; ++z )
		{
			for( u32 x = maxX; x < mGatherRect.mR; ++x )
			{
				tCell & cell = mCells[ z * dimx + x ];
				if( cell.fIsInstantiated( ) )
					fDestroyCell( cell );
			}
		}

		// Drop cells above
		for( u32 x = mGatherRect.mL; x < mGatherRect.mR; ++x )
		{
			for( u32 z = mGatherRect.mT; z < minZ; ++z )
			{
				tCell & cell = mCells[ z * dimx + x ];
				if( cell.fIsInstantiated( ) )
					fDestroyCell( cell );
			}
		}

		//Drop cells below
		for( u32 x = mGatherRect.mL; x < mGatherRect.mR; ++x )
		{
			for( u32 z = maxZ; z < mGatherRect.mB; ++z )
			{
				tCell & cell = mCells[ z * dimx + x ];
				if( cell.fIsInstantiated( ) )
					fDestroyCell( cell );
			}
		}

		mGatherRect.mL = minX;
		mGatherRect.mR = maxX;
		mGatherRect.mT = minZ;
		mGatherRect.mB = maxZ;

		fInstantiateCells( minX, minZ, maxX, maxZ, false );
	}

	//------------------------------------------------------------------------------
	// tRenderList
	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fSpawn( 
		tUnusedTable & table,
		const tGroundCoverCloudDef * def, 
		const Math::tMat3f & ownerWorld, 
		u32 cX, u32 cZ, f32 pX, f32 pZ )
	{
		tGrowableArray<tSpawnInfo> spawns;
		fSpawn( def, cX, cZ, pX, pZ, spawns );

		tRenderableGatherer gatherer;
		tGrowableArray< tParent > newParents;

		const u32 spawnCount = spawns.fCount( );
		for( u32 s = 0; s < spawnCount; ++s )
		{
			const tSpawnInfo & info = spawns[ s ];
			if( !info.mDoSpawn )
				continue;

			tParent parent;

			tGrowableArray< tSceneRefEntityPtr > * unused = table.fFind( 
				info.mElement->fSgResourcePtr( )->fGetPath( ) );

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

			fApplyProperties( def, parent, *info.mElement, gatherer.mRenderables.fBegin( ) );

			newParents.fPushBack( parent );
		}

		mParents.fDeleteArray( );
		newParents.fDisown( mParents );

		mRenderables.fDeleteArray( );
		gatherer.mRenderables.fDisown( mRenderables );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fSpawn(
		const tGroundCoverCloudDef * def,
		u32 cX, u32 cZ, f32 pX, f32 pZ,
		tGrowableArray<tSpawnInfo> & spawns, 
		b32 skipHeights )
	{
		const tGroundCoverCloudDef::tElement * elements = def->fElements( );
		const u32 elementCount = def->fElementCount( );

		f32 a = tMersenneGenerator( *(u32*)&pX ).fFloatInRange( -pZ, pZ );
		f32 b = tMersenneGenerator( *(u32*)&pZ ).fFloatInRange( -pX, pX );
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
				tSpawnInfo info;
				
				info.mElement = &element;
				info.mLocalXform = Math::tMat3f::cIdentity;
				info.mLocalXform.mRow0.w = pX; info.mLocalXform.mRow2.w = pZ;

				// We do the spawn if the rand test passes the mask test and the frequency test
				info.mDoSpawn = ( ( byte )( spawnRand.fFloatZeroToOne( )  * 255 ) >= def->fMask( cX, cZ ) );
				info.mDoSpawn = info.mDoSpawn && ( spawnRand.fFloatZeroToOne( ) <= element.fFrequency( ) );

				if( !skipHeights )
					info.mLocalXform.mRow1.w = def->fHeight( cX, cZ, spawns.fCount( ) );

				Math::tVec3f translation( 0 );
				if( fGetTranslation( def, transRand, translation ) )
					info.mLocalXform.fTranslateGlobal( translation );

				Math::tEulerAnglesf angles( 0 );
				if( fGetRotation( def, rotRand, angles ) )
					info.mLocalXform *= Math::tMat3f( Math::tQuatf( angles ) );

				f32 scale;
				if( fGetScale( def, scaleRand, scale ) )
					info.mLocalXform.fScaleLocal( Math::tVec3f( scale ) );
				

				// Add the spawn info
				spawns.fPushBack( info );
			}
		}

	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fMoveTo( 
		const tGroundCoverCloudDef * def, 
		const Math::tMat3f & ownerWorld, 
		u32 cX, u32 cZ, f32 pX, f32 pZ )
	{
		tGrowableArray<tSpawnInfo> spawns;
		fSpawn( def, cX, cZ, pX, pZ, spawns );

		u32 p = 0;
		const u32 spawnCount = spawns.fCount( );
		for( u32 s = 0; s < spawnCount; ++s )
		{
			const tSpawnInfo & info = spawns[ s ];
			if( !info.mDoSpawn )
				continue;

			sigassert( mParents[ p ].mEntity->fSgResource( ) == info.mElement->fSgResourcePtr( ) );
			mParents[ p++ ].mEntity->fMoveTo( ownerWorld * info.mLocalXform );
		}

		sigassert( p == mParents.fCount( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fApplyProperties( 
		const tGroundCoverCloudDef * def,
		const tGroundCoverCloudDef::tElement & element )
	{
		const u32 parentCount = mParents.fCount( );
		for( u32 p = 0; p < parentCount; ++p )
		{
			tParent & parent = mParents[ p ];
			if( parent.mEntity->fSgResource( ) != element.fSgResourcePtr( ) )
				continue;

			fApplyProperties( def, parent, element, mRenderables.fBegin( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fDestroy( tUnusedTable & table ) 
	{ 
		// Add our parents to the unused table
		const u32 parentCount = mParents.fCount( );
		for( u32 p = 0; p < parentCount; ++p )
		{
			const tParent & parent = mParents[ p ];
			const tFilePathPtr & path = parent.mEntity->fSgResource( )->fGetPath( );

			tGrowableArray< tSceneRefEntityPtr > * unused = table.fFind( path );
			if( !unused )
				unused = table.fInsert( path, tGrowableArray< tSceneRefEntityPtr >( ) );

			unused->fPushBack( parent.mEntity );
		}

		fDestroy( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tRenderList::fApplyProperties( 
		const tGroundCoverCloudDef * def,
		tParent & parent, 
		const tGroundCoverCloudDef::tElement & element, 
		tRenderableEntity * renderables[] )
	{
		// Apply element properties to each renderable entity
		for( u32 r = parent.mRenderableStart; r < parent.mRenderableEnd; ++r )
		{
			tRenderableEntity * renderable = renderables[ r ];
			renderable->fSetCastsShadow( element.fCastsShadow( ) );
			renderable->fSetFadeSettingOnThis( *parent.mEntity, Gfx::tRenderableEntity::cFadeNever, def ? gGatherRadii[ def->fVisibility( ) ] : 0.f );
		}
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::tRenderList::fGetRotation( 
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
		default:
			// Assert?
			break;
		}

		return !angles.fIsZero( );
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::tRenderList::fGetTranslation( 
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
		default:
			// Assert?
			break;
		}

		return !offset.fIsZero( );
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverCloud::tRenderList::fGetScale( 
		const tGroundCoverCloudDef * def, tMersenneGenerator & random, f32 & scale )
	{
		if( !def->fHasScaleRange( ) )
			return false;

		f32 range = def->fScaleRange( );
		scale = random.fFloatInRange( 1.f - range, 1.f + range );
		return scale != 1;
	}

	//------------------------------------------------------------------------------
	// tCell
	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tCell::fInstantiate( tRenderList & oldOwner ) 
	{ 
		sigassert( !mInstantiated );

		mList.fDestroy( );
		mList = oldOwner;

		oldOwner.fClear( );

		mInstantiated = true; 
	}

	//------------------------------------------------------------------------------
	void tGroundCoverCloud::tCell::fRelease( tRenderList & newOwner ) 
	{ 
		sigassert( mInstantiated );

		newOwner = mList; 
		mList.fClear( ); 

		mInstantiated = false; 
	}
	
}}
