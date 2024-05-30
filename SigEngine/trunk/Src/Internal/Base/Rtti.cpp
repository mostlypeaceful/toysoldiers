#include "BasePch.hpp"
#include "Rtti.hpp"

using namespace Sig::Rtti::Private;

namespace Sig { namespace Rtti
{

	void* fNewClass( tClassId cid, b32 noopConstruct )
	{
		tClassAllocFunctions* find = tFactoryMap::fInstance( ).fFind( cid );
		log_assert( find, "Class ID [" << std::hex << cid << "] not registered for factory creation." );

		return find->mClassNew( noopConstruct );
	}

	void* fNewClassInPlace( tClassId cid, void* p, b32 noopConstruct )
	{
		tClassAllocFunctions* find = tFactoryMap::fInstance( ).fFind( cid );
		log_assert( find, "Class ID [" << std::hex << cid << "] not registered for factory creation." );

		return find->mClassNewInPlace( p, noopConstruct );
	}

	b32 fIsFactoryRegistered( tClassId cid )
	{
		return tFactoryMap::fInstance( ).fFind( cid ) != 0;
	}

	b32 tReflector::fContainsPointers( ) const
	{
		if( mIsPolymorphic )
			return true;

		for( const tBaseClassDesc* bc = fBaseClasses( ); bc && !bc->fNull( ); ++bc )
			if( bc->fReflector( )->fContainsPointers( ) )
				return true;

		for( const tClassMemberDesc* cm = fClassMembers( ); cm && !cm->fNull( ); ++cm )
		{
			if( cm->fIsPointer( ) )
				return true;
			if( cm->fReflector( )->fContainsPointers( ) )
				return true;
		}

		return false;
	}

}}
