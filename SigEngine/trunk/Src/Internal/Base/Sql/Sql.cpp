//------------------------------------------------------------------------------
// \file Sql.cpp - 28 Oct 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "Sql.hpp"

namespace Sig { namespace Sql 
{
	//------------------------------------------------------------------------------
	// tSelect
	//------------------------------------------------------------------------------
	tSelect::tSelect( const char * tableName )
		: mTableName( tableName )
		, mDistinct( false )
	{
	}

	//------------------------------------------------------------------------------
	std::string tSelect::fBuild( ) const
	{
		std::stringstream ss;
		ss << "SELECT ";

		if( mDistinct )
			ss << "DISTINCT ";

		const u32 columnCount = mColumnNames.fCount( );
		if( !columnCount ) ss << "*";
		else
		{
			// Add the first
			ss << mColumnNames[ 0 ].c_str( );

			// Add the rest prepended with ','
			for( u32 c = 1; c < columnCount; ++c )
				ss << "," << mColumnNames[ c ].c_str( );
		}

		ss << " FROM " << mTableName.c_str( );

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	// tWhere
	//------------------------------------------------------------------------------
	void tWhere::fPushScope( )
	{
		mCurrentScope++;
	}

	//------------------------------------------------------------------------------
	void tWhere::fPopScope( )
	{
		sigassert( mCurrentScope && "Invalid scope pop in WHERE clause" );
		--mCurrentScope;
	}

	//------------------------------------------------------------------------------
	void tWhere::fEqual( const char * column, const char * value, b32 columnIsText )
	{
		fBuildSimpleOp( cOperatorEqual, column, value, columnIsText );
	}

	//------------------------------------------------------------------------------
	void tWhere::fNotEqual( const char * column, const char * value, b32 columnIsText )
	{
		fBuildSimpleOp( cOperatorNotEqual, column, value, columnIsText );
	}

	//------------------------------------------------------------------------------
	void tWhere::fLess( const char * column, const char * value, b32 columnIsText )
	{
		fBuildSimpleOp( cOperatorLess, column, value, columnIsText );
	}

	//------------------------------------------------------------------------------
	void tWhere::fGreater( const char * column, const char * value, b32 columnIsText )
	{
		fBuildSimpleOp( cOperatorGreater, column, value, columnIsText );
	}

	//------------------------------------------------------------------------------
	void tWhere::fLessOrEqual( const char * column, const char * value, b32 columnIsText )
	{
		fBuildSimpleOp( cOperatorLessOrEqual, column, value, columnIsText );
	}

	//------------------------------------------------------------------------------
	void tWhere::fGreatorOrEqual( const char * column, const char * value, b32 columnIsText )
	{
		fBuildSimpleOp( cOperatorGreaterOrEqual, column, value, columnIsText );
	}

	//------------------------------------------------------------------------------
	void tWhere::fBetween( const char * column, const char * value1, const char * value2, b32 columnIsText )
	{
		std::stringstream ss;
		
		if( columnIsText ) ss << "'" << value1 << "'";
		else ss << value1;

		ss << " AND ";

		if( columnIsText ) ss << "'" << value2 << "'";
		else ss << value2;		

		fPushOperation( cOperatorBetween, column, ss.str( ).c_str( ) );
	}

	//------------------------------------------------------------------------------
	void tWhere::fLike( const char * column, const char * pattern, b32 not )
	{
		fBuildSimpleOp( not ? cOperatorNotLike : cOperatorLike, column, pattern, true );
	}

	//------------------------------------------------------------------------------
	void tWhere::fIn( const char * column, u32 count, const char * values[], b32 columnIsText )
	{
		sigassert( count >= 2  && "IN operation requires a minimum of 2 values" );

		std::stringstream ss;
		ss << "(";

		// First one
		if( columnIsText ) ss << "'" << values[ 0 ] << "'";
		else ss << values[ 0 ];

		// The rest
		if( columnIsText )
		{
			for( u32 v = 1; v < count; ++v )
				ss << ",'" << values[ v ] << "'";
		}
		else
		{
			for( u32 v = 1; v < count; ++v )
				ss << "," << values[ v ];
		}

		ss << ")";

		fPushOperation( cOperatorIn, column, ss.str( ).c_str( ) );
	}

	//------------------------------------------------------------------------------
	void tWhere::fAnd( )
	{
		fPushOperation( cOperatorAnd, "", "" );
	}

	//------------------------------------------------------------------------------
	void tWhere::fOr( )
	{
		fPushOperation( cOperatorOr, "", "" );
	}

	//------------------------------------------------------------------------------
	std::string tWhere::fBuild( ) const
	{
		if( !mOperations.fCount( ) )
			return "";

		std::stringstream ss; ss << "WHERE ";

		u32 buildScope = 0;
		const u32 opCount = mOperations.fCount( );
		for( u32 o = 0; o < opCount; ++o )
		{
			const tOperation & op = mOperations[ o ];

			// Build up to scope
			while( buildScope < op.mScope )
			{
				ss << "(";
				++buildScope;
			}

			// Reduce down to scope
			while( buildScope > op.mScope )
			{
				ss << ")";
				--buildScope;
			}

			// Push the column
			ss << op.mColumnName;

			// Push the operation
			switch( op.mOperator )
			{
			case cOperatorEqual:			ss << "="; break;
			case cOperatorNotEqual:			ss << "<>"; break;
			case cOperatorLess:				ss << "<"; break;
			case cOperatorGreater:			ss << ">"; break;
			case cOperatorLessOrEqual:		ss << "<="; break;
			case cOperatorGreaterOrEqual:	ss << ">="; break;
			case cOperatorBetween:			ss << " BETWEEN "; break;
			case cOperatorLike:				ss << " LIKE "; break;
			case cOperatorNotLike:			ss << " NOT LIKE "; break;
			case cOperatorIn:				ss << " IN "; break;
			case cOperatorAnd:				ss << " AND "; break;
			case cOperatorOr:				ss << " OR "; break;
			default: sig_nodefault( ); break;
			}

			// Push the value
			ss << op.mValue;
		}
		
		// Reduce down to no scope
		while( buildScope-- )
			ss << ")";

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	void tWhere::fBuildSimpleOp( u32 op, const char * col, const char * value, b32 isText )
	{
		if( isText )
		{
			std::stringstream ss;
			ss << "'" << value << "'";
			fPushOperation( op, col, ss.str( ).c_str( ) );
		}
		else fPushOperation( op, col, value );
	}

	//------------------------------------------------------------------------------
	void tWhere::fPushOperation( u32 op, const char * col, const char * value )
	{
		tOperation operation;
		operation.mColumnName = col;
		operation.mOperator = op;
		operation.mScope = mCurrentScope;
		operation.mValue = value;

		mOperations.fPushBack( operation );
	}

	//------------------------------------------------------------------------------
	// tOrderBy
	//------------------------------------------------------------------------------
	void tOrderBy::fAddAscend( const char * column, b32 explicitASC )
	{
		fPushOrder( explicitASC ? cAscendExplicit : cAscend, column );
	}

	//------------------------------------------------------------------------------
	void tOrderBy::fAddDescend( const char * column )
	{
		fPushOrder( cDescend, column );
	}

	//------------------------------------------------------------------------------
	std::string tOrderBy::fBuild( ) const
	{
		if( !mOrders.fCount( ) )
			return "";
		
		std::stringstream ss;
		ss << "ORDER BY ";

		const u32 orderCount = mOrders.fCount( );
		for( u32 o = 0; o < orderCount; ++o )
		{
			const tOrder & order = mOrders[ o ];

			if( o ) ss << ", ";

			ss << order.mColumn;

			switch( order.mDirection )
			{
			case cAscend: break;
			case cAscendExplicit:	ss << " ASC"; break;
			case cDescend:			ss << " DESC"; break;
			default: sig_nodefault( ); break;
			}
		}

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	void tOrderBy::fPushOrder( u32 dir, const char * column )
	{
		tOrder order;
		order.mDirection = dir;
		order.mColumn = column;

		mOrders.fPushBack( order );
	}

	//------------------------------------------------------------------------------
	// tInsertInto
	//------------------------------------------------------------------------------
	tInsertInto::tInsertInto( const char * tableName )
		: mTableName( tableName )
	{
	}

	//------------------------------------------------------------------------------
	void tInsertInto::fAddValue( const char * value, b32 columnIsText )
	{
		sigassert( !mColumns.fCount( ) && 
			"Cannot add value only entries if a column has already been added" );

		fPushValue( value, columnIsText );
	}

	//------------------------------------------------------------------------------
	void tInsertInto::fAddValue( const char * column, const char * value, b32 columnIsText )
	{
		sigassert( ( mValues.fCount( ) == mColumns.fCount( ) )  && 
			"Cannot add column/value entries if a value only entry has already been added" );

		fPushValue( value, columnIsText );
		mColumns.fPushBack( std::string( column ) );
	}

	//------------------------------------------------------------------------------
	void tInsertInto::fAddColumn( const char * column )
	{
		sigassert( !mValues.fCount( ) && 
			"Cannot add column only entries if a value has already been added" );

		mColumns.fPushBack( std::string( column ) );
	}

	//------------------------------------------------------------------------------
	std::string tInsertInto::fBuild( ) const
	{
		const u32 columnCount = mColumns.fCount( );
		const u32 valueCount = mValues.fCount( );

		sigassert( ( columnCount || valueCount ) && "Insert Into operation requires column and/or value information" );

		std::stringstream ss;
		ss << "INSERT INTO " << mTableName << " ";

		if( columnCount )
		{
			sigassert( ( !valueCount || valueCount == columnCount ) && "Sanity!" );
			
			ss << "(";
			
			// First one
			ss << mColumns[ 0 ];

			// The rest
			for( u32 c = 1; c < columnCount; ++c )
				ss << ", " << mColumns[ c ];

			ss << ")";
		}

		if( valueCount )
		{
			sigassert( ( !columnCount || columnCount == valueCount ) && "Sanity!" );

			if( columnCount ) ss << " ";
			ss << "VALUES (";

			// First one
			ss << mValues[ 0 ];

			// The rest
			for( u32 v = 1; v < valueCount; ++v )
				ss << ", " << mValues[ v ];

			ss << ")";
		}

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	void tInsertInto::fPushValue( const char * value, b32 isText )
	{
		if( isText )
		{
			std::stringstream ss;
			ss << "'" << value << "'";
			mValues.fPushBack( ss.str( ) );
		}
		else mValues.fPushBack( std::string( value ) );
	}

	//------------------------------------------------------------------------------
	// tUpdate
	//------------------------------------------------------------------------------
	tUpdate::tUpdate( const char * tableName )
		: mTableName( tableName )
	{

	}

	//------------------------------------------------------------------------------
	void tUpdate::fSet( const char * columnName, const char * value, b32 columnIsText )
	{
		std::stringstream ss;
		ss << columnName << "=";

		if( columnIsText ) ss << "'" << value << "'";
		else ss << value;

		mSets.fPushBack( ss.str( ) );
	}

	//------------------------------------------------------------------------------
	std::string tUpdate::fBuild( ) const
	{
		sigassert( mSets.fCount( ) && "Cannot update table without any columns to set" );

		std::stringstream ss;
		ss << "UPDATE " << mTableName << " SET ";

		// First one
		ss << mSets[ 0 ];

		// The rest
		const u32 setCount = mSets.fCount( );
		for( u32 s = 1; s < setCount; ++s )
			ss << ", " << mSets[ s ];

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	// tDelete
	//------------------------------------------------------------------------------
	std::string tDelete::fBuild( ) const
	{
		std::stringstream ss;

		ss << "DELETE FROM " << mTableName;

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	// tLimit
	//------------------------------------------------------------------------------
	std::string tLimit::fBuild( ) const
	{
		std::stringstream ss;
		ss << "LIMIT " << mLimit;
		return ss.str( );
	}

	//------------------------------------------------------------------------------
	// tArray
	//------------------------------------------------------------------------------
	void tArray::fAddValue( const char * value, b32 isText )
	{
		if( isText )
		{
			std::stringstream ss;
			ss << "\"" << value << "\"";
			mValues.fPushBack( ss.str( ) );
		}
		else mValues.fPushBack( std::string( value ) );
	}

	//------------------------------------------------------------------------------
	std::string tArray::fBuild( ) const
	{
		const u32 valueCount = mValues.fCount( );

		if( !valueCount )
			return "{}";

		std::stringstream ss;

		ss << "{";

		// First one
		ss << mValues[ 0 ];

		// The rest
		for( u32 v = 1; v < valueCount; ++v )
			ss << ", " << mValues[ v ];

		ss << "}";

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	// tSelectFunction
	//------------------------------------------------------------------------------
	tSelectFunction::tSelectFunction( const char * functionName )
		: mFunctionName( functionName )
	{
	}

	//------------------------------------------------------------------------------
	void tSelectFunction::fAddArgument( const char * argument, b32 columnIsText ) 
	{ 
		if( columnIsText ) 
		{	
			std::stringstream ss;
			ss << "'" << argument << "'";
			mArguments.fPushBack( ss.str( ) );
		}
		else mArguments.fPushBack( std::string( argument ) );
			
	}

	//------------------------------------------------------------------------------
	std::string tSelectFunction::fBuild( ) const
	{
		std::stringstream ss;
		ss << "SELECT ";
			
	
		// Columns
		if( const u32 columnCount = mColumnNames.fCount( ) )
		{
			// Add the first
			ss << mColumnNames[ 0 ].c_str( );

			// Add the rest prepended with ','
			for( u32 c = 1; c < columnCount; ++c )
				ss << "," << mColumnNames[ c ].c_str( );
		}
		else ss << "*";

		ss << " FROM " << mFunctionName << "( ";

		// Args
		if( const u32 argCount = mArguments.fCount( ) )
		{
			// Add the first
			ss << mArguments[ 0 ].c_str( );

			// Add the rest prepended with ','
			for( u32 a = 1; a < argCount; ++a )
				ss << "," << mArguments[ a ].c_str( );
		}

		ss << " )";

		return ss.str( );
	}


} } // ::Sig::Sql