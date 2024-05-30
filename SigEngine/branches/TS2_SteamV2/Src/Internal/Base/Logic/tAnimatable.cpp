#include "BasePch.hpp"
#include "tAnimatable.hpp"
#include "tSceneRefEntity.hpp"

namespace Sig { namespace Logic
{

	const tAnimatedSkeletonPtr& tAnimatable::fCreateAnimatedSkeleton( const tResourcePtr& skeletonResource, const Math::tMat3f& skelBinding, const Math::tMat3f& skelBindingInv )
	{
		mAnimatedSkeleton.fReset( NEW tAnimatedSkeleton( skeletonResource ) );
		mAnimatedSkeleton->fSetBindOffset( skelBinding, skelBindingInv );
		mAnimatedSkeleton->fSetToIdentity( );

		if( mMotionMap.fMotionMap( ) )
			mMotionMap.fMotionMap( )->fSetAnimationStack( mAnimatedSkeleton );

		return mAnimatedSkeleton;
	}
	void tAnimatable::fListenForAnimEvents( tLogic& logic )
	{
		if( mAnimatedSkeleton )
			mAnimatedSkeleton->fAddEventListener( logic );
	}
	void tAnimatable::fClearResponseToEndEvent( )
	{
		if( mAnimatedSkeleton )
			mAnimatedSkeleton->fClearResponseToEndEvent( );
	}
	void tAnimatable::fOnDelete( )
	{
		if( mAnimatedSkeleton )
		{
			mAnimatedSkeleton->fClearTracks( );
			mAnimatedSkeleton->fDeleteBoneProxies( );
			mAnimatedSkeleton.fRelease( );
		}
	}
	void tAnimatable::fOnSpawn( )
	{
		if( mAnimatedSkeleton )
			mAnimatedSkeleton->fStep( 0.05f ); //can't step them zero, need blend strengths to be non zero even on tracks with blend in times
	}
	void tAnimatable::fAnimateMT( f32 dt )
	{
		if( mAnimatedSkeleton )
			mAnimatedSkeleton->fStepMTInJobs( dt );
	}
	void tAnimatable::fMoveST( f32 dt )
	{
		if( mAnimatedSkeleton )
			mAnimatedSkeleton->fStepMTMainThread( );
	}
#ifdef sig_devmenu
	void tAnimatable::fAddWorldDebugText( std::stringstream& ss ) const
	{
		if( !mAnimatedSkeleton )
			return;

		ss << "MoState: " << mCurrentMotionStateName.fCStr( ) << std::endl;
		for( s32 i = mAnimatedSkeleton->fTrackCount( ) - 1; i >= 0; --i )
		{
			mAnimatedSkeleton->fTrack( i ).fDebugTrackName( ss, 1 );
			mAnimatedSkeleton->fTrack( i ).fDebugTrackData( ss, 1 );
		}

		tAnimatedSkeleton::tAnimTrackStack& postTracks = mAnimatedSkeleton->fPostTracks( );
		if( postTracks.fCount( ) > 0 )
		{
			ss << "Post Tracks: " << std::endl;
			for( s32 i = postTracks.fCount( ) - 1; i >= 0; --i )
			{
				postTracks[ i ]->fDebugTrackName( ss, 1 );
				postTracks[ i ]->fDebugTrackData( ss, 1 );
			}
		}
	}
#endif//sig_devmenu
	void tAnimatable::fExecuteMotionState( const char* motionStateName, const Sqrat::Object& motionStateParams )
	{
		Sqrat::Function func = fMotionMap( ).fMapState( motionStateName );
		if_devmenu( fSetCurrentMotionStateName( tStringPtr( motionStateName ) ) );
		func.Execute( motionStateParams );
	}
	void tAnimatable::fExportScriptInterface( tScriptVm& vm )
	{
#define logic_event_script_export
#	include "AnimationEvents.hpp"
#undef logic_event_script_export

		Sqrat::Class<tAnimatable, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			.Func(_SC("OnSpawn"),		&tAnimatable::fOnSpawn)
			.Prop(_SC("MotionMap"),		&tAnimatable::fScriptMotionMapObject, &tAnimatable::fSetScriptMotionMapObject)
			.Func(_SC("ExecuteMotionState"), &tAnimatable::fExecuteMotionState)
			;

		vm.fRootTable( ).Bind(_SC("Animatable"), classDesc);
	}
	void tAnimatable::fSetScriptMotionMapObject( const Sqrat::Object& obj )
	{
		// Log if no skeleton?
		//if( !mAnimatedSkeleton )
		//{
		//	log_warning( Log::cFlagAnimation, "Tried to set a motion map on an object with no skeleton. Set the skeleton path in the editor." );
		//	if( mLogic )
		//	{
		//		log_warning( Log::cFlagAnimation, "   Type : " << mLogic->fDebugTypeName( ) );
		//		if( mLogic->fOwnerEntity( ) )
		//		{
		//			tSceneRefEntity* sg = mLogic->fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( );
		//			if( sg )
		//				log_warning( Log::cFlagAnimation, "   Sigml: " << sg->fSgResource( )->fGetPath( ) );
		//		}
		//	}
		//}

		mMotionMap = tScriptMotionMap( obj );
		if( mMotionMap.fMotionMap( ) )
		{
			mMotionMap.fMotionMap( )->fSetLogic( mLogic );
			mMotionMap.fMotionMap( )->fSetAnimationStack( mAnimatedSkeleton );
		}
	}
}}

