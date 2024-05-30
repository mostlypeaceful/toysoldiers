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

#ifdef target_tools

	///
	/// \brief This method is only required to overcome the problem of calling the
	/// templatized tClassIdGenerator::fGetClassId method below across multiple DLLs.
	/// See http://www.gamedev.net/community/forums/topic.asp?topic_id=479183 for
	/// a broader discussion.
	base_export tClassId fGetRuntimeClassId(const type_info& ti);

#endif//target_tools

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

		static inline tClassId fGetClassId( )
		{
#ifdef target_tools

			// unfortunately we have to resort to c++'s built-in rtti 
			// for this to be safe across dll boundaries
			return fGetRuntimeClassId(typeid(t));

#else//target_tools not defined

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
