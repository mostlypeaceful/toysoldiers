//------------------------------------------------------------------------------
// \file tDeviceSelector.cpp - 14 Nov 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tDeviceSelector.hpp"
#include "tContent.hpp"
#include "tUser.hpp"


namespace Sig
{
	//------------------------------------------------------------------------------
	// tDeviceSelector
	//------------------------------------------------------------------------------
	const tContentDeviceId tDeviceSelector::cInvalidDeviceId = tContentData::cContentDeviceIdAny;

	//------------------------------------------------------------------------------
	tDeviceSelector::tDeviceSelector( u32 contentType, u64 minSize )
		: mSelectionState( cSelectionState_Idle )
		, mForceSelection( false )
		, mLocalHwIdx( tUser::cMaxLocalUsers )
		, mContentType( contentType )
		, mMinFileSize( minSize )
		, mDeviceId( cInvalidDeviceId )
	{
		fPlatformConstruct( );
	}

	//------------------------------------------------------------------------------
	tDeviceSelector::~tDeviceSelector( )
	{
		fPlatformDestruct( );
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fSetDeviceId( tContentDeviceId id )
	{
		sigassert( mSelectionState != cSelectionState_Selecting );
		mSelectionState = cSelectionState_Idle;
		mDeviceId = id;

		return fNeedsToChooseDevice( );
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fDeviceHasEnoughSpace( u64 size ) const
	{
		if( !fDeviceSelected( ) )
			return false;

		return fPlatformDeviceFreeSpace( ) >= fPlatformFullSize( size );
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fNeedsToChooseDevice( ) const
	{
		if( !fDeviceHasEnoughSpace( mMinFileSize ) )
			return true;

		return mSelectionState == cSelectionState_Idle;
	}

	//------------------------------------------------------------------------------
	void tDeviceSelector::fChooseDevice( u32 localHwIdx, b32 forceShow )
	{
		sigassert( localHwIdx < tUser::cMaxLocalUsers && "Invalid local use idx passed to fChooseDevice" );
		sigassert( mSelectionState == cSelectionState_Idle && "Cannot overlap device selections!" );

		if( !tUser::fSignedIn( localHwIdx ) )
		{
			log_warning( "User must be signed in to select storage device" );
			return;
		}

		mLocalHwIdx = localHwIdx;
		mDeviceId = cInvalidDeviceId;
		mSelectionState = cSelectionState_Desired;
		mForceSelection = forceShow;
	}

	//------------------------------------------------------------------------------
	void tDeviceSelector::fTick( )
	{
		switch( mSelectionState )
		{
		case cSelectionState_Desired: 
			{
				sigassert( mLocalHwIdx < tUser::cMaxLocalUsers && "Sanity!" );
				if( fPlatformShowUI( mForceSelection ) )
					mSelectionState = cSelectionState_Selecting;
			} break;
		case cSelectionState_Selecting:
			{
				b32 success;
				if( fPlatformUpdateUI( success, false ) )
				{
					mSelectionState = cSelectionState_Idle;
				}
			}
			break;
		}
	}

	//------------------------------------------------------------------------------
	void tDeviceSelector::fReset( u32 contentType, u64 minSize )
	{
		// Wait for the op to complete
		if( mSelectionState == cSelectionState_Selecting )
		{
			b32 ignored;
			fPlatformUpdateUI( ignored, true );
		}

		mSelectionState = cSelectionState_Idle;
		mDeviceId = cInvalidDeviceId;
		mContentType = contentType;
		mMinFileSize = minSize;
	}

} // ::Sig
