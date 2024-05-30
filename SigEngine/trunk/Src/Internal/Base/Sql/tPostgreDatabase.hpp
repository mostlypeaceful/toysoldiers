//------------------------------------------------------------------------------
// \file tPostgreDatabase.hpp - 21 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tPostgreDatabase__
#define __tPostgreDatabase__

#include "Net/tServerConnection.hpp"

namespace Sig { namespace Sql
{
	///
	/// \class tPostgreDatabase
	/// \brief Provides database semantics for a Postgre database
	class tPostgreDatabase
	{
	public:

		//public structures
		enum tDataType
		{
			cBigInt = 20,
			cInt = 23,
			cText = 25,
			cReal = 700,
			cIntArray = 1007,
			cTextArray = 1009,
			cRealArray = 1021,
			cTimeStamp = 1114,
			cBit = 1560,
			cVoid = 2278
		};

		struct tColumnHeader
		{
			const char* mName;
			tDataType mType;
		};

		struct tCell
		{
			//these can't go into a union so they are placed on the outside to make parsing easier for the user
			std::string mText;
			tDynamicArray<s32> mIntArray;
			tGrowableArray<std::string> mTextArray;
			tDynamicArray<f32> mRealArray;

			//union to try to keep the data size small
			union
			{
				s64 mBigInt;
				s32 mInt;
				f32 mReal;
				b32 mBool;
			};
			void fSet( tDataType type, const char* value );
		};
		typedef tDynamicArray<tCell> tRow;

		struct tTable
		{
			tDynamicArray<tColumnHeader> mHeaders;
			tGrowableArray<tRow> mRows;
		};
		typedef tDelegate< void ( b32, const tTable& ) > tOnQueryComplete;


	public:

		tPostgreDatabase( );
		
		b32 fFailed( ) const { return mState == cStateError; }
		b32 fDisconnected( ) const { return mState == cStateDisconnected; }
		b32 fConnected( ) const { return mState >= cStateConnected; }
		b32 fBusy( ) const { return mState == cStateWaitForRecv || ( mState == cStateConnected && mQueries.fNumItems( ) ); }

		void fTick( );

		void fConnect( const char * serverName, u16 port, const tStringPtr & userName, const tStringPtr & databaseName );
		void fDisconnect( );

		void fPushQuery( const char * query, const tOnQueryComplete& onQueryComplete );

	private:

		enum tState 
		{ 
			cStateError, 
			cStateDisconnected, 
			cStateNull, 
			cStateTestAuth, 
			cStateConnected, 
			cStateWaitForRecv 
		};

		struct tQuery
		{
			std::string mQuery;
			tOnQueryComplete mCallback;
		};

	private:

		b32 fOnResponse( const void * buffer, u32 numBytes );

	private:

		//static funcs
		static const char* fDataTypeToString( tDataType type );
		static void fDebugPrintTable( const tTable& table );
		
	private:

		Net::tServerConnection mConnection;
		Net::tServerConnection::tResponseCallback mResponder;
		tStringPtr mUserName;
		tStringPtr mDatabaseName;

		tState mState;
		tQuery mCurrentQuery;
		tRingBuffer< tQuery > mQueries;
	};

}} // ::Sig::Sql

#endif//__tPostgreDatabase__
