//------------------------------------------------------------------------------
// \file tAnimatedSkeleton.cpp - 15 Jul 2008
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tSkeletonFile.hpp"
#include "tSceneGraph.hpp"
#include "tProfiler.hpp"

#include "tKeyFrameAnimTrack.hpp"
#include "Logic/tAnimatable.hpp"

namespace Sig
{
	devvar( bool, Perf_BoneProxyIsolated, false );

	tBoneProxy::tBoneProxy( tEntity& parent, tAnimatedSkeleton& skeleton, u32 boneIndex )
		: mBindPose( skeleton.fBindOffsetInverse( ) * parent.fParentRelative( ) )
		, mParentRelativeOg( parent.fParentRelative( ) )
		, mParentRelativeCur( parent.fParentRelative( ) )
		, mParent( &parent )
		, mBoneIndex( boneIndex )
	{
	}
	void tBoneProxy::fRecomputeParentRelative( const tAnimatedSkeleton& skeleton )
	{
		mParentRelativeCur = skeleton.fBindOffset( ) * skeleton.fMatrixPalette( )[ mBoneIndex ] * mBindPose;
	}
	void tBoneProxy::fUpdateParent( ) const
	{
		if( !Perf_BoneProxyIsolated )
			mParent->fSetParentRelativeXform( mParentRelativeCur );
		else
			mParent->fSetParentRelativeXformIsolated( mParentRelativeCur ); //this line is a tiny bit faster (does not recursively update children), but risks the mObjectToWorld transform being out of sync
	}
	void tBoneProxy::fResetParent( ) const
	{
		mParent->fSetParentRelativeXform( mParentRelativeOg );
		//mParent->fSetParentRelativeXformIsolated( mParentRelativeOg ); //this line is a tiny bit faster (does not recursively update children), but risks the mObjectToWorld transform being out of sync
	}


	b32 tAnimatedSkeleton::fIsReachedEndOfOneShotEvent( const Logic::tEvent& e )
	{
		if( e.fEventId( ) == tKeyFrameAnimTrack::fEventKeyFrameID( ) )
		{
			const tKeyFrameEventContext* context = e.fContext< tKeyFrameEventContext >( );
			if( context->mEventTypeCppValue == Logic::AnimationEvent::cEventReachedEndOneShot )
				return true;
		}

		return false;
	}

	tAnimatedSkeleton::tAnimatedSkeleton( const tResourcePtr& skeletonRes )
		: mSkeleton( skeletonRes )
		, mRefFrameDelta( Math::tPRSXformf::cZeroXform )
		, mBindOffset( Math::tMat3f::cIdentity )
		, mBindOffsetInverse( Math::tMat3f::cIdentity )
		, mTimeScale( 1.0f )
	{
		const tSkeletonFile* skeleton = mSkeleton->fCast< tSkeletonFile >( );
		sigassert( skeleton );
		mEvaluationResult.mBoneResults.fNewArray( skeleton->mMasterBoneList.fCount( ) );
		mEvaluationResult.mBoneResults.fFill( Math::tPRSXformf::cIdentity );
		mMatrixPalette.fNewArray( skeleton->mMasterBoneList.fCount( ) );
		//mMatrixPalette.fFill( Math::tMat3f::cIdentity );
	}

	tAnimatedSkeleton::~tAnimatedSkeleton( )
	{
	}

	void tAnimatedSkeleton::fPushTrack( const tAnimTrackPtr& animTrack )
	{
		fPushAnimTrack(mAnimTracks, animTrack);
	}

	void tAnimatedSkeleton::fPushPostTrack( const tAnimTrackPtr& animTrack )
	{
		fPushAnimTrack(mPostAnimTracks, animTrack);
	}

	void tAnimatedSkeleton::fPushTrackBeforeTag( const tAnimTrackPtr& animTrack, const tStringPtr& tag )
	{
		//s32 lastPostTrack = -1;
		u32 tCnt = mAnimTracks.fCount( );
		for( u32 t = 0; t < tCnt; ++t )
		{
			tAnimTrackPtr& atp = mAnimTracks[ t ];
			if( atp->fTag( ) == tag )
			{
				mAnimTracks.fInsert( t, animTrack );
				animTrack->fOnPushed(*this); // Signal that it's been added
				return;
			}
		}

		// No tag match found, just push it on the end
		fPushAnimTrack(mAnimTracks, animTrack);
	}

