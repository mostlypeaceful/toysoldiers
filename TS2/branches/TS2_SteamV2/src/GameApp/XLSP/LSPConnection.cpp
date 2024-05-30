#include "GameAppPch.hpp"
#include "LSPConnection.h"

using namespace Sig;

namespace XLSP
{
#ifdef platform_xbox360
	//Define this to connect directly to the server
	//Useful for PartnerNet outages.
	//DO NOT SHIP WITH THIS DEFINED!
	//#define DEBUG_DIRECT_CONNECT

	#ifdef DEBUG_DIRECT_CONNECT
	static const char *DIRECT_CONNECT_TO_IP = "131.107.244.35";
	static const unsigned short SERVER_PORT = 80;
	#else
	static const unsigned short TEST_SERVER_PORT = 80; //from sgconfig.ini "Server" entry on the sg server vm
	static const unsigned short STRESS_SERVER_PORT = 80; //from sgconfig.ini "Server" entry on the sg server vm
	static const unsigned short BETA_SERVER_PORT = 80; //from sgconfig.ini "Server" entry on the sg server vm
	static const unsigned short PRODUCTION_SERVER_PORT = 80; //from sgconfig.ini "Server" entry on the sg server vm
	#endif

	#ifdef _XBOX
	static const char *TEST_SERVER_NAME = "ToySoldiers2_Test";
	static const char *STRESS_SERVER_NAME = "ToySoldiers2_Stress";
	static const char *BETA_SERVER_NAME = "ToySoldiers2_Beta";
	static const char *PRODUCTION_SERVER_NAME = "ToySoldiers2_Production";
	static const unsigned int SERVER_SERVICE_ID = 0x4D5308CF; //from sgconfig.ini "Service" entry on the sg server vm
	static const DWORD MAX_SERVERS = 10; 
	static const int HTTP_OK_CODE = 200;
	static const unsigned int TIMEOUT_THRESHOLD = 8000;
	static const unsigned int INITIAL_FIND_SERVER_INTERVAL = 5 * 1000; //5 seconds for first retry
	static const unsigned int FIND_SERVER_INTERVAL = 5 * 60 * 1000; //5 minutes for subsequent retries
	static const unsigned int MAX_CONNECT_ATTEMPTS = 5;
	#endif


	LSPConnection::LSPConnection() :
		m_Initialized( false ),
		m_LspGroup(LspGroup_Production),
		m_bufServerInfo( NULL ),
		m_hEnumerator( NULL ),
		m_State( NO_SERVER ),
		m_Socket( NULL ),
		m_SendSize( 0 ),
		m_FailedLastRequest( false ),
		m_ExpectsResponse( false ),
		m_ResponseLength( 0 ),
		m_pNextBuf( NULL ),
		m_StartTime( 0 ),
		m_FindServerInterval(INITIAL_FIND_SERVER_INTERVAL),
		m_RequestId( 0 )
	{
		m_addrServer.s_addr = 0;
		m_addrServerSecure.s_addr = 0;
	}

	LSPConnection::~LSPConnection()
	{
		Cleanup();
	}

	void LSPConnection::Initialize(LspGroup lspGroup)
	{
	#ifdef _XBOX
		m_LspGroup = lspGroup;
		m_Initialized = true;
		m_State = NO_SERVER;
		m_ConnectAttempts = 0;
		m_FailedLastRequest = false;
		m_ExpectsResponse = false;
		m_SendSize = 0;
		m_ResponseLength = 0;
		FindServer();
		m_FindServerInterval = INITIAL_FIND_SERVER_INTERVAL;
	#endif
	}

	void LSPConnection::Cleanup()
	{	
	#ifdef _XBOX
		if( m_hEnumerator )
		{
			if( !XHasOverlappedIoCompleted( &m_xOverlapped ) )
			{
				XCancelOverlapped( &m_xOverlapped );
			}
			CloseHandle( m_hEnumerator );
			m_hEnumerator = NULL;
		}
		if( m_bufServerInfo )
		{
			MemoryMan::Free( m_bufServerInfo );
			m_bufServerInfo = NULL;
		}
		m_addrServer.s_addr = 0;
		Disconnect( true );
		m_Initialized = false;
	#endif //_XBOX
	}

