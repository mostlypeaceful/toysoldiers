#include "BasePch.hpp"
#include "tLocalizationFile.hpp"
#include <iomanip>
#include "tGameAppBase.hpp"

namespace Sig
{
	devvar( bool, Debug_Localization_ShowMissingId, false );

	// we rely on this being true
#define wchart_is_sizeof_u16( ) ( sizeof( u16 ) == sizeof( wchar_t ) )

	namespace
	{
		static const tLocalizedString cEmptyString = tLocalizedString( L"" );
		static const tLocalizedString cErrorString = tLocalizedString( L"string not in loc file" );
		static const tLocalizedString cNullString = tLocalizedString( L"(null) loc string" );
	}

	tLocalizedString::~tLocalizedString( )
	{
	}

	tLocalizedString tLocalizedString::fConstructMoneyString( const char* moneyString )
	{
		std::stringstream thousandsSeparator;
		thousandsSeparator << tGameAppBase::fInstance( ).fGetLocalizedThousandsSeparator( );
		const std::string convertedMoneyString = StringUtil::fAddCommaEvery3Digits( moneyString, "$", 0, thousandsSeparator.str( ).c_str( ) );
		return tLocalizedString::fFromCString( convertedMoneyString.c_str( ) );
	}

	Sig::tLocalizedString tLocalizedString::fConstructTimeString( f32 time, b32 showPartialSeconds )
	{
		u32 minutes = fRoundDown< u32 >( time / 60.0f );
		u32 seconds = fRoundDown< u32 >( time - (minutes * 60.0f) );
		u32 remainder = fRoundDown< u32 >( ( time - fRoundDown< f32 >( time ) ) * 100 );

		std::stringstream ss;
		ss << minutes << ":" << std::setfill( '0' ) << std::setw( 2 ) << seconds;

		if( showPartialSeconds )
			ss << "." << std::setfill( '0' ) << std::setw( 2 ) << remainder;

		return tLocalizedString::fFromCString( ss.str( ).c_str( ) );
	}

	const tLocalizedString& tLocalizedString::fEmptyString( )
	{
		return cEmptyString;
	}

	const tLocalizedString& tLocalizedString::fNullString( )
	{
		return cNullString;
	}

	void tLocalizedString::fFromCStr( const char* s, u32 len )
	{
		if( len == 0 )
			len = fNullTerminatedLength( s );
		fDeleteArray( );
		fNewArray( len + 1 );
		for( u32 i = 0; i < len; ++i )
			fIndex( i ) = s[ i ];
		fBack( ) = 0;
	}

	void tLocalizedString::fFromCStr( const u16* s, u32 len )
	{
		if( len == 0 )
			len = fNullTerminatedLength( s );
		fDeleteArray( );
		fNewArray( len + 1 );
		tCopier<u16>::fCopy( fBegin( ), s, fCount( ) );
		fBack( ) = 0;
	}

	void tLocalizedString::fFromCStr( const wchar_t* s, u32 len )
	{
		if( wchart_is_sizeof_u16( ) )
		{
			fFromCStr( ( const u16* )s, len );
			return;
		}

		if( len == 0 )
			len = fNullTerminatedLength( s );
		fDeleteArray( );
		fNewArray( len + 1 );
		for( u32 i = 0; i < len; ++i )
			fIndex( i ) = s[ i ];
		if( len > 0 )
			fBack( ) = 0;
	}

	tLocalizedString& tLocalizedString::fJoinWithLocString( const tLocalizedString& locStr )
	{
		fInsert( fCount( ) - 1, locStr.fBegin( ), locStr.fCount( ) - 1 );
		return *this;
	}

	tLocalizedString& tLocalizedString::fJoinWithCString( const char* str )
	{
		tLocalizedString locStr;
		locStr.fFromCStr( str );
		fInsert( fCount( ) - 1, locStr.fBegin( ), locStr.fCount( ) - 1 );
		return *this;
	}

