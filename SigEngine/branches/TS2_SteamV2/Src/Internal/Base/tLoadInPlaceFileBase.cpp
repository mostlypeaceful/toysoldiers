#include "BasePch.hpp"
#include "tResourceDepot.hpp"

namespace Sig
{

	tLoadInPlaceFileBase::tLoadInPlaceFileBase( )
		: mOwnerResource( 0 )
		, mNumSubResourcesLoadedSuccess( 0 )
		, mNumSubResourcesLoadedFailed( 0 )
	{
	}

	tLoadInPlaceFileBase::tLoadInPlaceFileBase( tNoOpTag )
		: tBinaryFileBase( cNoOpTag )
		, mStringPtrTable( cNoOpTag )
		, mResourceIdTable( cNoOpTag )
		, mResourcePtrTable( cNoOpTag )
		, mOnSubResourceLoaded( cNoOpTag )
	{
	}

	void tLoadInPlaceFileBase::fOnSubResourceLoaded( tResource& resource, b32 success )
	{
		if( success )
			++mNumSubResourcesLoadedSuccess;
		else
			++mNumSubResourcesLoadedFailed;

		if( fNumSubResourcesFinished( ) == mResourcePtrTable.fCount( ) )
		{
			// we need to store the owner resource in a safe pointer, otherwise there's a chance it'll get
			// deleted during the completion callbacks (resulting in bad memory)
			tResourcePtr ownerResource( ( ( tResource* )mOwnerResource ) );

			// all sub resources have been loaded (successfully or not); time to notify anyone who's curious...
			fOnSubResourcesLoaded( *ownerResource );

			const b32 success = ( mNumSubResourcesLoadedFailed == 0 );
			ownerResource->fIssueLoadCompleteEvent( success );
		}
	}

	void tLoadInPlaceFileBase::fInitializeLoadInPlaceTables( )
	{
		// we want to construct in place all the pointer objects...

		for( u32 i = 0; i < mStringPtrTable.fCount( ); ++i )
			mStringPtrTable[i]->mStringPtr.fConstruct( mStringPtrTable[i]->mRawString.fBegin( ) );
		for( u32 i = 0; i < mResourceIdTable.fCount( ); ++i )
			mResourceIdTable[i]->mFilePathPtr.fConstruct( mResourceIdTable[i]->mRawPath.fBegin( ) );
		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
		{
			mResourcePtrTable[i]->mFilePathPtr.fConstruct( mResourcePtrTable[i]->mRawPath.fBegin( ) );
			mResourcePtrTable[i]->mResourcePtr.fConstruct( );
		}
		mOnSubResourceLoaded.fConstruct( );
		mOnSubResourceLoaded.fTreatAsObject( ).fFromMethod<tLoadInPlaceFileBase, &tLoadInPlaceFileBase::fOnSubResourceLoaded>( this );
	}

	void tLoadInPlaceFileBase::fCleanUpLoadInPlaceTables( )
	{
		// we need to force destruct all our in-place pointer objects...

		for( u32 i = 0; i < mStringPtrTable.fCount( ); ++i )
			mStringPtrTable[i]->mStringPtr.fDestroy( );
		for( u32 i = 0; i < mResourceIdTable.fCount( ); ++i )
			mResourceIdTable[i]->mFilePathPtr.fDestroy( );
		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
		{
			mResourcePtrTable[i]->mResourcePtr.fDestroy( );
			mResourcePtrTable[i]->mFilePathPtr.fDestroy( );
		}
		mOnSubResourceLoaded.fDestroy( );
	}

	void tLoadInPlaceFileBase::fRelocateLoadInPlaceTables( ptrdiff_t delta )
	{
		mStringPtrTable.fRelocateInPlace( delta );
		for( u32 i = 0; i < mStringPtrTable.fCount( ); ++i )
		{
			mStringPtrTable[i] = ( tLoadInPlaceStringPtr* )( mStringPtrTable[i].fRawPtr( ) + delta );
			mStringPtrTable[i]->fRelocateInPlace( delta );
		}

		mResourceIdTable.fRelocateInPlace( delta );
		for( u32 i = 0; i < mResourceIdTable.fCount( ); ++i )
		{
			mResourceIdTable[i] = ( tLoadInPlaceResourceId* )( mResourceIdTable[i].fRawPtr( ) + delta );
			mResourceIdTable[i]->fRelocateInPlace( delta );
		}

		mResourcePtrTable.fRelocateInPlace( delta );
		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
		{
			mResourcePtrTable[i] = ( tLoadInPlaceResourcePtr* )( mResourcePtrTable[i].fRawPtr( ) + delta );
			mResourcePtrTable[i]->fRelocateInPlace( delta );
		}
	}

