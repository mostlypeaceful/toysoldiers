//------------------------------------------------------------------------------
// \file tPostgreDatabase.cpp - 21 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tPostgreDatabase.hpp"

namespace Sig { namespace Sql
{
	namespace
	{
		static const u32 DATABASE_VERSION = ENET_HOST_TO_NET_32( ((3 << 16) | 0) );

		struct tPacket
		{
			char mBuffer[4096];
			u32 mBufferPos;

			b32 mTagged;

			void fResetForSend(unsigned char tag = 0)
			{
				mBufferPos = 0;

				if( tag )
				{
					mTagged = true;
					fPush( tag );
				}
				else mTagged = false;

				fPush( mBufferPos );
			}
			template<class T>
			void fPush( const T& t )
			{
				sigassert( mBufferPos + sizeof( T ) < sizeof( mBuffer ) );
				memcpy( mBuffer + mBufferPos, &t, sizeof( T ) );
				mBufferPos += sizeof( T );
			}
			template<>
			void fPush( const std::string& t )
			{
				sigassert( mBufferPos + t.length( ) + 1 < sizeof( mBuffer ) );
				memcpy( mBuffer + mBufferPos, t.c_str( ), t.length( ) + 1 );
				mBufferPos += t.length( ) + 1;
			}
			template<>
			void fPush( const tStringPtr& t )
			{
				sigassert( mBufferPos + t.fLength( ) + 1 < sizeof( mBuffer ) );
				memcpy( mBuffer + mBufferPos, t.fCStr( ), t.fLength( ) + 1 );
				mBufferPos += t.fLength( ) + 1;
			}
			int fNetSend( const ENetSocket& sock, const ENetAddress & addr)
			{
				fPrepareForSend( );

				ENetBuffer buffer = { mBufferPos, mBuffer };
				const int sentBytes = enet_socket_send( sock, &addr, &buffer, 1 );

				if( sentBytes == mBufferPos )
					return 0;

				log_warning( "sendto failed. " << sentBytes << " == " << mBufferPos << " Error Code: " << WSAGetLastError( ) );
				return ~0; //ERROR
			}
			void fPrepareForSend( )
			{
				//null terminate
				fPush( (char)0 );

				//write total size to start of buffer
				if( mTagged )
				{
					//special-case code for tag 'cause for some reason an extra null isn't added like the other command...
					mBufferPos -= 2;
					memcpy( mBuffer + 1, &mBufferPos, sizeof( mBufferPos ) );
					++mBufferPos;
				}
				else
					memcpy( mBuffer, &mBufferPos, sizeof( mBufferPos ) );
			}
			int fNetRecv( const ENetSocket& sock )
			{
				ENetBuffer buffer = { sizeof( mBuffer ), mBuffer };
				const int recvBytes = enet_socket_receive( sock, NULL, &buffer, 1 );
				mBufferPos = recvBytes;

				//we got a problem. too much data!
				if( recvBytes == sizeof( mBuffer ) )
					log_warning( "{DB} we just recieved more data than we can handle! " << recvBytes );

				return recvBytes;
			}
		};

		struct tPacketReader
		{
			const tPacket& mPacket;
			u32 mPos;

			tPacketReader( const tPacket& packet )
				: mPacket( packet )
				, mPos( 0 )
			{}
			void fIgnore(u32 numBytes)
			{
				mPos += numBytes;
				sigassert( mPos <= mPacket.mBufferPos );
			}
			char fReadChar()
			{
				mPos += sizeof(char);
				sigassert( mPos <= mPacket.mBufferPos );
				return mPacket.mBuffer[ mPos - sizeof(char) ];
			}
			u32 fReadU32()
			{
				mPos += sizeof(u32);
				sigassert( mPos <= mPacket.mBufferPos );
				return ENET_NET_TO_HOST_32( *(u32*)&mPacket.mBuffer[ mPos - sizeof(u32) ] );
			}
			u16 fReadU16()
			{
				mPos += sizeof(u16);
				sigassert( mPos <= mPacket.mBufferPos );
				return ENET_NET_TO_HOST_16( *(u16*)&mPacket.mBuffer[ mPos - sizeof(u16) ] );
			}
			const char* fReadStr(const u32 toRead = ~0) //~0 means look for NULL terminator
			{
				const char* retval = &mPacket.mBuffer[ mPos ];
				
				if( toRead == ~0 ) while( mPacket.mBuffer[ mPos++ ] );
				else mPos += toRead;

				sigassert( mPos <= mPacket.mBufferPos );
				return retval;
			}
		};

