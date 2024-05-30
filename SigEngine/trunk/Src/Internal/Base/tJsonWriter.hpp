//------------------------------------------------------------------------------
// \file tJsonWriter.hpp - 14 June 2012
// \author colins
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonWriter__
#define __tJsonWriter__

#ifdef platform_xbox360
	#include <XJSON.h>
#endif

namespace Sig
{

#ifdef platform_xbox360
	#define json_writer_has_handle
	typedef HJSONWRITER tJsonWriterHandle;
#endif

	/// \class	tJsonWriter
	/// \brief	Used for writing JSON objects to a memory buffer
	class tJsonWriter : public tRefCounter
	{
		debug_watch( tJsonWriter );
		declare_uncopyable( tJsonWriter );
	public:
		tJsonWriter( );
		~tJsonWriter( );

		b32 fBeginObject( );
		b32 fEndObject( );

		b32 fBeginArray( );
		b32 fEndArray( );

		b32 fBeginField( const char* name );
		b32 fEndField( );

		b32 fWriteField( const char* name, const char* value, u32 valueLen = ~0 );
		b32 fWriteField( const wchar_t* name, const wchar_t * value, u32 valueLen = ~0 );
		b32 fWriteField( const char* name, const tStringPtr& value );
		b32 fWriteField( const char* name, const std::string& value );
		b32 fWriteField( const char* name, f64 value );
		b32 fWriteBooleanField( const char* name, b32 value );
		b32 fWriteNullField( const char* name );

		b32 fWriteValue( const char* value, u32 valueLen = ~0 );
		b32 fWriteValue( const wchar_t * value, u32 valueLen = ~0 );
		b32 fWriteValue( const tStringPtr& value );
		b32 fWriteValue( const std::string& value );
		b32 fWriteValue( f64 value );
		b32 fWriteValueCompressed( f64 value ); //So that our floats don't take up a ton of characters in the JSON string buffer
		b32 fWriteBooleanValue( b32 value );
		b32 fWriteNullValue( );

		/// \brief	Take the values written so far and read out the resulting JSON, given a buffer and size of that buffer.
		/// \param	buffer			Required.  The ASCII/UTF8 (char) or UTF16 (wchar_t) character buffer to write the JSON to.
		/// \param	bufferSizeInOut	Required.  In:  The size of the buffer in characters.  Out: The number of characters required to store the entire buffer, including the '\0' terminal.  May be larger than original input!
		/// \return	True if the buffer was large enough and had all JSON fully written to it, false otherwise.
		b32 fGetBuffer( char * buffer, u32 * bufferSizeInOut );
		b32 fGetBuffer( wchar_t * buffer, u32 * bufferSizeInOut );

		/// \brief	Get the size of the buffer as fGetBuffer will try and write out (including the '\0' terminal.)
		u32 fGetBufferSize( ) const;

#ifdef json_writer_has_handle
		/// \note	NOTE WELL: Not available on all platforms!  Platform specific!
		/// \brief	Get the underlying native handle for interop with e.g. SmartGlass.
		tJsonWriterHandle fHandle( ) const { return mWriter; }
#endif

		static f64 fToCompressedValue( f64 value );
		static f64 fFromCompressedValue( f64 value );


	private:

#ifdef json_writer_has_handle
		tJsonWriterHandle mWriter;

#else
		// No underlying native implementation handle to worry about.
		// Store and write directly to a buffer instead.

		/// \brief	Ongoing buffer of Low-ASCII/UTF8 JSON.  Likely contains extra trailing commas after writing
		///			object / array elements.  These are assumed to be removed by fEnd* and fGetBuffer if
		///			unnecessary or undesired.  E.G. before fEndArray: "[1,2,"   After: "[1,2]"
		tGrowableArray< char > mWriter;

#endif
	};


	typedef tRefCounterPtr< tJsonWriter > tJsonWriterPtr;
}
#endif//__tJsonWriter__
