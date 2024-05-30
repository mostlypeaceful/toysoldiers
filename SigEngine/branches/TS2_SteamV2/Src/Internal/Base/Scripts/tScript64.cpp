//------------------------------------------------------------------------------
// \file tScript64.cpp - 03 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tScript64.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	void tScript64::fExportScriptInterface( tScriptVm & vm )
	{
		Sqrat::Class<tScript64, Sqrat::DefaultAllocator<tScript64> > classDesc( vm.fSq( ) );
		vm.fRootTable( ).Bind(_SC("Script64"), classDesc);
	}
}
