#ifndef __BuildConfig__
#define __BuildConfig__
#ifndef __BasePch__
#error This file must be included via BasePch.hpp!
#endif//__BasePch__

///
/// \section build-type defines
/*
build_debug
build_internal
build_playtest
build_profile
build_release
*/

///
/// \section platform defines
/*
platform_wii
platform_pcdx9
platform_pcdx10
platform_xbox360
platform_ps3ppu
platform_ps3spu
platform_ios
platform_android
platform_pcdx11
platform_metro
*/

///
/// \section target-type defines
/*
target_game
target_tools
*/

///
/// \section actual build configs
/*
tools_pcdx9_debug
tools_pcdx9_release

game_pcdx9_debug
game_pcdx9_release
game_pcdx9_internal
game_pcdx9_profile
*/


namespace Sig
{
	enum tPlatformId
	{
		// don't change the order of these!
		cPlatformWii=1,
		cPlatformPcDx9,
		cPlatformPcDx10,
		cPlatformXbox360,
		cPlatformPs3Ppu,
		cPlatformPs3Spu,
		cPlatformiOS,
		cPlatformAndroid,
		cPlatformPcDx11,
		cPlatformMetro,

		// special values here
		cPlatformLastPlusOne,
		cPlatformLast=cPlatformLastPlusOne-1,
		cPlatformNone=0,
		cPlatformFirst=1,
	};

	enum tEndianType
	{
		cInvalidEndian	= 0x00,
		cLittleEndian	= 0x01,
		cBigEndian		= 0x02,
	};

	class tPlatformIdIterator
	{
		tPlatformId mPid;
	public:
		inline tPlatformIdIterator( ) : mPid( cPlatformFirst ) { }
		inline tPlatformIdIterator( tPlatformId pid ) : mPid( pid ) { }
		inline operator tPlatformId( ) const { return mPid; }
		inline void fStart( ) { mPid = cPlatformFirst; }
		inline void fNext( ) { mPid = ( tPlatformId )( mPid + 1 ); }
		inline bool fDone( ) { return mPid==cPlatformLastPlusOne; }
	};

	inline unsigned fPlatformIdFlag( tPlatformId pid )
	{
		return 1u<<pid;
	}
}

// wipe these defines clean so we can be sure we're the only ones setting them
#undef _DEBUG
#undef NDEBUG





// BUILD TYPES


// DEBUG
#if defined( build_debug )

#	define	_DEBUG

#	define	sig_logging
#	define	sig_assert
#	define	sig_devmenu

#	ifdef target_game
#		define	sig_debugfloat
#		define	sig_profile
#	endif//target_game

// INTERNAL
#elif defined( build_internal )

#	define	NDEBUG

#	define	sig_logging
#	define	sig_assert
#	ifdef target_game
//#		define	sig_debugfloat
#		define	sig_devmenu
#		define	sig_profile
#	endif//target_game

// PLAY TEST
#elif defined( build_playtest )

#	define	NDEBUG

//#	define	sig_logging
//#	define	sig_assert

#	ifdef target_game
#		define	sig_devmenu
//#		define	sig_profile
#	endif//target_game


// PROFILE
#elif defined( build_profile )

#	define	NDEBUG

#	ifdef target_game
#		define	sig_devmenu
#		define	sig_profile
//#		define	sig_logging
//#		define	sig_assert
#	else
#		define	sig_logging
#		define	sig_assert
#	endif//target_game

// RELEASE
#elif defined( build_release )

#	define	NDEBUG

#	ifndef target_game
#		define	sig_logging
#		define	sig_devmenu
#		define	sig_assert
#	endif//target_game

#else

#	error Unrecognized build-type!

#endif // build_xxx



// PLATFORMS

#	if defined( platform_msft )
#		define sig_use_wwise 1
#	endif
#	if defined( sig_use_wwise )
#		define if_wwise( x ) x
#	else
#		define if_wwise( x )
#	endif

#if defined( platform_msft )

#	define inline		__forceinline
#	define OVERRIDE		override
#	if defined( platform_xbox360 )
#		define dll_export
#		define dll_import
#	else
#		define dll_export	__declspec(dllexport)
#		define dll_import	__declspec(dllimport)
#	endif
#	define interlocked_inc		::Sig::Threads::fAtomicIncrement
#	define interlocked_dec		::Sig::Threads::fAtomicDecrement
#	define interlocked_cmp_ex	::Sig::Threads::fAtomicCompareExchange
#	define interlocked_ex		::Sig::Threads::fAtomicExchange
#	define snprintf _snprintf
#	define sigmalloca( type, amount ) ((type*)_malloca(sizeof(type)*amount))
#	define sigfreea( pointer ) _freea( pointer )
#	define snwprintf _snwprintf

