#include <BasePch.hpp>
#include <Memory/MemoryUtil.hpp>

namespace Sig {
namespace Memory 
{
	const tAllocStamp tAllocStamp::cNoContext( "No Context", 0 );

	std::string tAllocStamp::fToString( const std::string& tab ) const
	{
		std::string result;
		result += tab + "Size: " + StringUtil::fToString( mSize ) + " bytes (" + StringUtil::fToString( mSize / (1024.f*1024.f) ) + " mb)" + "\n";
		if( mFile )
			result += tab + "File: " + std::string( mFile ) + "\n";
		if( mLineNum != ~0 )
			result += tab + "Line: " + StringUtil::fToString( mLineNum ) + "\n";
		if( mTypeName )
			result += tab + "Type: " + std::string( mTypeName ) + "\n";
		if( mContext )
			result += tab + "Context: " + std::string( mContext ) + "\n";

		//return std::string( mFile ); // + " ln: " + StringUtil::fToString( mLineNum );
		return result;
	}

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
		, mReleaseEmptyPages( mReleaseEmptyPages )
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
		: mMainProvider(		tHeapBehavior( 128 * cOneMB, 1, 1, 32, 64, 32, false ) )
		/* Resource Note: You always need a meg more than you think here, for the sacb header*/
		, mResourceProvider(	tHeapBehavior( 128 * cOneMB, 1, 1, 32, 1, 32, false ) )
		, mNoCache(				tHeapBehavior( 1024       , 1, 0, 32, 16, 1, false ) )
		, mVRam(				tHeapBehavior( 32 * 9 * cOneMB, 1, 1, 128, 512, 128, false ) )
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