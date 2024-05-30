#include "BasePch.hpp"
#include "tResourceDepot.hpp"

// for querying common resources
#include "FileSystem.hpp"
#include "tFilePackage.hpp"
#include "tFilePackageFile.hpp"
#include "tSceneGraphFile.hpp"
#include "Scripts/tScriptFile.hpp"
#include "Gfx/tTextureFile.hpp"

namespace Sig
{
	devvar( bool,	Debug_Resources_QueryLoadBlockHitchWarning,			true );
	devvar( u32,	Debug_Resources_QueryLoadBlockHitchThreshholdMS,	100 );

	tResourceDepot::tResourceDepot( )
		: mResourceTable( 32 )
		, mResourceLoadingTimeoutMS( 1.0f )
	{
	}

	tResourceDepot::~tResourceDepot( )
	{
		for( tResourceTable::tIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			i->mValue->mOwner = 0;
		}
	}

	void tResourceDepot::fSetResourceLoadingTimeoutMS( f32 ms )
	{
		mResourceLoadingTimeoutMS = ms;
	}

	void tResourceDepot::fAddResourceProvider( const tResourceProviderPtr& provider )
	{
		mResourceProviders.fPushBack( provider );
	}

	void tResourceDepot::fRemoveResourceProvider( const tResourceProviderPtr& provider )
	{
		mResourceProviders.fFindAndEraseOrdered( provider );
	}

	b32 tResourceDepot::fResourceExists( const tFilePathPtr& path ) const
	{
		for( u32 i = 0; i < mResourceProviders.fCount( ); ++i )
		{
			if( mResourceProviders[ i ]->fResourceExists( path ) )
				return true;
		}

		return false;
	}

	tResourcePtr tResourceDepot::fQuery( const tResourceId& in_rid )
	{
		if( !in_rid.fGetPath( ).fExists( ) )
		{
			log_warning( "Resource queried with no filepath." );
			return tResourcePtr( );
		}

		// Remap the resource if necessary
		tResourceId rid = fResolveRemap( in_rid );
		tResource** find = mResourceTable.fFind( rid );

		if( !find )
		{
			// Find a resource provider that can provide the resource. For now disable if we're looking at file packages
			tResourceProviderPtr resourceProvider;
			for( u32 i = 0; i < mResourceProviders.fCount( ); ++i )
			{
				if( mResourceProviders[ i ]->fResourceExists( rid.fGetPath( ) ) )
				{
					resourceProvider = mResourceProviders[ i ];
					break;
				}
			}

			if( !resourceProvider )
			{
				log_warning( "Query for invalid file: " << rid.fGetPath( ) );
				return tResourcePtr( );
			}

			// it doesn't exist, so create it
			find = mResourceTable.fInsert( rid, NEW_TYPED( tResource )( rid, resourceProvider.fGetRawPtr( ) ) );
			sigassert( find );
			(*find)->fSetOwner( this );
		}

		sigassert( find );
		return tResourcePtr( *find );
	}

	tResourcePtr tResourceDepot::fQueryLoadBlock( const tResourceId& rid, const tResource::tLoadCallerId& lcid )
	{
#ifdef sig_profile
		const tFilePathPtr& path = rid.fGetPath( );
		const char* cpath = path.fCStr( );
		cpath += fMax<s32>(0,path.fLength( ) - (31 - 9)); //-9 for the "RES:QLB->"
		char tempBuffer[32];
		sprintf_s(tempBuffer,32,"RES:QLB->%s",cpath);
		profile_pix(tempBuffer);
#endif // def sig_profile

#ifdef sig_logging
		Time::tStopWatch hitchTimer;
#endif // def sig_logging

		tResourcePtr o = fQuery( rid );
		const b32 wasLoaded = o && o->fLoaded( );
		if( o )
		{
			o->fLoadDefault( lcid );
			o->fBlockUntilLoaded( );
		}

#ifdef sig_logging
		const u32 hitchTime = (u32)hitchTimer.fGetElapsedMs( );
		if( !wasLoaded && Debug_Resources_QueryLoadBlockHitchWarning && hitchTime >= Debug_Resources_QueryLoadBlockHitchThreshholdMS )
			log_warning( "fQueryLoadBlock Hitch (" << hitchTime << " ms): " << rid );
#endif // def sig_logging

		return o;
	}

