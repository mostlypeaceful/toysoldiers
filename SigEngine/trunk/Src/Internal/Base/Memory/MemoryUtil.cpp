#include <BasePch.hpp>
#include <Memory/MemoryUtil.hpp>
#include "tHeap.hpp"

namespace Sig {
namespace Memory 
{

	tOptionalDebugString::tOptionalDebugString( const char* str )
		: mStr( NULL )
	{
		fReset( str );
	}

	tOptionalDebugString::tOptionalDebugString( const tOptionalDebugString& other )
		: mStr( NULL )
	{
		operator = ( other );
	}

	tOptionalDebugString& tOptionalDebugString::operator = ( const tOptionalDebugString& other )
	{
		if( other.fStr( ) )
			fReset( other.fStr( )->c_str( ) );
		else
			fRelease( );

		return *this;
	}
	
	tOptionalDebugString::~tOptionalDebugString( )
	{
		fRelease( );
	}

	void tOptionalDebugString::fReset( const char* str )
	{
		if( mStr )
			fRelease( );

		if( str )
		{
			tHeapStacker stacker( &tDebugMemoryHeap::fInstance( ) );
			mStr = NEW std::string( str );
		}
	}

	void tOptionalDebugString::fRelease( )
	{
		if( mStr )
		{
			tHeapStacker stacker( &tDebugMemoryHeap::fInstance( ) );
			delete mStr;
			mStr = NULL;
		}
	}


	const tAllocStamp& tAllocStamp::fNoContextStamp( )
	{
		//Had to access this var from a static function to ensure that if anyone wanted to use it or
		// set their context to it that it was initialized.
		//This was a problem we ran into in game_debug. Game would crash right away because
		// a stamp (other than cNoContext) would do its default ctor (which does *this = cNoContext).
		// And then that context wouldn't actually be the same as the cNoContext so shit would assert
		// later on.
		//PLEASE DONT CHANGE THIS BACK. If you are worried about performance in this game you are 
		// looking in the wrong place. This shit will most likely be optimized out by the compiler.
		static const tAllocStamp cNoContext( __FILE__, __LINE__, "INVALID TYPE", "No Context", 0 );
		return cNoContext;
	}

	namespace AllocStampContext
	{
		const char* cAllocStampContextSysVram = "SysVram";
		const char* cAllocStampContextSysTextures = "SysTextures";
		const char* cAllocStampContextSysGeometry = "SysGeometry";
		const char* cAllocStampContextSysShader = "SysShader";
		const char* cAllocStampContextTexture = "TextureVram";
		const char* cAllocStampContextGeometry = "GeometryVram";
		const char* cAllocStampContextFui = "FuiVram";
		const char* cAllocStampContextMovie = "MovieVram";
		const char* cAllocStampContextGroundCover = "GroundCover";
	}

	std::string tAllocStamp::fToString( const std::string& tab ) const
	{
		std::string result;
		result += tab + "Size: " + StringUtil::fToString( mSize ) + " bytes (" + StringUtil::fToString( mSize / (1024.f*1024.f) ) + " mb)" + "\n";
		if( mCppFile )
			result += tab + "File: " + std::string( mCppFile ) + "\n";
		if( mLineNum != ~0 )
			result += tab + "Line: " + StringUtil::fToString( mLineNum ) + "\n";
		if( mTypeName )
			result += tab + "Type: " + std::string( mTypeName ) + "\n";
		if( mContext )
			result += tab + "Context: " + std::string( mContext ) + "\n";
		if( mUserString.fStr( ) )
			result += tab + "UserString: " + *mUserString.fStr( ) + "\n";

		return result;
	}

	void tAllocStamp::fLog( u32 flags, const char* tab ) const
	{
		Log::fPrintf( flags, "%sSize: %u bytes (%.2f mb)\n", tab, mSize, Memory::fToMB<f32>( mSize ) );
		if( mCppFile )
			Log::fPrintf( flags, "%sFile: %s\n", tab, mCppFile );
		if( mLineNum != ~0 )
			Log::fPrintf( flags, "%sLine: %u\n", tab, mLineNum );
		if( mTypeName )
			Log::fPrintf( flags, "%sType: %s\n", tab, mTypeName );
		if( mContext )
			Log::fPrintf( flags, "%sContext: %s\n", tab, mContext );
		if( mUserString.fStr( ) )
			Log::fPrintf( flags, "%UserString: %s\n", tab, *mUserString.fStr( ) );
	}

#ifdef sig_memory_dump
	void tAllocStampPtr::fReset( const tAllocStamp& stamp )
	{
		fRelease( );

		// dont do it unless we have a heap top.
		if( tHeapStack::fGlobalHeapStackInstantiated( ) == 1 )
		{
			tHeapStacker stacker( &tDebugMemoryHeap::fInstance( ) );
			mP = NEW tAllocStamp( stamp );
		}
	}

