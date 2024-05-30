//------------------------------------------------------------------------------
// \file TestSql.cpp - 28 Oct 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "UnitTestsPch.hpp"
#include "Sql/Sql.hpp"

using namespace Sig;

define_unittest(TestSql)
{
	const char * const cTableName = "TableName";
	const char * const cColumnName0 = "Column0";
	const char * const cColumnName1 = "Column1";
	const char * const cColumnName2 = "Column2";

	// SELECT
	{
		Sql::tSelect select( cTableName );
		
		std::string target( "SELECT * FROM TableName" );
		std::string actual = select.fBuild( );
		fAssertEqual( target, actual );

		target = "SELECT Column0 FROM TableName";
		select.fAddColumn( cColumnName0 );
		actual = select.fBuild( );
		fAssertEqual( target, actual );

		target = "SELECT Column0,Column1,Column2 FROM TableName";
		select.fAddColumn( cColumnName1 );
		select.fAddColumn( cColumnName2 );
		actual = select.fBuild( );
		fAssertEqual( target, actual );

		target = "SELECT DISTINCT Column0,Column1,Column2 FROM TableName";
		select.fSetDistinct( true );
		actual = select.fBuild( );
		fAssertEqual( target, actual );
	}

	// WHERE
	{
		Sql::tWhere whereClause;

		const char * testText[] = { "Hello", "World", "4Real" };
		const char * testNum[] = { "1", "2", "3" };

		whereClause.fPushScope( );
			whereClause.fIn( cColumnName0, array_length( testText ), testText, true );
			whereClause.fOr( );
			whereClause.fIn( cColumnName1, array_length( testNum ), testNum, false );
		whereClause.fPopScope( );
		whereClause.fAnd( );
		whereClause.fPushScope( );
			whereClause.fEqual( cColumnName2, "thug", true );
			whereClause.fAnd( );
			whereClause.fNotEqual( cColumnName2, "goof", true );
			whereClause.fOr( );
			whereClause.fPushScope( );
				whereClause.fLess( cColumnName1, "10", false );
				whereClause.fAnd( );
				whereClause.fGreatorOrEqual( cColumnName1, "5", false );
			whereClause.fPopScope( );
			whereClause.fAnd( );
			whereClause.fPushScope( );
				whereClause.fGreater( cColumnName0, "Butts", true );
				whereClause.fOr( );
				whereClause.fLessOrEqual( cColumnName0, "Hella", true );
			whereClause.fPopScope( );
		whereClause.fPopScope( );
		whereClause.fOr( );
		whereClause.fPushScope( );
			whereClause.fBetween( cColumnName1, "50", "100", false );
			whereClause.fAnd( );
			whereClause.fPushScope( );
				whereClause.fLike( cColumnName0, "%dude%", false );
				whereClause.fOr( );
				whereClause.fLike( cColumnName0, "%bro%", true );
			whereClause.fPopScope( );
		whereClause.fPopScope( );


		std::string target = "WHERE (Column0 IN ('Hello','World','4Real') OR Column1 IN (1,2,3)) AND "
							 "(Column2='thug' AND Column2<>'goof' OR (Column1<10 AND Column1>=5) AND (Column0>'Butts' OR Column0<='Hella')) OR "
							 "(Column1 BETWEEN 50 AND 100 AND (Column0 LIKE '%dude%' OR Column0 NOT LIKE '%bro%'))";
		std::string actual = whereClause.fBuild( );
		fAssertEqual( target, actual );
	}

	// Order By
	{
		Sql::tOrderBy orderBy;

		orderBy.fAddAscend( cColumnName0 );
		std::string target = "ORDER BY Column0";
		std::string actual = orderBy.fBuild( );
		fAssertEqual( target, actual );

		orderBy.fAddDescend( cColumnName1 );
		orderBy.fAddAscend( cColumnName2, true );
		target = "ORDER BY Column0, Column1 DESC, Column2 ASC";
		actual = orderBy.fBuild( );
		fAssertEqual( target, actual );
	}

	// Insert Into
	{
		Sql::tInsertInto columnTest( cTableName );
		columnTest.fAddColumn( cColumnName0 );

		std::string target = "INSERT INTO TableName (Column0)";
		std::string actual = columnTest.fBuild( );
		fAssertEqual( target, actual );

		columnTest.fAddColumn( cColumnName1 );
		columnTest.fAddColumn( cColumnName2 );
		target = "INSERT INTO TableName (Column0, Column1, Column2)";
		actual = columnTest.fBuild( );
		fAssertEqual( target, actual );

		Sql::tInsertInto valueTest( cTableName );
		valueTest.fAddValue( "Name", true );
		valueTest.fAddValue( "3", false );
		valueTest.fAddValue( "CityStreet", true );
		target = "INSERT INTO TableName VALUES ('Name', 3, 'CityStreet')";
		actual = valueTest.fBuild( );
		fAssertEqual( target, actual );

		Sql::tInsertInto columnValueTest( cTableName );
		columnValueTest.fAddValue( cColumnName0, "Name", true );
		columnValueTest.fAddValue( cColumnName1, "3", false);
		columnValueTest.fAddValue( cColumnName2, "CityStreet", true );
		target = "INSERT INTO TableName (Column0, Column1, Column2) VALUES ('Name', 3, 'CityStreet')";
		actual = columnValueTest.fBuild( );
		fAssertEqual( target, actual );
	}

	// Update
	{
		Sql::tUpdate update( cTableName );

		update.fSet( cColumnName0, "Name", true );
		std::string target = "UPDATE TableName SET Column0='Name'";
		std::string actual = update.fBuild( );
		fAssertEqual( target, actual );

		update.fSet( cColumnName1, "3", false );
		update.fSet( cColumnName2, "CityStreet", true );
		target = "UPDATE TableName SET Column0='Name', Column1=3, Column2='CityStreet'";
		actual = update.fBuild( );
		fAssertEqual( target, actual );
	}

	// Delete
	{
		Sql::tDelete del( cTableName );

		std::string target = "DELETE FROM TableName";
		std::string actual = del.fBuild( );
		fAssertEqual( target, actual );
	}

	// Limit
	{
		Sql::tLimit limit( 15 );

		std::string target = "LIMIT 15";
		std::string actual = limit.fBuild( );
		fAssertEqual( target, actual );
	}

	// Array
	{
		Sql::tArray strTest;
		
		strTest.fAddValue( "Hello", true );
		std::string target = "{\"Hello\"}";
		std::string actual = strTest.fBuild( );
		fAssertEqual( target, actual );

		strTest.fAddValue( "World", true );
		target = "{\"Hello\", \"World\"}";
		actual = strTest.fBuild( );
		fAssertEqual( target, actual );

		Sql::tArray valTest;

		valTest.fAddValue( "10", false );
		target = "{10}";
		actual = valTest.fBuild( );
		fAssertEqual( target, actual );

		valTest.fAddValue( "500", false );
		target = "{10, 500}";
		actual = valTest.fBuild( );
		fAssertEqual( target, actual );
	}
}