	void tAnimatedSkeleton::fStep( f32 dt )
	{
		fStepMTInJobs( dt );
		fStepMTMainThread( );
	}

	void tAnimatedSkeleton::fStepMTInJobs( f32 dt )
	{
		// reset queued events
		mKeyFrameEvents.fSetCount( 0 );

		const f32 deltaTime = mTimeScale * dt;

		if( deltaTime != 0.f )
		{
			// step all tracks, obtaining the combined/blended reference frame delta xform
			const u32 bottom = fHighestFullyBlendedTrack( );
			mRefFrameDelta = Math::tPRSXformf::cZeroXform;
			for( u32 i = 0; i < mAnimTracks.fCount( ); ++i )
				mAnimTracks[ i ]->fStep( mRefFrameDelta, *this, i == bottom, deltaTime );

			for( u32 i = 0; i < mPostAnimTracks.fCount( ); ++i )
				mPostAnimTracks[ i ]->fStep( mRefFrameDelta, *this, false, deltaTime );
		}
		else
		{
			mRefFrameDelta = Math::tPRSXformf::cZeroXform;
		}

		fEvaluate( );
	}

	void tAnimatedSkeleton::fStepMTMainThread( )
	{
		if( mTimeScale > 0.f )
		{
			fStepBoneProxiesMTMainThread( );
			fCullDeadTracks( );
			fFireEvents( );
		}

#ifdef sig_logging
		if( mAnimTracks.fCount( ) > 20 )
		{
			log_warning_nospam( Log::cFlagAnimation, "Track count on skeleton [" << mSkeleton->fGetPath( ) << "] is getting high: " << mAnimTracks.fCount( ) << "; logging stack tags:" );
			for( u32 i = 0; i < mAnimTracks.fCount( ); ++i )
			{
				if( mAnimTracks[ i ]->fTag( ).fExists( ) )
					log_line( 0, "\t" << mAnimTracks[ i ]->fTag( ) );
			}
		}
#endif//sig_logging
	}

	void tAnimatedSkeleton::fEvaluate( )
	{
		if( mAnimTracks.fCount( ) == 0 ) return;

		const tSkeletonFile* skeleton = mSkeleton->fCast< tSkeletonFile >( );
		//sigassert( skeleton ); // TODO should be able to assert
		if( !skeleton ) return;

		// evaluate all tracks (starting from highest fully blended track)
		const u32 bottom = fHighestFullyBlendedTrack( );
		for( u32 i = bottom; i < mAnimTracks.fCount( ); ++i )
			mAnimTracks[ i ]->fEvaluate( mEvaluationResult, *this, i == bottom );

		// convert evaluation result to matrices
		for( u32 i = 0; i < mEvaluationResult.mBoneResults.fCount( ); ++i )
			mEvaluationResult.mBoneResults[ i ].fToMatrix( mMatrixPalette[ i ] );

		// convert bones into model space
		const u16* preOrderTraversal	= skeleton->mPreOrderTraversal.fBegin( );
		const u16* preOrderTraversalEnd = skeleton->mPreOrderTraversal.fEnd( );
		for( ; preOrderTraversal != preOrderTraversalEnd; ++preOrderTraversal )
		{
			const u32 currentIndex = *preOrderTraversal;
			const u32 parentIndex  = skeleton->fBone( currentIndex ).mParent;
			mMatrixPalette[ currentIndex ] = mMatrixPalette[ parentIndex ] * mMatrixPalette[ currentIndex ];
		}

		// apply post animation processing
		for( u32 i = 0; i < mPostAnimTracks.fCount( ); ++i )
			mPostAnimTracks[ i ]->fPostAnimEvaluate( *this );

		// convert bone by reference pose xform
		for( u32 i = 0; i < mMatrixPalette.fCount( ); ++i )
			mMatrixPalette[ i ] = mMatrixPalette[ i ] * skeleton->fBone( i ).mRefPoseInv;

		fStepBoneProxiesMTInJobs( );
	}

