//------------------------------------------------------------------------------
// \file Sql.hpp - 28 Oct 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __Sql__
#define __Sql__

namespace Sig { namespace Sql 
{
	///
	/// \class tSelect
	/// \brief Helper for building an SQL SELECT statement. If no column names are added
	///		   then SELECT * is used
	class base_export tSelect
	{
	public:
		
		tSelect( const char * tableName );
		
		void fSetDistinct( b32 distinct ) { mDistinct = distinct; }
		void fAddColumn( const char * columnName ) { mColumnNames.fPushBack( std::string( columnName ) ); }

		std::string fBuild( ) const;

	private:

		b32 mDistinct;
		std::string mTableName;
		tGrowableArray< std::string > mColumnNames;
	};


	///
	/// \class tWhere
	/// \brief Helper for building an SQL WHERE Clause.
	class base_export tWhere
	{
	public:

		tWhere( ) : mCurrentScope( 0 ) { }

		void fPushScope( );
		void fPopScope( );

		void fEqual( const char * column, const char * value, b32 columnIsText );
		void fNotEqual( const char * column, const char * value, b32 columnIsText );
		void fLess( const char * column, const char * value, b32 columnIsText );
		void fGreater( const char * column, const char * value, b32 columnIsText );
		void fLessOrEqual( const char * column, const char * value, b32 columnIsText );
		void fGreatorOrEqual( const char * column, const char * value, b32 columnIsText );
		void fBetween( const char * column, const char * value1, const char * value2, b32 columnIsText );
		void fLike( const char * column, const char * pattern, b32 not );
		void fIn( const char * column, u32 count, const char * values[], b32 columnIsText );

		void fAnd( );
		void fOr( );

		std::string fBuild( ) const;

	private:

		enum tOperator
		{
			cOperatorEqual,
			cOperatorNotEqual,
			cOperatorLess,
			cOperatorGreater,
			cOperatorLessOrEqual,
			cOperatorGreaterOrEqual,
			cOperatorBetween,
			cOperatorLike,
			cOperatorNotLike,
			cOperatorIn,

			cOperatorAnd,
			cOperatorOr,
		};


		struct tOperation
		{
			u32 mScope;
			u32 mOperator;
			std::string mColumnName;
			std::string mValue;
		};

	private:

		void fBuildSimpleOp( u32 op, const char * col, const char * value, b32 isText );
		void fPushOperation( u32 op, const char * col, const char * value );

	private:

		u32 mCurrentScope;
		tGrowableArray< tOperation > mOperations;
	};

	///
	/// \class tOrderBy
	/// \brief Helper for building an SQL ORDER BY Statement
	class base_export tOrderBy
	{
	public:

		void fAddAscend( const char * column, b32 explicitASC = false );
		void fAddDescend( const char * column );

		std::string fBuild( ) const;

	private:

		enum { cAscend, cAscendExplicit, cDescend };

		struct tOrder
		{
			u32 mDirection;
			std::string mColumn;
		};

	private:

		void fPushOrder( u32 dir, const char * column );

	private:

		tGrowableArray< tOrder > mOrders;

	};

	///
	/// \class tInsertInto
	/// \brief Helper for building SQL INSERT INTO command
	class base_export tInsertInto
	{
	public:

		tInsertInto( const char * tableName );

		// Once one of these methods of adding values has been used, 
		// it is the only method that can be used without causing an assert.
		void fAddValue( const char * value, b32 columnIsText );
		void fAddValue( const char * column, const char * value, b32 columnIsText );
		void fAddColumn( const char * column );		

		std::string fBuild( ) const;

	private:

		void fPushValue( const char * value, b32 isText );

	private:
		
		u32 mValueSetting;
		std::string mTableName;

		tGrowableArray< std::string > mColumns;
		tGrowableArray< std::string > mValues;
	};

	///
	/// \class tUpdate
	/// \brief Helper for building SQL UPDATE command
	class base_export tUpdate
	{
	public:

		tUpdate( const char * tableName );

		void fSet( const char * columnName, const char * value, b32 columnIsText );

		std::string fBuild( ) const;

	private:

		std::string mTableName;
		tGrowableArray< std::string > mSets;
	};

	///
	/// \class tDelete
	/// \brief Helper for building SQL DELETE FROM command
	class base_export tDelete
	{
	public:

		tDelete( const char * tableName ) : mTableName( tableName ) { }

		std::string fBuild( ) const;

	private:
		std::string mTableName;
	};

	///
	/// \class tLimit
	/// \brief Helper for building SQL LIMIT statements for queries
	class base_export tLimit
	{
	public:

		tLimit( u32 limit ) : mLimit( limit ) { }
		void fSetLimit( u32 limit ) { mLimit = limit; }

		std::string fBuild( ) const;

	private:

		u32 mLimit;
	};

	///
	/// \class tArray
	/// \brief Helper for building SQL Arrays
	class base_export tArray
	{
	public:

		void fAddValue( const char * value, b32 isText );

		std::string fBuild( ) const;

	private:

		tGrowableArray<std::string> mValues;
	};

	///
	/// \class tSelectFunction
	/// \brief Helper for building an SQL function call
	class base_export tSelectFunction
	{
	public:
		
		tSelectFunction( const char * functionName );

		void fAddColumn( const char * columnName ) { mColumnNames.fPushBack( std::string( columnName ) ); }
		void fAddArgument( const char * argument, b32 columnIsText );

		template< class tArgType >
		void fAddArgument( const tArgType & arg, b32 columnIsText )
		{
			std::stringstream ss; ss << arg; 
			fAddArgument( ss.str( ).c_str( ), columnIsText );
		}

		std::string fBuild( ) const;

	private:

		std::string mFunctionName;
		tGrowableArray< std::string > mArguments;
		tGrowableArray< std::string > mColumnNames;
	};

} } // ::Sig::Sql


#endif//__Sql__
