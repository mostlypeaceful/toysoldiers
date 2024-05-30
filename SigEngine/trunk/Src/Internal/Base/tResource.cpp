#include "BasePch.hpp"
#include "tResource.hpp"
#include "tResourceDepot.hpp"
#include "FileSystem.hpp"
#include "tFilePackage.hpp"
#include "tFilePackageFile.hpp"
#include "tStandardResourceLoader.hpp"
#include "tDirectResourceLoader.hpp"
#include "tApplication.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	void tResource::fUnloadAndRemoveFromDepot( )
	{
		sigassert( mOwner );

		// Unload
		if( fHasLoadCaller( ) )
		{
			fInitiateUnload( );

			// hack. wait till unload is finished.
			fBlockUntilLoaded( );
		}

		// Remove From depot
		mOwner->fRemoveLoadingResource( this );
		mOwner->fRemoveResource( this );
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fConvertPathAddB( const tFilePathPtr& path )
	{
		if( path.fLength( ) > 0 && path.fCStr( )[ path.fLength( ) - 1 ] != 'b' )
		{
			char buf[312]={0};
			sigassert( array_length( buf ) > path.fLength( ) );
			fMemCpy( buf, path.fCStr( ), path.fLength( ) );
			buf[ path.fLength( ) ] = 'b';
			return tFilePathPtr( buf );
		}

		return path;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fConvertPathAddBB( const tFilePathPtr& path )
	{
		if( path.fLength( ) > 1 && path.fCStr( )[ path.fLength( ) - 1 ] == 'b' && path.fCStr( )[ path.fLength( ) - 2 ] != 'b' )
		{
			char buf[312]={0};
			sigassert( array_length( buf ) > path.fLength( ) );
			fMemCpy( buf, path.fCStr( ), path.fLength( ) );
			buf[ path.fLength( ) ] = 'b';
			return tFilePathPtr( buf );
		}

		return path;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fConvertPathML2B( const tFilePathPtr& path )
	{
		if( path.fLength( ) > 2 && path.fCStr( )[ path.fLength( ) - 1 ] != 'b' )
		{
			char buf[312]={0};
			sigassert( array_length( buf ) > path.fLength( ) );
			fMemCpy( buf, path.fCStr( ), path.fLength( ) );
			buf[ path.fLength( ) - 2 ] = 'b';
			buf[ path.fLength( ) - 1 ] = '\0';
			return tFilePathPtr( buf );
		}

		return path;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fConvertPathPK2B( const tFilePathPtr& path )
	{
		if( path.fLength( ) > 2 && path.fCStr( )[ path.fLength( ) - 1 ] != 'b' )
		{
			char buf[312]={0};
			sigassert( array_length( buf ) > path.fLength( ) );
			fMemCpy( buf, path.fCStr( ), path.fLength( ) );
			buf[ path.fLength( ) - 2 ] = 'b';
			buf[ path.fLength( ) - 1 ] = '\0';
			return tFilePathPtr( buf );
		}

		return path;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fConvertPathSubB( const tFilePathPtr& path )
	{
		if( path.fLength( ) > 0 && path.fCStr( )[ path.fLength( ) - 1 ] == 'b' )
		{
			char buf[312]={0};
			sigassert( array_length( buf ) > path.fLength( ) );
			fMemCpy( buf, path.fCStr( ), path.fLength( )-1 );
			return tFilePathPtr( buf );
		}

		return path;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fConvertPathSubBB( const tFilePathPtr& path )
	{
		if( path.fLength( ) > 1 && path.fCStr( )[ path.fLength( ) - 1 ] == 'b' && path.fCStr( )[ path.fLength( ) - 2 ] == 'b' )
		{
			char buf[312]={0};
			sigassert( array_length( buf ) > path.fLength( ) );
			fMemCpy( buf, path.fCStr( ), path.fLength( )-1 );
			return tFilePathPtr( buf );
		}

		return path;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fConvertPathB2ML( const tFilePathPtr& path )
	{
		if( path.fLength( ) > 2 && path.fCStr( )[ path.fLength( ) - 1 ] == 'b' )
		{
			char buf[312]={0};
			sigassert( array_length( buf ) > path.fLength( ) );
			fMemCpy( buf, path.fCStr( ), path.fLength( )-1 );
			buf[ path.fLength( ) - 1 ] = 'm';
			buf[ path.fLength( ) - 0 ] = 'l';
			return tFilePathPtr( buf );
		}

		return path;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fConvertPathB2PK( const tFilePathPtr& path )
	{
		if( path.fLength( ) > 2 && path.fCStr( )[ path.fLength( ) - 1 ] == 'b' )
		{
			char buf[312]={0};
			sigassert( array_length( buf ) > path.fLength( ) );
			fMemCpy( buf, path.fCStr( ), path.fLength( )-1 );
			buf[ path.fLength( ) - 1 ] = 'p';
			buf[ path.fLength( ) - 0 ] = 'k';
			return tFilePathPtr( buf );
		}

		return path;
	}

	//------------------------------------------------------------------------------
	tResource::tResource( const tResourceId& rid, tResourceProvider* provider )
		: tResourceId( rid )
		, mOwner( NULL )
		, mProvider( provider )
		, mDisableLoadCompleteEvent( false )
		, mLockLoadCompleteEvent( false )
		, mOnLoadComplete( false )
		, mLoader( 0 )
		, mFileTimeStamp( 0 )
	{ 
	}

	//------------------------------------------------------------------------------
	void tResource::fSetOwner( tResourceDepot* owner )
	{
		mOwner = owner;
	}

	//------------------------------------------------------------------------------
	tResource::~tResource( )
	{
		if( mOwner )
			fUnloadAndRemoveFromDepot( );

		sigassert( !mLoader );
		sigassert( !mBuffer );
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tResource::fAbsolutePhysicalPath( ) const
	{
		tFilePathPtr o;

		if( mProvider && mProvider->fRootPath( ).fExists( ) )
			o = tFilePathPtr::fConstructPath( mProvider->fRootPath( ), fGetPath( ) );
		else
			o = fGetPath( );

		return o;
	}

	//------------------------------------------------------------------------------
	void tResource::fCallWhenLoaded( tOnLoadComplete::tObserver& cb )
	{
		mOnLoadComplete.fAddObserver( &cb );

		if( !mDisableLoadCompleteEvent && !mLockLoadCompleteEvent && ( fLoaded( ) || !fLoading( ) ) )
			cb( *this, fLoaded( ) );
	}

	//------------------------------------------------------------------------------
	void tResource::fRemoveLoadCallback( tOnLoadComplete::tObserver& cb )
	{
		mOnLoadComplete.fRemoveObserver( &cb );
	}

	//------------------------------------------------------------------------------
	void tResource::fLoadDefault( const tLoadCallerId& lcid )
	{
		sigassert( mProvider );
		fLoad( lcid, mProvider->fCreateResourceLoader( tResourcePtr( this ) ) );
	}

	//------------------------------------------------------------------------------
	void tResource::fLoad( const tLoadCallerId& lcid, const tResourceLoaderPtr& loader )
	{
		if( fHasLoadCaller( ) )
		{
			delete loader;
			return;
		}

		fAddLoadCallerId( lcid );

		// check for already loading/loaded
		if( fLoaded( ) || fLoading( ) )
		{
			// If the load was canceled, we need to force it finish canceling before we restart the load. Or the load will fail.
			if( mLoader && mLoader->fGetCancel( ) )
				fBlockUntilLoaded( );

			delete loader;
			return;
		}

		fInitiateLoad( loader );
	}

	//------------------------------------------------------------------------------
	b32 tResource::fReload( b32 saveDependents )
	{
		// check time stamp first
		tResourcePtrList subs;
		if( saveDependents )
		{
			const u64 newTimeStamp = FileSystem::fGetLastModifiedTimeStamp( fAbsolutePhysicalPath( ) );
			if( newTimeStamp == mFileTimeStamp )
				return false;

			// before unloading, we want to add load references to each sub-resource to prevent them from getting unloaded immediately
			fGatherSubResources( subs );
		}

		log_line( 0, "Reloading resource " << fGetResourceId( ) );

		fInitiateUnload( );

		sigassert( mProvider );
		fInitiateLoad( mProvider->fCreateResourceLoader( tResourcePtr( this ) ) );

		fBlockUntilLoaded( );

		return true;
	}

	//------------------------------------------------------------------------------
	void tResource::fAddLoadCallerId( const tLoadCallerId& lcid )
	{
		// we'll put one caller id in. consider this a flag that "loaded has been called at least once."
		if( !mLoadCallerIds.fCount( ) )
			mLoadCallerIds.fPushBack( lcid );
	}

	//------------------------------------------------------------------------------
	tResource::tLoadResult tResource::fUpdateLoad( )
	{
		error_context( "tResource::fUpdateLoad(" << this << ")[" << fGetPath( ) << "]\n" );
		sigassert( mLoader );

		tLoadResult loadResult = mLoader->fUpdate( );
		if( loadResult != tResourceLoader::cLoadPending )
		{
			if( !fOnLoadComplete( loadResult ) )
				loadResult = tResourceLoader::cLoadPending;
		}

		return loadResult;
	}

	//------------------------------------------------------------------------------
	void tResource::fBlockUntilLoaded( )
	{
#ifdef sig_profile
		const tFilePathPtr& path = fGetPath( );
		const char* cpath = path.fCStr( );
		cpath += fMax<s32>(0,path.fLength( ) - (31 - 9)); //-9 for the "RES:BUL->"
		char tempBuffer[32];
		sprintf_s(tempBuffer,32,"RES:BUL->%s",cpath);
		profile_pix(tempBuffer);
#endif//sig_profile
		while( fLoading( ) )
		{
			mOwner->fUpdateLoadingResources( );
			mOwner->fUpdateForBlockingLoad( );
			fSleep( 1 );
		}
	}

	//------------------------------------------------------------------------------
	void tResource::fBlockUnitLoadedNoDependencies( )
	{
		while( fLoading( ) )
		{
			mOwner->fUpdateLoadingOneResource( this );
			mOwner->fUpdateForBlockingLoad( );
			fSleep( 1 );
		}
	}

	//------------------------------------------------------------------------------
	void tResource::fDisableLoadCompleteEvent( )
	{
		mDisableLoadCompleteEvent = true;
	}

	//------------------------------------------------------------------------------
	void tResource::fIssueLoadCompleteEvent( b32 success )
	{
		if( !success )
		{
			fInitiateUnload( );
		}

		// reset disabled state
		mDisableLoadCompleteEvent = false;

		if( !mLockLoadCompleteEvent )
		{
			// issue event callbacks
			mOnLoadComplete.fFire( *this, success );
		}
	}

	//------------------------------------------------------------------------------
	b32 tResource::fIsSubResource( const tResource& testIfSub, tVisitedList& visited ) const
	{
		if( !fGetResourceBuffer( ) || mLoader )
			return false;

		if( visited.fFind( this ) )
			return false; // prevents cycling through other chains which 'testIfSub' is not a part

		visited.fPushBack( this );

		tTypeSpecificProperties* props = tTypeSpecificPropertiesTable::fInstance( ).fFind( fGetClassId( ) );
		if( props && props->mIsSubResource )
			return props->mIsSubResource( *this, testIfSub, visited );

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tResource::fWaitingOnDependents( ) const
	{
		return fGetResourceBuffer( ) && !mLoader && mDisableLoadCompleteEvent;
	}

	//------------------------------------------------------------------------------
	void tResource::fGatherSubResources( tResourcePtrList& subs ) const
	{
		tTypeSpecificProperties* props = tTypeSpecificPropertiesTable::fInstance( ).fFind( fGetClassId( ) );
		if( props && props->mGatherSubResources )
			props->mGatherSubResources( *this, subs );
	}

	//------------------------------------------------------------------------------
	void tResource::fInitiateLoad( const tResourceLoaderPtr& loader )
	{
		sigassert( loader && !mLoader );
		loader->fInitiate( );

		if( !mDisableLoadCompleteEvent && !mLockLoadCompleteEvent )
		{
			if( fLoaded( ) )
				mOnLoadComplete.fFire( *this, true );
			else if( !fLoading( ) )
				mOnLoadComplete.fFire( *this, false );
		}
	}

	//------------------------------------------------------------------------------
	void tResource::fInitiateUnload( )
	{
		if_logging( mPaperTrail.fSetCount( 0 ); )

		mLoadCallerIds.fSetCount( 0 );

		// clear flag, as we're unloading
		mDisableLoadCompleteEvent = false;

		if( mLoader )
		{
			// there is still file IO happening...
			mLoader->fCancel( );
		}
		else if( fGetResourceBuffer( ) )
		{
			log_line( Log::cFlagResource, "Unloading resource [" << fGetPath( ) << "]" );

			// invoke type-specific before-destruction callback
			tTypeSpecificProperties* props = tTypeSpecificPropertiesTable::fInstance( ).fFind( fGetClassId( ) );
			if( props && props->mPreDestroy )
				props->mPreDestroy( *this, mBuffer.fGetRawPtr( ) );

			// free any resource memory
			fFreeResourceBuffer( );
		}
		else if( !mBuffer.fNull( ) )
		{
			mBuffer.fRelease( );
		}
	}

	//------------------------------------------------------------------------------
	void tResource::fAddToLoadList( )
	{
		sigassert( mOwner );
		mOwner->fAddLoadingResource( this );
	}

	//------------------------------------------------------------------------------
	b32 tResource::fOnLoadComplete( tResourceLoader::tLoadResult loadResult )
	{
		b32 success = ( loadResult == tResourceLoader::cLoadSuccess ? true : false );

		if( success )
		{
			// Load stage 0 triggers the post load call
			if( !mLoader || !mLoader->fGetLoadStage( ) )
			{
				mLockLoadCompleteEvent = true;

				// invoke type-specific file-loaded callback
				tTypeSpecificProperties* props = tTypeSpecificPropertiesTable::fInstance( ).fFind( fGetClassId( ) );
				if( props && props->mPostLoad )
					success = props->mPostLoad( *this, mBuffer.fGetRawPtr( ) );

				mLockLoadCompleteEvent = false;
			}

			// If we have more load stages signal so now
			if( mLoader && mLoader->fGetLoadStage( ) > 0 )
				return false;
		}
		else
		{
#ifdef sig_logging
			if( loadResult == tResourceLoader::cLoadFailure )
			{
				log_warning( "Resource " << fGetResourceId( ) << " failed to load." );
				if( mPaperTrail.fCount( ) > 0 )
				{
					log_output( 0, "\tFollow the paper trail: " );
					for( u32 i = mPaperTrail.fCount( ); i > 0; --i )
						log_output( 0, mPaperTrail[ i - 1 ] << ( i > 1 ? ", " : "" ) );
					log_newline( );
				}
				else
					log_line( 0, "\tNo paper trail for this resource." );
			}
#endif//sig_logging

			// make sure any resource memory has been freed
			fFreeResourceBuffer( );
		}

		// release loader object
		if( mLoader )
		{
			delete mLoader;
			mLoader = 0;
		}

		// The game app may need to exit the application if the app has not gotten to a point
		//  where it is capable of handing failed resource loads.
		// Also, no need to burden the user with managing this as an OnLoad callback
		if( loadResult == tResourceLoader::cLoadFailure )
			tApplication::fInstance( ).fResourceLoadFailure( this );

		// if we're cancelled, we need to notify immediately
		if( loadResult == tResourceLoader::cLoadCancel )
			mDisableLoadCompleteEvent = false;

		// notify observers
		if( !mDisableLoadCompleteEvent )
			mOnLoadComplete.fFire( *this, success );

		return true;
	}

}

