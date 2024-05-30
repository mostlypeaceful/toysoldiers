#include "basePch.hpp"
#include "tDataTableQuery.hpp"
#include "tDataTableFile.hpp"

namespace Sig { namespace DataTableQuery
{

	namespace
	{
		struct tCompare
		{
			b32 mMatch;

			tCompare( ) : mMatch( false ) { }
		};

#define define_compare( op, name ) struct name : public tCompare { template< typename t > name( const t& left, const t& right ) { mMatch = (left op right); } };

		define_compare( <, tCompareLesser );
		define_compare( <=, tCompareLesserEqual );
		define_compare( >, tCompareGreater );
		define_compare( >=, tCompareGreaterEqual );
		define_compare( ==, tCompareEqual );

		struct tCompareContains : public tCompare 
		{ 
			tCompareContains( const tStringPtr& left, const tStringPtr& right ) 
			{ 
				mMatch = (left.fLength( ) && right.fLength( ) && strstr( left.fCStr( ), right.fCStr( ) ) != NULL); 
			} 

			template< typename t >
			tCompareContains( const t& left, const t& right ) 
			{ 
				mMatch = false; 
			} 
		};

		tData fMakeData( tDataTableRef& table, u32 row, u32 column )
		{
			if( column == tColumnRef::cRowNameIndex )
				return tData( table.mTable->fRowName( row ) );

			const tDataTableCellArray& ar = *table.mTable->mCellArrays[ column ];

			if( ar.fClassId( ) == tDataTableCellArrayStringPtr::fGetReflector( ).fClassId( ) )
				return tData( ar.fValue<tStringPtr>( row, table.mTable ) );
			else if( ar.fClassId( ) == tDataTableCellArrayNumeric::fGetReflector( ).fClassId( ) )
				return tData( ar.fValue<f32>( row, table.mTable ) );

			sigassert( !"Unsupported type!" );
			return tData( );
		}

		struct tPotentialTables
		{
			tDataTableRef mTable;
			tGrowableArray< u32 > mColumns;

			tPotentialTables( ) { }
			tPotentialTables( const tDataTableRef& ref ) : mTable( ref ) { }
			b32 operator == ( const tDataTable* right ) const { return mTable.mTable == right; }
		};
	}



	b32 tData::fCompare( const tDataTable& table, u32 column, u32 row, u32 compareFunc ) const
	{
		switch( compareFunc )
		{
		case tData::cComparisonEquals:			return fCompareImp< tCompareEqual >( table, column, row ); break;
		case tData::cComparisonGreater:			return fCompareImp< tCompareGreater >( table, column, row ); break;
		case tData::cComparisonGreaterEqual:	return fCompareImp< tCompareGreaterEqual >( table, column, row ); break;
		case tData::cComparisonLesser:			return fCompareImp< tCompareLesser >( table, column, row ); break;
		case tData::cComparisonLesserEqual:		return fCompareImp< tCompareLesserEqual >( table, column, row ); break;
		case tData::cComparisonContains:		return fCompareImp< tCompareContains >( table, column, row ); break;
		default:
			sigassert( !"Unsupported comparison type!" );
		}

		return false;
	}

	void tData::fLog( ) const
	{
		switch( mType )
		{
		case cTypeFloat: log_output( 0, mFloatVal ); break;
		case cTypeString: log_output( 0, mStrVal ); break;
		}
	}

	u32 tDataTableQueryResult::fFindRow( const tRow& compare ) const
	{
		for( u32 r = 0; r < mOutput.fCount( ); ++r )
		{
			b32 found = true;

			const tRow& row = mOutput[ r ];
			for( u32 c = 0; c < row.fCount( ); ++c )
			{
				if( row[ c ] != compare[ c ] )
				{
					found = false;
					break;
				}
			}

			if( found )
				return r;
		}

		return ~0;
	}

	void tDataTableQueryResult::fLog( ) const
	{
		log_line( 0, "Data Query Result: " );

		for( u32 r = 0; r < mOutput.fCount( ); ++r )
		{
			const tRow& row = mOutput[ r ];
			for( u32 c = 0; c < row.fCount( ); ++c )
			{
				row[ c ].fLog( );
				log_output( 0, " " );
			}
			log_output( 0, std::endl );
		}
	}

