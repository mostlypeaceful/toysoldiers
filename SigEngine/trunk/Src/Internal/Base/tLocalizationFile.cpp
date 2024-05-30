#include "BasePch.hpp"
#include "tLocalizationFile.hpp"
#include <iomanip>
#include "tGameAppBase.hpp"

namespace Sig
{
	// we rely on this being true
#define wchart_is_sizeof_u16( ) ( sizeof( u16 ) == sizeof( wchar_t ) )

	namespace
	{
		static const tLocalizedString cEmptyString = tLocalizedString( L"" );
#ifdef build_release
		static const tLocalizedString cErrorString = cEmptyString;
#else
		static const tLocalizedString cErrorString = tLocalizedString( L"!!!! string not in loc file !!!!" );
#endif//build_release
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

	tLocalizedString tLocalizedString::fConstructTimeString( f32 hours, b32 showPartialSeconds )
	{
		u32 minutes = fRoundDown< u32 >( hours / 60.0f );
		u32 seconds = fRoundDown< u32 >( hours - (minutes * 60.0f) );
		u32 remainder = fRoundDown< u32 >( ( hours - fRoundDown< f32 >( hours ) ) * 100 );

		std::stringstream ss;
		ss << minutes << ":" << std::setfill( '0' ) << std::setw( 2 ) << seconds;

		if( showPartialSeconds )
			ss << "." << std::setfill( '0' ) << std::setw( 2 ) << remainder;

		return tLocalizedString::fFromCString( ss.str( ).c_str( ) );
	}

	tLocalizedString tLocalizedString::fConstructTimeStringFromSeconds( s64 totalSeconds )
	{
		const s64 hours = fRoundDown< s64 >( totalSeconds / 3600.0f );
		totalSeconds -= hours * 3600;
		sigassert( totalSeconds < 3600 );
		const s64 minutes = fRoundDown< s64 >( totalSeconds / 60.0f );
		totalSeconds -= minutes * 60;
		sigassert( totalSeconds < 60 );
		const s64 seconds = totalSeconds;

		std::stringstream ss;
		ss	<< std::setfill( '0' ) << std::setw( 2 ) << hours << ":" 
			<< std::setfill( '0' ) << std::setw( 2 ) << minutes << ":"
			<< std::setfill( '0' ) << std::setw( 2 ) << seconds;

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
		if( len == ~0 )
			len = fNullTerminatedLength( s );

		fNewArray( len + 1 );
		for( u32 i = 0; i < len; ++i )
			fIndex( i ) = s[ i ];
		fBack( ) = 0;
	}

	void tLocalizedString::fFromCStr( const u16* s, u32 len )
	{
		if( len == ~0 )
			len = fNullTerminatedLength( s );

		fNewArray( len + 1 );
		tCopier<u16>::fCopy( fBegin( ), s, len );
		fBack( ) = 0;
	}

	void tLocalizedString::fFromCStr( const wchar_t* s, u32 len )
	{
		if( wchart_is_sizeof_u16( ) )
		{
			fFromCStr( ( const u16* )s, len );
			return;
		}

		if( len == ~0 )
			len = fNullTerminatedLength( s );

		fNewArray( len + 1 );
		for( u32 i = 0; i < len; ++i )
			fIndex( i ) = s[ i ];
		if( len > 0 )
			fBack( ) = 0;
	}

	tLocalizedString& tLocalizedString::fJoinWithLocString( const tLocalizedString& locStr )
	{
		if( locStr.fLength( ) )
		{
			if( fCount( ) )
				fInsert( fLength( ), locStr.fBegin( ), locStr.fLength( ) );
			else 
				*this = locStr;
		}

		return *this;
	}

	tLocalizedString& tLocalizedString::fJoinWithCString( const wchar_t * str, u32 len )
	{
		if( len == ~0 )
			len = fNullTerminatedLength( str );

		if( len )
		{
			if( wchart_is_sizeof_u16( ) )
			{
				if( fCount( ) )
					fInsert( fLength( ), (const u16*)str, len );
				else
					fFromCStr( str, len );
			}
			else
			{
				tLocalizedString locStr;
				locStr.fFromCStr( str, len );

				fJoinWithLocString( locStr );
			}
		}

		return *this;
	}

