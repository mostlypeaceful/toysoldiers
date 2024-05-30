#include "BasePch.hpp"
#if defined( platform_ios )
#include "tSaveGameStorage.hpp"
#include "FileSystem.hpp"

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
}
#endif//#if defined( platform_ios )