		//------------------------------------------------------------------------------
		static b32 fIsValidDataType( tPostgreDatabase::tDataType type )
		{
			switch(type)
			{
			case tPostgreDatabase::cBigInt:		return true;
			case tPostgreDatabase::cInt:		return true;
			case tPostgreDatabase::cText:		return true;
			case tPostgreDatabase::cReal:		return true;
			case tPostgreDatabase::cIntArray:	return true;
			case tPostgreDatabase::cTextArray:	return true;
			case tPostgreDatabase::cRealArray:	return true;
			case tPostgreDatabase::cTimeStamp:	return true;
			case tPostgreDatabase::cBit:		return true;
			case tPostgreDatabase::cVoid:		return true;
			}
			return false;
		}


		//------------------------------------------------------------------------------
		static b32 fParseTable( tPacketReader& reader, tPostgreDatabase::tTable& table )
		{
			const u32 chunkLen = reader.fReadU32(); //I think this is supposed to be chunk length? I think we could use this to jump straight to the row information if we wanted to
			const u16 numFields = reader.fReadU16();

			log_output( Log::cFlagDatabase, "{DB} Parsing table. Num fields: " << numFields << std::endl );

			sigassert( numFields < 100 ); //how about we be reasonable? A table with 100+ columns? WTF?
			table.mHeaders.fResize( numFields );

			//parse headers
			for(u32 i = 0; i < table.mHeaders.fCount( ); ++i)
			{
				tPostgreDatabase::tColumnHeader & header = table.mHeaders[ i ];

				header.mName = reader.fReadStr( );
				reader.fIgnore(sizeof(u32) + sizeof(u16)); //let's ignore a bunch of stuff that postgres thinks is necessary. obviously I don't think it is
				header.mType = (tPostgreDatabase::tDataType)reader.fReadU32( );
				reader.fIgnore(sizeof(u32) + sizeof(u16)*2); //And ignore a bunch of stuff after the column type

				if( !fIsValidDataType( header.mType ) )
				{
					log_warning( "{DB} Invalid data type (" << header.mType << ") parsing data table." );
					return false;
				}
			}

			//parse rows
			char tempBuff[512];
			while( reader.mPos < reader.mPacket.mBufferPos )
			{
				const u32 readerStartPos = reader.mPos;
				char tag = reader.fReadChar( );
				if( tag == 'C' ) //CommandComplete
				{
					//read CommandComplete
					const u32 len = reader.fReadU32( );
					const char* str = reader.fReadStr( );

					//read ReadyForQuery
					const char tagZ = reader.fReadChar( );	
					sigassert( tagZ == 'Z' );
					const u32 rfqLen = reader.fReadU32( );
					sigassert( rfqLen == 5 );
					const char status = reader.fReadChar( );
					log_assert( 
						status == 'I' ||	//idle (not in a transaction block)
						status == 'T' ||	//in a transaction block
						status == 'E',		//in a failed transaction block
						((void*)&reader) << " / " << ((void*)&table) << " - " << status );

					//verify CommandComplete + end of packet
					if_assert(
						std::stringstream ss; ss << str;
						std::string command; ss >> command;
						u32 value = 0; ss >> value;
					)
					log_assert( command == "SELECT", ((void*)&reader) << " / " << ((void*)&table) << " - " << command );
					log_assert( value == table.mRows.fCount( ), ((void*)&reader) << " / " << ((void*)&table) << " - " << value << " != " << table.mRows.fCount( ) );
					log_assert( reader.mPos == reader.mPacket.mBufferPos, ((void*)&reader) << " / " << ((void*)&table) << " - " << reader.mPos << " != " << reader.mPacket.mBufferPos );
					break;
				}
				//'D' = DataRow
				if( tag != 'D' )
				{
					log_warning( __FILE__ << "(" << __LINE__ << ") {DB} Error: INVALID TAG - " << tag );
					return false;
				}
				const u32 rowLen = reader.fReadU32( );
				const u16 numColumns = reader.fReadU16( );
				sigassert( numColumns == table.mHeaders.fCount( ) );
				if( numColumns != table.mHeaders.fCount( ) )
				{
					log_warning( __FILE__ << "(" << __LINE__ << ") {DB} Error: Parsing row columns failed" );
					return false;
				}

				//parse each cell in row
				tPostgreDatabase::tRow row;
				for(u32 i = 0; i < numColumns; ++i)
				{
					tPostgreDatabase::tCell cell;

					const u32 strLen = reader.fReadU32( );
					if( strLen != ~0 ) // If data was returned
					{
						sigassert( strLen < sizeof(tempBuff) );
						if( strLen >= sizeof(tempBuff) )
						{
							log_warning( __FILE__ << "(" << __LINE__ << ") {DB} Error: String to big to handle" );
							return false;
						}
						const char* str = reader.fReadStr( strLen );
						memcpy(tempBuff, str, strLen); //copy to temporary buffer to append null terminator before parsing
						tempBuff[strLen] = 0;
						cell.fSet( table.mHeaders[i].mType, tempBuff );
					}
					else cell.fSet( table.mHeaders[ i ].mType, "" );

					row.fPushBack( cell );
				}

				sigassert( row.fCount( ) == numColumns );
				if( row.fCount( ) != numColumns )
				{
					log_warning( __FILE__ << "(" << __LINE__ << ") {DB} Error: Invalid # of columns in row. Have: " << row.fCount( ) << " expected: " << numColumns );
					return false;
				}
				table.mRows.fPushBack( row );

				sigassert( reader.mPos == readerStartPos + rowLen + 1 ); //+1 for the starting tag
				if( reader.mPos != readerStartPos + rowLen + 1 )
				{
					log_warning( __FILE__ << "(" << __LINE__ << ") {DB} Error: invalid row parsing length" );
					return false;
				}
			}

			log_line( Log::cFlagDatabase, "{DB} Parsing Complete. Num rows: " << table.mRows.fCount( ) << std::endl );
			sigassert( reader.mPos == reader.mPacket.mBufferPos );

			return true;
		}
		
	} // ::_anonymous


