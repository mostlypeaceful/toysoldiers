#ifndef __EndianUtil__
#define __EndianUtil__

namespace Sig { namespace EndianUtil
{

	base_export void fSwap8 ( void* p, u32 count = 1 );
	base_export void fSwap16( void* p, u32 count = 1 );
	base_export void fSwap32( void* p, u32 count = 1 );
	base_export void fSwap64( void* p, u32 count = 1 );

	///
	/// \brief Only endian-swaps if the endianness of the current platform
	/// and the target platform are different.
	base_export void fSwapForTargetPlatform( void* p, u32 size, tPlatformId pid, u32 count = 1 );

}}

#endif//__EndianUtil__
