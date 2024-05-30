#pragma once

#include "stubs.h"

namespace XLSP
{
#ifdef platform_xbox360

	class LSPConnection
	{
	public:
		static const int bufferSize = int(0.25f * (1024*1024)); //8192;

		enum LspGroup
		{
			LspGroup_Test,
			LspGroup_Stress,
			LspGroup_Beta,
			LspGroup_Production,

			//last
			LspGroup_Count
		};

	protected:
		enum State
		{
			NO_SERVER,
			FINDING_SERVER,
			IDLE,
			CONNECTING_SERVER,
			CONNECTING_SOCKET,
			SENDING_REQUEST,
			WAITING_FOR_RESPONSE,
		};

		bool					m_Initialized;
		LspGroup				m_LspGroup;
		State					m_State;
		unsigned int			m_ConnectAttempts;
#ifdef _XBOX
		SOCKET					m_Socket;
		XOVERLAPPED				m_xOverlapped;
		WSAOVERLAPPED			m_wsaOverlapped;
		HANDLE					m_hEnumerator;
		IN_ADDR					m_addrServer;
		IN_ADDR					m_addrServerSecure;
#endif
		void					*m_bufServerInfo;
		char					m_NetBuf[bufferSize];
		char					*m_pNextBuf;
		unsigned int			m_SendSize;
		bool					m_FailedLastRequest;
		bool					m_ExpectsResponse;
		unsigned int			m_RequestId;
		DWORD					m_ResponseLength;
		DWORD					m_StartTime;
		DWORD					m_FindServerInterval;

		void ResetServerFindTime(DWORD interval);
		void FindServer();
		void SendInternal();
		void Disconnect( bool bSuccess );
		void WaitForResponse();
		void ConnectToServer();

		void UpdateFindingServer();
		void UpdateSendingRequest();
		void UpdateWaitingForResponse();
		void UpdateConnectingServer();
		void UpdateConnectingSocket();

	public:
		LSPConnection();
		~LSPConnection();

		void									Initialize(LspGroup lspGroup);
		void									Update();
		void									Cleanup();

		unsigned int							Send( const char *pBuf, unsigned int bytes, bool expectsResponse );
		bool									IsBusy( unsigned int requestId );
		bool									GetResult( unsigned int requestId, char **ppResult, unsigned int *pSize );

		bool									IsServiceAvailable();
		
		const char								*GetServerName();
		unsigned short							GetServerPort();
	};

#endif
}
