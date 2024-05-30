//------------------------------------------------------------------------------
// \file tJsonGameArchiveSaver.cpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tJsonGameArchiveSaver.hpp"

namespace Sig
{
	tJsonGameArchiveSaver::tJsonGameArchiveSaver( )
		: tJsonGameArchive( cGameArchiveModeSave )
	{
	}

	tJsonWriter& tJsonGameArchiveSaver::fWriter( )
	{
		return mWriter;
	}

	const tJsonWriter& tJsonGameArchiveSaver::fWriter( ) const
	{
		return mWriter;
	}

	void tJsonGameArchiveSaver::fTransferBufferTo( tGrowableBuffer& target )
	{
		target.fSetCount( mWriter.fGetBufferSize( ) ); // sufficiently large?
		u32 actualSize = target.fCount( );
		sigcheckfail( mWriter.fGetBuffer( reinterpret_cast< char* >( target.fBegin( ) ), &actualSize ), target.fSetCount( 0 ); return );
		sigcheckfail( actualSize == target.fCount( ), target.fSetCount( 0 ); return );
		target.fPopBack( ); // Don't include the terminal '\0'
	}
}