	void LSPConnection::Update()
	{
		//This is called unconditionally.  Bail if we're not initialized
		if( !m_Initialized )
		{
			return;
		}

	#ifdef _XBOX
		switch( m_State )
		{
		case NO_SERVER:
			if( m_SendSize > 0 || GetTickCount() - m_StartTime > m_FindServerInterval )
			{
				FindServer();
			}
			break;

		case FINDING_SERVER:
			UpdateFindingServer();
			break;

		case IDLE:
			if( m_SendSize > 0 )
			{
				ConnectToServer();
			}
			break;

		case CONNECTING_SERVER:
			UpdateConnectingServer();
			break;

		case CONNECTING_SOCKET:
			UpdateConnectingSocket();
			break;

		case SENDING_REQUEST:
			UpdateSendingRequest();
			break;

		case WAITING_FOR_RESPONSE:
			UpdateWaitingForResponse();
			break;

		default:
			//Eek!
			log_assert( 0, "LSPConnection in unknown state %d. " << m_State );
		}

	#endif //_XBOX
	}

	void LSPConnection::FindServer()
	{
	#ifdef _XBOX
		//Kick off an enumeration for custom servers
		m_addrServer.s_addr = 0;
		m_FindServerInterval = FIND_SERVER_INTERVAL;

	#ifdef DEBUG_DIRECT_CONNECT
		m_addrServer.s_addr = inet_addr( DIRECT_CONNECT_TO_IP );
		m_State = IDLE;
	#else
		DWORD dwBufSize = 0;
		DWORD dwResult = XTitleServerCreateEnumerator( NULL, MAX_SERVERS, &dwBufSize, &m_hEnumerator );
		if( dwResult == ERROR_SUCCESS )
		{
			m_bufServerInfo = MemoryMan::AllocCpuMem(dwBufSize, LSPAlloc);
			ZeroMemory( &m_xOverlapped, sizeof( m_xOverlapped ) );
			dwResult = XEnumerate( m_hEnumerator, m_bufServerInfo, dwBufSize, NULL, &m_xOverlapped );
			if( dwResult == ERROR_IO_PENDING )
			{
				m_State = FINDING_SERVER;
			}
		}

		//Something went wrong.  Give up for now.
		if( m_State != FINDING_SERVER )
		{
			if( m_bufServerInfo )
			{
				MemoryMan::Free( m_bufServerInfo );
				m_bufServerInfo = NULL;
			}
			if( m_hEnumerator != NULL )
			{
				CloseHandle( m_hEnumerator );
				m_hEnumerator = NULL;
			}
			Disconnect( false );
		}
	#endif //DEBUG_DIRECT_CONNECT
	#endif //_XBOX
	}

	const char *LSPConnection::GetServerName()
	{
		static const char *ServerNames[] = 
		{
			TEST_SERVER_NAME,
			STRESS_SERVER_NAME,
			BETA_SERVER_NAME,
			PRODUCTION_SERVER_NAME,
		};

		return ServerNames[m_LspGroup];
	}

	unsigned short LSPConnection::GetServerPort()
	{
		const unsigned short ServerPorts[] =
		{
			TEST_SERVER_PORT,
			STRESS_SERVER_PORT,
			BETA_SERVER_PORT,
			PRODUCTION_SERVER_PORT,
		};

		return ServerPorts[m_LspGroup];
	}

