#ifndef __TestSerializable_hpp__
#define __TestSerializable_hpp__
#include "tLoadInPlaceDeserializer.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "tSceneGraphFile.hpp"
#include "tPolySoupOctree.hpp"

namespace Sig
{

struct A;
struct B;
struct C;

struct C
{
	declare_reflector();

	C():i(0xc){}
	C( tNoOpTag ) { }
	int i;

};

struct B
{
	declare_reflector();

	B():i(0xb),pa(0){}
	B( tNoOpTag ) : c( cNoOpTag ){ }
	int i;
	C c;
	A* pa;

};


struct A
{
	declare_reflector();

	A():i(0xa),pa(0),pb(0),pc(0){}
	A( tNoOpTag ) : b( tNoOpTag() ) { }
	int i;
	A* pa;
	B* pb;
	C* pc;
	B b;

};

class iBase : public Rtti::tSerializableBaseClass
{
	declare_reflector();
	implement_rtti_serializable_base_class( iBase, 0xDD552913 );

public:

	iBase( ) : mVelocity(1.f) { }
	iBase( tNoOpTag ) { }
	virtual ~iBase( ) { }
	virtual f32 fGetVelocity( ) const { return mVelocity; }

	f32 mVelocity;
};

class iDerived0 : public iBase
{
	declare_reflector();
	implement_rtti_serializable_base_class( iDerived0, 0x8C0367BD );

public:

	iDerived0( ) : mVelocity1(2.f) { }
	iDerived0( tNoOpTag ) { }
	virtual ~iDerived0( ) { }
	virtual f32 fGetVelocity( ) const { return mVelocity + mVelocity1; }

	f32 mVelocity1;
};

register_rtti_factory( iDerived0, true )

class iDerived1 : public iBase
{
	declare_reflector();
	implement_rtti_serializable_base_class( iDerived1, 0x9BCA3807 );

public:

	iDerived1( ) : mVelocity1(3.f) { }
	iDerived1( tNoOpTag ) { }
	virtual ~iDerived1( ) { }
	virtual f32 fGetVelocity( ) const { return mVelocity + mVelocity1; }

	f32 mVelocity1;
};

register_rtti_factory( iDerived1, true )


namespace SerializeTest
{
	class tId
	{
		declare_reflector( );
	public:
		u32 mId;
		tId( )
		{
			static u32 gId = 0;
			mId = ++gId;
		}
		tId( tNoOpTag )
		{
		}
	};

	class tSubMesh : public tId
	{
		declare_reflector( );
	public:
		tPolySoupOctree mOctree;
		tSubMesh( );
		b32 fEqual( const tSubMesh& other ) const;
	};

	class tMesh : public tId
	{
		declare_reflector( );
	public:
		tSubMesh mSubMesh;
		b32 fEqual( const tMesh* other ) const;
	};

	class tObject : public Rtti::tSerializableBaseClass, public tId
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tObject, 0x5EB02AA1);
	public:
		tObject( ) { }
		tObject( tNoOpTag ) : tId( cNoOpTag ) { }
		virtual ~tObject( ) { }
		virtual b32 fEqual( const tObject* other ) const;
	};

	class tGeomObject : public tObject
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tGeomObject, 0xAEB65528);
	private:
		tMesh* mMesh;
		tSubMesh* mSubMesh;
	public:
		tGeomObject( tMesh* mesh ) : mMesh( mesh ), mSubMesh( &mesh->mSubMesh ) { }
		tGeomObject( ) : mMesh( 0 ), mSubMesh( 0 ) { }
		tGeomObject( tNoOpTag ) : tObject( cNoOpTag ) { }
		virtual ~tGeomObject( ) { }
		virtual b32 fEqual( const tObject* other ) const;
	};
	register_rtti_factory( tGeomObject, true )

	class tFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
	public:
		implement_rtti_serializable_base_class(tFile, 0x24DF3047);

		tDynamicArray< tLoadInPlacePtrWrapper<tObject> > mObjects;
		tDynamicArray< tLoadInPlacePtrWrapper<tMesh> > mMeshes;
		tDynamicArray< tLoadInPlacePtrWrapper<tSubMesh> > mSubMeshes;

		tFile( );
		tFile( tNoOpTag );
		~tFile( );
		b32 fEqual( const tFile& other ) const;
	};
	register_rtti_factory( tFile, true )
}

}

#endif//__TestSerializable_hpp__
