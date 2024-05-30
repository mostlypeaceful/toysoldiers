//------------------------------------------------------------------------------
// \file tContentEnumerator.hpp - 23 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tContentEnumerator__
#define __tContentEnumerator__
#include "XtlUtil.hpp"
#include "tContent.hpp"

namespace Sig
{
	

	///
	/// \class tContentEnumerator
	/// \brief 
	class tContentEnumerator 
	{
	public:

		enum tState
		{
			cStateNull = 0,
			cStateCreated,
			cStateEnumerating,
			cStateSuccess,
			cStateFail
		};

	public:

		tContentEnumerator( );

		inline tState fState( ) const { return mState; }

		// Step 1
		b32 fCreate( 
			u32 userIndex,			// [ 0 , tUser::cMaxLocalUsers )
			u32 deviceId,			// A device id or tContentData::cContentDeviceIdAny for any device
			u32 contentType,		// Any of tContentData::cContentType*
			b32 userSpecificOnly,	// Exclude all content not specific to the specified user
			u32 numItems );			// Maximum number of items to return

		// Step 2
		b32 fEnumerate( );

		// Step 3
		b32 fAdvance( );			// Returns if the enumeration is finished
		void fWait( );

		// Step 4
		u32 fResultCount( );
		void fResult( u32 idx, tContentData & data, XCONTENT_CROSS_TITLE_DATA& dataOut );

		// Step 5, if returning to Step 1
		void fDestroy( );
		
	private:

		tState mState;

#ifdef platform_xbox360
		XtlUtil::tEnumerateOp mEnumOp;
#endif
	};
}

#endif//__tContentEnumerator__
