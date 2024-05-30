#include "BasePch.hpp"
#if defined( platform_metro )
#include "MetroUtil.hpp"
#include "Debug\CrashDump.hpp"
#include "FileSystem.hpp"
#include "StringUtil.hpp"
#include <commctrl.h>
#include <psapi.h>
#include <shellapi.h>

using namespace Platform;
using namespace Windows::Foundation;

namespace Sig { namespace MetroUtil
{
	std::string fErrorCodeToString( Microsoft::Xbox::Foundation::ErrorCode errorCode )
	{
		using namespace Microsoft::Xbox::Foundation;
		switch( errorCode )
		{
		case ErrorCode::AccountManagementRequired:	return "Account management required";
		case ErrorCode::AppReceiptError:			return "Application reciept error";
		case ErrorCode::NoAppReceipt:				return "No application recipet";
		case ErrorCode::NotSignedIn:				return "Not signed in";
		case ErrorCode::NoXboxLIVEAccount:			return "No XBLA Account";
		case ErrorCode::ServerConnectionFailure:	return "Server connection failure";
		case ErrorCode::ServiceDeprecated:			return "Service deprecated";
		case ErrorCode::SpaFileError:				return "Spa file error";
		case ErrorCode::UserBanned:					return "User banned";
		case ErrorCode::WindowsLiveSignInFailure:	return "Windows LIVE sign in failure";
		case ErrorCode::WindowsLiveUserChanged:		return "Windows LIVE user changed";
		case ErrorCode::XboxLIVESignInFailure:		return "XBLA sign in failure";
		default:									return fErrorCodeToString((DWORD)errorCode);
		}
	}

	std::string fErrorCodeToString( DWORD errorCode )
	{ 
		static wchar_t gMessageText[256]; 

		FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			gMessageText,
			array_length( gMessageText ) - 1,
			NULL );

		std::string s = StringUtil::fWStringToMultiByte(gMessageText);
		s = StringUtil::fEatWhiteSpace(s);
		return s;
	}

	std::string fAsyncStatusToString(  Windows::Foundation::AsyncStatus status )
	{
		using namespace Windows::Foundation;

		switch( status )
		{
		case AsyncStatus::Canceled:		return "Canceled";
		case AsyncStatus::Completed:	return "Completed";
		case AsyncStatus::Error:		return "Error";
		case AsyncStatus::Started:		return "Started";
		default:						return "Invalid";
		}
	}

	/*tTemporaryCurrentDirectoryChange::tTemporaryCurrentDirectoryChange( const char* newDir )
	{
		fGetCurrentDirectory( mSavedCurrentDir );
		fSetCurrentDirectory( newDir );
	}

	tTemporaryCurrentDirectoryChange::~tTemporaryCurrentDirectoryChange( )
	{
		fSetCurrentDirectory( mSavedCurrentDir.c_str( ) );
	}*/

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
		log_warning_unimplemented(0);
	}

	void tRecentlyOpenedFileList::fLoadInternal( HKEY hKey )
	{
		log_warning_unimplemented(0);
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
		tFixedArray<wchar_t,512> buf, buf2;
		buf[511] = buf2[511] = '\0';

		fZeroOut( buf );
		fZeroOut( buf2 );
		HMODULE module = 0;
		log_warning_banned_api( 0, "GetModuleHandleExW, GetModuleFileNameW, and GetFullPathNameW" );
		//GetModuleHandleExW( 0, NULL, &module );
		//GetModuleFileNameW( module, buf.fBegin(), buf.fCount( )-1 );
		//GetFullPathNameW( buf.fBegin(), buf2.fCount()-1, buf2.fBegin(), NULL );
		//return StringUtil::fWStringToMultiByte( buf2.fBegin( ) );
		return "";
	}

	void fGetCurrentDirectory( std::string& out )
	{
		tFixedArray<wchar_t,512> buf;
		fZeroOut( buf );

		log_warning_banned_api( 0, "GetCurrentDirectory" );
		//GetCurrentDirectoryW( sizeof(buf)-1, buf.fBegin( ) );

		out = StringUtil::fWStringToMultiByte(buf.fBegin( ));
	}

	void fSetCurrentDirectory( const char* path )
	{
		log_warning_banned_api( 0, "SetCurrentDirectory" );
		//SetCurrentDirectoryW( StringUtil::fMultiByteToWString( path ).c_str() );
	}

	bool fCreateDirectory( const char* path )
	{
		return CreateDirectoryW(StringUtil::fMultiByteToWString(path).c_str(),0)!=0;
	}

	HANDLE fCreateReadOnlyFileHandle( const char* path, b32 directory )
	{
		CREATEFILE2_EXTENDED_PARAMETERS params = {};
		params.dwSize = sizeof(params);
		params.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		params.dwFileFlags = FILE_FLAG_NO_BUFFERING | ( directory ? FILE_FLAG_BACKUP_SEMANTICS : 0 );
		params.dwSecurityQosFlags = 0;
		params.lpSecurityAttributes = NULL;
		params.hTemplateFile = NULL;

		return CreateFile2(
			StringUtil::fMultiByteToWString(path).c_str(),
			0/*GENERIC_READ*/, 
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			OPEN_EXISTING,
			&params );
	}

	std::wstring fMultiByteToWString( const std::string& s )
	{
		return StringUtil::fMultiByteToWString( s );
	}

	std::string fWStringToMultiByte( const std::wstring& ws )
	{
		return StringUtil::fWStringToMultiByte( ws );
	}

