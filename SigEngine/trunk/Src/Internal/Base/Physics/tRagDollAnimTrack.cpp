#include "BasePch.hpp"
#include "tRagDollAnimTrack.hpp"

using namespace Sig::Math;

namespace Sig { namespace Anim
{

	devvar( f32, Physics_Ragdoll_AnimVelTime, 0.12f );

	tRagDollAnimTrack::tRagDollAnimTrack( const tRagDollAnimTrackDesc& desc, tAnimatedSkeleton& skeleton )
		: tAnimTrack( desc )
		, mDoll( desc.mDoll )
		, mOwner( desc.mEntity )
		, mVelocitiesSet( false )
		, mSnapShotting( false )
		, mVelocityTimer( Physics_Ragdoll_AnimVelTime )
	{
		tSkeletonFile* skelFile = skeleton.fSkeletonResource( )->fCast< tSkeletonFile >( );

		mPallet.fResize( skelFile->mMasterBoneList.fCount( ) );
		mOutput.fResize( mDoll->fData( ).mBodies.fCount( ) );

		sigassert( mOwner );
		mDoll->fSetLocation( mOwner->fObjectToWorld( ), skeleton );

		if( desc.mLimits )
			mDoll->fSetConstraintLimits( desc.mLimits );
	}

	tRagDollAnimTrack::~tRagDollAnimTrack( )
	{
		mDoll->fSetApplyExpectedLocation( false );
	}

	void tRagDollAnimTrack::fStepST( f32 dt, tAnimatedSkeleton& animSkel ) 
	{ 
		if( mSnapShotting )
		{
			mDoll->fEndComputingDelta( dt );
			mSnapShotting = false;
			mVelocitiesSet = true;
		}
		else if( !mVelocitiesSet )
		{
			mVelocityTimer -= dt;
			if( mVelocityTimer <= 0.f )
			{
				mSnapShotting = true;
				mDoll->fSetLocation( mOwner->fObjectToWorld( ), animSkel );
				mDoll->fBeginComputingDelta( );
			}
		}
		else
		{
			mDoll->fApplyLaunch( dt );
		}
	}

	void tRagDollAnimTrack::fPostAnimEvaluate( tAnimatedSkeleton& animSkel )
	{
		if( mVelocitiesSet )
		{
			// only inform our logci to apply the expected location once we have blended the results into the rig.
			mDoll->fSetApplyExpectedLocation( true );
			fComputeResult( );
			fBlendResult( animSkel );
		}
	}

	void tRagDollAnimTrack::fComputeResult( )
	{
		Math::tMat3f inverseParentToWorld = mDoll->fExpectedEntityXform( ).fInverse( );

		for( u32 i = 0; i < mDoll->fData( ).mBodies.fCount( ); ++i )
		{
			const Physics::tRagDollBody& body = mDoll->fData( ).mBodies[ i ];

			tMat3f objectSpaceXform = inverseParentToWorld * body.mBody->fTransform( ) * body.mBodyToBone;
			mOutput[ i ] = tPRSXformf( objectSpaceXform );
		}
	}

	namespace
	{
		struct tRagDollBodyIterator
		{
			u32 mNextID;
			Physics::tRagDoll& mDoll;

			tRagDollBodyIterator( Physics::tRagDoll& doll )
				: mDoll( doll )
				, mNextID( 0 )
			{ }

			u32 fBody( u32 masterBoneIndex )
			{
				if( mNextID < mDoll.fData( ).mBodies.fCount( ) 
					&& masterBoneIndex == mDoll.fData( ).mBodies[ mNextID ].mMasterBoneIndex )
					return mNextID++;
				else
					return ~0;
			}

			b32 fOutOfBodies( ) const 
			{
				return mNextID >= mDoll.fData( ).mBodies.fCount( );
			}

			void fReset( )
			{
				mNextID = 0;
			}
		};
	}

