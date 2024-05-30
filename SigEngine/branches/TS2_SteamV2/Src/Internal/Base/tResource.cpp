#include "BasePch.hpp"
#include "tResource.hpp"
#include "tResourceDepot.hpp"
#include "FileSystem.hpp"
#include "tFilePackage.hpp"
#include "tStandardResourceLoader.hpp"
#include "tApplication.hpp"

namespace Sig
{
	devvar( bool, Debug_Resources_EnableLogging, false );
	devvar( bool, Debug_Resources_PackFileOnly, false );

	b32 tResource::fLoggingEnabled( )
	{
		return Debug_Resources_EnableLogging;
	}

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

	tResource::tResource( const tResourceId& rid, tResourceDepot* owner )
		: tResourceId( rid )
		, mOwner( owner )
		, mDisableLoadCompleteEvent( false )
		, mLockLoadCompleteEvent( false )
		, mOnLoadComplete( false )
		, mLoader( 0 )
		, mFileTimeStamp( 0 )
	{ 
	}

	tResource::~tResource( )
	{
		sigassert( !mLoader );
		log_assert( mLoadCallerIds.fCount( ) == 0, "Resource " << fGetResourceId( ) << " is being destroyed, but still has load caller ids." );

		fInitiateUnload( );
		fRemoveFromDepot( );
	}

	tFilePathPtr tResource::fAbsolutePhysicalPath( ) const
	{
		tFilePathPtr o;

		if( mOwner && mOwner->fRootPath( ).fExists( ) )
			o = tFilePathPtr::fConstructPath( mOwner->fRootPath( ), fGetPath( ) );
		else
			o = fGetPath( );

		return o;
	}

	void tResource::fCallWhenLoaded( tOnLoadComplete::tObserver& cb )
	{
		mOnLoadComplete.fAddObserver( &cb );

		if( !mDisableLoadCompleteEvent && !mLockLoadCompleteEvent && ( fLoaded( ) || !fLoading( ) ) )
			cb( *this, fLoaded( ) );
	}

	void tResource::fRemoveLoadCallback( tOnLoadComplete::tObserver& cb )
	{
		mOnLoadComplete.fRemoveObserver( &cb );
	}

	b32 tResource::fLoadFromPackage( const tLoadCallerId& lcid )
	{
		tFilePackageLink* fpl = fFindBestPackageToLoadFrom( );
		if( !fpl )
			return false; // couldn't find any packages to load from

		fpl->mPackage->fLoadSubFile( lcid, tResourcePtr( this ), fpl->mPackage->fAccessFile( ), fpl->mIthHeader );
		return true;
	}

	void tResource::fLoadStandard( const tLoadCallerId& lcid )
	{
		fLoad( lcid, tResourceLoaderPtr( NEW tStandardResourceLoader( this ) ) );
	}

	void tResource::fLoadDefault( const tLoadCallerId& lcid )
	{
		if( Debug_Resources_PackFileOnly )
		{
			if( mFilePackageLinks.fCount( ) > 0 )
				fLoadFromPackage( lcid );
			else
			{
				fAddLoadCallerId( lcid );
				fOnLoadComplete( tResourceLoader::cLoadFailure );
			}
		}
		else
		{
			if( mFilePackageLinks.fCount( ) > 0 )
				fLoadFromPackage( lcid );
			else
				fLoadStandard( lcid );
		}
	}

	void tResource::fLoad( const tLoadCallerId& lcid, const tResourceLoaderPtr& loader )
	{
		// add load caller id
		if( !fAddLoadCallerId( lcid ) )
		{
			delete loader;
			return;
		}

		// check for already loading/loaded
		if( fLoaded( ) || fLoading( ) )
		{
			delete loader;
			return;
		}

		fInitiateLoad( loader );
	}