	tLocalizedString tLocalizedString::fClone() const
	{
		return tLocalizedString( *this );
	}

	tLocalizedString tLocalizedString::fFromCString( const char* s )
	{
		tLocalizedString o;
		o.fFromCStr( s );
		return o;
	}

	tLocalizedString& fLocConcatenate( tLocalizedString* obj, const Sqrat::Object& rhs )
	{
		switch( rhs.GetType( ) )
		{
		case OT_STRING:
			return obj->fJoinWithCString( rhs.Cast< const char* >( ) );
			break;

		case OT_INSTANCE:
			return obj->fJoinWithLocString( rhs.Cast< const tLocalizedString& >( ) );
			break;
		}

		log_warning( 0, "Tried to concatenate a loc string with something that isn't a C-string or a LocString!" );
		return *obj;
	}

	tLocalizedString tLocalizedString::fSlice( u32 start, u32 end ) const
	{
		tLocalizedString out;

		if( start >= fCount( ) )
			return out;

		for( u32 i = start; i <= end && i < fCount( ); ++i )
			out.fPushBack( operator[]( i ) );

		return out;
	}

	b32 tLocalizedString::fIsWhiteSpace( u32 index ) const
	{
		if( index >= fCount( ) )
			return true;

		u16 c = operator[]( index );
		switch( c )
		{
		case ' ':
		case '\n':
		case '\t':
			return true;
		default:
			return false;
		}
	}

	tLocalizedString& tLocalizedString::fReplace( const char* macroId, const char* locKey )
	{
		tLocalizedString locStr = tGameAppBase::fInstance( ).fLocString( tStringPtr( locKey ) );
		return fReplace( macroId, locStr );
	}

	tLocalizedString& tLocalizedString::fReplace( const char* macroId, const tLocalizedString& locStr )
	{
		std::wstring str = c_str( );
		std::wstring macroStr = L"{" + StringUtil::fStringToWString( macroId ) + L"}";
		StringUtil::fWReplaceAllOf( str, macroStr.c_str( ), locStr.c_str( ) );
		fFromCStr( str.c_str( ) );
		return *this;
	}

	tLocalizedString& tLocalizedString::fReplaceCString( const char* macroId, const char* str )
	{
		tLocalizedString locStr = tLocalizedString::fFromCString( str );
		return fReplace( macroId, locStr );
	}

	tLocalizedString& tLocalizedString::fReplaceValue( const char* macroId, s32 val )
	{
		return fReplaceCString( macroId, StringUtil::fToString( val ).c_str( ) );
	}

	tLocalizedString& tLocalizedString::fReplaceValue( const char* macroId, f32 val )
	{
		return fReplaceCString( macroId, StringUtil::fToString( val ).c_str( ) );
	}

	tLocalizedString& tLocalizedString::fReplaceScript( const char* macroId, Sqrat::Object loc )
	{
		switch( loc.GetType( ) )
		{
		case OT_STRING:
			return fReplace( macroId, loc.Cast< const char* >( ) );
			break;

		case OT_INSTANCE:
			return fReplace( macroId, loc.Cast< const tLocalizedString& >( ) );
			break;

		case OT_INTEGER:
			return fReplaceValue( macroId, loc.Cast< int >( ) );
			break;

		case OT_FLOAT:
			return fReplaceValue( macroId, loc.Cast< float >( ) );
			break;
		}

		log_warning( 0, "Tried to replace a loc string macro with something that isn't a C-string locId, LocString, int or float!" );
		return *this;
	}	

	b32 tLocalizedString::fCompare( const tLocalizedString& rhs )
	{
		if( fCount( ) != rhs.fCount( ) )
			return false;

		u16* a = (u16*)fBegin( );
		u16* b = (u16*)rhs.fBegin( );
		for( ; *a == *b; a++, b++ )
		{
			if( a == fEnd( ) )
				return true;
		}
		return false;
	}

