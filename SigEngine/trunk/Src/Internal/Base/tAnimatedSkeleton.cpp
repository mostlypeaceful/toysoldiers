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

#include "tKeyFrameAnimation.hpp"
#include "tAnimPackFile.hpp"
#include "tBlendAnimTrack.hpp"

#include "tApplication.hpp"


namespace Sig { namespace Anim
{
	devvar( bool, Perf_BoneProxyIsolated, false );

	namespace
	{
		tAnimPackFile* fGetAnimPack( const tFilePathPtr& path )
		{
			const tResourcePtr res = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<tAnimPackFile>( tAnimPackFile::fAnipkPathToAnib( path ) ) );
			if( res->fLoaded( ) )
				return res->fCast<tAnimPackFile>( );
			else
			{
				log_warning( "AnimPack does not exist: " << path );
				return NULL;
			}
		}
	}

	//------------------------------------------------------------------------------
	// tBoneProxy
	//------------------------------------------------------------------------------
	tBoneProxy::tBoneProxy( tEntity& parent, const Math::tMat3f & bindPose, u32 boneIndex )
		: mBindPose( bindPose )
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

	//------------------------------------------------------------------------------
	// tAnimCommandBuffer
	//------------------------------------------------------------------------------
	void tAnimCommandBuffer::fGeneratePushCmd( const tAnimTrackPtr& animTrack )
	{
		tAnimCommand animCmd;
		animCmd.trackDesc = animTrack->fDesc( );
		animCmd.trackData = animTrack->fData( );

		if( tKeyFrameAnimTrack* keyFrameTrack = animTrack->fDynamicCast< tKeyFrameAnimTrack >( ) )
		{
			animCmd.mId = tAnimCommand::cIdPushKeyFrameTrack;

			// Anim name
			const tKeyFrameAnimation& anim = keyFrameTrack->fAnim( );
			animCmd.animIndex = anim.mPackFile->fIndexOfAnim( anim );

			// Resource name
			tResourcePtr packFileRes = anim.mPackFile->fOwnerResource( );
			animCmd.packFileName = packFileRes->fGetPath( );

			fPushBack( animCmd );
		}
		else if( tBlendAnimTrack* blendTrack = animTrack->fDynamicCast< tBlendAnimTrack >( ) )
		{
			animCmd.mId = tAnimCommand::cIdPushBlendTrack;

			const tBlendAnimTrack::tTrackList& subTracks = blendTrack->fSubTracks( );
			for( u32 i = 0; i < subTracks.fCount( ); ++i )
			{
				animCmd.childCommands.fGeneratePushCmd( subTracks[ i ] );
			}

			fPushBack( animCmd );
		}
		else
		{
			log_warning_nospam( "AnimCommand generation not implemented for " << animTrack->fDebugTypeName( ) );
		}
	}

	void tAnimCommandBuffer::fGenerateUpdateCmd( const tAnimTrackPtr& animTrack )
	{
		tAnimCommand animCmd;
		animCmd.trackData = animTrack->fData( );
		animCmd.mId = tAnimCommand::cIdUpdateTrackData;

		if( tKeyFrameAnimTrack* keyFrameTrack = animTrack->fDynamicCast< tKeyFrameAnimTrack >( ) )
		{
			fPushBack( animCmd );
		}
		else if( tBlendAnimTrack* blendTrack = animTrack->fDynamicCast< tBlendAnimTrack >( ) )
		{
			// Generate commands for sub-tracks if this one is visible
			if( blendTrack->fBlendScale( ) > tAnimTrack::cVisibleThresh )
			{
				const tBlendAnimTrack::tTrackList& subTracks = blendTrack->fSubTracks( );
				for( u32 i = 0; i < subTracks.fCount( ); ++i )
					animCmd.childCommands.fGenerateUpdateCmd( subTracks[ i ] );
			}

			fPushBack( animCmd );
		}
	}