	void LSPConnection::UpdateFindingServer()
	{
	#ifdef _XBOX
		sigassert( m_bufServerInfo );
		if( XHasOverlappedIoCompleted( &m_xOverlapped ) )
		{
			HRESULT hr = XGetOverlappedExtendedError( &m_xOverlapped );
			if( XGetOverlappedExtendedError( &m_xOverlapped ) == ERROR_SUCCESS && m_xOverlapped.InternalHigh > 0 )
			{
				XTITLE_SERVER_INFO *pServers = static_cast<XTITLE_SERVER_INFO*>( m_bufServerInfo );
				std::vector<int> serverIndices;

				//find servers that match our lsp service name
				const char *serverName = GetServerName();
				size_t nameLen = strlen(serverName);
				for( unsigned int i = 0; i < m_xOverlapped.InternalHigh; ++i )
				{
					if( _strnicmp( pServers[i].szServerInfo, GetServerName(), nameLen) == 0)
					{
						serverIndices.push_back(i);
						log_line( 0, "LSPConnection::UpdateFindingServer() found lsp server " << pServers[i].inaServer.s_net << " " << pServers[i].inaServer.s_host << " " << pServers[i].inaServer.s_lh << " " << pServers[i].inaServer.s_impno);
					}
				}

				//if we found at least 1, choose randomly which one to connect to
				if (serverIndices.size() > 0)
				{
					Random rand(GetTickCount());
					m_addrServer = pServers[serverIndices[rand.Rand() % serverIndices.size()]].inaServer;
					log_line( 0, "LSPConnection::UpdateFindingServer() lsp servers found: %d, connecting to " << " " << serverIndices.size() << " " << m_addrServer.s_net << " " << m_addrServer.s_host << " " << m_addrServer.s_lh << " " << m_addrServer.s_impno);
				}

				if( m_addrServer.s_addr != 0 )
				{
					m_State = IDLE;
				}
				else
				{
					Disconnect( false );
				}
			}
			else
			{
				Disconnect( false );
			}

			MemoryMan::Free( m_bufServerInfo );
			m_bufServerInfo = NULL;
			CloseHandle( m_hEnumerator );
			m_hEnumerator = NULL;
		}
	#endif //_XBOX
	}

	void LSPConnection::ConnectToServer()
	{
	#ifdef _XBOX
	#ifdef DEBUG_DIRECT_CONNECT
		m_addrServerSecure = m_addrServer;
		m_State = CONNECTING_SERVER;
	#else
		//Transform the raw IP into a secure address that we can connect on
		if( XNetServerToInAddr( m_addrServer, SERVER_SERVICE_ID, &m_addrServerSecure ) == ERROR_SUCCESS )
		{
			if( XNetConnect( m_addrServerSecure ) == ERROR_SUCCESS )
			{
				m_State = CONNECTING_SERVER;
			}
			else
			{
				XNetUnregisterInAddr( m_addrServerSecure );
			}
		}

		//Error handling
		if( m_State != CONNECTING_SERVER )
		{
			//Support retrying the connection a few times for better results
			//when the server is busy.
			m_addrServer.s_addr = 0;
			if( ++m_ConnectAttempts >= MAX_CONNECT_ATTEMPTS )
			{
				Disconnect( false );
			}

			m_State = NO_SERVER;
		}
	#endif //DEBUG_DIRECT_CONNECT
	#endif //_XBOX
	}

	void LSPConnection::UpdateConnectingServer()
	{
	#ifdef _XBOX
	#ifdef DEBUG_DIRECT_CONNECT
		DWORD dwResult = XNET_CONNECT_STATUS_CONNECTED;
	#else
		DWORD dwResult = XNetGetConnectStatus( m_addrServerSecure );
	#endif
		switch( dwResult )
		{
		case XNET_CONNECT_STATUS_PENDING:
			//Still waiting
			break;

		case XNET_CONNECT_STATUS_CONNECTED:
			{
				//Now that we've established communication, open up a socket
				m_Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

				//Default state is blocking, which would be bad
				u_long nonBlocking = 1;
				ioctlsocket( m_Socket, FIONBIO, &nonBlocking );

				sockaddr_in sockAddr;
				ZeroMemory( &sockAddr, sizeof(sockAddr) );
				sockAddr.sin_family = AF_INET;
				sockAddr.sin_port = htons(GetServerPort());
				sockAddr.sin_addr = m_addrServerSecure;

				//connect always returns SOCKET_ERROR here because it's non-blocking,
				//so error handling is not appropriate: we'll do that in the update.
				connect( m_Socket, reinterpret_cast<sockaddr*>( &sockAddr ), sizeof( sockAddr ) );
				m_StartTime = GetTickCount();
				m_State = CONNECTING_SOCKET;
			}
			break;

		case XNET_CONNECT_STATUS_IDLE:
		case XNET_CONNECT_STATUS_LOST:
			{
				Disconnect( false );
			}
			break;
		}
	#endif //_XBOX
	}

