//------------------------------------------------------------------------------
// \file tJsonWriter.cpp - 01 Oct 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tJsonWriter.hpp"

namespace Sig
{
	b32 tJsonWriter::fWriteField( const char* name, const tStringPtr& value )
	{
		return fWriteField( name, value.fCStr( ), (u32)value.fLength( ) );
	}

	b32 tJsonWriter::fWriteField( const char* name, const std::string& value )
	{
		return fWriteField( name, value.c_str( ), (u32)value.length( ) );
	}

	b32 tJsonWriter::fWriteValue( const tStringPtr& value )
	{
		return fWriteValue( value.fCStr( ), (u32)value.fLength( ) );
	}

	b32 tJsonWriter::fWriteValue( const std::string& value )
	{
		return fWriteValue( value.c_str( ), (u32)value.length( ) );
	}

	b32 tJsonWriter::fWriteValueCompressed( f64 value )
	{
		return fWriteValue( fToCompressedValue( value ) );
	}

	namespace
	{
		const f64 cCompressionFactor = 100.0f;
		s64 fRoundAndTruncate( f64 v )
		{
			return (s64)floor( v + 0.5 );
		}
	}

	f64 tJsonWriter::fToCompressedValue( f64 value )
	{
		return (f64)fRoundAndTruncate( value * cCompressionFactor );
	}

	f64 tJsonWriter::fFromCompressedValue( f64 value )
	{
		return value / cCompressionFactor;
	}
}