	tLocalizedString tLocalizedString::fLocalizeNumber( f32 val )
	{
		s32 intValue = fRoundDown< s32 >( val );
		tLocalizedString intString = fLocalizeNumber( intValue );
		f32 fractPart = val - intValue;
		u32 fractDigits = fRoundDown< s32 >( fractPart * 100 );
		std::stringstream fractString;
		if( fractDigits == 0 )
			return intString;
		else if( fractDigits % 10 == 0 )
			fractString << (char)tGameAppBase::fInstance( ).fGetLocalizedDecimalSeparator( ) << StringUtil::fToString( fractDigits / 10 );
		else
			fractString << (char)tGameAppBase::fInstance( ).fGetLocalizedDecimalSeparator( ) << StringUtil::fToString( fractDigits );
		intString.fJoinWithCString( fractString.str( ).c_str( ) );
		return intString;
	}

	tLocalizedString tLocalizedString::fLocalizeNumber( s32 val )
	{
		std::string negative = ( val < 0 ? "-" : "" );
		std::string baseValue = StringUtil::fToString( val < 0 ? -val: val );
		std::stringstream thousandsSeparator;
		thousandsSeparator << tGameAppBase::fInstance( ).fGetLocalizedThousandsSeparator( );
		return tLocalizedString::fFromCString( ( negative + StringUtil::fAddCommaEvery3Digits( baseValue.c_str( ), NULL, NULL, thousandsSeparator.str( ).c_str( ) ) ).c_str( ) );
	}

	tLocalizedString tLocalizedString::fLocalizeNumberScript( Sqrat::Object val )
	{
		switch( val.GetType( ) )
		{
		case OT_INTEGER:
			return fLocalizeNumber( val.Cast< int >( ) );
			break;

		case OT_FLOAT:
			return fLocalizeNumber( val.Cast< float >( ) );
			break;
		}

		log_warning( 0, "Tried to localize a number with something that isn't an int or float!" );
		return cNullString.fClone( );
	}

	void tLocalizedString::fStripNewlines( )
	{
		std::wstring str = c_str( );
		std::wstring newlineString = L"\n";
		std::wstring carriageReturnString = L"\r";
		StringUtil::fWReplaceAllOf( str, newlineString.c_str( ), L" " );
		StringUtil::fWRemoveAllOf( str, carriageReturnString.c_str( ) );
		fFromCStr( str.c_str( ) );
	}

