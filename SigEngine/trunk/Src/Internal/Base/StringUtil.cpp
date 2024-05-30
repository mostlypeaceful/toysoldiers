#include "BasePch.hpp"
#include "tGameAppBase.hpp"

#ifdef platform_apple
#include <string.h>
#include <wchar.h>
#endif

namespace Sig { namespace StringUtil
{
	u32 fStrLen( const char* a )
	{
		return strlen( a );
	}
	u32 fStrLen( const wchar_t* a )
	{
		u32 n = 0;
		while( *a++ != L'\0' )
			++n;
		return n;
	}
	s32	fStricmp( const char* a, const char* b )
	{
#ifdef platform_apple
		return strcasecmp( a, b );
#else
		return _stricmp( a, b );
#endif
	}
	s32 fStrnicmp( const char* a, const char* b, u32 count )
	{
#ifdef platform_apple
		return strncasecmp(a,b,count);
#else
		return _strnicmp( a, b, count );
#endif
	}

	s32 fWStricmp( const wchar_t* a, const wchar_t* b )
	{
#ifdef platform_apple
		return wcscasecmp(a,b);
#else
		return _wcsicmp( a, b );
#endif
	}

	s32 fWStrnicmp( const wchar_t* a, const wchar_t* b, u32 count )
	{
#ifdef platform_apple
		return wcsncasecmp(a,b,count);
#else
		return _wcsnicmp( a, b, count );
#endif
	}

	std::string fAppend( const std::string& text, u32 number )
	{
		std::stringstream ss;
		ss << text << number;
		return ss.str( );
	}

	std::string fAppend( const char* text, u32 number )
	{
		std::stringstream ss;
		ss << text << number;
		return ss.str( );
	}

	const char* fStrStrI( const char* searchIn, const char* searchFor )
	{
		if( !searchIn || !searchFor )
			return 0;

		const size_t searchForLen = strlen( searchFor );

		for( ; *searchIn; ++searchIn )
		{
			if( fStrnicmp( searchIn, searchFor, searchForLen ) == 0 )
				return searchIn;
		}

		return 0;
	}

	const wchar_t* fWStrStrI( const wchar_t* searchIn, const wchar_t* searchFor )
	{
		if( !searchIn || !searchFor )
			return 0;

		const size_t searchForLen = wcslen( searchFor );

		for( ; *searchIn; ++searchIn )
		{
			if( fWStrnicmp( searchIn, searchFor, searchForLen ) == 0 )
				return searchIn;
		}

		return 0;
	}

	const char*	fFirstCharAfter( const char* searchIn, const char* searchFor )
	{
		if( !searchIn || !searchFor )
			return 0;

		const size_t searchForLen = strlen( searchFor );

		for( ; *searchIn; ++searchIn )
		{
			if( strncmp( searchIn, searchFor, searchForLen ) == 0 )
				return searchIn + searchForLen;
		}

		return 0;
	}

	const char* fReadLine( const char* searchIn )
	{
		if( !searchIn || !*searchIn ) return NULL;

		const char* result = strchr( searchIn, '\n' );

		if( !result ) return NULL;
		
		return result + 1;
	}

	const char* fReadUntilNonSpace( const char* searchIn )
	{
		if( !searchIn || !*searchIn ) return NULL;

		while( *searchIn == ' ' || *searchIn == '\t' ) ++searchIn;

		return searchIn;
	}

	const char* fReadUntilCharacter( const char* searchIn, char character )
	{
		if( !searchIn || !*searchIn ) return searchIn;

		while( *searchIn != character 
			&& *searchIn != 0 ) ++searchIn;

		return searchIn;
	}

	const char* fReadUntilNewLineOrCharacter( const char* searchIn, char character )
	{
		if( !searchIn || !*searchIn ) return searchIn;

		while( *searchIn != '\n' 
			&& *searchIn != character 
			&& *searchIn != 0 ) ++searchIn;

		return searchIn;
	}

	b32 fIsAnyOf( const char* options, char character )
	{
		char searchee[] = { character, '\0' };
		return fStrStrI( options, searchee ) != 0;
	}

	b32 fStartsWith( const char* searchIn, const char* prefix )
	{
		return strncmp( searchIn, prefix, strlen(prefix) ) == 0;
	}

