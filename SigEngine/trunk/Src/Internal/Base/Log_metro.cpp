#include "BasePch.hpp"
#if defined( platform_metro )
#include "StringUtil.hpp"
#include "Debug/CrashDump.hpp"
#include <fstream>
#include <ppltasks.h>

namespace Sig { namespace Log
{
	namespace Detail { namespace LogOnlyOps
	{
		std::ostream& operator<<( std::ostream& os, ::Platform::String^ ws )
		{
			os << ws->Begin();
			return os;
		}

		std::ostream& operator<<( std::ostream& os, const std::wstring& ws )
		{
			os << StringUtil::fWStringToString(ws);
			return os;
		}
		std::ostream& operator<<( std::ostream& os, const wchar_t* ws )
		{
			os << StringUtil::fWStringToString(ws);
			return os;
		}
	}}

	void fFatalError( const char* lastTextUserWillSee )
	{
		fPrintf( 0, "@ERROR@ %s@END@\n", ( lastTextUserWillSee ? lastTextUserWillSee : "" ) );

		if( Log::tAssert::fShouldShowDialog() && lastTextUserWillSee )
		{
			using namespace Concurrency;
			auto dialog = ref new Windows::UI::Popups::MessageDialog( ref new Platform::String( StringUtil::fMultiByteToWString( lastTextUserWillSee ).c_str() ), "Fatal Error" );
			volatile b32 done = false;
			task<Windows::UI::Popups::IUICommand^> op( dialog->ShowAsync( ) );
			op.then( [&]( Windows::UI::Popups::IUICommand^ command )
			{
				done=true;
			});
			while( !done )
			{
				Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
			}
		}

		if( Log::tAssert::fShouldSetBreakpoint() )
		{
			__debugbreak( );
		}

		if( Log::tAssert::fShouldCrashHard() )
		{
			*((int*)0)=0;
		}
		else try
		{
			Windows::UI::Core::CoreWindow^ w = Windows::UI::Core::CoreWindow::GetForCurrentThread();
			if( w != nullptr ) w->Close( );
		}
		catch( ... )
		{
		}

		// N.B. *( int* )0 = 0; and _exit(0) will both lead to the program staying hung instead of terminating
		//      This is because they only hard terminate the EXE's threads, and on metro base is built as a seperate DLL.
		//      They try to unregister the DLL, causing ~Thread etc to be invoked and deadlock.
		//      Yes, *another* bug caused by the stupid metro dynamic linking requirement
		//
		// So instead we're abusing abort as it's also available on ARM.
		// N.B. with some flag settings, abort is merely equivalent to exit(3)
		// I'd use TerminateProcess if it was available on ARM.
		if( !Log::tAssert::fShouldAllowContinue() )
		{
			//_set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );
			_set_abort_behavior( 0, _WRITE_ABORT_MSG );
			abort();
			//exit(0);
			//_exit(0);
			//Windows::ApplicationModel::Core::CoreApplication::Exit();
			// *( int* )0 = 0;
		}
	}

#ifdef sig_logging

	static std::wstring fGetLogFilePath()
	{
		Platform::String^ path = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
		std::wstring cpppath( path->Begin(), path->End() );
		cpppath += L"\\gamelog.txt";
		Debug::fAddFileToDumps( tFilePathPtr( StringUtil::fWStringToMultiByte(cpppath.c_str()) ) );
		return cpppath;
	}

	void fStandardOutputFunction( const char* text, u32 flag )
	{
		static std::ofstream log;
		if( !log.is_open() )
		{
			log.open( fGetLogFilePath().c_str() );
			log.sync_with_stdio( false );
		}
		OutputDebugString( text );
		log << text;
		log.flush();
	}

#endif//sig_logging

}}
#endif//#if defined( platform_metro )
