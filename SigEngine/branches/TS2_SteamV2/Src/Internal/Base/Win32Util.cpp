#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "Win32Util.hpp"
#include "FileSystem.hpp"
#include <commctrl.h>
#include <psapi.h>

#include "Threads/tProcess.hpp"
#include <shellapi.h>

namespace Sig { namespace Win32Util
{
	const char* fErrorCodeToString( DWORD errorCode ) 
	{ 
		static char gMessageText[256]; 

		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)gMessageText,
			array_length( gMessageText ) - 1,
			NULL );

		return gMessageText;
	}

	tTemporaryCurrentDirectoryChange::tTemporaryCurrentDirectoryChange( const char* newDir )
	{
		fGetCurrentDirectory( mSavedCurrentDir );
		fSetCurrentDirectory( newDir );
	}

	tTemporaryCurrentDirectoryChange::~tTemporaryCurrentDirectoryChange( )
	{
		fSetCurrentDirectory( mSavedCurrentDir.c_str( ) );
	}

	void tRegistrySerializer::fSave( )
	{
		HKEY hKey = fOpenRegKey( );				
		if( !hKey )
			return;

		fSaveInternal( hKey );

		fCloseRegistryKey( hKey );
	}

	b32 tRegistrySerializer::fLoad( )
	{
		HKEY hKey = fOpenRegKey( );
		if( !hKey )
			return false;

		fLoadInternal( hKey );

		fCloseRegistryKey( hKey );
		return true;
	}

	HKEY tRegistrySerializer::fOpenRegKey( )
	{
		return fGetRegistryKeyCurrentUser( fRegistryKeyName( ).c_str( ) );
	}


	tRecentlyOpenedFileList::tRecentlyOpenedFileList( const std::string& baseRegKeyName )
		: mRegKeyName( baseRegKeyName )
	{
	}

	std::string tRecentlyOpenedFileList::fRegistryKeyName( ) const
	{
		return mRegKeyName;
	}

	void tRecentlyOpenedFileList::fSaveInternal( HKEY hKey )
	{
		const u32 cappedCount = fMin( 20u, fCount( ) );
		fSetRegistryKeyValue( hKey, cappedCount, "count" );
		for( u32 i = 0; i < cappedCount; ++i )
			fSetRegistryKeyValue( hKey, (*this)[ i ].fCStr( ), fCreateListItemName( i ).c_str( ) );
	}

	void tRecentlyOpenedFileList::fLoadInternal( HKEY hKey )
	{
		u32 count = 0;
		fGetRegistryKeyValue( hKey, count, "count" );
		if( count > 0 )
			fSetCount( count );
		for( u32 i = 0; i < fCount( ); ++i )
		{
			std::string s;
			fGetRegistryKeyValue( hKey, s, fCreateListItemName( i ).c_str( ) );
			(*this)[ i ] = tFilePathPtr( s.c_str( ) );
		}
	}

	void tRecentlyOpenedFileList::fAdd( const tFilePathPtr& filePath )
	{
		const tFilePathPtr* recentFile = fFind( filePath );
		if( !recentFile )
		{
			fPushBack( filePath );
			recentFile = &fBack( );
		}

		sigassert( recentFile );
		fMoveToFront( fPtrDiff( recentFile, fBegin( ) ) );
	}

	void tRecentlyOpenedFileList::fMoveToFront( u32 ithItem )
	{
		tFilePathPtr save = (*this)[ ithItem ];
		fEraseOrdered( ithItem );
		fGrowCount( 1 );
		for( s32 i = ( s32 )fCount( ) - 2; i >= 0; --i )
			(*this)[ i + 1 ] = (*this)[ i ];
		fFront( ) = save;
	}

	std::string tRecentlyOpenedFileList::fCreateListItemName( u32 ithItem ) const
	{
		std::stringstream ss;
		ss << "File" << ithItem;
		return ss.str( );
	}


	std::string fGetCurrentApplicationFileName( )
	{
		tFixedArray<char,512> buf;
		fZeroOut( buf );
		GetModuleFileName( GetModuleHandle(0), buf.fBegin( ), buf.fCount( ) );
		return buf.fBegin( );
	}

	void fGetCurrentDirectory( std::string& out )
	{
		tFixedArray<char,512> buf;
		fZeroOut( buf );

		GetCurrentDirectory( sizeof(buf)-1, buf.fBegin( ) );

		out = buf.fBegin( );
	}

	void fSetCurrentDirectory( const char* path )
	{
		SetCurrentDirectory( path );
	}

	bool fCreateDirectory( const char* path )
	{
		return CreateDirectory( path, 0 )!=0;
	}

	HANDLE fCreateReadOnlyFileHandle( const char* path, b32 directory )
	{
		return CreateFile( 
			path, 
			0/*GENERIC_READ*/, 
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | ( directory ? FILE_FLAG_BACKUP_SEMANTICS : 0 ),
			0 );
	}

	std::wstring fMultiByteToWString( const std::string& s )
	{
		return StringUtil::fMultiByteToWString( s );
	}

	std::string fWStringToMultiByte( const std::wstring& ws )
	{
		return StringUtil::fWStringToMultiByte( ws );
	}

	namespace
	{
		static std::string fProcName( DWORD procId )
		{
			TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

			// Get a handle to the process.
			HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
										   PROCESS_VM_READ,
										   FALSE, procId );

			// Get the process name.
			if( NULL != hProcess )
			{
				HMODULE hMod;
				DWORD cbNeeded;

				if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
					 &cbNeeded) )
				{
					GetModuleBaseName( hProcess, hMod, szProcessName, 
									   sizeof(szProcessName)/sizeof(TCHAR) );
				}
			}

			CloseHandle( hProcess );

			return szProcessName;
		}
	}

	b32 fIsProcessRunning( const char* targetProcName )
	{
		DWORD procIds[1024], procIdsBytes = 0;
		if ( !EnumProcesses( procIds, sizeof(procIds), &procIdsBytes ) )
			return false;

		// Calculate how many process identifiers were returned.
		const u32 numProcs = procIdsBytes / sizeof( DWORD );

		// Print the name and process identifier for each process.
		for( u32 i = 0; i < numProcs; ++i )
		{
			if( !procIds[ i ] ) continue;

			const std::string procName = fProcName( procIds[ i ] );

			if( _stricmp( procName.c_str( ), targetProcName ) == 0 )
				return true;
		}

		return false;
	}

	b32 fGetEnvVar( const char* envVarName, std::string& valueOut )
	{
		char buf[256]={0};
		const u32 maxChars = sizeof(buf)-1;
		const DWORD result = GetEnvironmentVariable( envVarName, buf, maxChars );
		const b32 success = ( result > 0 && result <= maxChars );

		if( success )
			valueOut = buf;

		return success;
	}

	void fBroadcastEnvVarsChanged( )
	{
		DWORD result=0;
		SendMessageTimeout( HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, &result );
	}

	b32 fIsVisibleOnConnectedMonitor( u32 x, u32 y )
	{
		POINT winPoint;
		winPoint.x = x;
		winPoint.y = y;
		return MonitorFromPoint( winPoint, MONITOR_DEFAULTTONULL ) != 0;
	}

	RECT fGetDesktopWorkAreaRect( )
	{
		RECT r={0};
		SystemParametersInfo( SPI_GETWORKAREA, 0, &r, 0 );
		return r;
	}

	u32 fGetDesktopWidth( )
	{
		return GetSystemMetrics( SM_CXFULLSCREEN );
	}

	u32 fGetDesktopHeight( )
	{
		return GetSystemMetrics( SM_CYFULLSCREEN );
	}

	HKEY fGetSubRegistryKey( HKEY parent, const char* name )
	{
		HKEY key = 0;

		if( RegOpenKeyEx( parent, name, 0, KEY_ALL_ACCESS, &key ) != ERROR_SUCCESS )
		{
			// failed to open key, try to create it
			if( RegCreateKeyEx( parent, name, 0, 0, 0, KEY_ALL_ACCESS, 0, &key, 0 ) != ERROR_SUCCESS )
				return 0;
		}

		return key;
	}

	HKEY fGetRegistryKeyLocalMachine( const char* name, const char* category )
	{
		HKEY key = 0;
		
		if( category )
		{
			HKEY parent = fGetSubRegistryKey( HKEY_LOCAL_MACHINE, category );

			if( parent )
			{
				key = fGetSubRegistryKey( parent, name );
				fCloseRegistryKey( parent );
			}
		}
		else
		{
			key = fGetSubRegistryKey( HKEY_LOCAL_MACHINE, name );
		}

		return key;
	}

	HKEY fGetRegistryKeyCurrentUser( const char* name, const char* category )
	{
		HKEY key = 0;
		
		if( category )
		{
			key = fGetSubRegistryKey( HKEY_CURRENT_USER, category );
			key = fGetSubRegistryKey( key, name );
		}
		else
		{
			key = fGetSubRegistryKey( HKEY_CURRENT_USER, name );
		}

		return key;
	}

	void fCloseRegistryKey( HKEY key )
	{
		if( key )
			RegCloseKey( key );
	}

	template<>
	b32 fSetRegistryKeyValue<std::string>( HKEY key, const std::string& value, const char* valueName )
	{
		const char* hasExpandableEnvVar = strchr( value.c_str( ), '%' );
		if( hasExpandableEnvVar )
			hasExpandableEnvVar = strchr( hasExpandableEnvVar+1, '%' ); // there need to be at least two
		return RegSetValueEx( key, valueName, 0, hasExpandableEnvVar ? REG_EXPAND_SZ : REG_SZ, ( const BYTE* )value.c_str( ), ( DWORD )value.length( ) + 1 ) == ERROR_SUCCESS; 
	}

	template<>
	b32	fGetRegistryKeyValue<std::string>( HKEY key, std::string& value, const char* valueName )
	{
		value.clear( );

		char buf[512]={0};
		DWORD bufSize = sizeof(buf)-1;
		DWORD typeOut;
		if( RegQueryValueEx( key, valueName, 0, &typeOut, ( BYTE* )buf, &bufSize ) != ERROR_SUCCESS )
		{
			if( bufSize < 1024*1024 ) // if it's a reasonable size, try again with dynamically allocated memory
			{
				value.resize( bufSize+1 );
				if( RegQueryValueEx( key, valueName, 0, &typeOut, ( BYTE* )&value[0], &bufSize ) != ERROR_SUCCESS )
					return false; // still a problem
			}
			else
			{
				return false;
			}
		}
		if( typeOut != REG_SZ && typeOut != REG_EXPAND_SZ && typeOut != REG_MULTI_SZ )
			return false;

		if( value.size( ) == 0 )
			value = buf;
		return true;
	}


	void fDisplayToolTip( HINSTANCE hinst, HWND hwnd, const char* tip, b32 immediateAreaOnly )
	{
		char bigBuf[256]={0};
		strncpy( bigBuf, tip, sizeof( bigBuf ) );
		LPTSTR lptstr = bigBuf;

		TOOLINFO ti;
		unsigned int uid = 0;       // for ti initialization
		RECT rect;                  // for client area coordinates

		/* INITIALIZE COMMON CONTROLS */
		INITCOMMONCONTROLSEX iccex; 
		iccex.dwICC = ICC_WIN95_CLASSES;
		iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCommonControlsEx(&iccex);

		/* CREATE A TOOLTIP WINDOW */
		HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwnd,
			NULL,
			hinst,
			NULL
			);

		SetWindowPos(hwndTT,
			HWND_TOPMOST,
			0,
			0,
			0,
			0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		/* GET COORDINATES OF THE MAIN CLIENT AREA */
		GetClientRect (hwnd, &rect);

		if( immediateAreaOnly )
		{
			POINT cursorPos;
			GetCursorPos( &cursorPos );
			ScreenToClient( hwnd, &cursorPos );
			const s32 centerX = cursorPos.x;
			const s32 centerY = cursorPos.y;
			rect.left = fMax<s32>( centerX - 2, 0 );
			rect.right = fMin<s32>( centerX + 2, rect.right );
			rect.top = fMax<s32>( centerY - 2, 0 );
			rect.bottom = fMin<s32>( centerY + 2, rect.bottom );
		}

		/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = hwnd;
		ti.hinst = hinst;
		ti.uId = uid;
		ti.lpszText = lptstr;
			// ToolTip control will cover the whole window
		ti.rect.left = rect.left;    
		ti.rect.top = rect.top;
		ti.rect.right = rect.right;
		ti.rect.bottom = rect.bottom;

		/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
		SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	}

	std::string	fGetCaseSensitiveFile( const tFilePathPtr& file )
	{
		WIN32_FIND_DATA wfd;
		HANDLE hFile = FindFirstFile( file.fCStr( ), &wfd );
		if( hFile == INVALID_HANDLE_VALUE )
			return std::string( );

		std::string ret( StringUtil::fDirectoryFromPath( file.fCStr( ) ) );
		ret += wfd.cFileName;

		FindClose( hFile );

		return ret;
	}

	void fExploreToDirectoryAndSelectFile( const char* filePath )
	{
		std::string params = "/n, /select, ";
		params += filePath;
		ShellExecute( NULL, "open", "explorer.exe", params.c_str( ), NULL, SW_SHOWDEFAULT );
	}

	b32 fIsFileReadOnly( const tFilePathPtr& path )
	{
		if( !FileSystem::fFileExists( path ) )
			return false;
		const DWORD attributes = GetFileAttributes( path.fCStr( ) );
		return ( attributes & FILE_ATTRIBUTE_READONLY );
	}

}}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )

