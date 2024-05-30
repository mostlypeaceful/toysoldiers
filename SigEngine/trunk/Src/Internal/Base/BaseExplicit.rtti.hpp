#ifndef __BaseExplicit_rtti__
#define __BaseExplicit_rtti__

//___________________________________________________________________________________________
// From file [Src\Internal\Base\tArray.hpp]
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
	
}


//___________________________________________________________________________________________
// From file [Src\Internal\Base\Math\tVector.hpp]
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
#endif//__BaseExplicit_rtti__
