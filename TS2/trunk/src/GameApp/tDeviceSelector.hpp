#ifndef __tDeviceSelectorLogic__
#define __tDeviceSelectorLogic__
#endif // __tDeviceSelectorLogic__

namespace Sig { 

	class tUser;

	class tDeviceSelector
	{
	public:
		tDeviceSelector( u64 minSize );

		u32 fSaveDeviceId( );
		void fSetSaveDeviceId( u8 deviceId );
		void fChooseSaveDeviceId( tUser& user, b32 forceShow );
		b32 fNeedsToChooseSaveDevice( u8 curDeviceID );
		b32 fDeviceSelected( );
		void fTick( );

		b32 fDeviceHasEnoughSpace( u64 size );
		b32 fDeviceIsAvailable( );

		void fReset( );

		b32							mLaunchedDeviceSelectionUI;
		b32							mSelectionJustCompleted;

	private:
		b32 fMoreThanOneDevice( );
	private:
#if defined( platform_xbox360 )
		XCONTENTDEVICEID			mSaveDeviceId;
		XOVERLAPPED					mDeviceIdXov;
#endif
		DWORD						mPrevStatusResult;

		tUser*						mUser;
		u64							mMinFileSize;
		b32							mForceShowUi;
		b32							mTryingToShowUi;
	};
}
