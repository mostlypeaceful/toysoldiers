#include "BasePch.hpp"
#include "tIndexBufferSysRam.hpp"
#include "EndianUtil.hpp"

namespace Sig { namespace Gfx
{
	void tIndexBufferSysRam::fAllocate( const tIndexFormat& format, u32 numIndices )
	{
		fDeallocate( );
		mFormat = format;
		mBuffer.fNewArray( mFormat.mSize * numIndices );
	}

	void tIndexBufferSysRam::fDeallocate( )
	{
		mFormat = tIndexFormat( );
		mBuffer.fDeleteArray( );
	}

	void tIndexBufferSysRam::fEndianSwapBuffer( tDynamicBuffer& buffer, tPlatformId targetPlatform ) const
	{
		sigassert( mFormat.fValid( ) );
		EndianUtil::fSwapForTargetPlatform( buffer.fBegin( ), mFormat.mSize, targetPlatform, fIndexCount( ) );
	}

	void tIndexBufferSysRam::fSetIndices( u32 start, const u32* srcIndices, u32 numSrcIndices )
	{
		sigassert( start + numSrcIndices <= fIndexCount( ) );

		Sig::byte* pIdxStart = mBuffer.fBegin( ) + start * mFormat.mSize;

		if( mFormat.mStorageType == tIndexFormat::cStorageU16 )
		{
			u16* pIdx = ( u16* )pIdxStart;

			for( u32 i = 0; i < numSrcIndices; ++i, ++pIdx )
			{
				sigassert( srcIndices[ i ] <= mFormat.mMaxValue );
				*pIdx = ( u16 )srcIndices[ i ];
			}			
		}
		else if( mFormat.mStorageType == tIndexFormat::cStorageU32 )
		{
			u32* pIdx = ( u32* )pIdxStart;

			for( u32 i = 0; i < numSrcIndices; ++i, ++pIdx )
			{
				sigassert( srcIndices[ i ] <= mFormat.mMaxValue );
				*pIdx = ( u32 )srcIndices[ i ];
			}
		}
		else
		{
			sigassert( !"invalid index format" );
		}




	}

}}

