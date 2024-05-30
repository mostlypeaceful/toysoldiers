//------------------------------------------------------------------------------
// \file MemoryUtil.hpp - 28 Sep 2010
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __Memory_Util__
#define __Memory_Util__

// unComment this to dump in release
//# ifndef target_tools
//#	define sig_memory_dump							///< use to allow memory dumping
//# endif

#ifndef build_release
	# ifndef target_tools
	#	define sig_memory_dump							///< use to allow memory dumping
	# endif

	#	define sig_memory_tokenfill				///< use to fill allocated/freed/uninitialized portions of memory with special tokens (not too slow, good to leave on)
	#	define sig_memory_validate						///< controls all sub-validation defines (below); also, if on, individual chunk validation will still occur (not too slow, good to leave on)
	#	ifdef sig_memory_validate
		#	define sig_memory_validate_assert			///< use to assert on validation failure (might as well, right?)
		//#	define sig_memory_validate_tokens	///< use only when on a crusade against memory corruption, is very slow!
		//#	define sig_memory_validate_eachalloc	///< causes a fair amount of slow down, due to 2 validations per allocation/deallocation (before and after!)
	#	endif//sig_memory_validate
#endif//build_release

namespace Sig { 
namespace Memory
{
	struct tAlignedAllocation
	{
		inline tAlignedAllocation( ) 
			: mOriginalAddr(0)
			, mAlignedAddr(0)
		{
		}
		inline tAlignedAllocation( void* og, u32 alignment ) 
			: mOriginalAddr( og )
			, mAlignedAddr( ( void* )fAlignHigh<u32>( ( u32 )og, alignment ) )
		{ 
		}
		void* mOriginalAddr;
		void* mAlignedAddr;
	};


#ifdef sig_memory_validate_eachalloc
	class MemoryValidatorObject
	{
		// helper object type for performing memory validation once on construction,
		// and once on destruction; i.e., we instantiate one of these at the top
		// of a function, and validation will be performed once prior to allocation/deallocation,
		// and once after, helping to identify the actual allocation/deallocation where 
		// memory corruption occured.
	public:
		MemoryValidatorObject();
		~MemoryValidatorObject();
	};
#else//sig_memory_validate_eachalloc
	class MemoryValidatorObject
	{
		// the do-nothing version
	public:
		inline MemoryValidatorObject() {  }
		inline ~MemoryValidatorObject() {  }
	};
#endif//sig_memory_validate_eachalloc

#ifdef sig_memory_validate_assert
	#define assert_mem_valid( x ) sigassert( x )
#else//sig_memory_validate_assert
	#define assert_mem_valid( x )
#endif//sig_memory_validate_assert

	// encapsulate the tokens used for debug memory marking
	static const byte cMemTokenAllocated = 0xfa; // 0xfa for allocated
	static const byte cMemTokenFreed = 0xff; // 0xff for freed
	static const byte cMemTokenGarbage = 0xfb; // 0xfb for new-born

	inline void fMarkMemAsFreed(void* mem, u32 bytes)
	{
#ifdef sig_memory_tokenfill
		fMemSet( mem, cMemTokenFreed, bytes );
#endif
	}

	inline void fMarkMemAsAllocated(void* mem, u32 bytes)
	{
#ifdef sig_memory_tokenfill
		fMemSet(mem, cMemTokenAllocated, bytes);
#endif
	}

	inline void fMarkMemAsGarbage(void* mem, u32 bytes)
	{
#ifdef sig_memory_tokenfill
		fMemSet(mem, cMemTokenGarbage, bytes );
#endif
	}

	struct base_export tMemoryOptions
	{
		static const u32 cOneMB = 1024 * 1024;

		enum tMemoryPools
		{
			c8_BytePool,
			c16_BytePool,
			c32_BytePool,
			c64_BytePool,

			c128_BytePool,
			c256_BytePool,
			c512_BytePool,
			c1024_BytePool,

			c1280_BytePool,
			c1536_BytePool,
			c1792_BytePool,
			c2048_BytePool,
			
			cMemoryPoolCount
		};

		struct tHeapBehavior
		{
			u32 mPageSize;
			u32 mMaxPages;
			u32 mInitialPageCount;
			u32 mAlignment;
			u32 mMaxFreeChunks;
			u32 mMinUserMemSize;
			b32	mReleaseEmptyPages;

			tHeapBehavior( u32 pageSize = 10 * cOneMB
				, u32 maxPages = ~0
				, u32 initialPageCount = 1
				, u32 alignment = 8
				, u32 maxFreeChunks = 64
				, u32 minUserMemSize = 1
				, b32 rleaseEmptyPages = true );
		};

		struct tPoolBehavior
		{
			u32 mInitialPageCount;

			tPoolBehavior( u32 initialPageCount = ~0 );
		};

		tMemoryOptions( );

		tHeapBehavior	mMainProvider;
		tHeapBehavior	mResourceProvider;
		tHeapBehavior	mNoCache;
		tHeapBehavior	mVRam;
		u32				mPoolInitialPageCount[ cMemoryPoolCount ];
		b32				mDebugAllowNewPages;
	};

	struct base_export tAllocStamp
	{
		static const tAllocStamp cNoContext;

		explicit tAllocStamp( )
		{ 
			*this = cNoContext;
		}

		explicit tAllocStamp( const char* file, u32 lineNum = ~0, const char* typeName = "", const char* context = "", u32 size = 0 )
			: mTypeName( typeName )
			, mFile( file )
			, mLineNum( lineNum )
			, mContext( context )
			, mSize( size )
		{ }

		b32 operator == ( const tAllocStamp& right ) const 
		{ 
			return mTypeName == right.mTypeName
				&& mFile == right.mFile 
				&& mLineNum == right.mLineNum
				&& mContext == right.mContext
				&& mSize == right.mSize; 
		}

		b32 operator != ( const tAllocStamp& right ) const
		{
			return !operator == ( right ); 
		}

		std::string fToString( const std::string& tab ) const;

		const char* mTypeName;
		const char*	mFile;
		u32			mLineNum;
		const char* mContext;
		u32			mSize;
	};

}}

#ifdef sig_memory_dump
	#define NEW new ( ::Sig::Memory::tAllocStamp( __FILE__, __LINE__ ) ) 
	#define NEW_TYPED( type ) new ( ::Sig::Memory::tAllocStamp( __FILE__, __LINE__, typeid( type ).name(), "" ) ) 
	#define NEW_CONTEXTED( context ) new ( ::Sig::Memory::tAllocStamp( __FILE__, __LINE__, "", context ) ) 
#else
	#define NEW new
	#define NEW_TYPED( typeName ) new
	#define NEW_CONTEXTED( context ) new 
#endif



#endif//__Memory_Util__
