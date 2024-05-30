#include "BasePch.hpp"
#include "tAnimatable.hpp"
#include "tSceneRefEntity.hpp"
#include "tApplication.hpp"

namespace Sig { namespace Logic
{

	const Anim::tAnimatedSkeletonPtr& tAnimatable::fCreateAnimatedSkeleton( const tResourcePtr& skeletonResource, const Math::tMat3f& skelBinding, const Math::tMat3f& skelBindingInv, tEntity& owner )
	{
		mAnimatedSkeleton.fReset( NEW_TYPED( Anim::tAnimatedSkeleton )( skeletonResource, owner ) );
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
	void tAnimatable::fOnDelete( )
	{
		if( mAnimatedSkeleton )
		{
			mAnimatedSkeleton->fOnDelete( );
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
			mAnimatedSkeleton->fStepMT( dt );
	}
	void tAnimatable::fMoveST( f32 dt )
	{
		if( mSigAnimMotionMap )
			mSigAnimMotionMap->fStep( dt );

		if( mAnimatedSkeleton )
			mAnimatedSkeleton->fStepST( dt );
	}
#ifdef sig_devmenu
	void tAnimatable::fAddWorldDebugText( std::stringstream& ss ) const
	{
		ss << "MoState: " << mCurrentMotionStateName.fCStr( ) << std::endl;

		if( mAnimatedSkeleton )
			mAnimatedSkeleton->fAddDebugText( ss );
	}
#endif//sig_devmenu
	const Anim::tAnimatedSkeletonPtr& tAnimatable::fAnimatedSkeleton( ) const
	{
#ifdef sig_devmenu
			if( !mAnimatedSkeleton )
			{
				tSceneRefEntity* ref = mLogic && mLogic->fOwnerEntity( ) ? mLogic->fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( ) : 0;
				log_warning( "No skeleton specified for " << ( ref ? ref->fSgResource( )->fGetPath( ).fCStr( ) : "{UNKNOWN LOGIC/ENT}" ) );
				sigassert( mAnimatedSkeleton );
			}
#endif//sig_devmenu

		return mAnimatedSkeleton;
	}
	void tAnimatable::fExecuteMotionState( const char* motionStateName, const Sqrat::Object& motionStateParams )
	{
		Sqrat::Function func = fMotionMap( ).fMapState( motionStateName );
		if_devmenu( fSetCurrentMotionStateName( tStringPtr( motionStateName ) ) );
		func.Execute( motionStateParams );
	}
	void tAnimatable::fSetScriptMotionMapObject( const Sqrat::Object& obj )
	{
		// Log if no skeleton?
		//if( !mAnimatedSkeleton )
		//{
		//	log_warning( "Tried to set a motion map on an object with no skeleton. Set the skeleton path in the editor." );
		//	if( mLogic )
		//	{
		//		log_warning( "   Type : " << mLogic->fDebugTypeName( ) );
		//		if( mLogic->fOwnerEntity( ) )
		//		{
		//			tSceneRefEntity* sg = mLogic->fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( );
		//			if( sg )
		//				log_warning( "   Sigml: " << sg->fSgResource( )->fGetPath( ) );
		//		}
		//	}
		//}

		mMotionMap = Anim::tScriptMotionMap( obj );
		if( mMotionMap.fMotionMap( ) )
		{
			mMotionMap.fMotionMap( )->fSetLogic( mLogic );
			mMotionMap.fMotionMap( )->fSetAnimationStack( mAnimatedSkeleton );
		}
	}

	void tAnimatable::fLoadMotionMap( const tFilePathPtr& motionMapPath, const tFilePathPtr& animapPath )
	{
		tResourcePtr moMap = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<tMotionMapFile>( motionMapPath ) );
		tResourcePtr animap = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<tAniMapFile>( animapPath ) );
		
		log_assert( moMap->fLoaded( ), "Motion map not loaded: " << motionMapPath );
		log_assert( animap->fLoaded( ), "Animap not loaded: " << animapPath );
		sigassert( mAnimatedSkeleton && "Need a skeleton!" );
		
		mSigAnimMotionMap.fReset( NEW Anim::tSigAnimMoMap( *moMap->fCast<tMotionMapFile>( ), *animap->fCast<tAniMapFile>( ), *mAnimatedSkeleton ) );
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
			.Func(_SC("LoadMotionMap"), &tAnimatable::fLoadMotionMap)
			;

		vm.fRootTable( ).Bind(_SC("Animatable"), classDesc);
	}

}}

