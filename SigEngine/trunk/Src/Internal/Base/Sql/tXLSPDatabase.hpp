//------------------------------------------------------------------------------
// \file tXLSPDatabase.hpp - 21 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tXLSPDatabase__
#define __tXLSPDatabase__

#include "Net/tTitleServerConnection.hpp"

namespace Sig { namespace Sql
{
	///
	/// \class tXLSPFunctionCall
	/// \brief Helper to build function call information
	class tXLSPFunctionCall
	{
	public:
		tXLSPFunctionCall( const char * funcName ) { mStream << funcName; }

		std::string fBuild( ) const { return mStream.str( ); }

		template<class tArgType>
		void fAddArgument( const char * name, const tArgType & arg )
		{
			std::stringstream ss; ss << arg;
			fAddArgument( name, ss.str( ) );
		}

		template<>
		void fAddArgument( const char * name, const std::string & arg )
		{
			std::string sanArg; 
			fSanitize( arg.c_str( ), arg.length( ), sanArg );
			fPushArg( name, sanArg );
		}

		template<>
		void fAddArgument( const char * name, const tStringPtr & arg )
		{
			std::string sanArg; 
			fSanitize( arg.fCStr( ), arg.fLength( ), sanArg );
			fPushArg( name, sanArg );
		}

		template<>
		void fAddArgument( const char * name, const tFilePathPtr& arg )
		{
			std::string sanArg; 
			fSanitize( arg.fCStr( ), arg.fLength( ), sanArg );
			fPushArg( name, sanArg );
		}

		void fAddArgument( const char * name, const char * arg )
		{
			std::string sanArg; 
			fSanitize( arg, ~0, sanArg );
			fPushArg( name, sanArg );
		}

	private:

		void fPushArg( const char * name, const std::string & sanArg );
		void fSanitize( const char * a, u32 aLen, std::string & output );

	private:

		std::stringstream mStream;
	};

	///
	/// \class tXLSPDatabase
	/// \brief Wraps a title server connection and simplifies data
	class tXLSPDatabase
	{
	public:

		enum tState
		{
			cStateNull,
		};

		enum tQueryType
		{
			cQueryTypeSproc,
			cQueryTypeSecureSproc,
			cQueryTypeUri,
			cQueryTypeSequence,
			cQueryTypeSecureSequence
		};

		
		///
		/// \class tResultSet
		/// \brief Results of an xlsp db query
		class tResultSet
		{
			friend class tXLSPDatabase;
		public:

			tResultSet( ) : mResults( NULL ) { }

			u32 fResultCount( ) const;
			
			b32 fGetResultColumn( u32 result, const char * column, std::string & out ) const;
			b32 fGetResultColumn( u32 result, const char * column, tStringPtr& out ) const;
			b32 fGetResultColumn( u32 result, const char * column, tFilePathPtr& out ) const;

			b32 fGetResultColumn( u32 result, const char * column, b32& out ) const;

			b32 fGetResultColumn( u32 result, const char * column, GUID& out ) const;

			template<class t>
			b32 fGetResultColumn( u32 result, const char * column, t & out ) const
			{
				std::string value;
				if( fGetResultColumn( result, column, value ) )
				{
					std::stringstream ss; ss.str( value );
					ss >> out;

					return true;
				}

				return false;
			}
			
		private:

			void * mResults;
		};

		typedef tDelegate< void ( const tResultSet & ) > tQueryCallback;
		typedef tDelegate< void ( ) > tFailedCallback;

	public:

		tXLSPDatabase( u32 priorityCount = 1 );

		void fClearServersToIgnore( ) { mConnection.fClearServersToIgnore( ); }
		void fAddServerToIgnore( const tStringPtr& toIgnore ) { mConnection.fAddServerToIgnore( toIgnore ); }

		void fConnect( const char * serverName, u32 serviceId, u16 port, const tFailedCallback & onFailed );
		void fDisconnect( );

		void fUpdate( );

		void fPushQuery( u32 priority, const char * query, tQueryType queryType );
		void fPushQuery
			( u32 priority
			, const char * query
			, const tQueryCallback & callback
			, tQueryType queryType );

		void fClearQueries( );

	private:

		struct tQuery
		{
			tQueryType mQueryType;
			std::string mQuery;
			tQueryCallback mCallback;
		};

		typedef tRingBuffer<tQuery> tQueryBuffer;

	private:

		static const u32 cMaxSendAttempts = 5;
		static const u32 cMaxNoResponseSendAttempts = 3;

	private:

		b32 fOnResponse( const void * buffer, u32 numBytes );

	private:

		u32 mState;
		Net::tTitleServerConnection mConnection;
		Net::tTitleServerConnection::tResponseCallback mResponder;
		tFailedCallback mOnFailed;

		tQuery mCurrentQuery;
		tDynamicArray< tQueryBuffer > mQueryBuffers;

		b32 mReceivedResponse;
		u32 mSendAttempts;
		u32 mSendAttemptsWithNoResponse;
	};

}} // ::Sig::Sql

#endif//__tXLSPDatabase__