	void LSPConnection::UpdateConnectingSocket()
	{
	#ifdef _XBOX
		fd_set sockWriteQuery;
		FD_ZERO( &sockWriteQuery );
		FD_SET( m_Socket, &sockWriteQuery );

		fd_set sockErrorQuery;
		FD_ZERO( &sockErrorQuery );
		FD_SET( m_Socket, &sockErrorQuery );

		//Don't wait
		timeval waitTime;
		waitTime.tv_sec = 0;
		waitTime.tv_usec = 0;

		bool bFailed = false;
		int selectResult = select( 0, NULL, &sockWriteQuery, &sockErrorQuery, &waitTime );
		if( selectResult == 0 )
		{
			//0 indicates time out, i.e. we're still waiting
			if( GetTickCount() - m_StartTime > TIMEOUT_THRESHOLD )
			{
				bFailed = true;
			}
		}
		else if( selectResult != SOCKET_ERROR )
		{
			//connect completed.  Check our socket is ready.
			//A failed connect will still set the socket in sockWriteQuery, so we need
			//to check sockErrorQuery too.
			int errorSet = FD_ISSET( m_Socket, &sockErrorQuery );
			int writeSet = FD_ISSET( m_Socket, &sockWriteQuery );
			if( !errorSet && writeSet)
			{
				SendInternal();
			}
			else
			{
				bFailed = true;
			}
		}
		else
		{
			//connect failed
			//int error = WSAGetLastError(
			bFailed = true;
		}

		if( bFailed )
		{
			Disconnect( false );
		}
	#endif //_XBOX
	}

	void LSPConnection::UpdateSendingRequest()
	{
	#ifdef _XBOX
		DWORD dwBytesTransferred = 0;
		DWORD dwFlags = 0;
		if( WSAGetOverlappedResult( m_Socket, &m_wsaOverlapped, &dwBytesTransferred, FALSE, &dwFlags ) )
		{
			sigassert( dwBytesTransferred <= m_SendSize );
			m_SendSize -= dwBytesTransferred;
			if( m_SendSize > 0 )
			{
				m_pNextBuf += dwBytesTransferred;
				SendInternal();
			}
			else
			{
				//Yay, successfully sent our request.  Wait for a response.
				m_pNextBuf = m_NetBuf;
				ZeroMemory( m_NetBuf, _countof( m_NetBuf ) );
				if( m_ExpectsResponse )
				{
					WaitForResponse();
				}
				else
				{
					Disconnect( true );
				}
			}
		}
		else
		{
			if( WSAGetLastError() != WSA_IO_INCOMPLETE || GetTickCount() - m_StartTime > TIMEOUT_THRESHOLD )
			{
				//Failed the send.  Possibly retry
				Disconnect( false );
			}
		}
	#endif //_XBOX
	}

	void LSPConnection::WaitForResponse()
	{
	#ifdef _XBOX
		//-1 to guarantee that we've always got space for a terminating character
		WSABUF wsaBuf;
		wsaBuf.len = _countof( m_NetBuf ) - 1;
		wsaBuf.buf = m_NetBuf;

		DWORD dwFlags = 0;
		ZeroMemory( &m_wsaOverlapped, sizeof( m_wsaOverlapped ) );
		int result = WSARecv( m_Socket, &wsaBuf, 1, &m_ResponseLength, &dwFlags, &m_wsaOverlapped, NULL );
		if( result == 0 )
		{
			Disconnect( true );
		}
		else
		{
			m_State = WAITING_FOR_RESPONSE;
			m_StartTime = GetTickCount();
			int error = WSAGetLastError();
			if( error != WSA_IO_PENDING )
			{
				// Eek!
				Disconnect( false );
			}
		}
	#endif //_XBOX
	}

	void LSPConnection::UpdateWaitingForResponse()
	{
	#ifdef _XBOX
		DWORD dwFlags = 0;
		if( WSAGetOverlappedResult( m_Socket, &m_wsaOverlapped, &m_ResponseLength, FALSE, &dwFlags ) )
		{
			Disconnect( true );
		}
		else
		{
			if( WSAGetLastError() != WSA_IO_INCOMPLETE || GetTickCount() - m_StartTime > TIMEOUT_THRESHOLD )
			{
				//Failed the recieve call.
				Disconnect( false );
			}
		}
	#endif //_XBOX
	}

