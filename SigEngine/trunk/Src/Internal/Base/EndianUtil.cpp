#include "BasePch.hpp"
#include "EndianUtil.hpp"
#include "tPlatform.hpp"

namespace Sig { namespace EndianUtil
{

	void fSwap8 ( void* p, u32 count )
	{
		// nothing to do for 8-bit values
	}
	void fSwap16( void* p, u32 count )
	{
		byte* pb = ( byte* )p;

		for( u32 i = 0; i < count; ++i )
		{
			fSwap( pb[0], pb[1] );

			pb += sizeof( u16 );
		}
	}
	void fSwap32( void* p, u32 count )
	{
		byte* pb = ( byte* )p;

		for( u32 i = 0; i < count; ++i )
		{
			fSwap( pb[0], pb[3] );
			fSwap( pb[1], pb[2] );

			pb += sizeof( u32 );
		}
	}
	void fSwap64( void* p, u32 count )
	{
		byte* pb = ( byte* )p;

		for( u32 i = 0; i < count; ++i )
		{
			fSwap( pb[0], pb[7] );
			fSwap( pb[1], pb[6] );
			fSwap( pb[2], pb[5] );
			fSwap( pb[3], pb[4] );

			pb += sizeof( u64 );
		}
	}
	void fSwapForTargetPlatform( void* p, u32 size, tPlatformId pid, u32 count )
	{
		if( size>1 && !fPlatformNeedsEndianSwap( cCurrentPlatform, pid ) )
			return;

		switch( size )
		{
		case 1:		fSwap8 ( p, count ); break;
		case 2:		fSwap16( p, count ); break;
		case 4:		fSwap32( p, count ); break;
		case 8:		fSwap64( p, count ); break;
		default:	sigassert(!"invalid size for built-in type"); break;
		}
	}

}}
