#include "UnitTestsPch.hpp"
#include "TestSerializable.hpp"
#include "tFileWriter.hpp"
#include "FileSystem.hpp"
#include "tStrongPtr.hpp"
#include "tRandom.hpp"
#include "UnitTests.rtti.hpp"

using namespace Sig;

define_unittest( TestSerializable )
{
	tFilePathPtr testPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePathPtr( "a.class.bin" ) );

	A ao0,ao1;
	tDynamicBuffer dataBuffer;

	{
		ao0.pb = &ao0.b;
		ao0.pc = &ao0.b.c;
		ao0.pa = &ao1;
		ao0.b.pa = &ao0;

		ao1.pb = &ao1.b;
		ao1.pc = &ao1.b.c;
		ao1.pa = 0;

		tFileWriter o( testPath );
		tLoadInPlaceSerializer so;
		so.fSave( ao0, o, cCurrentPlatform );
	}

	{
		FileSystem::fReadFileToBuffer( dataBuffer, testPath );
		tLoadInPlaceDeserializer si;
		si.fLoad<A>( dataBuffer.fBegin( ) );
	}

	A& ai = *reinterpret_cast<A*>( dataBuffer.fBegin( ) );
	fAssertNotNull( ai.pa );
	fAssertNotNull( ai.pb );
	fAssertNotNull( ai.pc );
	fAssertNotNull( ai.b.pa );
	fAssertEqual( ai.b.pa, &ai );
	fAssertNull( ai.pa->pa );
	fAssertEqual( ai.pb, &ai.b );
	fAssertEqual( ai.pc, &ai.b.c );
	fAssertNotEqual( ai.pa, &ai );
	fAssertEqual( ai.pa->i, 0xa );
	fAssertEqual( ai.pb->i, 0xb );
	fAssertEqual( ai.pc->i, 0xc );
	fAssertEqual( ai.i, 0xa );
	fAssertEqual( ai.b.i, 0xb );
	fAssertEqual( ai.b.c.i, 0xc );
}

define_unittest( TestPolymorphicSerializable )
{
	tStrongPtr<iBase> basePtr0( new iDerived0 );
	tStrongPtr<iBase> basePtr1( new iDerived1 );

	fAssertEqual( basePtr0->fClassId( ), iDerived0::cClassId );
	fAssertEqual( basePtr1->fClassId( ), iDerived1::cClassId );

	basePtr0 = tStrongPtr<iBase>( ( iBase* )Rtti::fNewClass( iDerived0::cClassId ) );
	basePtr1 = tStrongPtr<iBase>( ( iBase* )Rtti::fNewClass( iDerived1::cClassId ) );

	fAssertEqual( basePtr0.fNull( ), false );
	fAssertEqual( basePtr1.fNull( ), false );
	fAssertEqual( basePtr0->fClassId( ), iDerived0::cClassId );
	fAssertEqual( basePtr1->fClassId( ), iDerived1::cClassId );


	tFilePathPtr testPath0 = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePathPtr( "iDerived0.class.bin" ) );
	tFilePathPtr testPath1 = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePathPtr( "iDerived1.class.bin" ) );

	tDynamicBuffer dataBuffer0,dataBuffer1;

	{
		tFileWriter o( testPath0 );
		tLoadInPlaceSerializer so;
		so.fSave( *basePtr0.fGetRawPtr( ), o, cCurrentPlatform );
	}

	{
		tFileWriter o( testPath1 );
		tLoadInPlaceSerializer so;
		so.fSave( *basePtr1.fGetRawPtr( ), o, cCurrentPlatform );
	}

	{
		FileSystem::fReadFileToBuffer( dataBuffer0, testPath0 );
		tLoadInPlaceDeserializer si;
		si.fLoad<iBase>( dataBuffer0.fBegin( ) );

		iBase* ptr =  ( iBase* )dataBuffer0.fBegin( );
		const f32 v = ptr->fGetVelocity( );
		fAssertEqual( v, 3.f );
	}

	{
		FileSystem::fReadFileToBuffer( dataBuffer1, testPath1 );
		tLoadInPlaceDeserializer si;
		si.fLoad<iBase>( dataBuffer1.fBegin( ) );

		iBase* ptr =  ( iBase* )dataBuffer1.fBegin( );
		const f32 v = ptr->fGetVelocity( );
		fAssertEqual( v, 4.f );
	}
}

