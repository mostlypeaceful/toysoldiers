//------------------------------------------------------------------------------
// \file tWeakPtr.cpp - 22 Feb 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tWeakPtr.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tWeakPtrHead
	//------------------------------------------------------------------------------
	void tWeakPtrHead::fLink( tLink * link )
	{
		static_assert( offsetof( tWeakPtrHead, mNext ) == offsetof( tLink, mNext ) );

		fAquireLock( );
		{
			// By always inserting after the head, we ensure that nothing ever 
			// accesses this except to set the mNext component - the offset of 
			// which is guaranteed to be equal to that of tLink by the above static_assert
			link->mPrevious = (tLink*)this;
			link->mNext = mNext;
			if( mNext )
				mNext->mPrevious = link;
			mNext = link;
		}
		fReleaseLock( );
	}

	//------------------------------------------------------------------------------
	void tWeakPtrHead::fUnlink( tLink * lnk )
	{
		fAquireLock( );
		{
			fUnlinkNoLock( lnk );
		}
		fReleaseLock( );
	}

	//------------------------------------------------------------------------------
	void tWeakPtrHead::fUnlinkAll( )
	{
		fAquireLock( );
		{
			while( mNext )
				fUnlinkNoLock( mNext );
		}
		fReleaseLock( );
	}

	//------------------------------------------------------------------------------
	void tWeakPtrHead::fAquireLock( )
	{
		while( interlocked_cmp_ex( &mLock, 1u, 0u ) );
	}

	//------------------------------------------------------------------------------
	void tWeakPtrHead::fReleaseLock( )
	{
		const u32 wasLocked = interlocked_ex( &mLock, 0u );
		sigassert( wasLocked && "Released an already unlocked lock" );
	}

	//------------------------------------------------------------------------------
	void tWeakPtrHead::fUnlinkNoLock( tLink * lnk )
	{
		sigassert( lnk->mPrevious && "All links should have a previous" );

		lnk->mPrevious->mNext = lnk->mNext;
		if( lnk->mNext )
			lnk->mNext->mPrevious = lnk->mPrevious;

		lnk->mNext = NULL;
		lnk->mPrevious = NULL;
	}

} // ::Sig