	tLocalizedString& tLocalizedString::fJoinWithCString( const char* str, u32 len )
	{
		tLocalizedString locStr;
		locStr.fFromCStr( str, len );

		return fJoinWithLocString( locStr );
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

	const char* fLocTypeof( tLocalizedString* obj )
	{
		return "tLocalizedString";
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
				
		default:
			break;
		}

		log_warning( "Tried to concatenate a loc string with something that isn't a C-string or a LocString!" );
		
		return *obj;
	}

	tLocalizedString tLocalizedString::fSlice( u32 start, u32 end ) const
	{
		if( start >= fCount( ) )
			return fEmptyString( );

		end = fMin( fCount( ), end + 1 );
		return tLocalizedString( std::wstring( fBegin( ) + start, fBegin( ) + end ) );
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
		//tLocalizedString locStr = tGameAppBase::fInstance( ).fLocString( tStringPtr( locKey ) );
		//return fReplace( macroId, locStr ); // returns *this

		log_warning_unimplemented( );
		return *this;
	}

	tLocalizedString& tLocalizedString::fReplace( const char* macroId, const tLocalizedString& locStr )
	{
		std::wstring str = fCStr( );
		std::wstring macroStr = L"{" + StringUtil::fStringToWString( macroId ) + L"}";
		StringUtil::fWReplaceAllOf( str, macroStr.c_str( ), locStr.fCStr( ) );
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

	tLocalizedString& tLocalizedString::fReplaceValue( const char* macroId, u32 val )
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
		default:
			break;
		}

		log_warning( "Tried to replace a loc string macro with something that isn't a C-string locId, LocString, int or float!" );
		return *this;
	}	

	b32 tLocalizedString::fCompare( const tLocalizedString& rhs )
	{
		if( fCount( ) != rhs.fCount( ) )
			return false;

		const u16* a = fBegin( );
		const u16* b = rhs.fBegin( );
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
		return tLocalizedString::fLocalizeNumber( (s64)val );
	}

	tLocalizedString tLocalizedString::fLocalizeNumber( s64 val )
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
				
