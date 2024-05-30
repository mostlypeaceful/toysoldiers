#ifndef __RttiDetail__
#define __RttiDetail__
#ifndef __Rtti__
#error This file should only be included via Rtti.hpp
#endif//__Rtti__

///
/// The rtti_detail namespace is NOT intended for general purpose use; 
/// should only by referenced internally within the rtti module (rtti.hpp/.cpp).
namespace Sig { namespace Rtti { namespace Private 
{
	typedef u32 tClassId;

	typedef void* (*tClassNew)( b32 noopConstruct );
	typedef void* (*tClassNewInPlace)( void* p, b32 noopConstruct );

	struct base_export tClassAllocFunctions
	{
		inline tClassAllocFunctions( tClassNew cn=0, tClassNewInPlace cnip=0 ) : mClassNew(cn), mClassNewInPlace(cnip) { }
		tClassNew				mClassNew;
		tClassNewInPlace		mClassNewInPlace;
	};

	static const tClassId cInvalidClassId				= 0x00000000;
	static const tClassId cBaseSerializableClassId		= 0x80000000;

#if defined( target_tools ) || defined( platform_metro )
	/// \brief This method is only required to overcome the problem of calling the
	/// templatized tClassIdGenerator::fGetClassId method below across multiple DLLs.
	/// See http://www.gamedev.net/community/forums/topic.asp?topic_id=479183 for
	/// a broader discussion.
	base_export void fGetRuntimeClassId( tClassId& cid, const type_info& ti);

#elif defined( build_debug )
	/// \brief Meant for debug use only.  Ensures this type hasn't been
	/// registered before in a (hopefully) thread safe manner.
	base_export void fDebugRegisterClassId( b32& notFirstinit, b32& unique, const std::type_info& ti, void* uuid);

#endif // defined( target_tools ) || defined( platform_metro )

	///
	/// This general template provides class ids for non-serializable
	/// classes (i.e., the class id, while unique, is generated at run-time,
	/// and can vary depending on program execution and compiler).
	template<class t, bool isRttiSubClass>
	class tClassIdGenerator
	{
	private:
		static u32 gCidObject;
	public:

		static tClassId fGetClassId( )
		{
			// What a mess! This is because:
			// 1) We want this to be fast
			// 2) DP0 Metro needs this to be a DLL, and Tools use it as a DLL
			// 3) Template static vars aren't unique when we use DLLs
			// 4) Threads, threads everywhere.
			// 
			// Hopefully this is the last time I need to fix this.
			// 
			// We pass our static stuff into various implementation
			// functions to avoid needing to drag in tMutex.hpp -- in part
			// because we're in the PCH.
			//
			// N.B. Static initialization isn't thread safe, so we *zero*-init
			// our stuff first (twice if I'm correct -- once safely and
			// automagically per the C++ standard's rules on static data
			// initialization, once per our explicit initialization for the
			// sake of clarity.)
			// 
			// We then pass our static vars off to other functions to avoid
			// dragging in e.g. tMutex et all into this header, which is
			// used in the PCH, and would lead to inclusion ordering issues
			//
			// Of course, we have one final case where we're a game AND
			// statically linked AND don't really need to cache anything.
			// No extra functions or locks needed then.
			// 
			// We abuse gCidObject to "generate" a unique address when we
			// are assumed to link Base statically.  I worry this could
			// potentially conflict with RttiGen generated IDs...?
			
#if defined( target_tools ) || defined( platform_metro )

			// Assume we're building/consuming base as
			// a DLL and need to use runtime info:

			static tClassId rcid = 0; // N.B. *Zero* initialized
			fGetRuntimeClassId(rcid,typeid(t));

			if_assert( tClassId rcid2 = 0; )
			if_assert( fGetRuntimeClassId(rcid2,typeid(t)); )
			sigassert( rcid == rcid2 ); // Ensure fGetRuntimeClassId isn't broken horribly in some fashion.

			return rcid;

#elif defined( build_debug )

			// Assume we're intended to build/consume base as a static
			// library, and double check that things aren't breaking since
			// this is a debug build:

			static b32 notFirstInit = false; // N.B. *Zero* initialized
			static b32 unique = false; // N.B. *Zero* initialized
			fDebugRegisterClassId( notFirstInit, unique, typeid(t), &gCidObject );

			return ( tClassId )&gCidObject;
#else

			// Assume we're intended to build/consume base as a static
			// library.  No debug checks this time, speed speed speed!

			return ( tClassId )&gCidObject;

#endif//target_tools

		}
	};
	template<class t, bool isRttiSubClass>
	u32 tClassIdGenerator<t,isRttiSubClass>::gCidObject;


	///
	/// This partial template specialization provides class ids for serializable
	/// classes (really, it just returns the id that has already been specified
	/// as part of the class).
	template<class t>
	class tClassIdGenerator<t,true>
	{
	public:
		static inline tClassId fGetClassId( ) { return t::cClassId; }
	};


}}}


#endif//__RttiDetail__
