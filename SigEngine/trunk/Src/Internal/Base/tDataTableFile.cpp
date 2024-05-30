#include "BasePch.hpp"
#include "tDataTableFile.hpp"


namespace Sig
{
	register_rtti_factory( tDataTableCellArrayNumeric, true );
	register_rtti_factory( tDataTableCellArrayDoubleNumeric, true );
	register_rtti_factory( tDataTableCellArrayBoolean, true );
	register_rtti_factory( tDataTableCellArrayStringPtr, true );
	register_rtti_factory( tDataTableCellArrayFilePathPtr, true );
	register_rtti_factory( tDataTableCellArrayDateTime, true );
	register_rtti_factory( tDataTableCellArrayUnicodeString, true );
	register_rtti_factory( tDataTableCellArrayVector4, true );

	//------------------------------------------------------------------------------
	// tDataTableCellArray
	//------------------------------------------------------------------------------
	tDataTableCellArray::tDataTableCellArray( )
	{
	}
	tDataTableCellArray::tDataTableCellArray( tNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	// tDataTableCellArrayNumeric
	//------------------------------------------------------------------------------
	tDataTableCellArrayNumeric::tDataTableCellArrayNumeric( )
	{
	}
	tDataTableCellArrayNumeric::tDataTableCellArrayNumeric( const tDynamicArray<f32>& vals )
		: mValues( vals )
	{
	}
	tDataTableCellArrayNumeric::tDataTableCellArrayNumeric( tNoOpTag )
		: tDataTableCellArray( cNoOpTag )
		, mValues( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	// tDataTableCellArrayDoubleNumeric
	//------------------------------------------------------------------------------
	tDataTableCellArrayDoubleNumeric::tDataTableCellArrayDoubleNumeric( )
	{
	}
	tDataTableCellArrayDoubleNumeric::tDataTableCellArrayDoubleNumeric( const tDynamicArray<f64>& vals )
		: mValues( vals )
	{
	}
	tDataTableCellArrayDoubleNumeric::tDataTableCellArrayDoubleNumeric( tNoOpTag )
		: tDataTableCellArray( cNoOpTag )
		, mValues( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	// tDataTableCellArrayBoolean
	//------------------------------------------------------------------------------
	tDataTableCellArrayBoolean::tDataTableCellArrayBoolean( )
	{
	}
	tDataTableCellArrayBoolean::tDataTableCellArrayBoolean( const tDynamicArray<b32>& vals )
	{
		mValueCount = vals.fCount( );
		mValues.fNewArray( mValueCount );

		for( u32 b = 0; b < mValueCount; ++b )
			mValues.fSetBit( b, vals[ b ] );
	}
	tDataTableCellArrayBoolean::tDataTableCellArrayBoolean( tNoOpTag )
		: tDataTableCellArray( cNoOpTag )
		, mValues( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	// tDataTableCellArrayStringPtr
	//------------------------------------------------------------------------------
	tDataTableCellArrayStringPtr::tDataTableCellArrayStringPtr( )
	{
	}
	tDataTableCellArrayStringPtr::tDataTableCellArrayStringPtr( const tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> >& vals )
		: mValues( vals )
	{
	}
	tDataTableCellArrayStringPtr::tDataTableCellArrayStringPtr( tNoOpTag )
		: tDataTableCellArray( cNoOpTag )
		, mValues( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	// tDataTableCellArrayFilePathPtr 
	//------------------------------------------------------------------------------
	tDataTableCellArrayFilePathPtr::tDataTableCellArrayFilePathPtr( )
	{
	}
	tDataTableCellArrayFilePathPtr::tDataTableCellArrayFilePathPtr( const tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceFilePathPtr> >& vals )
		: mValues( vals )
	{
	}
	tDataTableCellArrayFilePathPtr::tDataTableCellArrayFilePathPtr( tNoOpTag )
		: tDataTableCellArray( cNoOpTag )
		, mValues( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	// tDataTableCellArrayDateTime
	//------------------------------------------------------------------------------
	tDataTableCellArrayDateTime::tDataTableCellArrayDateTime( )
	{
	}
	tDataTableCellArrayDateTime::tDataTableCellArrayDateTime( const tDynamicArray< u64 >& vals )
		: mValues( vals )
	{
	}
	tDataTableCellArrayDateTime::tDataTableCellArrayDateTime( tNoOpTag )
		: tDataTableCellArray( cNoOpTag )
		, mValues( cNoOpTag )
	{
	}


	//------------------------------------------------------------------------------
	// tDataTableCellArrayUnicodeString
	//------------------------------------------------------------------------------
	tDataTableCellArrayUnicodeString::tDataTableCellArrayUnicodeString( )
	{
	}
	tDataTableCellArrayUnicodeString::tDataTableCellArrayUnicodeString( const tDynamicArray< tLocalizedString >& vals )
		: mValues( vals )
	{
	}
	tDataTableCellArrayUnicodeString::tDataTableCellArrayUnicodeString( tNoOpTag )
		: tDataTableCellArray( cNoOpTag )
		, mValues( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	// tDataTableCellArrayVector4
	//------------------------------------------------------------------------------
	tDataTableCellArrayVector4::tDataTableCellArrayVector4( )
	{
	}
	tDataTableCellArrayVector4::tDataTableCellArrayVector4( const tDynamicArray<Math::tVec4f>& vals )
		: mValues( vals )
	{
	}
	tDataTableCellArrayVector4::tDataTableCellArrayVector4( tNoOpTag )
		: tDataTableCellArray( cNoOpTag )
		, mValues( cNoOpTag )
	{
	}

	const std::string tDataTable::ColumnTypes::mType[ cTypesCount ] = { "String", "Float", "Double", "Filepath", "Int", "Vector", "Bool", "DateTime" };
	const std::string tDataTable::ColumnTypes::mDirective[ cTypesCount ] = { "s", "n", "N", "f", "n", "v", "b", "dt" };

	//------------------------------------------------------------------------------
	// tDataTable
	//------------------------------------------------------------------------------
	tDataTable::tDataTable( ) 
		: mName( 0 )
	{
	}

	tDataTable::tDataTable( tNoOpTag )
		: mColNames( cNoOpTag )
		, mRowNames( cNoOpTag )
		, mCellArrays( cNoOpTag )
	{
	}

	tDataTable::~tDataTable( )
	{
#ifdef platform_tools
		for( u32 i = 0; i < mCellArrays.fCount( ); ++i )
			delete mCellArrays[ i ];
#endif//platform_tools
	}

	u32 tDataTable::fColIndex( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mColNames.fCount( ); ++i )
			if( mColNames[ i ] && mColNames[ i ]->fGetStringPtr( ) == name )
				return i;
		return ~0;
	}

	u32 tDataTable::fRowIndex( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mRowNames.fCount( ); ++i )
			if( mRowNames[ i ] && mRowNames[ i ]->fGetStringPtr( ) == name )
				return i;
		return ~0;
	}

	b32 tDataTable::fIsColumnType( u32 index, ColumnTypes::tTypes type ) const
	{
		if( index >= fColCount( ) ) 
		{
			log_assert( 0, "DataTableValidation: " << fName( ) << " Missing column ordinal: " << index << " Type: " << ColumnTypes::mType[ type ] );
			return false;
		}

		b32 typeMatches = true;

		switch( type )
		{
		case ColumnTypes::cString:
			typeMatches = (mCellArrays[ index ]->fClassId( ) == Rtti::fGetClassId<tDataTableCellArrayStringPtr>( )); 
			break;
		case ColumnTypes::cFilepath:
			typeMatches = (mCellArrays[ index ]->fClassId( ) == Rtti::fGetClassId<tDataTableCellArrayFilePathPtr>( )); 
			break;
		case ColumnTypes::cFloat:
		case ColumnTypes::cInt:
			typeMatches = (mCellArrays[ index ]->fClassId( ) == Rtti::fGetClassId<tDataTableCellArrayNumeric>( ));
			break;
		case ColumnTypes::cDouble:
			typeMatches = (mCellArrays[ index ]->fClassId( ) == Rtti::fGetClassId<tDataTableCellArrayDoubleNumeric>( ));
			break;
		case ColumnTypes::cVector:
			typeMatches = (mCellArrays[ index ]->fClassId( ) == Rtti::fGetClassId<tDataTableCellArrayVector4>( ));
			break;
		case ColumnTypes::cDateTime:
			typeMatches = (mCellArrays[ index ]->fClassId( ) == Rtti::fGetClassId<tDataTableCellArrayDateTime>( ));
			break;
		}

		return typeMatches;
	}

	void tDataTable::fValidateColumn( u32 index, ColumnTypes::tTypes type ) const
	{
		log_assert( fIsColumnType( index, type ), "DataTableValidation: " << fName( ) << " Col: " << index << " is supposed to be: " << ColumnTypes::mType[ type ] << " Use directive: '" << ColumnTypes::mDirective[ type ] << "'" );
	}

	void tDataTable::fValidateCount( u32 count ) const
	{
		if( count < fColCount( ) ) 
		{
			log_assert( 0, "DataTableValidation: " << fName( ) << " Too many columns. Expected: " << count );
		}
		else if( count > fColCount( ) ) 
		{
			log_assert( 0, "DataTableValidation: " << fName( ) << " Not enough columns. Expected: " << count );
		}
	}

	define_lip_version( tDataTableFile, 1, 1, 1 );

	tFilePathPtr tDataTableFile::fMakeBinaryPath( const tFilePathPtr& excelFilePath )
	{
		char buf[256]={0};
		sigassert( excelFilePath.fLength( ) < array_length( buf ) - 1 );
		strcat( buf, excelFilePath.fCStr( ) );
		strcat( buf, "b" );
		return tFilePathPtr( buf );
	}

	tDataTableFile::tDataTableFile( )
	{
	}
	tDataTableFile::tDataTableFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mTables( cNoOpTag )
	{
	}
	tDataTableFile::~tDataTableFile( )
	{
#ifdef platform_tools
		for( u32 i = 0; i < mTables.fCount( ); ++i )
			delete mTables[ i ];
#endif//platform_tools
	}
	void tDataTableFile::fOnFileLoaded( const tResource& ownerResource )
	{
	}
	void tDataTableFile::fOnFileUnloading( const tResource& ownerResource )
	{
	}
	const tDataTable* tDataTableFile::fFindTable( const tStringPtr& tableName ) const
	{
		u32 index = fTableIndex( tableName );
		if( index != ~0 ) return mTables[ index ];
		else return NULL;
	}
	const tDataTable& tDataTableFile::fGetTable( const tStringPtr & tableName ) const
	{
		const tDataTable * table = fFindTable( tableName );
		log_assert( table, "Could not find required table " << tableName );
		return *table;
	}

	u32 tDataTableFile::fTableIndex( const tStringPtr& tableName ) const
	{
		for( u32 i = 0; i < mTables.fCount( ); ++i )
		{
			if( mTables[ i ]->mName && mTables[ i ]->mName->fGetStringPtr( ) == tableName )
				return i;
		}
		return ~0;
	}


	///
	///  Hashed table stuff
	void tStringHashDataTableFile::fSet( const tResourcePtr& tables )
	{
		mTables = tables;
		fRefreshHash( );
	}

	void tStringHashDataTableFile::fRefreshHash( )
	{
		tDataTableFile& tabs = fTables( );
		u32 cnt = tabs.fTableCount( );
		mHashes.fSetCount( cnt );

		for( u32 i = 0; i < cnt; ++i )
		{
			mTableHash.fInsert( tabs.fIndexTable( i ).fName( ), i );
			mHashes[ i ].fSet( mTables, i );
		}
	}

	const tStringHashDataTable* tStringHashDataTableFile::fFindTable( const tStringPtr& tableName ) const
	{
		u32 *index = mTableHash.fFind( tableName );
		if( index ) return &mHashes[ *index ];
		else return NULL;
	}

	void tStringHashDataTable::fSet( tResourcePtr& tables, u32 index )
	{
		mTables = tables;
		mTable = &tables->fCast<tDataTableFile>( )->fIndexTable( index );
		mHash.fClear( );
		for( u32 r = 0; r < mTable->fRowCount( ); ++r )
			mHash.fInsert( mTable->fRowName( r ), r );
	}

	u32 tStringHashDataTable::fRowIndex( const tStringPtr& row ) const
	{
		u32 *index = mHash.fFind( row );
		if( index ) return *index;
		else return ~0;
	}

}


namespace Sig
{
	namespace
	{
		static tDataTable* fFindDataTable( const tDataTableFile* dataTableFile, const tStringPtr& name )
		{
			return ( tDataTable* )dataTableFile->fFindTable( name );
		}
		static u32 fDataTableCount( const tDataTableFile* dataTableFile )
		{
			return dataTableFile->mTables.fCount( );
		}
		static tDataTable* fIndexDataTableArray( const tDataTableFile* dataTableFile, u32 ithTable )
		{
			return dataTableFile->mTables[ ithTable ];
		}

		static const char* fAccessDataTable_RowCol_String( const tDataTable* dataTable, u32 row, u32 col )
		{
			return dataTable->fIndexByRowCol<tStringPtr>( row, col ).fCStr( );
		}
		static f32 fAccessDataTable_RowCol_Float( const tDataTable* dataTable, u32 row, u32 col )
		{
			return dataTable->fIndexByRowCol<f32>( row, col );
		}
		static int fAccessDataTable_RowCol_Int( const tDataTable* dataTable, u32 row, u32 col )
		{
			return fRound<int>( dataTable->fIndexByRowCol<f32>( row, col ) );
		}
	}

	void tDataTable::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tDataTable, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("RowCount"), &tDataTable::fRowCount)
			.Prop(_SC("ColCount"), &tDataTable::fColCount)
			.Func(_SC("RowIndex"), &tDataTable::fRowIndex)
			.Func(_SC("ColIndex"), &tDataTable::fColIndex)
			.GlobalFunc(_SC("GetString"), &fAccessDataTable_RowCol_String)
			.GlobalFunc(_SC("GetFloat"), &fAccessDataTable_RowCol_Float)
			.GlobalFunc(_SC("GetInt"), &fAccessDataTable_RowCol_Int)
			;
		vm.fRootTable( ).Bind(_SC("DataTable"), classDesc);
	}
	void tDataTableFile::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tDataTableFile, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("FindTable"), &fFindDataTable)
			.GlobalFunc(_SC("GetTableCount"), &fDataTableCount)
			.GlobalFunc(_SC("GetTable"), &fIndexDataTableArray)
			;
		vm.fRootTable( ).Bind(_SC("DataTableFile"), classDesc);
	}
}


