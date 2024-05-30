#ifndef __tHeap__
#define __tHeap__
#include "tPool.hpp"
#include "Memory/MemoryUtil.hpp"
#include "Memory/tMemoryDump.hpp"
#include "Memory/DebugMemory.hpp"

namespace Sig { namespace Memory
{
	struct tMemoryDumpOptions
	{
		tMemoryDumpOptions( b32 post = false )
			: mPost( post )
			, mDump( NULL )
		{ }

		b32			mPost;
		tMemoryDump* mDump;
	};

	class base_export tHeap
	{
	public:
		static tHeap& fInstance( );
		static b32 fInstanceCreated( );

		virtual ~tHeap( ) { }
		virtual void* fAlloc( u32 numBytes, const tAllocStamp& stamp ) = 0;
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp ) = 0;
		virtual void  fFree( void* memory ) = 0;
		virtual u32 fSizeOfAlloc( void* memory ) = 0;

		virtual void fConsolidate( ) { }
		virtual void fDump( const tMemoryDumpOptions& ) { }
		virtual void fLogFreeMemory( ) const { Log::fPrintf( 0, "No logging for heap: %s\n", mName ); }

		virtual void fAddStats( std::wstringstream& ss ) { }

		// Debugging
	public:
		// set this before you make any vram allocation.
		static const char* cVramHeapName;
		static tAllocStamp mVramContext;
		static void fSetVramContext( const tAllocStamp& context ) { sigassert( mVramContext == tAllocStamp::fNoContextStamp( ) ); mVramContext = context; }
		static void fResetVramContext( ) { mVramContext = tAllocStamp::fNoContextStamp( ); }

		const char* fName( ) const { return mName; }

	protected:
		tHeap( const char* name ) 
			: mName( name ) 
		{ }

		const char* mName;

		void fPageLimitHit( const tAllocStamp& stamp );
		void fOutOfMemory( const tAllocStamp& stamp );
	};

	struct tVramContextScope
	{
		tVramContextScope( const tAllocStamp& context ) { tHeap::fSetVramContext( context ); }
		~tVramContextScope( ) { tHeap::fResetVramContext( ); }
	};

	// Raw dog memory allocation, still consumes title memory though does not interact with the game allocators.
	class base_export tMallocHeap : public tHeap
	{
	public:
		virtual void* fAlloc( u32 numBytes, const tAllocStamp& stamp );
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp );
		virtual void  fFree( void* memory );
		virtual u32	  fSizeOfAlloc( void* memory );

	private:
		tMallocHeap( ) : tHeap( "tMallocHeap" ) { }
		declare_singleton_define_own_ctor_dtor( tMallocHeap );
	};

	// This is used for debugging if you need to allocate memory which does not effect the actual programs memory layout.
	//  Pulls from a reserved memory space on extended memory boxes.
	class base_export tDebugMemoryHeap : public tHeap
	{
	public:
		virtual void* fAlloc( u32 numBytes, const tAllocStamp& stamp )					{ return fDebugMemAlloc( numBytes ); }
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp )	{ sigassert( !"Realloc Not Implemented." ); return NULL; }
		virtual void  fFree( void* memory )												{ fDebugMemFree( memory ); }
		virtual u32	  fSizeOfAlloc( void* memory )										{ sigassert( !"AllocSize Not Implemented." ); return 0; }

		static tHeap& fInstance( )
		{
			if( fDebugMemEnabled( ) )
			{
				static tDebugMemoryHeap gDebugHeap;
				return gDebugHeap;
			}
			else
			{
				// log_warning commented out to prevent stack overflow
				//log_warning( "No debug memory present. Falling back to malloc heap." );
				return tMallocHeap::fInstance( );
			}
		}

	private: 
		tDebugMemoryHeap( ) : tHeap( "tDebugMemoryHeap" ) { sigassert( fDebugMemEnabled( ) ); }
	};

	class base_export tPoolBasedHeap : public tHeap
	{
	public:
		typedef tFixedArray<tPool,12> tPoolList;
	protected:
		tPoolList mPools;
	public:
		tPoolBasedHeap( const char* name );
		virtual ~tPoolBasedHeap( );
		virtual void* fAlloc( u32 numBytes, const tAllocStamp& stamp );
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp );
		virtual void fFree( void* memory );
		virtual u32 fSizeOfAlloc( void* memory );

		virtual void fDump( tMemoryDumpOptions& );
	};


#ifdef sig_memory_dump
	/*
		The global heap stack allows you to push a new global heap for new and delete.
		This is extremely convenient for debugging, to push a debug heap or a malloc heap temporarily.

		There is one of these per thread. Access it via fGlobalHeapStack
	*/
	class base_export tHeapStack : public tUncopyable
	{
	public:
		static tHeapStack& fThreadHeapStack( );

		tHeap& fHeapTop( ) const
		{
			sigassert( mHeapTop );
			return *mHeapTop;
		}

		static u32& fGlobalHeapStackInstantiated( )
		{
			static u32 gHeapStackInstantiated = 0;
			return gHeapStackInstantiated;
		}

	private:
		tHeap* mHeapTop;

		tHeapStack( tHeap* heap )
			: mHeapTop( heap )
		{
		}

		void fSetTop( tHeap* heap )
		{
			mHeapTop = heap;
		}

		friend class tHeapStacker;
	};

	/*
		The tHeapStacker is is an RAII controlled change of the stack's heap. When the stacker goes out of scope, the heap stack will be popped.
	*/
	class base_export tHeapStacker : public tUncopyable
	{
	public:
		// Pass in the new heap you want pushed on top of the stack.
		tHeapStacker( tHeap* heap );

		// It will be restored to the previous value when this type is destroyed.
		~tHeapStacker( );

	private:
		tHeapStack* mStack;
		tHeap* mPrevHeap;
	};

	#define sig_heap_top( ) ::Sig::Memory::tHeapStack::fThreadHeapStack( ).fHeapTop( )

