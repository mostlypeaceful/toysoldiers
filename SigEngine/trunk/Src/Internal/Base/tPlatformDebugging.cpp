#include "BasePch.hpp"
#include "tPlatformDebugging.hpp"
#include "tGameAppBase.hpp"

#if defined( platform_debugging_enabled )
namespace Sig
{

	tPlatformDebuggerCommandArguments::tPlatformDebuggerCommandArguments( byte* data, size_t size )
	{
		// Ensure a null terminator exists inside the data - otherwise we
		// know something is wrong because it delimits the command string
		size_t curPos = 0;
		char* curChar = reinterpret_cast<char*>( data );
		for ( curPos = 0; curChar[curPos] != '\0' && curPos < size; ++curPos );
		sigassert( curPos < size && "No command string was found in the argument data." );

		mCommand = std::string( reinterpret_cast<char*>( data ) );

		// The arguments begin AFTER the command string - adjust
		// starting position and size accordingly
		mArguments.fInitialize( data + curPos + 1, size - curPos - 1 );
	}

	tPlatformDebugger::tPlatformDebugger( )
	{
		log_line( 0, "Initializing Platform Debugger..." );
		sigassert( enet_initialize( ) == 0 && "Could not initialize enet!" );

		log_line( 0, "Creating Platform Debugger host...");
		mAddress.host = cHOST_IP;
		mAddress.port = cHOST_PORT;

		if( ( mHost = enet_host_create( &mAddress, cCLIENT_COUNT, cCHANNEL_COUNT, cOUTGOING_BANDWIDTH_RATE, cINCOMING_BANDWIDTH_RATE ) ) == NULL )
		{
			log_warning("Could not create Platform Debugger host! Another instance of it may be running on this computer.");
			return;
		}

		// Register a callback on ourself for performing live memory dumps
		Sig::tPlatformDebugger::tPlatformDebuggerCallback callback = make_delegate_memfn( tPlatformDebugger::tPlatformDebuggerCallback, Sig::tPlatformDebugger, fMemoryDumpCallback );
		tPlatformDebugger::fInstance( ).fRegisterCallback( std::string( "memorydump" ), callback );

		// Seed the randomizer for choosing random buttons
		srand( static_cast<u32>( time( NULL ) ) );

		// Register a callback on ourself for handling button sequences on the PC platform
		callback = make_delegate_memfn( tPlatformDebugger::tPlatformDebuggerCallback, Sig::tPlatformDebugger, fButtonSequenceCallback );
		tPlatformDebugger::fInstance( ).fRegisterCallback( std::string( "buttonsequence" ), callback );
	}

	tPlatformDebugger::~tPlatformDebugger( )
	{
		if( mHost != NULL )
		{
			log_line( 0, "Destroying Platform Debugger host...");
			enet_host_destroy( mHost );
		}

		log_line( 0, "Deinitializing Platform Debugger...");
		enet_deinitialize( );
	}

	void tPlatformDebugger::fDoWork()
	{
		if( mHost == NULL )
			return;

		ENetEvent event;

		// Process any new ENet events that have happened since the last call to tGameAppBase::fTickApp( )
		while( enet_host_service( mHost, &event, 0 ) > 0 )
		{
			char address[1024];
			enet_address_get_host_ip( &event.peer->address, address, 1024 );

			// Check for pending outgoing data. If there's data waiting to be sent,
			// check to see if we're connected to that IP. If we're not, then
			// attempt to connect to that IP. If we're already connected, send
			// the data.

			switch( event.type )
			{
			case ENET_EVENT_TYPE_CONNECT:
				{
					log_line(0, "Connected to " << address << ":" << event.peer->address.port << "." );
				}
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				{
					log_line( 0, "Received data from " << address << ":" << event.peer->address.port << ". Size: " << event.packet->dataLength << "." );

					fExecuteCallback( tPlatformDebuggerCommandArguments( event.packet->data, event.packet->dataLength ) );

					enet_packet_destroy( event.packet );
				}
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				{
					log_line(0, "Disconnected from " << address << ":" << event.peer->address.port << "." );
				}
				break;
			}
		}
	}

