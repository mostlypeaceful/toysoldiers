#ifndef __Rtti__
#define __Rtti__
#include "RttiDetail.hpp"

namespace Sig { 

///
/// Rtti is a global namespace-style providing:
/// - unique class id access via a single function; note this will auto-detect whether the class has an
///		embedded serializable class id or not, and return the appropriate one
/// - class creation using tClassId (facilitates serializing and reconstruction of derived object hierarchies)
namespace Rtti
{

	///
	/// The building block of the Rtti system, tClassId is the basic numeric type represnting a unique
	/// class identifier.
	typedef Private::tClassId							tClassId;
	
	static const tClassId cInvalidClassId				= Private::cInvalidClassId;
	static const tClassId cBaseSerializableClassId		= Private::cBaseSerializableClassId;
	
	class tReflector;
	class tBaseClassDesc;
	class tClassMemberDesc;
	
	///
	/// \brief Stores information pertaining to a class, such that you can traverse all its
	/// members, all its bases, and generally "walk" the layout of the class. Essential as
	/// part of the automatic serialization system.
	class base_export tReflector
	{
		u32							mClassSizeof;
		Rtti::tClassId				mCid;
		const char*					mType;
		b32							mIsBuiltIn;
		b32							mIsPolymorphic;
		const tBaseClassDesc*		mBaseClasses;
		const tClassMemberDesc*		mClassMembers;
		
	public:
		
		tReflector( )
		{
			fZeroOut( this );
			mCid = cInvalidClassId;
		}
		
		tReflector( 
				   u32 cs, 
				   Rtti::tClassId cid, 
				   const char* type, 
				   b32 isBuiltIn,
				   b32 isPolymorphic, 
				   const tBaseClassDesc* baseClassesArray, 
				   const tClassMemberDesc* membersArray )
		: mClassSizeof( cs )
		, mCid( cid )
		, mType( type )
		, mIsBuiltIn( isBuiltIn )
		, mIsPolymorphic( isPolymorphic )
		, mBaseClasses( baseClassesArray )
		, mClassMembers( membersArray )
		{
		}
		
		inline u32						fClassSizeof( )		const { return mClassSizeof; }
		inline Rtti::tClassId			fClassId( )			const { return mCid; }
		inline const char*				fTypeName( )		const { return mType; }
		inline b32						fIsBuiltIn( )		const { return mIsBuiltIn; }
		inline b32						fIsPolymorphic( )	const { return mIsPolymorphic; }
		inline const tBaseClassDesc*	fBaseClasses( )		const { return mBaseClasses; }
		inline const tClassMemberDesc*	fClassMembers( )	const { return mClassMembers; }
		
		b32 fContainsPointers( ) const;
	};	
	
	///
	/// \brief Macro used for declaring a class reflector object. Additionally, this macro is used as a tag
	/// in the Sig code preprocessor; if this tag is found in the class declaration, complete reflection
	/// information will be auto-generated and placed in the appropriate .cpp/.hpp files.
#	define declare_reflector( ) \
		private:	static const ::Sig::Rtti::tReflector gReflector; \
		private:	static const ::Sig::Rtti::tBaseClassDesc gBases[]; \
		private:	static const ::Sig::Rtti::tClassMemberDesc gMembers[]; \
		public:		static inline const ::Sig::Rtti::tReflector& fGetReflector( ) { return gReflector; }
	
#	define declare_null_reflector( ) \
		public:		static inline const ::Sig::Rtti::tReflector& fGetReflector( ) { static ::Sig::Rtti::tReflector gReflector; return gReflector; }
	
	
	///
	/// tBaseClass is the base of all polymorphic types desiring useful
	/// virtual functionality for accessing the derived class id and sizeof
	class base_export tBaseClass
	{
		declare_null_reflector( );

	public:

		virtual						~tBaseClass( )				{ }

		///
		/// \brief Allows for querying of the type of the fully derived class from a base pointer.
		virtual tClassId			fClassId( )			const	= 0;

		///
		/// \brief Allows for querying sizeof the type of the fully derived class from a base pointer.
		virtual u32					fClassSizeof( )		const	= 0;
	};

