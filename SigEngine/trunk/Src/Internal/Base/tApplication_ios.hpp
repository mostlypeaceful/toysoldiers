#if defined( platform_ios )
#ifndef __tApplication_ios__
#define __tApplication_ios__
#include "Memory/tExternalHeap.hpp"

namespace Sig
{
	struct tGameSessionInfo;

	class tGameInvite : public tRefCounter
	{
	public:
		explicit tGameInvite( u32 localHwIndex );
		const tGameSessionInfo & fSessionInfo( ) const;
	public:
		u32 mLocalHwIndex;
	};

	typedef tRefCounterPtr< tGameInvite > tGameInvitePtr;

	///
	/// \brief Provides a pc-specific implementation of tApplication.
	class base_export tApplicationPlatformBase
	{
	public:

		struct base_export tPlatformStartupOptions
		{
			tPlatformStartupOptions( )
			{
			}
		};

	public:

		tApplicationPlatformBase( );
		inline u64 fGetWindowHandleGeneric( ) const { return ( u64 )0; }
		void fShowWindow( u32 width, u32 height, s32 x = 0, s32 y = 0, b32 maximize = false );
		void fGetLocalSystemTime( u32& hour, u32& minute, u32& second );

	protected:
		tGameInvitePtr mGameInvite;
	};
}//Sig

#endif//__tApplication_ios__
#endif//#if defined( platform_ios )
