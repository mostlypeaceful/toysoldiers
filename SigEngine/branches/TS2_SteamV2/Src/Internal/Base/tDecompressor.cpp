#include "BasePch.hpp"
#include "tDecompressor.hpp"

namespace Sig
{
	namespace
	{
		voidpf fZLibAllocDefault( voidpf opaque, uInt items, uInt size )
		{
			const size_t allocSize = items * size;
			return NEW byte[ allocSize ];
		}

		void fZLibFreeDefault( voidpf opaque, voidpf address )
		{
			delete [] address;
		}

	}

	tDecompressor::tDecompressor( b32 begin, alloc_func allocFunc, free_func freeFunc )
		: mDecompBegan( false )
		, mAllocFunc( allocFunc ? allocFunc : fZLibAllocDefault )
		, mFreeFunc( freeFunc ? freeFunc : fZLibFreeDefault )
	{
		if( begin )
			fBeginDecompress( );
	}

	tDecompressor::~tDecompressor( )
	{
		if( mDecompBegan )
			fEndDecompress( );
	}

	void tDecompressor::fBeginDecompress( )
	{
		sigassert( !mDecompBegan );

		mDecompBegan = true;

		mStream.zalloc		= mAllocFunc;
		mStream.zfree		= mFreeFunc;
		mStream.opaque		= this;
		mStream.avail_in	= 0;
		mStream.next_in		= Z_NULL;
		const s32 ret		= inflateInit( &mStream );
		sigassert( ret == Z_OK );
	}

	void tDecompressor::fEndDecompress( )
	{
		sigassert( mDecompBegan );

		inflateEnd( &mStream );

		mDecompBegan = false;
	}

	u32 tDecompressor::fDecompress( const byte* src, u32 srcBytes, byte* dst, u32 dstBytes )
	{
		sigassert( mDecompBegan );

		mStream.next_in			= ( Bytef* )src;
		mStream.avail_in		= srcBytes;
		mStream.next_out		= ( Bytef* )dst;
		mStream.avail_out		= dstBytes;

		const size_t totalOut0	= mStream.total_out;

		const s32 ret = inflate( &mStream, Z_NO_FLUSH );
		if( ret != Z_OK && ret != Z_STREAM_END )
		{
			if( ret == Z_MEM_ERROR )
			{
				log_warning( Log::cFlagFile, "Error decompressing data: Out of Memory." );
			}
			else
			{
				log_warning( Log::cFlagFile, "Error decompressing data." );
			}
			return ~0;
		}

		const size_t totalOut1 = mStream.total_out;

		const size_t amountDecompressed = totalOut1 - totalOut0;

		return ( u32 )amountDecompressed;
	}

}