	void tPlatformDebugger::fRegisterCallback( std::string& command, tPlatformDebuggerCallback& callback )
	{
		mCallbackTable.fRemove( command ); // Don't allow duplicate entries!
		mCallbackTable.fInsert( command, callback );

		log_line(0, "Registered command handler: " << command );
	}

	void tPlatformDebugger::fExecuteCallback( tPlatformDebuggerCommandArguments& arguments )
	{
		// Find the registered handler for the command
		tPlatformDebuggerCallback* callback = mCallbackTable.fFind( arguments.mCommand );
		if( callback == NULL )
		{
			log_warning( "No callback registered for received command: " << arguments.mCommand );
			return;
		}

		( *callback )( arguments );
	}

	b32 tPlatformDebugger::fMemoryDumpCallback( tPlatformDebuggerCommandArguments& arguments )
	{
		{
			Memory::tHeapStacker cHeapStack( &Memory::tDebugMemoryHeap::fInstance( ) );

			tDynamicArray<byte> mMemoryDumpStorage;
			tGameAppBase::fInstance( ).fDumpMemory( "DevMenu", false, &mMemoryDumpStorage );

			ENetPacket* packet = enet_packet_create( mMemoryDumpStorage.fBegin( ), mMemoryDumpStorage.fCount( ), ENET_PACKET_FLAG_RELIABLE );
			enet_peer_send( &mHost->peers[ 0 ], 1, packet );
		}

		return true;
	}

	b32 tPlatformDebugger::fButtonSequenceCallback( tPlatformDebuggerCommandArguments& arguments )
	{
		std::string dataString( reinterpret_cast< char const* >( arguments.mArguments.fBegin( ) ), 0, arguments.mArguments.fCount( ) );

		// Split the number of frames to press the combination from the combination itself
		tGrowableArray<std::string> commandComponents;
		Sig::StringUtil::fSplit( commandComponents, dataString.c_str( ), " " );

		std::string mapString = commandComponents[ 1 ];
		for( u32 i = 2; i < commandComponents.fCount( ); ++i )
			mapString += " " + commandComponents[ i ];
		Sig::Input::tGamepad::tButton button = fMapStringToButtonConstant( mapString );

		u32 framesToHold = atoi( commandComponents[ 0 ].c_str( ) );

		for( u32 i = 0; i < framesToHold; ++i )
		{
			mControllerInputs.fPushBack( button );

			// We want to release between each press when pressing random buttons
			if( commandComponents[ 1 ] == "random" )
				mControllerInputs.fPushBack( fMapStringToButtonConstant( "release" ) );
		}

		// If we're going to "hold" buttons for a few frames, at the end let's clear
		// everything as if they were "released"
		mControllerInputs.fPushBack( fMapStringToButtonConstant( "release" ) );

		return true;
	}

	Sig::Input::tGamepad::tButton tPlatformDebugger::fMapStringToButtonConstant( std::string mapStr )
	{
		tGrowableArray<std::string> mapComponents;
		Sig::StringUtil::fSplit( mapComponents, mapStr.c_str( ), " " );

		std::string button = mapComponents[ 0 ];
		if( button == "release" )
			return 0;
		else if( button == "start" )
			return Sig::Input::tGamepad::cButtonStart;
		else if( button == "x" )
			return Sig::Input::tGamepad::cButtonX;
		else if( button == "a" )
			return Sig::Input::tGamepad::cButtonA;
		else if( button == "b" )
			return Sig::Input::tGamepad::cButtonB;
		else if( button == "d-right" )
			return Sig::Input::tGamepad::cButtonDPadRight;
		else if( button == "d-left" )
			return Sig::Input::tGamepad::cButtonDPadLeft;
		else if( button == "d-up" )
			return Sig::Input::tGamepad::cButtonDPadUp;
		else if( button == "d-down" )
			return Sig::Input::tGamepad::cButtonDPadDown;
		else if( button == "random" )
		{
			// Create a list of buttons that are allowed
			tGrowableArray<Sig::Input::tGamepad::tButton> allowedButtons;
			for( u32 i = 1; i < mapComponents.fCount( ); ++i )
				allowedButtons.fPushBack( fMapStringToButtonConstant( mapComponents[ i ] ) );

			u32 index = rand( ) % allowedButtons.fCount( );

			return allowedButtons[ index ];
		}

		return -1;
	}