	tResourcePtr tResourceDepot::fQueryLoadBlockNoDependencies( const tResourceId& rid, const tResource::tLoadCallerId& lcid )
	{
		tResourcePtr o = fQuery( rid );
		o->fLoadDefault( lcid );
		o->fBlockUnitLoadedNoDependencies( );
		return o;
	}

	void tResourceDepot::fUpdateLoadingResources( )
	{
		Time::tStopWatch timer;

		for( u32 i = 0; i < mLoadingResources.fCount( ); ++i )
		{
			// in order to prevent initiating "too many" loads at once, we continue to cycle through the first N resources;
			// this is just an empirically-arrived-at number which seems to yield a decent combination of memory/speed etc,
			// with the idea being we want to maximize our IO throughput while not allocating too many read buffers at once
			i = ( i % 16 );

			tResourcePtr resource( mLoadingResources[ i ] );
			sigassert( !resource.fNull( ) );

			const tResourceLoader::tLoadResult loadResult = resource->fUpdateLoad( );
			if( loadResult != tResourceLoader::cLoadPending )
			{
				// load was either successful or failed, either way it's done
				mLoadingResources.fEraseOrdered( i );
				--i;
			}

			// check timer to see if we should continue updating resources
			if( timer.fGetElapsedMs( ) >= mResourceLoadingTimeoutMS )
				break;
		}
	}

	void tResourceDepot::fCancelLoadingResources( )
	{
		for( u32 i = 0; i < mLoadingResources.fCount( ); ++i )
		{
			mLoadingResources[ i ]->fGetLoader( )->fCancel( );
		}
	}

	void tResourceDepot::fUpdateLoadingOneResource( tResource* res )
	{
		// this will not try to load dependencies, only call it on resources with no dependencies
		tResourcePtr resource( res );
		sigassert( !resource.fNull( ) );

		const tResourceLoader::tLoadResult loadResult = resource->fUpdateLoad( );
		if( loadResult != tResourceLoader::cLoadPending )
		{
			// load was either successful or failed, either way it's done
			mLoadingResources.fFindAndEraseOrdered( resource );
		}
	}

	void tResourceDepot::fQueryByType( Rtti::tClassId cid, tGrowableArray<tResourcePtr>& resources ) const
	{
		for( tResourceTable::tConstIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( cid != i->mValue->fGetClassId( ) )
				continue;
			if( !i->mValue->fLoaded( ) )
				continue;
			resources.fPushBack( tResourcePtr( i->mValue ) );
		}
	}

	void tResourceDepot::fQueryByTypes( const tGrowableArray<Rtti::tClassId>& cids, tGrowableArray<tResourcePtr>& resources ) const
	{
		for( u32 i = 0; i < cids.fCount( ); ++i )
			fQueryByType( cids[ i ], resources );
	}

	void tResourceDepot::fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const
	{
		for( u32 i = 0; i < mResourceProviders.fCount( ); ++i )
		{
			tGrowableArray<tResourceId> curResourceIds;
			mResourceProviders[ i ]->fQueryCommonResourcesByFolder( curResourceIds, resourceFolders, resourceFolderCount );
			resourceIds.fJoin( curResourceIds );
		}
	}

	//------------------------------------------------------------------------------
	void tResourceDepot::fQueryResourcesByFolder( 
		tFilePathPtrList& filePaths, 
		const tFilePathPtr & ext, 
		const tFilePathPtr resourceFolders[], 
		u32 resourceFolderCount ) const
	{
		for( u32 i = 0; i < mResourceProviders.fCount( ); ++i )
		{
			tFilePathPtrList curFilePaths;
			mResourceProviders[ i ]->fQueryResourcesByFolder( curFilePaths, ext, resourceFolders, resourceFolderCount );
			filePaths.fJoin( curFilePaths );
		}
	}

	void tResourceDepot::fReloadResourcesByType( const tGrowableArray<Rtti::tClassId>& cids )
	{
		for( tResourceTable::tIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( !i->mValue->fLoaded( ) )
				continue;
			if( !cids.fFind( i->mValue->fGetClassId( ) ) )
				continue;
			i->mValue->fReload( );
		}
	}