	b32 fStartsWithI( const char* searchIn, const char* prefix )
	{
		return fStrnicmp( searchIn, prefix, strlen(prefix) ) == 0;
	}

	b32 fEndsWith( const char* searchIn, const char* postfix )
	{
		const u32 searcheeLen = fStrLen(searchIn);
		const u32 postfixLen = fStrLen(postfix);
		return postfixLen<=searcheeLen && strncmp( searchIn+searcheeLen-postfixLen, postfix, postfixLen ) == 0;
	}

	b32 fEndsWithI( const char* searchIn, const char* postfix )
	{
		const u32 searcheeLen = fStrLen(searchIn);
		const u32 postfixLen = fStrLen(postfix);
		return postfixLen<=searcheeLen && fStrnicmp( searchIn+searcheeLen-postfixLen, postfix, postfixLen ) == 0;
	}

	b32 fAndSearch( const char* searchIn, const tGrowableArray< std::string >& includes )
	{
		for( u32 i = 0; i < includes.fCount(); ++i )
		{
			if( StringUtil::fStrStrI( searchIn, includes[i].c_str() ) == NULL )
				return false;
		}

		return true;
	}

	b32 fNotSearch( const char* searchIn, const tGrowableArray< std::string >& excludes )
	{
		for( u32 i = 0; i < excludes.fCount(); ++i )
		{
			if( StringUtil::fStrStrI( searchIn, excludes[i].c_str() ) != NULL )
				return false;
		}

		return true;
	}

	void fSplitSearchParams( tGrowableArray< std::string >& includesOut, tGrowableArray< std::string >& excludesOut, const char* src )
	{
		StringUtil::fSplit( includesOut, src, " " );

		for( u32 i = 0; i < includesOut.fCount( ); ++i )
		{
			std::string thisString = includesOut[i];
			const char* cStr = thisString.c_str( );

			// Check if this is a string to exclude from search results.
			if( *thisString.begin( ) == '-' )
			{
				includesOut.fErase( i-- );

				// Only keep exclusions that are actually things.
				if( thisString.length( ) >= 2 )
					excludesOut.fPushBack( StringUtil::fFirstCharAfter( thisString.c_str( ), "-" ) );
			}
		}
	}

	void fStrCpyAdvancePtr( char*& dst, const char* src )
	{
		while(( *dst++ = *src++ ))
			;
		--dst;
	}

	void fStrCpyFromEnd( char * dest, const char * src, u32 destSize )
	{
		const u32 inLen = strlen( src );
		const u32 outLen = destSize - 1;

		u32 i = outLen < inLen ? inLen - outLen : 0;
		u32 o = 0;

		for( ; i < inLen && o < outLen; ++i, ++o )
			dest[ o ] = src[ i ];

		// Null terminate
		dest[ o < outLen ? o : outLen ] = 0;
	}

	void fSplit( tGrowableArray<std::string>& subStringsOut, const char* src, const char* symbolToSplitOn, b32 keepEmpty )
	{
		const u32 symbolLen = ( u32 )strlen( symbolToSplitOn );

		for( const char* found = fStrStrI( src, symbolToSplitOn ); found; found = fStrStrI( src, symbolToSplitOn ) )
		{
			if( keepEmpty || src != found ) // don't accept empty sub-strings
				subStringsOut.fPushBack( std::string( src, found ) );
			src = found + symbolLen;
		}

		if( src && *src )
			subStringsOut.fPushBack( std::string( src ) );
	}

	void fSplit( tGrowableArray<std::wstring>& subStringsOut, const wchar_t* src, const wchar_t* symbolToSplitOn, b32 keepEmpty )
	{
		const u32 symbolLen = ( u32 )wcslen( symbolToSplitOn );

		for( const wchar_t* found = fWStrStrI( src, symbolToSplitOn ); found; found = fWStrStrI( src, symbolToSplitOn ) )
		{
			if( keepEmpty || src != found ) // don't accept empty sub-strings
				subStringsOut.fPushBack( std::wstring( src, found ) );
			src = found + symbolLen;
		}

		if( src && *src )
			subStringsOut.fPushBack( std::wstring( src ) );
	}