using namespace SerializeTest;
namespace Sig { namespace SerializeTest
{
	tSubMesh::tSubMesh( )
	{
		const f32 worldSize = 20.f + 2 * ( fRound<u32>( 100.f * tRandom::fSubjectiveRand( ).fFloatZeroToOne( ) ) / 2 );
		const f32 increment = 0.5f;
		const Math::tAabbf aabb( Math::tVec3f( -worldSize ), Math::tVec3f( +worldSize ) );

		const u32 numTris = ( u32 )Math::fSquare( 2.f * worldSize / increment );
		tPolySoupVertexList verts( numTris * 3 );
		tPolySoupTriangleList tris( numTris );
		u32 ithVertex = 0;
		u32 ithTri = 0;

		for( f32 i = -worldSize; i < worldSize; i += increment )
		{
			for( f32 j = -worldSize; j < worldSize; j += increment )
			{
				verts[ ithVertex++ ] = Math::tVec3f( i, 0.f, j );
				verts[ ithVertex++ ] = Math::tVec3f( i + increment, 0.f, j );
				verts[ ithVertex++ ] = Math::tVec3f( i, 0.f, j + increment );
				tris[ ithTri++ ] = Math::tVec3u( ithVertex - 3, ithVertex - 2, ithVertex - 1 );
			}
		}

		sigassert( ithVertex == verts.fCount( ) );
		sigassert( ithTri == tris.fCount( ) );

		mOctree.fConstruct( aabb, verts, tris, 6, 64, 0.25f );
	}

	b32 tSubMesh::fEqual( const tSubMesh& other ) const
	{
		if( mId != other.mId )
			return false;
		if( !mOctree.fEqual( other.mOctree ) )
			return false;
		return true;
	}

	b32 tMesh::fEqual( const tMesh* other ) const
	{
		if( mId != other->mId )
			return false;
		if( !mSubMesh.fEqual( other->mSubMesh ) )
			return false;
		return true;
	}

	b32 tObject::fEqual( const tObject* other ) const
	{
		if( mId != other->mId )
			return false;
		return true;
	}

	b32 tGeomObject::fEqual( const tObject* other ) const
	{
		if( mId != other->mId )
			return false;
		if( !static_cast<const tGeomObject*>( other )->mMesh->fEqual( mMesh ) )
			return false;
		if( !static_cast<const tGeomObject*>( other )->mSubMesh->fEqual( *mSubMesh ) )
			return false;
		return true;
	}

	tFile::tFile( )
	{
		mMeshes.fNewArray( 3 );
		for( u32 i = 0; i < mMeshes.fCount( ); ++i )
			mMeshes[ i ] = new tMesh( );

		mObjects.fNewArray( 10 );
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[ i ] = new tGeomObject( mMeshes[ i % mMeshes.fCount( ) ] );
	}
	tFile::tFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mObjects( cNoOpTag )
		, mMeshes( cNoOpTag )
		, mSubMeshes( cNoOpTag )
	{
	}
	tFile::~tFile( )
	{
		for( u32 i = 0; i < mMeshes.fCount( ); ++i )
			delete mMeshes[ i ];
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			delete mObjects[ i ];
	}
	b32 tFile::fEqual( const tFile& other ) const
	{
		if( mMeshes.fCount( ) != other.mMeshes.fCount( ) )
			return false;
		if( mObjects.fCount( ) != other.mObjects.fCount( ) )
			return false;

		for( u32 i = 0; i < mMeshes.fCount( ); ++i )
			if( !mMeshes[ i ]->fEqual( other.mMeshes[ i ] ) )
				return false;
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			if( !mObjects[ i ]->fEqual( other.mObjects[ i ] ) )
				return false;
		return true;
	}
}}

define_unittest( TestPolySoupOctreeSerializing )
{
	const tRandom savedRandom = tRandom::fSubjectiveRand( );
	tRandom::fSubjectiveRand( ) = tRandom( 1 );

	tFilePathPtr testPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePathPtr( "raycastoctree.bin" ) );

	tFile data;
	tDynamicBuffer dataBuffer;

	{
		tFileWriter o( testPath );
		tLoadInPlaceSerializer so;
		so.fSave( data, o, cCurrentPlatform );
	}

	{
		FileSystem::fReadFileToBuffer( dataBuffer, testPath );
		tLoadInPlaceDeserializer si;
		si.fLoad<tFile>( dataBuffer.fBegin( ) );
	}

	tFile& loadedData = *reinterpret_cast<tFile*>( dataBuffer.fBegin( ) );

	const b32 equal = data.fEqual( loadedData );
	fAssert( equal );

	tRandom::fSubjectiveRand( ) = savedRandom;
}
