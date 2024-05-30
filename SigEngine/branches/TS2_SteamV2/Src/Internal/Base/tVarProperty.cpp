#include "BasePch.hpp"
#include "tVarProperty.hpp"

namespace Sig
{

#ifdef _var_property_use_hash_table_

	void tVarPropertyTable::fAddClones( tVarPropertyTable& addTo ) const
	{
		addTo.mTagMask |= mTagMask;
		for( tVarPropertyTableBase::tConstIterator i = fBegin( ), end = fEnd( ); i != end; ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			tVarPropertyPtr* find = addTo.fFind( i->mValue->fName( ) );
			if( find )
				addTo.fRemove( find );
			addTo.fInsert( i->mValue->fName( ), tVarPropertyPtr( i->mValue->fClone( ) ) );
		}
	}

#else//_var_property_use_hash_table_

	void tVarPropertyTable::fAddClones( tVarPropertyTable& addTo ) const
	{
		addTo.mTagMask |= mTagMask;
		for( tVarPropertyTableBase::tConstIterator i = fBegin( ), end = fEnd( ); i != end; ++i )
		{
			tVarPropertyPtr* find = addTo.fFind( (*i)->fName( ) );
			if( find )
				addTo.fRemove( (*find)->fName( ) );
			addTo.fInsert( (*i)->fName( ), tVarPropertyPtr( (*i)->fClone( ) ) );
		}
	}

	void tVarPropertyTable::fInsert( const tStringPtr& name, const tVarPropertyPtr& prop )
	{
		sigassert( !prop.fNull( ) && !prop->fName( ).fNull( ) );
		fPushBack( prop );
	}

	tVarPropertyPtr* tVarPropertyTable::fFind( const tStringPtr& name )
	{
		tVarPropertyPtr* items = fBegin( );
		for( u32 i = fCount( ); i > 0; --i )
		{
			if( items[ i - 1 ]->fName( ) == name )
				return &items[ i - 1 ];
		}
		return 0;
	}

	b32 tVarPropertyTable::fRemove( const tStringPtr& name )
	{
		tVarPropertyPtr* items = fBegin( );
		for( u32 i = fCount( ); i > 0; --i )
		{
			if( items[ i - 1 ]->fName( ) == name )
			{
				fErase( i - 1 );
				return true;
			}
		}
		return false;
	}

	void tVarPropertyTable::fRemove( tVarPropertyPtr* prop )
	{
		fErase( fPtrDiff( prop, fBegin( ) ) );
	}

#endif//_var_property_use_hash_table_

}

