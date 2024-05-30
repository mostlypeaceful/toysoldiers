//
// SqratUtil: Squirrel Utilities
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

#if !defined(_SCRAT_UTIL_H_)
#define _SCRAT_UTIL_H_

#include <squirrel.h>
#include <string.h>

#include "SqratTypes.h"

namespace Sqrat {

	class Exception {
	public:
		Exception(const string& msg) : message(msg) {}
		Exception(const Exception& ex) : message(ex.message) {}
		
		const string Message() const {
			return message;
		}

	private:
		string message;
	};

	inline string LastErrorString( HSQUIRRELVM vm ) {
		const SQChar* sqErr;
		sq_getlasterror(vm);
		if(sq_gettype(vm, -1) == OT_NULL) {
			return string();
		}
		sq_tostring(vm, -1);
		sq_getstring(vm, -1, &sqErr);
		return string(sqErr);
	}

}

#endif
