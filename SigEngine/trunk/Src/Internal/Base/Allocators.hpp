//------------------------------------------------------------------------------
// \file Allocators.hpp - 06 Aug 2013
// \author cbramwell
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __Allocators__
#define __Allocators__
#include "IAllocator.hpp"

namespace Sig { namespace Allocators
{
	extern IAllocator* gMalloc;
	extern IAllocator* gScript;
	extern IAllocator* gFui;

	void fInit();
	void fShutdown();
}}//Sig::Allocators

#endif//__Allocators__
