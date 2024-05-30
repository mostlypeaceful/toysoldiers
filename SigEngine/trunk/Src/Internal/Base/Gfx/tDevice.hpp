#ifndef __tDevice__
#define __tDevice__
#include "tRenderState.hpp"

namespace Sig { namespace Gfx
{
	class tDevice;
	class tDeviceResource;

	define_smart_ptr( base_export, tRefCounterPtr, tDevice );

	enum tVsync
	{
		VSYNC_NONE,
		VSYNC_60HZ,
		VSYNC_30HZ
	};

	struct base_export tDeviceCaps
	{
		u32 mMaxAnisotropy;
	};

	struct base_export tDisplayMode
	{
		u32 mBackBufferWidth;
		u32 mBackBufferHeight;
		u32 mMultiSamplePower;
		tDisplayMode( ) { fZeroOut( this ); }
		tDisplayMode( u32 bbWidth, u32 bbHeight, u32 msPower = 0 ) : mBackBufferWidth( bbWidth ), mBackBufferHeight( bbHeight ), mMultiSamplePower( msPower ) { }
		inline b32 operator==( const tDisplayMode& other ) const { return fMemCmp( this, &other, sizeof( other ) ) == 0; }
		inline b32 operator!=( const tDisplayMode& other ) const { return !operator==( other ); }
	};

	class base_export tDisplayModeList : public tGrowableArray< tDisplayMode >
	{
	public:
		u32 fFindClosestMatch( const tDisplayMode& target ) const;
	};

	class base_export tDevicePlatformBase : public tUncopyable, public tRefCounter
	{
	public:
		typedef Sig::byte* tPlatformHandle;

	protected:
		tDeviceCaps					mCaps;
		tRenderState				mLastAppliedRenderState;
		b8							mLastRenderStateInvalid;
		b8							mSingleScreenDevice;
		b8							mBorrowedDevice;
		b8							pad;
		tGrowableArray<tDeviceResource*> mDeviceResources;

	public:
		static void					fSetDefaultDevice( const tDevicePtr& defaultDevice );
		static const tDevicePtr&	fGetDefaultDevice( );
		static void					fEnumerateDisplayModes( tDisplayModeList& displayModes );

	protected:
		tDevicePlatformBase( );
		~tDevicePlatformBase( );

	public:
		inline const tDeviceCaps&	fCaps( ) const { return mCaps; }
		inline void					fSetLastAppliedRenderState( const tRenderState& rs ) { mLastAppliedRenderState = rs; }
		inline const tRenderState&	fLastAppliedRenderState( ) const { return mLastAppliedRenderState; }
		inline void					fInvalidateLastRenderState( b32 invalid = true ) { mLastRenderStateInvalid = invalid; }
		inline b32					fLastRenderStateInvalid( ) const { return mLastRenderStateInvalid; }
		inline b32					fSingleScreenDevice( ) const { return mSingleScreenDevice; }
		void						fAddDeviceResource( tDeviceResource* dr );
		void						fRemoveDeviceResource( tDeviceResource* dr );
		void						fSetShaderGPRAllocation( u32 vsGprs, u32 psGprs );
		void						fSetScissorRect( const Math::tRect* rect );
		void						fSetGammaRamp( f32 gamma );
	};
}}

#if defined( platform_pcdx9 )
#	include "tDevice_pcdx9.hpp"
#elif defined( platform_xbox360 )
#	include "tDevice_xbox360.hpp"
#elif defined( platform_ios )
#	include "tDevice_ios.hpp"
#else
#	error Invalid platform for tDevice defined!
#endif

#endif//__tDevice__