	void tRagDollAnimTrack::fBlendResult( tAnimatedSkeleton& animSkel )
	{
		tAnimatedSkeleton::tMatrixPalette& pallette = animSkel.fMatrixPalette( );
		tSkeletonFile* skelFile = animSkel.fSkeletonResource( )->fCast< tSkeletonFile >( );

		// apply real animation stack to ragdoll driver:
		tRagDollBodyIterator ragBodys( *mDoll );

		for( u32 i = 0; i < skelFile->mMasterBoneList.fCount( ); ++i )
		{
			const tBone& bone = skelFile->mMasterBoneList[ i ];

			// see if this joint is affected by a bone
			u32 bodyIndex = ragBodys.fBody( bone.mMasterIndex );
			if( bodyIndex != ~0 )
			{
				const Physics::tRagDollBody& body = mDoll->fData( ).mBodies[ bodyIndex ];
				if( body.mConstraint )
				{
					// wow this is convoluted. :(
					const Physics::tRagDollBody& parentBody = mDoll->fData( ).mBodies[ body.mParentBody ];
					const tBone& parentBone = skelFile->mMasterBoneList[ parentBody.mMasterBoneIndex ];

					tQuatf objSpace( pallette[ bone.mMasterIndex ] );
					tQuatf parentXform( pallette[ parentBone.mMasterIndex ] );
					tQuatf parentAnchorXform = parentXform * tQuatf( parentBody.mBoneToBody ) * tQuatf( body.mConstraint->fAnchorPointA( ) );
					tQuatf delta = parentAnchorXform.fInverse( ) * objSpace;

					body.mConstraint->fSetDesiredAnimOrient( delta );
				}

				if( ragBodys.fOutOfBodies( ) )
					break;
			}
		}

		// convert effected bones to parent relative
		for( s32 i = skelFile->mMasterBoneList.fCount( )-1; i >= 0; --i )
		{
			const tBone& bone = skelFile->mMasterBoneList[ i ];

			if( bone.mMasterIndex != 0 )
				mPallet[ i ] = Math::tPRSXformf( pallette[ bone.mParent ].fInverse( ) * pallette[ bone.mMasterIndex ] );
			else
				mPallet[ i ] = Math::tPRSXformf( pallette[ bone.mMasterIndex ] );
		}

		// transform parent relative back into object space, incorperating ragdoll data
		ragBodys.fReset( );

		for( u32 i = 0; i < skelFile->mMasterBoneList.fCount( ); ++i )
		{
			tMat3f parentRelative = Math::tMat3f( mPallet[ i ] );
			const tBone& bone = skelFile->mMasterBoneList[ i ];

			// see if this joint is affected by a bone
			u32 bodyIndex = ragBodys.fBody( bone.mMasterIndex );
			if( bodyIndex != ~0 )
			{
				// it is. super hack, just set it in object space!
				//  alternatively compute rotation delta in bind space, ie ignore translation
				pallette[ bone.mMasterIndex ] = tMat3f( mOutput[ bodyIndex ] );
			}
			else
			{

				// this bone is not driven by ragdoll, animate as normal, in parent relative space
				if( bone.mMasterIndex != 0 )
					pallette[ bone.mMasterIndex ] = pallette[ bone.mParent ] * parentRelative;
				else
					pallette[ bone.mMasterIndex ] = parentRelative;
			}
		}
	}

	void tRagDollAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign ) 
	{ /* do nothing */ }

	void tRagDollAnimTrack::fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend ) 
	{ /*do nothing*/ }







	namespace
	{

		static void fPushRagDollAnim( const tRagDollAnimTrackDesc* desc, tAnimatedSkeleton* stack )
		{
			sigassert( desc->mEntity && "Entity desc property must be set!" );
			sigassert( desc->mDoll && "Doll desc property must be set!" );
			stack->fRemovePostTracksOfType<tRagDollAnimTrack>( );

			tRagDollAnimTrack *newTrack = NEW tRagDollAnimTrack( *desc, *stack );
			stack->fPushPostTrack( tAnimTrackPtr( newTrack ) );
		}

	}

	void tRagDollAnimTrackDesc::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass< tRagDollAnimTrackDesc, tAnimTrackDesc, Sqrat::DefaultAllocator<tRagDollAnimTrackDesc> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("Doll"),				&tRagDollAnimTrackDesc::mDoll)
				.Var(_SC("Entity"),				&tRagDollAnimTrackDesc::mEntity)
				.Var(_SC("Limits"),				&tRagDollAnimTrackDesc::mLimits)
				.GlobalFunc(_SC("Push"),		&fPushRagDollAnim)
				;
			
			vm.fNamespace(_SC("Anim")).Bind( _SC("RagDollAnimTrack"), classDesc );
		}
	}
} }
