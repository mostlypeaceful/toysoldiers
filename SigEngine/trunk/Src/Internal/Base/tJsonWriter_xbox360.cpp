//------------------------------------------------------------------------------
// \file tJsonWriter_xbox360.cpp - 14 June 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tJsonWriter.hpp"

namespace Sig
{
	//--------------------------------------------------------------------------------------
	// tJsonWriter
	//--------------------------------------------------------------------------------------
	tJsonWriter::tJsonWriter( )
	{
		mWriter = XJSONCreateWriter();
		sigassert( mWriter && "XJSONCreateWriter failed" );
	}

	tJsonWriter::~tJsonWriter( )
	{
		if( mWriter )
			XJSONCloseWriter( mWriter );
	}

	b32 tJsonWriter::fBeginObject( )
	{
		HRESULT result = XJSONBeginObject( mWriter );
		if( FAILED( result ) )
		{
			log_warning( "XJSONBeginObject failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fEndObject( )
	{
		HRESULT result = XJSONEndObject( mWriter );
		if( FAILED( result ) )
		{
			log_warning( "XJSONEndObject failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fBeginArray( )
	{
		HRESULT result = XJSONBeginArray( mWriter );
		if( FAILED( result ) )
		{
			log_warning( "XJSONBeginArray failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fEndArray( )
	{
		HRESULT result = XJSONEndArray( mWriter );
		if( FAILED( result ) )
		{
			log_warning( "XJSONEndArray failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fBeginField( const char* name )
	{
		HRESULT result = XJSONBeginField( mWriter, name, strlen( name ) );
		if( FAILED( result ) )
		{
			log_warning( "XJSONBeginField failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fEndField( )
	{
		HRESULT result = XJSONEndField( mWriter );
		if( FAILED( result ) )
		{
			log_warning( "XJSONEndField failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteField( const char* name, const char* value, u32 valueLen )
	{
		if( valueLen == ~0 )
			valueLen = strlen( value );

		HRESULT result = XJSONWriteFieldString( mWriter, name, strlen( name ), value, valueLen );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteFieldString failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteField( const wchar_t* name, const wchar_t * value, u32 valueLen )
	{
		if( valueLen == ~0 )
			valueLen = wcslen( value );

		HRESULT result = XJSONWriteFieldString( mWriter, name, wcslen( name ), value, valueLen );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteStringValue failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteField( const char* name, f64 value )
	{
		HRESULT result = XJSONWriteFieldNumber( mWriter, name, strlen( name ), value );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteFieldNumber failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteBooleanField( const char* name, b32 value )
	{
		HRESULT result = XJSONWriteFieldBoolean( mWriter, name, strlen( name ), ( value != 0 ) );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteFieldBoolean failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteNullField( const char* name )
	{
		HRESULT result = XJSONWriteFieldNull( mWriter, name, strlen( name ) );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteFieldNull failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteValue( const char* value, u32 valueLen )
	{
		if( valueLen == ~0 )
			valueLen = strlen( value );

		HRESULT result = XJSONWriteStringValue( mWriter, value, valueLen );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteStringValue failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteValue( const wchar_t* value, u32 valueLen )
	{
		if( valueLen == ~0 )
			valueLen = wcslen( value );

		HRESULT result = XJSONWriteStringValue( mWriter, value, valueLen );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteStringValue failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteValue( f64 value )
	{
		HRESULT result = XJSONWriteNumberValue( mWriter, value );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteNumberValue failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteBooleanValue( b32 value )
	{
		HRESULT result = XJSONWriteBooleanValue( mWriter, ( value != 0 ) );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteBooleanValue failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fWriteNullValue( )
	{
		HRESULT result = XJSONWriteNullValue( mWriter );
		if( FAILED( result ) )
		{
			log_warning( "XJSONWriteNullValue failed with result: " << result );
			return false;
		}
		return true;
	}

	b32 tJsonWriter::fGetBuffer( char * buffer, u32 * bufferSizeInOut )
	{
		log_sigcheckfail( bufferSizeInOut, "bufferSizeInOut is a *required* parameter!", return false );
		DWORD bufferSize = *bufferSizeInOut;
		HRESULT result = XJSONGetBuffer( mWriter, buffer, &bufferSize );
		*bufferSizeInOut = bufferSize;
		log_sigcheckfail( buffer, "Prefer fGetBufferSize( ) instead of passing a null buffer.", return false );

		if( FAILED( result ) )
		{
			log_warning( "XJSONGetBuffer failed with result: " << result );
			return false;
		}

		return true;
	}

	b32 tJsonWriter::fGetBuffer( wchar_t * buffer, u32 * bufferSizeInOut )
	{
		log_sigcheckfail( bufferSizeInOut, "bufferSizeInOut is a *required* parameter!", return false );
		DWORD bufferSize = *bufferSizeInOut;
		HRESULT result = XJSONGetBuffer( mWriter, buffer, &bufferSize );
		*bufferSizeInOut = bufferSize;
		log_sigcheckfail( buffer, "Prefer fGetBufferSize( ) instead of passing a null buffer.", return false );

		if( FAILED( result ) )
		{
			log_warning( "XJSONGetBuffer failed with result: " << result );
			return false;
		}

		return true;
	}

	u32 tJsonWriter::fGetBufferSize( ) const
	{
		DWORD bufferSize = 0;
		const HRESULT hr = XJSONGetBuffer( mWriter, (char*)NULL, &bufferSize );
		if( FAILED(hr) && hr != JSON_E_BUFFER_TOO_SMALL ) // JSON_E_BUFFER_TOO_SMALL appears to be the typical result when passing NULL for buffer.
			log_warning( "XJSONGetBuffer failed with result: " << hr );
		return (u32)bufferSize;
	}
}
#endif//#if defined( platform_xbox360 )
