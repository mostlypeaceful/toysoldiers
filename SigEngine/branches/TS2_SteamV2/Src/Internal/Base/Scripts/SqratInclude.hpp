#ifndef __SqratInclude__
#define __SqratInclude__
#include <squirrel.h>
#include "sqrat.h"

namespace Sqrat
{
#define sqrat_define_string_var2(tTemplateType, tStringType) \
	template<> \
	struct Var<tTemplateType> { \
		tStringType value; \
		inline tStringType& Value() { \
			return value; \
		} \
		Var(HSQUIRRELVM vm, SQInteger idx) { \
			const SQChar* ret; \
			sq_tostring(vm, idx); \
			sq_getstring(vm, -1, &ret); \
			value = tStringType(ret); \
			sq_pop(vm,1); \
		} \
		static void push(HSQUIRRELVM vm, const tStringType& value) { \
			sq_pushstring(vm, value.fCStr(), -1); \
		} \
	};

#define sqrat_define_string_var(tStringType) \
	sqrat_define_string_var2(tStringType, tStringType)\
	sqrat_define_string_var2(tStringType&, tStringType)\
	sqrat_define_string_var2(const tStringType&, tStringType)

	sqrat_define_string_var(Sig::tStringPtr)
	sqrat_define_string_var(Sig::tFilePathPtr)

#define sqrat_define_refcounterptr_var2(tTemplateType, tClassTypePtr) \
	template<> \
	struct Var<tTemplateType> { \
		tClassTypePtr value; \
		Var(HSQUIRRELVM vm, SQInteger idx) { \
			value = tClassTypePtr( ClassType<tClassTypePtr::tValueType>::GetInstance(vm, idx) ); \
		} \
		static void push(HSQUIRRELVM vm, const tClassTypePtr& value) { \
			ClassType<tClassTypePtr::tValueType>::PushInstance(vm, value.fGetRawPtr( )); \
		} \
	};

#define sqrat_define_refcounterptr_var(tClassTypePtr) \
	sqrat_define_refcounterptr_var2(tClassTypePtr, tClassTypePtr)\
	sqrat_define_refcounterptr_var2(tClassTypePtr&, tClassTypePtr)\
	sqrat_define_refcounterptr_var2(const tClassTypePtr&, tClassTypePtr)
}

namespace Sig
{
	template<class tCppType>
	class tScriptObjectPtr
	{
	protected:
		Sqrat::Object	mScriptObject;
		tCppType*		mCachedCppObject; // points to the object stored in script (bound up in mScriptObject)
	public:
		inline tScriptObjectPtr( ) : mCachedCppObject( 0 ) { }
		inline explicit tScriptObjectPtr( const Sqrat::Object& o ) : mScriptObject( o ), mCachedCppObject( o.IsNull( ) ? 0 : o.Cast< tCppType* >( ) ) { }
		inline const Sqrat::Object& fScriptObject( ) const { return mScriptObject; }
		inline b32 fIsNull( ) const { return mScriptObject.IsNull( ); }
		inline tCppType* fCodeObject( ) const { return mCachedCppObject; }
		inline b32 operator==( const tScriptObjectPtr& other ) const { return fCodeObject( ) == other.fCodeObject( ); }
	};


	template<class tCppType>
	class tScriptOrCodeObjectPtr : public tScriptObjectPtr<tCppType>
	{
	public:
		tScriptOrCodeObjectPtr( ) { }
		explicit tScriptOrCodeObjectPtr( const Sqrat::Object& o ) : tScriptObjectPtr<tCppType>( o ) { }
		explicit tScriptOrCodeObjectPtr( tCppType* p )
		{
			tScriptObjectPtr<tCppType>::mCachedCppObject = p;
			fOnCopyCodeOwned( );
		}
		tScriptOrCodeObjectPtr( const tScriptOrCodeObjectPtr& other ) { fCopy( other ); }
		tScriptOrCodeObjectPtr& operator=( const tScriptOrCodeObjectPtr& other )
		{
			if( &other != this )
			{
				fDestroy( );
				fCopy( other );
			}
			return *this;
		}
		~tScriptOrCodeObjectPtr( ) { fDestroy( ); }
		b32 fIsNull( ) const { return !tScriptObjectPtr<tCppType>::mCachedCppObject; }
		b32 fIsCodeOwned( ) const { return tScriptObjectPtr<tCppType>::mCachedCppObject && tScriptObjectPtr<tCppType>::mScriptObject.IsNull( ); }

	private:
		void fDestroy( )
		{
			if( fIsCodeOwned( ) )
				fRefCounterPtrDecRef( tScriptObjectPtr<tCppType>::mCachedCppObject );
			tScriptObjectPtr<tCppType>::mCachedCppObject = 0;
			tScriptObjectPtr<tCppType>::mScriptObject = Sqrat::Object( );
		}
		void fCopy( const tScriptOrCodeObjectPtr& other )
		{
			tScriptObjectPtr<tCppType>::mCachedCppObject = other.mCachedCppObject;
			tScriptObjectPtr<tCppType>::mScriptObject = other.mScriptObject;
			fOnCopyCodeOwned( );
		}
		void fOnCopyCodeOwned( )
		{
			if( fIsCodeOwned( ) )
				fRefCounterPtrAddRef( tScriptObjectPtr<tCppType>::mCachedCppObject );
		}
	};
}

#endif//__SqratInclude__