#if defined( platform_xbox360 )
#ifndef __tApplication_xbox360__
#define __tApplication_xbox360__
#include "Memory/tExternalHeap.hpp"

namespace Sig
{
	struct tGameSessionInfo;

	class tGameInvite : public tRefCounter, public XINVITE_INFO
	{
	public:
		explicit tGameInvite( u32 localHwIndex, const XINVITE_INFO& xinvite );
		const tGameSessionInfo & fSessionInfo( ) const;
	public:
		u32 mLocalHwIndex;
	};

	typedef tRefCounterPtr< tGameInvite > tGameInvitePtr;

	///
	/// \brief Provides a xbox-specific implementation of tApplication.
	class base_export tApplicationPlatformBase
	{
	public:
		struct base_export tPlatformStartupOptions
		{
		};

	public:
		static LPVOID	fXMemAlloc( SIZE_T dwSize, DWORD dwAllocAttributes );
		static VOID		fXMemFree( PVOID pAddress, DWORD dwAllocAttributes );
		static SIZE_T	fXMemSize( PVOID pAddress, DWORD dwAllocAttributes );

	public:

		tApplicationPlatformBase( );
		inline u64 fGetWindowHandleGeneric( ) const { return ( u64 )0; }
		void fShowWindow( u32 width, u32 height, s32 x = 0, s32 y = 0, b32 maximize = false );
		void fGetLocalSystemTime( u32& hour, u32& minute, u32& second );

		const Memory::tExternalHeap& fNoCacheHeap( ) const { return mNoCacheHeap; }
		const Memory::tExternalHeap& fVramHeap( ) const { return mVramHeap; }

	protected:
		Memory::tExternalHeap mNoCacheHeap;
		Memory::tExternalHeap mVramHeap;
		HANDLE mEventListener;
		tGameInvitePtr mGameInvite;

		// Data specifically for xbox demo disks.
		LD_DEMO*	mDemoData;
		DWORD		mLaunchDataSize;
	};


#define implement_application( derivedAppClass ) \
	sig_static_assert( __ClassRegistration__ ); \
	int __cdecl main( ) \
	{ \
		derivedAppClass* theApp = NEW derivedAppClass( ); \
		const int result = theApp->fRun( ::Sig::tCmdLineBuffer::fConvert( GetCommandLine( ) ) ); \
		delete theApp; \
		return result; \
	} \
	LPVOID WINAPI XMemAlloc( SIZE_T dwSize, DWORD dwAllocAttributes ) { return ::Sig::tApplicationPlatformBase::fXMemAlloc( dwSize, dwAllocAttributes ); } \
	VOID WINAPI XMemFree( PVOID pAddress, DWORD dwAllocAttributes ) { ::Sig::tApplicationPlatformBase::fXMemFree( pAddress, dwAllocAttributes ); } \
	SIZE_T WINAPI XMemSize( PVOID pAddress, DWORD dwAllocAttributes ) { return ::Sig::tApplicationPlatformBase::fXMemSize( pAddress, dwAllocAttributes ); }


}

#endif//__tApplication_xbox360__
#endif//#if defined( platform_xbox360 )
