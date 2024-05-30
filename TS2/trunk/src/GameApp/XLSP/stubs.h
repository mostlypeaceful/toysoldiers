#pragma once

#include "tUser.hpp"
#include "Log.hpp"

#include <vector>
#include <list>
#include <deque>
#include <set>

//replace all of the following stubs with your own code!!!

namespace XLSP
{

	typedef ULONGLONG SigXUID;

	const int Tag_XLSP = Sig::Log::cFlagLSP;
	const int LSPAlloc = 1;

	class MemoryMan
	{
	public:
		static void* AllocCpuMem(size_t size, const int group) { return NEW byte[size]; }
		//static void FreeCpuMem(void *p) {delete [] p;}
		static void Free(void *p) { delete [] p; }
	};

	class Random
	{
	public:
		Random( int seed ) 
			: mRandom( seed ) 
		{ }

		int Rand( ) { return mRandom.fInt( ); }

		Sig::tRandom mRandom;
	};

	class Player
	{
	public:
		DWORD GetUserIndex( )			{ sigassert( mUser ); return mUser->fLocalHwIndex( ); }
		SigXUID GetXuid( )				{ sigassert( mUser ); return mUser->fPlatformId( ); }
		bool HasMultiplayerPrivilege( ) { sigassert( mUser ); return (mUser->fIsOnlineEnabled( ) != 0); }

		Sig::tUserPtr mUser;
	};

	extern Player g_LocalPlayer;

	class PlayerManager
	{
	public:
		Player *GetLocalPlayer( ) { return &g_LocalPlayer; }
	};

	extern PlayerManager g_PlayerManager;
}
