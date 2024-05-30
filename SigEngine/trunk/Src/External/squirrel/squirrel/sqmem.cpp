/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"

/* !!!ADDED July 28, 2008 by Max Wagner, not part of original Squirrel API!!!*/
/* Added global function pointers for malloc, realloc, and free, so that these can be set from application code. */

namespace
{
	void* mallocWrap( size_t size )
	{
		return ::malloc( size ); 
	}
	void* reallocWrap( void* p, size_t oldSize, size_t newSize )
	{
		return ::realloc( p, newSize ); 
	}
	void freeWrap( void* p, size_t size )
	{
		::free( p ); 
	}
}


SQMALLOCFUNC _sqmalloc(::mallocWrap);
SQREALLOCFUNC _sqrealloc(::reallocWrap);
SQFREEFUNC _sqfree(::freeWrap);

void sq_setallocfuncs( SQMALLOCFUNC m, SQREALLOCFUNC r, SQFREEFUNC f )
{
	_sqmalloc = m;
	_sqrealloc = r;
	_sqfree = f;
}


/* !!!MODIFIED July 28, 2008 by Max Wagner, not part of original Squirrel API!!!*/
/* I just re-routed these calls to use the new function pointers just above */

void *sq_vm_malloc(SQUnsignedInteger size){	return _sqmalloc(size); }

void *sq_vm_realloc(void *p, SQUnsignedInteger oldsize, SQUnsignedInteger size){ return _sqrealloc(p, oldsize, size); }

void sq_vm_free(void *p, SQUnsignedInteger size){	_sqfree(p, size); }
