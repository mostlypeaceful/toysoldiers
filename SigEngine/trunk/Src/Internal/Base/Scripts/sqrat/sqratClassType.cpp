///
/// Not originally part of SQrat.  We require this file to deal with two cases:
/// 
/// Case 1: ClassType<T> is instantiated from GameApp only.  In this case we
///   cannot have the class marked base_export as Base doesn't export it,
///   which would result in link errors.
///   
/// Case 2: ClassType<T> is instantiated from Base and GameApp.  In this case
///   we would need to have the class marked base_export, otherwise GameApp's
///   static variables will be completely separate from Base's.
///   
/// We could deal with each individual case by just using base_export or not,
/// but combined, we get this.
///   -- mrickert


#include "BasePch.hpp"
#include "sqratClassType.h"
#include "tHashTable.hpp"

namespace Sqrat
{
#if true
	/// \brief Local/internal to Base[.dll]

	Sig::tHashTable< Sig::Rtti::tClassId, StaticClassTypeData* > gStaticClassTypeDataByName;

	StaticClassTypeData& StaticClassTypeData::fGetStaticClassTypeDataByClassId( Sig::Rtti::tClassId cid )
	{
		StaticClassTypeData** instance = gStaticClassTypeDataByName.fFind( cid );

		if (!instance)
		{
			StaticClassTypeData* data = new StaticClassTypeData();
			data->mCopyFunc = 0;
			data->mInitialized = false;

			instance = gStaticClassTypeDataByName.fInsert( cid, data );
		}

		return **instance;
	}
#else
	/// \brief Local/internal to Base[.dll]
	Sig::tHashTable< const char*, StaticClassTypeData* > gStaticClassTypeDataByName;

	StaticClassTypeData& StaticClassTypeData::fGetStaticClassTypeDataByName( const char* typeName )
	{
		StaticClassTypeData** instance = gStaticClassTypeDataByName.fFind(typeName);

		if (!instance)
		{
			StaticClassTypeData* data = new StaticClassTypeData();
			data->mCopyFunc = 0;
			data->mInitialized = false;

			instance = gStaticClassTypeDataByName.fInsert( typeName, data );
		}

		return **instance;
	}
#endif
}
