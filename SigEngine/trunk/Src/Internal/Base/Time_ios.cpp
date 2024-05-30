#include "BasePch.hpp"
#if defined( platform_ios )
#include <sys/time.h>

static_assert( sizeof( timeval ) == sizeof( Sig::u64 ) );

namespace Sig { namespace Time
{
	tStamp fGetStamp( )
	{
		tStamp o;
		gettimeofday( ( timeval* )&o, NULL );
		return o;
	}

	f32 fGetElapsedS( tStamp start, tStamp end )
	{
		const timeval t1 = *( timeval* )&start;
		const timeval t2 = *( timeval* )&end;
		double elapsedTime = (t2.tv_sec - t1.tv_sec);
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000000.0;		// us to sec
		return ( f32 )elapsedTime;
	}
	
	f32 fGetElapsedMs( tStamp start, tStamp end )
	{
		const timeval t1 = *( timeval* )&start;
		const timeval t2 = *( timeval* )&end;
		double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;			// us to ms
		return ( f32 )elapsedTime;
	}
	
}}

#endif//#if defined( platform_ios )