	//------------------------------------------------------------------------------
	// tPostgreDatabase::tCell
	//------------------------------------------------------------------------------
	void tPostgreDatabase::tCell::fSet( tDataType type, const char* value )
	{
		//printf("%s - %s\n",tPostgreDatabase::fDataTypeToString(type),value);
		switch( type )
		{
		case cBigInt:
			mBigInt = _strtoui64(value,0,10);
			break;
		case cInt:
			mInt = atoi(value);
			break;
		case cText:
			mText = value;
			break;
		case cReal:
			mReal = (f32)atof(value);
			break;
		case cIntArray:
			{
				char tempBuff[256];

				const char* p = value;
				if( *p == '\0' )
					break;

				sigassert( *p == '{' );
				++p;
				while(*p != '}')
				{
					const char* start = p;
					while(*p != ',' && *p != '}') ++p;
					sigassert(start != p);
					sigassert(p > start);
					const u32 strLen = (p - start);
					if( sizeof(tempBuff) <= strLen )
					{
						log_warning( "{DB} Parse error: size of string buffer is too small for int value. Have: " << sizeof(tempBuff) << " need space for: " << strLen );
						return;
					}
					memcpy(tempBuff, start, strLen);
					tempBuff[strLen] = 0;

					mIntArray.fPushBack( atoi(tempBuff) );
					if( *p == '}' )
						break;
					++p;
				}
			}
			break;
		case cTextArray:
			{
				char tempBuff[256];

				const char* p = value;
				if( *p == '\0' )
					break;

				sigassert( *p == '{' );
				++p;
				while(*p != '}')
				{
					const char* start = p;
					while(*p != ',' && *p != '}') ++p;
					sigassert(start != p);
					sigassert(p > start);
					const u32 strLen = (p - start);
					if( sizeof(tempBuff) <= strLen )
					{
						log_warning( "{DB} Parse error: size of string buffer is too small for string value. Have: " << sizeof(tempBuff) << " need space for: " << strLen );
						return;
					}
					memcpy(tempBuff, start, strLen);
					tempBuff[strLen] = 0;

					mTextArray.fPushBack( std::string( tempBuff ) );
					if( *p == '}' )
						break;
					++p;
				}
			}
			break;
		case cRealArray:
			{
				char tempBuff[256];

				const char* p = value;
				if( *p == '\0' )
					break;

				sigassert( *p == '{' );
				++p;
				while(*p != '}')
				{
					const char* start = p;
					while(*p != ',' && *p != '}') ++p;
					sigassert(start != p);
					sigassert(p > start);
					const u32 strLen = (p - start);
					if( sizeof(tempBuff) <= strLen )
					{
						log_warning( "{DB} Parse error: size of string buffer is too small for float value. Have: " << sizeof(tempBuff) << " need space for: " << strLen );
						return;
					}
					memcpy(tempBuff, start, strLen);
					tempBuff[strLen] = 0;

					mRealArray.fPushBack( (f32)atof(tempBuff) );
					if( *p == '}' )
						break;
					++p;
				}
			}
			break;

		case cTimeStamp:
			mText = value;
			break;
		case cBit:
			mBool = atoi( value );
			break;
		case cVoid: break;
		default:
			sigassert( !"{DB} ERROR UNKNOWN TYPE" );
		};
	}

