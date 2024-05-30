#ifndef __Base_rtti__
#define __Base_rtti__

//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tArray.hpp]
//
#include "tArray.hpp"
namespace Sig
{

//
// tFixedArray
template<class t, int N>
const ::Sig::Rtti::tBaseClassDesc tFixedArray< t,  N >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t, int N>
const ::Sig::Rtti::tClassMemberDesc tFixedArray< t,  N >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFixedArray, t >( "tFixedArray", "t", "mItems", "public", offsetof( tFixedArray, mItems ), N, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t, int N>
const ::Sig::Rtti::tReflector tFixedArray< t,  N >::gReflector = ::Sig::Rtti::fDefineReflector< tFixedArray >( "tFixedArray", tFixedArray< t,  N >::gBases, tFixedArray< t,  N >::gMembers );


//
// tArraySleeve
template<class t>
const ::Sig::Rtti::tBaseClassDesc tArraySleeve< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tArraySleeve< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tArraySleeve, t* >( "tArraySleeve", "t*", "mItems", "public", offsetof( tArraySleeve, mItems ), 1, offsetof( tArraySleeve, mCount ), sizeof_member( tArraySleeve, mCount ) ),
	::Sig::Rtti::fDefineClassMemberDesc< tArraySleeve, u32 >( "tArraySleeve", "u32", "mCount", "public", offsetof( tArraySleeve, mCount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tArraySleeve< t >::gReflector = ::Sig::Rtti::fDefineReflector< tArraySleeve >( "tArraySleeve", tArraySleeve< t >::gBases, tArraySleeve< t >::gMembers );


//
// tDynamicArray
template<class t>
const ::Sig::Rtti::tBaseClassDesc tDynamicArray< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tDynamicArray, tArraySleeve<t> >( "tDynamicArray", "tArraySleeve<t>", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tDynamicArray< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tDynamicArray< t >::gReflector = ::Sig::Rtti::fDefineReflector< tDynamicArray >( "tDynamicArray", tDynamicArray< t >::gBases, tDynamicArray< t >::gMembers );


//
// tFixedGrowingArray
template<class t, int N>
const ::Sig::Rtti::tBaseClassDesc tFixedGrowingArray< t,  N >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t, int N>
const ::Sig::Rtti::tClassMemberDesc tFixedGrowingArray< t,  N >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFixedGrowingArray, t >( "tFixedGrowingArray", "t", "mItems", "public", offsetof( tFixedGrowingArray, mItems ), N, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tFixedGrowingArray, u32 >( "tFixedGrowingArray", "u32", "mUsedCount", "public", offsetof( tFixedGrowingArray, mUsedCount ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t, int N>
const ::Sig::Rtti::tReflector tFixedGrowingArray< t,  N >::gReflector = ::Sig::Rtti::fDefineReflector< tFixedGrowingArray >( "tFixedGrowingArray", tFixedGrowingArray< t,  N >::gBases, tFixedGrowingArray< t,  N >::gMembers );


//
// tEnum
template<class tRealEnum, class tStorage>
const ::Sig::Rtti::tBaseClassDesc tEnum< tRealEnum,  tStorage >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class tRealEnum, class tStorage>
const ::Sig::Rtti::tClassMemberDesc tEnum< tRealEnum,  tStorage >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tEnum, tStorage >( "tEnum", "tStorage", "mValue", "public", offsetof( tEnum, mValue ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class tRealEnum, class tStorage>
const ::Sig::Rtti::tReflector tEnum< tRealEnum,  tStorage >::gReflector = ::Sig::Rtti::fDefineReflector< tEnum >( "tEnum", tEnum< tRealEnum,  tStorage >::gBases, tEnum< tRealEnum,  tStorage >::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tLoadInPlaceObjects.hpp]
//
#include "tLoadInPlaceObjects.hpp"
namespace Sig
{

//
// tLoadInPlacePtrWrapper
template<class t>
const ::Sig::Rtti::tBaseClassDesc tLoadInPlacePtrWrapper< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tLoadInPlacePtrWrapper< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlacePtrWrapper, t* >( "tLoadInPlacePtrWrapper", "t*", "mP", "public", offsetof( tLoadInPlacePtrWrapper, mP ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tLoadInPlacePtrWrapper< t >::gReflector = ::Sig::Rtti::fDefineReflector< tLoadInPlacePtrWrapper >( "tLoadInPlacePtrWrapper", tLoadInPlacePtrWrapper< t >::gBases, tLoadInPlacePtrWrapper< t >::gMembers );


//
// tLoadInPlaceRuntimeObject
template<class t>
const ::Sig::Rtti::tBaseClassDesc tLoadInPlaceRuntimeObject< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tLoadInPlaceRuntimeObject< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tLoadInPlaceRuntimeObject, tObjectBuffer >( "tLoadInPlaceRuntimeObject", "tObjectBuffer", "mObjectBuffer", "public", offsetof( tLoadInPlaceRuntimeObject, mObjectBuffer ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tLoadInPlaceRuntimeObject< t >::gReflector = ::Sig::Rtti::fDefineReflector< tLoadInPlaceRuntimeObject >( "tLoadInPlaceRuntimeObject", tLoadInPlaceRuntimeObject< t >::gBases, tLoadInPlaceRuntimeObject< t >::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\tRingBuffer.hpp]
//
#include "tRingBuffer.hpp"
namespace Sig
{

//
// tRingBuffer
template<class t>
const ::Sig::Rtti::tBaseClassDesc tRingBuffer< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tRingBuffer, tDynamicArray<t> >( "tRingBuffer", "tDynamicArray<t>", "protected" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tRingBuffer< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tRingBuffer, u32 >( "tRingBuffer", "u32", "mHead", "public", offsetof( tRingBuffer, mHead ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tRingBuffer, u32 >( "tRingBuffer", "u32", "mTail", "public", offsetof( tRingBuffer, mTail ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tRingBuffer, u32 >( "tRingBuffer", "u32", "mNumItems", "public", offsetof( tRingBuffer, mNumItems ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tRingBuffer< t >::gReflector = ::Sig::Rtti::fDefineReflector< tRingBuffer >( "tRingBuffer", tRingBuffer< t >::gBases, tRingBuffer< t >::gMembers );

}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tAabb.hpp]
//
#include "Math/tAabb.hpp"
namespace Sig
{
namespace Math
{

//
// tAabb
template<class t>
const ::Sig::Rtti::tBaseClassDesc tAabb< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tAabb< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tAabb, tVector3<t> >( "tAabb", "tVector3<t>", "mMin", "public", offsetof( tAabb, mMin ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tAabb, tVector3<t> >( "tAabb", "tVector3<t>", "mMax", "public", offsetof( tAabb, mMax ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tAabb< t >::gReflector = ::Sig::Rtti::fDefineReflector< tAabb >( "tAabb", tAabb< t >::gBases, tAabb< t >::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tFrustum.hpp]
//
#include "Math/tFrustum.hpp"
namespace Sig
{
namespace Math
{

//
// tFrustum
template<class t>
const ::Sig::Rtti::tBaseClassDesc tFrustum< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tFrustum< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tFrustum, tPlaneArray >( "tFrustum", "tPlaneArray", "mPlanes", "public", offsetof( tFrustum, mPlanes ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tFrustum< t >::gReflector = ::Sig::Rtti::fDefineReflector< tFrustum >( "tFrustum", tFrustum< t >::gBases, tFrustum< t >::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tMatrix.hpp]
//
#include "Math/tMatrix.hpp"
namespace Sig
{
namespace Math
{

//
// tMatrix3
template<class t>
const ::Sig::Rtti::tBaseClassDesc tMatrix3< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tMatrix3, tBaseVector<t, 12, tMatrix3<t> > >( "tMatrix3", "tBaseVector<t, 12, tMatrix3<t> >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tMatrix3< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMatrix3, tVector4<t> >( "tMatrix3", "tVector4<t>", "mRow0", "public", offsetof( tMatrix3, mRow0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMatrix3, tVector4<t> >( "tMatrix3", "tVector4<t>", "mRow1", "public", offsetof( tMatrix3, mRow1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMatrix3, tVector4<t> >( "tMatrix3", "tVector4<t>", "mRow2", "public", offsetof( tMatrix3, mRow2 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tMatrix3< t >::gReflector = ::Sig::Rtti::fDefineReflector< tMatrix3 >( "tMatrix3", tMatrix3< t >::gBases, tMatrix3< t >::gMembers );


//
// tMatrix4
template<class t>
const ::Sig::Rtti::tBaseClassDesc tMatrix4< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tMatrix4, tBaseVector<t, 16, tMatrix4<t> > >( "tMatrix4", "tBaseVector<t, 16, tMatrix4<t> >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tMatrix4< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tMatrix4, tVector4<t> >( "tMatrix4", "tVector4<t>", "mRow0", "public", offsetof( tMatrix4, mRow0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMatrix4, tVector4<t> >( "tMatrix4", "tVector4<t>", "mRow1", "public", offsetof( tMatrix4, mRow1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMatrix4, tVector4<t> >( "tMatrix4", "tVector4<t>", "mRow2", "public", offsetof( tMatrix4, mRow2 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tMatrix4, tVector4<t> >( "tMatrix4", "tVector4<t>", "mRow3", "public", offsetof( tMatrix4, mRow3 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tMatrix4< t >::gReflector = ::Sig::Rtti::fDefineReflector< tMatrix4 >( "tMatrix4", tMatrix4< t >::gBases, tMatrix4< t >::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tObb.hpp]
//
#include "Math/tObb.hpp"
namespace Sig
{
namespace Math
{

//
// tObb
template<class t>
const ::Sig::Rtti::tBaseClassDesc tObb< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tObb< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tObb, tVector3<t> >( "tObb", "tVector3<t>", "mCenter", "public", offsetof( tObb, mCenter ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tObb, tFixedArray<tVector3<t>,3> >( "tObb", "tFixedArray<tVector3<t>,3>", "mAxes", "public", offsetof( tObb, mAxes ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tObb, tVector3<t> >( "tObb", "tVector3<t>", "mExtents", "public", offsetof( tObb, mExtents ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tObb< t >::gReflector = ::Sig::Rtti::fDefineReflector< tObb >( "tObb", tObb< t >::gBases, tObb< t >::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tPlane.hpp]
//
#include "Math/tPlane.hpp"
namespace Sig
{
namespace Math
{

//
// tPlane
template<class t>
const ::Sig::Rtti::tBaseClassDesc tPlane< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tPlane, tBaseVector<t, 4, tPlane<t> > >( "tPlane", "tBaseVector<t, 4, tPlane<t> >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tPlane< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPlane, t >( "tPlane", "t", "a", "public", offsetof( tPlane, a ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPlane, t >( "tPlane", "t", "b", "public", offsetof( tPlane, b ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPlane, t >( "tPlane", "t", "c", "public", offsetof( tPlane, c ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPlane, t >( "tPlane", "t", "d", "public", offsetof( tPlane, d ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tPlane< t >::gReflector = ::Sig::Rtti::fDefineReflector< tPlane >( "tPlane", tPlane< t >::gBases, tPlane< t >::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tPRSXform.hpp]
//
#include "Math/tPRSXform.hpp"
namespace Sig
{
namespace Math
{

//
// tPRSXform
template<class t>
const ::Sig::Rtti::tBaseClassDesc tPRSXform< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tPRSXform< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tPRSXform, tVector3<t> >( "tPRSXform", "tVector3<t>", "mP", "public", offsetof( tPRSXform, mP ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPRSXform, t >( "tPRSXform", "t", "pad0", "public", offsetof( tPRSXform, pad0 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPRSXform, tQuaternion<t> >( "tPRSXform", "tQuaternion<t>", "mR", "public", offsetof( tPRSXform, mR ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPRSXform, tVector3<t> >( "tPRSXform", "tVector3<t>", "mS", "public", offsetof( tPRSXform, mS ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tPRSXform, t >( "tPRSXform", "t", "pad1", "public", offsetof( tPRSXform, pad1 ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tPRSXform< t >::gReflector = ::Sig::Rtti::fDefineReflector< tPRSXform >( "tPRSXform", tPRSXform< t >::gBases, tPRSXform< t >::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tQuaternion.hpp]
//
#include "Math/tQuaternion.hpp"
namespace Sig
{
namespace Math
{

//
// tQuaternion
template<class t>
const ::Sig::Rtti::tBaseClassDesc tQuaternion< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tQuaternion, tBaseVector<t, 4, tQuaternion<t> > >( "tQuaternion", "tBaseVector<t, 4, tQuaternion<t> >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tQuaternion< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tQuaternion, t >( "tQuaternion", "t", "x", "public", offsetof( tQuaternion, x ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tQuaternion, t >( "tQuaternion", "t", "y", "public", offsetof( tQuaternion, y ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tQuaternion, t >( "tQuaternion", "t", "z", "public", offsetof( tQuaternion, z ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tQuaternion, t >( "tQuaternion", "t", "w", "public", offsetof( tQuaternion, w ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tQuaternion< t >::gReflector = ::Sig::Rtti::fDefineReflector< tQuaternion >( "tQuaternion", tQuaternion< t >::gBases, tQuaternion< t >::gMembers );

}
}


//___________________________________________________________________________________________
// Generated from file [Src\Internal\Base\Math\tSphere.hpp]
//
#include "Math/tSphere.hpp"
namespace Sig
{
namespace Math
{

//
// tSphere
template<class t>
const ::Sig::Rtti::tBaseClassDesc tSphere< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tSphere< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tSphere, tVector3<t> >( "tSphere", "tVector3<t>", "mCenter", "public", offsetof( tSphere, mCenter ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tSphere, t >( "tSphere", "t", "mRadius", "public", offsetof( tSphere, mRadius ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tSphere< t >::gReflector = ::Sig::Rtti::fDefineReflector< tSphere >( "tSphere", tSphere< t >::gBases, tSphere< t >::gMembers );

}
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
// tVector1
template<class t>
const ::Sig::Rtti::tBaseClassDesc tVector1< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVector1, tBaseVector<t, 1, tVector1<t> > >( "tVector1", "tBaseVector<t, 1, tVector1<t> >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tVector1< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVector1, t >( "tVector1", "t", "x", "public", offsetof( tVector1, x ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tVector1< t >::gReflector = ::Sig::Rtti::fDefineReflector< tVector1 >( "tVector1", tVector1< t >::gBases, tVector1< t >::gMembers );


//
// tVector2
template<class t>
const ::Sig::Rtti::tBaseClassDesc tVector2< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVector2, tBaseVector<t, 2, tVector2<t> > >( "tVector2", "tBaseVector<t, 2, tVector2<t> >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tVector2< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVector2, t >( "tVector2", "t", "x", "public", offsetof( tVector2, x ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVector2, t >( "tVector2", "t", "y", "public", offsetof( tVector2, y ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tVector2< t >::gReflector = ::Sig::Rtti::fDefineReflector< tVector2 >( "tVector2", tVector2< t >::gBases, tVector2< t >::gMembers );


//
// tVector3
template<class t>
const ::Sig::Rtti::tBaseClassDesc tVector3< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVector3, tBaseVector<t, 3, tVector3<t> > >( "tVector3", "tBaseVector<t, 3, tVector3<t> >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tVector3< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVector3, t >( "tVector3", "t", "x", "public", offsetof( tVector3, x ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVector3, t >( "tVector3", "t", "y", "public", offsetof( tVector3, y ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVector3, t >( "tVector3", "t", "z", "public", offsetof( tVector3, z ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tVector3< t >::gReflector = ::Sig::Rtti::fDefineReflector< tVector3 >( "tVector3", tVector3< t >::gBases, tVector3< t >::gMembers );


//
// tVector4
template<class t>
const ::Sig::Rtti::tBaseClassDesc tVector4< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tVector4, tBaseVector<t, 4, tVector4<t> > >( "tVector4", "tBaseVector<t, 4, tVector4<t> >", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template<class t>
const ::Sig::Rtti::tClassMemberDesc tVector4< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tVector4, t >( "tVector4", "t", "x", "public", offsetof( tVector4, x ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVector4, t >( "tVector4", "t", "y", "public", offsetof( tVector4, y ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVector4, t >( "tVector4", "t", "z", "public", offsetof( tVector4, z ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc< tVector4, t >( "tVector4", "t", "w", "public", offsetof( tVector4, w ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template<class t>
const ::Sig::Rtti::tReflector tVector4< t >::gReflector = ::Sig::Rtti::fDefineReflector< tVector4 >( "tVector4", tVector4< t >::gBases, tVector4< t >::gMembers );

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
// tBinaryTemplateGraph
template< class t >
const ::Sig::Rtti::tBaseClassDesc tBinaryTemplateGraph< t >::gBases[]={
	::Sig::Rtti::fDefineBaseClassDesc< tBinaryTemplateGraph, tBinaryGraph >( "tBinaryTemplateGraph", "tBinaryGraph", "public" ),
	::Sig::Rtti::fDefineBaseClassDesc()
};
template< class t >
const ::Sig::Rtti::tClassMemberDesc tBinaryTemplateGraph< t >::gMembers[]={
	::Sig::Rtti::fDefineClassMemberDesc< tBinaryTemplateGraph, tDynamicArray<t> >( "tBinaryTemplateGraph", "tDynamicArray<t>", "mValueArray", "public", offsetof( tBinaryTemplateGraph, mValueArray ), 1, 0, 0 ),
	::Sig::Rtti::fDefineClassMemberDesc()
};
template< class t >
const ::Sig::Rtti::tReflector tBinaryTemplateGraph< t >::gReflector = ::Sig::Rtti::fDefineReflector< tBinaryTemplateGraph >( "tBinaryTemplateGraph", tBinaryTemplateGraph< t >::gBases, tBinaryTemplateGraph< t >::gMembers );

}
}

#endif//__Base_rtti__
