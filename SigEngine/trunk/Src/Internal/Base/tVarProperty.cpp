#include "BasePch.hpp"
#include "tVarProperty.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tVarPropertyTag
	//------------------------------------------------------------------------------
	const Rtti::tClassId tVarPropertyTag::cCid = Rtti::fGetClassId<tStringPtr>( );

	//------------------------------------------------------------------------------
	// tVarPropertyBag
	//------------------------------------------------------------------------------
	void tVarPropertyBag::fAdd( const tVarPropertyPtr& prop )
	{
		sigassert( prop && "Cannot add null properties to the property bag!" );
		sigassert( !fFind( prop->fName( ) ) && "Cannot add duplicate properties to bag" );

		mProperties.fPushBack( prop );
	}

	//------------------------------------------------------------------------------
	b32 tVarPropertyBag::fRemove( const tStringPtr& name )
	{
		const s32 index = fIndexOf( name );
		if( index >= 0 )
		{
			mProperties.fErase( index );
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	tVarProperty* tVarPropertyBag::fFind( const tStringPtr& name ) const
	{
		const s32 index = fIndexOf( name );
		if( index >= 0 )
			return mProperties[ index ].fGetRawPtr( );

		return NULL;
	}

	//------------------------------------------------------------------------------
	s32 tVarPropertyBag::fIndexOf( const tStringPtr& name ) const
	{
		const u32 cPropCount = mProperties.fCount( );
		for( u32 p = 0; p < cPropCount; ++p )
		{
			if( mProperties[ p ]->fName( ) == name )
				return p;
		}

		return -1;
	}
}

