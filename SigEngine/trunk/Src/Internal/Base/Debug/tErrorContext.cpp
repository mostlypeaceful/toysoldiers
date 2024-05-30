#include "BasePch.hpp"
#include "tErrorContext.hpp"

namespace { void fShutUpCompilerAboutErrorContextCpp( ) { } }

#if defined( sig_assert )
namespace Sig
{
	namespace
	{
		Threads::tCriticalSection gThreadCS;
	}

	//------------------------------------------------------------------------------
	// tErrorContext -- statics
	//------------------------------------------------------------------------------
	u32 tErrorContext::gThreadCount = 0;
	tErrorContext::tThreadContext tErrorContext::gThreads[ cMaxThreads ];
	//------------------------------------------------------------------------------
	char* tErrorContext::fGetCurrentThreadContextStack( )
	{
		// Get current thread id
		const u32 threadId = Threads::tThread::fCurrentThreadId( );

		// Search for stack with same id
		for( u32 i = 0; i < gThreadCount; ++i )
		{
			if( gThreads[ i ].mThreadId == threadId )
				return gThreads[ i ].mStack;
		}

		// Not found, try to add
		if( gThreadCount != cMaxThreads )
		{
			Threads::tMutex mutex( gThreadCS );
			gThreads[ gThreadCount ].mStack[ 0 ] = 0;
			gThreads[ gThreadCount ].mThreadId = threadId;
			// Make sure this happens AFTER we initialize the thread data (someone could be looping on it)
			return gThreads[ gThreadCount++ ].mStack; 
		}

		// No room in # of allocated stacks. Bump the cMaxThreads #.
		return NULL;
	}
	//------------------------------------------------------------------------------
	u32 tErrorContext::fPush( const char* data )
	{
		// Get data for copying
		char* stack = fGetCurrentThreadContextStack( );
		if( !stack )
			return 0;
		const u32 stackLen = StringUtil::fStrLen( stack );
		const u32 dataLen = StringUtil::fStrLen( data );
		const u32 toCopy = (u32)fClamp<s32>( cStackSize - 1 - stackLen, 0, dataLen );

		// Copy and Null terminate
		fMemCpy( stack + stackLen, data, toCopy );
		stack[ stackLen + toCopy ] = 0;

		// Return bytes copied
		return toCopy;
	}
	//------------------------------------------------------------------------------
	void tErrorContext::fPop( u32 count )
	{
		char* stack = fGetCurrentThreadContextStack( );
		if( !stack )
			return;
		const u32 stackLen = StringUtil::fStrLen( stack );
		if( count > stackLen )
			stack[ 0 ] = 0; // Something messed up, let's just auto-fix ourselves
		else
			stack[ stackLen - count ] = 0;
	}
	//------------------------------------------------------------------------------
	void tErrorContext::fPrint( )
	{
		char threadInfo[64];
		for( u32 i = 0; i < gThreadCount; ++i )
		{
			if( gThreads[ i ].mStack[ 0 ] == 0 )
				continue; //no need to dump empty info

			// Print which thread
			u32 len = sprintf_s( threadInfo, "[%u]ThreadId:%u-----------------\n", i, gThreads[ i ].mThreadId );
			fwrite( threadInfo, 1, len, stdout );

			// Print thread context
			len = StringUtil::fStrLen( gThreads[ i ].mStack );
			fwrite( gThreads[ i ].mStack, 1, len, stdout );
		}		
	}

	//------------------------------------------------------------------------------
	// tErrorContext -- User-Interface
	//------------------------------------------------------------------------------
	tErrorContext::tErrorContext( )
		: mPushed( 0 )
	{
	}
	//------------------------------------------------------------------------------
	tErrorContext::~tErrorContext( )
	{
		fPop( mPushed );
	}
	//------------------------------------------------------------------------------
	tErrorContext& tErrorContext::operator<<( const char* data )
	{
		mPushed += fPush( data );
		return *this;
	}
	//------------------------------------------------------------------------------
	tErrorContext& tErrorContext::operator<<( const void* ptr )
	{
		char buff[ 32 ];
		sprintf_s( buff, "0x%p", ptr );
		mPushed += fPush( buff );
		return *this;
	}
	//------------------------------------------------------------------------------
	tErrorContext& tErrorContext::operator<<( s32 data )
	{
		char buff[ 32 ];
		sprintf_s( buff, "%d", data );
		mPushed += fPush( buff );
		return *this;
	}
	//------------------------------------------------------------------------------
	tErrorContext& tErrorContext::operator<<( u32 data )
	{
		char buff[ 32 ];
		sprintf_s( buff, "%u", data );
		mPushed += fPush( buff );
		return *this;
	}
	//------------------------------------------------------------------------------
	tErrorContext& tErrorContext::operator<<( f32 data )
	{
		char buff[ 32 ];
		sprintf_s( buff, "%f", data );
		mPushed += fPush( buff );
		return *this;
	}
	//------------------------------------------------------------------------------
	// tErrorContext -- Signal-Specific
	//------------------------------------------------------------------------------
	tErrorContext& tErrorContext::operator<<( const tStringPtr& str )
	{
		return operator<<( str.fCStr( ) );
	}
	//------------------------------------------------------------------------------
	tErrorContext& tErrorContext::operator<<( const tFilePathPtr& str )
	{
		return operator<<( str.fCStr( ) );
	}
}//Sig
#endif//defined( sig_assert )
