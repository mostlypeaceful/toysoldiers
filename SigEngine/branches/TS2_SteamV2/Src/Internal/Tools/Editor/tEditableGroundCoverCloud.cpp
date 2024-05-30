//------------------------------------------------------------------------------
// \file tEditableGroundCoverCloud.cpp - 16 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tEditableGroundCoverCloud.hpp"
#include "tEditableObjectContainer.hpp"
#include "tHeightFieldMeshEntity.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tEditableGroundCoverCloud::tEditableGroundCoverCloud( tEditableObjectContainer & container, f32 xLen, f32 zLen )
		: Gfx::tGroundCoverCloud( &mDefInstance )
		, mContainer( container )
		, mNeedsHeights( false )
		, mVisible( true )
		, mDirtyFlags( cDirtyNone )
	{
		mOnLoadComplete.fFromMethod< 
			tEditableGroundCoverCloud, 
			&tEditableGroundCoverCloud::fOnResourceLoaded>( this );

		mDefInstance.mVisibility = Gfx::tGroundCoverCloudDef::cVisibilityForever;
		mDefInstance.mWorldLengthX = xLen;
		mDefInstance.mWorldLengthZ = zLen;
	}

	//------------------------------------------------------------------------------
	tEditableGroundCoverCloud::~tEditableGroundCoverCloud( )
	{
		const u32 resCount = mResources.fCount( );
		for( u32 r = 0; r < resCount; ++r )
		{
			mResources[ r ]->fGetResourcePtr( )->fUnload( this );
			delete mResources[ r ];
		}
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fSetDimensions( f32 xLen, f32 zLen )
	{
		mDefInstance.mWorldLengthX = xLen;
		mDefInstance.mWorldLengthZ = zLen;
	}


	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateDef( 
		const Sigml::tGroundCoverLayer & layer,
		b32 updateCover  )
	{
		mVisible = layer.fVisible( );

		//NOTE - we purposely ignore the visibility range

		b32 newPaintUnits = mDefInstance.mPaintUnits != layer.fPaintUnits( );
		if( newPaintUnits )
		{
			mDefInstance.mPaintUnits = layer.fPaintUnits( );
			Gfx::tGroundCoverCloudDef::fConvertMask( 
				mDefInstance.mDimX, mDefInstance.mDimZ, mFullMask.fBegin( ), 
				mDefInstance.mPaintUnits, 
				mDefInstance.mMaskDimX, mDefInstance.mMaskDimZ, mDefInstance.mMask );
		}

		b32 newUnitSize =  mDefInstance.mUnitSize != layer.fUnitSize( );
		if( newUnitSize )
		{
			mDefInstance.mUnitSize = layer.fUnitSize( );
		}

		b32 newRotation = mDefInstance.mRotation != layer.fRotation( );
		if( newRotation )
		{
			mDefInstance.mRotation = (Gfx::tGroundCoverCloudDef::tRotation)layer.fRotation( );
		}

		if( mDefInstance.mYRotationScale != layer.fYRotationScale( ) )
		{
			mDefInstance.mYRotationScale = layer.fYRotationScale( );
			newRotation = newRotation ||
				mDefInstance.mRotation != Gfx::tGroundCoverCloudDef::cRotationNone;
		}

		if( mDefInstance.mXZRotationScale != layer.fXZRotationScale( ) )
		{
			mDefInstance.mXZRotationScale = layer.fXZRotationScale( );
			newRotation = newRotation ||
				mDefInstance.mRotation == Gfx::tGroundCoverCloudDef::cRotationRandomXYZ;
		}

		b32 newTranslation = mDefInstance.mTranslation != layer.fTranslation( );
		if( newTranslation )
			mDefInstance.mTranslation = (Gfx::tGroundCoverCloudDef::tTranslation)layer.fTranslation( );

		if( mDefInstance.mXZTranslationScale != layer.fXZTranslationScale( ) )
		{
			mDefInstance.mXZTranslationScale = layer.fXZTranslationScale( );
			newTranslation = newTranslation || 
				( mDefInstance.mTranslation != Gfx::tGroundCoverCloudDef::cTranslationNone );
		}

		if( mDefInstance.mYTranslationScale != layer.fYTranslationScale( ) )
		{
			mDefInstance.mYTranslationScale = layer.fYTranslationScale( );
			newTranslation = newTranslation || 
				( mDefInstance.mTranslation == Gfx::tGroundCoverCloudDef::cTranslationXYZ );
		}

		b32 newScale = mDefInstance.mScaleRangeAdjustor != layer.fScaleRangeAdjustor( );
		if( newScale )
			mDefInstance.mScaleRangeAdjustor = layer.fScaleRangeAdjustor( );

		const u32 elementCount = layer.fElementCount( );
		const Sigml::tGroundCoverLayer::tElement * elements = layer.fElements( );
		
		b32 newElements = false;

		// Before doing all this work, let's check if they're the same
		if( elementCount != mDefInstance.fElementCount( ) )
			newElements = true;
		else
		{
			for( u32 e = 0; e < elementCount; ++e )
			{
				const Gfx::tGroundCoverCloudDef::tElement & from = mDefInstance.mElements[ e ];
				const Sigml::tGroundCoverLayer::tElement & to = elements[ e ];

				tFilePathPtr binPath = tResourceConvertPath<tSceneGraphFile>::fConvertToBinary( to.mSgPath );

				if( binPath == from.fSgResourcePtr( )->fGetPath( ) )
					continue;

				newElements = true;
				break;
			}
		}

		if( newElements )
		{
			tGrowableArray< tLoadInPlaceResourcePtr * > oldResources = mResources;
			
			mResources.fSetCount( 0 );
			mDefInstance.mElements.fNewArray( elementCount );

			for( u32 e = 0; e < elementCount; ++e )
			{
				tLoadInPlaceResourcePtr * ptr = 0;

				const Sigml::tGroundCoverLayer::tElement & from = elements[ e ];

				tResourceId rid = tResourceId::fMake<tSceneGraphFile>( from.mSgPath );

				// Search in main list
				const u32 newResCount = mResources.fCount( );
				for( u32 r = 0; r < newResCount; ++r )
				{
					if( mResources[ r ]->fGetResourceId( ) != rid )
						continue;

					ptr = mResources[ r ];
					break;
				}

				// Search in old list
				if( !ptr )
				{
					const u32 oldResCount = oldResources.fCount( ); 
					for( u32 r = 0; r < oldResCount; ++r )
					{
						if( oldResources[ r ]->fGetResourceId( ) != rid )
							continue;

						ptr = oldResources[ r ];
						mResources.fPushBack( ptr );
						oldResources.fErase( r );
						
						break;
					}
				}

				// Create
				if( !ptr )
				{
					ptr = new tLoadInPlaceResourcePtr( );
					fZeroOut( ptr );

					ptr->mClassId = rid.fGetClassId( );
					ptr->mRawPath.fCreateNullTerminated( rid.fGetPath( ).fCStr( ) );
					ptr->mFilePathPtr.fConstruct( ptr->mRawPath.fBegin( ) );
					ptr->mResourcePtr.fConstruct( );

					tResourcePtr & resPtr = ptr->mResourcePtr.fTreatAsObject( );
					resPtr = mContainer.fGetResourceDepot( )->fQuery( ptr->fGetResourceId( ) );
					resPtr->fLoadDefault( this );
					mResources.fPushBack( ptr );
				}

				Gfx::tGroundCoverCloudDef::tElement & to = mDefInstance.mElements[ e ];
				to.mSgFile = ptr;
				to.mCastsShadow = from.mCastsShadow;
				to.mFrequency = from.mFrequency;
				to.mSpawnCount = from.mSpawnCount;

			}

			// Delete old resources that are no longer used
			const u32 oldResCount = oldResources.fCount( );
			for( u32 r = 0; r < oldResCount; ++r )
			{
				oldResources[ r ]->fGetResourcePtr( )->fUnload( this );
				delete oldResources[ r ];
			}
		}

		if( newUnitSize || newTranslation )
			mNeedsHeights = true;

		if( newPaintUnits || newUnitSize )
			mDirtyFlags |= cDirtyCells;

		if( newElements )
			mDirtyFlags |= cDirtyElements;

		if( newRotation || newTranslation || newScale )
			mDirtyFlags |= cDirtyXforms;

		if( updateCover && mDirtyFlags )
			fUpdateCover( );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateDef( 
		const tTerrainGeometry::tGroundCover & gc, b32 updateCover  )
	{

		if( mDefInstance.mDimX != gc.mDimX || 
			mDefInstance.mDimZ != gc.mDimZ || 
			mDefInstance.mMaxUnitSpawns != gc.mMaxUnitSpawns )
		{
			mDefInstance.mDimX = gc.mDimX;
			mDefInstance.mDimZ = gc.mDimZ;
			mDefInstance.mMaxUnitSpawns = gc.mMaxUnitSpawns;
			
			mDefInstance.mHeights.fNewArray( gc.mHeights.fCount( ) );
		}

		// We can only do the conversion now if we have a valid paint units
		mFullMask = gc.mMask;
		if( mDefInstance.mPaintUnits )
		{
			Gfx::tGroundCoverCloudDef::fConvertMask(
				gc.mDimX, gc.mDimZ, gc.mMask.fBegin( ), 
				mDefInstance.mPaintUnits, 
				mDefInstance.mMaskDimX, mDefInstance.mMaskDimZ, mDefInstance.mMask );
		}

		fMemCpy(
			mDefInstance.mHeights.fBegin( ),
			gc.mHeights.fBegin( ),
			mDefInstance.mHeights.fTotalSizeOf( ) );

		mDirtyFlags |= ( cDirtyCells | cDirtyXforms );

		if( updateCover )
			fUpdateCover( );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateMask( 
		u32 minX, u32 minZ, u32 maxX, u32 maxZ, const f32 mask[] )
	{
		sigassert( mDefInstance.mPaintUnits );

		sigassert( minX <= mDef->fDimX( ) );
		sigassert( maxX <= mDef->fDimX( ) );
		sigassert( minZ <= mDef->fDimZ( ) );
		sigassert( maxZ <= mDef->fDimZ( ) );

		const u32 xdim = maxX - minX;
		const u32 zdim = maxZ - minZ;

		if( !xdim || !zdim )
			return;

		for( u32 zS = 0, zD = minZ; zS < zdim;  ++zS, ++zD )
		{
			fMemCpy( 
				&mFullMask[ zD * mDefInstance.mDimX + minX ],
				&mask[ zS * xdim ],
				sizeof( *mask ) * xdim );

			//for (u32 xS = 0, xD = minX; xS < xdim; ++xS, ++xD )
			//	mFullMask[ zD * mDefInstance.mDimX + xD ] = mask[ zS * xdim + xS ];
		}

		Gfx::tGroundCoverCloudDef::fConvertMask( 
			mDefInstance.mDimX, mDefInstance.mDimZ, mFullMask.fBegin( ), 
			mDefInstance.mPaintUnits, 
			mDefInstance.mMaskDimX, mDefInstance.mMaskDimZ, mDefInstance.mMask );


		minX = ( minX / mDefInstance.mPaintUnits ) * mDefInstance.mPaintUnits;
		minZ = ( minZ / mDefInstance.mPaintUnits ) * mDefInstance.mPaintUnits;
		maxX = ( ( maxX + mDefInstance.mPaintUnits ) / mDefInstance.mPaintUnits ) * mDefInstance.mPaintUnits;
		maxZ = ( ( maxZ + mDefInstance.mPaintUnits ) / mDefInstance.mPaintUnits ) * mDefInstance.mPaintUnits;

		// Reinstantiate any affected cells
		tGroundCoverCloud::fInstantiateCells( 
			fClamp( minX, 0u, mDefInstance.mDimX ), 
			fClamp( minZ, 0u, mDefInstance.mDimZ ), 
			fClamp( maxX, 0u, mDefInstance.mDimX ), 
			fClamp( maxZ, 0u, mDefInstance.mDimZ ), 
			true );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateHeights( const u32 count, const f32 * heights, b32 skipRefresh )
	{
		const u32 dimx = mDefInstance.fDimX( );
		const u32 dimz = mDefInstance.fDimZ( );
		sigassert( count == ( dimx * dimz ) * mDefInstance.mMaxUnitSpawns );

		fUpdateHeights( 0, 0, dimx, dimz, heights, skipRefresh );

		// We just updated all the xforms
		if( !skipRefresh )
			mDirtyFlags &= ~cDirtyXforms;
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateHeights( 
		u32 minX, u32 minZ, u32 maxX, u32 maxZ, const f32 * heights, b32 skipRefresh )
	{
		sigassert( minX <= mDefInstance.mDimX );
		sigassert( minZ <= mDefInstance.mDimZ );
		sigassert( maxX <= mDefInstance.mDimX );
		sigassert( maxZ <= mDefInstance.mDimZ );

		const u32 xdim = maxX - minX;
		const u32 zdim = maxZ - minZ;

		if( !xdim || !zdim )
			return;

		const u32 totalXDim = mDefInstance.mDimX;
		for( u32 zS = 0, zD = minZ; zS < zdim; ++zS, ++zD )
		{
			fMemCpy( 
				&mDefInstance.mHeights[ ( zD * totalXDim + minX ) * mDefInstance.mMaxUnitSpawns ], 
				&heights[ ( zS * xdim ) * mDefInstance.mMaxUnitSpawns ], 
				( sizeof( *heights ) * xdim ) * mDefInstance.mMaxUnitSpawns );
		}

		if( !skipRefresh )
			fRefreshXforms( minX, minZ, maxX, maxZ );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateShadows( const Sigml::tGroundCoverLayer & layer )
	{
		const u32 elementCount = mDefInstance.mElements.fCount( );
		for( u32 e = 0; e < elementCount; ++e )
		{
			Gfx::tGroundCoverCloudDef::tElement & to = mDefInstance.mElements[ e ];

			const Sigml::tGroundCoverLayer::tElement * from = layer.fFindElement(
				tResourceConvertPath<tSceneGraphFile>::fConvertToSource( to.mSgFile->fGetFilePathPtr( ) ) );
			sigassert( from );

			// They're the same, nothing to do
			if( to.fCastsShadow( ) == from->mCastsShadow )
				continue;

			to.mCastsShadow = from->mCastsShadow;
			
			const u32 cellCount = mCells.fCount( );
			for( u32 c = 0; c < cellCount; ++c )
				mCells[ c ].fRenderList( ).fApplyProperties( 0, to );
		}
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateFrequency( const Sigml::tGroundCoverLayer & layer )
	{
		const u32 elementCount = mDefInstance.mElements.fCount( );
		for( u32 e = 0; e < elementCount; ++e )
		{
			Gfx::tGroundCoverCloudDef::tElement & to = mDefInstance.mElements[ e ];

			const Sigml::tGroundCoverLayer::tElement * from = layer.fFindElement(
				tResourceConvertPath<tSceneGraphFile>::fConvertToSource( to.mSgFile->fGetFilePathPtr( ) ) );
			sigassert( from );

			// They're the same, nothing to do
			if( to.fFrequency( ) == from->mFrequency )
				continue;

			to.mFrequency = from->mFrequency;
			mDirtyFlags |= cDirtyCells;
		}

		if( mDirtyFlags )
			fUpdateCover( );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateSpawnCount( const Sigml::tGroundCoverLayer & layer )
	{
		const u32 elementCount = mDefInstance.mElements.fCount( );
		for( u32 e = 0; e < elementCount; ++e )
		{
			Gfx::tGroundCoverCloudDef::tElement & to = mDefInstance.mElements[ e ];

			const Sigml::tGroundCoverLayer::tElement * from = layer.fFindElement(
				tResourceConvertPath<tSceneGraphFile>::fConvertToSource( to.mSgFile->fGetFilePathPtr( ) ) );
			sigassert( from );

			// They're the same, nothing to do
			if( to.fSpawnCount( ) == from->mSpawnCount )
				continue;

			to.mSpawnCount = from->mSpawnCount;
			mDirtyFlags |= cDirtyCells;
		}

		//if( mDirtyFlags )
		//	fUpdateCover( );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fPrepareRenderables( const Gfx::tCamera & camera )
	{
		if( mOnLoadComplete.fOwnerEventCount( ) || !mVisible )
			return;

		tGroundCoverCloud::fPrepareRenderables( camera );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fGatherRenderables( tGatherCb * cb, b32 forShadows )
	{
		if( mOnLoadComplete.fOwnerEventCount( ) || !mVisible )
			return;

		tGroundCoverCloud::fGatherRenderables( cb, forShadows );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fUpdateCover( )
	{
		// Not spawned
		if( !fParent( ) )
			return;

		// Waiting on resources
		const u32 resCount = mResources.fCount( );
		for( u32 r = 0; r < resCount; ++r )
		{
			const tResourcePtr & res = mResources[ r ]->fGetResourcePtr( );
			if( !res->fLoading( ) )
				continue;

			res->fCallWhenLoaded( mOnLoadComplete );
			return;
		}

		b32 cellsDestroyed = false;

		const u32 dimx = mDefInstance.fDimX( );
		const u32 dimz = mDefInstance.fDimZ( );
		if( mCells.fCount( ) != dimx * dimz )
		{
			mDirtyFlags |= cDirtyCells;

			// If we have new elements then we need to free resources
			if( mDirtyFlags & cDirtyElements )
			{
				fAllocateCells( dimx * dimz );
				mDirtyFlags &= ~cDirtyElements;
			}

			// Otherwise reallocate, but save unused
			else
				fAllocateCells( dimx * dimz, mUnused );

			mGatherRect.fInvalidate( );
			cellsDestroyed = true;
		}

		if( mDirtyFlags & ( cDirtyCells | cDirtyElements ) )
		{
			if( mDirtyFlags & cDirtyElements )
			{
				const u32 cellCount = mCells.fCount( );
				for( u32 c = 0; c < cellCount; ++c )
					mCells[ c ].fDestroy( );

				mUnused.fClear( );

				mDirtyFlags &= ~cDirtyElements;
			}
			else if( !cellsDestroyed )
			{
				const u32 cellCount = mCells.fCount( );
				for( u32 c = 0; c < cellCount; ++c )
					mCells[ c ].fDestroy( mUnused );
			}
			
			// TODO: optimize
			fInstantiateCells( 0, 0, dimx, dimz, true );

			// Clear all since we're recreating all the cells
			mDirtyFlags = cDirtyNone;
		}

		if( mDirtyFlags & cDirtyXforms )
		{
			fRefreshXforms( 0, 0, mDef->fDimX( ), mDef->fDimZ( ) );
			mDirtyFlags &= ~cDirtyXforms;
		}
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fOnResourceLoaded( tResource & theResource, b32 success )
	{
		// Make sure we're not registered with any resources
		mOnLoadComplete.fRemoveFromAllEvents( );

		fUpdateCover( );
	}

	//------------------------------------------------------------------------------
	void tEditableGroundCoverCloud::fRefreshXforms( u32 minX, u32 minZ, u32 maxX, u32 maxZ )
	{
		sigassert( minX <= mDef->fDimX( ) );
		sigassert( maxX <= mDef->fDimX( ) );
		sigassert( minZ <= mDef->fDimZ( ) );
		sigassert( maxZ <= mDef->fDimZ( ) );

		const f32 unitSize = mDefInstance.fUnitSize( );
		const Math::tMat3f & objectToWorld = fObjectToWorld( );

		f32 zP = fZPos( minZ );
		for( u32 z = minZ; z < maxZ; ++z, zP += unitSize  )
		{
			f32 xP = fXPos( minX );
			for( u32 x = minX; x < maxX; ++x, xP += unitSize )
			{
				tCell & cell = mCells[ z * mDefInstance.mDimX + x ];

				if( cell.fIsDeadOrEmpty( ) )
					continue;

				cell.fRenderList( ).fMoveTo(mDef, objectToWorld, x, z, xP, zP );
			}
		}
	}
}
