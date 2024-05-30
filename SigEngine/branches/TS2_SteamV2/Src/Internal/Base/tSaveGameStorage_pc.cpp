#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tSaveGameStorage.hpp"
#include "FileSystem.hpp"
#if defined( use_steam )
#include "isteamremotestorage.h"
#endif

namespace Sig
{
	Threads::tThreadReturn thread_call tSaveGameStorageEnumerator::fThreadMain( void* threadParam )
	{
		sigassert( threadParam );
		tSaveGameStorageEnumerator* enumerator = static_cast< tSaveGameStorageEnumerator* >( threadParam );
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

		switch( saveGame->fMode( ) )
		{
		case tSaveGameStorage::cModeRead:
			saveGame->fReadToBuffer( );
			break;
		case tSaveGameStorage::cModeWrite:
			saveGame->fWriteFromBuffer( );
			break;
		};

		saveGame->fSetFinished( );
		thread_return( 0 );
	}
	
	tFilePathPtr tSaveGameStorage::fSaveGameFilePath( ) const
	{
		return tFilePathPtr( mSaveDesc.mFileName.fCStr( ) );
	}

#if defined( use_steam )
	//------------------------------------------------------------------------------
	void tSaveGameStorageWriter::fWriteFromBuffer( )
	{
		sigassert( mState == cStateReadingWriting );
		log_line( Log::cFlagNone, "Writing " << fSaveGameFilePath( ).fCStr( ) << " to Steam Cloud" );

		if_assert( auto result = ) SteamRemoteStorage( )->FileWrite( fSaveGameFilePath( ).fCStr( ), mBuffer.fBegin( ), mBuffer.fCount( ) );
		sigassert( result && "Failed to write to Steam Cloud" );
	}

	//------------------------------------------------------------------------------
	void tSaveGameStorageReader::fReadToBuffer( )
	{
		sigassert( mState == cStateReadingWriting );
		const tFilePathPtr path = fSaveGameFilePath( );
		log_line( Log::cFlagNone, "Reading " << path.fCStr( ) << " from Steam Cloud" );

		tGrowableArray<byte> tempBuffer;
		tempBuffer.fSetCount( SteamRemoteStorage( )->GetFileSize( path.fCStr( ) ) );
		if_assert( auto bytes = ) SteamRemoteStorage( )->FileRead( path.fCStr( ), tempBuffer.fBegin( ), tempBuffer.fCount( ) );
		sigassert( bytes == tempBuffer.fCount( ) && "Failed to read from Steam Cloud" );

		tempBuffer.fDisown( mBuffer );
	}
#endif
}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