	// Return false if this comparison caused a failure. True if it wasn't applicable or comparison passed.
	b32 tInput::fCompare( const tDataTableRef& table, u32 row ) const
	{
		if( table != mColumn.mTable ) 
			return true;

		b32 compares = mValue.fCompare( *table.mTable, mColumn.mColumn, row, mCompareFunc );
		for( u32 i = 0; i < mOrs.fCount( ); ++i )
			if( mOrs[ i ].fCompare( table, row ) )
				compares = true;

		if( !compares )
			return false;

		for( u32 i = 0; i < mAnds.fCount( ); ++i )
			if( !mAnds[ i ].fCompare( table, row  ) )
				return false;

		return true;
	}

	void tDataTableQuery::fExecute( tDataTableQueryResult& result )
	{
		// collect seed tables
		tGrowableArray< tPotentialTables > tables;
		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
		{
			tColumnRef& c = mOutputs[ i ];
			tPotentialTables* pt = tables.fFind( c.mTable.mTable );
			if( !pt )
			{
				tables.fPushBack( tPotentialTables( c.mTable ) );
				pt = &tables.fBack( );
			}
			pt->mColumns.fPushBack( c.mColumn );
		}

		std::sort( mMappings.fBegin( ), mMappings.fEnd( ) );

		tRow potentialRow;
		potentialRow.fSetCount( mOutputs.fCount( ) );

		tGrowableArray< tDataTableRef > searchHistory;

		// each row in each output table is a potential row generator.
		for( u32 i = 0; i < tables.fCount( ); ++i )
		{
			tPotentialTables& pt = tables[ i ];
			for( u32 r = 0; r < pt.mTable.mTable->fRowCount( ); ++r )
			{
				// apply the mappings to this row, if it matches, consider it an output.
				if( fMatchRow( pt.mTable, r, potentialRow, searchHistory ) )
				{
					if( !mUnique || result.fFindRow( potentialRow ) == ~0 )
						result.mOutput.fPushBack( potentialRow );
				}
			}
		}
	}

	b32 tDataTableQuery::fMatchRow( tDataTableRef& table, u32 row, tRow& potentialRow, tGrowableArray< tDataTableRef >& searchHistory, tInput* additionalConstraint )
	{
		if( !mInput.fCompare( table, row ) )
			return false; //input keys did not match

		if( additionalConstraint && !additionalConstraint->fCompare( table, row ) )
			return false;

		// Search history prevents infinite recursion or back tracking which can invalidate results.
		searchHistory.fPushBack( table );
		for( u32 i = 0; i < mMappings.fCount( ); ++i )
		{
			tMapping& m = mMappings[ i ];
			if( (m.mLeft.mTable == table || m.mRight.mTable == table) )
			{
				//this mapping applies
				tColumnRef& mine = (m.mLeft.mTable == table) ? m.mLeft : m.mRight;
				tColumnRef& other = (m.mLeft.mTable == table) ? m.mRight : m.mLeft;

				// Only recurse if we haven't already searched this table.
				if( searchHistory.fIndexOf( other.mTable ) == ~0 )
				{
					tInput addConstraint( other, m.mCompareFunc, fMakeData( table, row, mine.mColumn ) );

					b32 match = false;

					for( u32 r = 0; r < other.mTable.mTable->fRowCount( ); ++r )
					{
						// apply the mappings to this row, if it matches, consider it an output.
						if( fMatchRow( other.mTable, r, potentialRow, searchHistory, &addConstraint ) )
							match = true;
					}

					if( !match )
					{
						searchHistory.fPopBack( );
						return false;
					}
				}
			}
		}

		searchHistory.fPopBack( );

		//save out data from this table.
		sigassert( potentialRow.fCount( ) == mOutputs.fCount( ) );
		for( u32 i = 0; i < potentialRow.fCount( ); ++i )
		{
			if( mOutputs[ i ].mTable == table )
				potentialRow[ i ] = fMakeData( table, row, mOutputs[ i ].mColumn );
		}

		return true;
	}

} }