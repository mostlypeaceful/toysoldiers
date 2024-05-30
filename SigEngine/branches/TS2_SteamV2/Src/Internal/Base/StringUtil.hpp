#ifndef __StringUtil__
#define __StringUtil__

namespace Sig { namespace StringUtil
{
	base_export s32			fStricmp( const char* a, const char* b );
	base_export s32			fStrnicmp( const char* a, const char* b, u32 count );
	base_export s32			fWStrnicmp( const wchar_t* a, const wchar_t* b, u32 count );
	base_export std::string fAppend( const std::string& text, u32 number );
	base_export std::string fAppend( const char* text, u32 number );
	base_export void		fStrCpyAdvancePtr( char*& dst, const char* src );
	base_export void		fStrCpyFromEnd( char * dest, const char * src, u32 destSize );
	base_export void		fSplit( tGrowableArray<std::string>& subStringsOut, const char* src, const char* symbolToSplitOn, b32 keepEmpty = false );
	base_export std::string fReplaceAllOf( std::string& str, const char* oldSymbol, const char* newSymbol );
	base_export std::wstring fWReplaceAllOf( std::wstring& str, const wchar_t* oldSymbol, const wchar_t* newSymbol );
	base_export std::string fRemoveAllOf( std::string& str, const char* oldSymbol );
	base_export std::wstring fWRemoveAllOf( std::wstring& str, const wchar_t* oldSymbol );
	base_export std::string fReplaceLine( std::string& str, const char* newSymbol, u32 lineNumber );
	base_export std::string fReplaceNonWhiteSpaceWithWhiteSpace( std::string& str );
	base_export std::string fStripQuotes( std::string& str );
	base_export std::string fEatWhiteSpace( const char* text, b32 eatFromEnd = true, b32 eatFromStart = true );
	base_export std::string fEatWhiteSpace( const std::string& text, b32 eatFromEnd = true, b32 eatFromStart = true );
	base_export std::string	fAddCommaEvery3Digits( const char* numberValue, const char* prepend = 0, const char* append = 0, const char* commaValue = "," );
	
	// Conversion
	base_export std::string  fToString( f32 value, u32 decimals = 2 );
	base_export std::string  fToString( s32 value );
	base_export std::string  fToString( u32 value );
	base_export std::string  fToString( s64 value );
	base_export std::string  fToString( u64 value );
	base_export std::wstring fStringToWString( const std::string& text );
	base_export std::wstring fStringToWString( const char* text );
	base_export std::string  fWStringToString( const std::wstring& text );
	base_export std::wstring fMultiByteToWString( const std::string& s );
	base_export std::string  fWStringToMultiByte( const std::wstring& ws );
	base_export std::string  fToLower( const std::string& text );
	base_export std::string  fToUpper( const std::string& text );

	// Character searching
	base_export const char*	fStrStrI( const char* searchIn, const char* searchFor );
	base_export const wchar_t*	fWStrStrI( const wchar_t* searchIn, const wchar_t* searchFor );
	base_export const char*	fFirstCharAfter( const char* searchIn, const char* searchFor );
	base_export const char* fReadLine( const char* searchIn ); // Returns pointer to first character in first line after searchIn
	base_export const char* fReadUntilNonSpace( const char* searchIn ); // Returns pointer to first non space or tab character after and including searchIn
	base_export const char* fReadUntilCharacter( const char* searchIn, char character ); //returns poitner to first instance of character, or the null terminator
	base_export const char* fReadUntilNewLineOrCharacter( const char* searchIn, char character ); //Returns a pointer to either the first instance of character, the next newline, or the null terminator if reached

	// File path manipulation
	base_export b32			fCheckExtension( const char* path, const char* ext );
	base_export b32			fCheckExtension( const std::string& path, const char* ext );
	base_export std::string	fStripExtension( const char* path );
	base_export std::string	fGetExtension( const char* path, b32 includeDot = true );
	base_export std::string	fNameFromPath( const char* path, b32 stripExt = false );
	base_export std::string	fDirectoryFromPath( const char* path );
	base_export std::string	fUpNDirectories( const char* path, u32 N );
	base_export std::string	fTrimTrailingSlashes( const char* path );
	base_export std::string fTrimOversizePath( const char* pathText, u32 maxChars, u32 pathTextLen = 0 );

	// Script Export
	void fExportScriptInterface( tScriptVm& vm );

}}

#endif//__StringUtil__
