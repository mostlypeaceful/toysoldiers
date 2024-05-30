#include "BasePch.hpp"
#include "tSharedStringTable.hpp"

namespace Sig
{
	tSharedStringTable& tStringPtrString::fAccessTable( ) { static tSharedStringTable table; return table; }
}

namespace Sig
{
	namespace
	{
		static tSharedStringCriticalSection& fCritSec( )
		{
			static tSharedStringCriticalSection gCritSec;
			return gCritSec;
		}

		static tStringPtrString* fNewStringPtrString( const char* str )
		{
#ifndef sig_sharedstring_thread_safe
			sigassert_is_main_thread( );
#endif//sig_sharedstring_thread_safe
			tSharedStringMutex threadSafe( fCritSec( ) );
			return static_cast< tStringPtrString* >( tStringPtrString::fAccessTable( ).fFindString( tStringPtrString::fNewInstance, str ) );
		}
	}

	const tStringPtr tStringPtr::cNullPtr;

	tStringPtr::tStringPtr( const char* str )
	{
		if( str )
			mStringRep.fReset( fNewStringPtrString( str ) );
	}
	tStringPtr::tStringPtr( const std::string& str )
	{
		if( str.c_str( ) )
			mStringRep.fReset( fNewStringPtrString( str.c_str( ) ) );
	}
	tStringPtr::~tStringPtr( )
	{
		tSharedStringMutex threadSafe( fCritSec( ) );
		mStringRep.fRelease( );
	}
	tStringPtr::tStringPtr(const tStringPtr& other)
	{
		tSharedStringMutex threadSafe( fCritSec( ) );
		mStringRep = other.mStringRep;
	}
	tStringPtr& tStringPtr::operator=(const tStringPtr& other)
	{
		tSharedStringMutex threadSafe( fCritSec( ) );
		if( this != &other )
			mStringRep = other.mStringRep;
		return *this;
	}
}