	void tAllocStampPtr::fRelease( )
	{
		if( mP )
		{
			tHeapStacker stacker( &tDebugMemoryHeap::fInstance( ) );

			// todo, push into a "free tracker" to track freed memory.

			delete mP;
			mP = NULL;
		}
	}
#else
	void tAllocStampPtr::fReset( const tAllocStamp& stamp )
	{
	}

	void tAllocStampPtr::fRelease( )
	{
	}
#endif

	tMemoryOptions::tHeapBehavior::tHeapBehavior( u32 pageSize
		, u32 maxPages
		, u32 initialPageCount
		, u32 alignment
		, u32 maxFreeChunks
		, u32 minUserMemSize
		, b32 releaseEmptyPages )
		: mPageSize( pageSize )
		, mMaxPages( maxPages )
		, mInitialPageCount( initialPageCount )
		, mAlignment( alignment )
		, mMaxFreeChunks( maxFreeChunks )
		, mMinUserMemSize( minUserMemSize )
		, mReleaseEmptyPages( releaseEmptyPages )
	{ }

	tMemoryOptions::tPoolBehavior::tPoolBehavior( u32 initialPageCount )
		: mInitialPageCount( initialPageCount )
	{ }

#ifdef target_tools
	// wide open for tools.
	tMemoryOptions::tMemoryOptions( )
		: mMainProvider(		tHeapBehavior( 32 * cOneMB, 16, 1, 32, 64, 32, true ) )
		, mResourceProvider(	tHeapBehavior( 81 * cOneMB, 16, 1, 32, 64, 32, true ) )
		, mNoCache(				tHeapBehavior( 1024       , 16, 1, 32, 16, 1, true ) )
		, mVRam(				tHeapBehavior( 32 * cOneMB, 16, 1, 128, 512, 128, true ) )
		, mDebugAllowNewPages(  false )
	{
		for( u32 i = 0; i < cMemoryPoolCount; ++i )
			mPoolInitialPageCount[ i ] = 0;
	}
#else
	tMemoryOptions::tMemoryOptions( )
		: mMainProvider(		tHeapBehavior( 50 * cOneMB, 1, 1, 32, 64, 32, false ) )
		/* Resource Note: You always need a meg more than you think here, for the sacb header*/
		, mResourceProvider(	tHeapBehavior( 80 * cOneMB, 1, 1, 32, 1, 32, false ) )
		, mNoCache(				tHeapBehavior( 1024       , 1, 0, 32, 16, 1, false ) )
		, mVRam(				tHeapBehavior( 32 * cOneMB, 15, 8, 128, 512, 128, false ) )
		, mDebugAllowNewPages(  false )
	{
		for( u32 i = 0; i < cMemoryPoolCount; ++i )
			mPoolInitialPageCount[ i ] = 1;

		// Unfortunately allocating this much memory up front will not leave enough for the wmv encoder hogs.
		//// These were found by high-water marking the game and recording them.
		//mPoolInitialPageCount[ c8_BytePool ] = 28;
		//mPoolInitialPageCount[ c16_BytePool ] = 20;
		//mPoolInitialPageCount[ c32_BytePool ] = 61;
		//mPoolInitialPageCount[ c64_BytePool ] = 163;

		//mPoolInitialPageCount[ c128_BytePool ] = 65;
		//mPoolInitialPageCount[ c256_BytePool ] = 33;
		//mPoolInitialPageCount[ c512_BytePool ] = 66;
		//mPoolInitialPageCount[ c1024_BytePool ] = 8;

		//mPoolInitialPageCount[ c1280_BytePool ] = 3;
		//mPoolInitialPageCount[ c1536_BytePool ] = 1;
		//mPoolInitialPageCount[ c1792_BytePool ] = 1;
		//mPoolInitialPageCount[ c2048_BytePool ] = 1;
	}
#endif

} }