	void tResource::fUnload( const tLoadCallerId& lcid )
	{
		// IMPORTANT we don't actually remove the load caller id
		// until AFTER we've initiated the unload; this is so
		// that any unloading callbacks have access to the complete
		// list of caller ids.

		// first look for lcid in our list; if it's not there, we ignore the call
		const tLoadCallerId* find = mLoadCallerIds.fFind( lcid );
		if( !find )
			return;

		// now, if there are still other load callers, we can't actually unload the resource yet
		if( mLoadCallerIds.fCount( ) == 1 )
			fInitiateUnload( );

		// now we actually remove the load caller id
		mLoadCallerIds.fErase( fPtrDiff( find, mLoadCallerIds.fBegin( ) ) );
	}

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
			for( u32 i = 0; i < subs.fCount( ); ++i )
				if( subs[ i ] ) subs[ i ]->fAddLoadCallerId( ( tLoadCallerId )0xDEADFACE );
		}

		log_line( Log::cFlagResource, "Reloading resource " << fGetResourceId( ) );

		fInitiateUnload( );

		tFilePackageLink* fpl = fFindBestPackageToLoadFrom( );
		if( fpl )
			fInitiateLoad( fpl->mPackage->fCreateResourceLoader( tResourcePtr( this ), fpl->mPackage->fAccessFile( ), fpl->mIthHeader ) );
		else
			fInitiateLoad( tResourceLoaderPtr( NEW tStandardResourceLoader( this ) ) );

		fBlockUntilLoaded( );

		// now that we're fully reloaded, we can remove the temporary load reference on the sub resources
		for( u32 i = 0; i < subs.fCount( ); ++i )
			if( subs[ i ] ) subs[ i ]->fUnload( ( tLoadCallerId )0xDEADFACE );

		return true;
	}

	b32 tResource::fIsLoadCaller( const tLoadCallerId& lcid ) const
	{
		return mLoadCallerIds.fFind( lcid ) != 0;
	}

	b32 tResource::fAddLoadCallerId( const tLoadCallerId& lcid )
	{
		// first look for load caller id; if they've already called load, skip
		if( mLoadCallerIds.fFind( lcid ) )
			return false;
		// register the caller id
		mLoadCallerIds.fPushBack( lcid );
		return true;
	}

	tResource::tLoadResult tResource::fUpdateLoad( )
	{
		sigassert( mLoader );

		if( Debug_Resources_EnableLogging )
			log_line( Log::cFlagResource, "tResource::fUpdateLoad: " << fGetPath( ) );

		const tLoadResult loadResult = mLoader->fUpdate( );
		if( loadResult != tResourceLoader::cLoadPending )
			fOnLoadComplete( loadResult );

		return loadResult;
	}

	void tResource::fBlockUntilLoaded( )
	{
		while( fLoading( ) )
		{
			mOwner->fUpdateLoadingResources( 1.f );
			fSleep( 1 );
		}
	}

	void tResource::fBlockUnitLoadedNoDependencies( )
	{
		while( fLoading( ) )
		{
			mOwner->fUpdateLoadingOneResource( this );
			fSleep( 1 );
		}
	}

	void tResource::fAddFilePackageLink( tFilePackage* package, u32 ithHeader )
	{
		if( !mFilePackageLinks.fFind( package ) )
			mFilePackageLinks.fPushBack( tFilePackageLink( package, ithHeader ) );
	}

	void tResource::fRemoveFilePackageLink( const tFilePackage* package )
	{
		mFilePackageLinks.fFindAndEraseOrdered( package );
	}

	tResource::tFilePackageLink tResource::fGetFilePackageLink( const tFilePackage* package )
	{
		const tFilePackageLink* find = mFilePackageLinks.fFind( package );
		return find ? *find : tFilePackageLink( );
	}

	void tResource::fDisableLoadCompleteEvent( )
	{
		mDisableLoadCompleteEvent = true;
	}

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

	b32 tResource::fWaitingOnDependents( ) const
	{
		return fGetResourceBuffer( ) && !mLoader && mDisableLoadCompleteEvent;
	}

	void tResource::fGatherSubResources( tResourcePtrList& subs ) const
	{
		tTypeSpecificProperties* props = tTypeSpecificPropertiesTable::fInstance( ).fFind( fGetClassId( ) );
		if( props && props->mGatherSubResources )
			props->mGatherSubResources( *this, subs );
	}

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

	void tResource::fInitiateUnload( )
	{
		if_logging( mPaperTrail.fSetCount( 0 ); )

		// clear flag, as we're unloading
		mDisableLoadCompleteEvent = false;

		if( mLoader )
		{
			// there is still file IO happening...
			mLoader->fCancel( );
		}
		else if( fGetResourceBuffer( ) )
		{
			if( fLoggingEnabled( ) )
				log_line( Log::cFlagResource, "Unloading resource [" << fGetPath( ) << "]" );

			// invoke type-specific before-destruction callback
			tTypeSpecificProperties* props = tTypeSpecificPropertiesTable::fInstance( ).fFind( fGetClassId( ) );
			if( props && props->mPreDestroy )
				props->mPreDestroy( *this, mBuffer.fGetRawPtr( ) );

			// free any resource memory
			fFreeResourceBuffer( );
		}
	}

	void tResource::fRemoveFromDepot( )
	{
		// just in case
		if( mOwner )
		{
			mOwner->fRemoveLoadingResource( this );
			mOwner->fRemoveResource( this );
		}
	}

	void tResource::fAddToLoadList( )
	{
		sigassert( mOwner );
		mOwner->fAddLoadingResource( this );
	}


	tResource::tFilePackageLink* tResource::fFindBestPackageToLoadFrom( )
	{
		// priority is given to the last registered package
		if( mFilePackageLinks.fCount( ) > 0 )
			return &mFilePackageLinks.fBack( );
		return 0;
	}

	void tResource::fOnLoadComplete( tResourceLoader::tLoadResult loadResult )
	{
		b32 success = ( loadResult == tResourceLoader::cLoadSuccess ? true : false );

		// release loader object
		if( mLoader )
		{
			delete mLoader;
			mLoader = 0;
		}

		if( success )
		{
			mLockLoadCompleteEvent = true;

			// invoke type-specific file-loaded callback
			tTypeSpecificProperties* props = tTypeSpecificPropertiesTable::fInstance( ).fFind( fGetClassId( ) );
			if( props && props->mPostLoad )
				success = props->mPostLoad( *this, mBuffer.fGetRawPtr( ) );

			mLockLoadCompleteEvent = false;
		}
		else
		{
#ifdef sig_logging
			if( loadResult == tResourceLoader::cLoadFailure )
			{
				log_warning( Log::cFlagResource,
					"Resource " << fGetResourceId( ) << " failed to load (PackFileOnly=" << (Debug_Resources_PackFileOnly?"true":"false") << ")." );
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
	}

}