	b32 tLoadInPlaceFileBase::fLoadSubResources( tResource& ownerResource )
	{
		if( mResourcePtrTable.fCount( ) == 0 )
		{
			fOnSubResourcesLoaded( ownerResource );
			return true; // no sub-resources to take care of, bail now
		}

		// now we store a pointer to the owner resource; this should be
		// safe because ultimately i'm an object that's stored inside the resource;
		// i.e., when the resource dies, so do i
		mOwnerResource = ( Sig::byte* )&ownerResource;

		// override the ownerResource's notification mechanism; specifically,
		// we want to call the load-completion notification event only
		// after all sub-resources have also been loaded.
		ownerResource.fDisableLoadCompleteEvent( );

		tResourceDepot* depot = ownerResource.fGetOwner( );
		sigassert( depot );

		const tResource::tLoadCallerIdArray& lcids = ownerResource.fGetLoadCallerIds( );

		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
		{
			// query the depot for this sub-resource
			tResourcePtr& subResource = mResourcePtrTable[ i ]->mResourcePtr.fTreatAsObject( );
			subResource = depot->fQuery( mResourcePtrTable[ i ]->fGetResourceId( ) );

			sigassert( !subResource.fNull( ) );

#if defined( sig_logging )
			const tResource::tPaperTrail& paperTrail = ownerResource.fPaperTrail( );
			for( u32 i = 0; i < paperTrail.fCount( ); ++i )
				subResource->fAddToPaperTrail( paperTrail[ i ] );
			subResource->fAddToPaperTrail( ownerResource.fGetPath( ) );
#endif // sig_logging

			// before trying to load the resource, we need to do a cyclic reference check to
			// avoid an infinite wait (resource0 waiting on resource1, and resource1 waiting on resource0)
			tResource::tVisitedList visited;
			if( subResource->fIsSubResource( ownerResource, visited ) )
			{
				log_line( Log::cFlagResource, "cyclic resources - A: [" << ownerResource.fGetPath( ) << "], B: [" << subResource->fGetPath( ) << "]" );
				if( !subResource->fWaitingOnDependents( ) )
					log_warning( Log::cFlagResource, "\thmmm [" << subResource->fGetPath( ) << "] is not WaitingOnDependents..." );

				// we tell a little white lie here
				fOnSubResourceLoaded( *subResource, true );
			}
			else
			{
				// issue a load on the sub-resource; we treat ourself as the load owner
				subResource->fLoadDefault( this );

				// each sub-resource will call this same callback when it's done; we
				// tally up all the successful and unsuccessful loads; when the sum equals
				// our total number of sub-resources, we notify interested parties
				subResource->fCallWhenLoaded( mOnSubResourceLoaded.fTreatAsObject( ) );
			}
		}

		if( fNumSubResourcesFinished( ) == mResourcePtrTable.fCount( ) && mNumSubResourcesLoadedFailed > 0 )
			return false;
		return true;
	}

	void tLoadInPlaceFileBase::fUnloadSubResources( tResource& ownerResource )
	{
		const tResource::tLoadCallerIdArray& lcids = ownerResource.fGetLoadCallerIds( );

		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
		{
			// unload the sub-resource (we're the load owner of the sub-resource
			tResourcePtr& resource = mResourcePtrTable[i]->mResourcePtr.fTreatAsObject( );
			resource->fUnload( this );
		}
	}

	b32 tLoadInPlaceFileBase::fIsSubResource( const tResource& testIfSub, tResource::tVisitedList& visited ) const
	{
		// go through all sub-resources to check for cyclic reference
		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
		{
			const tResourcePtr& sub = mResourcePtrTable[ i ]->mResourcePtr.fTreatAsObject( );

			if( sub.fNull( ) )
				continue;

			if( sub == &testIfSub )
				return true;

			if( sub->fIsSubResource( testIfSub, visited ) )
				return true;
		}

		return false;
	}

	void tLoadInPlaceFileBase::fGatherSubResources( tResourcePtrList& subs ) const
	{
		subs.fSetCount( mResourcePtrTable.fCount( ) );
		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
			subs[ i ] = mResourcePtrTable[ i ]->mResourcePtr.fTreatAsObject( );
	}


#ifdef target_tools
	void tLoadInPlaceFileBase::fCleanUpTables( )
	{
		for( u32 i = 0; i < mStringPtrTable.fCount( ); ++i )
			delete mStringPtrTable[i];
		mStringPtrTable.fDeleteArray( );

		for( u32 i = 0; i < mResourceIdTable.fCount( ); ++i )
			delete mResourceIdTable[i];
		mResourceIdTable.fDeleteArray( );

		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
			delete mResourcePtrTable[i];
		mResourcePtrTable.fDeleteArray( );
	}

