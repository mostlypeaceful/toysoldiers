//------------------------------------------------------------------------------
// \file tCloudStorageResourceLoader.cpp - 19 July 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tCloudStorageResourceLoader.hpp"
#include "tLoadInPlaceResourceBuffer.hpp"

namespace Sig
{

	tCloudStorageResourceLoader::tCloudStorageResourceLoader( tResource* res, const iCloudStoragePtr& cloudStorage )
		: tResourceLoader( res )
	{
		mFileDownloader = tAsyncFileDownloader::fCreate( fGetResource( )->fGetPath( ), cloudStorage );
	}

	void tCloudStorageResourceLoader::fInitiate( )
	{
		fSetResourceBuffer( NEW tLoadInPlaceResourceBuffer( ) );
		fSetSelfOnResource( );

		// Start the download
		if( mFileDownloader )
		{
			const u32 resBufferSize = mFileDownloader->fFileSize( );

			tGenericBuffer* resBuffer = fGetResourceBuffer( );
			resBuffer->fAlloc( resBufferSize, fMakeStamp( resBufferSize ) );

			const tFilePathPtr& rootPath = fGetResource( )->fGetProvider( )->fRootPath( );

			mFileDownloader->fStart( resBuffer->fGetBuffer( ), resBufferSize, rootPath );
		}
	}

	tResourceLoader::tLoadResult tCloudStorageResourceLoader::fUpdate( )
	{
		if( !mFileDownloader )
			return cLoadFailure;

		if( fGetCancel( ) )
		{
			fCleanupAfterFailure( );
			return cLoadCancel;
		}

		mFileDownloader->fUpdate( );

		if( !mFileDownloader->fIsComplete( ) )
			return cLoadPending;

		if( mFileDownloader->fSuccess( ) )
		{
			fCleanupAfterSuccess( );
			return cLoadSuccess;
		}
		else
		{
			fCleanupAfterFailure( );
			return cLoadFailure;
		}
	}

	void tCloudStorageResourceLoader::fCleanupAfterSuccess( )
	{
		mFileDownloader.fRelease( );
	}

	void tCloudStorageResourceLoader::fCleanupAfterFailure( )
	{
		if( fGetResourceBuffer( ) )
			fGetResourceBuffer( )->fFree( );
		mFileDownloader.fRelease( );
	}

}