#	pragma warning( default	: 4265 ) // Example: warning C4265: class has virtual functions, but destructor is not virtual
#	pragma warning( error	: 4002 ) // Example: warning C4002: too many actual parameters for macro 'log_line'
#	pragma warning( error	: 4003 ) // Example: warning C4003: not enough actual parameters for macro 'log_line'

#	if defined( platform_xbox360 )
#		pragma warning(disable : 4826)
#	else
#		pragma warning(disable : 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#		pragma warning(disable : 4275)
#	endif

	namespace Sig
	{
#if defined( platform_pcdx9 )
		static const tPlatformId cCurrentPlatform = cPlatformPcDx9;
#elif defined( platform_pcdx10 )
		static const tPlatformId cCurrentPlatform = cPlatformPcDx10;
#elif defined( platform_pcdx11 )
		static const tPlatformId cCurrentPlatform = cPlatformPcDx11;
#elif defined( platform_xbox360 )
		static const tPlatformId cCurrentPlatform = cPlatformXbox360;
#elif defined( platform_metro )
		static const tPlatformId cCurrentPlatform = cPlatformMetro;
#else
#	error "Unknown microsoft platform"
#endif

		typedef unsigned char			byte;
		typedef unsigned char			u8;
		typedef signed char				s8;

		typedef unsigned short			u16;
		typedef signed short			s16;

		typedef unsigned int			u32;
		typedef signed int				s32;

		typedef unsigned long long		u64;
		typedef signed long long		s64;

		typedef float					f32;
		typedef double					f64;

		typedef unsigned char			b8;
		typedef unsigned short			b16;
		typedef unsigned int			b32;
	}

#elif defined( platform_ios )

//#	define inline		__forceinline
#	define OVERRIDE
#	define dll_export
#	define dll_import
#	define interlocked_inc		::Sig::Threads::fAtomicIncrement
#	define interlocked_dec		::Sig::Threads::fAtomicDecrement
#	define interlocked_cmp_ex	::Sig::Threads::fAtomicCompareExchange
#	define interlocked_ex		::Sig::Threads::fAtomicExchange
#	define break_if_debugger( )
#	define sigmalloca( type, amount ) alloca( sizeof(type)*amount )
#	define sigfreea( pointer ) ((void)pointer)

namespace Sig
{
	static const tPlatformId cCurrentPlatform = cPlatformiOS;
	
	typedef uint8_t					byte;
	typedef uint8_t					u8;
	typedef int8_t					s8;
	
	typedef uint16_t				u16;
	typedef int16_t					s16;
	
	typedef uint32_t				u32;
	typedef int32_t					s32;
	
	typedef uint64_t				u64;
	typedef int64_t					s64;
	
	typedef float					f32;
	typedef double					f64;
	
	typedef uint8_t					b8;
	typedef uint16_t				b16;
	typedef uint32_t				b32;
}

#else

#	error Unrecognized platform!

#endif//platform_xxx


#if defined( target_game )

#	if defined( build_internal ) || defined( build_debug )
#		define sig_resource_logging 1
#	else
#		define sig_resource_logging 0
#	endif

#	if defined( platform_metro ) // Current metro tools don't let us build static metro .libs, so we end up with Base.dll :<
#		if defined( base_lib )
#			define base_export dll_export
#		else
#			define base_export dll_import
#			define game_inherit_base dll_export
#		endif
#	else
#		define base_export
#		define game_inherit_base
#	endif

#elif defined( target_tools )

#	define sig_resource_logging 0

#	if defined( platform_pcdx9 )
#		define _BOOL
#		define REQUIRE_IOSTREAM
#	endif

#if defined( base_lib )
#		define base_export dll_export
#else
#		define base_export dll_import
#endif

#if defined( tools_lib )
#		define tools_export dll_export
#else
#		define tools_export dll_import
#endif

#if defined( toolsgui_lib )
#		define toolsgui_export dll_export
#else
#		define toolsgui_export dll_import
#endif

#if defined( toolsxdk_lib )
#		define toolsxdk_export dll_export
#else
#		define toolsxdk_export dll_import
#endif

#if defined( tools_siged )
#		define siged_export dll_export
#else
#		define siged_export dll_import
#endif

#else

#	error Unrecognized target-type!

#endif

#endif//__BuildConfig__
