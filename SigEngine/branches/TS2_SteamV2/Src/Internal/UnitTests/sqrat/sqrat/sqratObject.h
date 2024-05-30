//
// SqratObject: Referenced Squirrel Object Wrapper
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

#if !defined(_SCRAT_OBJECT_H_)
#define _SCRAT_OBJECT_H_

#include <squirrel.h>
#include <string.h>
#include "SqratTypes.h"

namespace Sqrat {

	class DefaultVM {
	private:
		static HSQUIRRELVM& staticVm() {
			static HSQUIRRELVM vm;
			return vm;
		}
	public:
		static HSQUIRRELVM Get() {
			return staticVm();
		}
		static void Set(HSQUIRRELVM vm) {
			staticVm() = vm;
		}
	};

	class Object {
	protected:
		HSQUIRRELVM vm;
		HSQOBJECT obj;
		bool release;

		Object(HSQUIRRELVM v, bool releaseOnDestroy = true) : vm(v), release(releaseOnDestroy) {
			sq_resetobject(&obj);
		}

	public:
		Object() {
			sq_resetobject(&obj);
		}

		Object(const Object& so) : vm(so.vm), obj(so.obj) {
			sq_addref(vm, &obj);
		}

		Object(HSQOBJECT o, HSQUIRRELVM v = DefaultVM::Get()) : vm(v), obj(o) {
			sq_addref(vm, &obj);
		}

		template<class T>
		Object(T* instance, HSQUIRRELVM v = DefaultVM::Get()) : vm(v) {
			ClassType<T>::PushInstance(vm, instance);
			sq_getstackobj(vm, -1, &obj);
			sq_addref(vm, &obj);
		}

		virtual ~Object() {
			if(release) {
				Release();
			}
		}

		Object& operator=(const Object& so) {
			Release();
			vm = so.vm;
			obj = so.obj;
			sq_addref(vm, &GetObject());
			return *this;
		}

		HSQUIRRELVM& GetVM() {
			return vm;
		}

		HSQUIRRELVM GetVM() const {
			return vm;
		}

		SQObjectType GetType() const {
			return GetObject()._type;
		}

		bool IsNull() const {
			return sq_isnull(GetObject());
		}

		virtual HSQOBJECT GetObject() const {
			return obj;
		}

		virtual HSQOBJECT& GetObject() {
			return obj;
		}

		operator HSQOBJECT&() {
			return GetObject();
		}

		void Release() {
			sq_release(vm, &obj);
		}

		SQUserPointer GetInstanceUP(SQUserPointer tag = NULL) const {
			SQUserPointer up;
			sq_pushobject(vm, GetObject());
			sq_getinstanceup(vm, -1, &up, tag);
			sq_pop(vm, 1);
			return up;
		}

		Object GetSlot(const SQChar* slot) const {
			HSQOBJECT slotObj;
			sq_pushobject(vm, GetObject());
			sq_pushstring(vm, slot, -1);
			if(SQ_FAILED(sq_get(vm, -2))) {
				sq_pop(vm, 1);
				return Object(vm); // Return a NULL object
			} else {
				sq_getstackobj(vm, -1, &slotObj);
				sq_pop(vm, 2);
				return Object(slotObj, vm);
			}
		}

		template <class T>
		T Cast() const {
			sq_pushobject(vm, GetObject());
			T ret = Var<T>(vm, -1).value;
			sq_pop(vm, 1);
			return ret;
		}

	protected:
		// Bind a function and it's associated Squirrel closure to the object
		inline void BindFunc(const SQChar* name, void* method, size_t methodSize, SQFUNCTION func, bool staticVar = false) {
			sq_pushobject(vm, GetObject());
			sq_pushstring(vm, name, -1);

			SQUserPointer methodPtr = sq_newuserdata(vm, static_cast<SQUnsignedInteger>(methodSize));
			memcpy(methodPtr, method, methodSize);

			sq_newclosure(vm, func, 1);
			sq_newslot(vm, -3, staticVar);
			sq_pop(vm,1); // pop table
		}

		// Set the value of a variable on the object. Changes to values set this way are not reciprocated
		template<class V>
		inline void BindValue(const SQChar* name, const V& val, bool staticVar = false) {
			sq_pushobject(vm, GetObject());
			sq_pushstring(vm, name, -1);
			PushVar(vm, val);
			sq_newslot(vm, -3, staticVar);
			sq_pop(vm,1); // pop table
		}

		// Set the value of an instance on the object. Changes to values set this way are reciprocated back to the source instance
		template<class V>
		inline void BindInstance(const SQChar* name, V* val, bool staticVar = false) {
			sq_pushobject(vm, GetObject());
			sq_pushstring(vm, name, -1);
			PushVar(vm, val);
			sq_newslot(vm, -3, staticVar);
			sq_pop(vm,1); // pop table
		}
	};

	//
	// Overridden Getter/Setter
	//

	template<>
	struct Var<Object> {
		Object value;
		Var(HSQUIRRELVM vm, SQInteger idx) {
			HSQOBJECT sqValue;
			sq_getstackobj(vm, idx, &sqValue);
			value = Object(sqValue, vm);
		}
		static void push(HSQUIRRELVM vm, Object& value) {
			sq_pushobject(vm, value.GetObject());
		}
	};
}

#endif
