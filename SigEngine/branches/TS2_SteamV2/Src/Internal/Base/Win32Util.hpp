#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#ifndef __Win32Util__
#define __Win32Util__
#include "Win32Include.hpp"

namespace Sig { namespace Win32Util
{
	base_export const char* fErrorCodeToString( DWORD errorCode );

	///
	/// \brief Use this type to temporarily change the current
	/// win32 directory; an object of this type will automatically
	/// handle resetting the current directory to its previous
	/// value after it goes out of scope.
	class base_export tTemporaryCurrentDirectoryChange : tUncopyable
	{
		std::string mSavedCurrentDir;
	public:
		tTemporaryCurrentDirectoryChange( const char* newDir );
		~tTemporaryCurrentDirectoryChange( );
	};

	///
	/// \brief Base class to facilitate saving multiple values
	class base_export tRegistrySerializer
	{
	public:
		virtual ~tRegistrySerializer( ) { }
		void fSave( );
		b32 fLoad( );
		virtual std::string fRegistryKeyName( ) const = 0;
	protected:
		///
		/// \brief Redefine this in your derived type to properties
		virtual void fSaveInternal( HKEY hKey ) = 0;
		///
		/// \brief Redefine this in your derived type to load properties.
		virtual void fLoadInternal( HKEY hKey ) = 0;
	private:
		HKEY fOpenRegKey( );
	};

	///
	/// \brief Manages a list of "recently opened files" as might
	/// be found in an application's file menu; manages saving/loading
	/// to/from registry.
	class base_export tRecentlyOpenedFileList 
		: public tRegistrySerializer
		, public tFilePathPtrList
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
	base_export bool		fCreateDirectory( const char* path );
	base_export HANDLE		fCreateReadOnlyFileHandle( const char* path, b32 directory );

	base_export std::wstring	fMultiByteToWString( const std::string& s );
	base_export std::string		fWStringToMultiByte( const std::wstring& ws );

	base_export b32			fIsProcessRunning( const char* procName );
	base_export b32			fGetEnvVar( const char* envVarName, std::string& valueOut );
	base_export void		fBroadcastEnvVarsChanged( );

	base_export b32			fIsVisibleOnConnectedMonitor( u32 x, u32 y );
	base_export RECT		fGetDesktopWorkAreaRect( );
	base_export u32			fGetDesktopWidth( );
	base_export u32			fGetDesktopHeight( );

	base_export HKEY		fGetRegistryKeyLocalMachine( const char* name, const char* category=0 );
	base_export HKEY		fGetRegistryKeyCurrentUser( const char* name, const char* category=0 );
	base_export HKEY		fGetSubRegistryKey( HKEY parent, const char* name );
	base_export void		fCloseRegistryKey( HKEY key );

	template<class t>
	b32 fSetRegistryKeyValue( HKEY hKey, const t& value, const char* name=0 )
	{
		std::stringstream ss;
		ss << value;
		return fSetRegistryKeyValue( hKey, ss.str( ), name );
	}

	template<class t>
	b32 fGetRegistryKeyValue( HKEY hKey, t& value, const char* name=0 )
	{
		std::string s;

		if( fGetRegistryKeyValue( hKey, s, name ) )
		{
			std::stringstream ss;
			ss << s;
			ss >> value;
			return true;
		}
		else
			value = t( );

		return false;
	}

	template<>
	base_export b32			fSetRegistryKeyValue<std::string>( HKEY key, const std::string& value, const char* valueName );

	template<>
	base_export b32			fGetRegistryKeyValue<std::string>( HKEY key, std::string& value, const char* valueName );

	base_export	void		fDisplayToolTip( HINSTANCE hinst, HWND hwnd, const char* tip, b32 immediateAreaOnly = false );

	///
	/// \brief
	/// This will return a mixed-case file path. The file name will be case sensitive and the rest
	/// of the file path will be grabbed from the FilePathPtr that is all lowercase.
	base_export std::string	fGetCaseSensitiveFile( const tFilePathPtr& file );

	/// 
	/// \brief
	/// This will open a new Windows Explorer instance and show the selected file.
	base_export void fExploreToDirectoryAndSelectFile( const char* dir );

	///
	/// \brief Find out if the file is read-only or not.
	base_export b32 fIsFileReadOnly( const tFilePathPtr& path );
}}

#endif//__Win32Util__
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
