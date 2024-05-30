//------------------------------------------------------------------------------
// \file tIndexBufferSysRam.cpp - 06 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tIndexBufferSysRam.hpp"
#include "EndianUtil.hpp"

namespace Sig { namespace Gfx
{
	//------------------------------------------------------------------------------
	void tIndexBufferSysRam::fAllocate( const tIndexFormat& format, u32 numIndices )
	{
		fDeallocate( );
		mFormat = format;
		mBuffer.fNewArray( mFormat.mSize * numIndices );
	}

	//------------------------------------------------------------------------------
	void tIndexBufferSysRam::fDeallocate( )
	{
		mFormat = tIndexFormat( );
		mBuffer.fDeleteArray( );
	}

	//------------------------------------------------------------------------------
	void tIndexBufferSysRam::fEndianSwapBuffer( tDynamicBuffer& buffer, tPlatformId targetPlatform ) const
	{
		sigassert( mFormat.fValid( ) );
		EndianUtil::fSwapForTargetPlatform( buffer.fBegin( ), mFormat.mSize, targetPlatform, fIndexCount( ) );
	}

	//------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------
	void tIndexBufferSysRam::fSetIndices( u32 start, u32 srcStart, const tIndexBufferSysRam & src, u32 numSrcIndices )
	{
		sigassert( start + numSrcIndices <= fIndexCount( ) );
		sigassert( srcStart + numSrcIndices <= src.fIndexCount( ) );
		sigassert( src.mFormat.mPrimitiveType == mFormat.mPrimitiveType );

		Sig::byte * destBuffer = mBuffer.fBegin( ) + start * mFormat.mSize;
		const Sig::byte * srcBuffer = src.mBuffer.fBegin( ) + srcStart * src.mFormat.mSize;


		// If they're the same we can just mem copy
		if( src.mFormat.mStorageType == mFormat.mStorageType )
		{
			fMemCpy( destBuffer, srcBuffer, numSrcIndices * mFormat.mSize );
		}

		// Dest is 16 bit, src is 32 bit
		else if( mFormat.mStorageType == tIndexFormat::cStorageU16 )
		{
			sigassert( src.mFormat.mStorageType == tIndexFormat::cStorageU32 );

			u16* destIndex = ( u16* )destBuffer;
			const u32 * srcIndex = ( const u32* )srcBuffer;

			for( u32 i = 0; i < numSrcIndices; ++i, ++destIndex, ++srcIndex )
			{
				sigassert( *srcIndex <= mFormat.mMaxValue ); // Non-convertible
				*destIndex = ( u16 )*srcIndex;
			}
		}

		// Dest is 32 bit, src is 16 bit
		else if( mFormat.mStorageType == tIndexFormat::cStorageU32 )
		{
			sigassert( src.mFormat.mStorageType == tIndexFormat::cStorageU16 );

			u32* destIndex = ( u32* )destBuffer;
			const u16 * srcIndex = ( const u16* )srcBuffer;

			for( u32 i = 0; i < numSrcIndices; ++i, ++destIndex, ++srcIndex )
			{
				sigassert( *srcIndex <= mFormat.mMaxValue ); // Non-convertible
				*destIndex = ( u32 )*srcIndex;
			}			
		}
		else
		{
			sigassert( !"Invalid index format!" );
		}
	}

	//------------------------------------------------------------------------------
	void tIndexBufferSysRam::fGetIndices( u32 start, u32 * destIndices, u32 numSrcIndices ) const
	{
		sigassert( start + numSrcIndices <= fIndexCount( ) );

		const Sig::byte* pIdxStart = mBuffer.fBegin( ) + start * mFormat.mSize;

		if( mFormat.mStorageType == tIndexFormat::cStorageU16 )
		{
			const u16* pIdx = ( const u16* )pIdxStart;
			for( u32 i = 0; i < numSrcIndices; ++i, ++pIdx )
				destIndices[ i ] = ( u32 )*pIdx;

		}
		else if( mFormat.mStorageType == tIndexFormat::cStorageU32 )
		{
			const u32* pIdx = ( const u32* )pIdxStart;
			for( u32 i = 0; i < numSrcIndices; ++i, ++pIdx )
				 destIndices[ i ] = *pIdx;
		}
		else
		{
			sigassert( !"invalid index format" );
		}
	}

}}

