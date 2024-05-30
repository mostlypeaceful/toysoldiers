//------------------------------------------------------------------------------
// \file tSkeletableSgFileRefEntity.cpp - 23 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigAnimPch.hpp"
#include "tSkeletableSgFileRefEntity.hpp"
#include "tAnimPackFile.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tSkeletableSgFileRefEntity::tSkeletableSgFileRefEntity( 
		tEditableObjectContainer & container, 
		const tResourcePtr & sigbResource )
		: tEditableSgFileRefEntity( container, sigbResource )
		, mIsCursor( false )
		, mIsPaused( false )
		, mForceStep( 0 )
		, mTimeScale( 1 )
		, mTimeReversed( false )
		, mApplyRefFrame( false )
		, mMotionRange( 20 )
		, mRenderSkeleton( false )
		, mUpdatingXform( false )
	{
		fCommonConstruct( );
		sigbResource->fCallWhenLoaded( mOnSkelResourceLoaded );
	}

	//------------------------------------------------------------------------------
	tSkeletableSgFileRefEntity::tSkeletableSgFileRefEntity( 
		tEditableObjectContainer& container, 
		const tResourcePtr & sigbResource,
		const tResourcePtr & skelResource )
		: tEditableSgFileRefEntity( container, sigbResource )
		, mSkelResource( skelResource )
		, mIsCursor( false )
		, mIsPaused( false )
		, mForceStep( 0 )
		, mTimeScale( 1 )
		, mTimeReversed( false )
		, mApplyRefFrame( false )
		, mMotionRange( 20 )
		, mRenderSkeleton( false )
		, mUpdatingXform( false )
	{
		fCommonConstruct( );
		skelResource->fLoadDefault( this );
		skelResource->fCallWhenLoaded( mOnSkelResourceLoaded );

		mAnimPackInfo.mSkeletonsToProcess.fPushBack( mSkelResource );
	}

	//------------------------------------------------------------------------------
	tSkeletableSgFileRefEntity::~tSkeletableSgFileRefEntity( )
	{
		if( mSkelResource )
			mSkelResource->fUnload( this );

		const u32 animPackCount = mAnimPackInfo.mAnimPacks.fCount( );
		for( u32 a = 0; a < animPackCount; ++a )
			mAnimPackInfo.mAnimPacks[a]->fUnload( this );
	}

	//------------------------------------------------------------------------------
	void tSkeletableSgFileRefEntity::fSetSkeletonResource( const tResourcePtr & skelRes )
	{
		sigassert( !mSkelResource && !mSkeleton && !mSgResource->fLoading( ) );
		sigassert( skelRes );

		mSkelResource = skelRes;
		mSkelResource->fLoadDefault( this );
		mSkelResource->fCallWhenLoaded( mOnSkelResourceLoaded );

		mAnimPackInfo.mSkeletonsToProcess.fPushBack( mSkelResource );
	}

	//------------------------------------------------------------------------------
	b32 tSkeletableSgFileRefEntity::fSkeletonResIsFromSgFile( ) const
	{
		// No base resource so no
		if( !mSgResource )
			return false;
		
		// No skel resource so we will take it or there is none
		if( !mSkelResource )
			return true;

		// If there is a skel resource and we're not loaded then no
		if( !mSgResource->fLoaded( ) )
			return false;

		const tSceneGraphFile * file = mSgResource->fCast<tSceneGraphFile>( );
		sigassert( file );

		// Does the skel res match the sg res skeleton?
		return file->mSkeletonFile && ( file->mSkeletonFile->fGetResourcePtr( ) == mSkelResource );
	}

	//------------------------------------------------------------------------------
	void tSkeletableSgFileRefEntity::fAddAnimPack( const tResourcePtr & pack )
	{
		// Sanity check that it's the right kind of resource
		sigassert(pack->fGetClassId( ) == Rtti::fGetClassId<tAnimPackFile>( ) );
		
		// No duplicates
		if( mAnimPackInfo.mAnimPacks.fFind( pack ) )
			return;

		// Load and add
		pack->fLoadDefault( this );
		mAnimPackInfo.mAnimPacks.fPushBack( pack );
	}

	//------------------------------------------------------------------------------
	tEntityPtr tSkeletableSgFileRefEntity::fClone( )
	{
		tSkeletableSgFileRefEntity * ptr;
		for( ;; )
		{
			// No skel resource means we're a one file pony
			if( !mSkelResource )
			{
				ptr = new tSkeletableSgFileRefEntity( mContainer, mSgResource );
				break;
			}

			// We have a skeleton resource, but we didn't load the sg res
			// which means we're a two file pony
			if( mSgResource->fLoading( ) || mSgResource->fLoadFailed( ) )
			{
				ptr = new tSkeletableSgFileRefEntity( mContainer, mSgResource, mSkelResource );
				break;
			}

			const tSceneGraphFile * sgFile = mSgResource->fCast<tSceneGraphFile>( );
			sigassert( sgFile );

			// If the skeleton is the same as the skeleton referenced we're a one filer
			if( sgFile->mSkeletonFile && mSkelResource == sgFile->mSkeletonFile->fGetResourcePtr( ) )
			{
				ptr = new tSkeletableSgFileRefEntity( mContainer, mSgResource );
				break;
			}

			// Deuce files!
			ptr = new tSkeletableSgFileRefEntity( mContainer, mSgResource, mSkelResource );
			break;
		}

		ptr->fMoveTo( fObjectToWorld( ) );
		ptr->fAddToWorld( );

		return tEntityPtr( ptr );
	}

	//------------------------------------------------------------------------------
	void tSkeletableSgFileRefEntity::fOnSpawn( )
	{
		tEditableSgFileRefEntity::fOnSpawn( );
		fRunListInsert( tLogic::cRunListThinkST );
	}

	//------------------------------------------------------------------------------
	void tSkeletableSgFileRefEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tEditableSgFileRefEntity::fOnMoved( recomputeParentRelative );

		if( !mUpdatingXform )
			mBaseXform = fObjectToWorld( );
	}

	//------------------------------------------------------------------------------
	void tSkeletableSgFileRefEntity::fThinkST( f32 dt )
	{
		if( mIsCursor )
			return;

		// Fix up the dt
		dt *= mTimeScale;
		if( mTimeReversed )
			dt *= -1;

		// Handle the skeleton
		if( mSkeleton )
		{
			if( mForceStep )
			{
				dt = mForceStep;
				mForceStep = 0;
			}
			else if( mIsPaused )
			{
				dt = 0;
			}

			mSkeleton->fStep( dt );

			if( dt != 0 )
			{
				if( mApplyRefFrame )
				{
					Math::tMat3f xform = fObjectToWorld( );
					mSkeleton->fApplyRefFrameDelta( xform );

					// don't let the entity get too far away
					{
						const Math::tVec3f basePos = mBaseXform.fGetTranslation( );
						const Math::tVec3f diff = xform.fGetTranslation( ) - basePos;
						if( diff.fLengthSquared( ) > Math::fSquare( mMotionRange ) )
							xform.fSetTranslation( basePos );
					}

					mUpdatingXform = true;
					fMoveTo( xform );
					mUpdatingXform = false;
				}
				else
				{
					mUpdatingXform = true;
					fMoveTo( mBaseXform );
					mUpdatingXform = false;
				}
			}

			if( mRenderSkeleton )
				mRenderableSkeleton.fSyncToSkeleton( fObjectToWorld( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tSkeletableSgFileRefEntity::fCommonConstruct( )
	{
		mBaseXform = fObjectToWorld( );
		mOnSkelResourceLoaded.fFromMethod<
			tSkeletableSgFileRefEntity, &tSkeletableSgFileRefEntity::fOnSkelLoaded
		>( this );

		mOnResourceLoaded.fFromMethod<
			tSkeletableSgFileRefEntity, &tSkeletableSgFileRefEntity::fOnResLoaded
		>( this );
	}

	//------------------------------------------------------------------------------
	void tSkeletableSgFileRefEntity::fOnSkelLoaded( tResource& theResource, b32 success )
	{
		if( !success )
			return;

		const tSceneGraphFile * sgFile = mSgResource->fCast<tSceneGraphFile>( );
		if( !sgFile )
			return;

		if( !mSkelResource )
		{
			sigassert( &theResource == mSgResource.fGetRawPtr( ) );

			if( !sgFile->mSkeletonFile )
				return;

			mSkelResource = sgFile->mSkeletonFile->fGetResourcePtr( );
			mSkelResource->fLoadDefault( this ); // Add a reference so we can blindly remove later

			mAnimPackInfo.mSkeletonsToProcess.fPushBack( mSkelResource );
		}

		mSkeleton.fReset( new tAnimatedSkeleton( mSkelResource ) );
		mSkeleton->fSetBindOffset( sgFile->mSkeletonBinding, sgFile->mSkeletonBindingInv );
		mSkeleton->fSetToIdentity( );

		if( &theResource != mSgResource.fGetRawPtr( ) )
			mSgResource->fCallWhenLoaded( mOnResourceLoaded );
		else
			fOnResLoaded( theResource, success );
	}

	//------------------------------------------------------------------------------
	void tSkeletableSgFileRefEntity::fOnResLoaded( tResource & theResource, b32 success )
	{
		fPropagateSkeleton( *mSkeleton );
	}
}
