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

		Memory::tExternalHeap& fNoCacheHeap( ) { return mNoCacheHeap; }
		Memory::tExternalHeap& fVramHeap( ) { return mMainVramHeap; }

	protected:
		Memory::tExternalHeap mNoCacheHeap;
		Memory::tExternalHeap mMainVramHeap;

		HANDLE mEventListener;
		tGameInvitePtr mGameInvite;
	};
}//Sig

#endif//__tApplication_xbox360__
#endif//#if defined( platform_xbox360 )
