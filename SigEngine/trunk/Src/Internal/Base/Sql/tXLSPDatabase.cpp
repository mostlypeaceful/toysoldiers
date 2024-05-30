//------------------------------------------------------------------------------
// \file tXLSPDatabase.cpp - 21 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tXLSPDatabase.hpp"
#include "tXmlFile.hpp"
#include "tUrlEncode.hpp"

namespace Sig { namespace Sql
{
	devvar( bool, Debug_Database_AlwaysFail, false );

	namespace
	{
		///
		/// \class tXLSPResponseParser
		/// \brief Used for parsing responses from the xlsp
		class tXLSPResponseParser
		{
		public:

			enum tState
			{
				cStateIncomplete,
				cStateSuccess,
				cStateFail
			};

			tXLSPResponseParser( const char * response );


			inline u32 fHttpCode( ) const { return mHttpCode; }
			inline u32 fState( ) const { return mState; }
			const tXmlFileData & fResults( ) const { return mResults; }

		private:

			u32 mState;
			u32 mHttpCode;
			tXmlFileData mResults;
		};

		//------------------------------------------------------------------------------
		tXLSPResponseParser::tXLSPResponseParser( const char * response )
			: mHttpCode( 0 )
			, mState( cStateFail )
		{
			sscanf_s( response, "HTTP/1.1 %d", &mHttpCode );
			if( mHttpCode == 200 ) // 200 is the OK code
			{
				if( const char * xmlStart = strstr( response, "<?xml" ) )
				{
					// Walk backwards to find the beginning of the xml size indicator
					const char * xmlLenStr = xmlStart - 2; // 1 for the newline, 1 for the carriage return
					while( *(xmlLenStr - 1) != '\n' && xmlLenStr > response )
						xmlLenStr -= 1;

					// Get the xml size indicator
					u32 targetXmlLen = 0;
					sscanf_s( xmlLenStr, "%x\r\n<?xml", &targetXmlLen );

					// Test that we have at least that much
					const u32 currXmlLen = strlen( xmlStart );
					if( currXmlLen < targetXmlLen )
						mState = cStateIncomplete;
					else if( mResults.fParse( xmlStart ) )
					{
						mState = cStateSuccess;
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	// tXLSPFunctionCall
	//------------------------------------------------------------------------------
	void tXLSPFunctionCall::fPushArg( const char * name, const std::string & sanArg )
	{
		mStream << "&" << name << "=" << sanArg;
	}

	//------------------------------------------------------------------------------
	void tXLSPFunctionCall::fSanitize( const char * a, u32 aLen, std::string & output )
	{
		tUrlEncode::fEncode( a, aLen, output );
	}

	//------------------------------------------------------------------------------
	// tXLSPDatabase
	//------------------------------------------------------------------------------
	tXLSPDatabase::tXLSPDatabase( u32 priorityCount )
		: mState( cStateNull )
		, mReceivedResponse( false )
		, mSendAttempts( 0 )
		, mSendAttemptsWithNoResponse( 0 )
	{
		mQueryBuffers.fNewArray( fMax( 1u, priorityCount ) );

		mResponder = make_delegate_memfn( 
				Net::tTitleServerConnection::tResponseCallback,
				tXLSPDatabase, fOnResponse );
	}

	//------------------------------------------------------------------------------
	void tXLSPDatabase::fConnect( const char * serverName, u32 serviceId, u16 port, const tFailedCallback & onFailed )
	{
		mOnFailed = onFailed;
		mConnection.fConnect( serverName, serviceId, port );
	}

	//------------------------------------------------------------------------------
	void tXLSPDatabase::fDisconnect( )
	{
		mConnection.fDisconnect( );
	}

	//------------------------------------------------------------------------------
	void tXLSPDatabase::fUpdate( )
	{
		mConnection.fUpdate( );

		// Wait till the connection is not busy before issuing requests
		if( mConnection.fIsBusyWithRequest( ) )
			return;

		// Find the first of our buffers that have items to send
		tQueryBuffer* buffer = NULL;
		{
			const u32 bufferCount = mQueryBuffers.fCount( );
			for( u32 b = 0; b < bufferCount; ++b )
			{
				if( mQueryBuffers[ b ].fNumItems( ) )
				{
					buffer = &mQueryBuffers[ b ];
					break;
				}
			}
		}

		// If we have more queries or the previous failed
		if( buffer || !mCurrentQuery.mQuery.empty( ) )
		{
			if( mCurrentQuery.mQuery.empty( ) )
			{
				buffer->fPopFirst( mCurrentQuery );
				mSendAttempts = 0;
				mSendAttemptsWithNoResponse = 0;
			}
			else
			{
				if( !mReceivedResponse )
					++mSendAttemptsWithNoResponse;

				if( mSendAttempts >= cMaxSendAttempts || 
					mSendAttemptsWithNoResponse >= cMaxNoResponseSendAttempts )
				{
					log_warning( "Failed to send query: " << mCurrentQuery.mQuery );

					if( !mOnFailed.fNull( ) )
						mOnFailed( );
				}
			}

			mReceivedResponse = false;
			++mSendAttempts;

			log_line( Log::cFlagDatabase, "Sending query: " << mCurrentQuery.mQuery );

			std::stringstream ss;
			ss	<< "GET /api_hamster.do?";

			switch(mCurrentQuery.mQueryType)
			{
			case cQueryTypeSproc: ss << "req=NativeSproc&sproc="; break;
			case cQueryTypeSecureSproc: ss << "req=NativeSSproc&sproc="; break;
			case cQueryTypeUri: ss << "req=NativeUri&uri="; break;
			case cQueryTypeSequence: ss << "req=NativeSequence&sequence="; break;
			case cQueryTypeSecureSequence: ss << "req=NativeSSequence&sequence="; break;
			default: sig_nodefault( );
			}

			ss	<< mCurrentQuery.mQuery;
			ss	<< " HTTP/1.1\r\nHost: " << mConnection.fServerName( ) << "\r\n\r\n";

			std::string finalQuery = ss.str( );

			// Send the next one
			mConnection.fSend(
				finalQuery.c_str( ), 
				finalQuery.length( ), 
				mResponder );
		}
	}

	//------------------------------------------------------------------------------
	void tXLSPDatabase::fPushQuery( u32 priority, const char * query, tQueryType queryType )
	{
		fPushQuery( priority, query, tQueryCallback( ), queryType );
	}

	//------------------------------------------------------------------------------
	void tXLSPDatabase::fPushQuery( u32 priority, const char * query, const tQueryCallback & callback, tQueryType queryType )
	{
		tQueryBuffer& buffer = mQueryBuffers[ priority ];

		buffer.fReserve( 1 );

		tQuery newQuery;
		newQuery.mQuery = query;
		newQuery.mCallback = callback;
		newQuery.mQueryType = queryType;

		buffer.fPushLast( newQuery );
	}

	//------------------------------------------------------------------------------
	void tXLSPDatabase::fClearQueries( )
	{
		for( u32 i = 0; i < mQueryBuffers.fCount( ); ++i )
		{
			mQueryBuffers[ i ].fReset( );
		}

		mCurrentQuery.mQuery.clear( );
		mReceivedResponse = false;
		mSendAttempts = 0;

		if( mConnection.fIsBusyWithRequest( ) )
			mConnection.fDisconnect( );
	}

	//------------------------------------------------------------------------------
	b32 tXLSPDatabase::fOnResponse( const void * buffer, u32 numBytes )
	{
		tXLSPResponseParser parser( (const char *)buffer );
		if( parser.fState( ) == tXLSPResponseParser::cStateIncomplete )
			return false;

		mReceivedResponse = true;

		if( parser.fState( ) == tXLSPResponseParser::cStateFail || Debug_Database_AlwaysFail )
		{
			log_warning( "XLSP Query \"" << mCurrentQuery.mQuery << "\" failed with code: " << parser.fHttpCode( ) );
			return true;
		}

		if( !mCurrentQuery.mCallback.fNull( ) )
		{
			tXmlNodeList resultNodes;
			tResultSet results;

			const b32 success = parser.fState( ) == tXLSPResponseParser::cStateSuccess;

			// Build the result set
			if( success )
			{
				parser.fResults( ).fGetRoot( ).fFindChildren( resultNodes, "Result" );
				results.mResults = &resultNodes;
			}

			mCurrentQuery.mCallback( results );
		}

		// Clear our current query so the next one gets sent
		mCurrentQuery.mQuery.clear( );

		return true;
	}

	//------------------------------------------------------------------------------
	// tXLSPDatabase::tResultSet
	//------------------------------------------------------------------------------
	u32 tXLSPDatabase::tResultSet::fResultCount( ) const
	{
		if( mResults )
		{
			tXmlNodeList * results = (tXmlNodeList *)mResults;
			return results->fCount( );
		}
		return 0;
	}

	//------------------------------------------------------------------------------
	b32 tXLSPDatabase::tResultSet::fGetResultColumn( 
		u32 result, const char * column, std::string & out ) const
	{
		sigassert( mResults && "Accessing results column without any results" );
		tXmlNodeList * results = (tXmlNodeList *)mResults;
		if( results->fCount( ) <= result )
		{
			log_warning( "Incomplete results!" );
			return false;
		}

		return (*results)[ result ].fGetAttribute( column, out );
	}

	//------------------------------------------------------------------------------
	b32 tXLSPDatabase::tResultSet::fGetResultColumn( 
		u32 result, const char * column, tStringPtr& out ) const
	{
		std::string str;
		if( fGetResultColumn( result, column, str ) )
		{
			out = tStringPtr( str );
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tXLSPDatabase::tResultSet::fGetResultColumn( 
		u32 result, const char * column, tFilePathPtr& out ) const
	{
		std::string str;
		if( fGetResultColumn( result, column, str ) )
		{
			out = tFilePathPtr( str );
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tXLSPDatabase::tResultSet::fGetResultColumn( u32 result, const char * column, b32& out ) const
	{
		std::string value;
		if( fGetResultColumn( result, column, value ) )
		{
			std::stringstream ss; ss.str( value );
			if( value.length( ) )
			{
				switch( value[ 0 ] )
				{
				case 'T':
					out = true;
					break;
				case 'F':
					out = false;
					break;
				default:
					ss >> out;
				}
			}
			else
			{
				ss >> out;
			}

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tXLSPDatabase::tResultSet::fGetResultColumn( u32 result, const char * column, GUID& out ) const
	{
		std::string value;
		if( fGetResultColumn( result, column, value ) && !value.empty( ) )
		{
			u16 data4[8] = {0};
			const s32 ret = sscanf
				( value.c_str( )
				, "%8x-%4hx-%4hx-%2hx%2hx-%2hx%2hx%2hx%2hx%2hx%2hx"
				, &out.Data1
				, &out.Data2
				, &out.Data3
				, &data4[0]
				, &data4[1]
				, &data4[2]
				, &data4[3]
				, &data4[4]
				, &data4[5]
				, &data4[6]
				, &data4[7] );

			for( u32 i = 0; i < 8; ++i )
				out.Data4[ i ] = ( byte )data4[ i ];

			sigcheckfail( ret == 11 && "Invalid GUID", return false );

			return true;
		}

		return false;
	}


}} // ::Sig::Sql
