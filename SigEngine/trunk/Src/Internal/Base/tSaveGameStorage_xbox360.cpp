#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tSaveGameStorage.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	namespace 
	{ 
		static const tStringPtr cRootName( "save" );
	}
}

namespace Sig
{

	Threads::tThreadReturn thread_call tSaveGameStorageEnumerator::fThreadMain( void* threadParam )
	{
		sigassert( threadParam );
		tSaveGameStorageEnumerator* enumerator = static_cast< tSaveGameStorageEnumerator* >( threadParam );

		HANDLE handle;
		DWORD buffer;

		tGrowableArray< XCONTENT_DATA > contentItems( enumerator->fMaxSaveFiles( ) );
		const u32 userIndex = enumerator->fUser( )->fLocalHwIndex( );
		u32 ret = XContentCreateEnumerator( userIndex, enumerator->fDeviceId( ), XCONTENTTYPE_SAVEDGAME, 0, contentItems.fCount( ), &buffer, &handle );

		if( ret != ERROR_SUCCESS )
		{
			CloseHandle( handle );
			log_warning( "tSaveGameStorageEnumerator::fThreadMain: XContentCreateEnumerator failed with error code: " << ret );
			enumerator->fSetFinished( );
			return ret;
		}

		DWORD returnCount;
		if( XEnumerate( handle, contentItems.fBegin( ), sizeof( *contentItems.fBegin( ) ), &returnCount, 0 ) != ERROR_SUCCESS )
		{
			CloseHandle( handle );
			log_warning( "tSaveGameStorageEnumerator::fThreadMain: XEnumerate failed with error code: " << ret );
			enumerator->fSetFinished( );
			return ret;
		}

		for( u32 i = 0; i < returnCount; ++i )
		{
			log_line( 0, "Save file found.  File name( " << contentItems[ i ].szFileName << " Display Name( " << contentItems[ i ].szDisplayName << " )" );
			enumerator->fAddSaveFile( tSaveGameStorageDesc( contentItems[ i ].DeviceID, tStringPtr( contentItems[ i ].szFileName ), tLocalizedString( contentItems[ i ].szDisplayName ) ) );
		}

		CloseHandle( handle );

		enumerator->fSetFinished( );

		thread_return( 0 );
	}
}

namespace Sig
{
	Threads::tThreadReturn thread_call tSaveGameStorage::fThreadMain( void* threadParam )
	{
		sigassert( threadParam );
		tSaveGameStorage* saveGame = static_cast< tSaveGameStorage* >( threadParam );

		XCONTENT_DATA contentData = {0};
		strcpy_s( contentData.szFileName, saveGame->fSaveGameDesc( ).mFileName.fCStr( ) );
		if( saveGame->fSaveGameDesc( ).mDisplayName.fLength( ) )
			wcscpy_s( contentData.szDisplayName, saveGame->fSaveGameDesc( ).mDisplayName.fCStr( ) );
		contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
		contentData.DeviceID = saveGame->fSaveGameDesc( ).mDeviceId;

		const u32 userIndex = saveGame->fUser( )->fLocalHwIndex( );
		const u32 contentFlag = ( saveGame->fMode( ) == tSaveGameStorage::cModeRead ) ? XCONTENTFLAG_OPENEXISTING : XCONTENTFLAG_CREATEALWAYS;
		u32 ret = XContentCreate( userIndex, cRootName.fCStr( ), &contentData, contentFlag, 0, 0, NULL );

		if( ret != ERROR_SUCCESS )
		{
			log_warning( "tSaveGameStorage::fThreadMain: XContentCreate failed with error code " << ret );

			if( ret == ERROR_FILE_CORRUPT )
				saveGame->fSetErrored( tSaveGameStorage::cCorruptData );
			else
				saveGame->fSetErrored( tSaveGameStorage::cMissingData );
			return ret;
		}

		switch( saveGame->fMode( ) )
		{
		case tSaveGameStorage::cModeRead:
			saveGame->fReadToBuffer( );
			break;
		case tSaveGameStorage::cModeWrite:
			saveGame->fWriteFromBuffer( );
			break;
		};

		ret = XContentClose( cRootName.fCStr( ), NULL );
		saveGame->fSetFinished( );

		return ret;
	}
	
	tFilePathPtr tSaveGameStorage::fSaveGameFilePath( ) const
	{
		std::stringstream ss;
		ss << cRootName.fCStr( ) << ":\\" << mSaveDesc.mFileName.fCStr( );
		return tFilePathPtr( ss.str( ).c_str( ) );
	}
}
#endif//#if defined( platform_xbox360 )

