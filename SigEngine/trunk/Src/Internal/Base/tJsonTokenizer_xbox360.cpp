//------------------------------------------------------------------------------
// \file tJsonTokenizer.hpp - 19 Oct 2012
// \author mrickert
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tJsonTokenizer.hpp"

namespace Sig
{
	const char* fPlaintextXJsonHresult( HRESULT hr )
	{
		switch( hr )
		{
#define S( json_e_hresult ) case json_e_hresult: return # json_e_hresult
		S( JSON_E_MISSING_CLOSING_QUOTE       );
		S( JSON_E_NOT_FIELDNAME_TOKEN         );
		S( JSON_E_INVALID_PARAMETER           );
		S( JSON_E_NUMBER_TOO_LONG             );
		S( JSON_E_BUFFER_TOO_SMALL            );
		S( JSON_E_MISSING_NAME_SEPARATOR      );
		S( JSON_E_UNEXPECTED_TOKEN            );
		S( JSON_E_UNEXPECTED_END_ARRAY        );
		S( JSON_E_UNEXPECTED_END_OBJECT       );
		S( JSON_E_INVALID_TOKEN               );
		S( JSON_E_UNEXPECTED_NAME_SEPARATOR   );
		S( JSON_E_UNEXPECTED_VALUE_SEPARATOR  );
		S( JSON_E_MISSING_END_OBJECT          );
		S( JSON_E_MISSING_END_ARRAY           );
		S( JSON_E_NOT_STRING_TOKEN            );
		S( JSON_E_MAX_NESTING_EXCEEDED        );
		S( JSON_E_UNEXPECTED_ESCAPE_CHARACTER );
		S( JSON_E_INVALID_ESCAPED_CHARACTER   );
		S( JSON_E_INVALID_UNICODE_ESCAPE      );
		S( JSON_E_INVALID_STRING_CHARACTER    );
		S( JSON_E_INVALID_NUMBER              );
		S( JSON_E_NOT_NUMBER_TOKEN            );
		S( JSON_E_NUMBER_OUT_OF_RANGE         );
		S( JSON_E_OUT_OF_BUFFER               );
		S( JSON_E_NOT_VALUE_TOKEN             );
		S( JSON_E_UNEXPECTED_BEGIN_OBJECT     );
		S( JSON_E_UNEXPECTED_BEGIN_ARRAY      );
		S( JSON_E_UNEXPECTED_BEGIN_FIELD      );
		S( JSON_E_UNEXPECTED_END_FIELD        );
		S( JSON_E_UNEXPECTED_VALUE            );
		S( JSON_E_INVALID_UTF8_STRING         );
#undef S
		default: return "JSON_E_???";
		}
	}

	tJsonTokenizer::tJsonTokenizer( const char* buffer, u32 bufferSize )
		: mReader( XJSONCreateReader( ) )
		, mOwnsReader( true )
	{
		const HRESULT hr = XJSONSetBuffer( mReader, buffer, bufferSize, FALSE );
		log_sigcheckfail( hr == S_OK, "XJSONSetBuffer failed with HRESULT: " << hr, fReleaseReader( ); return );
	}

	tJsonTokenizer::~tJsonTokenizer( )
	{
		fReleaseReader( );
	}

	b32 tJsonTokenizer::fReadToken( Json::tTokenType* tokenType, u32* tokenLength, u32* newBufferPos )
	{
		JSONTOKENTYPE	jtt;
		DWORD			tl;
		DWORD			nbp;
		const HRESULT hr = XJSONReadToken( mReader, &jtt, &tl, &nbp );

		if( hr != S_OK && hr != JSON_E_OUT_OF_BUFFER ) // JSON_E_OUT_OF_BUFFER is typical end-of-stream condition
			log_warning
				( "XJSONReadToken failed with result: " << fPlaintextXJsonHresult( hr )
				<< " (0x" << std::setw(8) << std::right << std::hex << hr << ")"
				);

		if( hr != S_OK )
			return false;

		if( tokenType )
			*tokenType = (Json::tTokenType)jtt;
		if( tokenLength )
			*tokenLength = (u32)tl;
		if( newBufferPos )
			*newBufferPos = (u32)nbp;

		return true;
	}

	b32 tJsonTokenizer::fGetTokenValue( char* buffer, u32 maxBufferChars ) const
	{
		const HRESULT hr = XJSONGetTokenValue( mReader, buffer, (DWORD)maxBufferChars );
		if( hr != S_OK )
			log_warning
				( "XJSONGetTokenValue failed with result: " << fPlaintextXJsonHresult( hr )
				<< " (0x" << std::setw(8) << std::right << std::hex << hr << ")"
				);
		return hr == S_OK;
	}

	tJsonTokenizer::tJsonTokenizer( HJSONREADER reader, b32 ownHandle )
		: mReader( reader )
		, mOwnsReader( ownHandle )
	{
	}

	void tJsonTokenizer::fReleaseReader( )
	{
		if( mReader && mOwnsReader )
			XJSONCloseReader( mReader );
		mReader = NULL;
		mOwnsReader = false;
	}
} // namespace Sig

#else

void fIgnoreNoSymbols_tJsonTokenizer_xbox360( ) { }

#endif // platform_xbox360