	void tResourceDepot::fRemoveResource( tResource* res )
	{
		sigassert( res );
		sigassert( res->mOwner == this );
		sigassert( !res->mLoader );

		res->mOwner = 0;
		tResource** find = mResourceTable.fFind( res->fGetResourceId( ) );
		sigassert( find );
		mResourceTable.fRemove( find );
	}

	void tResourceDepot::fRemoveLoadingResource( tResource* res )
	{
		mLoadingResources.fFindAndErase( res );
	}

	void tResourceDepot::fAddLoadingResource( tResource* res )
	{
		sigassert( res );
		sigassert( !mLoadingResources.fFind( res ) );
		mLoadingResources.fPushBack( res );
	}

	tResourceId tResourceDepot::fResolveRemap( const tResourceId& in_rid )
	{
		//check to see if we need to update the requested resource load
		// if not, use what was provided.
		const tFilePathPtr* tgtPath = mRemapTable.fFind( in_rid.fGetPath( ) );
		//if_logging( if( tgtPath ) log_line( 0, in_rid.fGetPath( ) << " SWAPPED TO: " << *tgtPath ); )
		if( tgtPath )
			return fResolveRemap( tResourceId::fMake( in_rid.fGetClassId( ), *tgtPath ) );
		else
			return in_rid;
	}

	u32 tResourceDepot::fResourcesWithLoadRefsCount( ) const
	{
		u32 o = 0;
		for( tResourceTable::tConstIterator i = mResourceTable.fBegin( ), iend = mResourceTable.fEnd( );
			 i != iend;
			 ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( !i->mValue->fHasLoadCaller( ) )
				continue;
			++o;
		}
		return o;
	}

	void tResourceDepot::fAddRemap( const tFilePathPtr& from, const tFilePathPtr& to )
	{
		mRemapTable.fInsert( from, to );

#ifdef sig_assert
		for( tResourceTable::tConstIteratorNoNullOrRemoved it( mResourceTable.fBegin( ), mResourceTable.fEnd( ) ); it.fNotDone( ); ++it )
		{
			tErrorContext ec;
			ec << "PaperTrail:\n";
			for( u32 i = it->mValue->fPaperTrail( ).fCount( ); i > 0; --i )
				ec << it->mValue->fPaperTrail( )[ i - 1 ] << "\n";
			log_assert( it->mKey.fGetPath( ) != from, "Resource [" << from << "] already loaded before the choice could be made to remap it to [" << to << "]" );
		}
#endif
	}

	void tResourceDepot::fClearRemapTable( )
	{
		mRemapTable.fClear( );
	}

	void tResourceDepot::fDumpRemapTable( ) const
	{
#ifdef sig_logging

		tRemapTable::tConstIterator itr = mRemapTable.fBegin( );
		const tRemapTable::tConstIterator end = mRemapTable.fEnd( );

		while( itr != end )
		{
			if( !itr->fNullOrRemoved( ) )
				log_line( 0, "Remap: " << itr->mKey << " to " << itr->mValue );

			++itr;
		}
#endif
	}

	void tResourceDepot::fUpdateForBlockingLoad( )
	{
		for( u32 i = 0; i < mResourceProviders.fCount( ); ++i )
			mResourceProviders[ i ]->fUpdateForBlockingLoad( );
	}

#ifdef target_tools
	tResourceDepot::tToolsTextureDelegate tResourceDepot::sToolsTextureDelegate;

	Gfx::tTextureReference tResourceDepot::fToolsTextureQuery( const tFilePathPtr& path, Math::tVec2f& sizeOut )
	{
		if( !sToolsTextureDelegate.fNull( ) )
			return sToolsTextureDelegate( path, sizeOut );
		else
			return Gfx::tTextureReference( );
	}
#endif

	void tResourceDepot::fAddToStatText( std::wstringstream& currentStats )
	{
		if( mLoadingResources.fCount( ) )
			currentStats << "tResourceDepot loading " << mLoadingResources.fCount( ) << " resources..." << std::endl;
	}
	
}