	void tAnimatedSkeleton::fCullDeadTracks( )
	{
		profile( cProfilePerfCullDeadTracksST );

		// clear out any tracks that are underneath a stack-clearing track
		// (a stack-clearing track is one that is blended at full, non-partial,
		// and set to loop indefinitely)
		const u32 lowestStackClearingTrack = fHighestStackClearingTrack( );
		if( lowestStackClearingTrack > 0 )
		{
			for( u32 i = lowestStackClearingTrack; i < mAnimTracks.fCount( ); ++i )
				mAnimTracks[ i - lowestStackClearingTrack ] = mAnimTracks[ i ];

			mAnimTracks.fSetCount( mAnimTracks.fCount( ) - lowestStackClearingTrack );
		}

		// cull out dead (zero strength) tracks
		u32 culled = 0;
		for( u32 i = 0; i < mAnimTracks.fCount( ) - culled; ++i )
		{
			tAnimTrackPtr &atp = mAnimTracks[ i ];

			if( atp->fBlendStrength( ) <= 0.f )
			{
				for( u32 j = i + 1; j < mAnimTracks.fCount( ); ++j )
					mAnimTracks[ j - 1 ] = mAnimTracks[ j ];

				--i;
				++culled;
			}
		}
		if( culled > 0 )
			mAnimTracks.fSetCount( mAnimTracks.fCount( ) - culled );
	}

	b32 tAnimatedSkeleton::fApplyRefFrameDelta( Math::tMat3f& xformInOut, f32 scale ) const
	{
		return mRefFrameDelta.fApplyAsRefFrameDelta( xformInOut, scale );
	}

	f32 tAnimatedSkeleton::fComputeLinearSpeed( const Math::tMat3f& objToWorld, f32 scale ) const
	{
		return objToWorld.fXformVector( scale * mRefFrameDelta.mP ).fLength( );
	}

	tAnimTrackPtr tAnimatedSkeleton::fFirstTrackWithTag( const tStringPtr& tag ) const
	{
		for( u32 i = 0; i < mAnimTracks.fCount( ); ++i )
		{
			if( mAnimTracks[ i ]->fTag( ) == tag )
				return mAnimTracks[ i ];
		}

		return tAnimTrackPtr( );
	}
	
	tAnimTrackPtr tAnimatedSkeleton::fLastTrackWithTag( const tStringPtr& tag ) const
	{
		for( s32 i = mAnimTracks.fCount( )-1; i >= 0; --i )
		{
			if( mAnimTracks[ i ]->fTag( ) == tag )
				return mAnimTracks[ i ];
		}

		return tAnimTrackPtr( );
	}

	b32 tAnimatedSkeleton::fHasTrackWithTag( const tStringPtr& tag ) const
	{
		return fLastTrackWithTag( tag );
	}

	void tAnimatedSkeleton::fRemoveTrack( u32 index )
	{
		mAnimTracks.fEraseOrdered( index );
	}

	void tAnimatedSkeleton::fRemoveTracksWithTag( const tStringPtr& tag )
	{
		for( u32 i = 0; i < mAnimTracks.fCount( ); )
		{
			if( mAnimTracks[ i ]->fTag( ) == tag ) 
				fRemoveTrack( i );
			else ++i;
		}
	}

	void tAnimatedSkeleton::fBlendOutTracksWithTag( const tStringPtr& tag )
	{
		for( u32 i = 0; i < mAnimTracks.fCount( ); ++i )
		{
			if( mAnimTracks[ i ]->fTag( ) == tag ) 
				mAnimTracks[ i ]->fBeginBlendingOut( mAnimTracks[ i ]->fBlendOut( ) );
		}
	}

	void tAnimatedSkeleton::fClearTracks( )
	{
		mAnimTracks.fSetCount( 0 );
		mPostAnimTracks.fSetCount( 0 );
	}

