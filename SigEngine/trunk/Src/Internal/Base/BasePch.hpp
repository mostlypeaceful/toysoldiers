#ifndef __BasePch__
#define __BasePch__

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
#		include "xgraphics.h"
#		include <malloc.h>
#		undef min
#		undef max
#		undef big
#		undef small
#	endif

#	if defined( platform_pcdx9 ) || defined( platform_pcdx10 ) || defined( platform_pcdx11 )
#		define platform_msft
#		define platform_pcdx
#		include "Win32Include.hpp"
#		include <malloc.h>
#	endif

#	if defined( platform_metro ) // NFI if this is sane.
#		define platform_msft
#		undef UNICODE  // Metro configs lose the project option, trying to force unicode.
#		undef _UNICODE // Undefining these in the project config causes insuppressible warning spam, so we do it here instead.
//		This is a bit sketchy, but these are static classes -- and unless we wrap them up (why?) there's no other way to interchange these sanely.
#if 0
#		define CurrentAppForBuild CurrentApp
#else
#		define CurrentApp USE_CurrentAppForBuild_INSTEAD
#		define CurrentAppForBuild CurrentAppSimulator
#endif
#		include "Win32Include.hpp"
#		undef UNICODE  // Metro configs lose the project option, trying to force unicode.
#		undef _UNICODE // Undefining these in the project config causes insuppressible warning spam, so we do it here instead.
#		include <malloc.h>
#	endif

#	if defined( platform_pcdx9 )
#		define platform_msft
#		include <d3d9.h>
#		include <d3dx9.h>
#	endif

#	if defined( platform_pcdx10 )
#		define platform_msft
#		include <d3d10.h>
#		include <d3dx10.h>
#	endif

#	if defined( platform_pcdx11 )
#		define platform_msft
#		include <d3d11.h>
//#		include <d3dx11.h> // avoid if possible, unavail on metro
#		include <dxgi1_2.h>
#	endif

#	if defined( platform_metro )
// N.B. "D3DX9, D3DX10, D3DX11 are not supported for Metro-style applications.":
// http://diaryofagraphicsprogrammer.blogspot.com/2011/09/directx-111-notes.html
#		define platform_msft
#		include <wrl.h>
#		include <d3d11_1.h>
#		include <dxgi1_2.h>
#	endif

#	if defined( platform_ios )
#		include <unistd.h>
#		include <signal.h>
#		include <libkern/OSAtomic.h>
#		include <OpenGLES/ES2/gl.h>
#		include <OpenGLES/ES2/glext.h>
#	endif

#	if defined( platform_android )
#		error "Add NDK and OpenGL headers here."
#	endif

#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <sstream>
#include <stack>
#include <map>

#if defined( platform_metro )
//#	include "banned.h"
#endif

#include "BuildConfig.hpp"
#include "Debug/tErrorContext.hpp"
#include "tPixProfiler.hpp"
#include "Core.hpp"
#include "Log.hpp"
#include "Threads/tInterlocked.hpp" // until we remove e.g. interlocked_inc etc from BuildConfig.
#include "Rtti.hpp"
#include "RttiReflection.hpp"

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

#include "Memory/MemoryUtil.hpp"
#include "tArray.hpp"
#include "tHashTable.hpp"
#include "StringUtil.hpp"
#include "RttiFactory.hpp"

#include "Memory/tHeap.hpp"

#ifdef target_game
	define_global_new_delete( )
#endif

#include "Threads/tThread.hpp"
#include "tRefCounterPtr.hpp"
#include "tFilePathPtr.hpp"
#include "tStringPtr.hpp"
#include "Time.hpp"
#include "Math/Math.hpp"
#include "tDevMenu.hpp"
#include "tDebugWatcher.hpp"
#include "tLoadInPlaceFileBase.hpp"
#include "tEntity.hpp"

#if defined( platform_pcdx )
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

namespace Sig { namespace ApplicationEvent
{
	static const u32 cNullEventFlag			= ( 0 );
	static const u32 cApplicationEventFlag	= ( 1 << 16 ); // bits 0-15 reserved for event IDs
	static const u32 cGameEventFlag			= ( 1 << 17 );
	static const u32 cPhysicalEventFlag		= ( 1 << 18 );
	static const u32 cAnimationEventFlag	= ( 1 << 19 );
}}//Sig::ApplicationEvent

#endif//__BasePch__

