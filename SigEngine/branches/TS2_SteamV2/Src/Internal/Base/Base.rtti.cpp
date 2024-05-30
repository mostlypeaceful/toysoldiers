#include "BasePch.hpp"

//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\t3dGridEntity.hpp]
//
#include "t3dGridEntity.hpp"
namespace Sig
{

//
// t3dGridEntityDef
const ::Sig::Rtti::tBaseClassDesc t3dGridEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< t3dGridEntityDef, tEntityDef >( "t3dGridEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc t3dGridEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< t3dGridEntityDef, Math::tVec3u >( "t3dGridEntityDef", "Math::tVec3u", "mCellCounts", "public", offsetof( t3dGridEntityDef, mCellCounts ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector t3dGridEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< t3dGridEntityDef >( "t3dGridEntityDef", t3dGridEntityDef::gBases, t3dGridEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tAnimPackFile.hpp]
//
#include "tAnimPackFile.hpp"
namespace Sig
{

//
// tAnimPackFile
const ::Sig::Rtti::tBaseClassDesc tAnimPackFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tAnimPackFile, tLoadInPlaceFileBase >( "tAnimPackFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tAnimPackFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tAnimPackFile, tLoadInPlaceResourcePtr* >( "tAnimPackFile", "tLoadInPlaceResourcePtr*", "mSkeletonResource", "public", offsetof( tAnimPackFile, mSkeletonResource ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tAnimPackFile, tAnimList >( "tAnimPackFile", "tAnimList", "mAnims", "public", offsetof( tAnimPackFile, mAnims ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tAnimPackFile, tAnimMapStorage >( "tAnimPackFile", "tAnimMapStorage", "mAnimsByName", "public", offsetof( tAnimPackFile, mAnimsByName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tAnimPackFile::gReflector = ::Sig::Rtti::fDefineReflector< tAnimPackFile >( "tAnimPackFile", tAnimPackFile::gBases, tAnimPackFile::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tAttachmentEntity.hpp]
//
#include "tAttachmentEntity.hpp"
namespace Sig
{

//
// tAttachmentEntityDef
const ::Sig::Rtti::tBaseClassDesc tAttachmentEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tAttachmentEntityDef, tEntityDef >( "tAttachmentEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tAttachmentEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tAttachmentEntityDef, u16 >( "tAttachmentEntityDef", "u16", "mStateMask", "public", offsetof( tAttachmentEntityDef, mStateMask ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tAttachmentEntityDef, u16 >( "tAttachmentEntityDef", "u16", "pad0", "public", offsetof( tAttachmentEntityDef, pad0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tAttachmentEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tAttachmentEntityDef >( "tAttachmentEntityDef", tAttachmentEntityDef::gBases, tAttachmentEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tBinaryFileBase.hpp]
//
#include "tBinaryFileBase.hpp"
namespace Sig
{

//
// tBinaryFileBase
const ::Sig::Rtti::tBaseClassDesc tBinaryFileBase::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tBinaryFileBase, Rtti::tSerializableBaseClass >( "tBinaryFileBase", "Rtti::tSerializableBaseClass", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryFileBase::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryFileBase, tSignature >( "tBinaryFileBase", "tSignature", "mSignature", "public", offsetof( tBinaryFileBase, mSignature ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryFileBase, Rtti::tClassId >( "tBinaryFileBase", "Rtti::tClassId", "mFileType", "public", offsetof( tBinaryFileBase, mFileType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryFileBase, u32 >( "tBinaryFileBase", "u32", "mVersion", "public", offsetof( tBinaryFileBase, mVersion ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryFileBase::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryFileBase >( "tBinaryFileBase", tBinaryFileBase::gBases, tBinaryFileBase::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tByteFile.hpp]
//
#include "tByteFile.hpp"
namespace Sig
{

//
// tByteFile
const ::Sig::Rtti::tBaseClassDesc tByteFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tByteFile, tLoadInPlaceFileBase >( "tByteFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tByteFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tByteFile, tDynamicBuffer >( "tByteFile", "tDynamicBuffer", "mData", "public", offsetof( tByteFile, mData ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tByteFile::gReflector = ::Sig::Rtti::fDefineReflector< tByteFile >( "tByteFile", tByteFile::gBases, tByteFile::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tDataTableFile.hpp]
//
#include "tDataTableFile.hpp"
namespace Sig
{

//
// tDataTableCellArray
const ::Sig::Rtti::tBaseClassDesc tDataTableCellArray::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDataTableCellArray, Rtti::tSerializableBaseClass >( "tDataTableCellArray", "Rtti::tSerializableBaseClass", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDataTableCellArray::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDataTableCellArray::gReflector = ::Sig::Rtti::fDefineReflector< tDataTableCellArray >( "tDataTableCellArray", tDataTableCellArray::gBases, tDataTableCellArray::gMembers );


//
// tDataTable
const ::Sig::Rtti::tBaseClassDesc tDataTable::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDataTable::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tDataTable, tLoadInPlaceStringPtr* >( "tDataTable", "tLoadInPlaceStringPtr*", "mName", "public", offsetof( tDataTable, mName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDataTable, b32 >( "tDataTable", "b32", "mByColumn", "public", offsetof( tDataTable, mByColumn ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDataTable, tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> > >( "tDataTable", "tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> >", "mColNames", "public", offsetof( tDataTable, mColNames ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDataTable, tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> > >( "tDataTable", "tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> >", "mRowNames", "public", offsetof( tDataTable, mRowNames ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDataTable, tDynamicArray<tLoadInPlacePtrWrapper<tDataTableCellArray> > >( "tDataTable", "tDynamicArray<tLoadInPlacePtrWrapper<tDataTableCellArray> >", "mCellArrays", "public", offsetof( tDataTable, mCellArrays ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDataTable::gReflector = ::Sig::Rtti::fDefineReflector< tDataTable >( "tDataTable", tDataTable::gBases, tDataTable::gMembers );


//
// tDataTableFile
const ::Sig::Rtti::tBaseClassDesc tDataTableFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDataTableFile, tLoadInPlaceFileBase >( "tDataTableFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDataTableFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tDataTableFile, tDynamicArray<tLoadInPlacePtrWrapper<tDataTable> > >( "tDataTableFile", "tDynamicArray<tLoadInPlacePtrWrapper<tDataTable> >", "mTables", "public", offsetof( tDataTableFile, mTables ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDataTableFile::gReflector = ::Sig::Rtti::fDefineReflector< tDataTableFile >( "tDataTableFile", tDataTableFile::gBases, tDataTableFile::gMembers );


//
// tDataTableCellArrayNumeric
const ::Sig::Rtti::tBaseClassDesc tDataTableCellArrayNumeric::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDataTableCellArrayNumeric, tDataTableCellArray >( "tDataTableCellArrayNumeric", "tDataTableCellArray", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDataTableCellArrayNumeric::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tDataTableCellArrayNumeric, tDynamicArray<f32> >( "tDataTableCellArrayNumeric", "tDynamicArray<f32>", "mValues", "public", offsetof( tDataTableCellArrayNumeric, mValues ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDataTableCellArrayNumeric::gReflector = ::Sig::Rtti::fDefineReflector< tDataTableCellArrayNumeric >( "tDataTableCellArrayNumeric", tDataTableCellArrayNumeric::gBases, tDataTableCellArrayNumeric::gMembers );


//
// tDataTableCellArrayStringPtr
const ::Sig::Rtti::tBaseClassDesc tDataTableCellArrayStringPtr::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDataTableCellArrayStringPtr, tDataTableCellArray >( "tDataTableCellArrayStringPtr", "tDataTableCellArray", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDataTableCellArrayStringPtr::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tDataTableCellArrayStringPtr, tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> > >( "tDataTableCellArrayStringPtr", "tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> >", "mValues", "public", offsetof( tDataTableCellArrayStringPtr, mValues ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDataTableCellArrayStringPtr::gReflector = ::Sig::Rtti::fDefineReflector< tDataTableCellArrayStringPtr >( "tDataTableCellArrayStringPtr", tDataTableCellArrayStringPtr::gBases, tDataTableCellArrayStringPtr::gMembers );


//
// tDataTableCellArrayUnicodeString
const ::Sig::Rtti::tBaseClassDesc tDataTableCellArrayUnicodeString::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDataTableCellArrayUnicodeString, tDataTableCellArray >( "tDataTableCellArrayUnicodeString", "tDataTableCellArray", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDataTableCellArrayUnicodeString::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tDataTableCellArrayUnicodeString, tDynamicArray<tLocalizedString> >( "tDataTableCellArrayUnicodeString", "tDynamicArray<tLocalizedString>", "mValues", "public", offsetof( tDataTableCellArrayUnicodeString, mValues ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDataTableCellArrayUnicodeString::gReflector = ::Sig::Rtti::fDefineReflector< tDataTableCellArrayUnicodeString >( "tDataTableCellArrayUnicodeString", tDataTableCellArrayUnicodeString::gBases, tDataTableCellArrayUnicodeString::gMembers );


//
// tDataTableCellArrayVector4
const ::Sig::Rtti::tBaseClassDesc tDataTableCellArrayVector4::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDataTableCellArrayVector4, tDataTableCellArray >( "tDataTableCellArrayVector4", "tDataTableCellArray", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDataTableCellArrayVector4::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tDataTableCellArrayVector4, tDynamicArray<Math::tVec4f> >( "tDataTableCellArrayVector4", "tDynamicArray<Math::tVec4f>", "mValues", "public", offsetof( tDataTableCellArrayVector4, mValues ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDataTableCellArrayVector4::gReflector = ::Sig::Rtti::fDefineReflector< tDataTableCellArrayVector4 >( "tDataTableCellArrayVector4", tDataTableCellArrayVector4::gBases, tDataTableCellArrayVector4::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tEntityDef.hpp]
//
#include "tEntityDef.hpp"
namespace Sig
{

//
// tEntityCreationFlags
const ::Sig::Rtti::tBaseClassDesc tEntityCreationFlags::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tEntityCreationFlags::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tEntityCreationFlags, u16 >( "tEntityCreationFlags", "u16", "mRenderFlags", "public", offsetof( tEntityCreationFlags, mRenderFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityCreationFlags, u16 >( "tEntityCreationFlags", "u16", "mCreateFlags", "public", offsetof( tEntityCreationFlags, mCreateFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tEntityCreationFlags::gReflector = ::Sig::Rtti::fDefineReflector< tEntityCreationFlags >( "tEntityCreationFlags", tEntityCreationFlags::gBases, tEntityCreationFlags::gMembers );


//
// tEntityDefProperties::tEnumProperty
const ::Sig::Rtti::tBaseClassDesc tEntityDefProperties::tEnumProperty::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tEntityDefProperties::tEnumProperty::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties::tEnumProperty, u32 >( "tEntityDefProperties::tEnumProperty", "u32", "mEnumKey", "public", offsetof( tEntityDefProperties::tEnumProperty, mEnumKey ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties::tEnumProperty, u32 >( "tEntityDefProperties::tEnumProperty", "u32", "mEnumValue", "public", offsetof( tEntityDefProperties::tEnumProperty, mEnumValue ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tEntityDefProperties::tEnumProperty::gReflector = ::Sig::Rtti::fDefineReflector< tEntityDefProperties::tEnumProperty >( "tEntityDefProperties::tEnumProperty", tEntityDefProperties::tEnumProperty::gBases, tEntityDefProperties::tEnumProperty::gMembers );


//
// tEntityDefProperties
const ::Sig::Rtti::tBaseClassDesc tEntityDefProperties::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tEntityDefProperties::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties, tLoadInPlaceStringPtr* >( "tEntityDefProperties", "tLoadInPlaceStringPtr*", "mName", "public", offsetof( tEntityDefProperties, mName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties, tLoadInPlaceResourcePtr* >( "tEntityDefProperties", "tLoadInPlaceResourcePtr*", "mScriptFile", "public", offsetof( tEntityDefProperties, mScriptFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties, tLoadInPlaceStringPtr* >( "tEntityDefProperties", "tLoadInPlaceStringPtr*", "mOnEntityCreateOverride", "public", offsetof( tEntityDefProperties, mOnEntityCreateOverride ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties, tLoadInPlaceResourcePtr* >( "tEntityDefProperties", "tLoadInPlaceResourcePtr*", "mSkeletonFile", "public", offsetof( tEntityDefProperties, mSkeletonFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties, tLoadInPlaceStringPtr* >( "tEntityDefProperties", "tLoadInPlaceStringPtr*", "mBoneAttachment", "public", offsetof( tEntityDefProperties, mBoneAttachment ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties, tEntityCreationFlags >( "tEntityDefProperties", "tEntityCreationFlags", "mCreationFlags", "public", offsetof( tEntityDefProperties, mCreationFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties, tTagPropertyList >( "tEntityDefProperties", "tTagPropertyList", "mTagProperties", "public", offsetof( tEntityDefProperties, mTagProperties ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDefProperties, tEnumPropertyList >( "tEntityDefProperties", "tEnumPropertyList", "mEnumProperties", "public", offsetof( tEntityDefProperties, mEnumProperties ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tEntityDefProperties::gReflector = ::Sig::Rtti::fDefineReflector< tEntityDefProperties >( "tEntityDefProperties", tEntityDefProperties::gBases, tEntityDefProperties::gMembers );


//
// tEntityDef
const ::Sig::Rtti::tBaseClassDesc tEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tEntityDef, Rtti::tSerializableBaseClass >( "tEntityDef", "Rtti::tSerializableBaseClass", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc< tEntityDef, tEntityDefProperties >( "tEntityDef", "tEntityDefProperties", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDef, Math::tAabbf >( "tEntityDef", "Math::tAabbf", "mBounds", "public", offsetof( tEntityDef, mBounds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDef, Math::tMat3f >( "tEntityDef", "Math::tMat3f", "mObjectToLocal", "public", offsetof( tEntityDef, mObjectToLocal ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEntityDef, Math::tMat3f >( "tEntityDef", "Math::tMat3f", "mLocalToObject", "public", offsetof( tEntityDef, mLocalToObject ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tEntityDef >( "tEntityDef", tEntityDef::gBases, tEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tFilePackageFile.hpp]
//
#include "tFilePackageFile.hpp"
namespace Sig
{

//
// tFilePackageFile::tFileHeader
const ::Sig::Rtti::tBaseClassDesc tFilePackageFile::tFileHeader::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFilePackageFile::tFileHeader::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile::tFileHeader, u64 >( "tFilePackageFile::tFileHeader", "u64", "mLastModifiedTimeStamp", "public", offsetof( tFilePackageFile::tFileHeader, mLastModifiedTimeStamp ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile::tFileHeader, tDynamicArray<char> >( "tFilePackageFile::tFileHeader", "tDynamicArray<char>", "mFileName", "public", offsetof( tFilePackageFile::tFileHeader, mFileName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile::tFileHeader, Rtti::tClassId >( "tFilePackageFile::tFileHeader", "Rtti::tClassId", "mClassId", "public", offsetof( tFilePackageFile::tFileHeader, mClassId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile::tFileHeader, u32 >( "tFilePackageFile::tFileHeader", "u32", "mFlags", "public", offsetof( tFilePackageFile::tFileHeader, mFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile::tFileHeader, u32 >( "tFilePackageFile::tFileHeader", "u32", "mRawFileOffset", "public", offsetof( tFilePackageFile::tFileHeader, mRawFileOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile::tFileHeader, u32 >( "tFilePackageFile::tFileHeader", "u32", "mNumRawFileBytes", "public", offsetof( tFilePackageFile::tFileHeader, mNumRawFileBytes ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile::tFileHeader, u32 >( "tFilePackageFile::tFileHeader", "u32", "mNumRawFileBytesUncompressed", "public", offsetof( tFilePackageFile::tFileHeader, mNumRawFileBytesUncompressed ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile::tFileHeader, u32 >( "tFilePackageFile::tFileHeader", "u32", "reservedu32", "public", offsetof( tFilePackageFile::tFileHeader, reservedu32 ), 4, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFilePackageFile::tFileHeader::gReflector = ::Sig::Rtti::fDefineReflector< tFilePackageFile::tFileHeader >( "tFilePackageFile::tFileHeader", tFilePackageFile::tFileHeader::gBases, tFilePackageFile::tFileHeader::gMembers );


//
// tFilePackageFile
const ::Sig::Rtti::tBaseClassDesc tFilePackageFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tFilePackageFile, tLoadInPlaceFileBase >( "tFilePackageFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFilePackageFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile, u32 >( "tFilePackageFile", "u32", "mHeaderTableSize", "public", offsetof( tFilePackageFile, mHeaderTableSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFilePackageFile, tDynamicArray<tFileHeader> >( "tFilePackageFile", "tDynamicArray<tFileHeader>", "mFileHeaders", "public", offsetof( tFilePackageFile, mFileHeaders ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFilePackageFile::gReflector = ::Sig::Rtti::fDefineReflector< tFilePackageFile >( "tFilePackageFile", tFilePackageFile::gBases, tFilePackageFile::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tFxFileRefEntity.hpp]
//
#include "tFxFileRefEntity.hpp"
namespace Sig
{

//
// tEffectRefEntityDef
const ::Sig::Rtti::tBaseClassDesc tEffectRefEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tEffectRefEntityDef, tEntityDef >( "tEffectRefEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tEffectRefEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tEffectRefEntityDef, tLoadInPlaceResourcePtr* >( "tEffectRefEntityDef", "tLoadInPlaceResourcePtr*", "mReferenceFile", "public", offsetof( tEffectRefEntityDef, mReferenceFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEffectRefEntityDef, tSceneLODSettings* >( "tEffectRefEntityDef", "tSceneLODSettings*", "mLODSettings", "public", offsetof( tEffectRefEntityDef, mLODSettings ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEffectRefEntityDef, u32 >( "tEffectRefEntityDef", "u32", "mStartupFlags", "public", offsetof( tEffectRefEntityDef, mStartupFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEffectRefEntityDef, s32 >( "tEffectRefEntityDef", "s32", "mLoopCount", "public", offsetof( tEffectRefEntityDef, mLoopCount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEffectRefEntityDef, f32 >( "tEffectRefEntityDef", "f32", "mPreLoadTime", "public", offsetof( tEffectRefEntityDef, mPreLoadTime ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEffectRefEntityDef, u16 >( "tEffectRefEntityDef", "u16", "mStateMask", "public", offsetof( tEffectRefEntityDef, mStateMask ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tEffectRefEntityDef, u16 >( "tEffectRefEntityDef", "u16", "mSurfaceOrientation", "public", offsetof( tEffectRefEntityDef, mSurfaceOrientation ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tEffectRefEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tEffectRefEntityDef >( "tEffectRefEntityDef", tEffectRefEntityDef::gBases, tEffectRefEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tHeightFieldMesh.hpp]
//
#include "tHeightFieldMesh.hpp"
namespace Sig
{

//
// tHeightFieldRez
const ::Sig::Rtti::tBaseClassDesc tHeightFieldRez::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldRez::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, f32 >( "tHeightFieldRez", "f32", "mWorldSpaceLengthX", "public", offsetof( tHeightFieldRez, mWorldSpaceLengthX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, f32 >( "tHeightFieldRez", "f32", "mWorldSpaceLengthZ", "public", offsetof( tHeightFieldRez, mWorldSpaceLengthZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, u32 >( "tHeightFieldRez", "u32", "mVertexResX", "public", offsetof( tHeightFieldRez, mVertexResX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, u32 >( "tHeightFieldRez", "u32", "mVertexResZ", "public", offsetof( tHeightFieldRez, mVertexResZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, u32 >( "tHeightFieldRez", "u32", "mLogicalVertResX", "public", offsetof( tHeightFieldRez, mLogicalVertResX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, u32 >( "tHeightFieldRez", "u32", "mLogicalVertResZ", "public", offsetof( tHeightFieldRez, mLogicalVertResZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, u32 >( "tHeightFieldRez", "u32", "mChunkQuadResX", "public", offsetof( tHeightFieldRez, mChunkQuadResX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, u32 >( "tHeightFieldRez", "u32", "mChunkQuadResZ", "public", offsetof( tHeightFieldRez, mChunkQuadResZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, u32 >( "tHeightFieldRez", "u32", "mNumChunksX", "public", offsetof( tHeightFieldRez, mNumChunksX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldRez, u32 >( "tHeightFieldRez", "u32", "mNumChunksZ", "public", offsetof( tHeightFieldRez, mNumChunksZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldRez::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldRez >( "tHeightFieldRez", tHeightFieldRez::gBases, tHeightFieldRez::gMembers );


//
// tHeightFieldMesh::tLogicalVertex
const ::Sig::Rtti::tBaseClassDesc tHeightFieldMesh::tLogicalVertex::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldMesh::tLogicalVertex::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh::tLogicalVertex, u32 >( "tHeightFieldMesh::tLogicalVertex", "u32", "mBaseRenderVertexId", "public", offsetof( tHeightFieldMesh::tLogicalVertex, mBaseRenderVertexId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh::tLogicalVertex, u32 >( "tHeightFieldMesh::tLogicalVertex", "u32", "mBaseRenderIndexId", "public", offsetof( tHeightFieldMesh::tLogicalVertex, mBaseRenderIndexId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh::tLogicalVertex, tFixedArray<u16,4> >( "tHeightFieldMesh::tLogicalVertex", "tFixedArray<u16,4>", "mRenderVertexIds", "public", offsetof( tHeightFieldMesh::tLogicalVertex, mRenderVertexIds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh::tLogicalVertex, f32 >( "tHeightFieldMesh::tLogicalVertex", "f32", "mHeight", "public", offsetof( tHeightFieldMesh::tLogicalVertex, mHeight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh::tLogicalVertex, tEnum<tQuadState,u8> >( "tHeightFieldMesh::tLogicalVertex", "tEnum<tQuadState,u8>", "mQuadState", "public", offsetof( tHeightFieldMesh::tLogicalVertex, mQuadState ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh::tLogicalVertex, u8 >( "tHeightFieldMesh::tLogicalVertex", "u8", "mNumRenderVertexIds", "public", offsetof( tHeightFieldMesh::tLogicalVertex, mNumRenderVertexIds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh::tLogicalVertex, u8 >( "tHeightFieldMesh::tLogicalVertex", "u8", "pad0", "public", offsetof( tHeightFieldMesh::tLogicalVertex, pad0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh::tLogicalVertex, u8 >( "tHeightFieldMesh::tLogicalVertex", "u8", "pad1", "public", offsetof( tHeightFieldMesh::tLogicalVertex, pad1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldMesh::tLogicalVertex::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldMesh::tLogicalVertex >( "tHeightFieldMesh::tLogicalVertex", tHeightFieldMesh::tLogicalVertex::gBases, tHeightFieldMesh::tLogicalVertex::gMembers );


//
// tHeightFieldMesh
const ::Sig::Rtti::tBaseClassDesc tHeightFieldMesh::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tHeightFieldMesh, tEntityDef >( "tHeightFieldMesh", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldMesh::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh, tHeightFieldRez >( "tHeightFieldMesh", "tHeightFieldRez", "mRez", "public", offsetof( tHeightFieldMesh, mRez ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh, tLogicalVertexArray >( "tHeightFieldMesh", "tLogicalVertexArray", "mLogicalVerts", "public", offsetof( tHeightFieldMesh, mLogicalVerts ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMesh, tHeightFieldQuadTree >( "tHeightFieldMesh", "tHeightFieldQuadTree", "mQuadTree", "public", offsetof( tHeightFieldMesh, mQuadTree ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldMesh::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldMesh >( "tHeightFieldMesh", tHeightFieldMesh::gBases, tHeightFieldMesh::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tHeightFieldMeshEntity.hpp]
//
#include "tHeightFieldMeshEntity.hpp"
namespace Sig
{

//
// tHeightFieldMeshEntityDef::tChunkDesc
const ::Sig::Rtti::tBaseClassDesc tHeightFieldMeshEntityDef::tChunkDesc::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldMeshEntityDef::tChunkDesc::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMeshEntityDef::tChunkDesc, u32 >( "tHeightFieldMeshEntityDef::tChunkDesc", "u32", "mTriCount", "public", offsetof( tHeightFieldMeshEntityDef::tChunkDesc, mTriCount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMeshEntityDef::tChunkDesc, u32 >( "tHeightFieldMeshEntityDef::tChunkDesc", "u32", "mVtxCount", "public", offsetof( tHeightFieldMeshEntityDef::tChunkDesc, mVtxCount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldMeshEntityDef::tChunkDesc::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldMeshEntityDef::tChunkDesc >( "tHeightFieldMeshEntityDef::tChunkDesc", tHeightFieldMeshEntityDef::tChunkDesc::gBases, tHeightFieldMeshEntityDef::tChunkDesc::gMembers );


//
// tHeightFieldMeshEntityDef
const ::Sig::Rtti::tBaseClassDesc tHeightFieldMeshEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tHeightFieldMeshEntityDef, tHeightFieldMesh >( "tHeightFieldMeshEntityDef", "tHeightFieldMesh", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldMeshEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMeshEntityDef, tChunkDescList >( "tHeightFieldMeshEntityDef", "tChunkDescList", "mChunkDescs", "public", offsetof( tHeightFieldMeshEntityDef, mChunkDescs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMeshEntityDef, Gfx::tGroundCoverCloudDefList >( "tHeightFieldMeshEntityDef", "Gfx::tGroundCoverCloudDefList", "mGroundCoverDefs", "public", offsetof( tHeightFieldMeshEntityDef, mGroundCoverDefs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMeshEntityDef, tLoadInPlaceResourcePtr* >( "tHeightFieldMeshEntityDef", "tLoadInPlaceResourcePtr*", "mGeometryFile", "public", offsetof( tHeightFieldMeshEntityDef, mGeometryFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMeshEntityDef, Gfx::tMaterial* >( "tHeightFieldMeshEntityDef", "Gfx::tMaterial*", "mMaterial", "public", offsetof( tHeightFieldMeshEntityDef, mMaterial ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldMeshEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldMeshEntityDef >( "tHeightFieldMeshEntityDef", tHeightFieldMeshEntityDef::gBases, tHeightFieldMeshEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tHeightFieldQuadTree.hpp]
//
#include "tHeightFieldQuadTree.hpp"
namespace Sig
{

//
// tHeightFieldQuadTree
const ::Sig::Rtti::tBaseClassDesc tHeightFieldQuadTree::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldQuadTree::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldQuadTree, Math::tVec2f >( "tHeightFieldQuadTree", "Math::tVec2f", "mMinMaxHeight", "public", offsetof( tHeightFieldQuadTree, mMinMaxHeight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldQuadTree, Math::tVec2u >( "tHeightFieldQuadTree", "Math::tVec2u", "mMinMaxLogicalX", "public", offsetof( tHeightFieldQuadTree, mMinMaxLogicalX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldQuadTree, Math::tVec2u >( "tHeightFieldQuadTree", "Math::tVec2u", "mMinMaxLogicalZ", "public", offsetof( tHeightFieldQuadTree, mMinMaxLogicalZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldQuadTree, tCellArray >( "tHeightFieldQuadTree", "tCellArray", "mChildren", "public", offsetof( tHeightFieldQuadTree, mChildren ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldQuadTree::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldQuadTree >( "tHeightFieldQuadTree", tHeightFieldQuadTree::gBases, tHeightFieldQuadTree::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tKdTree.hpp]
//
#include "tKdTree.hpp"
namespace Sig
{

//
// tKdNode
const ::Sig::Rtti::tBaseClassDesc tKdNode::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tKdNode::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tKdNode, tFixedArray<tLoadInPlacePtrWrapper<tKdNode>,2> >( "tKdNode", "tFixedArray<tLoadInPlacePtrWrapper<tKdNode>,2>", "mChildren", "public", offsetof( tKdNode, mChildren ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKdNode, tEnum<tAxis,u32> >( "tKdNode", "tEnum<tAxis,u32>", "mAxis", "public", offsetof( tKdNode, mAxis ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKdNode, f32 >( "tKdNode", "f32", "mSplitPosition", "public", offsetof( tKdNode, mSplitPosition ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKdNode, tDynamicArray<u32> >( "tKdNode", "tDynamicArray<u32>", "mItems", "public", offsetof( tKdNode, mItems ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKdNode, tDynamicArray<Math::tAabbf> >( "tKdNode", "tDynamicArray<Math::tAabbf>", "mItemsBounds", "public", offsetof( tKdNode, mItemsBounds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tKdNode::gReflector = ::Sig::Rtti::fDefineReflector< tKdNode >( "tKdNode", tKdNode::gBases, tKdNode::gMembers );


//
// tKdTree
const ::Sig::Rtti::tBaseClassDesc tKdTree::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tKdTree::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tKdTree, Math::tAabbf >( "tKdTree", "Math::tAabbf", "mBounds", "public", offsetof( tKdTree, mBounds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKdTree, tKdNode >( "tKdTree", "tKdNode", "mFirstSplit", "public", offsetof( tKdTree, mFirstSplit ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tKdTree::gReflector = ::Sig::Rtti::fDefineReflector< tKdTree >( "tKdTree", tKdTree::gBases, tKdTree::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tKeyFrameAnimation.hpp]
//
#include "tKeyFrameAnimation.hpp"
namespace Sig
{

//
// tKeyFrameAnimation::tBone
const ::Sig::Rtti::tBaseClassDesc tKeyFrameAnimation::tBone::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tKeyFrameAnimation::tBone::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, u16 >( "tKeyFrameAnimation::tBone", "u16", "mMasterBoneIndex", "public", offsetof( tKeyFrameAnimation::tBone, mMasterBoneIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, u16 >( "tKeyFrameAnimation::tBone", "u16", "mFlags", "public", offsetof( tKeyFrameAnimation::tBone, mFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, tLoadInPlaceStringPtr* >( "tKeyFrameAnimation::tBone", "tLoadInPlaceStringPtr*", "mName", "public", offsetof( tKeyFrameAnimation::tBone, mName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, f32 >( "tKeyFrameAnimation::tBone", "f32", "mPMin", "public", offsetof( tKeyFrameAnimation::tBone, mPMin ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, f32 >( "tKeyFrameAnimation::tBone", "f32", "mPMax", "public", offsetof( tKeyFrameAnimation::tBone, mPMax ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, f32 >( "tKeyFrameAnimation::tBone", "f32", "mSMin", "public", offsetof( tKeyFrameAnimation::tBone, mSMin ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, f32 >( "tKeyFrameAnimation::tBone", "f32", "mSMax", "public", offsetof( tKeyFrameAnimation::tBone, mSMax ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, tKeyFrameNumberArray >( "tKeyFrameAnimation::tBone", "tKeyFrameNumberArray", "mPositionFrameNums", "public", offsetof( tKeyFrameAnimation::tBone, mPositionFrameNums ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, tPositionKeyArray >( "tKeyFrameAnimation::tBone", "tPositionKeyArray", "mPositionKeys", "public", offsetof( tKeyFrameAnimation::tBone, mPositionKeys ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, tKeyFrameNumberArray >( "tKeyFrameAnimation::tBone", "tKeyFrameNumberArray", "mRotationFrameNums", "public", offsetof( tKeyFrameAnimation::tBone, mRotationFrameNums ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, tRotationKeyArray >( "tKeyFrameAnimation::tBone", "tRotationKeyArray", "mRotationKeys", "public", offsetof( tKeyFrameAnimation::tBone, mRotationKeys ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, tKeyFrameNumberArray >( "tKeyFrameAnimation::tBone", "tKeyFrameNumberArray", "mScaleFrameNums", "public", offsetof( tKeyFrameAnimation::tBone, mScaleFrameNums ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, tScaleKeyArray >( "tKeyFrameAnimation::tBone", "tScaleKeyArray", "mScaleKeys", "public", offsetof( tKeyFrameAnimation::tBone, mScaleKeys ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, u16 >( "tKeyFrameAnimation::tBone", "u16", "mParentMasterBoneIndex", "public", offsetof( tKeyFrameAnimation::tBone, mParentMasterBoneIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, u16 >( "tKeyFrameAnimation::tBone", "u16", "mIKPriority", "public", offsetof( tKeyFrameAnimation::tBone, mIKPriority ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, Math::tAabbf >( "tKeyFrameAnimation::tBone", "Math::tAabbf", "mIKAxisLimits", "public", offsetof( tKeyFrameAnimation::tBone, mIKAxisLimits ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBone, u32 >( "tKeyFrameAnimation::tBone", "u32", "mIKAxisLimitsOrder", "public", offsetof( tKeyFrameAnimation::tBone, mIKAxisLimitsOrder ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tKeyFrameAnimation::tBone::gReflector = ::Sig::Rtti::fDefineReflector< tKeyFrameAnimation::tBone >( "tKeyFrameAnimation::tBone", tKeyFrameAnimation::tBone::gBases, tKeyFrameAnimation::tBone::gMembers );


//
// tKeyFrameAnimation::tKeyFrameEvent
const ::Sig::Rtti::tBaseClassDesc tKeyFrameAnimation::tKeyFrameEvent::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tKeyFrameAnimation::tKeyFrameEvent::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tKeyFrameEvent, f32 >( "tKeyFrameAnimation::tKeyFrameEvent", "f32", "mTime", "public", offsetof( tKeyFrameAnimation::tKeyFrameEvent, mTime ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tKeyFrameEvent, u32 >( "tKeyFrameAnimation::tKeyFrameEvent", "u32", "mEventTypeCppValue", "public", offsetof( tKeyFrameAnimation::tKeyFrameEvent, mEventTypeCppValue ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tKeyFrameEvent, tLoadInPlaceStringPtr* >( "tKeyFrameAnimation::tKeyFrameEvent", "tLoadInPlaceStringPtr*", "mTag", "public", offsetof( tKeyFrameAnimation::tKeyFrameEvent, mTag ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tKeyFrameAnimation::tKeyFrameEvent::gReflector = ::Sig::Rtti::fDefineReflector< tKeyFrameAnimation::tKeyFrameEvent >( "tKeyFrameAnimation::tKeyFrameEvent", tKeyFrameAnimation::tKeyFrameEvent::gBases, tKeyFrameAnimation::tKeyFrameEvent::gMembers );


//
// tKeyFrameAnimation::tBracket
const ::Sig::Rtti::tBaseClassDesc tKeyFrameAnimation::tBracket::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tKeyFrameAnimation::tBracket::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBracket, u32 >( "tKeyFrameAnimation::tBracket", "u32", "mLeft", "public", offsetof( tKeyFrameAnimation::tBracket, mLeft ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBracket, u32 >( "tKeyFrameAnimation::tBracket", "u32", "mRight", "public", offsetof( tKeyFrameAnimation::tBracket, mRight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation::tBracket, f32 >( "tKeyFrameAnimation::tBracket", "f32", "mZeroToOne", "public", offsetof( tKeyFrameAnimation::tBracket, mZeroToOne ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tKeyFrameAnimation::tBracket::gReflector = ::Sig::Rtti::fDefineReflector< tKeyFrameAnimation::tBracket >( "tKeyFrameAnimation::tBracket", tKeyFrameAnimation::tBracket::gBases, tKeyFrameAnimation::tBracket::gMembers );


//
// tKeyFrameAnimation
const ::Sig::Rtti::tBaseClassDesc tKeyFrameAnimation::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tKeyFrameAnimation::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, tLoadInPlaceStringPtr* >( "tKeyFrameAnimation", "tLoadInPlaceStringPtr*", "mName", "public", offsetof( tKeyFrameAnimation, mName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, u32 >( "tKeyFrameAnimation", "u32", "mFlags", "public", offsetof( tKeyFrameAnimation, mFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, u32 >( "tKeyFrameAnimation", "u32", "mFramesPerSecond", "public", offsetof( tKeyFrameAnimation, mFramesPerSecond ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, f32 >( "tKeyFrameAnimation", "f32", "mLengthOneShot", "public", offsetof( tKeyFrameAnimation, mLengthOneShot ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, f32 >( "tKeyFrameAnimation", "f32", "mLengthLooping", "public", offsetof( tKeyFrameAnimation, mLengthLooping ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, tBone >( "tKeyFrameAnimation", "tBone", "mReferenceFrame", "public", offsetof( tKeyFrameAnimation, mReferenceFrame ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, tBoneArray >( "tKeyFrameAnimation", "tBoneArray", "mBones", "public", offsetof( tKeyFrameAnimation, mBones ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, tKeyFrameEventArray >( "tKeyFrameAnimation", "tKeyFrameEventArray", "mEvents", "public", offsetof( tKeyFrameAnimation, mEvents ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKeyFrameAnimation, tAnimPackFile* >( "tKeyFrameAnimation", "tAnimPackFile*", "mPackFile", "public", offsetof( tKeyFrameAnimation, mPackFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tKeyFrameAnimation::gReflector = ::Sig::Rtti::fDefineReflector< tKeyFrameAnimation >( "tKeyFrameAnimation", tKeyFrameAnimation::gBases, tKeyFrameAnimation::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tLoadInPlaceFileBase.hpp]
//
#include "tLoadInPlaceFileBase.hpp"
namespace Sig
{

//
// tLoadInPlaceStringPtr
const ::Sig::Rtti::tBaseClassDesc tLoadInPlaceStringPtr::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLoadInPlaceStringPtr::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceStringPtr, tLoadInPlaceRuntimeObject<tStringPtr> >( "tLoadInPlaceStringPtr", "tLoadInPlaceRuntimeObject<tStringPtr>", "mStringPtr", "public", offsetof( tLoadInPlaceStringPtr, mStringPtr ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceStringPtr, tLoadInPlaceString >( "tLoadInPlaceStringPtr", "tLoadInPlaceString", "mRawString", "public", offsetof( tLoadInPlaceStringPtr, mRawString ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLoadInPlaceStringPtr::gReflector = ::Sig::Rtti::fDefineReflector< tLoadInPlaceStringPtr >( "tLoadInPlaceStringPtr", tLoadInPlaceStringPtr::gBases, tLoadInPlaceStringPtr::gMembers );


//
// tLoadInPlaceResourceId
const ::Sig::Rtti::tBaseClassDesc tLoadInPlaceResourceId::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLoadInPlaceResourceId::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceResourceId, Rtti::tClassId >( "tLoadInPlaceResourceId", "Rtti::tClassId", "mClassId", "public", offsetof( tLoadInPlaceResourceId, mClassId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceResourceId, tLoadInPlaceRuntimeObject<tFilePathPtr> >( "tLoadInPlaceResourceId", "tLoadInPlaceRuntimeObject<tFilePathPtr>", "mFilePathPtr", "public", offsetof( tLoadInPlaceResourceId, mFilePathPtr ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceResourceId, tLoadInPlaceString >( "tLoadInPlaceResourceId", "tLoadInPlaceString", "mRawPath", "public", offsetof( tLoadInPlaceResourceId, mRawPath ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceResourceId, b32 >( "tLoadInPlaceResourceId", "b32", "mWillResize", "public", offsetof( tLoadInPlaceResourceId, mWillResize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLoadInPlaceResourceId::gReflector = ::Sig::Rtti::fDefineReflector< tLoadInPlaceResourceId >( "tLoadInPlaceResourceId", tLoadInPlaceResourceId::gBases, tLoadInPlaceResourceId::gMembers );


//
// tLoadInPlaceResourcePtr
const ::Sig::Rtti::tBaseClassDesc tLoadInPlaceResourcePtr::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tLoadInPlaceResourcePtr, tLoadInPlaceResourceId >( "tLoadInPlaceResourcePtr", "tLoadInPlaceResourceId", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLoadInPlaceResourcePtr::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceResourcePtr, tLoadInPlaceRuntimeObject<tResourcePtr> >( "tLoadInPlaceResourcePtr", "tLoadInPlaceRuntimeObject<tResourcePtr>", "mResourcePtr", "public", offsetof( tLoadInPlaceResourcePtr, mResourcePtr ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLoadInPlaceResourcePtr::gReflector = ::Sig::Rtti::fDefineReflector< tLoadInPlaceResourcePtr >( "tLoadInPlaceResourcePtr", tLoadInPlaceResourcePtr::gBases, tLoadInPlaceResourcePtr::gMembers );


//
// tLoadInPlaceFileBase
const ::Sig::Rtti::tBaseClassDesc tLoadInPlaceFileBase::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tLoadInPlaceFileBase, tBinaryFileBase >( "tLoadInPlaceFileBase", "tBinaryFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLoadInPlaceFileBase::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceFileBase, tStringPtrTable >( "tLoadInPlaceFileBase", "tStringPtrTable", "mStringPtrTable", "public", offsetof( tLoadInPlaceFileBase, mStringPtrTable ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceFileBase, tResourceIdTable >( "tLoadInPlaceFileBase", "tResourceIdTable", "mResourceIdTable", "public", offsetof( tLoadInPlaceFileBase, mResourceIdTable ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceFileBase, tResourcePtrTable >( "tLoadInPlaceFileBase", "tResourcePtrTable", "mResourcePtrTable", "public", offsetof( tLoadInPlaceFileBase, mResourcePtrTable ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceFileBase, tLoadInPlaceRuntimeObject<tResource::tOnLoadComplete::tObserver> >( "tLoadInPlaceFileBase", "tLoadInPlaceRuntimeObject<tResource::tOnLoadComplete::tObserver>", "mOnSubResourceLoaded", "public", offsetof( tLoadInPlaceFileBase, mOnSubResourceLoaded ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceFileBase, Sig::byte* >( "tLoadInPlaceFileBase", "Sig::byte*", "mOwnerResource", "public", offsetof( tLoadInPlaceFileBase, mOwnerResource ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceFileBase, u32 >( "tLoadInPlaceFileBase", "u32", "mNumSubResourcesLoadedSuccess", "public", offsetof( tLoadInPlaceFileBase, mNumSubResourcesLoadedSuccess ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceFileBase, u32 >( "tLoadInPlaceFileBase", "u32", "mNumSubResourcesLoadedFailed", "public", offsetof( tLoadInPlaceFileBase, mNumSubResourcesLoadedFailed ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLoadInPlaceFileBase::gReflector = ::Sig::Rtti::fDefineReflector< tLoadInPlaceFileBase >( "tLoadInPlaceFileBase", tLoadInPlaceFileBase::gBases, tLoadInPlaceFileBase::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tLocalizationFile.hpp]
//
#include "tLocalizationFile.hpp"
namespace Sig
{

//
// tLocalizedString
const ::Sig::Rtti::tBaseClassDesc tLocalizedString::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tLocalizedString, tDynamicArray<u16> >( "tLocalizedString", "tDynamicArray<u16>", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLocalizedString::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLocalizedString::gReflector = ::Sig::Rtti::fDefineReflector< tLocalizedString >( "tLocalizedString", tLocalizedString::gBases, tLocalizedString::gMembers );


//
// tLocalizationFile::tStringEntry
const ::Sig::Rtti::tBaseClassDesc tLocalizationFile::tStringEntry::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLocalizationFile::tStringEntry::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLocalizationFile::tStringEntry, tLoadInPlaceStringPtr* >( "tLocalizationFile::tStringEntry", "tLoadInPlaceStringPtr*", "mId", "public", offsetof( tLocalizationFile::tStringEntry, mId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLocalizationFile::tStringEntry, tLocalizedString >( "tLocalizationFile::tStringEntry", "tLocalizedString", "mText", "public", offsetof( tLocalizationFile::tStringEntry, mText ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLocalizationFile::tStringEntry::gReflector = ::Sig::Rtti::fDefineReflector< tLocalizationFile::tStringEntry >( "tLocalizationFile::tStringEntry", tLocalizationFile::tStringEntry::gBases, tLocalizationFile::tStringEntry::gMembers );


//
// tLocalizationFile::tPathEntry
const ::Sig::Rtti::tBaseClassDesc tLocalizationFile::tPathEntry::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLocalizationFile::tPathEntry::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLocalizationFile::tPathEntry, tLoadInPlaceStringPtr* >( "tLocalizationFile::tPathEntry", "tLoadInPlaceStringPtr*", "mId", "public", offsetof( tLocalizationFile::tPathEntry, mId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLocalizationFile::tPathEntry, tLoadInPlaceResourceId* >( "tLocalizationFile::tPathEntry", "tLoadInPlaceResourceId*", "mPath", "public", offsetof( tLocalizationFile::tPathEntry, mPath ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLocalizationFile::tPathEntry::gReflector = ::Sig::Rtti::fDefineReflector< tLocalizationFile::tPathEntry >( "tLocalizationFile::tPathEntry", tLocalizationFile::tPathEntry::gBases, tLocalizationFile::tPathEntry::gMembers );


//
// tLocalizationFile
const ::Sig::Rtti::tBaseClassDesc tLocalizationFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tLocalizationFile, tLoadInPlaceFileBase >( "tLocalizationFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLocalizationFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLocalizationFile, tStringArray >( "tLocalizationFile", "tStringArray", "mRawStrings", "public", offsetof( tLocalizationFile, mRawStrings ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLocalizationFile, tStringMapStorage >( "tLocalizationFile", "tStringMapStorage", "mStringMap", "public", offsetof( tLocalizationFile, mStringMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLocalizationFile, tPathArray >( "tLocalizationFile", "tPathArray", "mRawPaths", "public", offsetof( tLocalizationFile, mRawPaths ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLocalizationFile, tPathMapStorage >( "tLocalizationFile", "tPathMapStorage", "mPathMap", "public", offsetof( tLocalizationFile, mPathMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLocalizationFile::gReflector = ::Sig::Rtti::fDefineReflector< tLocalizationFile >( "tLocalizationFile", tLocalizationFile::gBases, tLocalizationFile::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tMesh.hpp]
//
#include "tMesh.hpp"
namespace Sig
{

//
// tSubMesh
const ::Sig::Rtti::tBaseClassDesc tSubMesh::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSubMesh::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, Math::tAabbf >( "tSubMesh", "Math::tAabbf", "mBounds", "public", offsetof( tSubMesh, mBounds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, tPolySoupVertexList >( "tSubMesh", "tPolySoupVertexList", "mVertices", "public", offsetof( tSubMesh, mVertices ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, tPolySoupTriangleList >( "tSubMesh", "tPolySoupTriangleList", "mTriangles", "public", offsetof( tSubMesh, mTriangles ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, tPolySoupKDTree >( "tSubMesh", "tPolySoupKDTree", "mPolySoupKDTree", "public", offsetof( tSubMesh, mPolySoupKDTree ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, u32 >( "tSubMesh", "u32", "mGeometryBufferIndex", "public", offsetof( tSubMesh, mGeometryBufferIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, u32 >( "tSubMesh", "u32", "mIndexBufferIndex", "public", offsetof( tSubMesh, mIndexBufferIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, Gfx::tMaterial* >( "tSubMesh", "Gfx::tMaterial*", "mMaterial", "public", offsetof( tSubMesh, mMaterial ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSubMesh, Gfx::tVertexFormatVRam* >( "tSubMesh", "Gfx::tVertexFormatVRam*", "mVertexFormat", "public", offsetof( tSubMesh, mVertexFormat ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSubMesh::gReflector = ::Sig::Rtti::fDefineReflector< tSubMesh >( "tSubMesh", tSubMesh::gBases, tSubMesh::gMembers );


//
// tSkin::tInfluence
const ::Sig::Rtti::tBaseClassDesc tSkin::tInfluence::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSkin::tInfluence::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSkin::tInfluence, tLoadInPlaceStringPtr* >( "tSkin::tInfluence", "tLoadInPlaceStringPtr*", "mName", "public", offsetof( tSkin::tInfluence, mName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSkin::tInfluence::gReflector = ::Sig::Rtti::fDefineReflector< tSkin::tInfluence >( "tSkin::tInfluence", tSkin::tInfluence::gBases, tSkin::tInfluence::gMembers );


//
// tSkin
const ::Sig::Rtti::tBaseClassDesc tSkin::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSkin::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSkin, tInfluenceList >( "tSkin", "tInfluenceList", "mInfluences", "public", offsetof( tSkin, mInfluences ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSkin::gReflector = ::Sig::Rtti::fDefineReflector< tSkin >( "tSkin", tSkin::gBases, tSkin::gMembers );


//
// tMesh
const ::Sig::Rtti::tBaseClassDesc tMesh::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tMesh::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMesh, Math::tAabbf >( "tMesh", "Math::tAabbf", "mBounds", "public", offsetof( tMesh, mBounds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMesh, tLoadInPlaceResourcePtr* >( "tMesh", "tLoadInPlaceResourcePtr*", "mGeometryFile", "public", offsetof( tMesh, mGeometryFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMesh, tSubMeshArray >( "tMesh", "tSubMeshArray", "mSubMeshes", "public", offsetof( tMesh, mSubMeshes ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMesh, tSkin* >( "tMesh", "tSkin*", "mSkin", "public", offsetof( tMesh, mSkin ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tMesh::gReflector = ::Sig::Rtti::fDefineReflector< tMesh >( "tMesh", tMesh::gBases, tMesh::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tMeshEntity.hpp]
//
#include "tMeshEntity.hpp"
namespace Sig
{

//
// tMeshEntityDef
const ::Sig::Rtti::tBaseClassDesc tMeshEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tMeshEntityDef, tEntityDef >( "tMeshEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tMeshEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMeshEntityDef, tMesh* >( "tMeshEntityDef", "tMesh*", "mMesh", "public", offsetof( tMeshEntityDef, mMesh ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMeshEntityDef, s16 >( "tMeshEntityDef", "s16", "mStateIndex", "public", offsetof( tMeshEntityDef, mStateIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMeshEntityDef, tEnum<tStateType,u8> >( "tMeshEntityDef", "tEnum<tStateType,u8>", "mStateType", "public", offsetof( tMeshEntityDef, mStateType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMeshEntityDef, u8 >( "tMeshEntityDef", "u8", "pad0", "public", offsetof( tMeshEntityDef, pad0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMeshEntityDef, f32 >( "tMeshEntityDef", "f32", "mSortOffset", "public", offsetof( tMeshEntityDef, mSortOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tMeshEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tMeshEntityDef >( "tMeshEntityDef", tMeshEntityDef::gBases, tMeshEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tNavGraphEntity.hpp]
//
#include "tNavGraphEntity.hpp"
namespace Sig
{

//
// tNavGraphEntityDef
const ::Sig::Rtti::tBaseClassDesc tNavGraphEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tNavGraphEntityDef, tEntityDef >( "tNavGraphEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tNavGraphEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tNavGraphEntityDef, AI::tBuiltNavGraph* >( "tNavGraphEntityDef", "AI::tBuiltNavGraph*", "mNavGraph", "public", offsetof( tNavGraphEntityDef, mNavGraph ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tNavGraphEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tNavGraphEntityDef >( "tNavGraphEntityDef", tNavGraphEntityDef::gBases, tNavGraphEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tOctree.hpp]
//
#include "tOctree.hpp"
namespace Sig
{

//
// tOctree
const ::Sig::Rtti::tBaseClassDesc tOctree::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tOctree, tSpatialTree >( "tOctree", "tSpatialTree", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tOctree::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tOctree::gReflector = ::Sig::Rtti::fDefineReflector< tOctree >( "tOctree", tOctree::gBases, tOctree::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tPathDecalEntityDef.hpp]
//
#include "tPathDecalEntityDef.hpp"
namespace Sig
{

//
// tPathDecalEntityDef
const ::Sig::Rtti::tBaseClassDesc tPathDecalEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPathDecalEntityDef, tEntityDef >( "tPathDecalEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPathDecalEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, f32 >( "tPathDecalEntityDef", "f32", "mCameraDepthOffset", "public", offsetof( tPathDecalEntityDef, mCameraDepthOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, b8 >( "tPathDecalEntityDef", "b8", "mAcceptsLights", "public", offsetof( tPathDecalEntityDef, mAcceptsLights ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, s8 >( "tPathDecalEntityDef", "s8", "mDepthBias", "public", offsetof( tPathDecalEntityDef, mDepthBias ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, s8 >( "tPathDecalEntityDef", "s8", "mSlopeScaleDepthBias", "public", offsetof( tPathDecalEntityDef, mSlopeScaleDepthBias ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, b8 >( "tPathDecalEntityDef", "b8", "pad0", "public", offsetof( tPathDecalEntityDef, pad0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, tDynamicArray<Gfx::tDecalRenderVertex> >( "tPathDecalEntityDef", "tDynamicArray<Gfx::tDecalRenderVertex>", "mVerts", "public", offsetof( tPathDecalEntityDef, mVerts ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, tDynamicArray<u16> >( "tPathDecalEntityDef", "tDynamicArray<u16>", "mTriIndices", "public", offsetof( tPathDecalEntityDef, mTriIndices ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, tLoadInPlaceResourcePtr* >( "tPathDecalEntityDef", "tLoadInPlaceResourcePtr*", "mDiffuseTexture", "public", offsetof( tPathDecalEntityDef, mDiffuseTexture ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathDecalEntityDef, tLoadInPlaceResourcePtr* >( "tPathDecalEntityDef", "tLoadInPlaceResourcePtr*", "mNormalMap", "public", offsetof( tPathDecalEntityDef, mNormalMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPathDecalEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tPathDecalEntityDef >( "tPathDecalEntityDef", tPathDecalEntityDef::gBases, tPathDecalEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tPathEntity.hpp]
//
#include "tPathEntity.hpp"
namespace Sig
{

//
// tPathEntityDef
const ::Sig::Rtti::tBaseClassDesc tPathEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPathEntityDef, tEntityDef >( "tPathEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPathEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPathEntityDef, u32 >( "tPathEntityDef", "u32", "mGuid", "public", offsetof( tPathEntityDef, mGuid ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPathEntityDef, tConnectionList >( "tPathEntityDef", "tConnectionList", "mNextPoints", "public", offsetof( tPathEntityDef, mNextPoints ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPathEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tPathEntityDef >( "tPathEntityDef", tPathEntityDef::gBases, tPathEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tPolySoupKDTree.hpp]
//
#include "tPolySoupKDTree.hpp"
namespace Sig
{

//
// tKDNode
const ::Sig::Rtti::tBaseClassDesc tKDNode::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tKDNode::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tKDNode, tFixedArray<tLoadInPlacePtrWrapper<tKDNode>,2> >( "tKDNode", "tFixedArray<tLoadInPlacePtrWrapper<tKDNode>,2>", "mChildren", "public", offsetof( tKDNode, mChildren ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKDNode, tEnum<tAxis,u32> >( "tKDNode", "tEnum<tAxis,u32>", "mAxis", "public", offsetof( tKDNode, mAxis ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKDNode, f32 >( "tKDNode", "f32", "mSplitPosition", "public", offsetof( tKDNode, mSplitPosition ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tKDNode, tDynamicArray<u32> >( "tKDNode", "tDynamicArray<u32>", "mItems", "public", offsetof( tKDNode, mItems ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tKDNode::gReflector = ::Sig::Rtti::fDefineReflector< tKDNode >( "tKDNode", tKDNode::gBases, tKDNode::gMembers );


//
// tPolySoupKDTree
const ::Sig::Rtti::tBaseClassDesc tPolySoupKDTree::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPolySoupKDTree::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPolySoupKDTree, Math::tAabbf >( "tPolySoupKDTree", "Math::tAabbf", "mBounds", "public", offsetof( tPolySoupKDTree, mBounds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPolySoupKDTree, tKDNode >( "tPolySoupKDTree", "tKDNode", "mFirstSplit", "public", offsetof( tPolySoupKDTree, mFirstSplit ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPolySoupKDTree::gReflector = ::Sig::Rtti::fDefineReflector< tPolySoupKDTree >( "tPolySoupKDTree", tPolySoupKDTree::gBases, tPolySoupKDTree::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tPolySoupOctree.hpp]
//
#include "tPolySoupOctree.hpp"
namespace Sig
{

//
// tPolySoupOctree
const ::Sig::Rtti::tBaseClassDesc tPolySoupOctree::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPolySoupOctree, tOctree >( "tPolySoupOctree", "tOctree", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPolySoupOctree::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPolySoupOctree, tDynamicArray<u32> >( "tPolySoupOctree", "tDynamicArray<u32>", "mTriIndices", "public", offsetof( tPolySoupOctree, mTriIndices ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPolySoupOctree, tFixedArray<tLoadInPlacePtrWrapper<tPolySoupOctree>,cCellCount> >( "tPolySoupOctree", "tFixedArray<tLoadInPlacePtrWrapper<tPolySoupOctree>,cCellCount>", "mChildren", "public", offsetof( tPolySoupOctree, mChildren ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPolySoupOctree::gReflector = ::Sig::Rtti::fDefineReflector< tPolySoupOctree >( "tPolySoupOctree", tPolySoupOctree::gBases, tPolySoupOctree::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tQuadtree.hpp]
//
#include "tQuadtree.hpp"
namespace Sig
{

//
// tQuadtree
const ::Sig::Rtti::tBaseClassDesc tQuadtree::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tQuadtree, tSpatialTree >( "tQuadtree", "tSpatialTree", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tQuadtree::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tQuadtree::gReflector = ::Sig::Rtti::fDefineReflector< tQuadtree >( "tQuadtree", tQuadtree::gBases, tQuadtree::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tRefCounterPtr.hpp]
//
#include "tRefCounterPtr.hpp"
namespace Sig
{

//
// tRefCounter
const ::Sig::Rtti::tBaseClassDesc tRefCounter::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tRefCounter::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tRefCounter, s32 >( "tRefCounter", "s32", "mRefCount", "public", offsetof( tRefCounter, mRefCount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tRefCounter::gReflector = ::Sig::Rtti::fDefineReflector< tRefCounter >( "tRefCounter", tRefCounter::gBases, tRefCounter::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tSceneGraphFile.hpp]
//
#include "tSceneGraphFile.hpp"
namespace Sig
{

//
// tSceneGraphDefaultLight
const ::Sig::Rtti::tBaseClassDesc tSceneGraphDefaultLight::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSceneGraphDefaultLight::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphDefaultLight, Math::tVec3f >( "tSceneGraphDefaultLight", "Math::tVec3f", "mFrontColor", "public", offsetof( tSceneGraphDefaultLight, mFrontColor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphDefaultLight, Math::tVec3f >( "tSceneGraphDefaultLight", "Math::tVec3f", "mBackColor", "public", offsetof( tSceneGraphDefaultLight, mBackColor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphDefaultLight, Math::tVec3f >( "tSceneGraphDefaultLight", "Math::tVec3f", "mRimColor", "public", offsetof( tSceneGraphDefaultLight, mRimColor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphDefaultLight, Math::tVec3f >( "tSceneGraphDefaultLight", "Math::tVec3f", "mAmbientColor", "public", offsetof( tSceneGraphDefaultLight, mAmbientColor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphDefaultLight, Math::tVec3f >( "tSceneGraphDefaultLight", "Math::tVec3f", "mDirection", "public", offsetof( tSceneGraphDefaultLight, mDirection ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphDefaultLight, b32 >( "tSceneGraphDefaultLight", "b32", "mCastShadow", "public", offsetof( tSceneGraphDefaultLight, mCastShadow ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSceneGraphDefaultLight::gReflector = ::Sig::Rtti::fDefineReflector< tSceneGraphDefaultLight >( "tSceneGraphDefaultLight", tSceneGraphDefaultLight::gBases, tSceneGraphDefaultLight::gMembers );


//
// tSceneLODSettings
const ::Sig::Rtti::tBaseClassDesc tSceneLODSettings::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSceneLODSettings::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSceneLODSettings, u32 >( "tSceneLODSettings", "u32", "mFadeSetting", "public", offsetof( tSceneLODSettings, mFadeSetting ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneLODSettings, f32 >( "tSceneLODSettings", "f32", "mFadeOverride", "public", offsetof( tSceneLODSettings, mFadeOverride ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSceneLODSettings::gReflector = ::Sig::Rtti::fDefineReflector< tSceneLODSettings >( "tSceneLODSettings", tSceneLODSettings::gBases, tSceneLODSettings::gMembers );


//
// tPigmentChoice
const ::Sig::Rtti::tBaseClassDesc tPigmentChoice::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPigmentChoice::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPigmentChoice, u32 >( "tPigmentChoice", "u32", "mGuid", "public", offsetof( tPigmentChoice, mGuid ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPigmentChoice, f32 >( "tPigmentChoice", "f32", "mChanceWeight", "public", offsetof( tPigmentChoice, mChanceWeight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPigmentChoice::gReflector = ::Sig::Rtti::fDefineReflector< tPigmentChoice >( "tPigmentChoice", tPigmentChoice::gBases, tPigmentChoice::gMembers );


//
// tPigment
const ::Sig::Rtti::tBaseClassDesc tPigment::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPigment::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPigment, u32 >( "tPigment", "u32", "mGuid", "public", offsetof( tPigment, mGuid ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPigment, tDynamicArray<tPigmentChoice> >( "tPigment", "tDynamicArray<tPigmentChoice>", "mPigmentChoices", "public", offsetof( tPigment, mPigmentChoices ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPigment::gReflector = ::Sig::Rtti::fDefineReflector< tPigment >( "tPigment", tPigment::gBases, tPigment::gMembers );


//
// tTileDef
const ::Sig::Rtti::tBaseClassDesc tTileDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTileDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTileDef, f32 >( "tTileDef", "f32", "mChanceWeight", "public", offsetof( tTileDef, mChanceWeight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTileDef::gReflector = ::Sig::Rtti::fDefineReflector< tTileDef >( "tTileDef", tTileDef::gBases, tTileDef::gMembers );


//
// tTileSet
const ::Sig::Rtti::tBaseClassDesc tTileSet::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTileSet::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTileSet, u32 >( "tTileSet", "u32", "mGuid", "public", offsetof( tTileSet, mGuid ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTileSet, tDynamicArray<tTileDef> >( "tTileSet", "tDynamicArray<tTileDef>", "mTileDefs", "public", offsetof( tTileSet, mTileDefs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTileSet::gReflector = ::Sig::Rtti::fDefineReflector< tTileSet >( "tTileSet", tTileSet::gBases, tTileSet::gMembers );


//
// tTilePackage
const ::Sig::Rtti::tBaseClassDesc tTilePackage::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTilePackage::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTilePackage, tDynamicArray<tPigment> >( "tTilePackage", "tDynamicArray<tPigment>", "mPigments", "public", offsetof( tTilePackage, mPigments ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTilePackage, tDynamicArray<tTileSet> >( "tTilePackage", "tDynamicArray<tTileSet>", "mTileSets", "public", offsetof( tTilePackage, mTileSets ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTilePackage::gReflector = ::Sig::Rtti::fDefineReflector< tTilePackage >( "tTilePackage", tTilePackage::gBases, tTilePackage::gMembers );


//
// tSceneGraphFile
const ::Sig::Rtti::tBaseClassDesc tSceneGraphFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tSceneGraphFile, tLoadInPlaceFileBase >( "tSceneGraphFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc< tSceneGraphFile, tEntityDefProperties >( "tSceneGraphFile", "tEntityDefProperties", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSceneGraphFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphFile, Math::tAabbf >( "tSceneGraphFile", "Math::tAabbf", "mBounds", "public", offsetof( tSceneGraphFile, mBounds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphFile, Math::tMat3f >( "tSceneGraphFile", "Math::tMat3f", "mSkeletonBinding", "public", offsetof( tSceneGraphFile, mSkeletonBinding ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphFile, Math::tMat3f >( "tSceneGraphFile", "Math::tMat3f", "mSkeletonBindingInv", "public", offsetof( tSceneGraphFile, mSkeletonBindingInv ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphFile, tObjectArray >( "tSceneGraphFile", "tObjectArray", "mObjects", "public", offsetof( tSceneGraphFile, mObjects ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphFile, tSceneGraphDefaultLight* >( "tSceneGraphFile", "tSceneGraphDefaultLight*", "mDefaultLight", "public", offsetof( tSceneGraphFile, mDefaultLight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphFile, tSceneLODSettings* >( "tSceneGraphFile", "tSceneLODSettings*", "mLODSettings", "public", offsetof( tSceneGraphFile, mLODSettings ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneGraphFile, tTilePackage* >( "tSceneGraphFile", "tTilePackage*", "mTilePackage", "public", offsetof( tSceneGraphFile, mTilePackage ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSceneGraphFile::gReflector = ::Sig::Rtti::fDefineReflector< tSceneGraphFile >( "tSceneGraphFile", tSceneGraphFile::gBases, tSceneGraphFile::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tSceneRefEntity.hpp]
//
#include "tSceneRefEntity.hpp"
namespace Sig
{

//
// tSceneRefEntityDef
const ::Sig::Rtti::tBaseClassDesc tSceneRefEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tSceneRefEntityDef, tEntityDef >( "tSceneRefEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSceneRefEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSceneRefEntityDef, tLoadInPlaceResourcePtr* >( "tSceneRefEntityDef", "tLoadInPlaceResourcePtr*", "mReferenceFile", "public", offsetof( tSceneRefEntityDef, mReferenceFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSceneRefEntityDef, tSceneLODSettings* >( "tSceneRefEntityDef", "tSceneLODSettings*", "mLODSettings", "public", offsetof( tSceneRefEntityDef, mLODSettings ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSceneRefEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tSceneRefEntityDef >( "tSceneRefEntityDef", tSceneRefEntityDef::gBases, tSceneRefEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tShapeEntity.hpp]
//
#include "tShapeEntity.hpp"
namespace Sig
{

//
// tShapeEntityDef
const ::Sig::Rtti::tBaseClassDesc tShapeEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tShapeEntityDef, tEntityDef >( "tShapeEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tShapeEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tShapeEntityDef, tEnum<tShapeType,u8> >( "tShapeEntityDef", "tEnum<tShapeType,u8>", "mShapeType", "public", offsetof( tShapeEntityDef, mShapeType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShapeEntityDef, u8 >( "tShapeEntityDef", "u8", "pad2", "public", offsetof( tShapeEntityDef, pad2 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShapeEntityDef, u16 >( "tShapeEntityDef", "u16", "mStateMask", "public", offsetof( tShapeEntityDef, mStateMask ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShapeEntityDef, Math::tVec4f >( "tShapeEntityDef", "Math::tVec4f", "pad3", "public", offsetof( tShapeEntityDef, pad3 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShapeEntityDef, Math::tVec4f >( "tShapeEntityDef", "Math::tVec4f", "pad4", "public", offsetof( tShapeEntityDef, pad4 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tShapeEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tShapeEntityDef >( "tShapeEntityDef", tShapeEntityDef::gBases, tShapeEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tSkeletonFile.hpp]
//
#include "tSkeletonFile.hpp"
namespace Sig
{

//
// tBone
const ::Sig::Rtti::tBaseClassDesc tBone::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBone::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBone, s32 >( "tBone", "s32", "mMasterIndex", "public", offsetof( tBone, mMasterIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBone, tLoadInPlaceStringPtr* >( "tBone", "tLoadInPlaceStringPtr*", "mName", "public", offsetof( tBone, mName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBone, s32 >( "tBone", "s32", "mParent", "public", offsetof( tBone, mParent ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBone, Math::tMat3f >( "tBone", "Math::tMat3f", "mRefPose", "public", offsetof( tBone, mRefPose ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBone, Math::tMat3f >( "tBone", "Math::tMat3f", "mRefPoseInv", "public", offsetof( tBone, mRefPoseInv ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBone, Math::tPRSXformf >( "tBone", "Math::tPRSXformf", "mRefLocalPose", "public", offsetof( tBone, mRefLocalPose ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBone, tChildList >( "tBone", "tChildList", "mChildren", "public", offsetof( tBone, mChildren ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBone::gReflector = ::Sig::Rtti::fDefineReflector< tBone >( "tBone", tBone::gBases, tBone::gMembers );


//
// tSkeletonMap::tBoneMap
const ::Sig::Rtti::tBaseClassDesc tSkeletonMap::tBoneMap::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSkeletonMap::tBoneMap::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonMap::tBoneMap, u32 >( "tSkeletonMap::tBoneMap", "u32", "mTargetBoneIndex", "public", offsetof( tSkeletonMap::tBoneMap, mTargetBoneIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonMap::tBoneMap, u32 >( "tSkeletonMap::tBoneMap", "u32", "mSrc2TgtXformIndex", "public", offsetof( tSkeletonMap::tBoneMap, mSrc2TgtXformIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSkeletonMap::tBoneMap::gReflector = ::Sig::Rtti::fDefineReflector< tSkeletonMap::tBoneMap >( "tSkeletonMap::tBoneMap", tSkeletonMap::tBoneMap::gBases, tSkeletonMap::tBoneMap::gMembers );


//
// tSkeletonMap
const ::Sig::Rtti::tBaseClassDesc tSkeletonMap::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSkeletonMap::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonMap, tLoadInPlaceStringPtr* >( "tSkeletonMap", "tLoadInPlaceStringPtr*", "mSrcSkeletonPath", "public", offsetof( tSkeletonMap, mSrcSkeletonPath ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonMap, tDynamicArray<tBoneMap> >( "tSkeletonMap", "tDynamicArray<tBoneMap>", "mSrc2TgtBones", "public", offsetof( tSkeletonMap, mSrc2TgtBones ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonMap, tDynamicArray<Math::tPRSXformf> >( "tSkeletonMap", "tDynamicArray<Math::tPRSXformf>", "mSrc2TgtXforms", "public", offsetof( tSkeletonMap, mSrc2TgtXforms ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSkeletonMap::gReflector = ::Sig::Rtti::fDefineReflector< tSkeletonMap >( "tSkeletonMap", tSkeletonMap::gBases, tSkeletonMap::gMembers );


//
// tSkeletonFile
const ::Sig::Rtti::tBaseClassDesc tSkeletonFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tSkeletonFile, tLoadInPlaceFileBase >( "tSkeletonFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSkeletonFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonFile, tBone >( "tSkeletonFile", "tBone", "mReferenceFrame", "public", offsetof( tSkeletonFile, mReferenceFrame ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonFile, tMasterBoneArray >( "tSkeletonFile", "tMasterBoneArray", "mMasterBoneList", "public", offsetof( tSkeletonFile, mMasterBoneList ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonFile, tBoneMapStorage >( "tSkeletonFile", "tBoneMapStorage", "mBonesByName", "public", offsetof( tSkeletonFile, mBonesByName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonFile, tPreOrderTraversal >( "tSkeletonFile", "tPreOrderTraversal", "mPreOrderTraversal", "public", offsetof( tSkeletonFile, mPreOrderTraversal ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonFile, tSkeletonMapArray >( "tSkeletonFile", "tSkeletonMapArray", "mSkeletonMaps", "public", offsetof( tSkeletonFile, mSkeletonMaps ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSkeletonFile, s32 >( "tSkeletonFile", "s32", "mRoot", "public", offsetof( tSkeletonFile, mRoot ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSkeletonFile::gReflector = ::Sig::Rtti::fDefineReflector< tSkeletonFile >( "tSkeletonFile", tSkeletonFile::gBases, tSkeletonFile::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tTileEntity.hpp]
//
#include "tTileEntity.hpp"
namespace Sig
{

//
// tTileEntityDef
const ::Sig::Rtti::tBaseClassDesc tTileEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tTileEntityDef, tSceneRefEntityDef >( "tTileEntityDef", "tSceneRefEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTileEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTileEntityDef, tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceResourcePtr> > >( "tTileEntityDef", "tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceResourcePtr> >", "mTileScripts", "public", offsetof( tTileEntityDef, mTileScripts ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTileEntityDef, u32 >( "tTileEntityDef", "u32", "mPigmentGuid", "public", offsetof( tTileEntityDef, mPigmentGuid ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTileEntityDef, u32 >( "tTileEntityDef", "u32", "mTileType", "public", offsetof( tTileEntityDef, mTileType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTileEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tTileEntityDef >( "tTileEntityDef", tTileEntityDef::gBases, tTileEntityDef::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tXmlFile.hpp]
//
#include "tXmlFile.hpp"
namespace Sig
{

//
// tXmlFile
const ::Sig::Rtti::tBaseClassDesc tXmlFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tXmlFile, tByteFile >( "tXmlFile", "tByteFile", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tXmlFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tXmlFile::gReflector = ::Sig::Rtti::fDefineReflector< tXmlFile >( "tXmlFile", tXmlFile::gBases, tXmlFile::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Scripts\tScriptFile.hpp]
//
#include "Scripts/tScriptFile.hpp"
namespace Sig
{

//
// tScriptFile::tExportedFunction
const ::Sig::Rtti::tBaseClassDesc tScriptFile::tExportedFunction::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tScriptFile::tExportedFunction::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile::tExportedFunction, tLoadInPlaceStringPtr* >( "tScriptFile::tExportedFunction", "tLoadInPlaceStringPtr*", "mCallableName", "public", offsetof( tScriptFile::tExportedFunction, mCallableName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile::tExportedFunction, tLoadInPlaceStringPtr* >( "tScriptFile::tExportedFunction", "tLoadInPlaceStringPtr*", "mExportedName", "public", offsetof( tScriptFile::tExportedFunction, mExportedName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile::tExportedFunction, tScriptObjectStorage >( "tScriptFile::tExportedFunction", "tScriptObjectStorage", "mScriptObject", "public", offsetof( tScriptFile::tExportedFunction, mScriptObject ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tScriptFile::tExportedFunction::gReflector = ::Sig::Rtti::fDefineReflector< tScriptFile::tExportedFunction >( "tScriptFile::tExportedFunction", tScriptFile::tExportedFunction::gBases, tScriptFile::tExportedFunction::gMembers );


//
// tScriptFile
const ::Sig::Rtti::tBaseClassDesc tScriptFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tScriptFile, tLoadInPlaceFileBase >( "tScriptFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tScriptFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile, u32 >( "tScriptFile", "u32", "mFlags", "public", offsetof( tScriptFile, mFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile, u32 >( "tScriptFile", "u32", "mUniqueId", "public", offsetof( tScriptFile, mUniqueId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile, tLoadInPlaceStringPtr* >( "tScriptFile", "tLoadInPlaceStringPtr*", "mScriptClass", "public", offsetof( tScriptFile, mScriptClass ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile, tDynamicBuffer >( "tScriptFile", "tDynamicBuffer", "mByteCode", "public", offsetof( tScriptFile, mByteCode ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile, tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceResourcePtr> > >( "tScriptFile", "tDynamicArray<tLoadInPlacePtrWrapper<tLoadInPlaceResourcePtr> >", "mScriptImports", "public", offsetof( tScriptFile, mScriptImports ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile, tDynamicArray<tExportedFunction> >( "tScriptFile", "tDynamicArray<tExportedFunction>", "mExportedFunctions", "public", offsetof( tScriptFile, mExportedFunctions ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile, tDynamicArray<tExportedVariable> >( "tScriptFile", "tDynamicArray<tExportedVariable>", "mExportedVariables", "public", offsetof( tScriptFile, mExportedVariables ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tScriptFile, tFixedArray<tExportedFunction,cStandardExportedFunctionCount> >( "tScriptFile", "tFixedArray<tExportedFunction,cStandardExportedFunctionCount>", "mStandardExportedFunctions", "public", offsetof( tScriptFile, mStandardExportedFunctions ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tScriptFile::gReflector = ::Sig::Rtti::fDefineReflector< tScriptFile >( "tScriptFile", tScriptFile::gBases, tScriptFile::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tVector.hpp]
//
#include "Math/tVector.hpp"
namespace Sig
{
namespace Math
{

//
// tNoOpVec2f
const ::Sig::Rtti::tBaseClassDesc tNoOpVec2f::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tNoOpVec2f, tVec2f >( "tNoOpVec2f", "tVec2f", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tNoOpVec2f::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tNoOpVec2f::gReflector = ::Sig::Rtti::fDefineReflector< tNoOpVec2f >( "tNoOpVec2f", tNoOpVec2f::gBases, tNoOpVec2f::gMembers );


//
// tNoOpVec3f
const ::Sig::Rtti::tBaseClassDesc tNoOpVec3f::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tNoOpVec3f, tVec3f >( "tNoOpVec3f", "tVec3f", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tNoOpVec3f::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tNoOpVec3f::gReflector = ::Sig::Rtti::fDefineReflector< tNoOpVec3f >( "tNoOpVec3f", tNoOpVec3f::gBases, tNoOpVec3f::gMembers );


//
// tNoOpVec4f
const ::Sig::Rtti::tBaseClassDesc tNoOpVec4f::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tNoOpVec4f, tVec4f >( "tNoOpVec4f", "tVec4f", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tNoOpVec4f::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tNoOpVec4f::gReflector = ::Sig::Rtti::fDefineReflector< tNoOpVec4f >( "tNoOpVec4f", tNoOpVec4f::gBases, tNoOpVec4f::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gui\tFont.hpp]
//
#include "Gui/tFont.hpp"
namespace Sig
{
namespace Gui
{

//
// tFontDesc
const ::Sig::Rtti::tBaseClassDesc tFontDesc::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFontDesc::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u16 >( "tFontDesc", "u16", "mLineHeight", "public", offsetof( tFontDesc, mLineHeight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u16 >( "tFontDesc", "u16", "mBase", "public", offsetof( tFontDesc, mBase ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u16 >( "tFontDesc", "u16", "mScaleW", "public", offsetof( tFontDesc, mScaleW ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u16 >( "tFontDesc", "u16", "mScaleH", "public", offsetof( tFontDesc, mScaleH ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u16 >( "tFontDesc", "u16", "mPages", "public", offsetof( tFontDesc, mPages ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, b8 >( "tFontDesc", "b8", "mPacked", "public", offsetof( tFontDesc, mPacked ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, b8 >( "tFontDesc", "b8", "mBold", "public", offsetof( tFontDesc, mBold ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, b8 >( "tFontDesc", "b8", "mItalic", "public", offsetof( tFontDesc, mItalic ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, b8 >( "tFontDesc", "b8", "mOutline", "public", offsetof( tFontDesc, mOutline ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u8 >( "tFontDesc", "u8", "mAlphaChnl", "public", offsetof( tFontDesc, mAlphaChnl ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u8 >( "tFontDesc", "u8", "mRedChnl", "public", offsetof( tFontDesc, mRedChnl ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u8 >( "tFontDesc", "u8", "mGreenChnl", "public", offsetof( tFontDesc, mGreenChnl ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u8 >( "tFontDesc", "u8", "mBlueChnl", "public", offsetof( tFontDesc, mBlueChnl ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u8 >( "tFontDesc", "u8", "pad1", "public", offsetof( tFontDesc, pad1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontDesc, u8 >( "tFontDesc", "u8", "pad2", "public", offsetof( tFontDesc, pad2 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFontDesc::gReflector = ::Sig::Rtti::fDefineReflector< tFontDesc >( "tFontDesc", tFontDesc::gBases, tFontDesc::gMembers );


//
// tFontGlyphKerning
const ::Sig::Rtti::tBaseClassDesc tFontGlyphKerning::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFontGlyphKerning::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyphKerning, u16 >( "tFontGlyphKerning", "u16", "mSecond", "public", offsetof( tFontGlyphKerning, mSecond ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyphKerning, s16 >( "tFontGlyphKerning", "s16", "mAmount", "public", offsetof( tFontGlyphKerning, mAmount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFontGlyphKerning::gReflector = ::Sig::Rtti::fDefineReflector< tFontGlyphKerning >( "tFontGlyphKerning", tFontGlyphKerning::gBases, tFontGlyphKerning::gMembers );


//
// tFontGlyph
const ::Sig::Rtti::tBaseClassDesc tFontGlyph::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFontGlyph::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, u16 >( "tFontGlyph", "u16", "mId", "public", offsetof( tFontGlyph, mId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, u16 >( "tFontGlyph", "u16", "mX", "public", offsetof( tFontGlyph, mX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, u16 >( "tFontGlyph", "u16", "mY", "public", offsetof( tFontGlyph, mY ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, u16 >( "tFontGlyph", "u16", "mWidth", "public", offsetof( tFontGlyph, mWidth ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, u16 >( "tFontGlyph", "u16", "mHeight", "public", offsetof( tFontGlyph, mHeight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, s16 >( "tFontGlyph", "s16", "mXOffset", "public", offsetof( tFontGlyph, mXOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, s16 >( "tFontGlyph", "s16", "mYOffset", "public", offsetof( tFontGlyph, mYOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, s16 >( "tFontGlyph", "s16", "mXAdvance", "public", offsetof( tFontGlyph, mXAdvance ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, u16 >( "tFontGlyph", "u16", "mPage", "public", offsetof( tFontGlyph, mPage ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, u16 >( "tFontGlyph", "u16", "mChannel", "public", offsetof( tFontGlyph, mChannel ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontGlyph, tKernPairs >( "tFontGlyph", "tKernPairs", "mKernPairs", "public", offsetof( tFontGlyph, mKernPairs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFontGlyph::gReflector = ::Sig::Rtti::fDefineReflector< tFontGlyph >( "tFontGlyph", tFontGlyph::gBases, tFontGlyph::gMembers );


//
// tFontPage
const ::Sig::Rtti::tBaseClassDesc tFontPage::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFontPage::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFontPage, Gfx::tFontMaterial* >( "tFontPage", "Gfx::tFontMaterial*", "mMaterial", "public", offsetof( tFontPage, mMaterial ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFontPage::gReflector = ::Sig::Rtti::fDefineReflector< tFontPage >( "tFontPage", tFontPage::gBases, tFontPage::gMembers );


//
// tFont
const ::Sig::Rtti::tBaseClassDesc tFont::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tFont, tLoadInPlaceFileBase >( "tFont", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFont::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFont, tFontDesc >( "tFont", "tFontDesc", "mDesc", "public", offsetof( tFont, mDesc ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFont, tLoadInPlaceStringPtr* >( "tFont", "tLoadInPlaceStringPtr*", "mFaceName", "public", offsetof( tFont, mFaceName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFont, tGlyphArray >( "tFont", "tGlyphArray", "mRawGlyphs", "public", offsetof( tFont, mRawGlyphs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFont, tGlyphMapStorage >( "tFont", "tGlyphMapStorage", "mGlyphMap", "public", offsetof( tFont, mGlyphMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFont, tFontGlyph >( "tFont", "tFontGlyph", "mDefaultGlyph", "public", offsetof( tFont, mDefaultGlyph ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFont, tFontPageArray >( "tFont", "tFontPageArray", "mPages", "public", offsetof( tFont, mPages ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFont::gReflector = ::Sig::Rtti::fDefineReflector< tFont >( "tFont", tFont::gBases, tFont::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\DerivedShadeMaterialGlue.hpp]
//
#include "Gfx/DerivedShadeMaterialGlue.hpp"
namespace Sig
{
namespace Gfx
{

//
// tVsGlueModelToWorld
const ::Sig::Rtti::tBaseClassDesc tVsGlueModelToWorld::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueModelToWorld, tShadeMaterialGlue >( "tVsGlueModelToWorld", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueModelToWorld::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueModelToWorld, u16 >( "tVsGlueModelToWorld", "u16", "mPRegister", "public", offsetof( tVsGlueModelToWorld, mPRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueModelToWorld, u16 >( "tVsGlueModelToWorld", "u16", "mNRegister", "public", offsetof( tVsGlueModelToWorld, mNRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueModelToWorld::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueModelToWorld >( "tVsGlueModelToWorld", tVsGlueModelToWorld::gBases, tVsGlueModelToWorld::gMembers );


//
// tVsGlueModelToView
const ::Sig::Rtti::tBaseClassDesc tVsGlueModelToView::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueModelToView, tShadeMaterialGlue >( "tVsGlueModelToView", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueModelToView::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueModelToView, u32 >( "tVsGlueModelToView", "u32", "mRegister", "public", offsetof( tVsGlueModelToView, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueModelToView::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueModelToView >( "tVsGlueModelToView", tVsGlueModelToView::gBases, tVsGlueModelToView::gMembers );


//
// tVsGlueWorldToProjection
const ::Sig::Rtti::tBaseClassDesc tVsGlueWorldToProjection::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueWorldToProjection, tShadeMaterialGlue >( "tVsGlueWorldToProjection", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueWorldToProjection::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueWorldToProjection, u32 >( "tVsGlueWorldToProjection", "u32", "mRegister", "public", offsetof( tVsGlueWorldToProjection, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueWorldToProjection::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueWorldToProjection >( "tVsGlueWorldToProjection", tVsGlueWorldToProjection::gBases, tVsGlueWorldToProjection::gMembers );


//
// tVsGlueViewToProjection
const ::Sig::Rtti::tBaseClassDesc tVsGlueViewToProjection::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueViewToProjection, tShadeMaterialGlue >( "tVsGlueViewToProjection", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueViewToProjection::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueViewToProjection, u32 >( "tVsGlueViewToProjection", "u32", "mRegister", "public", offsetof( tVsGlueViewToProjection, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueViewToProjection::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueViewToProjection >( "tVsGlueViewToProjection", tVsGlueViewToProjection::gBases, tVsGlueViewToProjection::gMembers );


//
// tVsGlueViewToWorld
const ::Sig::Rtti::tBaseClassDesc tVsGlueViewToWorld::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueViewToWorld, tShadeMaterialGlue >( "tVsGlueViewToWorld", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueViewToWorld::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueViewToWorld, u32 >( "tVsGlueViewToWorld", "u32", "mRegister", "public", offsetof( tVsGlueViewToWorld, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueViewToWorld::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueViewToWorld >( "tVsGlueViewToWorld", tVsGlueViewToWorld::gBases, tVsGlueViewToWorld::gMembers );


//
// tVsGlueWorldEyePos
const ::Sig::Rtti::tBaseClassDesc tVsGlueWorldEyePos::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueWorldEyePos, tShadeMaterialGlue >( "tVsGlueWorldEyePos", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueWorldEyePos::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueWorldEyePos, u32 >( "tVsGlueWorldEyePos", "u32", "mRegister", "public", offsetof( tVsGlueWorldEyePos, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueWorldEyePos::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueWorldEyePos >( "tVsGlueWorldEyePos", tVsGlueWorldEyePos::gBases, tVsGlueWorldEyePos::gMembers );


//
// tVsGlueWorldToLight
const ::Sig::Rtti::tBaseClassDesc tVsGlueWorldToLight::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueWorldToLight, tShadeMaterialGlue >( "tVsGlueWorldToLight", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueWorldToLight::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueWorldToLight, u32 >( "tVsGlueWorldToLight", "u32", "mRegister", "public", offsetof( tVsGlueWorldToLight, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueWorldToLight::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueWorldToLight >( "tVsGlueWorldToLight", tVsGlueWorldToLight::gBases, tVsGlueWorldToLight::gMembers );


//
// tVsGlueViewToLight
const ::Sig::Rtti::tBaseClassDesc tVsGlueViewToLight::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueViewToLight, tShadeMaterialGlue >( "tVsGlueViewToLight", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueViewToLight::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueViewToLight, u32 >( "tVsGlueViewToLight", "u32", "mRegister", "public", offsetof( tVsGlueViewToLight, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueViewToLight::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueViewToLight >( "tVsGlueViewToLight", tVsGlueViewToLight::gBases, tVsGlueViewToLight::gMembers );


//
// tVsGlueSkinningPalette
const ::Sig::Rtti::tBaseClassDesc tVsGlueSkinningPalette::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVsGlueSkinningPalette, tShadeMaterialGlue >( "tVsGlueSkinningPalette", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVsGlueSkinningPalette::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVsGlueSkinningPalette, u32 >( "tVsGlueSkinningPalette", "u32", "mRegister", "public", offsetof( tVsGlueSkinningPalette, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVsGlueSkinningPalette::gReflector = ::Sig::Rtti::fDefineReflector< tVsGlueSkinningPalette >( "tVsGlueSkinningPalette", tVsGlueSkinningPalette::gBases, tVsGlueSkinningPalette::gMembers );


//
// tPsGlueBackFacePlusShadowMapParams
const ::Sig::Rtti::tBaseClassDesc tPsGlueBackFacePlusShadowMapParams::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueBackFacePlusShadowMapParams, tShadeMaterialGlue >( "tPsGlueBackFacePlusShadowMapParams", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueBackFacePlusShadowMapParams::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueBackFacePlusShadowMapParams, u32 >( "tPsGlueBackFacePlusShadowMapParams", "u32", "mRegister", "public", offsetof( tPsGlueBackFacePlusShadowMapParams, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueBackFacePlusShadowMapParams::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueBackFacePlusShadowMapParams >( "tPsGlueBackFacePlusShadowMapParams", tPsGlueBackFacePlusShadowMapParams::gBases, tPsGlueBackFacePlusShadowMapParams::gMembers );


//
// tPsGlueShadowMapTargetPos
const ::Sig::Rtti::tBaseClassDesc tPsGlueShadowMapTargetPos::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueShadowMapTargetPos, tShadeMaterialGlue >( "tPsGlueShadowMapTargetPos", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueShadowMapTargetPos::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueShadowMapTargetPos, u32 >( "tPsGlueShadowMapTargetPos", "u32", "mRegister", "public", offsetof( tPsGlueShadowMapTargetPos, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueShadowMapTargetPos::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueShadowMapTargetPos >( "tPsGlueShadowMapTargetPos", tPsGlueShadowMapTargetPos::gBases, tPsGlueShadowMapTargetPos::gMembers );


//
// tPsGlueShadowMapSplits
const ::Sig::Rtti::tBaseClassDesc tPsGlueShadowMapSplits::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueShadowMapSplits, tShadeMaterialGlue >( "tPsGlueShadowMapSplits", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueShadowMapSplits::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueShadowMapSplits, u32 >( "tPsGlueShadowMapSplits", "u32", "mRegister", "public", offsetof( tPsGlueShadowMapSplits, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueShadowMapSplits::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueShadowMapSplits >( "tPsGlueShadowMapSplits", tPsGlueShadowMapSplits::gBases, tPsGlueShadowMapSplits::gMembers );


//
// tPsGlueWorldToLightSpaceArray
const ::Sig::Rtti::tBaseClassDesc tPsGlueWorldToLightSpaceArray::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueWorldToLightSpaceArray, tShadeMaterialGlue >( "tPsGlueWorldToLightSpaceArray", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueWorldToLightSpaceArray::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueWorldToLightSpaceArray, u32 >( "tPsGlueWorldToLightSpaceArray", "u32", "mRegister", "public", offsetof( tPsGlueWorldToLightSpaceArray, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueWorldToLightSpaceArray::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueWorldToLightSpaceArray >( "tPsGlueWorldToLightSpaceArray", tPsGlueWorldToLightSpaceArray::gBases, tPsGlueWorldToLightSpaceArray::gMembers );


//
// tPsGlueFog
const ::Sig::Rtti::tBaseClassDesc tPsGlueFog::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueFog, tShadeMaterialGlue >( "tPsGlueFog", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueFog::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueFog, u16 >( "tPsGlueFog", "u16", "mPRegister", "public", offsetof( tPsGlueFog, mPRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueFog, u16 >( "tPsGlueFog", "u16", "mCRegister", "public", offsetof( tPsGlueFog, mCRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueFog::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueFog >( "tPsGlueFog", tPsGlueFog::gBases, tPsGlueFog::gMembers );


//
// tPsGlueFlatParticleColor
const ::Sig::Rtti::tBaseClassDesc tPsGlueFlatParticleColor::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueFlatParticleColor, tShadeMaterialGlue >( "tPsGlueFlatParticleColor", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueFlatParticleColor::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueFlatParticleColor, u32 >( "tPsGlueFlatParticleColor", "u32", "mRegister", "public", offsetof( tPsGlueFlatParticleColor, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueFlatParticleColor::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueFlatParticleColor >( "tPsGlueFlatParticleColor", tPsGlueFlatParticleColor::gBases, tPsGlueFlatParticleColor::gMembers );


//
// tPsGlueTime
const ::Sig::Rtti::tBaseClassDesc tPsGlueTime::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueTime, tShadeMaterialGlue >( "tPsGlueTime", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueTime::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueTime, u32 >( "tPsGlueTime", "u32", "mRegister", "public", offsetof( tPsGlueTime, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueTime::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueTime >( "tPsGlueTime", tPsGlueTime::gBases, tPsGlueTime::gMembers );


//
// tPsGlueInstanceTint
const ::Sig::Rtti::tBaseClassDesc tPsGlueInstanceTint::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueInstanceTint, tShadeMaterialGlue >( "tPsGlueInstanceTint", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueInstanceTint::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueInstanceTint, u32 >( "tPsGlueInstanceTint", "u32", "mRegister", "public", offsetof( tPsGlueInstanceTint, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueInstanceTint::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueInstanceTint >( "tPsGlueInstanceTint", tPsGlueInstanceTint::gBases, tPsGlueInstanceTint::gMembers );


//
// tPsGlueDynamicVec4
const ::Sig::Rtti::tBaseClassDesc tPsGlueDynamicVec4::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueDynamicVec4, tShadeMaterialGlue >( "tPsGlueDynamicVec4", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueDynamicVec4::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueDynamicVec4, u16 >( "tPsGlueDynamicVec4", "u16", "mRegister", "public", offsetof( tPsGlueDynamicVec4, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueDynamicVec4, u16 >( "tPsGlueDynamicVec4", "u16", "mNameIndex", "public", offsetof( tPsGlueDynamicVec4, mNameIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueDynamicVec4::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueDynamicVec4 >( "tPsGlueDynamicVec4", tPsGlueDynamicVec4::gBases, tPsGlueDynamicVec4::gMembers );


//
// tPsGlueRimLightParams
const ::Sig::Rtti::tBaseClassDesc tPsGlueRimLightParams::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueRimLightParams, tShadeMaterialGlue >( "tPsGlueRimLightParams", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueRimLightParams::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueRimLightParams, u32 >( "tPsGlueRimLightParams", "u32", "mRegister", "public", offsetof( tPsGlueRimLightParams, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueRimLightParams::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueRimLightParams >( "tPsGlueRimLightParams", tPsGlueRimLightParams::gBases, tPsGlueRimLightParams::gMembers );


//
// tPsGlueLightParams
const ::Sig::Rtti::tBaseClassDesc tPsGlueLightParams::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueLightParams, tShadeMaterialGlue >( "tPsGlueLightParams", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueLightParams::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueLightParams, u16 >( "tPsGlueLightParams", "u16", "mRegister", "public", offsetof( tPsGlueLightParams, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueLightParams, u16 >( "tPsGlueLightParams", "u16", "mNumLights", "public", offsetof( tPsGlueLightParams, mNumLights ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueLightParams::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueLightParams >( "tPsGlueLightParams", tPsGlueLightParams::gBases, tPsGlueLightParams::gMembers );


//
// tPsGlueShadowMap
const ::Sig::Rtti::tBaseClassDesc tPsGlueShadowMap::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueShadowMap, tShadeMaterialGlue >( "tPsGlueShadowMap", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueShadowMap::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueShadowMap, u32 >( "tPsGlueShadowMap", "u32", "mRegister", "public", offsetof( tPsGlueShadowMap, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueShadowMap::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueShadowMap >( "tPsGlueShadowMap", tPsGlueShadowMap::gBases, tPsGlueShadowMap::gMembers );


//
// tPsGlueWorldEyePos
const ::Sig::Rtti::tBaseClassDesc tPsGlueWorldEyePos::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueWorldEyePos, tShadeMaterialGlue >( "tPsGlueWorldEyePos", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueWorldEyePos::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueWorldEyePos, u32 >( "tPsGlueWorldEyePos", "u32", "mRegister", "public", offsetof( tPsGlueWorldEyePos, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueWorldEyePos::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueWorldEyePos >( "tPsGlueWorldEyePos", tPsGlueWorldEyePos::gBases, tPsGlueWorldEyePos::gMembers );


//
// tPsGlueMaterialVector
const ::Sig::Rtti::tBaseClassDesc tPsGlueMaterialVector::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueMaterialVector, tShadeMaterialGlue >( "tPsGlueMaterialVector", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueMaterialVector::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueMaterialVector, u16 >( "tPsGlueMaterialVector", "u16", "mRegister", "public", offsetof( tPsGlueMaterialVector, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueMaterialVector, u16 >( "tPsGlueMaterialVector", "u16", "mVectorIndex", "public", offsetof( tPsGlueMaterialVector, mVectorIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueMaterialVector::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueMaterialVector >( "tPsGlueMaterialVector", tPsGlueMaterialVector::gBases, tPsGlueMaterialVector::gMembers );


//
// tPsGlueMaterialSampler
const ::Sig::Rtti::tBaseClassDesc tPsGlueMaterialSampler::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPsGlueMaterialSampler, tShadeMaterialGlue >( "tPsGlueMaterialSampler", "tShadeMaterialGlue", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPsGlueMaterialSampler::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueMaterialSampler, u16 >( "tPsGlueMaterialSampler", "u16", "mRegister", "public", offsetof( tPsGlueMaterialSampler, mRegister ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPsGlueMaterialSampler, u16 >( "tPsGlueMaterialSampler", "u16", "mSamplerIndex", "public", offsetof( tPsGlueMaterialSampler, mSamplerIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPsGlueMaterialSampler::gReflector = ::Sig::Rtti::fDefineReflector< tPsGlueMaterialSampler >( "tPsGlueMaterialSampler", tPsGlueMaterialSampler::gBases, tPsGlueMaterialSampler::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tDecalMaterial.hpp]
//
#include "Gfx/tDecalMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tDecalRenderVertex
const ::Sig::Rtti::tBaseClassDesc tDecalRenderVertex::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDecalRenderVertex::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tDecalRenderVertex, Math::tVec3f >( "tDecalRenderVertex", "Math::tVec3f", "mP", "public", offsetof( tDecalRenderVertex, mP ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalRenderVertex, Math::tVec3f >( "tDecalRenderVertex", "Math::tVec3f", "mN", "public", offsetof( tDecalRenderVertex, mN ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalRenderVertex, Math::tVec4f >( "tDecalRenderVertex", "Math::tVec4f", "mTan", "public", offsetof( tDecalRenderVertex, mTan ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalRenderVertex, Math::tVec2f >( "tDecalRenderVertex", "Math::tVec2f", "mUv", "public", offsetof( tDecalRenderVertex, mUv ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalRenderVertex, u32 >( "tDecalRenderVertex", "u32", "mColor", "public", offsetof( tDecalRenderVertex, mColor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDecalRenderVertex::gReflector = ::Sig::Rtti::fDefineReflector< tDecalRenderVertex >( "tDecalRenderVertex", tDecalRenderVertex::gBases, tDecalRenderVertex::gMembers );


//
// tDecalMaterial
const ::Sig::Rtti::tBaseClassDesc tDecalMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDecalMaterial, tMaterial >( "tDecalMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDecalMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tDecalMaterial, tEnum<tVS,u16> >( "tDecalMaterial", "tEnum<tVS,u16>", "mVS", "public", offsetof( tDecalMaterial, mVS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalMaterial, tEnum<tPS,u16> >( "tDecalMaterial", "tEnum<tPS,u16>", "mPS", "public", offsetof( tDecalMaterial, mPS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalMaterial, tTextureReference >( "tDecalMaterial", "tTextureReference", "mDiffuseMap", "public", offsetof( tDecalMaterial, mDiffuseMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalMaterial, tTextureReference >( "tDecalMaterial", "tTextureReference", "mNormalMap", "public", offsetof( tDecalMaterial, mNormalMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalMaterial, Math::tVec4f >( "tDecalMaterial", "Math::tVec4f", "mDiffuseUvXform", "public", offsetof( tDecalMaterial, mDiffuseUvXform ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalMaterial, Math::tVec4f >( "tDecalMaterial", "Math::tVec4f", "mNormalUvXform", "public", offsetof( tDecalMaterial, mNormalUvXform ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tDecalMaterial, Math::tVec4f >( "tDecalMaterial", "Math::tVec4f", "mBumpDepth_SpecSize_Opacity_BackFaceFlip", "public", offsetof( tDecalMaterial, mBumpDepth_SpecSize_Opacity_BackFaceFlip ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDecalMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tDecalMaterial >( "tDecalMaterial", tDecalMaterial::gBases, tDecalMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tDefaultMaterial.hpp]
//
#include "Gfx/tDefaultMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tDefaultMaterial
const ::Sig::Rtti::tBaseClassDesc tDefaultMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDefaultMaterial, tMaterial >( "tDefaultMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tDefaultMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tDefaultMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tDefaultMaterial >( "tDefaultMaterial", tDefaultMaterial::gBases, tDefaultMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tFontMaterial.hpp]
//
#include "Gfx/tFontMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tFontMaterial
const ::Sig::Rtti::tBaseClassDesc tFontMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tFontMaterial, tMaterial >( "tFontMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFontMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFontMaterial, tEnum<tVSShaderSlot,u16> >( "tFontMaterial", "tEnum<tVSShaderSlot,u16>", "mVsSlot", "public", offsetof( tFontMaterial, mVsSlot ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontMaterial, tEnum<tPSShaderSlot,u16> >( "tFontMaterial", "tEnum<tPSShaderSlot,u16>", "mPsSlot", "public", offsetof( tFontMaterial, mPsSlot ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFontMaterial, tLoadInPlaceResourcePtr* >( "tFontMaterial", "tLoadInPlaceResourcePtr*", "mFontMap", "public", offsetof( tFontMaterial, mFontMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFontMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tFontMaterial >( "tFontMaterial", tFontMaterial::gBases, tFontMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tFullBrightMaterial.hpp]
//
#include "Gfx/tFullBrightMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tFullBrightMaterial
const ::Sig::Rtti::tBaseClassDesc tFullBrightMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tFullBrightMaterial, tMaterial >( "tFullBrightMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFullBrightMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFullBrightMaterial, tEnum<tShaderSlots,u16> >( "tFullBrightMaterial", "tEnum<tShaderSlots,u16>", "mShaderSlotVS", "public", offsetof( tFullBrightMaterial, mShaderSlotVS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFullBrightMaterial, tEnum<tShaderSlots,u16> >( "tFullBrightMaterial", "tEnum<tShaderSlots,u16>", "mShaderSlotPS", "public", offsetof( tFullBrightMaterial, mShaderSlotPS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFullBrightMaterial, tTextureReference >( "tFullBrightMaterial", "tTextureReference", "mColorMap", "public", offsetof( tFullBrightMaterial, mColorMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFullBrightMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tFullBrightMaterial >( "tFullBrightMaterial", tFullBrightMaterial::gBases, tFullBrightMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tGeometryBufferVRam.hpp]
//
#include "Gfx/tGeometryBufferVRam.hpp"
namespace Sig
{
namespace Gfx
{

//
// tGeometryBufferVRam
const ::Sig::Rtti::tBaseClassDesc tGeometryBufferVRam::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tGeometryBufferVRam::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryBufferVRam, tVertexFormatVRam >( "tGeometryBufferVRam", "tVertexFormatVRam", "mFormat", "public", offsetof( tGeometryBufferVRam, mFormat ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryBufferVRam, u32 >( "tGeometryBufferVRam", "u32", "mNumVerts", "public", offsetof( tGeometryBufferVRam, mNumVerts ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryBufferVRam, u32 >( "tGeometryBufferVRam", "u32", "mAllocFlags", "public", offsetof( tGeometryBufferVRam, mAllocFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryBufferVRam, tPlatformHandle >( "tGeometryBufferVRam", "tPlatformHandle", "mPlatformHandle", "public", offsetof( tGeometryBufferVRam, mPlatformHandle ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryBufferVRam, tPlatformHandle >( "tGeometryBufferVRam", "tPlatformHandle", "mDeviceResource", "public", offsetof( tGeometryBufferVRam, mDeviceResource ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryBufferVRam, Sig::byte* >( "tGeometryBufferVRam", "Sig::byte*", "mPermaLockAddress", "public", offsetof( tGeometryBufferVRam, mPermaLockAddress ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tGeometryBufferVRam::gReflector = ::Sig::Rtti::fDefineReflector< tGeometryBufferVRam >( "tGeometryBufferVRam", tGeometryBufferVRam::gBases, tGeometryBufferVRam::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tGeometryFile.hpp]
//
#include "Gfx/tGeometryFile.hpp"
namespace Sig
{
namespace Gfx
{

//
// tGeometryFile::tBufferPointer
const ::Sig::Rtti::tBaseClassDesc tGeometryFile::tBufferPointer::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tGeometryFile::tBufferPointer::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryFile::tBufferPointer, u32 >( "tGeometryFile::tBufferPointer", "u32", "mBufferOffset", "public", offsetof( tGeometryFile::tBufferPointer, mBufferOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryFile::tBufferPointer, u32 >( "tGeometryFile::tBufferPointer", "u32", "mBufferSize", "public", offsetof( tGeometryFile::tBufferPointer, mBufferSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tGeometryFile::tBufferPointer::gReflector = ::Sig::Rtti::fDefineReflector< tGeometryFile::tBufferPointer >( "tGeometryFile::tBufferPointer", tGeometryFile::tBufferPointer::gBases, tGeometryFile::tBufferPointer::gMembers );


//
// tGeometryFile::tGeometryPointer
const ::Sig::Rtti::tBaseClassDesc tGeometryFile::tGeometryPointer::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tGeometryFile::tGeometryPointer, tBufferPointer >( "tGeometryFile::tGeometryPointer", "tBufferPointer", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tGeometryFile::tGeometryPointer::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryFile::tGeometryPointer, tGeometryBufferVRam >( "tGeometryFile::tGeometryPointer", "tGeometryBufferVRam", "mVRamBuffer", "public", offsetof( tGeometryFile::tGeometryPointer, mVRamBuffer ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tGeometryFile::tGeometryPointer::gReflector = ::Sig::Rtti::fDefineReflector< tGeometryFile::tGeometryPointer >( "tGeometryFile::tGeometryPointer", tGeometryFile::tGeometryPointer::gBases, tGeometryFile::tGeometryPointer::gMembers );


//
// tGeometryFile::tIndexListPointer
const ::Sig::Rtti::tBaseClassDesc tGeometryFile::tIndexListPointer::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tGeometryFile::tIndexListPointer, tBufferPointer >( "tGeometryFile::tIndexListPointer", "tBufferPointer", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tGeometryFile::tIndexListPointer::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryFile::tIndexListPointer, tIndexBufferVRam >( "tGeometryFile::tIndexListPointer", "tIndexBufferVRam", "mVRamBuffer", "public", offsetof( tGeometryFile::tIndexListPointer, mVRamBuffer ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tGeometryFile::tIndexListPointer::gReflector = ::Sig::Rtti::fDefineReflector< tGeometryFile::tIndexListPointer >( "tGeometryFile::tIndexListPointer", tGeometryFile::tIndexListPointer::gBases, tGeometryFile::tIndexListPointer::gMembers );


//
// tGeometryFile
const ::Sig::Rtti::tBaseClassDesc tGeometryFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tGeometryFile, tLoadInPlaceFileBase >( "tGeometryFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tGeometryFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryFile, u32 >( "tGeometryFile", "u32", "mHeaderSize", "public", offsetof( tGeometryFile, mHeaderSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryFile, b32 >( "tGeometryFile", "b32", "mDiscardSysRamBuffers", "public", offsetof( tGeometryFile, mDiscardSysRamBuffers ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryFile, tDynamicArray<tGeometryPointer> >( "tGeometryFile", "tDynamicArray<tGeometryPointer>", "mGeometryPointers", "public", offsetof( tGeometryFile, mGeometryPointers ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGeometryFile, tDynamicArray<tIndexListPointer> >( "tGeometryFile", "tDynamicArray<tIndexListPointer>", "mIndexListPointers", "public", offsetof( tGeometryFile, mIndexListPointers ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tGeometryFile::gReflector = ::Sig::Rtti::fDefineReflector< tGeometryFile >( "tGeometryFile", tGeometryFile::gBases, tGeometryFile::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tGroundCoverCloud.hpp]
//
#include "Gfx/tGroundCoverCloud.hpp"
namespace Sig
{
namespace Gfx
{

//
// tGroundCoverCloudDef::tElement
const ::Sig::Rtti::tBaseClassDesc tGroundCoverCloudDef::tElement::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tGroundCoverCloudDef::tElement::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef::tElement, tLoadInPlaceResourcePtr* >( "tGroundCoverCloudDef::tElement", "tLoadInPlaceResourcePtr*", "mSgFile", "public", offsetof( tGroundCoverCloudDef::tElement, mSgFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef::tElement, b32 >( "tGroundCoverCloudDef::tElement", "b32", "mCastsShadow", "public", offsetof( tGroundCoverCloudDef::tElement, mCastsShadow ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef::tElement, f32 >( "tGroundCoverCloudDef::tElement", "f32", "mFrequency", "public", offsetof( tGroundCoverCloudDef::tElement, mFrequency ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef::tElement, u32 >( "tGroundCoverCloudDef::tElement", "u32", "mSpawnCount", "public", offsetof( tGroundCoverCloudDef::tElement, mSpawnCount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tGroundCoverCloudDef::tElement::gReflector = ::Sig::Rtti::fDefineReflector< tGroundCoverCloudDef::tElement >( "tGroundCoverCloudDef::tElement", tGroundCoverCloudDef::tElement::gBases, tGroundCoverCloudDef::tElement::gMembers );


//
// tGroundCoverCloudDef
const ::Sig::Rtti::tBaseClassDesc tGroundCoverCloudDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tGroundCoverCloudDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, f32 >( "tGroundCoverCloudDef", "f32", "mUnitSize", "public", offsetof( tGroundCoverCloudDef, mUnitSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, u32 >( "tGroundCoverCloudDef", "u32", "mPaintUnits", "public", offsetof( tGroundCoverCloudDef, mPaintUnits ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, u32 >( "tGroundCoverCloudDef", "u32", "mDimX", "public", offsetof( tGroundCoverCloudDef, mDimX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, u32 >( "tGroundCoverCloudDef", "u32", "mDimZ", "public", offsetof( tGroundCoverCloudDef, mDimZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, u32 >( "tGroundCoverCloudDef", "u32", "mMaskDimX", "public", offsetof( tGroundCoverCloudDef, mMaskDimX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, u32 >( "tGroundCoverCloudDef", "u32", "mMaskDimZ", "public", offsetof( tGroundCoverCloudDef, mMaskDimZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, u32 >( "tGroundCoverCloudDef", "u32", "mMaxUnitSpawns", "public", offsetof( tGroundCoverCloudDef, mMaxUnitSpawns ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, tEnum<tRotation,byte> >( "tGroundCoverCloudDef", "tEnum<tRotation,byte>", "mRotation", "public", offsetof( tGroundCoverCloudDef, mRotation ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, tEnum<tTranslation,byte> >( "tGroundCoverCloudDef", "tEnum<tTranslation,byte>", "mTranslation", "public", offsetof( tGroundCoverCloudDef, mTranslation ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, tEnum<tVisibility,byte> >( "tGroundCoverCloudDef", "tEnum<tVisibility,byte>", "mVisibility", "public", offsetof( tGroundCoverCloudDef, mVisibility ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, f32 >( "tGroundCoverCloudDef", "f32", "mYRotationScale", "public", offsetof( tGroundCoverCloudDef, mYRotationScale ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, f32 >( "tGroundCoverCloudDef", "f32", "mXZRotationScale", "public", offsetof( tGroundCoverCloudDef, mXZRotationScale ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, f32 >( "tGroundCoverCloudDef", "f32", "mXZTranslationScale", "public", offsetof( tGroundCoverCloudDef, mXZTranslationScale ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, f32 >( "tGroundCoverCloudDef", "f32", "mYTranslationScale", "public", offsetof( tGroundCoverCloudDef, mYTranslationScale ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, f32 >( "tGroundCoverCloudDef", "f32", "mScaleRangeAdjustor", "public", offsetof( tGroundCoverCloudDef, mScaleRangeAdjustor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, f32 >( "tGroundCoverCloudDef", "f32", "mWorldLengthX", "public", offsetof( tGroundCoverCloudDef, mWorldLengthX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, f32 >( "tGroundCoverCloudDef", "f32", "mWorldLengthZ", "public", offsetof( tGroundCoverCloudDef, mWorldLengthZ ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, tDynamicArray<tElement> >( "tGroundCoverCloudDef", "tDynamicArray<tElement>", "mElements", "public", offsetof( tGroundCoverCloudDef, mElements ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, tDynamicArray<byte> >( "tGroundCoverCloudDef", "tDynamicArray<byte>", "mMask", "public", offsetof( tGroundCoverCloudDef, mMask ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tGroundCoverCloudDef, tDynamicArray<f32> >( "tGroundCoverCloudDef", "tDynamicArray<f32>", "mHeights", "public", offsetof( tGroundCoverCloudDef, mHeights ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tGroundCoverCloudDef::gReflector = ::Sig::Rtti::fDefineReflector< tGroundCoverCloudDef >( "tGroundCoverCloudDef", tGroundCoverCloudDef::gBases, tGroundCoverCloudDef::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tHeightFieldMaterial.hpp]
//
#include "Gfx/tHeightFieldMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tHeightFieldMaterial::tLightingPass
const ::Sig::Rtti::tBaseClassDesc tHeightFieldMaterial::tLightingPass::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldMaterial::tLightingPass::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial::tLightingPass, tEnum<tVS,u16> >( "tHeightFieldMaterial::tLightingPass", "tEnum<tVS,u16>", "mVS", "public", offsetof( tHeightFieldMaterial::tLightingPass, mVS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial::tLightingPass, tEnum<tPS,u16> >( "tHeightFieldMaterial::tLightingPass", "tEnum<tPS,u16>", "mPS", "public", offsetof( tHeightFieldMaterial::tLightingPass, mPS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldMaterial::tLightingPass::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldMaterial::tLightingPass >( "tHeightFieldMaterial::tLightingPass", tHeightFieldMaterial::tLightingPass::gBases, tHeightFieldMaterial::tLightingPass::gMembers );


//
// tHeightFieldMaterial::tShadowMapPass
const ::Sig::Rtti::tBaseClassDesc tHeightFieldMaterial::tShadowMapPass::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldMaterial::tShadowMapPass::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial::tShadowMapPass, tEnum<tVS,u16> >( "tHeightFieldMaterial::tShadowMapPass", "tEnum<tVS,u16>", "mVS", "public", offsetof( tHeightFieldMaterial::tShadowMapPass, mVS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial::tShadowMapPass, tEnum<tPS,u16> >( "tHeightFieldMaterial::tShadowMapPass", "tEnum<tPS,u16>", "mPS", "public", offsetof( tHeightFieldMaterial::tShadowMapPass, mPS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldMaterial::tShadowMapPass::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldMaterial::tShadowMapPass >( "tHeightFieldMaterial::tShadowMapPass", tHeightFieldMaterial::tShadowMapPass::gBases, tHeightFieldMaterial::tShadowMapPass::gMembers );


//
// tHeightFieldMaterial
const ::Sig::Rtti::tBaseClassDesc tHeightFieldMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tHeightFieldMaterial, tMaterial >( "tHeightFieldMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tHeightFieldMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, u32 >( "tHeightFieldMaterial", "u32", "mVertexFormatSlot", "public", offsetof( tHeightFieldMaterial, mVertexFormatSlot ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, tLightingPass >( "tHeightFieldMaterial", "tLightingPass", "mLightingPass", "public", offsetof( tHeightFieldMaterial, mLightingPass ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, tShadowMapPass >( "tHeightFieldMaterial", "tShadowMapPass", "mShadowMapPass", "public", offsetof( tHeightFieldMaterial, mShadowMapPass ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, tTextureReference >( "tHeightFieldMaterial", "tTextureReference", "mMaskMap", "public", offsetof( tHeightFieldMaterial, mMaskMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, tTextureReference >( "tHeightFieldMaterial", "tTextureReference", "mMtlIdsMap", "public", offsetof( tHeightFieldMaterial, mMtlIdsMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, tTextureReference >( "tHeightFieldMaterial", "tTextureReference", "mDiffuseMap", "public", offsetof( tHeightFieldMaterial, mDiffuseMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, tTextureReference >( "tHeightFieldMaterial", "tTextureReference", "mNormalMap", "public", offsetof( tHeightFieldMaterial, mNormalMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, Math::tVec2f >( "tHeightFieldMaterial", "Math::tVec2f", "mWorldSpaceDims", "public", offsetof( tHeightFieldMaterial, mWorldSpaceDims ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, Math::tVec2u >( "tHeightFieldMaterial", "Math::tVec2u", "mTextureAtlasDims", "public", offsetof( tHeightFieldMaterial, mTextureAtlasDims ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, Math::tVec2f >( "tHeightFieldMaterial", "Math::tVec2f", "mSubDiffuseRectDims", "public", offsetof( tHeightFieldMaterial, mSubDiffuseRectDims ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, Math::tVec2f >( "tHeightFieldMaterial", "Math::tVec2f", "mSubNormalRectDims", "public", offsetof( tHeightFieldMaterial, mSubNormalRectDims ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tHeightFieldMaterial, tFixedArray<Math::tNoOpVec4f,8> >( "tHeightFieldMaterial", "tFixedArray<Math::tNoOpVec4f,8>", "mTileFactors", "public", offsetof( tHeightFieldMaterial, mTileFactors ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tHeightFieldMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tHeightFieldMaterial >( "tHeightFieldMaterial", tHeightFieldMaterial::gBases, tHeightFieldMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tIndexBufferVRam.hpp]
//
#include "Gfx/tIndexBufferVRam.hpp"
namespace Sig
{
namespace Gfx
{

//
// tIndexBufferVRam
const ::Sig::Rtti::tBaseClassDesc tIndexBufferVRam::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tIndexBufferVRam::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tIndexBufferVRam, tIndexFormat >( "tIndexBufferVRam", "tIndexFormat", "mFormat", "public", offsetof( tIndexBufferVRam, mFormat ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexBufferVRam, u32 >( "tIndexBufferVRam", "u32", "mNumIndices", "public", offsetof( tIndexBufferVRam, mNumIndices ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexBufferVRam, u32 >( "tIndexBufferVRam", "u32", "mNumPrimitives", "public", offsetof( tIndexBufferVRam, mNumPrimitives ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexBufferVRam, u32 >( "tIndexBufferVRam", "u32", "mAllocFlags", "public", offsetof( tIndexBufferVRam, mAllocFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexBufferVRam, tPlatformHandle >( "tIndexBufferVRam", "tPlatformHandle", "mPlatformHandle", "public", offsetof( tIndexBufferVRam, mPlatformHandle ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexBufferVRam, tPlatformHandle >( "tIndexBufferVRam", "tPlatformHandle", "mDeviceResource", "public", offsetof( tIndexBufferVRam, mDeviceResource ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexBufferVRam, Sig::byte* >( "tIndexBufferVRam", "Sig::byte*", "mPermaLockAddress", "public", offsetof( tIndexBufferVRam, mPermaLockAddress ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tIndexBufferVRam::gReflector = ::Sig::Rtti::fDefineReflector< tIndexBufferVRam >( "tIndexBufferVRam", tIndexBufferVRam::gBases, tIndexBufferVRam::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tIndexFormat.hpp]
//
#include "Gfx/tIndexFormat.hpp"
namespace Sig
{
namespace Gfx
{

//
// tIndexFormat
const ::Sig::Rtti::tBaseClassDesc tIndexFormat::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tIndexFormat::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tIndexFormat, tEnum<tStorageType,u8> >( "tIndexFormat", "tEnum<tStorageType,u8>", "mStorageType", "public", offsetof( tIndexFormat, mStorageType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexFormat, tEnum<tPrimitiveType,u8> >( "tIndexFormat", "tEnum<tPrimitiveType,u8>", "mPrimitiveType", "public", offsetof( tIndexFormat, mPrimitiveType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexFormat, u16 >( "tIndexFormat", "u16", "mSize", "public", offsetof( tIndexFormat, mSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tIndexFormat, u32 >( "tIndexFormat", "u32", "mMaxValue", "public", offsetof( tIndexFormat, mMaxValue ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tIndexFormat::gReflector = ::Sig::Rtti::fDefineReflector< tIndexFormat >( "tIndexFormat", tIndexFormat::gBases, tIndexFormat::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tLight.hpp]
//
#include "Gfx/tLight.hpp"
namespace Sig
{
namespace Gfx
{

//
// tLight
const ::Sig::Rtti::tBaseClassDesc tLight::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLight::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLight, tFixedArray<Math::tNoOpVec4f,cColorTypeCount> >( "tLight", "tFixedArray<Math::tNoOpVec4f,cColorTypeCount>", "mColors", "public", offsetof( tLight, mColors ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLight, Math::tVec2f >( "tLight", "Math::tVec2f", "mRadii", "public", offsetof( tLight, mRadii ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLight, Math::tVec2f >( "tLight", "Math::tVec2f", "mAngles", "public", offsetof( tLight, mAngles ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tLight, tEnum<tLightType,u32> >( "tLight", "tEnum<tLightType,u32>", "mLightType", "public", offsetof( tLight, mLightType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLight::gReflector = ::Sig::Rtti::fDefineReflector< tLight >( "tLight", tLight::gBases, tLight::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tLightEntity.hpp]
//
#include "Gfx/tLightEntity.hpp"
namespace Sig
{
namespace Gfx
{

//
// tLightEntityDef
const ::Sig::Rtti::tBaseClassDesc tLightEntityDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tLightEntityDef, tEntityDef >( "tLightEntityDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tLightEntityDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLightEntityDef, tLight >( "tLightEntityDef", "tLight", "mLightDesc", "public", offsetof( tLightEntityDef, mLightDesc ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tLightEntityDef::gReflector = ::Sig::Rtti::fDefineReflector< tLightEntityDef >( "tLightEntityDef", tLightEntityDef::gBases, tLightEntityDef::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tMaterial.hpp]
//
#include "Gfx/tMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tMaterial
const ::Sig::Rtti::tBaseClassDesc tMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tMaterial, Rtti::tSerializableBaseClass >( "tMaterial", "Rtti::tSerializableBaseClass", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc< tMaterial, tUncopyable >( "tMaterial", "tUncopyable", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc< tMaterial, tRefCounter >( "tMaterial", "tRefCounter", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMaterial, tLoadInPlaceResourcePtr* >( "tMaterial", "tLoadInPlaceResourcePtr*", "mMaterialFile", "public", offsetof( tMaterial, mMaterialFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterial, tRenderState >( "tMaterial", "tRenderState", "mRenderState", "public", offsetof( tMaterial, mRenderState ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterial, u32 >( "tMaterial", "u32", "mMaterialFlags", "public", offsetof( tMaterial, mMaterialFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tMaterial >( "tMaterial", tMaterial::gBases, tMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tMaterialFile.hpp]
//
#include "Gfx/tMaterialFile.hpp"
namespace Sig
{
namespace Gfx
{

//
// tShadeMaterialGlue
const ::Sig::Rtti::tBaseClassDesc tShadeMaterialGlue::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tShadeMaterialGlue, Rtti::tSerializableBaseClass >( "tShadeMaterialGlue", "Rtti::tSerializableBaseClass", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc< tShadeMaterialGlue, tUncopyable >( "tShadeMaterialGlue", "tUncopyable", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc< tShadeMaterialGlue, tRefCounter >( "tShadeMaterialGlue", "tRefCounter", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tShadeMaterialGlue::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tShadeMaterialGlue::gReflector = ::Sig::Rtti::fDefineReflector< tShadeMaterialGlue >( "tShadeMaterialGlue", tShadeMaterialGlue::gBases, tShadeMaterialGlue::gMembers );


//
// tMaterialFile::tShaderPointer
const ::Sig::Rtti::tBaseClassDesc tMaterialFile::tShaderPointer::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tMaterialFile::tShaderPointer::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile::tShaderPointer, tShadeMaterialGlueArray >( "tMaterialFile::tShaderPointer", "tShadeMaterialGlueArray", "mGlueShared", "public", offsetof( tMaterialFile::tShaderPointer, mGlueShared ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile::tShaderPointer, tShadeMaterialGlueArray >( "tMaterialFile::tShaderPointer", "tShadeMaterialGlueArray", "mGlueInstance", "public", offsetof( tMaterialFile::tShaderPointer, mGlueInstance ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile::tShaderPointer, tEnum<tShaderBufferType,u32> >( "tMaterialFile::tShaderPointer", "tEnum<tShaderBufferType,u32>", "mType", "public", offsetof( tMaterialFile::tShaderPointer, mType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile::tShaderPointer, u32 >( "tMaterialFile::tShaderPointer", "u32", "mBufferOffset", "public", offsetof( tMaterialFile::tShaderPointer, mBufferOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile::tShaderPointer, u32 >( "tMaterialFile::tShaderPointer", "u32", "mBufferSize", "public", offsetof( tMaterialFile::tShaderPointer, mBufferSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile::tShaderPointer, tShaderPlatformHandle >( "tMaterialFile::tShaderPointer", "tShaderPlatformHandle", "mPlatformHandle", "public", offsetof( tMaterialFile::tShaderPointer, mPlatformHandle ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tMaterialFile::tShaderPointer::gReflector = ::Sig::Rtti::fDefineReflector< tMaterialFile::tShaderPointer >( "tMaterialFile::tShaderPointer", tMaterialFile::tShaderPointer::gBases, tMaterialFile::tShaderPointer::gMembers );


//
// tMaterialFile
const ::Sig::Rtti::tBaseClassDesc tMaterialFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tMaterialFile, tLoadInPlaceFileBase >( "tMaterialFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tMaterialFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile, Rtti::tClassId >( "tMaterialFile", "Rtti::tClassId", "mMaterialCid", "public", offsetof( tMaterialFile, mMaterialCid ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile, u32 >( "tMaterialFile", "u32", "mHeaderSize", "public", offsetof( tMaterialFile, mHeaderSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile, b32 >( "tMaterialFile", "b32", "mDiscardShaderBuffers", "public", offsetof( tMaterialFile, mDiscardShaderBuffers ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMaterialFile, tDynamicArray<tShaderList> >( "tMaterialFile", "tDynamicArray<tShaderList>", "mShaderLists", "public", offsetof( tMaterialFile, mShaderLists ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tMaterialFile::gReflector = ::Sig::Rtti::fDefineReflector< tMaterialFile >( "tMaterialFile", tMaterialFile::gBases, tMaterialFile::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tParticleMaterial.hpp]
//
#include "Gfx/tParticleMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tParticleMaterial
const ::Sig::Rtti::tBaseClassDesc tParticleMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tParticleMaterial, tMaterial >( "tParticleMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tParticleMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tParticleMaterial, tTextureReference >( "tParticleMaterial", "tTextureReference", "mEmissiveMap", "public", offsetof( tParticleMaterial, mEmissiveMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tParticleMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tParticleMaterial >( "tParticleMaterial", tParticleMaterial::gBases, tParticleMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tPhongMaterial.hpp]
//
#include "Gfx/tPhongMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tPhongMaterial::tLightingPass
const ::Sig::Rtti::tBaseClassDesc tPhongMaterial::tLightingPass::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPhongMaterial::tLightingPass::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial::tLightingPass, tEnum<tVS,u16> >( "tPhongMaterial::tLightingPass", "tEnum<tVS,u16>", "mVS", "public", offsetof( tPhongMaterial::tLightingPass, mVS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial::tLightingPass, tEnum<tPS,u16> >( "tPhongMaterial::tLightingPass", "tEnum<tPS,u16>", "mPS", "public", offsetof( tPhongMaterial::tLightingPass, mPS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPhongMaterial::tLightingPass::gReflector = ::Sig::Rtti::fDefineReflector< tPhongMaterial::tLightingPass >( "tPhongMaterial::tLightingPass", tPhongMaterial::tLightingPass::gBases, tPhongMaterial::tLightingPass::gMembers );


//
// tPhongMaterial::tShadowMapPass
const ::Sig::Rtti::tBaseClassDesc tPhongMaterial::tShadowMapPass::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPhongMaterial::tShadowMapPass::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial::tShadowMapPass, tEnum<tVS,u16> >( "tPhongMaterial::tShadowMapPass", "tEnum<tVS,u16>", "mVS", "public", offsetof( tPhongMaterial::tShadowMapPass, mVS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial::tShadowMapPass, tEnum<tPS,u16> >( "tPhongMaterial::tShadowMapPass", "tEnum<tPS,u16>", "mPS", "public", offsetof( tPhongMaterial::tShadowMapPass, mPS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPhongMaterial::tShadowMapPass::gReflector = ::Sig::Rtti::fDefineReflector< tPhongMaterial::tShadowMapPass >( "tPhongMaterial::tShadowMapPass", tPhongMaterial::tShadowMapPass::gBases, tPhongMaterial::tShadowMapPass::gMembers );


//
// tPhongMaterial
const ::Sig::Rtti::tBaseClassDesc tPhongMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPhongMaterial, tMaterial >( "tPhongMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tPhongMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, u32 >( "tPhongMaterial", "u32", "mVertexFormatSlot", "public", offsetof( tPhongMaterial, mVertexFormatSlot ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, tLightingPass >( "tPhongMaterial", "tLightingPass", "mLightingPass", "public", offsetof( tPhongMaterial, mLightingPass ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, tShadowMapPass >( "tPhongMaterial", "tShadowMapPass", "mShadowMapPass", "public", offsetof( tPhongMaterial, mShadowMapPass ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, tLoadInPlaceResourcePtr* >( "tPhongMaterial", "tLoadInPlaceResourcePtr*", "mDiffuseMap", "public", offsetof( tPhongMaterial, mDiffuseMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, tLoadInPlaceResourcePtr* >( "tPhongMaterial", "tLoadInPlaceResourcePtr*", "mSpecColorMap", "public", offsetof( tPhongMaterial, mSpecColorMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, tLoadInPlaceResourcePtr* >( "tPhongMaterial", "tLoadInPlaceResourcePtr*", "mEmissiveMap", "public", offsetof( tPhongMaterial, mEmissiveMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, tLoadInPlaceResourcePtr* >( "tPhongMaterial", "tLoadInPlaceResourcePtr*", "mNormalMap", "public", offsetof( tPhongMaterial, mNormalMap ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, Math::tVec4f >( "tPhongMaterial", "Math::tVec4f", "mDiffuseUvXform", "public", offsetof( tPhongMaterial, mDiffuseUvXform ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, Math::tVec4f >( "tPhongMaterial", "Math::tVec4f", "mSpecColorUvXform", "public", offsetof( tPhongMaterial, mSpecColorUvXform ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, Math::tVec4f >( "tPhongMaterial", "Math::tVec4f", "mEmissiveUvXform", "public", offsetof( tPhongMaterial, mEmissiveUvXform ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, Math::tVec4f >( "tPhongMaterial", "Math::tVec4f", "mNormalUvXform", "public", offsetof( tPhongMaterial, mNormalUvXform ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, Math::tVec4f >( "tPhongMaterial", "Math::tVec4f", "mDiffuseColor", "public", offsetof( tPhongMaterial, mDiffuseColor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, Math::tVec4f >( "tPhongMaterial", "Math::tVec4f", "mSpecColor", "public", offsetof( tPhongMaterial, mSpecColor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, Math::tVec4f >( "tPhongMaterial", "Math::tVec4f", "mEmissiveColor", "public", offsetof( tPhongMaterial, mEmissiveColor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPhongMaterial, Math::tVec4f >( "tPhongMaterial", "Math::tVec4f", "mBumpDepth_SpecSize_Opacity_BackFaceFlip", "public", offsetof( tPhongMaterial, mBumpDepth_SpecSize_Opacity_BackFaceFlip ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tPhongMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tPhongMaterial >( "tPhongMaterial", tPhongMaterial::gBases, tPhongMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tRenderState.hpp]
//
#include "Gfx/tRenderState.hpp"
namespace Sig
{
namespace Gfx
{

//
// tRenderState
const ::Sig::Rtti::tBaseClassDesc tRenderState::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tRenderState::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tRenderState, u32 >( "tRenderState", "u32", "mFlags", "public", offsetof( tRenderState, mFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tRenderState, u8 >( "tRenderState", "u8", "mCutOutThreshold", "public", offsetof( tRenderState, mCutOutThreshold ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tRenderState, u8 >( "tRenderState", "u8", "mSrcDstBlend", "public", offsetof( tRenderState, mSrcDstBlend ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tRenderState, s8 >( "tRenderState", "s8", "mDepthBias", "public", offsetof( tRenderState, mDepthBias ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tRenderState, s8 >( "tRenderState", "s8", "mSlopeScaleBias", "public", offsetof( tRenderState, mSlopeScaleBias ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tRenderState::gReflector = ::Sig::Rtti::fDefineReflector< tRenderState >( "tRenderState", tRenderState::gBases, tRenderState::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tShadeMaterial.hpp]
//
#include "Gfx/tShadeMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tShadeMaterialGlueValues::tTextureGlue
const ::Sig::Rtti::tBaseClassDesc tShadeMaterialGlueValues::tTextureGlue::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tShadeMaterialGlueValues::tTextureGlue::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterialGlueValues::tTextureGlue, tEnum<tTextureSource,u32> >( "tShadeMaterialGlueValues::tTextureGlue", "tEnum<tTextureSource,u32>", "mTexSrc", "public", offsetof( tShadeMaterialGlueValues::tTextureGlue, mTexSrc ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterialGlueValues::tTextureGlue, tTextureReference >( "tShadeMaterialGlueValues::tTextureGlue", "tTextureReference", "mTexRef", "public", offsetof( tShadeMaterialGlueValues::tTextureGlue, mTexRef ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tShadeMaterialGlueValues::tTextureGlue::gReflector = ::Sig::Rtti::fDefineReflector< tShadeMaterialGlueValues::tTextureGlue >( "tShadeMaterialGlueValues::tTextureGlue", tShadeMaterialGlueValues::tTextureGlue::gBases, tShadeMaterialGlueValues::tTextureGlue::gMembers );


//
// tShadeMaterialGlueValues
const ::Sig::Rtti::tBaseClassDesc tShadeMaterialGlueValues::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tShadeMaterialGlueValues::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterialGlueValues, tDynamicArray<tTextureGlue> >( "tShadeMaterialGlueValues", "tDynamicArray<tTextureGlue>", "mSamplers", "public", offsetof( tShadeMaterialGlueValues, mSamplers ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterialGlueValues, tDynamicArray<Math::tVec4f> >( "tShadeMaterialGlueValues", "tDynamicArray<Math::tVec4f>", "mVectors", "public", offsetof( tShadeMaterialGlueValues, mVectors ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterialGlueValues, tDynamicArray<tStringGlue> >( "tShadeMaterialGlueValues", "tDynamicArray<tStringGlue>", "mStrings", "public", offsetof( tShadeMaterialGlueValues, mStrings ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterialGlueValues, f32 >( "tShadeMaterialGlueValues", "f32", "mBackFaceFlip", "public", offsetof( tShadeMaterialGlueValues, mBackFaceFlip ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tShadeMaterialGlueValues::gReflector = ::Sig::Rtti::fDefineReflector< tShadeMaterialGlueValues >( "tShadeMaterialGlueValues", tShadeMaterialGlueValues::gBases, tShadeMaterialGlueValues::gMembers );


//
// tShadeMaterial::tPass
const ::Sig::Rtti::tBaseClassDesc tShadeMaterial::tPass::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tShadeMaterial::tPass::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, Gfx::tVertexFormat >( "tShadeMaterial::tPass", "Gfx::tVertexFormat", "mVertexFormat", "public", offsetof( tShadeMaterial::tPass, mVertexFormat ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, u8 >( "tShadeMaterial::tPass", "u8", "mVsType", "public", offsetof( tShadeMaterial::tPass, mVsType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, u8 >( "tShadeMaterial::tPass", "u8", "mVsIndex", "public", offsetof( tShadeMaterial::tPass, mVsIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, u8 >( "tShadeMaterial::tPass", "u8", "mPsType", "public", offsetof( tShadeMaterial::tPass, mPsType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, u8 >( "tShadeMaterial::tPass", "u8", "mPsBaseIndex", "public", offsetof( tShadeMaterial::tPass, mPsBaseIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, u8 >( "tShadeMaterial::tPass", "u8", "mPsMaxLights", "public", offsetof( tShadeMaterial::tPass, mPsMaxLights ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, u8 >( "tShadeMaterial::tPass", "u8", "pad0", "public", offsetof( tShadeMaterial::tPass, pad0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, u8 >( "tShadeMaterial::tPass", "u8", "pad1", "public", offsetof( tShadeMaterial::tPass, pad1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial::tPass, u8 >( "tShadeMaterial::tPass", "u8", "pad2", "public", offsetof( tShadeMaterial::tPass, pad2 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tShadeMaterial::tPass::gReflector = ::Sig::Rtti::fDefineReflector< tShadeMaterial::tPass >( "tShadeMaterial::tPass", tShadeMaterial::tPass::gBases, tShadeMaterial::tPass::gMembers );


//
// tShadeMaterial
const ::Sig::Rtti::tBaseClassDesc tShadeMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tShadeMaterial, tMaterial >( "tShadeMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tShadeMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial, tPass >( "tShadeMaterial", "tPass", "mColorPass", "public", offsetof( tShadeMaterial, mColorPass ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial, tPass >( "tShadeMaterial", "tPass", "mDepthPass", "public", offsetof( tShadeMaterial, mDepthPass ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tShadeMaterial, tShadeMaterialGlueValues >( "tShadeMaterial", "tShadeMaterialGlueValues", "mGlueValues", "public", offsetof( tShadeMaterial, mGlueValues ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tShadeMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tShadeMaterial >( "tShadeMaterial", tShadeMaterial::gBases, tShadeMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tSolidColorMaterial.hpp]
//
#include "Gfx/tSolidColorMaterial.hpp"
namespace Sig
{
namespace Gfx
{

//
// tSolidColorMaterial
const ::Sig::Rtti::tBaseClassDesc tSolidColorMaterial::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tSolidColorMaterial, tMaterial >( "tSolidColorMaterial", "tMaterial", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tSolidColorMaterial::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tSolidColorMaterial::gReflector = ::Sig::Rtti::fDefineReflector< tSolidColorMaterial >( "tSolidColorMaterial", tSolidColorMaterial::gBases, tSolidColorMaterial::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tTextureFile.hpp]
//
#include "Gfx/tTextureFile.hpp"
namespace Sig
{
namespace Gfx
{

//
// tTextureFile::tSurfacePointer
const ::Sig::Rtti::tBaseClassDesc tTextureFile::tSurfacePointer::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTextureFile::tSurfacePointer::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile::tSurfacePointer, u32 >( "tTextureFile::tSurfacePointer", "u32", "mBufferOffset", "public", offsetof( tTextureFile::tSurfacePointer, mBufferOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile::tSurfacePointer, u32 >( "tTextureFile::tSurfacePointer", "u32", "mBufferSize", "public", offsetof( tTextureFile::tSurfacePointer, mBufferSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTextureFile::tSurfacePointer::gReflector = ::Sig::Rtti::fDefineReflector< tTextureFile::tSurfacePointer >( "tTextureFile::tSurfacePointer", tTextureFile::tSurfacePointer::gBases, tTextureFile::tSurfacePointer::gMembers );


//
// tTextureFile::tImage
const ::Sig::Rtti::tBaseClassDesc tTextureFile::tImage::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTextureFile::tImage::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile::tImage, tDynamicArray<tSurfacePointer> >( "tTextureFile::tImage", "tDynamicArray<tSurfacePointer>", "mMipMapBuffers", "public", offsetof( tTextureFile::tImage, mMipMapBuffers ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTextureFile::tImage::gReflector = ::Sig::Rtti::fDefineReflector< tTextureFile::tImage >( "tTextureFile::tImage", tTextureFile::tImage::gBases, tTextureFile::tImage::gMembers );


//
// tTextureFile::tLockedMip
const ::Sig::Rtti::tBaseClassDesc tTextureFile::tLockedMip::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTextureFile::tLockedMip::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile::tLockedMip, Sig::byte* >( "tTextureFile::tLockedMip", "Sig::byte*", "mBits", "public", offsetof( tTextureFile::tLockedMip, mBits ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile::tLockedMip, s32 >( "tTextureFile::tLockedMip", "s32", "mPitch", "public", offsetof( tTextureFile::tLockedMip, mPitch ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTextureFile::tLockedMip::gReflector = ::Sig::Rtti::fDefineReflector< tTextureFile::tLockedMip >( "tTextureFile::tLockedMip", tTextureFile::tLockedMip::gBases, tTextureFile::tLockedMip::gMembers );


//
// tTextureFile
const ::Sig::Rtti::tBaseClassDesc tTextureFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tTextureFile, tLoadInPlaceFileBase >( "tTextureFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTextureFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, tDynamicArray<tImage> >( "tTextureFile", "tDynamicArray<tImage>", "mImages", "public", offsetof( tTextureFile, mImages ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, tPlatformHandle >( "tTextureFile", "tPlatformHandle", "mPlatformHandle", "public", offsetof( tTextureFile, mPlatformHandle ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u32 >( "tTextureFile", "u32", "mHeaderSize", "public", offsetof( tTextureFile, mHeaderSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, b16 >( "tTextureFile", "b16", "mDiscardSurfaceBuffers", "public", offsetof( tTextureFile, mDiscardSurfaceBuffers ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u16 >( "tTextureFile", "u16", "mMipMapCount", "public", offsetof( tTextureFile, mMipMapCount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u16 >( "tTextureFile", "u16", "mWidth", "public", offsetof( tTextureFile, mWidth ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u16 >( "tTextureFile", "u16", "mHeight", "public", offsetof( tTextureFile, mHeight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, tEnum<tType,u8> >( "tTextureFile", "tEnum<tType,u8>", "mType", "public", offsetof( tTextureFile, mType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, tEnum<tSemantic,u8> >( "tTextureFile", "tEnum<tSemantic,u8>", "mSemantic", "public", offsetof( tTextureFile, mSemantic ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, tEnum<tFormat,u8> >( "tTextureFile", "tEnum<tFormat,u8>", "mFormat", "public", offsetof( tTextureFile, mFormat ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u8 >( "tTextureFile", "u8", "mIsAtlas", "public", offsetof( tTextureFile, mIsAtlas ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u16 >( "tTextureFile", "u16", "mSubTexWidth", "public", offsetof( tTextureFile, mSubTexWidth ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u16 >( "tTextureFile", "u16", "mSubTexHeight", "public", offsetof( tTextureFile, mSubTexHeight ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u16 >( "tTextureFile", "u16", "mSubTexCountX", "public", offsetof( tTextureFile, mSubTexCountX ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureFile, u16 >( "tTextureFile", "u16", "mSubTexCountY", "public", offsetof( tTextureFile, mSubTexCountY ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTextureFile::gReflector = ::Sig::Rtti::fDefineReflector< tTextureFile >( "tTextureFile", tTextureFile::gBases, tTextureFile::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tTextureReference.hpp]
//
#include "Gfx/tTextureReference.hpp"
namespace Sig
{
namespace Gfx
{

//
// tTextureReference
const ::Sig::Rtti::tBaseClassDesc tTextureReference::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tTextureReference::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tTextureReference, tLoadInPlaceResourcePtr* >( "tTextureReference", "tLoadInPlaceResourcePtr*", "mLip", "public", offsetof( tTextureReference, mLip ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureReference, tResource* >( "tTextureReference", "tResource*", "mDynamic", "public", offsetof( tTextureReference, mDynamic ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureReference, tPlatformHandle >( "tTextureReference", "tPlatformHandle", "mRaw", "public", offsetof( tTextureReference, mRaw ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureReference, tEnum<tFilterMode,u8> >( "tTextureReference", "tEnum<tFilterMode,u8>", "mFilterMode", "public", offsetof( tTextureReference, mFilterMode ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureReference, u8 >( "tTextureReference", "u8", "mAddressModeU", "public", offsetof( tTextureReference, mAddressModeU ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureReference, u8 >( "tTextureReference", "u8", "mAddressModeV", "public", offsetof( tTextureReference, mAddressModeV ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tTextureReference, u8 >( "tTextureReference", "u8", "mAddressModeW", "public", offsetof( tTextureReference, mAddressModeW ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tTextureReference::gReflector = ::Sig::Rtti::fDefineReflector< tTextureReference >( "tTextureReference", tTextureReference::gBases, tTextureReference::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tVertexFormat.hpp]
//
#include "Gfx/tVertexFormat.hpp"
namespace Sig
{
namespace Gfx
{

//
// tVertexElement
const ::Sig::Rtti::tBaseClassDesc tVertexElement::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVertexElement::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVertexElement, tEnum<tSemantic,u8> >( "tVertexElement", "tEnum<tSemantic,u8>", "mSemantic", "public", offsetof( tVertexElement, mSemantic ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVertexElement, tEnum<tFormat,u8> >( "tVertexElement", "tEnum<tFormat,u8>", "mFormat", "public", offsetof( tVertexElement, mFormat ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVertexElement, u8 >( "tVertexElement", "u8", "mSemanticIndex", "public", offsetof( tVertexElement, mSemanticIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVertexElement, u8 >( "tVertexElement", "u8", "mStreamIndex", "public", offsetof( tVertexElement, mStreamIndex ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVertexElement, u16 >( "tVertexElement", "u16", "mOffsetFromBase", "public", offsetof( tVertexElement, mOffsetFromBase ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVertexElement, u16 >( "tVertexElement", "u16", "mSize", "public", offsetof( tVertexElement, mSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVertexElement::gReflector = ::Sig::Rtti::fDefineReflector< tVertexElement >( "tVertexElement", tVertexElement::gBases, tVertexElement::gMembers );


//
// tVertexFormat
const ::Sig::Rtti::tBaseClassDesc tVertexFormat::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVertexFormat::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVertexFormat, u32 >( "tVertexFormat", "u32", "mVertexSize", "public", offsetof( tVertexFormat, mVertexSize ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVertexFormat, tDynamicArray<tVertexElement> >( "tVertexFormat", "tDynamicArray<tVertexElement>", "mVertexElements", "public", offsetof( tVertexFormat, mVertexElements ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVertexFormat::gReflector = ::Sig::Rtti::fDefineReflector< tVertexFormat >( "tVertexFormat", tVertexFormat::gBases, tVertexFormat::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Gfx\tVertexFormatVRam.hpp]
//
#include "Gfx/tVertexFormatVRam.hpp"
namespace Sig
{
namespace Gfx
{

//
// tVertexFormatVRam
const ::Sig::Rtti::tBaseClassDesc tVertexFormatVRam::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVertexFormatVRam, tVertexFormat >( "tVertexFormatVRam", "tVertexFormat", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tVertexFormatVRam::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVertexFormatVRam, tPlatformHandle >( "tVertexFormatVRam", "tPlatformHandle", "mPlatformHandle", "public", offsetof( tVertexFormatVRam, mPlatformHandle ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tVertexFormatVRam::gReflector = ::Sig::Rtti::fDefineReflector< tVertexFormatVRam >( "tVertexFormatVRam", tVertexFormatVRam::gBases, tVertexFormatVRam::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\FX\tFxFile.hpp]
//
#include "FX/tFxFile.hpp"
namespace Sig
{
namespace FX
{

//
// tFxFile
const ::Sig::Rtti::tBaseClassDesc tFxFile::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tFxFile, tLoadInPlaceFileBase >( "tFxFile", "tLoadInPlaceFileBase", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tFxFile::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFxFile, tObjectArray >( "tFxFile", "tObjectArray", "mObjects", "public", offsetof( tFxFile, mObjects ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFxFile, f32 >( "tFxFile", "f32", "mLifetime", "public", offsetof( tFxFile, mLifetime ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFxFile, Math::tAabbf >( "tFxFile", "Math::tAabbf", "mBounds", "public", offsetof( tFxFile, mBounds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tFxFile::gReflector = ::Sig::Rtti::fDefineReflector< tFxFile >( "tFxFile", tFxFile::gBases, tFxFile::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\FX\tFxGraph.hpp]
//
#include "FX/tFxGraph.hpp"
namespace Sig
{
namespace FX
{

//
// tBinaryGraph
const ::Sig::Rtti::tBaseClassDesc tBinaryGraph::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tBinaryGraph, Rtti::tSerializableBaseClass >( "tBinaryGraph", "Rtti::tSerializableBaseClass", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryGraph::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryGraph, tDynamicArray<Math::tVec2f> >( "tBinaryGraph", "tDynamicArray<Math::tVec2f>", "mRandoms", "public", offsetof( tBinaryGraph, mRandoms ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryGraph, b16 >( "tBinaryGraph", "b16", "mUseLerp", "public", offsetof( tBinaryGraph, mUseLerp ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryGraph, b16 >( "tBinaryGraph", "b16", "mUseRandom", "public", offsetof( tBinaryGraph, mUseRandom ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryGraph::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryGraph >( "tBinaryGraph", tBinaryGraph::gBases, tBinaryGraph::gMembers );


//
// tBinaryF32Graph
const ::Sig::Rtti::tBaseClassDesc tBinaryF32Graph::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tBinaryF32Graph, tBinaryTemplateGraph< f32 > >( "tBinaryF32Graph", "tBinaryTemplateGraph< f32 >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryF32Graph::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryF32Graph::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryF32Graph >( "tBinaryF32Graph", tBinaryF32Graph::gBases, tBinaryF32Graph::gMembers );


//
// tBinaryV2Graph
const ::Sig::Rtti::tBaseClassDesc tBinaryV2Graph::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tBinaryV2Graph, tBinaryTemplateGraph< Math::tVec2f > >( "tBinaryV2Graph", "tBinaryTemplateGraph< Math::tVec2f >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryV2Graph::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryV2Graph::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryV2Graph >( "tBinaryV2Graph", tBinaryV2Graph::gBases, tBinaryV2Graph::gMembers );


//
// tBinaryV3Graph
const ::Sig::Rtti::tBaseClassDesc tBinaryV3Graph::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tBinaryV3Graph, tBinaryTemplateGraph< Math::tVec3f > >( "tBinaryV3Graph", "tBinaryTemplateGraph< Math::tVec3f >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryV3Graph::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryV3Graph::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryV3Graph >( "tBinaryV3Graph", tBinaryV3Graph::gBases, tBinaryV3Graph::gMembers );


//
// tBinaryV4Graph
const ::Sig::Rtti::tBaseClassDesc tBinaryV4Graph::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tBinaryV4Graph, tBinaryTemplateGraph< Math::tVec4f > >( "tBinaryV4Graph", "tBinaryTemplateGraph< Math::tVec4f >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryV4Graph::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryV4Graph::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryV4Graph >( "tBinaryV4Graph", tBinaryV4Graph::gBases, tBinaryV4Graph::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\FX\tMeshSystem.hpp]
//
#include "FX/tMeshSystem.hpp"
namespace Sig
{
namespace FX
{

//
// tBinaryFxMeshSystemData
const ::Sig::Rtti::tBaseClassDesc tBinaryFxMeshSystemData::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryFxMeshSystemData::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryFxMeshSystemData, u32 >( "tBinaryFxMeshSystemData", "u32", "mSystemFlags", "public", offsetof( tBinaryFxMeshSystemData, mSystemFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryFxMeshSystemData, tEnum<tEmitterType,u8> >( "tBinaryFxMeshSystemData", "tEnum<tEmitterType,u8>", "mEmitterType", "public", offsetof( tBinaryFxMeshSystemData, mEmitterType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryFxMeshSystemData, u8 >( "tBinaryFxMeshSystemData", "u8", "mPad", "public", offsetof( tBinaryFxMeshSystemData, mPad ), 3, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryFxMeshSystemData, tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> > >( "tBinaryFxMeshSystemData", "tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> >", "mGraphs", "public", offsetof( tBinaryFxMeshSystemData, mGraphs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryFxMeshSystemData, tDynamicArray<u32> >( "tBinaryFxMeshSystemData", "tDynamicArray<u32>", "mAttractorIgnoreIds", "public", offsetof( tBinaryFxMeshSystemData, mAttractorIgnoreIds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryFxMeshSystemData::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryFxMeshSystemData >( "tBinaryFxMeshSystemData", tBinaryFxMeshSystemData::gBases, tBinaryFxMeshSystemData::gMembers );


//
// tMeshSystemDef
const ::Sig::Rtti::tBaseClassDesc tMeshSystemDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tMeshSystemDef, tEntityDef >( "tMeshSystemDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tMeshSystemDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMeshSystemDef, tLoadInPlacePtrWrapper<tBinaryFxMeshSystemData> >( "tMeshSystemDef", "tLoadInPlacePtrWrapper<tBinaryFxMeshSystemData>", "mBinaryData", "public", offsetof( tMeshSystemDef, mBinaryData ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMeshSystemDef, tLoadInPlaceStringPtr* >( "tMeshSystemDef", "tLoadInPlaceStringPtr*", "mFxMeshSystemName", "public", offsetof( tMeshSystemDef, mFxMeshSystemName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMeshSystemDef, tLoadInPlaceStringPtr* >( "tMeshSystemDef", "tLoadInPlaceStringPtr*", "mParticleSystemToSyncWith", "public", offsetof( tMeshSystemDef, mParticleSystemToSyncWith ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMeshSystemDef, tLoadInPlaceStringPtr* >( "tMeshSystemDef", "tLoadInPlaceStringPtr*", "mMeshResourceFile", "public", offsetof( tMeshSystemDef, mMeshResourceFile ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tMeshSystemDef::gReflector = ::Sig::Rtti::fDefineReflector< tMeshSystemDef >( "tMeshSystemDef", tMeshSystemDef::gBases, tMeshSystemDef::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\FX\tParticleAttractor.hpp]
//
#include "FX/tParticleAttractor.hpp"
namespace Sig
{
namespace FX
{

//
// tBinaryAttractorData
const ::Sig::Rtti::tBaseClassDesc tBinaryAttractorData::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryAttractorData::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryAttractorData, tEnum<tForceType,u8> >( "tBinaryAttractorData", "tEnum<tForceType,u8>", "mType", "public", offsetof( tBinaryAttractorData, mType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryAttractorData, u8 >( "tBinaryAttractorData", "u8", "mPad", "public", offsetof( tBinaryAttractorData, mPad ), 3, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryAttractorData, b32 >( "tBinaryAttractorData", "b32", "mParticleMustBeInRadius", "public", offsetof( tBinaryAttractorData, mParticleMustBeInRadius ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryAttractorData, b32 >( "tBinaryAttractorData", "b32", "mAffectParticlesDirection", "public", offsetof( tBinaryAttractorData, mAffectParticlesDirection ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryAttractorData, u32 >( "tBinaryAttractorData", "u32", "mId", "public", offsetof( tBinaryAttractorData, mId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryAttractorData, tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> > >( "tBinaryAttractorData", "tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> >", "mGraphs", "public", offsetof( tBinaryAttractorData, mGraphs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryAttractorData, u32 >( "tBinaryAttractorData", "u32", "mFlags", "public", offsetof( tBinaryAttractorData, mFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryAttractorData::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryAttractorData >( "tBinaryAttractorData", tBinaryAttractorData::gBases, tBinaryAttractorData::gMembers );


//
// tParticleAttractorDef
const ::Sig::Rtti::tBaseClassDesc tParticleAttractorDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tParticleAttractorDef, tEntityDef >( "tParticleAttractorDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tParticleAttractorDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tParticleAttractorDef, tLoadInPlacePtrWrapper<tBinaryAttractorData> >( "tParticleAttractorDef", "tLoadInPlacePtrWrapper<tBinaryAttractorData>", "mBinaryData", "public", offsetof( tParticleAttractorDef, mBinaryData ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleAttractorDef, tEnum<tForceType,u8> >( "tParticleAttractorDef", "tEnum<tForceType,u8>", "mType", "public", offsetof( tParticleAttractorDef, mType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleAttractorDef, u8 >( "tParticleAttractorDef", "u8", "mPad", "public", offsetof( tParticleAttractorDef, mPad ), 3, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleAttractorDef, u32 >( "tParticleAttractorDef", "u32", "mId", "public", offsetof( tParticleAttractorDef, mId ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleAttractorDef, b32 >( "tParticleAttractorDef", "b32", "mParticleMustBeInRadius", "public", offsetof( tParticleAttractorDef, mParticleMustBeInRadius ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleAttractorDef, b32 >( "tParticleAttractorDef", "b32", "mAffectParticlesDirection", "public", offsetof( tParticleAttractorDef, mAffectParticlesDirection ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleAttractorDef, tLoadInPlaceStringPtr* >( "tParticleAttractorDef", "tLoadInPlaceStringPtr*", "mAttractorName", "public", offsetof( tParticleAttractorDef, mAttractorName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleAttractorDef, u32 >( "tParticleAttractorDef", "u32", "mFlags", "public", offsetof( tParticleAttractorDef, mFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tParticleAttractorDef::gReflector = ::Sig::Rtti::fDefineReflector< tParticleAttractorDef >( "tParticleAttractorDef", tParticleAttractorDef::gBases, tParticleAttractorDef::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\FX\tParticleSystem.hpp]
//
#include "FX/tParticleSystem.hpp"
namespace Sig
{
namespace FX
{

//
// tParticleSystemDef
const ::Sig::Rtti::tBaseClassDesc tParticleSystemDef::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tParticleSystemDef, tEntityDef >( "tParticleSystemDef", "tEntityDef", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tParticleSystemDef::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, tLoadInPlaceStringPtr* >( "tParticleSystemDef", "tLoadInPlaceStringPtr*", "mParticleSystemName", "public", offsetof( tParticleSystemDef, mParticleSystemName ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, tLoadInPlaceResourcePtr* >( "tParticleSystemDef", "tLoadInPlaceResourcePtr*", "mMeshResource", "public", offsetof( tParticleSystemDef, mMeshResource ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, tBinaryParticleSystemStates >( "tParticleSystemDef", "tBinaryParticleSystemStates", "mStates", "public", offsetof( tParticleSystemDef, mStates ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, b32 >( "tParticleSystemDef", "b32", "mLocalSpace", "public", offsetof( tParticleSystemDef, mLocalSpace ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, f32 >( "tParticleSystemDef", "f32", "mCameraDepthOffset", "public", offsetof( tParticleSystemDef, mCameraDepthOffset ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, f32 >( "tParticleSystemDef", "f32", "mUpdateSpeedMultiplier", "public", offsetof( tParticleSystemDef, mUpdateSpeedMultiplier ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, f32 >( "tParticleSystemDef", "f32", "mLodFactor", "public", offsetof( tParticleSystemDef, mLodFactor ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, f32 >( "tParticleSystemDef", "f32", "mGhostParticleFrequency", "public", offsetof( tParticleSystemDef, mGhostParticleFrequency ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, f32 >( "tParticleSystemDef", "f32", "mGhostParticleLifetime", "public", offsetof( tParticleSystemDef, mGhostParticleLifetime ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, tDynamicArray<u32> >( "tParticleSystemDef", "tDynamicArray<u32>", "mAttractorIgnoreIds", "public", offsetof( tParticleSystemDef, mAttractorIgnoreIds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, u32 >( "tParticleSystemDef", "u32", "mSystemFlags", "public", offsetof( tParticleSystemDef, mSystemFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, tEnum<tEmitterType,u8> >( "tParticleSystemDef", "tEnum<tEmitterType,u8>", "mEmitterType", "public", offsetof( tParticleSystemDef, mEmitterType ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, tEnum<tParticleSortMode,u8> >( "tParticleSystemDef", "tEnum<tParticleSortMode,u8>", "mSortMode", "public", offsetof( tParticleSystemDef, mSortMode ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, u8 >( "tParticleSystemDef", "u8", "mPad0", "public", offsetof( tParticleSystemDef, mPad0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, u8 >( "tParticleSystemDef", "u8", "mPad1", "public", offsetof( tParticleSystemDef, mPad1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, Gfx::tRenderState >( "tParticleSystemDef", "Gfx::tRenderState", "mRenderState", "public", offsetof( tParticleSystemDef, mRenderState ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tParticleSystemDef, Gfx::tMaterial* >( "tParticleSystemDef", "Gfx::tMaterial*", "mMaterial", "public", offsetof( tParticleSystemDef, mMaterial ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tParticleSystemDef::gReflector = ::Sig::Rtti::fDefineReflector< tParticleSystemDef >( "tParticleSystemDef", tParticleSystemDef::gBases, tParticleSystemDef::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\FX\tParticleSystemStates.hpp]
//
#include "FX/tParticleSystemStates.hpp"
namespace Sig
{
namespace FX
{

//
// tBinaryParticleSystemState
const ::Sig::Rtti::tBaseClassDesc tBinaryParticleSystemState::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBinaryParticleSystemState::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryParticleSystemState, tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> > >( "tBinaryParticleSystemState", "tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> >", "mEmissionGraphs", "public", offsetof( tBinaryParticleSystemState, mEmissionGraphs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryParticleSystemState, tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> > >( "tBinaryParticleSystemState", "tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> >", "mPerParticleGraphs", "public", offsetof( tBinaryParticleSystemState, mPerParticleGraphs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryParticleSystemState, tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> > >( "tBinaryParticleSystemState", "tDynamicArray<tLoadInPlacePtrWrapper<tBinaryGraph> >", "mMeshGraphs", "public", offsetof( tBinaryParticleSystemState, mMeshGraphs ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryParticleSystemState, tDynamicArray<u32> >( "tBinaryParticleSystemState", "tDynamicArray<u32>", "mAttractorIgnoreIds", "public", offsetof( tBinaryParticleSystemState, mAttractorIgnoreIds ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryParticleSystemState, u32 >( "tBinaryParticleSystemState", "u32", "mSystemFlags", "public", offsetof( tBinaryParticleSystemState, mSystemFlags ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBinaryParticleSystemState::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryParticleSystemState >( "tBinaryParticleSystemState", tBinaryParticleSystemState::gBases, tBinaryParticleSystemState::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\AI\tBuiltNavGraph.hpp]
//
#include "AI/tBuiltNavGraph.hpp"
namespace Sig
{
namespace AI
{

//
// tBuiltPathNode
const ::Sig::Rtti::tBaseClassDesc tBuiltPathNode::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBuiltPathNode::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltPathNode, Math::tMat3f >( "tBuiltPathNode", "Math::tMat3f", "mObjToWorld", "public", offsetof( tBuiltPathNode, mObjToWorld ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltPathNode, Math::tObbf >( "tBuiltPathNode", "Math::tObbf", "mWorldSpaceObb", "public", offsetof( tBuiltPathNode, mWorldSpaceObb ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltPathNode, tDynamicArray<u32> >( "tBuiltPathNode", "tDynamicArray<u32>", "mEdges", "public", offsetof( tBuiltPathNode, mEdges ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBuiltPathNode::gReflector = ::Sig::Rtti::fDefineReflector< tBuiltPathNode >( "tBuiltPathNode", tBuiltPathNode::gBases, tBuiltPathNode::gMembers );


//
// tBuiltPathEdge
const ::Sig::Rtti::tBaseClassDesc tBuiltPathEdge::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBuiltPathEdge::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltPathEdge, u16 >( "tBuiltPathEdge", "u16", "mNodeA", "public", offsetof( tBuiltPathEdge, mNodeA ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltPathEdge, u16 >( "tBuiltPathEdge", "u16", "mNodeB", "public", offsetof( tBuiltPathEdge, mNodeB ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltPathEdge, tFixedArray<Math::tPlanef,4> >( "tBuiltPathEdge", "tFixedArray<Math::tPlanef,4>", "mHalfSpaces", "public", offsetof( tBuiltPathEdge, mHalfSpaces ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltPathEdge, tFixedArray<Math::tVec3f,2> >( "tBuiltPathEdge", "tFixedArray<Math::tVec3f,2>", "mHighEdge", "public", offsetof( tBuiltPathEdge, mHighEdge ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltPathEdge, tFixedArray<Math::tVec3f,2> >( "tBuiltPathEdge", "tFixedArray<Math::tVec3f,2>", "mLowEdge", "public", offsetof( tBuiltPathEdge, mLowEdge ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBuiltPathEdge::gReflector = ::Sig::Rtti::fDefineReflector< tBuiltPathEdge >( "tBuiltPathEdge", tBuiltPathEdge::gBases, tBuiltPathEdge::gMembers );


//
// tBuiltNavGraph
const ::Sig::Rtti::tBaseClassDesc tBuiltNavGraph::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tBuiltNavGraph, tRefCounter >( "tBuiltNavGraph", "tRefCounter", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
const ::Sig::Rtti::tClassMemberDesc tBuiltNavGraph::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltNavGraph, tDynamicArray<tBuiltPathNode> >( "tBuiltNavGraph", "tDynamicArray<tBuiltPathNode>", "mNodes", "public", offsetof( tBuiltNavGraph, mNodes ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltNavGraph, tDynamicArray<tBuiltPathEdge> >( "tBuiltNavGraph", "tDynamicArray<tBuiltPathEdge>", "mEdges", "public", offsetof( tBuiltNavGraph, mEdges ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltNavGraph, u32 >( "tBuiltNavGraph", "u32", "mNavGraphID", "public", offsetof( tBuiltNavGraph, mNavGraphID ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltNavGraph, tKdTree >( "tBuiltNavGraph", "tKdTree", "mNodeTree", "public", offsetof( tBuiltNavGraph, mNodeTree ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltNavGraph, tKdTree >( "tBuiltNavGraph", "tKdTree", "mEdgeTree", "public", offsetof( tBuiltNavGraph, mEdgeTree ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltNavGraph, tDynamicArray<Math::tVec3f> >( "tBuiltNavGraph", "tDynamicArray<Math::tVec3f>", "mBadPos", "public", offsetof( tBuiltNavGraph, mBadPos ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tBuiltNavGraph, tDynamicArray<tBuiltPathEdge> >( "tBuiltNavGraph", "tDynamicArray<tBuiltPathEdge>", "mBadEdges", "public", offsetof( tBuiltNavGraph, mBadEdges ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
const ::Sig::Rtti::tReflector tBuiltNavGraph::gReflector = ::Sig::Rtti::fDefineReflector< tBuiltNavGraph >( "tBuiltNavGraph", tBuiltNavGraph::gBases, tBuiltNavGraph::gMembers );

}
}

