#include "UnitTestsPch.hpp"

//___________________________________________________________________________________________
// Generated from file [Src\Internal\UnitTests\TestSerializable.hpp]
//
#include "TestSerializable.hpp"
namespace Sig
{

//
// C
const ::Sig::Rtti::tBaseClassDesc C::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc C::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< C, int >( "C", "int", "i", "public", offsetof( C, i ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector C::gReflector = ::Sig::Rtti::fDefineReflector< C >( "C", C::gBases, C::gMembers );


//
// B
const ::Sig::Rtti::tBaseClassDesc B::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc B::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< B, int >( "B", "int", "i", "public", offsetof( B, i ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< B, C >( "B", "C", "c", "public", offsetof( B, c ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< B, A* >( "B", "A*", "pa", "public", offsetof( B, pa ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector B::gReflector = ::Sig::Rtti::fDefineReflector< B >( "B", B::gBases, B::gMembers );


//
// A
const ::Sig::Rtti::tBaseClassDesc A::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc A::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< A, int >( "A", "int", "i", "public", offsetof( A, i ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< A, A* >( "A", "A*", "pa", "public", offsetof( A, pa ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< A, B* >( "A", "B*", "pb", "public", offsetof( A, pb ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< A, C* >( "A", "C*", "pc", "public", offsetof( A, pc ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< A, B >( "A", "B", "b", "public", offsetof( A, b ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector A::gReflector = ::Sig::Rtti::fDefineReflector< A >( "A", A::gBases, A::gMembers );


//
// iBase
const ::Sig::Rtti::tBaseClassDesc iBase::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< iBase, Rtti::tSerializableBaseClass >( "iBase", "Rtti::tSerializableBaseClass", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc iBase::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< iBase, f32 >( "iBase", "f32", "mVelocity", "public", offsetof( iBase, mVelocity ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector iBase::gReflector = ::Sig::Rtti::fDefineReflector< iBase >( "iBase", iBase::gBases, iBase::gMembers );


//
// iDerived0
const ::Sig::Rtti::tBaseClassDesc iDerived0::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< iDerived0, iBase >( "iDerived0", "iBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc iDerived0::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< iDerived0, f32 >( "iDerived0", "f32", "mVelocity1", "public", offsetof( iDerived0, mVelocity1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector iDerived0::gReflector = ::Sig::Rtti::fDefineReflector< iDerived0 >( "iDerived0", iDerived0::gBases, iDerived0::gMembers );


//
// iDerived1
const ::Sig::Rtti::tBaseClassDesc iDerived1::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< iDerived1, iBase >( "iDerived1", "iBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc iDerived1::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< iDerived1, f32 >( "iDerived1", "f32", "mVelocity1", "public", offsetof( iDerived1, mVelocity1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector iDerived1::gReflector = ::Sig::Rtti::fDefineReflector< iDerived1 >( "iDerived1", iDerived1::gBases, iDerived1::gMembers );

namespace SerializeTest
{

//
// tId
const ::Sig::Rtti::tBaseClassDesc tId::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tId::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tId, u32 >( "tId", "u32", "mId", "public", offsetof( tId, mId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tId::gReflector = ::Sig::Rtti::fDefineReflector< tId >( "tId", tId::gBases, tId::gMembers );


//
// tSubMesh
const ::Sig::Rtti::tBaseClassDesc tSubMesh::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tSubMesh, tId >( "tSubMesh", "tId", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSubMesh::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, tPolySoupOctree >( "tSubMesh", "tPolySoupOctree", "mOctree", "public", offsetof( tSubMesh, mOctree ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSubMesh::gReflector = ::Sig::Rtti::fDefineReflector< tSubMesh >( "tSubMesh", tSubMesh::gBases, tSubMesh::gMembers );


//
// tMesh
const ::Sig::Rtti::tBaseClassDesc tMesh::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tMesh, tId >( "tMesh", "tId", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tMesh::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMesh, tSubMesh >( "tMesh", "tSubMesh", "mSubMesh", "public", offsetof( tMesh, mSubMesh ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tMesh::gReflector = ::Sig::Rtti::fDefineReflector< tMesh >( "tMesh", tMesh::gBases, tMesh::gMembers );


//
// tObject
const ::Sig::Rtti::tBaseClassDesc tObject::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tObject, Rtti::tSerializableBaseClass >( "tObject", "Rtti::tSerializableBaseClass", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc< tObject, tId >( "tObject", "tId", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tObject::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tObject::gReflector = ::Sig::Rtti::fDefineReflector< tObject >( "tObject", tObject::gBases, tObject::gMembers );


//
// tGeomObject
const ::Sig::Rtti::tBaseClassDesc tGeomObject::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tGeomObject, tObject >( "tGeomObject", "tObject", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tGeomObject::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tGeomObject, tMesh* >( "tGeomObject", "tMesh*", "mMesh", "public", offsetof( tGeomObject, mMesh ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeomObject, tSubMesh* >( "tGeomObject", "tSubMesh*", "mSubMesh", "public", offsetof( tGeomObject, mSubMesh ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tGeomObject::gReflector = ::Sig::Rtti::fDefineReflector< tGeomObject >( "tGeomObject", tGeomObject::gBases, tGeomObject::gMembers );


//
// tFile
const ::Sig::Rtti::tBaseClassDesc tFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tFile, tLoadInPlaceFileBase >( "tFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFile, tDynamicArray<tLoadInPlacePtrWrapper<tObject> > >( "tFile", "tDynamicArray<tLoadInPlacePtrWrapper<tObject> >", "mObjects", "public", offsetof( tFile, mObjects ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFile, tDynamicArray<tLoadInPlacePtrWrapper<tMesh> > >( "tFile", "tDynamicArray<tLoadInPlacePtrWrapper<tMesh> >", "mMeshes", "public", offsetof( tFile, mMeshes ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFile, tDynamicArray<tLoadInPlacePtrWrapper<tSubMesh> > >( "tFile", "tDynamicArray<tLoadInPlacePtrWrapper<tSubMesh> >", "mSubMeshes", "public", offsetof( tFile, mSubMeshes ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFile::gReflector = ::Sig::Rtti::fDefineReflector< tFile >( "tFile", tFile::gBases, tFile::gMembers );

}
}