	void LSPConnection::SendInternal()
	{
	#ifdef _XBOX
		m_FailedLastRequest = false;

		WSABUF wsaBuf;
		wsaBuf.len = m_SendSize;
		wsaBuf.buf = m_pNextBuf;

		DWORD bytesSent = 0;
		ZeroMemory( &m_wsaOverlapped, sizeof( m_wsaOverlapped ) );
		int result = WSASend( m_Socket, &wsaBuf, 1, &bytesSent, 0, &m_wsaOverlapped, NULL );
		if( result == 0 )
		{
			sigassert( bytesSent <= m_SendSize );
			m_SendSize -= bytesSent;
			if( m_SendSize > 0 )
			{
				m_pNextBuf += bytesSent;
				SendInternal();
			}
			else
			{
				m_pNextBuf = NULL;
				if( m_ExpectsResponse )
				{
					m_pNextBuf = m_NetBuf;
					ZeroMemory( m_NetBuf, _countof( m_NetBuf ) );
					WaitForResponse();
				}
				else
				{
					Disconnect( true );
				}
			}
		}
		else
		{
			m_State = SENDING_REQUEST;
			m_StartTime = GetTickCount();
			int error = WSAGetLastError();
			if( error != WSA_IO_PENDING )
			{
				// Eek!
				Disconnect( false );
			}
		}
	#endif //_XBOX
	}

	void LSPConnection::Disconnect( bool bSuccess )
	{
	#ifdef _XBOX
		if( m_Socket != NULL )
		{
			WSACancelOverlappedIO( m_Socket );
			closesocket( m_Socket );
			m_Socket = NULL;
		}
		if( m_addrServerSecure.s_addr != 0 )
		{
	#ifndef DEBUG_DIRECT_CONNECT
			//Release our secure address.  We'll get a different one next
			//time we need it.
			XNetUnregisterInAddr( m_addrServerSecure );
	#endif
			m_addrServerSecure.s_addr = 0;
		}

		m_FailedLastRequest = !bSuccess;
		if( m_FailedLastRequest )
		{
			//Force a full reconnect next time - it might help resolve the error
			m_addrServer.s_addr = 0;
			m_StartTime = GetTickCount();
		}

		m_ConnectAttempts = 0;
		m_SendSize = 0;
		m_State = m_addrServer.s_addr != 0 ? IDLE : NO_SERVER;
	#endif //_XBOX
	}

	unsigned int LSPConnection::Send( const char *pBuf, unsigned int bytes, bool expectsResponse )
	{
		Disconnect( true );

		sigassert( bytes <= sizeof( m_NetBuf ) );
		if (bytes > sizeof(m_NetBuf))
			bytes = sizeof(m_NetBuf);
		m_ResponseLength = 0;
		m_SendSize = bytes;
		memcpy( m_NetBuf, pBuf, m_SendSize );
		m_pNextBuf = m_NetBuf;
		m_ExpectsResponse = expectsResponse;
		++m_RequestId;
		return m_RequestId;
	}

	bool LSPConnection::IsBusy( unsigned int requestID )
	{
		return m_RequestId == requestID && ( m_State > IDLE || m_SendSize != 0 );
	}

	bool LSPConnection::GetResult( unsigned int requestID, char **ppResult, unsigned int *pSize )
	{
		sigassert( ppResult );
		sigassert( pSize );

		if( IsBusy( requestID ) || m_RequestId != requestID || m_FailedLastRequest )
		{
			return false;
		}

		//Guarantee a terminating character
		m_NetBuf[ _countof( m_NetBuf ) - 1] = 0;
		if (ppResult)
			*ppResult= m_NetBuf;
		if (pSize)
			*pSize = m_ResponseLength;
		return true;
	}

	bool LSPConnection::IsServiceAvailable()
	{
		// Give the LSP the benefit of the doubt when we're trying a send.
		return ( m_State != NO_SERVER || m_SendSize != 0 );
	}
#endif
}
