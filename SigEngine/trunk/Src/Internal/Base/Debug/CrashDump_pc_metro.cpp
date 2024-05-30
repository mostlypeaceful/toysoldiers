#include "BasePch.hpp"
#if defined( platform_msft ) && !defined( platform_xbox360 )
#include "BannedApiConfig.hpp"
#include "Debug/CrashDump.hpp"
#include "Debug/tDebugger.hpp"
#include <DbgHelp.h>
#include <WERAPI.H>

namespace Sig { namespace Debug
{
	void fSetDumpPath( tFilePathPtr dumppath )
	{
		log_warning_unimplemented( );
	}

	void fAddFileToDumps( tFilePathPtr path )
	{
		WerRegisterFile( StringUtil::fMultiByteToWString(path.fCStr()).c_str(), WerRegFileTypeOther, 0 );
	}

	LONG base_export fDoSehCrash( PEXCEPTION_POINTERS pointers )
	{
		break_if_debugger();

#if defined(platform_metro) && defined(sig_use_banned_apis)
#if WINAPI_FAMILY_PARTITION(WINAPI_FAMILY_DESKTOP_APP) // seperate out since these macros aren't available on earlier SDKs
		Platform::String^ path = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
		std::wstring cpppath( path->Begin(), path->End() );
		cpppath += L"\\minidump.dmp";

		HANDLE file = CreateFile2(
			cpppath.c_str(),
			GENERIC_WRITE,
			0,
			CREATE_ALWAYS,
			NULL );

		MINIDUMP_EXCEPTION_INFORMATION info = { 
			GetCurrentThreadId(),
			pointers,
			FALSE,
		};

		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			file,
			(MINIDUMP_TYPE)(MiniDumpWithThreadInfo | MiniDumpWithCodeSegs), 
			&info,
			NULL,
			NULL );

		CloseHandle(file);
#endif
#endif

#if defined(platform_metro) // Apparently RaiseFailFastException was introduced in Windows 7.
		RaiseFailFastException(
			pointers->ExceptionRecord,
			pointers->ContextRecord,
			0 );
#endif

		return EXCEPTION_EXECUTE_HANDLER;
	}

	b32 base_export fDisableDoSehCrash()
	{
		//return IsDebuggerPresent();
		return false;
	}
}}

#endif // defined( platform_msft ) && !defined( platform_xbox360 )