	tAnimTrackPtr tAnimCommandBuffer::fExecutePushCmd( const tAnimCommand& animCmd )
	{
		if( animCmd.mId == tAnimCommand::cIdPushKeyFrameTrack )
		{
			tAnimPackFile* animPack = fGetAnimPack( animCmd.packFileName );
			if( animPack )
			{
				if( animCmd.animIndex < animPack->mAnims.fCount( ) )
				{
					const tKeyFrameAnimation& anim = animPack->mAnims[ animCmd.animIndex ];

					tKeyFrameAnimDesc animDesc( &anim, animCmd.trackDesc );

					tKeyFrameAnimTrack* newTrack = NEW tKeyFrameAnimTrack( animDesc );
					newTrack->fSetData( animCmd.trackData );
					return tAnimTrackPtr( newTrack );
				}
				else
				{
					log_warning( "Anim index is out of range: " << animCmd.animIndex );
				}
			}
		}
		else if( animCmd.mId == tAnimCommand::cIdPushBlendTrack )
		{
			// Create sub-tracks
			tBlendAnimTrack::tTrackList subTracks;
			const tAnimCommandBuffer& childCommands = animCmd.childCommands;
			for( u32 i = 0; i < childCommands.fCount( ); ++i )
			{
				const tAnimCommand& curCmd = childCommands[ i ];

				tAnimTrackPtr newAnimTrack = fExecutePushCmd( curCmd );
				if( newAnimTrack )
					subTracks.fPushBack( newAnimTrack );
			}

			tBlendAnimTrack* newTrack = NEW tBlendAnimTrack( subTracks, animCmd.trackDesc );
			newTrack->fSetData( animCmd.trackData );
			return tAnimTrackPtr( newTrack );
		}
		else
		{
			log_warning( "Unhandled animation command: " << animCmd.mId );
		}

		return tAnimTrackPtr( );
	}

	void tAnimCommandBuffer::fExecuteUpdateCmd( const tAnimCommand& animCmd, const tAnimTrackPtr& animTrack )
	{
		animTrack->fSetData( animCmd.trackData );

		if( tBlendAnimTrack* blendTrack = animTrack->fDynamicCast< tBlendAnimTrack >( ) )
		{
			const tBlendAnimTrack::tTrackList& subTracks = blendTrack->fSubTracks( );
			if( subTracks.fCount( ) == animCmd.childCommands.fCount( ) )
			{
				for( u32 i = 0; i < subTracks.fCount( ); ++i )
					fExecuteUpdateCmd( animCmd.childCommands[ i ], subTracks[ i ] );
			}
		}
	}

	//------------------------------------------------------------------------------
	// tAnimCommand
	//------------------------------------------------------------------------------
	b32 tAnimCommand::operator==( const tAnimCommand& rhs ) const
	{
		return mId == rhs.mId &&
			animIndex == rhs.animIndex &&
			trackDesc == rhs.trackDesc &&
			packFileName == rhs.packFileName &&
			trackData == rhs.trackData &&
			childCommands == rhs.childCommands;
	}

	//------------------------------------------------------------------------------
	// tAnimatedSkeleton
	//------------------------------------------------------------------------------
	b32 tAnimatedSkeleton::fIsReachedEndOfOneShotEvent( const Logic::tEvent& e )
	{
		if( e.fEventId( ) == tKeyFrameAnimTrack::fEventKeyFrameID( ) )
		{
			const Anim::tKeyFrameEventContext* context = e.fContext< Anim::tKeyFrameEventContext >( );
			if( context->mEventTypeCppValue == Logic::AnimationEvent::cEventReachedEndOneShot )
				return true;
		}

		return false;
	}

	tAnimatedSkeleton::tAnimatedSkeleton( const tResourcePtr& skeletonRes, tEntity& owner )
		: mSkeleton( skeletonRes )
		, mRefFrameDelta( Math::tPRSXformf::cZeroXform )
		, mBindOffset( Math::tMat3f::cIdentity )
		, mBindOffsetInverse( Math::tMat3f::cIdentity )
		, mBindScale( Math::tVec3f::cOnesVector )
		, mTimeScale( 1.0f )
		, mOwner( &owner )
		, mEnableAnimCommandBuffer( false )
		, mEvaluationEnabled( true )
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

	void tAnimatedSkeleton::fOnDelete( )
	{
		fClearTracks( );
		fDeleteBoneProxies( );
		fReleaseOwner( );
	}

