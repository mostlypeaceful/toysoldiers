//------------------------------------------------------------------------------
// \file tStringVerify.cpp - 12 Dec 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tStringVerify.hpp"
#include "tLocalizationFile.hpp"

namespace Sig
{
	const u32 tStringVerifyOpBase::cMaxStringCount = 10; // From XStringVerify docs

	//------------------------------------------------------------------------------
	// tStringVerifyOpBase
	//------------------------------------------------------------------------------
	const wchar_t* tStringVerifyOpBase::fText( u32 index )
	{
		return mStorage.fBegin( ) + mStrings[ index ];
	}

	//------------------------------------------------------------------------------
	u32 tStringVerifyOpBase::fTextLength( u32 index )
	{
		sigassert( index < mStrings.fCount( ) );
		const u32 cEnd = ( index == mStrings.fCount( ) - 1 ? mStorage.fCount( ) : mStrings[ index + 1 ] );
		return  cEnd - mStrings[ index ] - 1;
	}

	//------------------------------------------------------------------------------
	void tStringVerifyOpBase::fAddString( const tLocalizedString& text )
	{
		fAddString( text.fCStr( ), text.fLength( ) );
	}

	//------------------------------------------------------------------------------
	void tStringVerifyOpBase::fAddString( const wchar_t text[], u32 length )
	{
		if( length == ~0 )
			length = fNullTerminatedLength( text );

		mStrings.fPushBack( mStorage.fCount( ) );
		mStorage.fInsert( mStorage.fCount( ), text, length + 1 );
	}

	//------------------------------------------------------------------------------
	// tStringVerifyOp
	//------------------------------------------------------------------------------

#if defined( platform_xbox360 )

	//------------------------------------------------------------------------------
	b32 tStringVerifyOp::fBeginVerify( )
	{
		if( !mStrings.fCount( ) )
			return false;

		if( !fPreExecute( ) )
			return false;

		const u32 cStringCount = mStrings.fCount( );
		mInputs.fNewArray( cStringCount );

		for( u32 i = 0; i < cStringCount; ++i )
		{
			mInputs[ i ].pszString = (WCHAR*)fText( i );
			mInputs[ i ].wStringSize = fTextLength( i ) + 1;
		}

		mResults.fNewArray( sizeof( STRING_VERIFY_RESPONSE ) + ( sizeof( HRESULT ) * cStringCount ) );

		DWORD result = XStringVerify( 
			0, 
			"en-us", 
			cStringCount, 
			mInputs.fBegin( ), 
			mResults.fCount( ), 
			(STRING_VERIFY_RESPONSE*)mResults.fBegin( ),
			fOverlapped( ) );

		if( result != ERROR_IO_PENDING )
		{
			log_warning( "XStringVerify failed with result: " << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tStringVerifyOp::fIsComplete( ) const
	{
		return XtlUtil::tOverlappedOp::fIsComplete( );
	}

	//------------------------------------------------------------------------------
	b32 tStringVerifyOp::fIsTextValid( u32 index ) const
	{
		sigassert( fIsComplete( ) && "Cannot check results till operation is complete!" );

		const STRING_VERIFY_RESPONSE* response = (const STRING_VERIFY_RESPONSE*)mResults.fBegin( );

		if( response->wNumStrings != mStrings.fCount( ) )
			return false;

		return SUCCEEDED( response->pStringResult[ index ] );
	}

#endif // platform_xbox360

} // ::Sig
