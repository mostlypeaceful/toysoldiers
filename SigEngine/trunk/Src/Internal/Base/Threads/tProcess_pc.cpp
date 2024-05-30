#include "BasePch.hpp"
#if defined( platform_pcdx )
#include "tProcess.hpp"

namespace Sig { namespace Threads
{

	b32 tProcess::fSpawnAndForget( const char* procName, const char* procCmdLine, const char* startDir, b32 waitUntilFinished, b32 doNotPrependWhitespace )
	{
		tProcess proc( procName, procCmdLine, startDir, doNotPrependWhitespace );
		if( waitUntilFinished )
			proc.fWaitUntilFinished( );
		return proc.fCreatedSuccessfully( );
	}

	tProcess::tProcess( const char* procName, const char* procCmdLine, const char* startDir, b32 doNoPrependWhiteSpace )
	{
		char cmdLine[1024]={0};
		if( !doNoPrependWhiteSpace )
			strcat( cmdLine, "  " );
		if( procCmdLine )
			strcat( cmdLine, procCmdLine );

		STARTUPINFO si={0};
		si.cb = sizeof(si);

		if( !CreateProcess( 
			procName,
			cmdLine,
			0,
			0,
			FALSE,
			0,
			0,
			startDir,
			&si,
			&mProcInfo ) )
		{
			fZeroOut( mProcInfo );
		}
	}

	tProcess::~tProcess( )
	{
		if( fCreatedSuccessfully( ) )
		{
			CloseHandle( mProcInfo.hProcess );
			CloseHandle( mProcInfo.hThread );
		}
	}

	b32 tProcess::fCreatedSuccessfully( ) const
	{
		return mProcInfo.hProcess != 0 || mProcInfo.hThread != 0;
	}

	b32 tProcess::fIsRunning( ) const
	{
		if( !fCreatedSuccessfully( ) )
			return false;
		DWORD exitCode = 0;
		BOOL result = GetExitCodeProcess( mProcInfo.hProcess, &exitCode );
		return( result && exitCode == STILL_ACTIVE );
	}

	u32 tProcess::fProcessId( ) const
	{
		return mProcInfo.dwProcessId;
	}

	void tProcess::fWaitUntilFinished( )
	{
		while( fIsRunning( ) )
			Sleep( 1 );
	}

}}
#endif // #if defined( platform_pcdx )