#if !defined( target_game ) && !defined( build_release ) && WINAPI_FAMILY_PARTITION(WINAPI_FAMILY_DESKTOP_APP)
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
#endif

	namespace
	{
		static std::string fProcName( DWORD procId )
		{
			TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

			log_warning_unimplemented(0);

			return szProcessName;
		}
	}

	b32 fIsProcessRunning( const char* targetProcName )
	{
		log_warning_unimplemented(0);

		return false;
	}

	b32 fIsVisibleOnConnectedMonitor( u32 x, u32 y )
	{
		POINT winPoint;
		winPoint.x = x;
		winPoint.y = y;
		//return MonitorFromPoint( winPoint, MONITOR_DEFAULTTONULL ) != 0;
		log_warning_unimplemented(0);
		return true;
	}

	RECT fGetDesktopWorkAreaRect( )
	{
		RECT r={0};
		//SystemParametersInfo( SPI_GETWORKAREA, 0, &r, 0 );
		log_warning_unimplemented(0);
		r.right = 1366;
		r.bottom = 720;
		return r;
	}

	u32 fGetDesktopWidth( )
	{
		log_warning_unimplemented(0);
		return 1366;
		//return GetSystemMetrics( SM_CXFULLSCREEN );
	}

	u32 fGetDesktopHeight( )
	{
		log_warning_unimplemented(0);
		return 720;
		//return GetSystemMetrics( SM_CYFULLSCREEN );
	}

	std::string	fGetCaseSensitiveFile( const tFilePathPtr& file )
	{
		WIN32_FIND_DATAW wfd;
		HANDLE hFile = FindFirstFileExW( StringUtil::fMultiByteToWString( file.fCStr( ) ).c_str(), FindExInfoBasic, &wfd, FindExSearchNameMatch, 0, 0 );
		if( hFile == INVALID_HANDLE_VALUE )
			return std::string( );

		std::string ret( StringUtil::fDirectoryFromPath( file.fCStr( ) ) );
		ret += StringUtil::fWStringToMultiByte( wfd.cFileName );

		FindClose( hFile );

		return ret;
	}

	void fExploreToDirectoryAndSelectFile( const char* filePath )
	{
		log_warning_unimplemented(0);
	}

	b32 fIsFileReadOnly( const tFilePathPtr& path )
	{
		if( !FileSystem::fFileExists( path ) )
			return false;
		WIN32_FILE_ATTRIBUTE_DATA data = {};
		b32 success = GetFileAttributesExW( StringUtil::fMultiByteToWString( path.fCStr( ) ).c_str(), GetFileExInfoStandard, &data );
		sigassert(success);
		return ( data.dwFileAttributes & FILE_ATTRIBUTE_READONLY );
	}

	namespace
	{
		void fOnUrlLaunched( IAsyncOperation<bool>^ operation, AsyncStatus status )
		{
			ignore_unused( operation );
			ignore_unused( status );
		}
	}

	void fOpenUrl( const char* url )
	{
		auto launch = Windows::System::Launcher::LaunchUriAsync(ref new Windows::Foundation::Uri(ref new Platform::String(StringUtil::fStringToWString(url).c_str())));
		launch->Completed = Debug::fInferCallbackTypeAndDumpCrashSEH(&fOnUrlLaunched);
	}

	void fExportScriptInterface( tScriptVm& vm )
	{
		const char* namespaces[] = { "MetroUtil", "OsUtil" };

		for( int namespace_i = 0 ; namespace_i < array_length(namespaces) ; ++namespace_i )
		{
			vm.fNamespace(namespaces[namespace_i])
				.Func(_SC("OpenUrl"),&fOpenUrl)
				;
		}
	}
}}
#endif//#if defined( platform_metro )

