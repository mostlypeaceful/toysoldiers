#ifndef __BasePch__
#define __BasePch__

#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <sstream>
#include <stack>
#include <map>
#include <complex>

#ifdef __APPLE__
#	define platform_apple
#	include <TargetConditionals.h>
#	if !defined( TARGET_OS_IPHONE )
#		error Invalid Apple compilation target.
#	endif
#endif//__APPLE__


#	if defined( platform_xbox360 )
#		define platform_msft
#		include "xtl.h"
#		include "d3d9.h"
#		include <malloc.h>
#		undef min
#		undef max
#		undef big
#		undef small
#	endif

#	if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#		define platform_msft
#		define platform_pcdx
#		include "Win32Include.hpp"
#		include <malloc.h>
#       include <fstream>
#	endif

#	if defined( platform_pcdx9 )
#		include <d3d9.h>
#		include <d3dx9.h>
#	endif

#	if defined( platform_ios )
#		include <unistd.h>
#		include <signal.h>
#		include <libkern/OSAtomic.h>
#		include <OpenGLES/ES2/gl.h>
#		include <OpenGLES/ES2/glext.h>
#	endif

#include "BuildConfig.hpp"
#include "Core.hpp"
#include "Memory/MemoryUtil.hpp"
#include "Log.hpp"
#include "Time.hpp"
#include "Rtti.hpp"
#include "RttiReflection.hpp"
#include "tArray.hpp"
#include "tHashTable.hpp"
#include "StringUtil.hpp"
#include "RttiFactory.hpp"

#if defined( platform_pcdx9 ) || defined( platform_pcdx10 ) || defined( platform_pcdx11 )
#if defined( target_game )
#define use_steam
#define use_steam_gameserver
//#define use_steam_matchmaking
#pragma warning( push )
#pragma warning( disable : 4265 )
#include "steam_api.h"
#pragma warning( pop )
#include "steam_gameserver.h"
typedef unsigned long long int tAddr;
#else
typedef unsigned int tAddr;
#endif
#else
typedef unsigned int tAddr;
#endif

namespace Sig
{

	///
	/// \brief Inherit from tUncopyable to ensure no one copies instances of your class.
	class base_export tUncopyable
	{
		declare_null_reflector( );
	protected:
		inline tUncopyable( ) { }
	private:
		tUncopyable(const tUncopyable&) 
			{ sigassert( !"tUncopyable(const tUncopyable&): should never get here!" ); }
		tUncopyable& operator=(const tUncopyable&) 
			{ sigassert( !"tUncopyable& operator=(const tUncopyable&): should never get here!" ); return *this; }
	};

}

#include "tPixProfiler.hpp"
#include "Memory/tHeap.hpp"

#ifdef target_game
	define_global_new_delete( )
#endif

#include "tRefCounterPtr.hpp"
#include "tFilePathPtr.hpp"
#include "tStringPtr.hpp"
#include "Math/Math.hpp"
#include "tLoadInPlaceFileBase.hpp"
#include "tEntity.hpp"
#include "tDevMenu.hpp"

#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#	include "ToolsPaths.hpp"
#endif

#if !defined( platform_ios ) // causes a linker error in debug builds
namespace Sig
{
	template class base_export tArraySleeve<unsigned char>;
	template class base_export tArraySleeve<unsigned short>;

	template class base_export tDynamicArray<unsigned char>;
	template class base_export tDynamicArray<unsigned short>;

	template class base_export tGrowableArray<unsigned char>;
	template class base_export tGrowableArray<unsigned short>;
}
#endif//#if !defined( platform_ios )

#endif//__BasePch__

