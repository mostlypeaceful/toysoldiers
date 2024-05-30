#include "BasePch.hpp"
#include "tResourceLoadList.hpp"

namespace Sig
{
	tResourceLoadList::tResourceLoadList( )
		: mLoadAttemptCount( 0 )
		, mLoadSuccessCount( 0 )
		, mLoadFailureCount( 0 )
		, mOnLoadComplete( false )
	{
		mOnResourceLoaded.fFromMethod<tResourceLoadList, &tResourceLoadList::fOnResourceLoaded>( this );
	}

	tResourceLoadList::~tResourceLoadList( )
	{
		fUnloadAll( );
	}

	void tResourceLoadList::fAddToLoadList( const tResourcePtr& resource )
	{
		if( resource )
			mLoadList.fFindOrAdd( resource );
	}

	void tResourceLoadList::fLoadAll( )
	{
		mLoadAttemptCount = mLoadList.fCount( );
		for( u32 i = 0; i < mLoadList.fCount( ); ++i )
			fLoad( mLoadList[ i ] );

		if( mLoadList.fCount( ) == 0 )
			fOnAllResourcesLoaded( );
	}

	void tResourceLoadList::fUnloadAll( )
	{
		mLoadAttemptCount = 0;
		mLoadSuccessCount = 0;
		mLoadFailureCount = 0;

		mLoadList.fSetCount( 0 );
	}

	void tResourceLoadList::fCallOnLoadComplete( tOnLoadComplete::tObserver& cb )
	{
		mOnLoadComplete.fAddObserver( &cb );

		if( fLoadComplete( ) )
			cb( *this );
	}

	void tResourceLoadList::fLoad( const tResourcePtr& resource )
	{
		sigassert( !resource.fNull( ) );

		// initiate the load
		resource->fLoadDefault( this );
		resource->fCallWhenLoaded( mOnResourceLoaded );
	}

	void tResourceLoadList::fOnResourceLoaded( tResource& theResource, b32 success )
	{
		if( success )
			++mLoadSuccessCount;
		else
			++mLoadFailureCount;

		if( fLoadComplete( ) )
		{
			mOnResourceLoaded.fRemoveFromAllEvents( );
			mOnLoadComplete.fFire( *this );
			fOnAllResourcesLoaded( );
		}
	}

	void tResourceLoadList::fOnAllResourcesLoaded( )
	{
		// derived types can implement this method
	}

}

