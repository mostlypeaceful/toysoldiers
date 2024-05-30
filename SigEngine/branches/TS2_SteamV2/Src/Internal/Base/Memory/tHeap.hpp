#ifndef __tHeap__
#define __tHeap__
#include "tPool.hpp"
#include "Memory/MemoryUtil.hpp"
#include "Memory/tMemoryDump.hpp"

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

		// Debugging
	protected:
		tHeap( const char* name ) 
			: mName( name ) 
		{ }

		const char* mName;

		void fPageLimitHit( const tAllocStamp& stamp );
		void fOutOfMemory( const tAllocStamp& stamp );
	};

	// This is used for debugging if you need to allocate memory which does not effect the actual programs memory layout.
	class base_export tMallocHeap : public tHeap
	{
	public:
		virtual void* fAlloc( u32 numBytes, const tAllocStamp& stamp )					{ return malloc( numBytes ); }
		virtual void* fRealloc( void* memory, u32 numBytes, const tAllocStamp& stamp )	{ return realloc( memory, numBytes ); }
		virtual void  fFree( void* memory )												{ free( memory ); }
		virtual u32	  fSizeOfAlloc( void* memory )										{ return 0; }

	private:
		tMallocHeap( ) : tHeap( "tMallocHeap" ) { }
		declare_singleton_define_own_ctor_dtor( tMallocHeap );
	};

	class base_export tPoolBasedHeap : public tHeap
	{
	public:
		typedef tFixedArray<tPool,12> tPoolList;
	private:
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

	class base_export tHeapStack
	{
		static tHeap* gHeapTop;
		tHeap* mPrevHeap;

	public:
		tHeapStack( tHeap* heap )
			: mPrevHeap( gHeapTop )
		{
			gHeapTop = heap;
		}

		~tHeapStack( )
		{
			gHeapTop = mPrevHeap;
		}

		static tHeap& fHeapTop( )
		{
			//sigassert( gHeapTop );
			if( gHeapTop )
				return *gHeapTop;
			else
				return tHeap::fInstance( );
		}
	};
}}

// dont use these first two macros directly
#define define_global_new_delete_meat_and_potatos( ) \
	inline void fStartupGlobalHeap_( ); static ::Sig::tStaticFunctionCall _fStartupGlobalHeap_( fStartupGlobalHeap_ ); \
	inline void fStartupGlobalHeap_( ) \
	{ \
		::Sig::Memory::tHeap::fInstance( ); \
	} \
		inline void* operator new( size_t bytes ) \
	{ \
		return ::Sig::Memory::tHeapStack::fHeapTop( ).fAlloc( ( ::Sig::u32 )bytes, ::Sig::Memory::tAllocStamp::cNoContext ); \
	} \
		inline void* operator new( size_t bytes, const std::nothrow_t& ) throw() \
	{ \
		return ::Sig::Memory::tHeapStack::fHeapTop( ).fAlloc( ( ::Sig::u32 )bytes, ::Sig::Memory::tAllocStamp::cNoContext ); \
	} \
		inline void* operator new[]( size_t bytes ) \
	{ \
		return ::Sig::Memory::tHeapStack::fHeapTop( ).fAlloc( ( ::Sig::u32 )bytes, ::Sig::Memory::tAllocStamp::cNoContext ); \
	} \
		inline void* operator new[]( size_t bytes, const std::nothrow_t& ) throw() \
	{ \
		return ::Sig::Memory::tHeapStack::fHeapTop( ).fAlloc( ( ::Sig::u32 )bytes, ::Sig::Memory::tAllocStamp::cNoContext ); \
	} \
		inline void operator delete( void* memory ) \
	{ \
		::Sig::Memory::tHeapStack::fHeapTop( ).fFree( memory ); \
	} \
		inline void operator delete( void* memory, const std::nothrow_t& ) \
	{ \
		::Sig::Memory::tHeapStack::fHeapTop( ).fFree( memory ); \
	} \
		inline void operator delete[]( void* memory ) \
	{ \
		::Sig::Memory::tHeapStack::fHeapTop( ).fFree( memory ); \
	} \
		inline void operator delete[]( void* memory, const std::nothrow_t& ) \
	{ \
		::Sig::Memory::tHeapStack::fHeapTop( ).fFree( memory ); \
	}

#define define_global_new_delete_alloc_stamped( ) \
	inline void* operator new( size_t bytes, const ::Sig::Memory::tAllocStamp& stamp ) \
	{ \
		::Sig::Memory::tAllocStamp s = stamp; s.mSize = bytes; \
		return ::Sig::Memory::tHeapStack::fHeapTop( ).fAlloc( ( ::Sig::u32 )bytes, s ); \
	}\
	inline void* operator new( size_t bytes, const ::Sig::Memory::tAllocStamp& stamp, const std::nothrow_t& ) throw() \
	{ \
		::Sig::Memory::tAllocStamp s = stamp; s.mSize = bytes; \
		return ::Sig::Memory::tHeapStack::fHeapTop( ).fAlloc( ( ::Sig::u32 )bytes, s ); \
	}\
	inline void* operator new[ ] ( size_t bytes, const ::Sig::Memory::tAllocStamp& stamp ) \
	{ \
		::Sig::Memory::tAllocStamp s = stamp; s.mSize = bytes; \
		return ::Sig::Memory::tHeapStack::fHeapTop( ).fAlloc( ( ::Sig::u32 )bytes, s ); \
	} \
	inline void* operator new[ ] ( size_t bytes, const ::Sig::Memory::tAllocStamp& stamp, const std::nothrow_t& ) throw() \
	{ \
		::Sig::Memory::tAllocStamp s = stamp; s.mSize = bytes; \
		return ::Sig::Memory::tHeapStack::fHeapTop( ).fAlloc( ( ::Sig::u32 )bytes, s ); \
	} \
	inline void operator delete ( void* mem, const ::Sig::Memory::tAllocStamp& stamp ) \
	{ \
		::Sig::Memory::tHeapStack::fHeapTop( ).fFree( mem ); \
	} \
	inline void operator delete ( void* mem, const ::Sig::Memory::tAllocStamp& stamp, const std::nothrow_t& ) \
	{ \
		::Sig::Memory::tHeapStack::fHeapTop( ).fFree( mem ); \
	} \
	inline void operator delete[ ] ( void* mem, const ::Sig::Memory::tAllocStamp& stamp ) \
	{ \
		::Sig::Memory::tHeapStack::fHeapTop( ).fFree( mem ); \
	} \
	inline void operator delete[ ] ( void* mem, const ::Sig::Memory::tAllocStamp& stamp, const std::nothrow_t& ) \
	{ \
		::Sig::Memory::tHeapStack::fHeapTop( ).fFree( mem ); \
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
