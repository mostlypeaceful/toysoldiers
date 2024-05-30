//------------------------------------------------------------------------------
// \file tDeviceSelector.hpp - 14 Nov 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tDeviceSelector__
#define __tDeviceSelector__
namespace Sig 
{

#if defined( platform_xbox360 )
	typedef XCONTENTDEVICEID tContentDeviceId;
#else
	typedef u32 tContentDeviceId;
#endif

	///
	/// \class tDeviceSelector
	/// \brief Helper for selecting and maintaining a save device
	class tDeviceSelector
	{
	public:
		
		enum tSelectionState
		{
			cSelectionState_Idle,
			cSelectionState_Desired,
			cSelectionState_Selecting,
		};

	public:
		
		// contentType should be one of tContentData::cContentType*
		tDeviceSelector( u32 contentType, u64 minSize );
		~tDeviceSelector( );

		u32 fSelectionState( ) const { return mSelectionState; }

		b32 fDeviceSelected( ) const { return mDeviceId != cInvalidDeviceId; }
		tContentDeviceId fDeviceId( ) const { return mDeviceId; }
		b32 fSetDeviceId( tContentDeviceId id ); // Returns the result of fNeedsToChooseDevice

		b32 fDeviceIsAvailable( ) const { return fDeviceHasEnoughSpace( mMinFileSize ); }
		b32 fDeviceHasEnoughSpace( u64 size ) const;

		b32 fNeedsToChooseDevice( ) const;
		void fChooseDevice( u32 localHwIdx, b32 forceShow );

		void fTick( );

		void fReset( ) { fReset( mContentType, mMinFileSize ); }
		void fReset( u32 contentType, u64 minSize );

	private:
		
		static const tContentDeviceId cInvalidDeviceId;

		

	private:

		void fPlatformConstruct( );
		void fPlatformDestruct( );
		u64 fPlatformDeviceFreeSpace( ) const;
		u64 fPlatformFullSize( u64 size ) const;
		
		b32 fPlatformShowUI( b32 force );
		b32 fPlatformUpdateUI( b32& success, b32 wait );

	private:

		u32	mSelectionState;
		b32 mForceSelection;

		u32	mLocalHwIdx;
		u32	mContentType;
		u64	mMinFileSize;

		tContentDeviceId mDeviceId;

#if defined( platform_xbox360 )
		XOVERLAPPED					mOverlapped;
#endif

	};

} // ::Sig

#endif//__tDeviceSelector__