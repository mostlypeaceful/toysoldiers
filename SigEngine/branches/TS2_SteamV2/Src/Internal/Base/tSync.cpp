//------------------------------------------------------------------------------
// \file tSync.cpp - 09 Nov 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tSync.hpp"
#include "tFileWriter.hpp"
#include "tFileReader.hpp"
#include "GameArchiveString.hpp"
#include "EndianUtil.hpp"
#include "Threads\tThread.hpp"
#include "Threads\tMutex.hpp"
#include "Threads\LocalStorage.hpp"
#include "tCallStack.hpp"
#include "tModuleHelper.hpp"
#include "FileSystem.hpp"
#include "tCompressor.hpp"
#include "tDecompressor.hpp"
#include "tSharedStringTable.hpp"

namespace Sig
{

#ifndef sync_system_detect_only
	devvar( bool, Debug_SyncEnabled, false );
#else
	devvar( bool, Debug_SyncEnabled, true );
#endif

	devvar( bool, Debug_SyncSkipThreadAssert, true );

#ifndef sync_system_detect_only

	namespace
	{
		sig_static_assert( sizeof( char ) == 1 );

		template<class t>
		void fEndianSwap( tPlatformId source, byte * valuePtr )
		{
			sigassert( tIsBuiltInType<t>::cIs );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( t ), source );
		}

