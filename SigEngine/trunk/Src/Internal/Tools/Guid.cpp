//------------------------------------------------------------------------------
// \file Guid.cpp - 04 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "Guid.hpp"

namespace Sig
{
	u32 fGenerateGuid( )
	{
		GUID testGuid;
		CoCreateGuid( &testGuid );
		OLECHAR string[128];
		StringFromGUID2( testGuid, string, 128 );

		char buffer[128] = {0};
		for( u32 i = 0; string[i] != 0; ++i )
			buffer[i] = string[i];

		return Sig::Hash::fGenericHash( (const Sig::byte*)buffer, Strlen(buffer), ~0 );
	}
}
