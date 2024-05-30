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
//#	define sig_memory_dump								///< use to allow memory dumping
//# endif

// No memory debugging in profile or release
#if !defined(build_playtest) && !defined(build_profile) && !defined(build_release)
	# if !defined(target_tools) && !defined(platform_ios)
	#	define sig_memory_dump							///< use to allow memory dumping
	# endif

	#	define sig_memory_tokenfill						///< use to fill allocated/freed/uninitialized portions of memory with special tokens (not too slow, good to leave on)
	#	define sig_memory_validate						///< controls all sub-validation defines (below); also, if on, individual chunk validation will still occur (not too slow, good to leave on)
	#	ifdef sig_memory_validate
		#	define sig_memory_validate_assert			///< use to assert on validation failure (might as well, right?)
		//#	define sig_memory_validate_tokens			///< use only when on a crusade against memory corruption, is very slow!
		//#	define sig_memory_validate_eachalloc		///< causes a fair amount of slow down, due to 2 validations per allocation/deallocation (before and after!)
	#	endif
#endif

namespace Sig { 
namespace Memory
{
	template<class tReturnType>
	tReturnType fToKB( u32 inBytes )
	{
		const f32 div = 1024;
		return (tReturnType)( inBytes / div );
	}

	template<class tReturnType>
	tReturnType fToMB( u32 inBytes )
	{
		const f32 div = 1024 * 1024;
		return (tReturnType)( inBytes / div );
	}

	template<class tReturnType, class InType>
	tReturnType fFromKB( InType inKB )
	{
		return (tReturnType)( inKB * 1024 );
	}

	template<class tReturnType, class InType>
	tReturnType fFromMB( InType inMB )
	{
		return (tReturnType)( inMB * 1024 * 1024 );
	}

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
#else
	class MemoryValidatorObject
	{
		// the do-nothing version
	public:
		inline MemoryValidatorObject() {  }
		inline ~MemoryValidatorObject() {  }
	};
#endif

#ifdef sig_memory_validate_assert
	#define assert_mem_valid( x ) sigassert( x )
#else
	#define assert_mem_valid( x )
#endif

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
		b32				mDebugAllowNewPages;						// Allow heaps to expand even if they dont' want to. Dev-only.
	};

	// tOptionalDebugString
	//  Makes instantiation of char arry optional. Pass in NULL and fStr will be NULL.
	//  Also enforces use of debug heap for string storage.
	class base_export tOptionalDebugString
	{
	public:
		tOptionalDebugString( const char* str = NULL );
		tOptionalDebugString( const tOptionalDebugString& other );
		tOptionalDebugString& operator = ( const tOptionalDebugString& other );
		~tOptionalDebugString( );

		const std::string* fStr( ) const { return mStr; }

		void fReset( const char* str );
		void fRelease( );

	private:
		std::string* mStr;
	};

	struct base_export tAllocStamp
	{
		static const tAllocStamp& fNoContextStamp( );

		// UserString will be copied in, and does not need to persist in memory after this function.
		explicit tAllocStamp( const char* file, u32 lineNum = ~0, const char* typeName = "", const char* context = "", u32 size = 0, const char* userString = NULL )
			: mTypeName( typeName )
			, mCppFile( file )
			, mLineNum( lineNum )
			, mContext( context )
			, mSize( size )
			, mUserString( userString )
		{ }

		b32 operator == ( const tAllocStamp& right ) const 
		{ 
			return mTypeName == right.mTypeName
				&& mCppFile == right.mCppFile 
				&& mLineNum == right.mLineNum
				&& mContext == right.mContext
				&& mSize == right.mSize; 
		}

		b32 operator != ( const tAllocStamp& right ) const
		{
			return !operator == ( right ); 
		}

		std::string fToString( const std::string& tab ) const;
		void fLog( u32 flags, const char* tab ) const;

		// Be mindful that only the mUserString is owned by the stamp itself (for performance reason the others are expected to be static constant strings)
		const char* mTypeName;
		const char*	mCppFile;
		u32			mLineNum;
		const char* mContext;
		u32			mSize;
		tOptionalDebugString mUserString;
	};

	// This can't use the regular tRefCounterPtr due to complications in BasePch.hpp
	class tAllocStampPtr : public tUncopyable
	{
	public:
		tAllocStampPtr( ) : mP( NULL ) { }
		~tAllocStampPtr( ) { fRelease( ); }

		void fReset( const tAllocStamp& stamp );
		void fRelease( );

		tAllocStamp* fGetRawPtr( ) { return mP; }
		inline tAllocStamp& operator*() const { return *mP; }	
		inline tAllocStamp* operator->() const { return mP; }

	private:
		tAllocStamp* mP;
	};

	namespace AllocStampContext
	{
		extern const char* cAllocStampContextSysVram;
		extern const char* cAllocStampContextSysTextures;
		extern const char* cAllocStampContextSysGeometry;
		extern const char* cAllocStampContextSysShader;
		extern const char* cAllocStampContextTexture;
		extern const char* cAllocStampContextGeometry;
		extern const char* cAllocStampContextFui;
		extern const char* cAllocStampContextMovie;
		extern const char* cAllocStampContextGroundCover;
	}

}}

#define generic_alloc_stamp( ) ::Sig::Memory::tAllocStamp( __FILE__, __LINE__ )
#define vram_alloc_stamp( allocStampContext ) ::Sig::Memory::tAllocStamp( __FILE__, __LINE__, "", ::Sig::Memory::AllocStampContext::allocStampContext )

#ifdef sig_memory_dump
	#define NEW new ( generic_alloc_stamp( ) ) 
	#define NEW_TYPED( type ) new ( ::Sig::Memory::tAllocStamp( __FILE__, __LINE__, typeid(type).name(), "" ) ) type
	#define NEW_CONTEXTED( context ) new ( ::Sig::Memory::tAllocStamp( __FILE__, __LINE__, "", context ) ) 
#else
	#define NEW new
	#define NEW_TYPED( type ) new type
	#define NEW_CONTEXTED( context ) new 
#endif



#endif//__Memory_Util__
