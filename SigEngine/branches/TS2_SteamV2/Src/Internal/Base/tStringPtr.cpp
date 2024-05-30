#include "BasePch.hpp"
#include "tSharedStringTable.hpp"
#include "tGlobalCriticalSectionSingleton.cpp"

namespace Sig
{
	tSharedStringTable& tStringPtrString::fAccessTable( ) { static tSharedStringTable table; return table; }
}

namespace Sig
{
	namespace
	{
		static tStringPtrString* fNewStringPtrString( const char* str )
		{
			tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
			return static_cast< tStringPtrString* >( tStringPtrString::fAccessTable( ).fFindString( tStringPtrString::fNewInstance, str ) );
		}
	}

	const tStringPtr tStringPtr::cNullPtr;

	tStringPtr::tStringPtr( const char* str )
		: mStringRep( fNewStringPtrString( str ) )
	{
	}
	tStringPtr::tStringPtr( const std::string& str )
		: mStringRep( fNewStringPtrString( str.c_str( ) ) )
	{
	}
	tStringPtr::~tStringPtr( )
	{
		tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
		mStringRep.fRelease( );
	}
	tStringPtr::tStringPtr(const tStringPtr& other)
	{
		tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
		mStringRep = other.mStringRep;
	}
	tStringPtr& tStringPtr::operator=(const tStringPtr& other)
	{
		tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
		if( this != &other )
			mStringRep = other.mStringRep;
		return *this;
	}
}