	std::string fReplaceAllOf( std::string& str, const char* oldSymbol, const char* newSymbol )
	{
		if( str.length( )==0 )
			return str;

		const u32 oldSymbolLen = ( u32 )strlen( oldSymbol );

		tGrowableArray<std::string> parts;

		for( const char* found = fStrStrI( str.c_str( ), oldSymbol ); found; found = str.length( ) ? fStrStrI( &str[0], oldSymbol ) : NULL )
		{
			parts.fPushBack( std::string( str.c_str( ), found ) );
			str = std::string( found + oldSymbolLen, str.c_str( ) + str.length( ) );
		}

		parts.fPushBack( str );

		str = "";

		for( u32 ipart = 0; ipart < parts.fCount( ); ++ipart )
		{
			str += parts[ipart];
			if( newSymbol && ipart < parts.fCount( )-1 )
				str += newSymbol;
		}

		return str;
	}

	std::wstring fWReplaceAllOf( std::wstring& str, const wchar_t* oldSymbol, const wchar_t* newSymbol )
	{
		if( str.length( )==0 )
			return str;

		const u32 oldSymbolLen = ( u32 )wcslen( oldSymbol );

		tGrowableArray<std::wstring> parts;

		for( const wchar_t* found = fWStrStrI( str.c_str( ), oldSymbol ); found; found = str.length( ) ? fWStrStrI( &str[0], oldSymbol ) : NULL )
		{
			parts.fPushBack( std::wstring( str.c_str( ), found ) );
			str = std::wstring( found + oldSymbolLen, str.c_str( ) + str.length( ) );
		}

		parts.fPushBack( str );

		str = L"";

		for( u32 ipart = 0; ipart < parts.fCount( ); ++ipart )
		{
			str += parts[ipart];
			if( newSymbol && ipart < parts.fCount( )-1 )
				str += newSymbol;
		}

		return str;
	}

	std::string fRemoveAllOf( std::string& str, const char* oldSymbol )
	{
		return fReplaceAllOf( str, oldSymbol, NULL );
	}
	
	std::wstring fWRemoveAllOf( std::wstring& str, const wchar_t* oldSymbol )
	{
		return fWReplaceAllOf( str, oldSymbol, NULL );
	}

	std::string fReplaceLine( std::string& str, const char* newSymbol, u32 lineNumber )
	{
		if( str.length( )==0 )
			return str;

		//const u32 newSymbolLen = ( u32 )strlen( newSymbol );

		const char *endOfFirstPart = str.c_str( );
		for( u32 i = 1; i < lineNumber; ++i ) 
			endOfFirstPart = fReadLine( endOfFirstPart );

		const char* startOfSecondPart = fReadLine( endOfFirstPart );

		std::string firstPart( str.c_str( ), endOfFirstPart );
		std::string newLine( newSymbol );
		std::string secondPart( startOfSecondPart, str.c_str( ) + str.length( ) );

		str = firstPart + newLine + secondPart;

		return str;
	}
	
	std::string fReplaceNonWhiteSpaceWithWhiteSpace( std::string& str )
	{
		for( u32 i = 0; i < str.length( ); ++i )
		{
			char& c = str.at( i );

			if( c != ' ' && c != '\t' )
				c = ' ';
		}

		return str;
	}

	std::string fStripQuotes( std::string& str )
	{
		const char quote[] = { '"', 0 };
		const char nullStr[] = { 0 };
		return StringUtil::fReplaceAllOf( str, quote, nullStr );
	}

	b32 fCheckExtension( const char* path, const char* ext )
	{
		return fCheckExtension( std::string(path), ext );
	}

	b32 fCheckExtension( const std::string& path, const char* ext )
	{
		sigassert( ext );
		if( *ext == '.' )
			++ext; // start past the period, with the real extension string

		// check all extensions (i.e., this will successfully return true if your path
		// is a.b.c and your ext is .c, and it will also return true if your ext is .b.c.
		for( size_t i = path.find_last_of( "." ); i != std::string::npos; i = path.find_last_of( ".", i-1 ) )
		{
			if( i==path.length( )-1 )
				continue;
			if( fStricmp( path.c_str( ) + i + 1, ext ) == 0 )
				return true;
			if( i==0 )
				break;
		}

		return false;
	}