		default:
			break;
		}

		log_warning( "Tried to localize a number with something that isn't an int or float!" );
		return cNullString.fClone( );
	}

	void tLocalizedString::fStripNewlines( )
	{
		std::wstring str = fCStr( );
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
			//.GlobalFunc(_SC("_typeof"),&fLocTypeof)
			.GlobalFunc(_SC("_modulo"), &fLocConcatenate)
			.StaticFunc(_SC("ConstructMoneyString"), &tLocalizedString::fConstructMoneyString)
			.StaticFunc(_SC("ConstructTimeString"), &tLocalizedString::fConstructTimeString)
			.StaticFunc(_SC("LocalizeNumber"), &tLocalizedString::fLocalizeNumberScript)
			.Func(_SC("JoinWithLocString"), &tLocalizedString::fJoinWithLocString)
			.Func( _SC("JoinWithCString"), &tLocalizedString::fJoinWithCStringForScript)
			.Func(_SC("_cmp"), &tLocalizedString::fCompare)
			.Func(_SC("Clone"), &tLocalizedString::fClone)
			.Func(_SC("ToCString"), &tLocalizedString::fToCString)
			.StaticFunc( _SC("EmptyString"), &tLocalizedString::fEmptyString )
			.StaticFunc( _SC("FromCString"), &tLocalizedString::fFromCString )
			.Func(_SC("Slice"), &tLocalizedString::fSlice)
			.Func(_SC("Length"), &tLocalizedString::fCount)
			.Func(_SC("IsWhiteSpace"), &tLocalizedString::fIsWhiteSpace)
			.Func(_SC("Replace"), &tLocalizedString::fReplaceScript)
			.Func(_SC("ReplaceCString"), &tLocalizedString::fReplaceCString)
			.Func(_SC("StripNewlines"), &tLocalizedString::fStripNewlines)
			;
		vm.fRootTable( ).Bind(_SC("LocString"), classDesc);
	}

	define_lip_version( tLocalizationFile, 1, 1, 1 );

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
		mStringMap.fTreatAsObject( ).fSetCapacity( 2 * mRawStrings.fCount( ) + 1 );
		mPathMap.fTreatAsObject( ).fSetCapacity( 2 * mRawPaths.fCount( ) + 1 );

		// insert the strings into hash table
		tStringMap& stringMap = mStringMap.fTreatAsObject( );
		for( u32 i = 0; i < mRawStrings.fCount( ); ++i )
		{
			const tHashTablePtrInt hashValue = mRawStrings[ i ].mId->fGetStringPtr( ).fGetHashValue( );
#ifndef build_release
			const tStringEntry** entry = stringMap.fFind( hashValue );
			if( entry )
			{
				log_warning( "string [" << mRawStrings[ i ].mId->fGetStringPtr( ) << "] already in locml!" );
				continue;
			}
#endif//!build_release
			stringMap.fInsert( hashValue, &mRawStrings[ i ] );
		}
		// insert the paths into hash table
		tPathMap& pathMap = mPathMap.fTreatAsObject( );
		for( u32 i = 0; i < mRawPaths.fCount( ); ++i )
		{
			const tHashTablePtrInt hashValue = mRawPaths[ i ].mId->fGetStringPtr( ).fGetHashValue( );
#ifndef build_release
			const tPathEntry** entry = pathMap.fFind( hashValue );
			if( entry )
			{
				log_warning( "path [" << mRawPaths[ i ].mId->fGetStringPtr( ) << "] already in locml!" );
				continue;
			}
#endif//!build_release
			pathMap.fInsert( hashValue, &mRawPaths[ i ] );
		}
	}

	void tLocalizationFile::fOnFileUnloading( const tResource& ownerResource )
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
		if( !o )
		{
			tScriptVm::fDumpCallstack( );
			log_warning( "\"" << id << "\" Not found in localization file" );
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

	b32	tLocalizationFile::fJoinLocFile( const tLocalizationFile& appendFile )
	{
		//allocate total space
		u32 stringCount = mRawStrings.fCount( ) + appendFile.mRawStrings.fCount( );
		u32 pathCount = mRawPaths.fCount( ) + appendFile.mRawPaths.fCount( );

		mStringMap.fTreatAsObject( ).fClear( );
		mStringMap.fTreatAsObject( ).fSetCapacity( 2 * stringCount + 1 );
		mPathMap.fTreatAsObject( ).fClear( );
		mPathMap.fTreatAsObject( ).fSetCapacity( 2 * pathCount + 1 );

		// insert the strings into hash table
		tStringMap& stringMap = mStringMap.fTreatAsObject( );
		for( u32 i = 0; i < mRawStrings.fCount( ); ++i )
			stringMap.fInsert( mRawStrings[ i ].mId->fGetStringPtr( ).fGetHashValue( ), &mRawStrings[ i ] );

		// insert the strings into hash table
		for( u32 i = 0; i < appendFile.mRawStrings.fCount( ); ++i )
			stringMap.fInsert( appendFile.mRawStrings[ i ].mId->fGetStringPtr( ).fGetHashValue( ), &appendFile.mRawStrings[ i ] );

		// insert the paths into hash table
		tPathMap& pathMap = mPathMap.fTreatAsObject( );
		for( u32 i = 0; i < appendFile.mRawPaths.fCount( ); ++i )
			pathMap.fInsert( appendFile.mRawPaths[ i ].mId->fGetStringPtr( ).fGetHashValue( ), &appendFile.mRawPaths[ i ] );

		// insert the paths into hash table
		for( u32 i = 0; i < appendFile.mRawPaths.fCount( ); ++i )
			pathMap.fInsert( appendFile.mRawPaths[ i ].mId->fGetStringPtr( ).fGetHashValue( ), &appendFile.mRawPaths[ i ] );

		return false;
	}
}

