//------------------------------------------------------------------------------
// \file LocalStorage.hpp - 13 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __LocalStorage__
#define __LocalStorage__

namespace Sig { namespace Threads { namespace LocalStorage
{
	extern const u32 cOutOfIndices;

	u32 Allocate( );
	b32 Free( u32 index );
	b32 SetValue( u32 index, void * value );
	void * GetValue( u32 index );

} } }

#endif//__LocalStorage__