	void tAnimatedSkeleton::fPushTrack( const tAnimTrackPtr& animTrack )
	{
		fPushAnimTrack(mAnimTracks, animTrack);

		if( mEnableAnimCommandBuffer && mAnimCommandBuffer.fCount( ) < cMaxAnimCommandCount )
		{
			// If we're pushing a non-partial track, clear the existing commands since they won't affect the final animation.
			if( !animTrack->fPartial( ) )
				mAnimCommandBuffer.fSetCount( 0 );

			mAnimCommandBuffer.fGeneratePushCmd( animTrack );

#ifdef sig_logging
			if( mAnimCommandBuffer.fCount( ) >= cMaxAnimCommandCount )
				log_warning( "AnimCommandBuffer on skeleton [" << mSkeleton->fGetPath( ) << "] is full." );
#endif//sig_logging
		}
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
		fStepMT( dt );
		fStepST( dt );
	}

	void tAnimatedSkeleton::fStepMT( f32 dt )
	{
		profile_pix( "tAnimatedSkeleton::fStepMT" );
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

			// scale translation by our bind scale
			mRefFrameDelta.mPosition *= mBindScale;
		}
		else
		{
			mRefFrameDelta = Math::tPRSXformf::cZeroXform;
		}

		if( mEvaluationEnabled )
			fEvaluate( );
	}

	void tAnimatedSkeleton::fStepST( f32 dt )
	{
		profile_pix( "tAnimatedSkeleton::fStepST" );
		if( dt > 0.f && mTimeScale > 0.f )
		{
			fStepBoneProxiesST( );
			fCullDeadTracks( );

			for( u32 i = 0; i < mPostAnimTracks.fCount( ); ++i )
				mPostAnimTracks[ i ]->fStepST( dt, *this );

			fFireEvents( );
		}

#ifdef sig_logging
		if( mAnimTracks.fCount( ) > 20 )
		{
			log_warning( "Track count on skeleton [" << mSkeleton->fGetPath( ) << "] is getting high: " << mAnimTracks.fCount( ) << "; logging stack tags:" );
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

		fStepBoneProxiesMT( );
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

	void tAnimatedSkeleton::fRemovePostTrack( u32 index )
	{
		mPostAnimTracks.fEraseOrdered( index );
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
		log_warning_unimplemented( );
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

	void tAnimatedSkeleton::fFireEvents( )
	{
		profile( cProfilePerfSkeletonEventsST );
		for( u32 i = 0; i < mEventListeners.fCount( ); ++i )
		{
			for( u32 j = 0; j < mKeyFrameEvents.fCount( ); ++j )
				mEventListeners[ i ]->fOwnerEntity( )->fHandleLogicEvent( Logic::tEvent( mKeyFrameEvents[ j ].mLogicEventId, NEW Anim::tKeyFrameEventContext( mKeyFrameEvents[ j ] ) ) );
		}
	}

	void tAnimatedSkeleton::fAddBoneProxy( tEntity& parent, const tStringPtr& boneName, b32 boneRelative )
	{
		sigassert( fSkeletonResource( ) );
		const tSkeletonFile* skelFile = fSkeletonResource( )->fCast< tSkeletonFile >( );
		sigassert( skelFile );

		const tBone* findBone = skelFile->fFindBone( boneName );
		if( findBone )
		{
			Math::tMat3f bindPose;
			if( !boneRelative ) bindPose =  fBindOffsetInverse( ) * parent.fParentRelative( );
			else bindPose = ( findBone->mRefPose * parent.fParentRelative( ) );

			mBoneProxies.fPushBack( tBoneProxy( parent, bindPose, findBone->mMasterIndex ) );
		}
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

	void tAnimatedSkeleton::fEnableAnimCommandBuffer( b32 enable )
	{
		mEnableAnimCommandBuffer = enable;
		if( !enable )
			mAnimCommandBuffer.fDeleteArray( );
	}

	void tAnimatedSkeleton::fGeneratePushCommandsForAllTracks( tAnimCommandBuffer& animCommandBuffer )
	{
		for( u32 i = 0; i < mAnimTracks.fCount( ); ++ i )
			animCommandBuffer.fGeneratePushCmd( mAnimTracks[ i ] );
	}

	void tAnimatedSkeleton::fGenerateUpdateCommands( tAnimCommandBuffer& animCommandBuffer )
	{
		for( u32 i = fHighestNonPartialTrack( ); i < mAnimTracks.fCount( ); ++i )
			animCommandBuffer.fGenerateUpdateCmd( mAnimTracks[ i ] );
	}

	void tAnimatedSkeleton::fExecuteAnimCommands( const tAnimCommandBuffer& animCommandBuffer )
	{
		for( u32 i = 0; i < animCommandBuffer.fCount( ); ++i )
		{
			const tAnimCommand& curCmd = animCommandBuffer[ i ];

			if( curCmd.mId == tAnimCommand::cIdUpdateTrackData )
			{
				// Check if we have a track to apply the update to.
				// NOTE: This assumes that animCommandBuffer contains only update commands
				if( i + mAnimTracks.fCount( ) >= animCommandBuffer.fCount( ) )
				{
					u32 animTrackIdx = i + ( mAnimTracks.fCount( ) - animCommandBuffer.fCount( ) );
					tAnimCommandBuffer::fExecuteUpdateCmd( curCmd, mAnimTracks[ animTrackIdx ] );
				}
			}
			else // Assume it's a push command
			{
				tAnimTrackPtr newAnimTrack = tAnimCommandBuffer::fExecutePushCmd( curCmd );
				if( newAnimTrack )
					fPushTrack( newAnimTrack );
			}
		}
	}

	void tAnimatedSkeleton::fStepBoneProxiesMT( )
	{
		for( u32 i = 0; i < mBoneProxies.fCount( ); ++i )
			mBoneProxies[ i ].fRecomputeParentRelative( *this );
	}

	void tAnimatedSkeleton::fStepBoneProxiesST( )
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

	u32 tAnimatedSkeleton::fHighestNonPartialTrack( ) const
	{
		for( s32 i = mAnimTracks.fCount( ) - 1; i >= 0; --i )
			if( !mAnimTracks[ i ]->fPartial( ) )
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

#ifdef sig_devmenu
	void tAnimatedSkeleton::fAddDebugText( std::stringstream& ss ) const
	{
		for( s32 i = fTrackCount( ) - 1; i >= 0; --i )
		{
			fTrack( i ).fDebugTrackName( ss, 1 );
			fTrack( i ).fDebugTrackData( ss, 1 );
		}

		if( mPostAnimTracks.fCount( ) > 0 )
		{
			ss << "Post Tracks: " << std::endl;
			for( s32 i = mPostAnimTracks.fCount( ) - 1; i >= 0; --i )
			{
				mPostAnimTracks[ i ]->fDebugTrackName( ss, 1 );
				mPostAnimTracks[ i ]->fDebugTrackData( ss, 1 );
			}
		}
	}
#endif

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

} }


namespace Sig { namespace Anim
{
	void tKeyFrameEventContext::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tKeyFrameEventContext, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			.StaticFunc(_SC("Convert"), &tKeyFrameEventContext::fConvert)
			.Var(_SC("Tag"),			&tKeyFrameEventContext::mTag)
			.Var(_SC("Type"),			&tKeyFrameEventContext::mEventTypeCppValue)
			;

		vm.fNamespace(_SC("Anim")).Bind(_SC("KeyFrameEventContext"), classDesc);
	}

	void tAnimatedSkeleton::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tAnimatedSkeleton, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			.Func(_SC("ClearBelowTag"),			&tAnimatedSkeleton::fClearBelowTag)
			.Func(_SC("RemoveTracksWithTag"),	&tAnimatedSkeleton::fRemoveTracksWithTag)
			.Func(_SC("HasTrackWithTag"),		&tAnimatedSkeleton::fHasTrackWithTag)
			.Func(_SC("BlendOutTracksWithTag"), &tAnimatedSkeleton::fBlendOutTracksWithTag)
			.Func(_SC("ClearPostTracks"),		&tAnimatedSkeleton::fClearPostTracks)
			;

		vm.fNamespace(_SC("Anim")).Bind(_SC("Skeleton"), classDesc);

		tKeyFrameEventContext::fExportScriptInterface( vm );
	}
} }

