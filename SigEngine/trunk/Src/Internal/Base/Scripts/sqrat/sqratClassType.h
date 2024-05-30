//
// SqratClassType: Type Translators
//

//
// Copyright (c) 2009 Brandon Jones
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//	1. The origin of this software must not be misrepresented; you must not
//	claim that you wrote the original software. If you use this software
//	in a product, an acknowledgment in the product documentation would be
//	appreciated but is not required.
//
//	2. Altered source versions must be plainly marked as such, and must not be
//	misrepresented as being the original software.
//
//	3. This notice may not be removed or altered from any source
//	distribution.
//

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!! THIS FILE HAS BEEN MODIFIED !!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 
//    Jan 31, 2012:
//    
// After coming down with a severe case of "we can't simultaneously use
// WinRT and make static libraries", GameApp crashes because SQRat assumes
// there's only one instance of static variables declared with the static
// function trick.  Fixing this requires base_export, and in the case of
// ClassType<C>, extra initialization shenanigans.  --mrickert

#if !defined(_SCRAT_CLASSTYPE_H_)
#define _SCRAT_CLASSTYPE_H_

#include <squirrel.h>
#include <typeinfo>

namespace Sqrat {
	///
	/// \brief The aforementioned shenanigans!  Holds all the static data
	/// associated with a ClassType<C> so it can be shared across DLL
	/// boundaries.
	class StaticClassTypeData
	{
	private:
		/// \brief Get class static data by name.
		/// Consider abusing the address of T::gReflector instead?
		//base_export static StaticClassTypeData& fGetStaticClassTypeDataByName( const char* typeName );

		/// \brief Get class static data by class id.
		base_export static StaticClassTypeData& fGetStaticClassTypeDataByClassId( Sig::Rtti::tClassId cid );
	public:
		typedef SQInteger (*COPYFUNC)(HSQUIRRELVM, SQInteger, const void*);

		HSQOBJECT	mClassObj;
		HSQOBJECT	mGetTable;
		HSQOBJECT	mSetTable;
		COPYFUNC	mCopyFunc;
		bool		mInitialized;

		template < typename T >
		static StaticClassTypeData& fFor()
		{
			static const Sig::Rtti::tClassId staticId = Sig::Rtti::fGetClassId<T>();
			//static const char* staticName = typeid(T).name();
			static StaticClassTypeData& staticData = fGetStaticClassTypeDataByClassId(staticId);

#if defined( sig_assert )
			const Sig::Rtti::tClassId currentId = Sig::Rtti::fGetClassId<T>();
			//const char* currentName = typeid(T).name();
			StaticClassTypeData& currentData = fGetStaticClassTypeDataByClassId(currentId);
#endif

			sigassert( &staticData == &currentData );

			return staticData;
		}
	};


	//
	// ClassType
	//

	template<class C>
	struct ClassType {
#if defined( platform_metro ) // DLLs and shit, use workaround
		// Get the Squirrel Object for this Class (Modified)
		static inline HSQOBJECT& ClassObject() {
			return StaticClassTypeData::fFor<C>().mClassObj;
		}

		// Get the Get Table for this Class (Modified)
		static inline HSQOBJECT& GetTable() {
			return StaticClassTypeData::fFor<C>().mGetTable;
		}

		// Get the Set Table for this Class (Modified)
		static inline HSQOBJECT& SetTable() {
			return StaticClassTypeData::fFor<C>().mSetTable;
		}

		// Get the Copy Function for this Class
		typedef SQInteger (*COPYFUNC)(HSQUIRRELVM, SQInteger, const void*);

		static inline COPYFUNC& CopyFunc() {
			return StaticClassTypeData::fFor<C>().mCopyFunc; // (Modified)
		}

		static inline bool& Initialized() {
			return StaticClassTypeData::fFor<C>().mInitialized; // (Modified)
		}
#else
		// Get the Squirrel Object for this Class
		static inline HSQOBJECT& ClassObject() {
			static HSQOBJECT classObj; 
			return classObj;
		}

		// Get the Get Table for this Class
		static inline HSQOBJECT& GetTable() {
			static HSQOBJECT getTable; 
			return getTable;
		}

		// Get the Set Table for this Class
		static inline HSQOBJECT& SetTable() {
			static HSQOBJECT setTable; 
			return setTable;
		}

		// Get the Copy Function for this Class
		typedef SQInteger (*COPYFUNC)(HSQUIRRELVM, SQInteger, const void*);

		static inline COPYFUNC& CopyFunc() {
			static COPYFUNC copyFunc; 
			return copyFunc;
		}

		static inline bool& Initialized() {
			static bool initialized = false;
			return initialized;
		}
#endif

		static void PushInstance(HSQUIRRELVM vm, C* ptr) {
			sq_pushobject(vm, ClassObject());
			sq_createinstance(vm, -1);
			sq_remove(vm, -2);
			sq_setinstanceup(vm, -1, ptr);
		}

		static void PushInstanceCopy(HSQUIRRELVM vm, C& value) {
			sq_pushobject(vm, ClassObject());
			sq_createinstance(vm, -1);
			sq_remove(vm, -2);
			CopyFunc()(vm, -1, &value);
		}

		static C* GetInstance(HSQUIRRELVM vm, SQInteger idx) {
			C* ptr = NULL;
			sq_getinstanceup(vm, idx, (SQUserPointer*)&ptr, NULL);
			return ptr;
		}
	};
	
}

#endif
