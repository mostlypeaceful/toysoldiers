#ifndef __tAnimatable__
#define __tAnimatable__
#include "tMotionMap.hpp"

namespace Sig { namespace Logic { namespace AnimationEvent
{
#include "AnimationEvents.hpp"
}}}

namespace Sig { namespace Logic
{
	class base_export tAnimatable
	{
	private:
		Anim::tScriptMotionMap		mMotionMap;
		Anim::tAnimatedSkeletonPtr	mAnimatedSkeleton;
		tLogic						*mLogic;
		if_devmenu( tStringPtr		mCurrentMotionStateName );
		tRefCounterPtr<Anim::tSigAnimMoMap> mSigAnimMotionMap;

	public:
		tAnimatable( ) : mLogic( NULL ) { }
		virtual ~tAnimatable( ) { }
		void fSetLogic( tLogic* logic ) { mLogic = logic; }

		Anim::tScriptMotionMap& fMotionMap( ) { return mMotionMap; }
		b32 fHasSkeleton( ) const { return (mAnimatedSkeleton.fGetRawPtr( ) != NULL); }
		const Anim::tAnimatedSkeletonPtr& fAnimatedSkeleton( ) const;
		b32 fShouldSkeletonPropagate( const Anim::tAnimatedSkeleton& skeleton ) const
		{
			// if we already have an animated skeleton, and that skeleton is different from the one passed in, then we should NOT propagate any further
			return ( !mAnimatedSkeleton || mAnimatedSkeleton == &skeleton );
		}
		const Anim::tAnimatedSkeletonPtr& fCreateAnimatedSkeleton( const tResourcePtr& skeletonResource, const Math::tMat3f& skelBinding, const Math::tMat3f& skelBindingInv, tEntity& owner );
		void fListenForAnimEvents( tLogic& logic );
		void fClearResponseToEndEvent( );
		virtual void fOnDelete( );
		virtual void fOnSpawn( ); //Evaluate so we dont spawn in tpose.
		virtual void fAnimateMT( f32 dt );
		virtual void fMoveST( f32 dt );

		void fExecuteMotionState( const char* motionStateName, const Sqrat::Object& motionStateParams );
		
		void fLoadMotionMap( const tFilePathPtr& motionMapPath, const tFilePathPtr& animapPath );
		Anim::tSigAnimMoMap* fSigAnimMotionMap( ) const { return mSigAnimMotionMap.fGetRawPtr( ); }

	public: //debugging
		if_devmenu( void fSetCurrentMotionStateName( const tStringPtr& name ) { mCurrentMotionStateName = name; } );
		if_devmenu( virtual void fAddWorldDebugText( std::stringstream& ss ) const );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:// script-specific private member functions (exposed only to script, not code)
		Sqrat::Object				fScriptMotionMapObject( ) const { return mMotionMap.fScriptObject( ); }
		void						fSetScriptMotionMapObject( const Sqrat::Object& obj );
	};

}}

#endif//__tAnimatable__
