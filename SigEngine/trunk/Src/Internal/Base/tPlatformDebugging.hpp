#ifndef __tPlatformDebugging__
#define __tPlatformDebugging__

#include "BasePch.hpp"
#include "enet/enet.h"
#include "Threads/tWorkerThread.hpp"
#include "tUser.hpp"

#if defined( build_internal ) || defined( build_debug )
	#define platform_debugging_enabled
#endif

namespace Sig
{
#if defined( platform_debugging_enabled )
	struct tPlatformDebuggerCommandArguments
	{
	public:
		tPlatformDebuggerCommandArguments( byte* data, size_t size );

		std::string mCommand;
		tDynamicArray<byte> mArguments;
		tDynamicArray<byte> mResponse;
	};

	/*
		This is the client side object.
	*/
	class tPlatformDebugger
	{
		declare_singleton_define_own_ctor_dtor( tPlatformDebugger );

	public:
		typedef tDelegate<b32 ( tPlatformDebuggerCommandArguments& arguments )> tPlatformDebuggerCallback;

		tPlatformDebugger( );
		~tPlatformDebugger( );

		void fRegisterCallback( std::string& command, tPlatformDebuggerCallback& callback );
		
		void fDoWork( ); // Called once per frame by tGameAppBase::fTickApp( )

		void fExecuteButtonInterceptsForFrame( );
		
	private:
		const static enet_uint32 cHOST_IP = ENET_HOST_ANY;
		const static enet_uint32 cHOST_PORT = 6666;
		const static size_t cCLIENT_COUNT = 32;
		const static size_t cCHANNEL_COUNT = 2;
		const static size_t cOUTGOING_BANDWIDTH_RATE = 0;
		const static size_t cINCOMING_BANDWIDTH_RATE = 0;
		const static size_t cMAX_COMMAND_NAME_LENGTH = 1024;

		ENetAddress mAddress;
		ENetHost* mHost;

		tHashTable< std::string, tPlatformDebuggerCallback > mCallbackTable;

		void fExecuteCallback( tPlatformDebuggerCommandArguments& arguments );

		b32 fMemoryDumpCallback( tPlatformDebuggerCommandArguments& arguments );

		tGrowableArray<Sig::Input::tGamepad::tButton> mControllerInputs;

		b32 fButtonSequenceCallback( tPlatformDebuggerCommandArguments& arguments );
		Sig::Input::tGamepad::tButton fMapStringToButtonConstant( std::string mapStr );
	};
#endif//#if defined( platform_debugging_enabled )

#ifdef target_tools
	/*
		This is a tools side server object
	*/
	class base_export tPlatformDebuggingTools
	{
	public:
		static b32 fGetMemoryDump( const std::string& ipAddress, tDynamicArray<byte>& result );
	};
#endif
}


#endif//__tPlatformDebugging__