	//------------------------------------------------------------------------------
	// tPostgreDatabase
	//------------------------------------------------------------------------------
	tPostgreDatabase::tPostgreDatabase( )
		: mState( cStateNull )
	{
		mResponder = make_delegate_memfn( 
			Net::tServerConnection::tResponseCallback,
			tPostgreDatabase, fOnResponse );
	}

	//------------------------------------------------------------------------------
	void tPostgreDatabase::fTick( )
	{
		if( mState == cStateError )
		{
			//do nothing. something messed up when trying to connect to server
			return;
		}

		mConnection.fUpdate( );

		switch( mState )
		{
		case cStateNull:
			{
				// Send server connection packet
				if( mConnection.fReadyToSend( ) )
				{
					tPacket packet;
					packet.fResetForSend();
					packet.fPush(DATABASE_VERSION);
					packet.fPush("user"); packet.fPush( mUserName );
					packet.fPush("database"); packet.fPush( mDatabaseName );
					packet.fPrepareForSend( );

					b32 success = mConnection.fSend( packet.mBuffer, packet.mBufferPos, mResponder );
					if( success ) mState = cStateTestAuth;
					else mState = cStateError;
				}

			} break;

		case cStateConnected:
			{
				//send queries if we have any.
				if( mConnection.fReadyToSend( ) && mQueries.fNumItems( ) )
				{
					mQueries.fTryPopFirst( mCurrentQuery );

					log_line( Log::cFlagDatabase, "QUERY[" << mCurrentQuery.mQuery << "] Executing." );

					tPacket packet;
					packet.fResetForSend( 'Q' );
					packet.fPush( mCurrentQuery.mQuery );
					packet.fPrepareForSend( );
					if( mConnection.fSend( packet.mBuffer, packet.mBufferPos, mResponder ) )
						mState = cStateWaitForRecv;
					else
						mState = cStateError;

					////if query is "SELECT * FROM souls" then the packet should be this exact value below.
					//char raw[] = { 0x51, 0x00, 0x00, 0x00, 0x18, 0x53, 0x45, 0x4C, 0x45, 0x43, 0x54, 0x20, 0x2A, 0x20, 0x46, 0x52, 0x4F, 0x4D, 0x20, 0x73, 0x6F, 0x75, 0x6C, 0x73, 0x00};
					//sigassert( packet.mBufferPos == sizeof(raw) );
					//sigassert( !fMemCmp( raw, packet.mBuffer, sizeof(raw) ) );
				}
			} break;
		}
	}

