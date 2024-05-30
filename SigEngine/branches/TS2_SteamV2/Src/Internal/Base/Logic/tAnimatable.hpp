#ifndef __tAnimatable__
#define __tAnimatable__
#include "tMotionMap.hpp"

namespace Sig { namespace Logic { namespace AnimationEvent
{
#include "AnimationEvents.hpp"
}}}

namespace Sig { namespace Logic
{
	class tAnimatable
	{
	private:
		tScriptMotionMap		mMotionMap;
		tAnimatedSkeletonPtr	mAnimatedSkeleton;
		tLogic					*mLogic;
		if_devmenu( tStringPtr mCurrentMotionStateName );
	public:
		tAnimatable( ) : mLogic( NULL ) { }
		void fSetLogic( tLogic* logic ) { mLogic = logic; }

		tScriptMotionMap& fMotionMap( ) { return mMotionMap; }
		if_devmenu( void fSetCurrentMotionStateName( const tStringPtr& name ) { mCurrentMotionStateName = name; } );
		if_devmenu( void fAddWorldDebugText( std::stringstream& ss ) const );
		b32 fHasSkeleton( ) const { return (mAnimatedSkeleton.fGetRawPtr( ) != NULL); }
		const tAnimatedSkeletonPtr& fAnimatedSkeleton( ) const { sigassert( mAnimatedSkeleton && "No skeleton provided!" ); return mAnimatedSkeleton; }
		b32 fShouldSkeletonPropagate( const tAnimatedSkeleton& skeleton ) const
		{
			// if we already have an animated skeleton, and that skeleton is different from the one passed in, then we should NOT propagate any further
			return ( !mAnimatedSkeleton || mAnimatedSkeleton == &skeleton );
		}
		const tAnimatedSkeletonPtr& fCreateAnimatedSkeleton( const tResourcePtr& skeletonResource, const Math::tMat3f& skelBinding, const Math::tMat3f& skelBindingInv );
		void fListenForAnimEvents( tLogic& logic );
		void fClearResponseToEndEvent( );
		void fOnDelete( );
		void fOnSpawn( ); //Evaluate so we dont spawn in tpose.
		void fAnimateMT( f32 dt );
		void fMoveST( f32 dt );

		void fExecuteMotionState( const char* motionStateName, const Sqrat::Object& motionStateParams );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	private:// script-specific private member functions (exposed only to script, not code)
		Sqrat::Object				fScriptMotionMapObject( ) const { return mMotionMap.fScriptObject( ); }
		void						fSetScriptMotionMapObject( const Sqrat::Object& obj );
	};

}}

#endif//__tAnimatable__
