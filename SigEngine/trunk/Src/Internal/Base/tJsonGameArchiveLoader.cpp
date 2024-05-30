//------------------------------------------------------------------------------
// \file tJsonGameArchiveLoader.cpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tJsonGameArchiveLoader.hpp"

namespace Sig
{
	tJsonGameArchiveLoader::tJsonGameArchiveLoader( const byte* data, u32 dataLength )
		: tJsonGameArchive( cGameArchiveModeLoad )
		, mReader( reinterpret_cast< const char* >( data ), dataLength )
		, mTotalJsonSize( dataLength )
	{
		tJsonReader peek( reinterpret_cast< const char* >( data ), dataLength );
		fReadHeader( peek );
	}

	u32 tJsonGameArchiveLoader::fPeekHeaderVersion( )
	{
		return mHeader.mVersion;
	}

	tJsonReader& tJsonGameArchiveLoader::fReader( )
	{
		return mReader;
	}

	const tJsonReader& tJsonGameArchiveLoader::fReader( ) const
	{
		return mReader;
	}

	b32 tJsonGameArchiveLoader::fSanityCheckArrayAlloc( u32 typeSize, u32 arrayCount )
	{
		log_sigcheckfail
			( arrayCount <= mTotalJsonSize
			, "tJsonGameArchiveLoader::fSanityCheckArrayAlloc failed: total size (" << mTotalJsonSize << ") < arrayCount (" << arrayCount << ")"
			, fFail( )
			);
		return !fFailed();
	}

	void tJsonGameArchiveLoader::fReadHeader( tJsonReader& reader )
	{
		sigcheckfail( reader.fBeginObject( )							, fFail( ); return );
		sigcheckfail( reader.fGetField( "platform", mHeader.mPlatform )	, fFail( ); return );
		sigcheckfail( reader.fGetField( "version", mHeader.mVersion )	, fFail( ); return );
		sigcheckfail( reader.fGetField( "data" )						, fFail( ); return );
	}

	void tJsonGameArchiveLoader::fReadFooter( tJsonReader& reader )
	{
		sigcheckfail( reader.fEndObject( )								, fFail( ); return );
	}
}