	//------------------------------------------------------------------------------
	void tPostgreDatabase::fConnect( const char * serverName, u16 port, const tStringPtr & userName, const tStringPtr & databaseName )
	{
		mConnection.fConnect( serverName, port );
		mUserName = userName;
		mDatabaseName = databaseName;

		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	void tPostgreDatabase::fDisconnect( )
	{
		mConnection.fDisconnect( );

		mState = cStateDisconnected;
	}

	//------------------------------------------------------------------------------
	void tPostgreDatabase::fPushQuery( const char * query, const tOnQueryComplete& onQueryComplete )
	{
		// Don't queue if we can't send
		if( !fConnected( ) )
			return;

		tQuery newQuery;
		newQuery.mQuery = query;
		newQuery.mCallback = onQueryComplete;

		mQueries.fReserve( 1 );

		mQueries.fPushLast( newQuery );
	}

	//------------------------------------------------------------------------------
	b32 tPostgreDatabase::fOnResponse( const void * buffer, u32 numBytes )
	{
		switch( mState )
		{
		case cStateTestAuth:
			{
				//SUCCESS!
				//TODO, verify packet is good somehow?
				mState = cStateConnected;
			}
			break;
		case cStateWaitForRecv:
			{
				tPacket packet;

				//we got a problem. too much data!
				if( numBytes >= sizeof( packet.mBuffer ) )
				{
					log_warning( "{DB} we just recieved more data than we can handle! " << numBytes );
					break;
				}

				fMemCpy( packet.mBuffer, buffer, numBytes );
				packet.mBufferPos = numBytes;
				
				//SUCCESS!
				//now lets parse this packet like a boss.
				tPacketReader reader( packet );
				const char tag = reader.fReadChar();

				//take a look at http://www.postgresql.org/docs/9.0/interactive/protocol-message-formats.html
				// for more information on these tags and how to parse them

				if( tag == 'T' ) //RowDescription
				{
					if( !mCurrentQuery.mCallback.fNull( ) )
					{
						Time::tStopWatch stopWatch;

						tTable table;
						const b32 success = fParseTable( reader, table );
#ifdef sig_logging
						if( Log::fFlagEnabled( Log::cFlagDatabase ) )
						{
							log_output( Log::cFlagDatabase, "{DB} Parsing table succes: " << success << ". took: " << stopWatch.fGetElapsedMs( ) << "ms." << std::endl );
							fDebugPrintTable( table );
						}
#endif//sig_logging

						mCurrentQuery.mCallback( success, table );
					}

					mState = cStateConnected;
					break;
				}
				else if( tag == 'C' ) //CommandComplete
				{
					const u32 len = reader.fReadU32( );
					const char* str = reader.fReadStr( );

					log_output( Log::cFlagDatabase, "{DB} CommandComplete: " << str << std::endl );
					mState = cStateConnected;
					break;
				}

				log_output( Log::cFlagDatabase, "{DB} Some error occurred with server communication: [" );
				fwrite(packet.mBuffer, 1, packet.mBufferPos, stdout);
				log_output( Log::cFlagDatabase, "]\n");
				mState = cStateError;
			}
			break;
		default:
			sigassert( !"not handled" );
		}

		// The result indicates whether or not we got a full response
		return true;
	}

	//------------------------------------------------------------------------------
	const char* tPostgreDatabase::fDataTypeToString( tDataType type )
	{
		switch(type)
		{
		case cBigInt: return "bigint";
		case cInt: return "int";
		case cText: return "text";
		case cReal: return "real";
		case cIntArray: return "int[]";
		case cTextArray: return "text[]";
		case cRealArray: return "real[]";
		case cTimeStamp: return "timestamp";
		case cBit: return "bit";
		}
		return "{UNKNOWN TYPE}";
	}

	//------------------------------------------------------------------------------
	void tPostgreDatabase::fDebugPrintTable( const tTable& table )
	{
#ifdef sig_logging
		std::stringstream ss;
		for(u32 i = 0; i < table.mHeaders.fCount( ); ++i)
		{
			const tPostgreDatabase::tColumnHeader& h = table.mHeaders[i];
			ss <<  h.mName << "{" << tPostgreDatabase::fDataTypeToString(h.mType) << "}    ";
		}
		log_output( Log::cFlagDatabase, ss.str( ) << std::endl );


		for(u32 i = 0; i < table.mRows.fCount( ); ++i)
		{
			ss.str( "" ); // reset the stream

			const tPostgreDatabase::tRow& r = table.mRows[i];
			sigassert( r.fCount( ) == table.mHeaders.fCount( ) ); //this should ALWAYS be true
			ss << i << ": ";
			for(u32 c = 0; c < r.fCount( ); ++c)
			{
				const tPostgreDatabase::tCell& cell = r[c];
				switch(table.mHeaders[c].mType)
				{
				case tPostgreDatabase::cBigInt:
					ss << cell.mBigInt;
					break;
				case tPostgreDatabase::cInt:
					ss << cell.mInt;
					break;
				case tPostgreDatabase::cText:
					ss << cell.mText;
					break;
				case tPostgreDatabase::cReal:
					ss << cell.mReal;
					break;
				case tPostgreDatabase::cIntArray:
					ss << "{ ";
					for(u32 a = 0; a < cell.mIntArray.fCount( ); ++a)
						ss << cell.mIntArray[a] << " ";
					ss << "}";
					break;
				case tPostgreDatabase::cTextArray:
					ss << "{ ";
					for(u32 a = 0; a < cell.mTextArray.fCount( ); ++a)
						ss << cell.mTextArray[a] << " ";
					ss << "}";
					break;
				case tPostgreDatabase::cRealArray:
					ss << "{ ";
					for(u32 a = 0; a < cell.mRealArray.fCount( ); ++a)
						ss << cell.mRealArray[a] << " ";
					ss << "}";
					break;
				case tPostgreDatabase::cTimeStamp:
					ss << cell.mText;
					break;
				case tPostgreDatabase::cBit:
					ss << cell.mBool;
					break;
				case tPostgreDatabase::cVoid:
					ss << "VOID";
					break;
				default:
					sigassert( !"THIS SHOULD NEVER HAPPEN" );
				};

				ss << "    ";
			}

			log_output( Log::cFlagDatabase, ss.str( ) << std::endl );
		}
#endif//sig_logging

	}


}} // ::Sig::Sql
