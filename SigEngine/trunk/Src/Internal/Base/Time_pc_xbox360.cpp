#include "BasePch.hpp"
#if defined( platform_msft )
#include "Time.hpp"

namespace Sig { namespace Time
{
	namespace
	{
		f32 gSecsCoeff			= 0.f;
		f32 gMilliSecsCoeff		= 0.f;

		define_static_function( fInitTimeCoeffs )
		{
			u64 freq=0;
			QueryPerformanceFrequency( ( LARGE_INTEGER* )&freq );
			if( freq )
			{
				gSecsCoeff		= ( 1.f / freq );
				gMilliSecsCoeff = 1000.f * ( 1.f / freq );
			}
		}
	}

	tStamp fGetStamp( )
	{
		tStamp o;
		QueryPerformanceCounter( ( LARGE_INTEGER* )&o );
		return o;
	}

	f32 fGetElapsedS( tStamp start, tStamp end )
	{
		return ( end - start ) * gSecsCoeff;
	}
	
	f32 fGetElapsedMs( tStamp start, tStamp end )
	{
		return ( end - start ) * gMilliSecsCoeff;
	}
}}

#endif//#if defined( platform_msft )