		template<>
		void fEndianSwap<Math::tVec2u>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tVec2u ) == 2 * sizeof( u32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( u32 ), source, 2 );
		}

		template<>
		void fEndianSwap<Math::tVec3u>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tVec3u ) == 3 * sizeof( u32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( u32 ), source, 3 );
		}

		template<>
		void fEndianSwap<Math::tVec4u>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tVec4u ) == 4 * sizeof( u32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( u32 ), source, 4 );
		}

		template<>
		void fEndianSwap<Math::tVec2f>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tVec2f ) == 2 * sizeof( f32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( f32 ), source, 2 );
		}

		template<>
		void fEndianSwap<Math::tVec3f>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tVec3f ) == 3 * sizeof( f32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( f32 ), source, 3 );
		}

		template<>
		void fEndianSwap<Math::tVec4f>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tVec4f ) == 4 * sizeof( f32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( f32 ), source, 4 );
		}

		template<>
		void fEndianSwap<Math::tMat3f>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tMat3f ) == 12 * sizeof( f32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( f32 ), source, 12 );
		}

		template<>
		void fEndianSwap<Math::tMat4f>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tMat4f ) == 16 * sizeof( f32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( f32 ), source, 16 );
		}

		template<>
		void fEndianSwap<Math::tAabbf>( tPlatformId source, byte * valuePtr )
		{
			sig_static_assert( sizeof( Math::tAabbf ) == 6 * sizeof( f32 ) );
			EndianUtil::fSwapForTargetPlatform( valuePtr, sizeof( f32 ), source,  6 );
		}

		template<class t> struct tSyncEventType { };
		template<int n> 
		struct tSyncEventId 
		{ 
			static u32 fValueSizeForId( u32 type ) { sigassert( 0 && "Unrecognized type" ); return 0; } 
			static const char * fTypeNameForId( u32 type ) { sigassert( 0 && "Unrecognized type" ); return "Unrecognized type"; }
			static std::string fPrintValue( u32 type, const byte * valuePtr ) { sigassert( 0 && "Unrecognized type" ); return "Unrecognized type"; }
			static void fEndianSwapValue( tPlatformId source, u32 type, const byte * valuePtr ) { sigassert( 0 && "Unrecognized type" ); }
		};

		#define define_sync_event_type( t, id ) \
		template<> struct tSyncEventType<t> { static const u32 cId = id; }; \
		template<> struct tSyncEventId<id> \
		{ \
			static const u32 cValueSize = sizeof( t ); \
			static u32 fValueSizeForId( u32 type ) \
			{ \
				if( type > id ) \
					return tSyncEventId<id+1>::fValueSizeForId( type ); \
				return cValueSize;\
			} \
			static const char * fTypeNameForId( u32 type ) \
			{ \
				if( type > id ) \
					 return tSyncEventId<id+1>::fTypeNameForId( type ); \
				return #t; \
			} \
			static std::string fPrintValue( u32 type, const byte * valuePtr ) \
			{ \
				if( type > id ) \
					return tSyncEventId<id+1>::fPrintValue( type, valuePtr) ; \
				std::stringstream ss; \
				t temp; fMemCpy( &temp, valuePtr, sizeof( t ) ); \
				ss << temp; \
				return ss.str( ); \
			} \
			static void fEndianSwapValue( tPlatformId source, u32 type, byte * valuePtr )\
			{\
				if( type > id ) tSyncEventId<id+1>::fEndianSwapValue( source, type, valuePtr );\
				else fEndianSwap<t>( source, valuePtr );\
			}\
		}

		define_sync_event_type( Math::tAabbf, 14 );
		define_sync_event_type( Math::tQuatf, 13 );
		define_sync_event_type( Math::tMat4f, 12 );
		define_sync_event_type( Math::tMat3f, 11 );
		define_sync_event_type( Math::tVec4f, 10 );
		define_sync_event_type( Math::tVec3f, 9 );
		define_sync_event_type( Math::tVec2f, 8 );
		define_sync_event_type( Math::tVec2u, 7 );
		define_sync_event_type( Math::tVec3u, 6 );
		define_sync_event_type( Math::tVec4u, 5 );
		define_sync_event_type( f32, 4 );
		define_sync_event_type( u64, 3 );
		define_sync_event_type( s64, 2 );
		define_sync_event_type( u32, 1 );
		define_sync_event_type( s32, 0 );

		template< class t >
		static void fWrite( tGrowableArray< byte > & buffer, const t * value, u32 count )
		{
			const u32 byteCount = count * sizeof( t );
			buffer.fInsert( buffer.fCount( ), ( const byte * )&byteCount, sizeof( byteCount ) );
			buffer.fInsert( buffer.fCount( ), ( const byte * )value, byteCount );
		}

		template< class t >
		static void fWrite( tGrowableArray< byte > & buffer, const t & value )
		{
			//sigassert( tIsBuiltInType<t>::cIs );
			buffer.fInsert( buffer.fCount( ), ( const byte * )&value, sizeof( value ) );
		}

		template<>
		static void fWrite( tGrowableArray< byte > & buffer, const tSync::tSyncStrPtr & str )
		{
			fWrite( buffer, str.fCStr( ), str.fLength( ) );
		}

		template< class t >
		static const byte * fRead( tPlatformId source, const byte * data, u32 dataLen, t & out )
		{
			//sigassert( tIsBuiltInType<t>::cIs );
			sigassert( dataLen >= sizeof( out ) );

			fMemCpy( &out, data, sizeof( out ) );
			EndianUtil::fSwapForTargetPlatform( &out, sizeof( out ), source );
			return data + sizeof( out );
		}

		template<>
		static const byte * fRead( tPlatformId source, const byte * data, u32 dataLen, tSync::tSyncStrPtr & str )
		{
			const byte * term = data + dataLen;

			u32 byteCount;
			data = fRead( source, data, term - data, byteCount );

			sigassert( data + byteCount <= term );

			if( byteCount )
				str = tSync::tSyncStrPtr( std::string( data, data + byteCount ) );
			else
				str = tSync::tSyncStrPtr::cNullPtr;

			return data + byteCount;
		}

		template< class t >
		static const byte * fRead( tPlatformId source, const byte * data, u32 dataLen, tGrowableArray<t> & out )
		{
			const byte * term = data + dataLen;

			u32 byteCount;
			data = fRead( source, data, term - data, byteCount );

			sigassert( data + byteCount <= term );
			sigassert( byteCount % sizeof( t ) == 0 );

			out.fSetCount( byteCount / sizeof( t ) );
			fMemCpy( out.fBegin( ), data, byteCount );

			return data + byteCount;
		}
	}

	static const u32 cDataIdPageShift = 16;
	static const u32 cDataIdOffsetMask = 0x0000FFFF;

	//------------------------------------------------------------------------------
	// tSyncStrPtrStr
	//------------------------------------------------------------------------------
	tSharedStringTable& tSync::tSyncStrPtrStr::fAccessTable( ) { static tSharedStringTable table; return table; }

	//------------------------------------------------------------------------------
	// tSyncStrPtr
	//------------------------------------------------------------------------------
	const tSync::tSyncStrPtr tSync::tSyncStrPtr::cNullPtr;
	Threads::tCriticalSection tSync::tSyncStrPtr::mCritSect;

	tSync::tSyncStrPtr::tSyncStrPtr( const char* str )
	{
		if( str == NULL )
			return;

		Threads::tMutex mutex( mCritSect );
		mStringRep.fReset( 
			static_cast< tSyncStrPtrStr* >( 
				tSyncStrPtrStr::fAccessTable( ).fFindString( 
					tSyncStrPtrStr::fNewInstance, str ) ) );
	}
	tSync::tSyncStrPtr::tSyncStrPtr( const std::string& str )
	{
		if( str.length( ) <= 0 )
			return;

		Threads::tMutex mutex( mCritSect );
		mStringRep.fReset( 
			static_cast< tSync::tSyncStrPtrStr* >( 
				tSyncStrPtrStr::fAccessTable( ).fFindString( 
					tSyncStrPtrStr::fNewInstance, str.c_str( ) ) ) );
	}
	tSync::tSyncStrPtr::~tSyncStrPtr( )
	{
		if( mStringRep.fNull( ) )
			return;

		Threads::tMutex mutex( mCritSect );
		mStringRep.fRelease( );
	}
	tSync::tSyncStrPtr::tSyncStrPtr(const tSync::tSyncStrPtr& other)
	{
		//Threads::tMutex mutex( mCritSect );
		mStringRep = other.mStringRep;
	}
	tSync::tSyncStrPtr& tSync::tSyncStrPtr::operator=(const tSync::tSyncStrPtr& other)
	{
		//Threads::tMutex mutex( mCritSect );
		if( this != &other )
			mStringRep = other.mStringRep;
		return *this;
	}

	//------------------------------------------------------------------------------
	// tSyncEvent
	//------------------------------------------------------------------------------
	b32 tSync::tSyncEvent::operator==( const tSyncEvent & e ) const
	{
		return (	mType == e.mType && 
					mLine == e.mLine &&
					mContextId == e.mContextId &&
					mFileId == e.mFileId &&
					mFunctionId == e.mFunctionId &&
					mDataId == e.mDataId	);
	}

	//------------------------------------------------------------------------------
	b32 tSync::tSyncEvent::operator!=( const tSyncEvent & e ) const
	{
		return (	mType != e.mType ||
					mLine != e.mLine ||
					mContextId != e.mContextId ||
					mFileId != e.mFileId ||
					mFunctionId != e.mFunctionId ||
					mDataId != e.mDataId	);
	}

	//------------------------------------------------------------------------------
	// tFrameHeader
	//------------------------------------------------------------------------------
	template< class t >
	void tSync::tFrameHeader::fPushEvent( 
		const t & value, 
		const char * context, 
		const char * file, 
		const char * function,
		u32 line )
	{
		tCallStack trace( tSyncEvent::cCallStackDepth );
		const tCallStackData & callStack = trace.fData( );

		tSyncEvent e;
		e.mType = tSyncEventType< t >::cId;
		e.mLine = line;
		e.mContextId = fFindOrAdd( context, mContexts );
		e.mFileId = fFindOrAdd( file, mFiles );
		e.mFunctionId = fFindOrAdd( function, mFunctions );
		e.mDataId = fPushData( ( const byte * )&value, sizeof( value ) );
		
		//e.mCallStack.fFill( NULL );
		const u32 depth = fMin( callStack.mDepth, e.mCallStack.fCount( ) );
		for( u32 a = 0; a < depth; ++a )
			e.mCallStack[ a ] = callStack.mAddresses[ a ];
		if( depth < e.mCallStack.fCount( ) )
			e.mCallStack[ depth ] = NULL;

		mEvents.fPushBack( e );
	}

	//------------------------------------------------------------------------------
	void tSync::tFrameHeader::fWrite( tGrowableArray< byte > & out ) const
	{
		// No events means no data
		if( mEvents.fCount( ) == 0 )
			return;

		// Contexts
		const u32 contextCount = mContexts.fCount( );
		Sig::fWrite( out, contextCount );
		for( u32 c = 0; c < contextCount; ++c )
			Sig::fWrite( out, mContexts[ c ] );

		// Files
		const u32 fileCount = mFiles.fCount( );
		Sig::fWrite( out, fileCount );
		for( u32 f = 0; f < fileCount; ++f )
			Sig::fWrite( out, mFiles[ f ] );

		// Functions
		const u32 funcCount = mFunctions.fCount( );
		Sig::fWrite( out, funcCount );
		for( u32 f = 0; f < funcCount; ++f )
			Sig::fWrite( out, mFunctions[ f ] );

		// Data Pages
		const u32 pageCount = mDataPages.fCount( );
		Sig::fWrite( out, pageCount );
		for( u32 p = 0; p < pageCount; ++p )
		{
			const tDataPagePtr & ptr = mDataPages[ p ];
			Sig::fWrite( out, ptr->mData.fBegin( ), ptr->mFilled );
		}

		// Events
		const u32 eventCount = mEvents.fCount( );
		out.fReserve( eventCount * sizeof( tSyncEventData ) );
		Sig::fWrite( out, eventCount );
		for( u32 e = 0; e < eventCount; ++e )
			Sig::fWrite( out, static_cast< const tSyncEventData& >( mEvents[ e ] ) );

		//Sig::fWrite( out, mEvents.fBegin( ), mEvents.fCount( ) );
	}

	//------------------------------------------------------------------------------
	const byte * tSync::tFrameHeader::fRead( tPlatformId source, const byte * data, u32 dataLen )
	{
		const byte * term = data + dataLen;

		// Contexts
		u32 contextCount;
		data = Sig::fRead( source, data, term - data, contextCount );
		mContexts.fSetCount( contextCount );
		for( u32 c = 0; c < contextCount; ++c )
			data = Sig::fRead( source, data, term - data, mContexts[ c ] );

		// Files
		u32 fileCount;
		data = Sig::fRead( source, data, term - data, fileCount );
		mFiles.fSetCount( fileCount );
		for( u32 f = 0; f < fileCount; ++f )
			data = Sig::fRead( source, data, term - data, mFiles[ f ] );

		// Functions
		u32 funcCount;
		data = Sig::fRead( source, data, term - data, funcCount );
		mFunctions.fSetCount( funcCount );
		for( u32 f = 0; f < funcCount; ++f )
			data = Sig::fRead( source, data, term - data, mFunctions[ f ] );

		// Data Pages
		u32 pageCount;
		data = Sig::fRead( source, data, term - data, pageCount );
		mDataPages.fSetCount( pageCount );

		tGrowableArray< byte > buffer;
		for( u32 p = 0; p < pageCount; ++p )
		{
			data = Sig::fRead( source, data, term - data, buffer );

			tDataPagePtr & page = mDataPages[ p ];
			page.fReset( NEW tDataPage( ) );
			page->fWrite( buffer.fBegin( ), buffer.fCount( ) );
		}

		// Events
		u32 eventCount;
		data = Sig::fRead( source, data, term - data, eventCount );
		mEvents.fSetCount( eventCount );
		for( u32 e = 0; e < eventCount; ++e )
			data = Sig::fRead( source, data, term - data, static_cast< tSyncEventData& >( mEvents[ e ] ) );

		//data = Sig::fRead( source, data, term - data, mEvents );
	
		return data;
	}

	//------------------------------------------------------------------------------
	void tSync::tFrameHeader::fWriteStacks( tGrowableArray< byte > & out ) const
	{
		const u32 eventCount = mEvents.fCount( );
		if( !eventCount )
			return;

		out.fReserve( sizeof( eventCount ) + eventCount * sizeof( mEvents[ 0 ].mCallStack ) );

		Sig::fWrite( out, eventCount );

		// Push the callstack
		for( u32 e = 0; e < eventCount; ++e )
			Sig::fWrite( out, mEvents[ e ].mCallStack );
	}

	//------------------------------------------------------------------------------
	const byte * tSync::tFrameHeader::fReadStacks( tPlatformId source, const byte * data, u32 dataLen )
	{
		const byte * term = data + dataLen;

		u32 eventCount;
		data = Sig::fRead( source, data, data - term, eventCount );

		if( eventCount != mEvents.fCount( ) )
		{
			log_warning( 0, "Sync: Couldn't read callstacks because count did not match event count" );
			return data;
		}

		for( u32 e = 0; e < eventCount; ++e )
			data = Sig::fRead( source, data, data - term, mEvents[ e ].mCallStack );

		return data;
	}

	//------------------------------------------------------------------------------
	void tSync::tFrameHeader::fReset( )
	{
		mContexts.fSetCount( 0 );
		mFiles.fSetCount( 0 );
		mFunctions.fSetCount( 0 );
		mEvents.fSetCount( 0 );

		mUnusedPages.fInsert( mUnusedPages.fCount( ), mDataPages.fBegin( ), mDataPages.fCount( ) );
		mDataPages.fSetCount( 0 );
	}

	//------------------------------------------------------------------------------
	std::string tSync::tFrameHeader::fInfo( tPlatformId source, const tSyncEvent & e ) const
	{
		std::stringstream ss;
		ss	<< "Sync Event - File: " << fFile( e ).fCStr( );
		ss	<< " Line: " << e.mLine;
		ss	<< " Function: " << fFunction( e ).fCStr( );
		ss	<< " Type: " << tSyncEventId<0>::fTypeNameForId( e.mType );
		ss	<< " Context: " << fContext( e ).fCStr( );
			
		const u32 size = fDataSize( e );
		byte * value = ( byte * )_alloca( size );
		fMemCpy( value, fData( e ), size );

		// Endian swap
		tSyncEventId<0>::fEndianSwapValue( source, e.mType, value );

		// Print value
		ss << " Value: " << tSyncEventId<0>::fPrintValue( e.mType, value );

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	u32 tSync::tFrameHeader::fDataSize( const tSyncEvent & e ) const
	{
		return tSyncEventId< 0 >::fValueSizeForId( e.mType );
	}

	//------------------------------------------------------------------------------
	const byte * tSync::tFrameHeader::fData( const tSyncEvent & e ) const
	{
		// If the data could fit in the id, it is in the id
		if( fDataSize( e ) <= sizeof( e.mDataId ) )
			return ( const byte * )&e.mDataId;

		// Otherwise it's stored on the page
		const tDataPagePtr & dataPage = mDataPages[ e.mDataId >> cDataIdPageShift ];
		return dataPage->mData.fBegin( ) + ( e.mDataId & cDataIdOffsetMask );
	}

	//------------------------------------------------------------------------------
	u32 tSync::tFrameHeader::fFindOrAdd( const char * str, tStrTable & table )
	{
		// Check for special null string
		if( !str )
			return cNullStringId;

		tSyncStrPtr strPtr( str );

		// Find the str
		s32 index = table.fIndexOf( strPtr );

		// Doesn't exist so add it.
		if( index  < 0 )
		{
			index = table.fCount( );
			table.fPushBack( strPtr );
		}

		return ( u32 )index;
	}

	//------------------------------------------------------------------------------
	u32 tSync::tFrameHeader::fPushData( const byte * data, u32 size )
	{
		// If the value can fit in the id, then do so
		if( size <= sizeof( u32 ) ) // sizeof( tSyncEvent::mDataId )
			return *( const u32 * )data;

		u32 id = 0;

		tDataPagePtr table;

		// Try to find one with space
		const s32 count = mDataPages.fCount( );
		for( s32 d = count - 1; d >= 0; --d )
		{
			if( mDataPages[ d ]->fSpaceLeft( ) < size )
				continue;
			
			id |= ( d << cDataIdPageShift );
			table = mDataPages[ d ];
			break;
		}

		// Find an unused table if we have any
		if( !table && mUnusedPages.fCount( ) )
		{
			table = mUnusedPages.fBack( );
			table->mFilled = 0;

			mUnusedPages.fErase( mUnusedPages.fCount( ) - 1 );

			id |= ( mDataPages.fCount( ) << cDataIdPageShift );
			mDataPages.fPushBack( table );
		}

		// Create a table if we don't have one
		if( !table )
		{
			sigassert( size < tDataPage::cMaxDataSize );
			table.fReset( NEW tDataPage( ) );
			
			id |= ( mDataPages.fCount( ) << cDataIdPageShift );
			mDataPages.fPushBack( table );
		}

		sigassert( ( cDataIdOffsetMask & table->mFilled ) == table->mFilled );
		id |= ( cDataIdOffsetMask & table->mFilled );

		// Write the data
		table->fWrite( data, size );

		return id;
	}

	//------------------------------------------------------------------------------
	// tSyncReader
	//------------------------------------------------------------------------------
	tSync::tSyncReader::tSyncReader( )
		: mReader( NULL )
		, mTitle( "" )
		, mBuildId( ~0 )
		, mHeaderIdx( 0 )
		, mEventIdx( 0 )
		, mPlatformId( cPlatformLastPlusOne )
	{
		
	}

	//------------------------------------------------------------------------------
	tSync::tSyncReader::~tSyncReader( )
	{
		delete mReader;
	}

	//------------------------------------------------------------------------------
	b32 tSync::tSyncReader::fOpen( const tFilePathPtr & path )
	{
		fClose( );

		// Allocate
		if( !mReader )
			mReader = NEW tFileReader( );
		
		// Opened
		if( mReader->fOpen( path ) )
		{
			// Platform
			u32 platform;
			( *mReader )( &platform, sizeof( platform ) );
			EndianUtil::fSwapForTargetPlatform( &platform, sizeof( platform ), cPlatformPcDx9 );
			mPlatformId = ( tPlatformId )platform;

			// Title
			u32 byteCount;
			( *mReader )( &byteCount, sizeof( byteCount ) );
			EndianUtil::fSwapForTargetPlatform( &byteCount, sizeof( byteCount ), mPlatformId );
			char * temp = ( char * )alloca( byteCount );
			( *mReader )( temp, byteCount );
			mTitle = std::string( temp, temp + byteCount );

			// Build
			( *mReader )( &mBuildId, sizeof( mBuildId ) );
			EndianUtil::fSwapForTargetPlatform( &mBuildId, sizeof( mBuildId ), mPlatformId );

			return true;
		}

		// Failed
		else
		{
			delete mReader;
			mReader = NULL;
			return false;
		}
	}

	//------------------------------------------------------------------------------
	void tSync::tSyncReader::fOpen( 
		const byte * data, u32 dataLen, 
		const byte * csData, u32 csDataLen )
	{
		fClose( );
		
		mPlatformId = cCurrentPlatform;
		mBuildId = tSync::fInstance( ).fBuildId( );
		mTitle = tSync::fInstance( ).fTitle( );

		fReadFrame( data, dataLen, csData, csDataLen );
	}

	//------------------------------------------------------------------------------
	b32 tSync::tSyncReader::fIsOpen( ) const
	{
		// Reader is open, we're open
		if( mReader )
			return true;

		// We're past all the frame headers, so we're closed
		if( mHeaderIdx >= mFrameHeaders.fCount( ) )
			return false;

		// We have more events in this frame header, so we're open
		if( mEventIdx < mFrameHeaders[ mHeaderIdx ].fEventCount( ) )
			return true;

		// We have no more events in this header, but we have more headers, so we're open
		if( mHeaderIdx < mFrameHeaders.fCount( ) - 1 )
			return true;

		// So closed
		return false;
	}

	//------------------------------------------------------------------------------
	void tSync::tSyncReader::fClose( )
	{
		delete mReader;
		mReader = NULL;

		mHeaderIdx = 0;
		mEventIdx = 0;
		mFrameHeaders.fSetCount( 0 );
		mBuffer.fSetCount( 0 );

		mTitle = "";
		mBuildId = ~0;
		mPlatformId = cPlatformLastPlusOne;
	}

	//------------------------------------------------------------------------------
	b32 tSync::tSyncReader::fEventsAreEqual( const tSyncReader & other ) const
	{
		if( mHeaderIdx != other.mHeaderIdx )
			return false;

		if( mEventIdx != other.mEventIdx )
			return false;

		if( mEvent != other.mEvent )
			return false;

		if( mFrameHeaders[ mHeaderIdx ].fContext( mEvent ) != other.mFrameHeaders[ mHeaderIdx ].fContext( other.mEvent ) )
			return false;

		if( mFrameHeaders[ mHeaderIdx ].fFile( mEvent ) != other.mFrameHeaders[ mHeaderIdx ].fFile( other.mEvent ) )
			return false;

		if( mFrameHeaders[ mHeaderIdx ].fFunction( mEvent ) != other.mFrameHeaders[ mHeaderIdx ].fFunction( other.mEvent ) )
			return false;

		const u32 dataSize = mFrameHeaders[ mHeaderIdx ].fDataSize( mEvent );
		sigassert( other.mFrameHeaders[ mHeaderIdx ].fDataSize( other.mEvent ) == dataSize );

		if( fMemCmp( mFrameHeaders[ mHeaderIdx ].fData( mEvent ), other.mFrameHeaders[ mHeaderIdx ].fData( other.mEvent ), dataSize ) )
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tSync::tSyncReader::fAdvance( )
	{
		// If we've exceeded this headers event count, move to the next header
		if( mHeaderIdx < mFrameHeaders.fCount( ) && mEventIdx >= mFrameHeaders[ mHeaderIdx ].fEventCount( ) )
		{
			++mHeaderIdx;
			mEventIdx = 0;
		}

		// Do we need to read another frame?
		if( mHeaderIdx >= mFrameHeaders.fCount( ) )
		{
			// Read the file
			if( mReader )
			{
				// Since we only advance on incoming logs, we advance until we find a frame
				// that actually logged something
				u32 frameSize;
				do
				{
					u32 read = ( *mReader )(&frameSize, sizeof( frameSize ) );
					EndianUtil::fSwapForTargetPlatform( &frameSize, sizeof( frameSize ), mPlatformId );

					// Couldn't read frame size
					if( read != sizeof( frameSize ) )
					{
						if( read == 0 )
						{
							log_line( Log::cFlagNetwork, __FUNCTION__ << " Out of sync events" );
						}
						else
						{
							log_line( Log::cFlagNetwork, __FUNCTION__ << " Error: File is corrupt" );
						}

						fClose( );
						return false;
					}

				} while( frameSize == 0 );

				// Read the frame in
				mBuffer.fSetCount( frameSize );
				u32 read = ( *mReader )( mBuffer.fBegin( ), mBuffer.fCount( ) );
				if( read != mBuffer.fCount( ) )
				{
					log_warning( Log::cFlagNetwork, __FUNCTION__ << " Error: File is corrupt" );
					fClose( );
					return false;
				}

				mHeaderIdx = 0;
				mEventIdx = 0;

				fReadFrame( mBuffer.fBegin( ), mBuffer.fCount( ), NULL, 0 );
			}
			else
			{
				log_line( Log::cFlagNetwork, __FUNCTION__ << " Out of sync events" );
				fClose( );
				return false;
			}
		}

		// If we have frames
		if( mHeaderIdx < mFrameHeaders.fCount( ) )
		{
			// If we have events, move to the next one
			if( mEventIdx < mFrameHeaders[ mHeaderIdx ].fEventCount( ) )
				mEvent = mFrameHeaders[ mHeaderIdx ].fEvent( mEventIdx++ );
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tSync::tSyncReader::fReadFrame( 
		const byte * data, u32 dataLen, 
		const byte * csData, u32 csDataLen )
	{
		const byte * dataTerm = data + dataLen;

		u32 headerCount;
		data = Sig::fRead( mPlatformId, data, dataTerm - data, headerCount );
		
		mFrameHeaders.fSetCount( headerCount );
		for( u32 h = 0; h < headerCount; ++h )
			data = mFrameHeaders[ h ].fRead( mPlatformId, data, dataTerm - data );

		sigassert( data == dataTerm );

		if( csData && csDataLen )
		{
			const byte * csDataTerm = csData + csDataLen;
			for( u32 h = 0; h < headerCount; ++h )
				csData = mFrameHeaders[ h ].fReadStacks( mPlatformId, csData, csDataTerm - csData );
		}
	}

	//------------------------------------------------------------------------------
	// tSyncWriter
	//------------------------------------------------------------------------------
	tSync::tSyncWriter::tSyncWriter(  )
		: mWriter( NULL )
	{
		Sig::fWrite( mBuffer, 0u );
	}

	//------------------------------------------------------------------------------
	tSync::tSyncWriter::~tSyncWriter( )
	{
		fClose( );
	}

	//------------------------------------------------------------------------------
	b32 tSync::tSyncWriter::fOpen( const tFilePathPtr & path, const std::string & title, u32 buildId )
	{
		if( !mWriter )
			mWriter = NEW tFileWriter( );

		// Success
		if( mWriter->fOpen( path, false ) )
		{
			u32 currentPlatform = cCurrentPlatform; // Force to dword for safety

			// Store the platform id in a consistent platform so any platform can read it
			EndianUtil::fSwapForTargetPlatform( 
				&currentPlatform, 
				sizeof( currentPlatform ), 
				cPlatformPcDx9 );

			( *mWriter )( &currentPlatform, sizeof( currentPlatform ) );

			const u32 byteCount = title.length( ) * sizeof ( char );
			( *mWriter )( &byteCount, sizeof( byteCount ) );
			( *mWriter )( title.c_str( ), byteCount );
			( *mWriter )( &buildId, sizeof( buildId ) );

			return true;
		}

		// Error
		else
		{
			delete mWriter;
			mWriter = NULL;
			return false;
		}
	}

	//------------------------------------------------------------------------------
	void tSync::tSyncWriter::fClose( )
	{
		fFlush( );

		delete mWriter;
		mWriter = NULL;
	}

	//------------------------------------------------------------------------------
	void tSync::tSyncWriter::fWrite( const tFrameHeader & header )
	{
		// To determine if data was written
		const u32 preWriteCount = mBuffer.fCount( );

		header.fWrite( mBuffer );

		//log_line( 0, "\tSync: Frame header had " << mBuffer.fCount( ) - preWriteCount << " bytes" );

		// Only count headers that write to the buffer 
		if( preWriteCount != mBuffer.fCount( ) )
			++*(u32 *)mBuffer.fBegin( );
	}

	//------------------------------------------------------------------------------
	void tSync::tSyncWriter::fFlush( )
	{
		if( mWriter && mWriter->fIsOpen( ) )
		{
			u32 frameSize = mBuffer.fCount( );

			// If all we have is the header count, then write out zero
			// so that the frame gets skipped
			if( frameSize == sizeof( u32 ) )
				frameSize = 0;

			( *mWriter )( &frameSize, sizeof( frameSize ) );
			( *mWriter )( mBuffer.fBegin( ), frameSize );
		}

		mBuffer.fSetCount( 0 );
		Sig::fWrite( mBuffer, 0u );
	}

#else

	//------------------------------------------------------------------------------
	template< class t >
	void tSync::tFrameHeader::fPushEvent( 
		const t & value, 
		const char * context, 
		const char * file, 
		const char * function,
		u32 line )
	{
		mHash ^= Hash::fGenericHash( ( const Sig::byte* )&value, sizeof( value ), ~0 );
	}

#endif //sync_system_detect_only

	//------------------------------------------------------------------------------
	// tSync
	//------------------------------------------------------------------------------
	b32 tSync::fFramesEqual( 
		const tFrame & a, 
		const tFrame & b, 
		std::string & unequalInfo, 
		const tFilePathPtr & desyncFolder )
	{

		// To save time we check hashes first
		if( a.mHash == b.mHash )
			return true;

#ifndef sync_system_detect_only

		// Short comparison
		if( !a.mBuffer.fCount( ) && !b.mBuffer.fCount( ) )
		{
			unequalInfo = "Hashes do not match but no buffers to compare";
			return false;
		}

		tDynamicBuffer aB, bB;
		fDecompress( a.mBuffer.fBegin( ), a.mBuffer.fCount( ), aB );
		fDecompress( b.mBuffer.fBegin( ), b.mBuffer.fCount( ), bB );

		tDynamicBuffer aCSB, bCSB;
		fDecompress( a.mCallstackBuffer.fBegin( ), a.mCallstackBuffer.fCount( ), aCSB );
		fDecompress( b.mCallstackBuffer.fBegin( ), b.mCallstackBuffer.fCount( ), bCSB );

		tSyncReader aR; aR.fOpen( aB.fBegin( ), aB.fCount( ), aCSB.fBegin( ), aCSB.fCount( ) );
		tSyncReader bR; bR.fOpen( bB.fBegin( ), bB.fCount( ), bCSB.fBegin( ), bCSB.fCount( ) );

		const u32 cMatchingEvents = 10;
		const u32 cSubsequentEvents = 50;
		tRingBuffer< std::string > matching( cMatchingEvents );

		while( aR.fIsOpen( ) && bR.fIsOpen( ) )
		{
			// Advance to the first event
			aR.fAdvance( );
			bR.fAdvance( );

			if( !aR.fEventsAreEqual( bR ) )
			{
				fSaveDesync( desyncFolder, aR, Time::fGetStamp( ), 0 );

				std::stringstream ss;
				ss << "Frames failed comparison:\n\t" << aR.fEventInfo( ) << " didn't match \n\t" << bR.fEventInfo( ) << "\n";
				ss << "Last " << matching.fNumItems( ) << " matching events:";

				std::string info;
				while( matching.fGet( info ) )
					ss << "\n\t" << info;

				ss << "\nSubsequent A events:";
				for( u32 e = 0; aR.fIsOpen( ) && e < cSubsequentEvents; ++e )
				{
					aR.fAdvance( );
					ss << "\n\t" << aR.fEventInfo( );
				}

				ss << "\nSubsequent B events:";
				for( u32 e = 0; bR.fIsOpen( ) && e < cSubsequentEvents; ++e )
				{
					bR.fAdvance( );
					ss << "\n\t" << bR.fEventInfo( );
				}

				unequalInfo = ss.str( );

				return false;
			}

			else
			{
				matching.fPut( aR.fEventInfo( ) );
			}
		}

		if( aR.fIsOpen( ) || bR.fIsOpen( ) )
		{
			u64 stamp = Time::fGetStamp( );
			u32 idx = 0;

			std::stringstream ss; ss << 
				"Frames have different buffer sizes ( " << 
				a.mBuffer.fCount( ) << " vs " << b.mBuffer.fCount( ) << 
				"), but all available events are equal\n";

			ss << "Last " << matching.fNumItems( ) << " matching events:";
			std::string info;
			while( matching.fGet( info ) )
				ss << "\n\t" << info;

			ss << "\nEvents left in A:";
			while( aR.fIsOpen( ) )
			{
				aR.fAdvance( );
				fSaveDesync( desyncFolder, aR, stamp, idx++ );
				ss << "\n\t" << aR.fEventInfo( );
			}

			ss << "\nEvents left in B:";
			while( bR.fIsOpen( ) )
			{
				bR.fAdvance( );
				ss << "\n\t" << bR.fEventInfo( );
			}

			unequalInfo = ss.str( );

			return false;
		}

		return true;
#else

		std::stringstream ss;
		ss << "Hashes don't match - A( " << a.mHash << " ), B( " <<  b.mHash << " )";
		unequalInfo = ss.str( );
		return false;

#endif // sync_system_detect_only
	}

	//------------------------------------------------------------------------------
	b32 tSync::fFilesEqual( 
		const tFilePathPtr & file1Path, 
		const tFilePathPtr & file2Path, 
		u32 prependCount,
		u32 appendCount )
	{

#ifndef sync_system_detect_only

		// Open the files
		tSyncReader r1, r2;

		if( !r1.fOpen( file1Path ) )
		{
			log_warning( 0, __FUNCTION__ << ": File 1 ( " << file1Path << " ) couldn't be opened" );
			return false;
		}

		if( !r2.fOpen( file2Path ) )
		{
			log_warning( 0, __FUNCTION__ << ": File 2 ( " << file2Path << " ) couldn't be opened" );
			return false;
		}

		log_line( Log::cFlagNetwork, "Checking syncronization of files:\n\t" << file1Path << "\n\t" << file2Path );

		tRingBuffer< std::string > matching( prependCount );
		while( r1.fIsOpen( ) && r2.fIsOpen( ) )
		{
			// Advance to the first event
			if( !r1.fAdvance( ) ) break;
			if( !r2.fAdvance( ) ) break;

			if( !r1.fEventsAreEqual( r2 ) )
			{
				log_line( 0, "!!!Desync detected!!!" );
				log_line( 0, "\t" << r1.fEventInfo( ) << " didn't match\n\t" << r2.fEventInfo( ) );
				log_line( 0, "Last " << matching.fNumItems( ) << " matching:" );

				std::string info;
				while( matching.fGet( info ) )
				{
					log_line( 0, "\t" << info );
				}

				return false;
			}

			matching.fPut( r1.fEventInfo( ) );
		}

		if( r1.fIsOpen( ) || r2.fIsOpen( ) )
			return false;
#endif
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tSync::fSyncEnabled( )
	{
		return Debug_SyncEnabled;
	}

	//------------------------------------------------------------------------------
	void tSync::fInitialize( const char * title, u32 buildId )
	{
		sigassert( title && "Title is invalid" );
		sigassert( !mRunning && "Sync initialized while running" );

		mTitle = title;
		mBuildId = buildId;
	}

	//------------------------------------------------------------------------------
	void tSync::fRegisterThread( b32 mainThread )
	{
		sigassert( !mRunning && "Sync thread registered while running" );

		if( Threads::LocalStorage::GetValue( mFrameHeaderTLS ) )
		{
			log_warning( 0, "Thread was already registered with sync" );
			return;
		}

		// Create the header and set the frame
		tFrameHeader * header = NEW tFrameHeader( );
		Threads::LocalStorage::SetValue( mFrameHeaderTLS, header );

		Threads::tMutex mutex( mWriteFramesSection );
		if( !mainThread )
			mWriteFrames.fPushBack( tFrameHeaderPtr( header ) );
		else
			mWriteFrames.fPushFront( tFrameHeaderPtr( header ) );
	}

	//------------------------------------------------------------------------------
	void tSync::fDeregisterThread( )
	{
		sigassert( !mRunning && "Sync thread deregistered while running" );

		if( tFrameHeader * ptr = ( tFrameHeader * )Threads::LocalStorage::GetValue( mFrameHeaderTLS ) )
		{
			Threads::LocalStorage::SetValue( mFrameHeaderTLS, NULL );
			
			Threads::tMutex mutex( mWriteFramesSection );
			mWriteFrames.fFindAndErase( ptr );
		}
		else
			log_warning( 0, "Thread was not registered with sync" );

	}

	//------------------------------------------------------------------------------
	void tSync::fStartSync( const tFilePathPtr & logPath, const tFilePathPtr & compareTo )
	{
		sigassert( !mRunning && "Sync started without ending the previous sync" );

#ifndef sync_system_detect_only

		// Create the new writer
		if( !logPath.fNull( ) )
		{
			if( !mWriter.fOpen( logPath, mTitle, mBuildId ) )
			{
				log_warning( 0, "Sync Error: Couldn't open log file: " << logPath );
				sigassert( 0 && "Sync Error: Log file could not be opened" );
			}
		}

		// Create the reader for comparison
		if( !compareTo.fNull( ) )
		{
			if( !mReader.fOpen( compareTo ) )
			{
				log_warning( 0, "Sync Error: Couldn't open compare file: " << compareTo );
				sigassert( 0 && "Sync Error: Compare file could not be opened" );
			}
		}

#endif

		const u32 frameCount = mWriteFrames.fCount( );
		for( u32 f = 0; f < frameCount; ++f )
			mWriteFrames[ f ]->fReset( );

		mRunning = true;
	}

	//------------------------------------------------------------------------------
	void tSync::fEndSync( )
	{
		if( !mRunning )
			return;

		mRunning = false;



		const u32 frameCount = mWriteFrames.fCount( );
		for( u32 f = 0; f < frameCount; ++f )
			mWriteFrames[ f ]->fReset( );

#ifndef sync_system_detect_only

		mWriter.fClose( );
		mReader.fClose( );

#endif
	}

	//------------------------------------------------------------------------------
	void tSync::fMarkFrame( tFrame * frame, b32 verboseFrame )
	{

#ifndef sync_system_detect_only

		// Write the frame to the writer
		const u32 frameCount = mWriteFrames.fCount( );
		for( u32 f = 0; f < frameCount; ++f )
		{
			//log_line( 0, "Sync: writing frame header " << f );
			mWriter.fWrite( *mWriteFrames[ f ] );
		}

		if( frame )
		{
			// Capture the frame
			const byte * data = NULL;
			u32 dataLength = 0;
			if( mWriter.fHasData( ) )
			{
				data = mWriter.fBuffer( ).fBegin( );
				dataLength = mWriter.fBuffer( ).fTotalSizeOf( );
			}

			// Compress the data if they want the whole buffer instead of just the hash
			if( verboseFrame && dataLength )
			{
				u32 uncompressedSize = dataLength;
				frame->mBuffer.fNewArray( 
					sizeof( uncompressedSize ) + tCompressor::cCompressionOverhead + uncompressedSize );
				
				tCompressor compressor;
				u32 compressedSize = compressor.fCompress( 
					data, uncompressedSize, frame->mBuffer.fBegin( ) + sizeof( uncompressedSize ) );
				frame->mBuffer.fResize( compressedSize + sizeof( uncompressedSize ) );
				*(u32*)frame->mBuffer.fBegin( ) = uncompressedSize;

				data = frame->mBuffer.fBegin( );
				dataLength = frame->mBuffer.fTotalSizeOf( );

				// Capture the callstacks in a similar manner
				tGrowableArray<byte> stacks;
				for( u32 f = 0; f < frameCount; ++f )
					mWriteFrames[ f ]->fWriteStacks( stacks );

				uncompressedSize = stacks.fTotalSizeOf( );
				frame->mCallstackBuffer.fNewArray( 
					sizeof( uncompressedSize ) + tCompressor::cCompressionOverhead + uncompressedSize );

				compressedSize = compressor.fCompress( 
					stacks.fBegin( ), uncompressedSize, frame->mCallstackBuffer.fBegin( ) + sizeof( uncompressedSize ) );
				frame->mCallstackBuffer.fResize( compressedSize + sizeof( uncompressedSize ) );
				*(u32*)frame->mCallstackBuffer.fBegin( ) = uncompressedSize;
			}

			frame->mHash = Hash::fGenericHash( data, dataLength, ~0 );
		}

		for( u32 f = 0; f < frameCount; ++f )
			mWriteFrames[ f ]->fReset( );

		mWriter.fFlush( );

#else
		const u32 frameCount = mWriteFrames.fCount( );
		if( frame )
		{
			frame->mHash = 0;
			for( u32 f = 0; f < frameCount; ++f )
				frame->mHash ^= mWriteFrames[ f ]->mHash;
		}

		for( u32 f = 0; f < frameCount; ++f )
			mWriteFrames[ f ]->fReset( );

#endif // sync_system_detect_only 
	}

	//------------------------------------------------------------------------------
	#define define_synclog( t ) \
	void tSync::fLog( const t & value, const char * context, const char * file, const char * function, u32 line, u32 category ) \
	{ \
		if( !mRunning ) \
			return; \
		if( !Debug_SyncEnabled ) \
			return; \
		if( !category || ( category & mAllowedCategories ) ) \
		{ \
			tFrameHeader * header = (tFrameHeader *)Threads::LocalStorage::GetValue( mFrameHeaderTLS ); \
			if( !header ) \
			{ \
				sigassert( Debug_SyncSkipThreadAssert && "Sync event logged from unregistered thread" ); \
				return; \
			} \
			header->fPushEvent( value, context, file, function, line ); \
		} \
	}

	//log_line( 0, "Sync Event - V: " << value << " C: " << (context ? context : "") << " Fu: " << (function ? function : "") << " Fi: " << (file ? file : "")  << " L: " << line ); 

	define_synclog( u32 );
	define_synclog( s32 );
	define_synclog( u64 );
	define_synclog( s64 );
	define_synclog( f32 );
	define_synclog( Math::tVec2u );
	define_synclog( Math::tVec3u );
	define_synclog( Math::tVec4u );
	define_synclog( Math::tVec2f );
	define_synclog( Math::tVec3f );
	define_synclog( Math::tVec4f );
	define_synclog( Math::tMat3f );
	define_synclog( Math::tMat4f );
	define_synclog( Math::tQuatf );
	define_synclog( Math::tAabbf );

	#undef define_synclog

	//------------------------------------------------------------------------------
	tSync::tSync( )
		: mBuildId( ~0 )
		, mTitle( "NoTitle" )
		, mRunning( false )
		, mAllowedCategories( ~0 )
		, mFrameHeaderTLS( Threads::LocalStorage::Allocate( ) )
	{
		sigassert( mFrameHeaderTLS != Threads::LocalStorage::cOutOfIndices );
	}

	//------------------------------------------------------------------------------
	tSync::~tSync( )
	{
		Threads::LocalStorage::Free( mFrameHeaderTLS );
	}

#ifndef sync_system_detect_only

	//------------------------------------------------------------------------------
	void tSync::fDecompress( const byte * data, u32 dataLength, tDynamicBuffer & decomped )
	{
		if( !data || !dataLength )
		{
			decomped.fDeleteArray( );
			return;
		}

		u32 decompSize = *( u32* )data;
		data += sizeof( decompSize );
		dataLength -= sizeof( decompSize );

		decomped.fNewArray( decompSize );

		tDecompressor decompressor;
		u32 decompedBytes = decompressor.fDecompress( 
			data, dataLength, decomped.fBegin( ), decomped.fTotalSizeOf( ) );

		sigassert( decompedBytes == decomped.fTotalSizeOf( ) );
	}

	//------------------------------------------------------------------------------
	void tSync::fSaveDesync( 
		const tFilePathPtr & desyncFolder, const tSyncReader & reader, u64 stamp, u32 idx )
	{
		if( desyncFolder.fNull( ) )
			return;

		tGrowableArray< byte > buffer;

		// Find the currently loaded modules and save their info out
		tModuleHelper modules( true );
		modules.fSave( buffer );
		
		const tSyncEvent & desyncEvent = reader.fEvent( );

		// We reverse everything for the PC since these files can only be opened on the PC
		u32 depth = 0;
		u32 depthIndex = buffer.fCount( );
		buffer.fInsert( buffer.fCount( ), ( const byte * )&depth, sizeof( depth ) );

		for( ; depth < desyncEvent.mCallStack.fCount( ); ++depth )
		{
			u64 address = ( u64 )desyncEvent.mCallStack[ depth ];

			if( !address )
				break;

			EndianUtil::fSwapForTargetPlatform( &address, sizeof( address ), cPlatformPcDx9 );
			buffer.fInsert( buffer.fCount( ), ( const byte * )&address, sizeof( address ) );
		}

		EndianUtil::fSwapForTargetPlatform( &depth, sizeof( depth ), cPlatformPcDx9 );
		fMemCpy( buffer.fBegin( ) + depthIndex, &depth, sizeof( depth ) );

		tFilePathPtr path;
		do
		{
			std::stringstream ss;
			ss << reader.fTitle( ) << "_" << stamp << "_" << idx++ << ".desync";
			path = tFilePathPtr::fConstructPath( desyncFolder, tFilePathPtr( ss.str( ) ) );
		} while( FileSystem::fFileExists( path ) );


		tFileWriter writer;
		if( !writer.fOpen( path, false ) )
		{
			log_warning( 0, "Failed to open desync file path" );
			return;
		}

		writer( buffer.fBegin( ), buffer.fTotalSizeOf( ) );
		writer.fFlush( );
	}

#endif // sync_system_detect_only

}
