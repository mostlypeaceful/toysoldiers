//------------------------------------------------------------------------------
// \file tJsonGameArchiveSaver.impl.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonGameArchiveSaver_impl__
#define __tJsonGameArchiveSaver_impl__

#include "tJsonGameArchive_SaveLoadIntrinsics.hpp"

namespace Sig
{
	template< class t > void tJsonGameArchiveSaver::fSave( t& object, u8 version )
	{
		mHeader.mPlatform = cCurrentPlatform;
		mHeader.mVersion = version;
		sigcheckfail( mWriter.fBeginObject( )								, fFail( ); return );
		sigcheckfail( mWriter.fWriteField( "platform", mHeader.mPlatform )	, fFail( ); return );
		sigcheckfail( mWriter.fWriteField( "version", mHeader.mVersion )	, fFail( ); return );
		sigcheckfail( mWriter.fBeginField( "data" )							, fFail( ); return );
		mSafeToSaveLoad = true;
		fSaveLoad( object );
		mSafeToSaveLoad = false;
		sigcheckfail( mWriter.fEndField( )									, fFail( ); return );
		sigcheckfail( mWriter.fEndObject( )									, fFail( ); return );
	}

	template< class t > void tJsonGameArchiveSaver::fSaveLoad( t& object )
	{
		log_sigcheckfail( mSafeToSaveLoad, "You must call fSave before fSaveLoad", return );
		if( !fFailed( ) )
			tJsonGameArchive_SaveLoadIntrinsics< t >::fSave( *this, object );
	}
}

#endif //ndef __tJsonGameArchiveSaver_impl__
