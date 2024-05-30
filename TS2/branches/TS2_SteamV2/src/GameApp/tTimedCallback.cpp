#include "GameAppPch.hpp"
#include "tTimedCallback.hpp"


namespace Sig
{	
	namespace 
	{ 
	}

	tTimedCallback::tTimedCallback( )
		: mTime( -1.f )
		, mPausable( false )
	{
	}

	tTimedCallback::tTimedCallback( f32 time, const Sqrat::Function& func, b32 pausable )
		: mTime( time )
		, mFunction( func )
		, mPausable( pausable )
	{
	}

	b32 tTimedCallback::fStepST( f32 dt )
	{
		mTime -= dt;
		if( mTime <= 0.f )
		{
			if( !mFunction.IsNull( ) ) 
				mFunction.Execute( );
			return true;
		}

		return false;
	}
}


namespace Sig
{
}

