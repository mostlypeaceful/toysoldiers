#include "UnitTestsPch.hpp"
#include "tXmlFile.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"

using namespace Sig;

define_unittest( TestXmlFile )
{
	tXmlFileData xf;
	tXmlNode root = xf.fCreateRoot( "maximus" );
	tXmlNode a = root.fCreateChild( "a" );
	tXmlNode b = a.fCreateChild( "b" );
	a.fSetAttribute( "seek", 6 );
	a.fSetAttribute( "bleek", "yaya" );
	b.fSetAttribute( "eek", 5 );
	b.fSetAttribute( "eek", 4 );
	b.fSetContents( "abcdefghijklmnopqrstuvwxyz\nabcdefghijklmnopqrstuvwxyz\n" );

	int eek;
	const b32 success0 = b.fGetAttribute( "eek", eek );
	fAssert( success0 );
	fAssertEqual( eek, 4 );

	tFilePathPtr testPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePathPtr( "test.xml" ) );

	const b32 success1 = xf.fSave( testPath, false );
	fAssert( success1 );

	const b32 success2 = xf.fLoad( testPath );
	fAssert( success2 );

	if( xf.fGetRoot( ).fFindChild( a, "a" ) )
	{
		tXmlNodeList asChildren;
		const b32 success3 = a.fFindChildren( asChildren, "b" );
		fAssert( success3 );
		fAssertEqual( ( u32 )asChildren.fCount( ), 1u );
	}
}

class tTestXmlSerialization
{
public:

	struct tInner
	{
		tInner( ) : mI( 3 ) { }

		template<class tSerializer>
		void fSerializeXml( tSerializer& s );

		u32 mI;
	};

	std::string mNameAttribute;
	std::string mNameReal;

	b32 mWhatever;
	tGrowableArray<u32> mArray;
	tGrowableArray<tInner> mArray2;
	tGrowableArray< tStrongPtr<tInner> > mArray3;
	Math::tVec3f mCenter;

	b32 fSaveXml( const tFilePathPtr& path );
	b32 fLoadXml( const tFilePathPtr& path );

	template<class tSerializer>
	void fSerializeXml( tSerializer& s );

};

template<class tSerializer>
void tTestXmlSerialization::tInner::fSerializeXml( tSerializer& s )
{
	s( "mI", mI );
}


template<class tSerializer>
void tTestXmlSerialization::fSerializeXml( tSerializer& s )
{
	s.fAsAttribute( "NameAttr", mNameAttribute );
	s( "NameReal", mNameReal );
	s( "Whatever", mWhatever );
	s( "Array", mArray );
	s( "Array2", mArray2 );
	s( "Array3", mArray3 );
	s( "Center", mCenter );
}

b32 tTestXmlSerialization::fSaveXml( const tFilePathPtr& path )
{
	tXmlSerializer ser;
	return ser.fSave( path, "testxml", *this, false );
}

b32 tTestXmlSerialization::fLoadXml( const tFilePathPtr& path )
{
	tXmlDeserializer des;
	return des.fLoad( path, "testxml", *this );
}

define_unittest( TestXmlSerialize )
{
	tFilePathPtr testPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePathPtr( "serialize.xml" ) );

	tTestXmlSerialization obj;

	obj.mNameAttribute = "attribute";
	obj.mNameReal = "real";
	obj.mCenter.x = 100.f;
	obj.mCenter.y = 200.f;
	obj.mCenter.z = 300.f;
	obj.mWhatever = 1;
	obj.mArray.fSetCount( 3 );
	obj.mArray[0] = 11;
	obj.mArray[1] = 7;
	obj.mArray[2] = 13;
	obj.mArray2.fSetCount( 2 );
	obj.mArray2[0].mI = 4;
	obj.mArray2[1].mI = 6;
	obj.mArray3.fSetCount( 2 );
	obj.mArray3[0] = fNewStrongPtr<tTestXmlSerialization::tInner>( );
	obj.mArray3[1] = fNewStrongPtr<tTestXmlSerialization::tInner>( );
	obj.mArray3[0]->mI = 3030;
	obj.mArray3[1]->mI = 6060;

	const tTestXmlSerialization refObj = obj;

	obj.fSaveXml( testPath );

	obj = tTestXmlSerialization( );

	obj.fLoadXml( testPath );

	fAssertEqual( obj.mNameAttribute, refObj.mNameAttribute );
	fAssertEqual( obj.mNameReal, refObj.mNameReal );
	fAssertEqual( obj.mCenter.x, refObj.mCenter.x );
	fAssertEqual( obj.mCenter.y, refObj.mCenter.y );
	fAssertEqual( obj.mCenter.z, refObj.mCenter.z );
	fAssertEqual( obj.mWhatever, refObj.mWhatever );
	fAssertEqual( ( u32 )obj.mArray.fCount( ), ( u32 )refObj.mArray.fCount( ) );
	for( u32 i = 0; i < obj.mArray.fCount( ); ++i )
		fAssertEqual( obj.mArray[i], refObj.mArray[i] );
	fAssertEqual( ( u32 )obj.mArray2.fCount( ), ( u32 )refObj.mArray2.fCount( ) );
	for( u32 i = 0; i < obj.mArray2.fCount( ); ++i )
		fAssertEqual( obj.mArray2[i].mI, refObj.mArray2[i].mI );
	fAssertEqual( ( u32 )obj.mArray3.fCount( ), ( u32 )refObj.mArray3.fCount( ) );
	for( u32 i = 0; i < obj.mArray3.fCount( ); ++i )
	{
		fAssert( !obj.mArray3[i].fNull( ) );
		fAssertEqual( obj.mArray3[i]->mI, refObj.mArray3[i]->mI );
	}
}