	std::string fStripExtension( const char* path )
	{
		std::string o = path;

		size_t lastSlash = o.find_last_of( "/\\" );
		if( lastSlash == std::string::npos )
		{
			lastSlash = 0;
		}

		size_t firstDot = o.find( ".", lastSlash );
		if( firstDot == std::string::npos )
			return o;

		o.resize( firstDot );
		return o;
	}

	std::string	fGetExtension( const char* path, b32 includeDot )
	{
		std::string o = fNameFromPath( path );

		size_t firstDot = o.find( "." );
		if( firstDot == std::string::npos )
			return o;

		return &o[ firstDot + ( includeDot ? 0 : 1 ) ];
	}

	std::string fNameFromPath( const char* path, b32 stripExt )
	{
		std::string o = path;

		size_t lastSlash = o.find_last_of( "/\\" );
		if( lastSlash == std::string::npos )
			return o;

		o = std::string( &o[ lastSlash+1 ] );

		if( stripExt )
			o = fStripExtension( o.c_str( ) );

		return o;
	}

	std::string fDirectoryFromPath( const char* path )
	{
		std::string o = path;

		size_t lastSlash = o.find_last_of( "/\\" );
		if( lastSlash == std::string::npos )
			return o;

		o.resize( lastSlash+1 );
		return o;
	}

	std::string fUpNDirectories( const char* path, u32 N )
	{
		std::string o = path;

		if( o.length( ) == 0 )
			return o;

		for( u32 i = 0; i < N; ++i )
		{
			size_t lastSlash = o.find_last_of( "/\\" );
			while( lastSlash == o.size( )-1 )
			{
				o.resize( o.size( )-1 );
				lastSlash = o.find_last_of( "/\\" );
			}
			if( lastSlash == std::string::npos )
				return "";

			o.resize( lastSlash );
		}

		return o;
	}

	std::string	fTrimTrailingSlashes( const char* path )
	{
		std::string s = path;
		while( s.size( ) > 0 && ( s[s.size( )-1] == '\\' || s[s.size( )-1] == '/' ) )
			s.resize( s.size( ) - 1 );
		return s;
	}


	std::string fTrimOversizePath( const char* pathText, u32 maxChars, u32 pathTextLen )
	{
		if( !pathTextLen )
			pathTextLen = ( u32 )strlen( pathText );
		if( pathTextLen > maxChars )
		{
			std::stringstream ss;
			ss << "(...)" << ( pathText + pathTextLen - maxChars );
			return ss.str( );
		}

		return pathText;
	}

	std::string fEatWhiteSpace( const char* text, b32 eatFromEnd, b32 eatFromStart )
	{
		if( eatFromStart )
		{
			while( *text && isspace( *text ) )
				++text;
		}

		if( !eatFromEnd )
			return std::string( text );

		const char* end = text;
		while( *end ) ++end;

		while( end != text && isspace( *(end-1) ) )
			--end;

		return std::string( text, end );
	}

	std::string fEatWhiteSpace( const std::string& text, b32 eatFromEnd, b32 eatFromStart )
	{
		return fEatWhiteSpace( text.c_str( ), eatFromEnd, eatFromStart );
	}

	std::string	fAddCommaEvery3Digits( const char* numberValue, const char* prepend, const char* append, const char* commaValue )
	{
		const s32 ogStrLen = ( u32 )strlen( numberValue );

		std::stringstream ss;

		if( prepend )
			ss << prepend;

		for( s32 icounter = ogStrLen; *numberValue; ++numberValue, --icounter )
		{
			if( icounter != ogStrLen && icounter % 3 == 0 )
				ss << commaValue;
			ss << *numberValue;
		}

		if( append )
			ss << append;

		return ss.str( );
	}

	std::string fTrim( const char* text, const char* trimchars, b32 trimBegin, b32 trimEnd )
	{
		if( trimBegin )
		{
			while( *text != '\0' && fIsAnyOf( trimchars, *text ) )
				++text;
		}

		if( *text == '\0' )
			return std::string( );
		
		if( !trimEnd )
			return std::string( text );

		const char* const firstKeep = text;
		const char* lastKeep = firstKeep;
		++text;

		while( *text != '\0' )
		{
			if( !fIsAnyOf( trimchars, *text ) )
				lastKeep = text;
			++text;
		}

		return std::string( firstKeep, lastKeep+1 );
	}

