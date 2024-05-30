#ifndef __tDataTableQuery__
#define __tDataTableQuery__

namespace Sig { 
	
	class tDataTable;

namespace DataTableQuery
{
	struct tDataTableRef
	{
		const tDataTable* mTable;

		tDataTableRef( ) : mTable( NULL ) { }
		tDataTableRef( const tDataTable& table ) : mTable( &table ) { }

		b32 operator == ( const tDataTableRef& other ) const { return mTable == other.mTable; }
		b32 operator != ( const tDataTableRef& other ) const { return !operator == ( other ); }
	};

	struct tColumnRef
	{
		static const u32 cRowNameIndex = -2;

		tDataTableRef mTable;
		u32 mColumn;

		tColumnRef( ) : mColumn( ~0 ) { }

		tColumnRef( tDataTableRef& table, u32 column )
			: mTable( table )
			, mColumn( column )
		{ }

		b32 operator == ( const tColumnRef& other ) const
		{
			return mTable == other.mTable && mColumn == other.mColumn;
		}
	};

	struct tData
	{
		enum tType
		{
			cTypeString,
			cTypeFloat,
			cTypeInvalid
		};

		enum tComparisonType
		{
			cComparisonEquals,
			cComparisonGreater,
			cComparisonGreaterEqual,
			cComparisonLesser,
			cComparisonLesserEqual,
			cComparisonContains,
			cComparisonInvalid
		};

		u32			mType;		
		tStringPtr	mStrVal;
		f32			mFloatVal;

		tData( ) : mType( cTypeInvalid ) { }
		tData( float v ) : mType( cTypeFloat ), mFloatVal( v ) { }
		tData( const tStringPtr& v ) : mType( cTypeString ), mStrVal( v ) { }

		b32 fCompare( const tDataTable& table, u32 column, u32 row, u32 compareFunc ) const;
		void fLog( ) const;

		b32 operator == ( const tData& other ) const
		{
			return mType == other.mType && mStrVal == other.mStrVal && mFloatVal == other.mFloatVal;
		}

		b32 operator != ( const tData& other ) const
		{
			return !operator ==( other );
		}

		template< typename t >
		void fGet( t& output ) const
		{
			Rtti::tClassId outClass = Rtti::fGetClassId<t>( );
			if( outClass == Rtti::fGetClassId<tStringPtr>( ) )
			{
				sigassert( mType == cTypeString );
				output = *((t*)&mStrVal);
				return;
			}
			else if( outClass == Rtti::fGetClassId<f32>( ) )
			{
				sigassert( mType == cTypeFloat );
				output = *((t*)&mFloatVal);
				return;
			}
			sigassert( !"Unsupported data type!" );
		}

	private:
		template< class tCompare >
		b32 fCompareImp( const tDataTable& table, u32 column, u32 row ) const
		{
			if( column == tColumnRef::cRowNameIndex )
			{
				sigassert( mType == cTypeString );
				return tCompare( table.fRowName( row ), mStrVal ).mMatch;
			}
			else if( mType == cTypeString )
				return tCompare( table.fIndexByRowCol<tStringPtr>( row, column ), mStrVal ).mMatch;
			else if( mType == cTypeFloat )
				return tCompare( table.fIndexByRowCol<f32>( row, column ), mFloatVal ).mMatch;

			sigassert( !"Unsupported type!" );
			return false;
		}
	};

	struct tInput
	{
		tColumnRef	mColumn;
		tData		mValue;
		u32			mCompareFunc;

		tGrowableArray< tInput > mAnds; //needs all of these too.
		tGrowableArray< tInput > mOrs;  //or any of these.

		tInput( ) : mCompareFunc( tData::cComparisonInvalid ) { }

		tInput( const tColumnRef& column, u32 compareType, const tData& value )
			: mColumn( column )
			, mValue( value )
			, mCompareFunc( compareType )
		{ }

		// Return false if this comparison caused a failure. True if it wasn't applicable or comparison passed.
		b32 fCompare( const tDataTableRef& table, u32 row ) const;
	};

	struct tMapping
	{
		tColumnRef	mLeft;
		tColumnRef	mRight;
		u32			mCompareFunc;

		tMapping( ) : mCompareFunc( tData::cComparisonInvalid ) { }

		tMapping( const tColumnRef& left, u32 compareFunc, const tColumnRef& right )
			: mLeft( left )
			, mRight( right )
			, mCompareFunc( compareFunc )
		{ }

		b32 operator < ( const tMapping& other ) const
		{
			return (other.mRight == mLeft); 
		}
	};

	typedef tGrowableArray< tData > tRow;

	class tDataTableQueryResult
	{
	public:
		tGrowableArray< tRow > mOutput;

		u32 fRowCount( ) const { return mOutput.fCount( ); }

		template< typename t >
		t fIndexByRowCol( u32 row, u32 col ) const
		{
			t ret;
			mOutput[ row ][ col ].fGet( ret );
			return ret;
		}

		u32 fFindRow( const tRow& compare ) const;
		void fLog( ) const;
	};

	class tDataTableQuery
	{
	public:
		tInput							mInput;
		tGrowableArray< tColumnRef >	mOutputs;
		tGrowableArray< tMapping >		mMappings;
		b32								mUnique;

		tDataTableQuery( )
			: mUnique( true )
		{ }

		void fExecute( tDataTableQueryResult& result );

	private:
		b32 fMatchRow( tDataTableRef& table, u32 row, tRow& potentialRow, tGrowableArray< tDataTableRef >& searchHistory, tDataTableQuery::tInput* additionalConstraint = NULL );
	};
} }

#endif //__tDataTableQuery__