//------------------------------------------------------------------------------
// \file tInterlocked.hpp - 8 Feb 2012
// \author mrickert
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tInterlocked__
#define __tInterlocked__

// Possible 360 + data breakpoints bluescreen workaround
//#define sig_interlocked_fake_atomics

#if defined( sig_interlocked_fake_atomics )
#	include "Threads/tMutex.hpp"
#endif


namespace Sig { namespace Threads
{

#if defined( sig_interlocked_fake_atomics )
	tCriticalSection& base_export fAtomicFakingCriticalSection( );

	template < typename T > T fAtomicIncrement( volatile T* target )
	{
		static_assert( sizeof(T)==sizeof(LONG) );
		tMutex m( fAtomicFakingCriticalSection( ) );
		return ++*target;
	}

	template < typename T > T fAtomicDecrement( volatile T* target )
	{
		static_assert( sizeof(T)==sizeof(LONG) );
		tMutex m( fAtomicFakingCriticalSection( ) );
		return --*target;
	}
	
	template < typename T > T fAtomicCompareExchange( volatile T* target, T exchange, T compare )
	{
		static_assert( sizeof(T)==sizeof(LONG) );
		tMutex m( fAtomicFakingCriticalSection( ) );
		const T initialValue = *target;
		if( initialValue == compare )
			*target = exchange;
		return initialValue;
	}

	template < typename T > T fAtomicExchange( volatile T* target, T exchange )
	{
		static_assert( sizeof(T)==sizeof(LONG) );
		tMutex m( fAtomicFakingCriticalSection( ) );
		const T initialValue = *target;
		*target = exchange;
		return initialValue;
	}

#elif defined( platform_msft )
	// N.B. We could easily add 64-bit variants of some of these.

	template < typename T > T fAtomicIncrement( volatile T* target )
	{
		static_assert( sizeof(T)==sizeof(LONG) );
		return (T)InterlockedIncrement((LONG*)target);
	}

	template < typename T > T fAtomicDecrement( volatile T* target )
	{
		static_assert( sizeof(T)==sizeof(LONG) );
		return (T)InterlockedDecrement((LONG*)target);
	}
	
	template < typename T > T fAtomicCompareExchange( volatile T* target, T exchange, T compare )
	{
		static_assert( sizeof(T)==sizeof(LONG) );
		return InterlockedCompareExchange( (LONG*)target, exchange, compare );
	}

	template < typename T > T fAtomicExchange( volatile T* target, T exchange )
	{
		static_assert( sizeof(T)==sizeof(LONG) );
		return InterlockedExchange( (LONG*)target, exchange );
	}

#elif defined( platform_ios )
	// N.B. Interlocked*/x86 automatically toss in memory barriers.
	// I'm assuming our code is written assuming those memory barriers are in place,
	// and using the barrier versions bellow.  Consider fAtomic*NoBarrier functions
	// perhaps?  I don't know what the best option here is.  --mrickert
	
	template < typename T > T fAtomicIncrement( volatile T* target )
	{
		dependent_static_assert( sizeof(T)==sizeof(int32_t) );
		return (T)OSAtomicIncrement32Barrier((int32_t*)target);
	}

	template < typename T > T fAtomicDecrement( volatile T* target )
	{
		dependent_static_assert( sizeof(T)==sizeof(int32_t) );
		return (T)OSAtomicDecrement32Barrier((int32_t*)target);
	}
	
	template < typename T > void fAtomicCompareExchange( volatile T* target, T exchange, T compare )
	{
		dependent_static_assert( sizeof(T)==sizeof(int32_t) );
		OSAtomicCompareAndSwap32Barrier( compare, exchange, target );
	}
	
	template < typename T > void fAtomicExchange( volatile T* target, T exchange )
	{
		// No OSAtomicAssign/Exchange.  Roll our own inefficient version of it.
		dependent_static_assert( sizeof(T)==sizeof(int32_t) );
		log_assert( ((u64)target)%4==0, "fAtomicExchange on ARM/iOS can only guarantee atomic exchanges if target is 32-bits and 32-bit aligned.  " << (void*)target << " is not." );
		OSMemoryBarrier();
		*target = exchange;
		OSMemoryBarrier();
	}

#endif

}}

#endif //ndef __tInterlocked__
