//------------------------------------------------------------------------------
// \file tJsonWriter_fallback.cpp - 01 Oct 2012
// \author mrickert
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#if !defined( platform_xbox360 )

#include "tJsonWriter.hpp"

namespace Sig
{
	namespace
	{
		/// \brief	Takes a raw C string (either wide or narrow) and appends a JSON encoded string (including quotes and escape codes) to buffer.
		/// \param	buffer	The buffer to append to.
		/// \param	string	The raw C-style string.
		/// \param	length	The length of the C-style string.  ~0u indicates the length should be inferred from the NUL terminal.
		template< typename tChar >
		void fWriteEscapedJsonString( tGrowableArray< char >& buffer, const tChar* string, u32 length )
		{
			if( length == ~0u )
				length = StringUtil::fStrLen( string );

			buffer.fPushBack( '\"' );
			for( u32 i = 0; i < length; ++i, ++string )
			{
				log_sigcheckfail( *string < 0x80, "Unicode not (yet) supported, convert to \uXXXX escape codes", buffer.fPushBack('?'); continue );
				switch( *string )
				{
				case '\\':
					buffer.fPushBack('\\');
					buffer.fPushBack('\\');
					break;
				case '\"':
					buffer.fPushBack('\\');
					buffer.fPushBack('\"');
					break;
				case '\0':
					buffer.fPushBack('\\');
					buffer.fPushBack('0');
					break;
				default:
					buffer.fPushBack( (char)*string );
					break;
				}
			}
			buffer.fPushBack( '\"' );
		}

		u32 fGetBufferSizeImpl( const tGrowableArray<char>& mWriter )
		{
			return mWriter.fCount( )
				- ( ( mWriter.fCount( ) && mWriter.fBack( ) == ',' ) ? 1 : 0 ) // don't count any trailing commas
				+ 1 // do count trailing '\0'
				;
		}

		/// \brief 	Write out the contents of mWriter to buffer & bufferSizeInOut.
		/// \param	mWriter			The buffer to read from.
		/// \param	buffer			Required.  The contents of mWriter are copied into this buffer, minus any trailing comma, plus a '\0' terminator.
		/// \param	bufferSizeInOut	Required.  In: The size of the supplied buffer in characters.  Out: Number of characters required to store the entire buffer, including the '\0' terminator.
		/// \return	If buffer was written to (e.g. the supplied buffer was large enough.)
		template< typename tChar >
		b32 fGetBufferImpl( const tGrowableArray<char>& mWriter, tChar* buffer, u32* bufferSizeInOut )
		{
			sigcheckfail( bufferSizeInOut, return false );
			const u32 originalRoom = *bufferSizeInOut;
			const u32 requiredRoom = *bufferSizeInOut = fGetBufferSizeImpl( mWriter );
			sigcheckfail( buffer, return false );
			sigcheckfail( originalRoom >= requiredRoom, return false );

			for( u32 i = 0; i < requiredRoom - 1; ++i )
				buffer[ i ] = (tChar)mWriter[ i ];
			buffer[ requiredRoom - 1 ] = '\0';
			return true;
		}
	}

	//--------------------------------------------------------------------------------------
	// tJsonWriter
	//--------------------------------------------------------------------------------------
	tJsonWriter::tJsonWriter( )
	{
	}

	tJsonWriter::~tJsonWriter( )
	{
	}

	b32 tJsonWriter::fBeginObject( )
	{
		mWriter.fPushBack( '{' );
		return true;
	}

	b32 tJsonWriter::fEndObject( )
	{
		while( mWriter.fCount( ) && mWriter.fBack( ) == ',' )
			mWriter.fPopBack( );
		mWriter.fPushBack( '}' );
		mWriter.fPushBack( ',' );
		return true;
	}

	b32 tJsonWriter::fBeginArray( )
	{
		mWriter.fPushBack( '[' );
		return true;
	}

	b32 tJsonWriter::fEndArray( )
	{
		while( mWriter.fCount( ) && mWriter.fBack( ) == ',' )
			mWriter.fPopBack( );
		mWriter.fPushBack( ']' );
		mWriter.fPushBack( ',' );
		return true;
	}

	b32 tJsonWriter::fBeginField( const char* name )
	{
		fWriteEscapedJsonString( mWriter, name, strlen(name) );
		mWriter.fPushBack( ':' );
		return true;
	}

	b32 tJsonWriter::fEndField( )
	{
		mWriter.fPushBack( ',' );
		return true;
	}

	b32 tJsonWriter::fWriteField( const char* name, const char* value, u32 valueLen )
	{
		return fBeginField( name ) && fWriteValue( value, valueLen );
	}

	b32 tJsonWriter::fWriteField( const wchar_t* name, const wchar_t * value, u32 valueLen )
	{
		return fBeginField( StringUtil::fWStringToString(name).c_str( ) ) && fWriteValue( value, valueLen );
	}

	b32 tJsonWriter::fWriteField( const char* name, f64 value )
	{
		return fBeginField( name ) && fWriteValue( value );
	}

	b32 tJsonWriter::fWriteBooleanField( const char* name, b32 value )
	{
		return fBeginField( name ) && fWriteBooleanValue( value );
	}

	b32 tJsonWriter::fWriteNullField( const char* name )
	{
		return fBeginField( name ) && fWriteNullValue( );
	}

	b32 tJsonWriter::fWriteValue( const char* value, u32 valueLen )
	{
		fWriteEscapedJsonString( mWriter, value, valueLen );
		mWriter.fPushBack( ',' );
		return true;
	}

	b32 tJsonWriter::fWriteValue( const wchar_t* value, u32 valueLen )
	{
		fWriteEscapedJsonString( mWriter, value, valueLen );
		mWriter.fPushBack( ',' );
		return true;
	}

	b32 tJsonWriter::fWriteValue( f64 value )
	{
		std::stringstream ss; ss << value;
		mWriter.fInsert( mWriter.fCount( ), ss.str().c_str(), (u32)ss.tellp() );
		mWriter.fPushBack( ',' );
		return true;
	}

	b32 tJsonWriter::fWriteBooleanValue( b32 value )
	{
		const char* v = value ? "true" : "false";
		mWriter.fInsert( mWriter.fCount( ), v, strlen(v) );
		mWriter.fPushBack( ',' );
		return true;
	}

	b32 tJsonWriter::fWriteNullValue( )
	{
		const char* v = "null";
		mWriter.fInsert( mWriter.fCount( ), v, strlen(v) );
		mWriter.fPushBack( ',' );
		return true;
	}

	b32 tJsonWriter::fGetBuffer( char * buffer, u32 * bufferSizeInOut )
	{
		return fGetBufferImpl( mWriter, buffer, bufferSizeInOut );
	}

	b32 tJsonWriter::fGetBuffer( wchar_t * buffer, u32 * bufferSizeInOut )
	{
		return fGetBufferImpl( mWriter, buffer, bufferSizeInOut );
	}

	u32 tJsonWriter::fGetBufferSize( ) const
	{
		return fGetBufferSizeImpl( mWriter );
	}
}

// !defined( platform_xbox360 )
#else
// defined( platform_xbox360 )

void fNoHttptJsonWriterFallbackLinkerWarningsPlease() {}

#endif // platform_msft
