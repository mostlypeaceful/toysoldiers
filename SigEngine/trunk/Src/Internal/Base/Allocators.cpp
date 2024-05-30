//------------------------------------------------------------------------------
// \file Allocators.cpp - 06 Aug 2013
// \author cbramwell
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tMallocAllocator.hpp"
#include "tExactFitAllocator.hpp"

namespace Sig { namespace Allocators
{
	//------------------------------------------------------------------------------
	IAllocator* gMalloc = NULL;
	IAllocator* gScript = NULL;
	IAllocator* gFui = NULL;
	static tRefCounterPtr<IAllocator> mMallocPtr;
	static tRefCounterPtr<IAllocator> mScriptPtr;
	static tRefCounterPtr<IAllocator> mFuiPtr;
	//------------------------------------------------------------------------------


	//------------------------------------------------------------------------------
	void fInit()
	{
		sigcheckfail( !gMalloc && !gScript && !gFui, return );

		mMallocPtr.fReset( NEW_TYPED( tMallocAllocator )( ) );
		gMalloc = mMallocPtr.fGetRawPtr( );

		mScriptPtr = mMallocPtr;
		gScript = mScriptPtr.fGetRawPtr( );

		mFuiPtr.fReset( NEW_TYPED( tExactFitAllocator )( "fui", gMalloc ) );
		gFui = mFuiPtr.fGetRawPtr( );
	}

	//------------------------------------------------------------------------------
	void fShutdown()
	{
		gFui = NULL;
		gScript = NULL;
		gMalloc = NULL;
		mFuiPtr.fRelease();
		mScriptPtr.fRelease();
		mMallocPtr.fRelease();
	}
	
}}//Sig::Allocators
