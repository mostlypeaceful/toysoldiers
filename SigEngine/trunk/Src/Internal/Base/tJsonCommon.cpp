//------------------------------------------------------------------------------
// \file tJsonCommon.cpp - 03 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tJsonCommon.hpp"

namespace Sig { namespace Json
{
	b32 fHasValue( tTokenType type )
	{
		switch( type )
		{
		case cTokenString:
		case cTokenNumber:
		case cTokenTrue:
		case cTokenFalse:
		case cTokenNull:
		case cTokenFieldName:
			return true;
		default:
			return false;
		}
	}

	const char* fToDebugString( tTokenType type )
	{
		switch( type )
		{
#define S( type ) case type: return #type
		S( cTokenBeginArray );
		S( cTokenEndArray );
		S( cTokenBeginObject );
		S( cTokenEndObject );
		S( cTokenString );
		S( cTokenNumber );
		S( cTokenTrue );
		S( cTokenFalse );
		S( cTokenNull );
		S( cTokenFieldName );
		S( cTokenNameSeparator );
		S( cTokenObjectSeparator );
		S( cTokenValueSeparator );
		S( cTokenInvalid );
#undef S
		default: return "cToken?????";
		}
	}

	s32 fScopeDeltaOf( tTokenType type )
	{
		switch( type )
		{
		case cTokenBeginArray:
		case cTokenBeginObject:
			return +1;
		case cTokenEndArray:
		case cTokenEndObject:
			return -1;
		default:
			return 0;
		}
	}
}} // namespace Sig::Json
