#include "BasePch.hpp"

namespace Sig { namespace Rtti { namespace Private
{

#ifdef target_tools

	///
	/// \brief We use full type string compares to be absolutely certain we're not
	/// being duped by different dlls; i.e., different dlls could conceivably return
	/// different type_info objects, which, when performing a pointer compare could fail
	struct tRawStringEqual
	{
		inline b32 operator( )( const char* a, const char* b ) const
		{
			return strcmp( a, b ) == 0;
		}
	};

	tClassId fGetRuntimeClassId(const type_info& ti)
	{
		static tClassId gNextClassId = cInvalidClassId;
		static tHashTable<const char*,tClassId, tHashTableExpandOnlyResizePolicy, tHash<const char*>, tRawStringEqual> gRuntimeClassIdTable( 32 );

		const tClassId* find = gRuntimeClassIdTable.fFind( ti.name( ) );

		if( !find )
		{
			find = gRuntimeClassIdTable.fInsert( ti.name( ), ++gNextClassId );
		}

		return *find;
	}

#endif//target_tools

}}}

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
