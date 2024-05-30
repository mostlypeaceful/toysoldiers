//------------------------------------------------------------------------------
// \file tJsonCommon.hpp - 18 July 2012
// \author colins, mrickert
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonCommon__
#define __tJsonCommon__

#ifdef platform_xbox360
#include <XJSON.h>
#endif

namespace Sig { namespace Json
{
	enum tTokenType
	{
#ifdef platform_xbox360
															///	Example		Description
		cTokenBeginArray		= Json_BeginArray,			///< '['		JSON Array opening token
		cTokenEndArray			= Json_EndArray,			///< ']'		JSON Array closing token
		cTokenBeginObject		= Json_BeginObject,			///< '{'		JSON Object opening token
		cTokenEndObject			= Json_EndObject,			///< '}'		JSON Object closing token
		cTokenString			= Json_String,				///< "foobar"	JSON String Literal
		cTokenNumber			= Json_Number,				///< 123.45		JSON Number Literal (float or int!)
		cTokenTrue				= Json_True,				///< true		JSON boolean literal
		cTokenFalse				= Json_False,				///< false		JSON boolean literal
		cTokenNull				= Json_Null,				///< null		JSON reference literal
		cTokenFieldName			= Json_FieldName,			///< "foobar":	JSON object field name
		cTokenNameSeparator		= Json_NameSeparator,		///< N/A		Never actually returned by XJSON API!
		cTokenObjectSeparator	= Json_ObjectSeparator,		///< ','		JSON Object element seperator
		cTokenValueSeparator	= Json_ValueSeparator,		///< ','		JSON Array element seperator

		cTokenInvalid = 255,
#else
		cTokenBeginArray,
		cTokenEndArray,
		cTokenBeginObject,
		cTokenEndObject,
		cTokenString,
		cTokenNumber,
		cTokenTrue,
		cTokenFalse,
		cTokenNull,
		cTokenFieldName,
		cTokenNameSeparator,
		cTokenObjectSeparator,
		cTokenValueSeparator,

		cTokenInvalid,
#endif
	};

	/// \brief Returns true if this token type should generally carry a value (e.g. String, Number, True, False, Null, or FieldName)
	b32 fHasValue( tTokenType type );

	/// \brief Pretty name of the tTokenType for diagnostic purposes.
	const char* fToDebugString( tTokenType type );

	/// \brief The change in number of nested scopes that occurs when this token is encountered.
	/// \return +1 for cTokenBegin*, -1 for cTokenEnd*, 0 otherwise.
	s32 fScopeDeltaOf( tTokenType type );

}} // namespace Sig::Json

#endif //ndef __tJsonCommon__
