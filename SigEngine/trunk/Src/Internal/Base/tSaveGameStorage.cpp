//------------------------------------------------------------------------------
// \file tSaveGameStorage.cpp - 10 Aug 2010
// \author Robert West
// \author Josh Wittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tSaveGameStorage.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tSaveGameStorageDesc
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tSaveGameStorageDesc::tSaveGameStorageDesc( )
		: mDeviceId( 0 )
	{
	}

	//------------------------------------------------------------------------------
	tSaveGameStorageDesc::tSaveGameStorageDesc( 
		u32 deviceId, 
		const tStringPtr& fileName, 
		const tLocalizedString& displayName )
		: mDeviceId( deviceId )
		, mFileName( fileName )
		, mDisplayName( displayName )

	{
	}

	//------------------------------------------------------------------------------
	tSaveGameStorageEnumerator::tSaveGameStorageEnumerator( )
		: mState( cStateIdle )
		, mDeviceId( 0 )
		, mMaxSaveFiles( 0 )
	{
	}

	//------------------------------------------------------------------------------
	const tSaveGameStorageDesc& tSaveGameStorageEnumerator::fSaveFileDesc( u32 index )
	{
		sigassert( index < mSaveFiles.fCount( ) );
		return mSaveFiles[ index ];
	}

	//------------------------------------------------------------------------------
	void tSaveGameStorageEnumerator::fBeginEnumeration( 
		const tUserPtr& user, u32 deviceId, u32 numSaveFiles )
	{
		mUser = user;
		mDeviceId = deviceId;
		mMaxSaveFiles = numSaveFiles;
		mState = cStateEnumerating;

		mThread.fStart( &tSaveGameStorageEnumerator::fThreadMain, "SaveGameStorage_ThreadedMain", this );
	}
}

namespace Sig
{
	//------------------------------------------------------------------------------
	// tSaveGameStorage
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tSaveGameStorage::tSaveGameStorage( tMode mode ) 
		: mMode( mode )
		, mState( cStateIdle )
		, mErrorCode( cNoError )
	{
	}

	//------------------------------------------------------------------------------
	tSaveGameStorage::~tSaveGameStorage( )
	{
		// Wait for our thread to finish
		while( mThread.fRunning( ) )
			fSleep(1);
	}

	//------------------------------------------------------------------------------
	b32 tSaveGameStorage::fIsReadingOrWriting( ) const
	{
		return mState == cStateReadingWriting;
	}

	//------------------------------------------------------------------------------
	b32 tSaveGameStorage::fIsFinished( ) const
	{
		return !mThread.fRunning( ) && ( mState == cStateIdle || mState == cStateError );
	}

	//------------------------------------------------------------------------------
	void tSaveGameStorage::fSetup( const tUserPtr& user, const tSaveGameStorageDesc& desc )
	{
		mUser = user;
		mSaveDesc = desc;

		mThread.fStart( &tSaveGameStorage::fThreadMain, "SaveGameStorage_ThreadedMain", this );	
	}
}

namespace Sig
{
	//------------------------------------------------------------------------------
	// tSaveGameStorageWriter
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tSaveGameStorageWriter::tSaveGameStorageWriter( )
		: tSaveGameStorage( cModeWrite )
	{
	}
	
	//------------------------------------------------------------------------------
	void tSaveGameStorageWriter::fGiveBuffer( tDynamicArray<byte>& buffer ) 
	{
		// Set the new data
		buffer.fDisown( mBuffer );
	}

	//------------------------------------------------------------------------------
	void tSaveGameStorageWriter::fBeginWriting( const tUserPtr& user, const tSaveGameStorageDesc& desc )
	{
		sigassert( !fIsReadingOrWriting( ) );
		mState = cStateReadingWriting;
		fSetup( user, desc );
	}

	//------------------------------------------------------------------------------
	void tSaveGameStorageWriter::fWriteFromBuffer( )
	{
		sigassert( mState == cStateReadingWriting );

		if( !FileSystem::fWriteBufferToFile( mBuffer.fBegin( ), mBuffer.fCount( ), fSaveGameFilePath( ) ) )
		{
			log_warning( "Couldn't save to " << fSaveGameFilePath( ) );
		}
	}
}

namespace Sig
{
	//------------------------------------------------------------------------------
	// tSaveGameStorageReader
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tSaveGameStorageReader::tSaveGameStorageReader( )
		: tSaveGameStorage( cModeRead )
	{
	}

	//------------------------------------------------------------------------------
	void tSaveGameStorageReader::fGetBuffer( tDynamicArray<byte>& buffer ) 
	{
		sigassert( !fIsReadingOrWriting( ) );

		mBuffer.fDisown( buffer );
	}

	//------------------------------------------------------------------------------
	void tSaveGameStorageReader::fBeginReading( 
		const tUserPtr& user, const tSaveGameStorageDesc& desc )
	{
		sigassert( !fIsReadingOrWriting( ) );
		mState = cStateReadingWriting;
		fSetup( user, desc );
	}

	//------------------------------------------------------------------------------
	void tSaveGameStorageReader::fReadToBuffer( )
	{
		sigassert( mState == cStateReadingWriting );
		const tFilePathPtr path = fSaveGameFilePath( );

		tGrowableArray<byte> tempBuffer;
		tempBuffer.fSetCount( FileSystem::fGetFileSize( path ) );
		FileSystem::fReadFileToBuffer( tempBuffer.fBegin( ), tempBuffer.fCount( ), path );

		tempBuffer.fDisown( mBuffer );
	}
}
