#include "BasePch.hpp"
#include "tCompressor.hpp"

namespace Sig
{
	// TODO make this work like the decompressor, so you can compress a bit at a time

	// TODO figure out zlib alloc and free callbacks

	u32 tCompressor::fCompress( const byte* src, u32 srcBytes, byte* dst )
	{
const s32 level = Z_DEFAULT_COMPRESSION;

		int ret, flush;
		unsigned have;
		z_stream strm;
		const u32 Chunk = 128 * 1024;
		const byte*	in	= src;
		byte*		out = dst;

		u32 compressedBytes = 0;

		/* allocate deflate state */
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		ret = deflateInit( &strm, level );
		if( ret != Z_OK )
			return 0;

		// compress until end of file
		for( u32 totalHandled = 0; totalHandled < srcBytes; totalHandled += Chunk )
		{
			const u32 toRead = fMin( srcBytes - totalHandled, Chunk );
			strm.avail_in = toRead;//(uInt)fread(in, 1, Chunk, source);
			flush = ( totalHandled + toRead < srcBytes ) ? Z_NO_FLUSH : Z_FINISH;
			strm.next_in = (Bytef *)in;

			// run deflate() on input until output buffer not full, finish
			// compression if all of source has been read in
			do
			{
				strm.avail_out = Chunk;
				strm.next_out = (Bytef *)out;
				ret = deflate( &strm, flush );    // no bad return value
				sigassert( ret != Z_STREAM_ERROR );  // state not clobbered
				have = Chunk - strm.avail_out;

				out				+= have; // TODO convert to callback for user-specified options
				compressedBytes += have;

			} while (strm.avail_out == 0);

			sigassert(strm.avail_in == 0);     // all input will be used

			// done when last data in file processed
			in += toRead;
		}
		sigassert(ret == Z_STREAM_END);        // stream will be complete

		sigassert( compressedBytes == strm.total_out );

		// clean up and return
		deflateEnd( &strm );
		return compressedBytes;
	}

}

