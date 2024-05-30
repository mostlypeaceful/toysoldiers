#include "BasePch.hpp"
#include "tIKAnimTrackBase.hpp"


namespace Sig
{
	namespace
	{

		tAnimTrackPtr fRemoveIKTracks( tAnimatedSkeleton* stack, u32 channel, u32 group )
		{
			tAnimTrackPtr removedItem;

			tAnimatedSkeleton::tAnimTrackStack& pTracks = stack->fPostTracks( );
			for( s32 i = 0; i < (s32)pTracks.fCount( ); ++i )
			{
				tIKAnimTrack* ikTrack = pTracks[ i ]->fDynamicCast< tIKAnimTrack >( );
				if( ikTrack && ikTrack->fChannel( ) == channel && ikTrack->fGroup( ) == group ) 
				{
					removedItem = pTracks[ i ];
					pTracks.fEraseOrdered( i );
					--i;
				}
			}

			return removedItem;
		}

		template< typename tTrack >
		tAnimTrackPtr fMakeTrack( const tIKAnimTrackDesc* desc, tAnimatedSkeleton* stack )
		{
			tAnimTrackPtr removedItem = fRemoveIKTracks( stack, desc->mTargetChannel, desc->mTargetGroup );

			if( removedItem && desc->mReuse )
			{
				//reuse same track.
				return removedItem;
			}
			else
			{
				log_assert( desc->mAnim && desc->mOwner, "Anim and Owner desc properties must be set!" );
				tSkeletonFile* skelFile = stack->fSkeletonResource( )->fCast< tSkeletonFile >( );
				tTrack *newTrack = NEW tTrack( *desc, *skelFile );
				return tAnimTrackPtr( newTrack );
			}
		}


		static void fPushCCDAnim( const tIKAnimTrackDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fPushPostTrack( fMakeTrack< tIKCCDAnimTrack >( desc, stack ) );
		}

		static void fPushLimbAnim( const tIKAnimTrackDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fPushPostTrack( fMakeTrack< tIKLimbAnimTrack >( desc, stack ) );
		}

	}

	void tIKAnimTrackDesc::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class< tIKAnimTrack, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
			classDesc
				.StaticFunc(_SC("Remove"), &fRemoveIKTracks)
				;

			vm.fNamespace(_SC("Anim")).Bind( _SC("IKTrack"), classDesc );
		}
		{
			Sqrat::Class< tIKAnimTrackDesc, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("Anim"), &tIKAnimTrackDesc::mAnim)
				.Var(_SC("Owner"), &tIKAnimTrackDesc::mOwner)
				.Var(_SC("BlendIn"), &tIKAnimTrackDesc::mBlendIn)
				.Var(_SC("BlendOut"), &tIKAnimTrackDesc::mBlendOut)
				.Var(_SC("TimeScale"), &tIKAnimTrackDesc::mTimeScale)
				.Var(_SC("BlendScale"), &tIKAnimTrackDesc::mBlendScale)
				.Var(_SC("MinTime"), &tIKAnimTrackDesc::mMinTime)
				.Var(_SC("MaxTime"), &tIKAnimTrackDesc::mMaxTime)
				.Var(_SC("StartTime"), &tIKAnimTrackDesc::mStartTime)
				.Var(_SC("Flags"), &tIKAnimTrackDesc::mFlags)
				.Var(_SC("Tag"), &tIKAnimTrackDesc::mTag)
				.Var(_SC("TargetCallback"), &tIKAnimTrackDesc::mTargetCallback)
				.Var(_SC("TargetChannel"), &tIKAnimTrackDesc::mTargetChannel)
				.Var(_SC("TargetGroup"), &tIKAnimTrackDesc::mTargetGroup)
				.Var(_SC("IKBlendIn"), &tIKAnimTrackDesc::mIKBlendIn)
				.Var(_SC("IKBlendOut"), &tIKAnimTrackDesc::mIKBlendOut)
				.Var(_SC("IKBlendInCoolDown"), &tIKAnimTrackDesc::mIKBlendInCoolDown)
				.Var(_SC("Reuse"), &tIKAnimTrackDesc::mReuse)
				;
			
			vm.fNamespace(_SC("Anim")).Bind( _SC("IKAnimTrack"), classDesc );
		}
		{
			Sqrat::DerivedClass< tIKCCDAnimTrackDesc, tIKAnimTrackDesc, Sqrat::DefaultAllocator<tIKCCDAnimTrackDesc> > classDesc( vm.fSq( ) );
			classDesc
				.GlobalFunc(_SC("Push"), &fPushCCDAnim)
				;
			vm.fNamespace(_SC("Anim")).Bind( _SC("IKCCDTrack"), classDesc );
		}
		{
			Sqrat::DerivedClass< tIKLimbAnimTrackDesc, tIKAnimTrackDesc, Sqrat::DefaultAllocator<tIKLimbAnimTrackDesc> > classDesc( vm.fSq( ) );
			classDesc
				.GlobalFunc(_SC("Push"), &fPushLimbAnim)
				;
			vm.fNamespace(_SC("Anim")).Bind( _SC("IKLimbTrack"), classDesc );
		}
	}
}
