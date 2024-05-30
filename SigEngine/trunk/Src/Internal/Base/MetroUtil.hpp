#if defined( platform_metro )
#ifndef __MetroUtil__
#define __MetroUtil__
#include "Win32Include.hpp"

namespace Sig { namespace MetroUtil
{
	base_export std::string fErrorCodeToString( Microsoft::Xbox::Foundation::ErrorCode errorCode );
	base_export std::string fErrorCodeToString( DWORD errorCode );
	base_export std::string fAsyncStatusToString( Windows::Foundation::AsyncStatus status );

	///
	/// \brief Use this type to temporarily change the current
	/// win32 directory; an object of this type will automatically
	/// handle resetting the current directory to its previous
	/// value after it goes out of scope.
	/*class base_export tTemporaryCurrentDirectoryChange : tUncopyable
	{
	std::string mSavedCurrentDir;
	public:
	tTemporaryCurrentDirectoryChange( const char* newDir );
	~tTemporaryCurrentDirectoryChange( );
	};*/

	///
	/// \brief Manages a list of "recently opened files" as might
	/// be found in an application's file menu; manages saving/loading
	/// to/from registry.
	class base_export tRecentlyOpenedFileList 
		//: public tRegistrySerializer
		: public tFilePathPtrList
	{
		std::string mRegKeyName;
	public:
		tRecentlyOpenedFileList( const std::string& baseRegKeyName );
		virtual std::string fRegistryKeyName( ) const;
		virtual void fSaveInternal( HKEY hKey );
		virtual void fLoadInternal( HKEY hKey );
		void fAdd( const tFilePathPtr& filePath );
		void fMoveToFront( u32 ithItem );
	private:
		std::string fCreateListItemName( u32 ithItem ) const;
	};


	base_export std::string fGetCurrentApplicationFileName( );
	base_export void		fGetCurrentDirectory( std::string& out );
	base_export void		fSetCurrentDirectory( const char* path );
	base_export HANDLE		fCreateReadOnlyFileHandle( const char* path, b32 directory );
	base_export bool		fCreateDirectory( const char* path );

	base_export std::wstring	fMultiByteToWString( const std::string& s );
	base_export std::string		fWStringToMultiByte( const std::wstring& ws );

#if defined( target_tools ) || !defined( build_release )
	/// \brief For debug use only: Uses banned metro APIs.
	base_export b32			fGetEnvVar( const char* envVarName, std::string& valueOut );
#endif

	//base_export b32			fIsVisibleOnConnectedMonitor( u32 x, u32 y );
	//base_export RECT		fGetDesktopWorkAreaRect( );
	//base_export u32			fGetDesktopWidth( );
	//base_export u32			fGetDesktopHeight( );

	//base_export	void		fDisplayToolTip( HINSTANCE hinst, HWND hwnd, const char* tip, b32 immediateAreaOnly = false );

	///
	/// \brief
	/// This will return a mixed-case file path. The file name will be case sensitive and the rest
	/// of the file path will be grabbed from the FilePathPtr that is all lowercase.
	base_export std::string	fGetCaseSensitiveFile( const tFilePathPtr& file );

	/// 
	/// \brief
	/// This will open a new Windows Explorer instance and show the selected file.
	// base_export void fExploreToDirectoryAndSelectFile( const char* dir );

	///
	/// \brief Find out if the file is read-only or not.
	base_export b32 fIsFileReadOnly( const tFilePathPtr& path );

	base_export void fOpenUrl( const char* url );

	base_export void fExportScriptInterface( tScriptVm& vm );
}}

namespace Sig { namespace OsUtil {
	using namespace ::Sig::MetroUtil;
}}

#endif//__MetroUtil__
#endif//#if defined( platform_metro )