	void tLoadInPlaceFileBase::fCopyTables( const tLoadInPlaceFileBase& copyFrom )
	{
		// assumes we've already cleaned up any existing tables...

		mStringPtrTable.fNewArray( copyFrom.mStringPtrTable.fCount( ) );
		for( u32 i = 0; i < mStringPtrTable.fCount( ); ++i )
			mStringPtrTable[i] = NEW tLoadInPlaceStringPtr( *copyFrom.mStringPtrTable[i] );

		mResourceIdTable.fNewArray( copyFrom.mResourceIdTable.fCount( ) );
		for( u32 i = 0; i < mResourceIdTable.fCount( ); ++i )
			mResourceIdTable[i] = NEW tLoadInPlaceResourceId( *copyFrom.mResourceIdTable[i] );

		mResourcePtrTable.fNewArray( copyFrom.mResourcePtrTable.fCount( ) );
		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
			mResourcePtrTable[i] = NEW tLoadInPlaceResourcePtr( *copyFrom.mResourcePtrTable[i] );
	}

	tLoadInPlaceFileBase::tLoadInPlaceFileBase( const tLoadInPlaceFileBase& other )
	{
		fCopyTables( other );
	}

	tLoadInPlaceFileBase& tLoadInPlaceFileBase::operator=( const tLoadInPlaceFileBase& other )
	{
		if( this == &other )
			return *this;
		fCleanUpTables( );
		fCopyTables( other );
		return *this;
	}

	tLoadInPlaceFileBase::~tLoadInPlaceFileBase( )
	{
		fCleanUpTables( );
	}

	tLoadInPlaceStringPtr*	tLoadInPlaceFileBase::fAddLoadInPlaceStringPtr( const char* s )
	{
		sigassert( s );

		// first look for an existing equivalent entry and return it if found
		for( u32 i = 0; i < mStringPtrTable.fCount( ); ++i )
		{
			if( !strcmp( s, mStringPtrTable[i]->mRawString.fBegin( ) ) )
				return mStringPtrTable[i];
		}

		// didn't find an existing entry, add a new one

		tLoadInPlaceStringPtr* add = NEW tLoadInPlaceStringPtr;
		fZeroOut( add );
		add->mRawString.fCreateNullTerminated( s );

		mStringPtrTable.fPushBack( add );
		return add;
	}

	namespace
	{
		void fFillLIPResourceId( tLoadInPlaceResourceId* output, const tResourceId& rid )
		{
			output->mClassId = rid.fGetClassId( );
			output->mRawPath.fCreateNullTerminated( rid.fGetPath( ).fCStr( ) );
			output->mWillResize = rid.fWillResizeAfterLoad( );
		}
	}

	tLoadInPlaceResourceId* tLoadInPlaceFileBase::fAddLoadInPlaceResourceId( const tResourceId& rid )
	{
		// first look for an existing equivalent entry and return it if found
		for( u32 i = 0; i < mResourceIdTable.fCount( ); ++i )
		{
			const tResourceId test = mResourceIdTable[i]->fGetResourceIdRawPath( );
			if( rid == test )
				return mResourceIdTable[i];
		}

		// didn't find an existing entry, add a new one

		tLoadInPlaceResourceId* add = NEW tLoadInPlaceResourceId;
		fZeroOut( add );
		fFillLIPResourceId( add, rid );

		mResourceIdTable.fPushBack( add );
		return add;
	}
	tLoadInPlaceResourcePtr* tLoadInPlaceFileBase::fAddLoadInPlaceResourcePtr( const tResourceId& rid )
	{
		// first look for an existing equivalent entry and return it if found
		for( u32 i = 0; i < mResourcePtrTable.fCount( ); ++i )
		{
			const tResourceId test = mResourcePtrTable[i]->fGetResourceIdRawPath( );
			if( rid == test )
				return mResourcePtrTable[i];
		}

		// didn't find an existing entry, add a new one

		tLoadInPlaceResourcePtr* add = NEW tLoadInPlaceResourcePtr;
		fZeroOut( add );
		fFillLIPResourceId( add, rid );

		mResourcePtrTable.fPushBack( add );
		return add;
	}
#endif//target_tools

}

