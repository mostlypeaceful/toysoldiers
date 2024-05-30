#ifndef __tTimedCallback__
#define __tTimedCallback__
#include "tLogic.hpp"

namespace Sig
{

	class tTimedCallback
	{
	public:
		tTimedCallback( );
		tTimedCallback( f32 time, const Sqrat::Function& func, b32 pausable );

		b32 fPausable( ) const { return mPausable;}

		// returns true if executed
		b32 fStepST( f32 dt );

	private:
		f32 mTime;
		b32 mPausable;
		Sqrat::Function mFunction;
	};

}

#endif//__tTimedCallback__
