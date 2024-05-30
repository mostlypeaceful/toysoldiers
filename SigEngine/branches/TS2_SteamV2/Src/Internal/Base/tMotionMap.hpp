#ifndef __tMotionMap__
#define __tMotionMap__
#include "tAnimatedSkeleton.hpp"

namespace Sig
{
	class tMotionMap
	{
		tAnimatedSkeletonPtr mStack;
		tLogic *mLogic;
	public:
		tMotionMap( ) : mLogic( NULL ) { }
		virtual ~tMotionMap( ) { }

		void fSetLogic( tLogic* logic ) { mLogic = logic; }
		Sqrat::Object fGetLogicForScript( ) { sigassert( mLogic && "Forgot to call tAnimatable::fSetLogic( ) in your logic constructor." ); return mLogic->fOwnerEntity( )->fScriptLogicObject( ); }
		tAnimatedSkeleton* fAnimationStack( ) const {  log_assert( mStack.fGetRawPtr( ), "No animation stack for: " << (mLogic ? mLogic->fDebugTypeName( ) : "") ); return mStack.fGetRawPtr( ); }
		void fSetAnimationStack( const tAnimatedSkeletonPtr& stack ) { mStack = stack; }
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	///
	/// \brief Wrapper around base motion map type, providing glue between script and code.
	class tScriptMotionMap : public tScriptObjectPtr<tMotionMap>
	{
	public:
		tScriptMotionMap( ) { }
		explicit tScriptMotionMap( const Sqrat::Object& o ) : tScriptObjectPtr<tMotionMap>( o ) { }
		tMotionMap* fMotionMap( ) const { return fCodeObject( ); }
		Sqrat::Function fMapState( const char* stateName );
	};
}

#endif//__tMotionMap__
