#ifndef __tFastVector__
#define __tFastVector__

namespace Sig { namespace Math
{

#define FAST_VEC_TYPE2 tVector2
#define FAST_VEC_TYPE3 tVector3

#define FAST_VEC_SIZE 1
#define FAST_VEC_TYPE tVector1
#define FAST_VEC_XYZW( x, y, z, w ) x
#define FAST_VEC_XYZW_C( x, y, z, w ) x

#include "tFastVectorImp.hpp"

#undef FAST_VEC_SIZE
#undef FAST_VEC_TYPE
#undef FAST_VEC_XYZW
#undef FAST_VEC_XYZW_C

//////////////////////////////////////////////////////////////////////////////////////////////////

#define FAST_VEC_SIZE 2
#define FAST_VEC_TYPE tVector2
#define FAST_VEC_XYZW( x, y, z, w ) x y
#define FAST_VEC_XYZW_C( x, y, z, w ) x, y

#include "tFastVectorImp.hpp"

#undef FAST_VEC_SIZE
#undef FAST_VEC_TYPE
#undef FAST_VEC_XYZW
#undef FAST_VEC_XYZW_C

//////////////////////////////////////////////////////////////////////////////////////////////////

#define FAST_VEC_SIZE 3
#define FAST_VEC_TYPE tVector3
#define FAST_VEC_XYZW( x, y, z, w ) x y z
#define FAST_VEC_XYZW_C( x, y, z, w ) x, y, z

#include "tFastVectorImp.hpp"

#undef FAST_VEC_SIZE
#undef FAST_VEC_TYPE
#undef FAST_VEC_XYZW
#undef FAST_VEC_XYZW_C

//////////////////////////////////////////////////////////////////////////////////////////////////

#define FAST_VEC_SIZE 4
#define FAST_VEC_TYPE tVector4
#define FAST_VEC_XYZW( x, y, z, w ) x y z w
#define FAST_VEC_XYZW_C( x, y, z, w ) x, y, z, w

#include "tFastVectorImp.hpp"

#undef FAST_VEC_SIZE
#undef FAST_VEC_TYPE
#undef FAST_VEC_XYZW
#undef FAST_VEC_XYZW_C

#undef FAST_VEC_TYPE2
#undef FAST_VEC_TYPE3

} }


#endif//__tFastVector__