	void tAnimatedSkeleton::fClearBelowTag( const tStringPtr& tag )
	{
		log_warning_unimplemented( 0 );
		//for( s32 i = 0; i < (s32)mAnimTracks.fCount( ); ++i )
		//{
		//	tAnimTrackPtr& ptr = mAnimTracks[ i ];
		//	if( ptr->fTag( ) == tag )
		//	{
		//		mAnimTracks.fEraseOrdered( i );
		//		--i;
		//	}
		//}
	}

	void tAnimatedSkeleton::fSetToIdentity( )
	{
		for( u32 i = 0; i < mMatrixPalette.fCount( ); ++i )
			mMatrixPalette[ i ] = Math::tMat3f::cIdentity;

		// refresh all bone proxies
		for( u32 i = 0; i < mBoneProxies.fCount( ); ++i )
			mBoneProxies[ i ].fResetParent( );
	}

	void tAnimatedSkeleton::fAddEventListener( tLogic& logic )
	{
		mEventListeners.fFindOrAdd( &logic );
	}

	void tAnimatedSkeleton::fRemoveEventListener( tLogic& logic )
	{
		mEventListeners.fFindAndErase( &logic );
	}

	void tAnimatedSkeleton::fClearResponseToEndEvent( )
	{
		for( u32 i = 0; i < mAnimTracks.fCount( ); ++i )
			mAnimTracks[ i ]->fIgnoreEndEvent( true );
		for( u32 i = 0; i < mPostAnimTracks.fCount( ); ++i )
			mPostAnimTracks[ i ]->fIgnoreEndEvent( true );
	}

	void tAnimatedSkeleton::fFireEvents( )
	{
		profile( cProfilePerfSkeletonEventsST );
		for( u32 i = 0; i < mEventListeners.fCount( ); ++i )
		{
			for( u32 j = 0; j < mKeyFrameEvents.fCount( ); ++j )
				mEventListeners[ i ]->fHandleLogicEvent( Logic::tEvent( mKeyFrameEvents[ j ].mLogicEventId, NEW tKeyFrameEventContext( mKeyFrameEvents[ j ] ) ) );
		}
	}

	void tAnimatedSkeleton::fAddBoneProxy( tEntity& parent, const tStringPtr& boneName )
	{
		sigassert( fSkeletonResource( ) );
		const tSkeletonFile* skelFile = fSkeletonResource( )->fCast< tSkeletonFile >( );
		sigassert( skelFile );

		const tBone* findBone = skelFile->fFindBone( boneName );
		if( findBone )
			mBoneProxies.fPushBack( tBoneProxy( parent, *this, findBone->mMasterIndex ) );
	}

	void tAnimatedSkeleton::fRemoveBoneProxy( tEntity& parent, const tStringPtr& boneName )
	{
		if( mBoneProxies.fCount( ) == 0 )
			return; // don't waste time looking for the right one if there are none

		sigassert( fSkeletonResource( ) );
		const tSkeletonFile* skelFile = fSkeletonResource( )->fCast< tSkeletonFile >( );
		sigassert( skelFile );

		const tBone* findBone = skelFile->fFindBone( boneName );
		if( findBone )
		{
			for( u32 i = 0; i < mBoneProxies.fCount( ); ++i )
			{
				if( mBoneProxies[ i ].fIsMatch( parent, findBone->mMasterIndex ) )
				{
					mBoneProxies[ i ].fResetParent( );
					mBoneProxies.fErase( i );
					break;
				}
			}
		}
	}

	void tAnimatedSkeleton::fDeleteBoneProxies( )
	{
		for( u32 i = 0; i < mBoneProxies.fCount( ); ++i )
			mBoneProxies[ i ].fResetParent( );
		mBoneProxies.fSetCount( 0 );
	}

	void tAnimatedSkeleton::fStepBoneProxiesMTInJobs( )
	{
		for( u32 i = 0; i < mBoneProxies.fCount( ); ++i )
			mBoneProxies[ i ].fRecomputeParentRelative( *this );
	}

	void tAnimatedSkeleton::fStepBoneProxiesMTMainThread( )
	{
		profile( cProfilePerfStepBoneProxiesST );

		for( u32 i = 0; i < mBoneProxies.fCount( ); ++i )
			mBoneProxies[ i ].fUpdateParent( );
	}