#else
	/*
		These versions will compile out.
	*/

	// No tHeapStack available.

	class base_export tHeapStacker
	{
	public:
		tHeapStacker( tHeap* heap ) { }
	};

	#define sig_heap_top( ) ::Sig::Memory::tHeap::fInstance( )
	
#endif

}}

#if defined( platform_ios )
// Supress GCC warnings about missing the throw specifier:
#define SIG_THROWS_BAD_ALLOC throw(std::bad_alloc)
#else
// Supress VS warnings about ignoring the throw specifier:
#define SIG_THROWS_BAD_ALLOC
#endif

// dont use these first two macros directly
#define define_global_new_delete_meat_and_potatos( ) \
	inline void fStartupGlobalHeap_( ); static ::Sig::tStaticFunctionCall _fStartupGlobalHeap_( fStartupGlobalHeap_ ); \
	inline void fStartupGlobalHeap_( ) \
	{ \
		::Sig::Memory::tHeap::fInstance( ); \
	} \
		inline void* operator new( size_t bytes ) SIG_THROWS_BAD_ALLOC \
	{ \
		return sig_heap_top( ).fAlloc( ( ::Sig::u32 )bytes, ::Sig::Memory::tAllocStamp::fNoContextStamp( ) ); \
	} \
		inline void* operator new( size_t bytes, const std::nothrow_t& ) throw() \
	{ \
		return sig_heap_top( ).fAlloc( ( ::Sig::u32 )bytes, ::Sig::Memory::tAllocStamp::fNoContextStamp( ) ); \
	} \
		inline void* operator new[]( size_t bytes ) SIG_THROWS_BAD_ALLOC \
	{ \
		return sig_heap_top( ).fAlloc( ( ::Sig::u32 )bytes, ::Sig::Memory::tAllocStamp::fNoContextStamp( ) ); \
	} \
		inline void* operator new[]( size_t bytes, const std::nothrow_t& ) throw() \
	{ \
		return sig_heap_top( ).fAlloc( ( ::Sig::u32 )bytes, ::Sig::Memory::tAllocStamp::fNoContextStamp( ) ); \
	} \
		inline void operator delete( void* memory ) throw() \
	{ \
		sig_heap_top( ).fFree( memory ); \
	} \
		inline void operator delete( void* memory, const std::nothrow_t& ) throw() \
	{ \
		sig_heap_top( ).fFree( memory ); \
	} \
		inline void operator delete[]( void* memory ) throw() \
	{ \
		sig_heap_top( ).fFree( memory ); \
	} \
		inline void operator delete[]( void* memory, const std::nothrow_t& ) throw() \
	{ \
		sig_heap_top( ).fFree( memory ); \
	}

#define define_global_new_delete_alloc_stamped( ) \
	inline void* operator new( size_t bytes, const ::Sig::Memory::tAllocStamp& stamp ) SIG_THROWS_BAD_ALLOC \
	{ \
		::Sig::Memory::tAllocStamp s = stamp; s.mSize = bytes; \
		return sig_heap_top( ).fAlloc( ( ::Sig::u32 )bytes, s ); \
	}\
	inline void* operator new( size_t bytes, const ::Sig::Memory::tAllocStamp& stamp, const std::nothrow_t& ) throw() \
	{ \
		::Sig::Memory::tAllocStamp s = stamp; s.mSize = bytes; \
		return sig_heap_top( ).fAlloc( ( ::Sig::u32 )bytes, s ); \
	}\
	inline void* operator new[ ] ( size_t bytes, const ::Sig::Memory::tAllocStamp& stamp ) SIG_THROWS_BAD_ALLOC \
	{ \
		::Sig::Memory::tAllocStamp s = stamp; s.mSize = bytes; \
		return sig_heap_top( ).fAlloc( ( ::Sig::u32 )bytes, s ); \
	} \
	inline void* operator new[ ] ( size_t bytes, const ::Sig::Memory::tAllocStamp& stamp, const std::nothrow_t& ) throw() \
	{ \
		::Sig::Memory::tAllocStamp s = stamp; s.mSize = bytes; \
		return sig_heap_top( ).fAlloc( ( ::Sig::u32 )bytes, s ); \
	} \
	inline void operator delete ( void* mem, const ::Sig::Memory::tAllocStamp& stamp ) throw() \
	{ \
		sig_heap_top( ).fFree( mem ); \
	} \
	inline void operator delete ( void* mem, const ::Sig::Memory::tAllocStamp& stamp, const std::nothrow_t& ) throw() \
	{ \
		sig_heap_top( ).fFree( mem ); \
	} \
	inline void operator delete[ ] ( void* mem, const ::Sig::Memory::tAllocStamp& stamp ) throw() \
	{ \
		sig_heap_top( ).fFree( mem ); \
	} \
	inline void operator delete[ ] ( void* mem, const ::Sig::Memory::tAllocStamp& stamp, const std::nothrow_t& ) throw() \
	{ \
		sig_heap_top( ).fFree( mem ); \
	}  

#ifdef sig_memory_dump
	#define define_global_new_delete( ) \
		define_global_new_delete_meat_and_potatos( ) \
		define_global_new_delete_alloc_stamped( )
#else
	#define define_global_new_delete( ) \
		define_global_new_delete_meat_and_potatos( )
#endif

#endif//__tHeap__