	void tPlatformDebugger::fExecuteButtonInterceptsForFrame( )
	{
		if( mControllerInputs.fCount( ) <= 0 )
			return;

		Sig::Input::tGamepad::tButton button = mControllerInputs.fPopFront( );

		// Get access to the raw bits of the current gamepad state data
		Sig::Input::tGamepad& gamepad = const_cast<Sig::Input::tGamepad&> ( tApplication::fInstance( ).fLocalUsers( )[ 0 ]->fRawGamepad( ) );
		Sig::Input::tGamepad::tStateData& stateData =  const_cast<Sig::Input::tGamepad::tStateData&> ( gamepad.fGetStateData( ) );

		// Set the buttons for the current frame
		stateData.mButtonsDown = fSetBits( stateData.mButtonsDown, button );
	}
}
#endif//#if defined( platform_debugging_enabled )

#ifdef target_tools
namespace Sig
{
	b32 tPlatformDebuggingTools::fGetMemoryDump( const std::string& ipAddress, tDynamicArray<byte>& result )
	{
		b32 status = true;

		log_line( 0, "Initializing Enet..." );
		sigassert( enet_initialize( ) == 0 && "Could not initialize enet!" );

		log_line( 0, "Creating client...");
		ENetHost* client = enet_host_create( NULL, 1, 2, 0, 0 );
		sigassert( client != NULL && "Could not create client!");

		log_line( 0, "Connecting to host...");
		ENetAddress address;
		sigassert( enet_address_set_host( &address, ipAddress.c_str( ) ) >= 0 );
		address.port = 6666;
		ENetPeer* peer = enet_host_connect( client, &address, 200, 1234 );

		b32 waitingForConnection = true;

		while( result.fCount( ) == 0 )
		{
			ENetEvent event;
			while( enet_host_service( client, &event, 3000 ) > 0 )
			{
				char address[1024];
				enet_address_get_host_ip( &event.peer->address, address, 1024 );

				switch( event.type )
				{
				case ENET_EVENT_TYPE_CONNECT:
					{
						log_line(0, "Connected to " << address << ":" << event.peer->address.port << "." );

						waitingForConnection = false;

						ENetPacket* packet = enet_packet_create( "memorydump\0", strlen( "memorydump" ) + 1, ENET_PACKET_FLAG_RELIABLE );
						enet_peer_send( peer, 0, packet );
					}
					break;

				case ENET_EVENT_TYPE_RECEIVE:
					{
						log_line( 0, "Received data from " << address << ":" << event.peer->address.port << ".");

						size_t size = event.packet->dataLength;
						byte* data = event.packet->data;

						result.fInitialize( data, size );

						log_line( 0, "Disconnecting from host...");
						enet_peer_disconnect( peer, 0 );
					}
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					{
						log_line(0, "Disconnected from " << address << ":" << event.peer->address.port << "." );
					}
					break;
				}
			}

			if( waitingForConnection )
			{
				status = false;
				break;
			}
		}

		log_line( 0, "Destroying client...");
		enet_host_destroy( client );

		log_line( 0, "Deinitializing ENet...");
		enet_deinitialize( );

		return status;
	}
}

#else

namespace Sig
{
	void fShutup_tPlatformDebugging_Linker_Warnings( )
	{
	}
}

#endif