	u32 tAnimatedSkeleton::fHighestFullyBlendedTrack( ) const
	{
		for( s32 i = mAnimTracks.fCount( ) - 1; i >= 0; --i )
			if( mAnimTracks[ i ]->fBlendStrength( ) >= 1.f && !mAnimTracks[ i ]->fPartial( ) )
				return ( u32 )i;
		return 0u;
	}

	u32 tAnimatedSkeleton::fHighestStackClearingTrack( ) const
	{
		for( s32 i = mAnimTracks.fCount( ) - 1; i >= 0; --i )
			if( mAnimTracks[ i ]->fBlendStrength( ) >= 1.f && !mAnimTracks[ i ]->fPartial( ) && mAnimTracks[ i ]->fLooping( ) )
				return ( u32 )i;
		return 0u;
	}

	//------------------------------------------------------------------------------
	void tAnimatedSkeleton::fPushAnimTrack( tAnimTrackStack & stack, const tAnimTrackPtr & track )
	{
		stack.fPushBack( track );
		track->fOnPushed( *this );
	}

	void tAnimatedSkeleton::fConstructBoneLines( tBoneLine& boneLinesRoot, const Math::tMat3f& objectToWorld ) const
	{
		const tSkeletonFile* skeleton = mSkeleton->fCast< tSkeletonFile >( );
		sigassert( skeleton );

		fConstructBoneLines( skeleton->mRoot, boneLinesRoot, objectToWorld );
	}

	void tAnimatedSkeleton::fRenderDebug( Gfx::tDebugGeometryContainer& debugGeometry, const Math::tMat3f& objectToWorld, const Math::tVec4f& rgbaTint ) const
	{
		tBoneLine boneLines;
		fConstructBoneLines( boneLines, objectToWorld );

		fRenderDebug( debugGeometry, boneLines, rgbaTint );
	}

	void tAnimatedSkeleton::fConstructBoneLines( s32 boneIndex, tBoneLine& boneLinesRoot, const Math::tMat3f& objectToWorld ) const
	{
		const tSkeletonFile* skeleton = mSkeleton->fCast< tSkeletonFile >( );
		sigassert( skeleton );

		const tBone& bone = skeleton->fBone( boneIndex );

		boneLinesRoot.mXform = objectToWorld * mMatrixPalette[ boneIndex ] * bone.mRefPose;
		boneLinesRoot.mName = bone.mName->fGetStringPtr( );
		boneLinesRoot.mChildren.fNewArray( bone.mChildren.fCount( ) );
		for( u32 i = 0 ; i < boneLinesRoot.mChildren.fCount( ); ++i )
			fConstructBoneLines( bone.mChildren[ i ], boneLinesRoot.mChildren[ i ], objectToWorld );
	}

	void tAnimatedSkeleton::fRenderDebug( Gfx::tDebugGeometryContainer& debugGeometry, const tBoneLine& root, const Math::tVec4f& rgbaTint ) const
	{
		debugGeometry.fRenderOnce( root.mXform, 0.25f, rgbaTint.w );

		for( u32 i = 0; i < root.mChildren.fCount( ); ++i )
			debugGeometry.fRenderOnce( fMakePair( root.mXform.fGetTranslation( ), root.mChildren[ i ].mXform.fGetTranslation( ) ), rgbaTint );

		for( u32 i = 0; i < root.mChildren.fCount( ); ++i )
			fRenderDebug( debugGeometry, root.mChildren[ i ], rgbaTint );
	}

}


namespace Sig
{
	void tAnimatedSkeleton::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tAnimatedSkeleton, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			.Func(_SC("ClearBelowTag"),			&tAnimatedSkeleton::fClearBelowTag)
			.Func(_SC("RemoveTracksWithTag"),	&tAnimatedSkeleton::fRemoveTracksWithTag)
			.Func(_SC("HasTrackWithTag"),		&tAnimatedSkeleton::fHasTrackWithTag)
			.Func(_SC("BlendOutTracksWithTag"), &tAnimatedSkeleton::fBlendOutTracksWithTag)
			;

		vm.fNamespace(_SC("Anim")).Bind(_SC("Skeleton"), classDesc);
	}
}