	std::string fAddCommaEvery3DigitsScript( const char* numberValue )
	{
		std::stringstream thousandsSeparator;
		thousandsSeparator << tGameAppBase::fInstance( ).fGetLocalizedThousandsSeparator( );
		return StringUtil::fAddCommaEvery3Digits( numberValue, 0, 0, thousandsSeparator.str( ).c_str( ) );
	}

	std::string fToString( f32 value, u32 decimals )
	{
		std::stringstream ss;
		ss.setf( std::ios::fixed, std::ios::floatfield );
		ss.precision( decimals );
		ss << value;
		return ss.str( );
	}

	std::string fToString( s32 value )
	{
		std::stringstream ss;
		ss << value;
		return ss.str( );
	}

	std::string fToString( u32 value )
	{
		std::stringstream ss;
		ss << value;
		return ss.str( );
	}

	std::string fToString( s64 value )
	{
		std::stringstream ss;
		ss << value;
		return ss.str( );
	}

	std::string fToString( u64 value )
	{
		std::stringstream ss;
		ss << value;
		return ss.str( );
	}

	std::wstring fStringToWString( const std::string& text )
	{
		std::wstring ws;
		ws.assign( text.begin( ), text.end( ) );
		return ws;
	}

	std::wstring fStringToWString( const char* text )
	{
		std::wstring ws;
		ws.assign( text, text + strlen( text ) );
		return ws;
	}

	std::string fWStringToString( const std::wstring& text )
	{
		std::string s;
		s.resize( text.size( ) );
		for( u32 i = 0; i < text.size( ); ++i )
			s.at( i ) = ( char )text.at( i );

		return s;
	}
	
	std::string fToLower( const std::string& text )
	{
		std::string o = text;
		std::transform( o.begin( ), o.end( ), o.begin( ), tolower );
		return o;
	}

	std::string fToUpper( const std::string& text )
	{
		std::string o = text;
		std::transform( o.begin( ), o.end( ), o.begin( ), toupper );
		return o;
	}

	std::wstring fMultiByteToWString( const std::string& s )
	{
#if defined( platform_msft )
		// compute required number of wide characters
		const int reqLen = MultiByteToWideChar( CP_UTF8, 0, s.c_str( ), s.length( ), 0, 0 );

		// error of some sort, just expand the input string
		if( reqLen <= 0 )
			return StringUtil::fStringToWString( s );

		// now allocate wide-char buffer and convert input string
		tDynamicArray<wchar_t> buf( reqLen );
		const int actLen = MultiByteToWideChar( CP_UTF8, 0, s.c_str( ), s.length( ), buf.fBegin( ), buf.fCount( ) );

		// validate and return wstring
		sigassert( reqLen == actLen );
		return std::wstring( buf.fBegin( ), buf.fEnd( ) );
#else
		return fStringToWString( s );
#endif
	}

	std::string fWStringToMultiByte( const std::wstring& ws )
	{
#if defined( platform_msft )
		// compute required number of wide characters
		const int reqLen = WideCharToMultiByte( CP_UTF8, 0, ws.c_str( ), ws.length( ), 0, 0, 0, 0 );

		// error of some sort, just truncate the input string
		if( reqLen <= 0 )
			return StringUtil::fWStringToString( ws );

		// now allocate regular-char buffer and convert input string
		tDynamicArray<char> buf( reqLen );
		const int actLen = WideCharToMultiByte( CP_UTF8, 0, ws.c_str( ), ws.length( ), buf.fBegin( ), buf.fCount( ), 0, 0 );

		// validate and return multi-byte string
		sigassert( reqLen == actLen );
		return std::string( buf.fBegin( ), buf.fEnd( ) );
#else
		return fWStringToString( ws );
#endif
	}

	void fExportScriptInterface( tScriptVm& vm )
	{
		vm.fNamespace(_SC("StringUtil"))
			.Func< std::string (*)( f32, u32 ) >(_SC( "FloatToString"), &fToString)
			.Func(_SC("AddCommaEvery3Digits"), &fAddCommaEvery3DigitsScript)
			;
	}

}}