	void tLocalizedString::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tLocalizedString, Sqrat::DefaultAllocator<tLocalizedString> > classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("ConstructMoneyString"), &fConstructMoneyString)
			.StaticFunc(_SC("ConstructTimeString"), &fConstructTimeString)
			.StaticFunc(_SC("LocalizeNumber"), &tLocalizedString::fLocalizeNumberScript)
			.Func(_SC("JoinWithLocString"), &tLocalizedString::fJoinWithLocString)
			.Func(_SC("JoinWithCString"), &tLocalizedString::fJoinWithCString)
			.GlobalFunc(_SC("_modulo"), &fLocConcatenate)
			.Func(_SC("_cmp"), &tLocalizedString::fCompare)
			.Func(_SC("Clone"), &tLocalizedString::fClone)
			.Func(_SC("ToCString"), &tLocalizedString::fToCString)
			.StaticFunc( _SC("EmptyString"), &fEmptyString )
			.StaticFunc( _SC("FromCString"), &fFromCString )
			.Func(_SC("Slice"), &tLocalizedString::fSlice)
			.Func(_SC("Length"), &tLocalizedString::fCount)
			.Func(_SC("IsWhiteSpace"), &tLocalizedString::fIsWhiteSpace)
			.Func(_SC("Replace"), &tLocalizedString::fReplaceScript)
			.Func(_SC("ReplaceCString"), &tLocalizedString::fReplaceCString)
			.Func(_SC("StripNewlines"), &tLocalizedString::fStripNewlines)
			;
		vm.fRootTable( ).Bind(_SC("LocString"), classDesc);
	}

	const u32 tLocalizationFile::cVersion = 1;

	const char* tLocalizationFile::fGetFileExtension( )
	{
		return ".locb";
	}

	tLocalizationFile::tLocalizationFile( )
	{
	}

	tLocalizationFile::tLocalizationFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mRawStrings( cNoOpTag )
		, mStringMap( cNoOpTag )
		, mRawPaths( cNoOpTag )
		, mPathMap( cNoOpTag )
	{
	}

	tLocalizationFile::~tLocalizationFile( )
	{
	}

	void tLocalizationFile::fOnFileLoaded( const tResource& ownerResource )
	{
		// construct maps with twice as many actual entries just to maintain a fairly sparse harsh table
		mStringMap.fConstruct( 2 * mRawStrings.fCount( ) + 1 );
		mPathMap.fConstruct( 2 * mRawPaths.fCount( ) + 1 );

		// insert the strings into hash table
		tStringMap& stringMap = mStringMap.fTreatAsObject( );
		for( u32 i = 0; i < mRawStrings.fCount( ); ++i )
			stringMap.fInsert( mRawStrings[ i ].mId->fGetStringPtr( ).fGetHashValue( ), &mRawStrings[ i ] );
		// insert the paths into hash table
		tPathMap& pathMap = mPathMap.fTreatAsObject( );
		for( u32 i = 0; i < mRawPaths.fCount( ); ++i )
			pathMap.fInsert( mRawPaths[ i ].mId->fGetStringPtr( ).fGetHashValue( ), &mRawPaths[ i ] );
	}

	void tLocalizationFile::fOnFileUnloading( )
	{
		mStringMap.fDestroy( );
		mPathMap.fDestroy( );
	}

	s32 tLocalizationFile::fStringIndexFromId( const tStringPtr& id ) const
	{
		const tStringEntry* const* o = mStringMap.fTreatAsObject( ).fFind( id.fGetHashValue( ) );
		return o ? fPtrDiff( *o, mRawStrings.fBegin( ) ) : -1;
	}

	const tLocalizedString& tLocalizationFile::fStringFromId( const tStringPtr& id ) const
	{
		const tStringEntry* const* o = mStringMap.fTreatAsObject( ).fFind( id.fGetHashValue( ) );
		if ( !o )
		{
			log_warning( Log::cFlagLocalization, "\"" << id << "\" Not found in localization file" );
			if ( Debug_Localization_ShowMissingId )
			{
				static tLocalizedString wid;
				std::wstringstream ss;
				ss << L"[[" << id.fCStr( ) << L"]]";
				wid = tLocalizedString( ss.str( ) );
				return wid;
			}
		}
		return o ? (*o)->mText : cErrorString;
	}

	const tLocalizedString& tLocalizationFile::fStringFromIndex( u32 index ) const
	{
		return mRawStrings[ index ].mText;
	}

	s32 tLocalizationFile::fPathIndexFromId( const tStringPtr& id ) const
	{
		const tPathEntry* const* o = mPathMap.fTreatAsObject( ).fFind( id.fGetHashValue( ) );
		return o ? fPtrDiff( *o, mRawPaths.fBegin( ) ) : -1;
	}

	tFilePathPtr tLocalizationFile::fPathFromId( const tStringPtr& id ) const
	{
		const tPathEntry* const* o = mPathMap.fTreatAsObject( ).fFind( id.fGetHashValue( ) );
		return o ? (*o)->mPath->fGetFilePathPtr( ) : tFilePathPtr( );
	}

	tFilePathPtr tLocalizationFile::fPathFromIndex( u32 index ) const
	{
		return mRawPaths[ index ].mPath->fGetFilePathPtr( );
	}

	tStringPtr tLocalizationFile::fPathIdFromIndex( u32 index ) const
	{
		return mRawPaths[ index ].mId->fGetStringPtr( );
	}
}