	///
	/// tSerializableBaseClass is the base class for all classes requiring
	/// a serializable tClassId (i.e., objects can be serialized and instantiated at run-time,
	/// either over a network or from file, etc.).
	class base_export tSerializableBaseClass : public tBaseClass
	{
		declare_null_reflector( );

	public:

		///
		/// Derived classes must implement this, generally through the declare_polymorphic_reflector( )
		/// macro, which will automatically return the proper reflector object for your derived type.
		virtual const tReflector&	fGetDerivedReflector( ) const = 0;
	};

	

	///
	/// \brief Get the unique tClassId corresponding to the specified class.
	///
	/// For types that derive from rtti_class_derived<t,id>, this function will return 
	/// the unique serializable tClassId; for all other types, it will return a run-time 
	/// generated unique id.  relies on the the internal tClassIdGenerator template class 
	/// for resolving the class id.
	template<class t>
	inline tClassId fGetClassId( )
	{
		return Private::tClassIdGenerator< const t, can_convert( t, tSerializableBaseClass ) >::fGetClassId( );
	}

	///
	/// \brief Convenience method that accepts an objects (so you don't have to use the <> version above).
	template<class t>
	inline tClassId fGetClassId( const t& object )
	{
		return fGetClassId<t>( );
	}

}}

#ifdef sig_logging
#	define debug_type_name( className ) virtual const char*	fDebugTypeName( ) const { return #className; }
#	define static_cast_func( ) template<class t> inline t* fStaticCast( ) const { t* p = fDynamicCast<t>( ); sigassert( p ); return p; }
#else
#	define debug_type_name( className ) inline const char* fDebugTypeName( ) const { return ""; }
#	define static_cast_func( ) template<class t> inline t* fStaticCast( ) const { return ( t* )static_cast<const t*>( this ); }
#endif//sig_devmenu

#define define_dynamic_cast_base( className ) \
	public: \
		debug_type_name( className ) \
		static_cast_func( ) \
		virtual void* fDynamicCastInternal( const Rtti::tClassId& cid ) const { return ( cid == Rtti::fGetClassId( *this ) ) ? ( void* )this : 0; } \
		template<class t> \
		inline t* fDynamicCast( ) const { return ( t* )fDynamicCastInternal( Rtti::fGetClassId<t>( ) ); } \
		template<class t> \
		inline t& fDynamicCastRef( ) const { \
			t* p = ( t* )fDynamicCastInternal( Rtti::fGetClassId<t>( ) ); \
			sigassert( p ); \
			return *p; \
		}

#define define_dynamic_cast( className, baseClassName ) \
	public: \
		debug_type_name( className ) \
		virtual void* fDynamicCastInternal( const Rtti::tClassId& cid ) const \
		{ \
			if( cid == Rtti::fGetClassId( *this ) ) \
				return ( void* )this; \
			return baseClassName::fDynamicCastInternal( cid ); \
		}

#define define_dynamic_cast_double( className, baseClassName, alternateBaseClass ) \
	public: \
		debug_type_name( className ) \
		virtual void* fDynamicCastInternal( const Rtti::tClassId& cid ) const \
		{ \
			if( cid == Rtti::fGetClassId( *this ) ) \
				return ( void* )this; \
			if( cid == Rtti::fGetClassId<alternateBaseClass>( ) ) \
				return ( void* )static_cast<const alternateBaseClass*>( this ); \
			return baseClassName::fDynamicCastInternal( cid ); \
		}

#define define_dynamic_cast_triple( className, baseClassName, alternateBaseClass0, alternateBaseClass1 ) \
	public: \
		debug_type_name( className ) \
		virtual void* fDynamicCastInternal( const Rtti::tClassId& cid ) const \
		{ \
			if( cid == Rtti::fGetClassId( *this ) ) \
				return ( void* )this; \
			if( cid == Rtti::fGetClassId<alternateBaseClass0>( ) ) \
				return ( void* )static_cast<const alternateBaseClass0*>( this ); \
			if( cid == Rtti::fGetClassId<alternateBaseClass1>( ) ) \
				return ( void* )static_cast<const alternateBaseClass1*>( this ); \
			return baseClassName::fDynamicCastInternal( cid ); \
		}

#endif//__Rtti__
