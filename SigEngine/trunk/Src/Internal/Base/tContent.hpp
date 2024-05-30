//------------------------------------------------------------------------------
// \file tContent.hpp - 24 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tContent__
#define __tContent__
#include "XtlUtil.hpp"
#include "tLocalizationFile.hpp"
#include "tFilePathPtr.hpp"

namespace Sig
{
	///
	/// \class tContentData
	/// \brief 
	struct base_export tContentData
	{
		static const u32 cContentTypeSavedGame; // Player-specific content such as saved games
		static const u32 cContentTypeMarketPlace; // Content purchased on the Marketplace
		static const u32 cContentTypePublisher; // Publisher-specific content
		static const u32 cContentDeviceIdAny; // Look at any device for content

		u32 mDeviceId;
		u32 mContentType;
		tLocalizedString mDisplayName;
		CHAR mFileName[XCONTENT_MAX_FILENAME_LENGTH];
	};

	///
	/// \class tContent
	/// \brief 
	class base_export tContent
	{
	public:

		// 1. Only one of these
		static const u32 cContentFlagCreateNew;				// Creates a new container, failes if one exists
		static const u32 cContentFlagCreateAlways;			// Creates a new container, overwrites existing
		static const u32 cContentFlagOpenExisting;			// Opens a container, fails if none exists
		static const u32 cContentFlagOpenAlways;			// Opens a container, if none exists acts like CreateNew
		static const u32 cContentFlagTruncateExisting;		// Opens the container and deletes associated files

		// 2. Can be combined with many of these
		static const u32 cContentFlagNoDeviceTransfer;		// Creates content that cannot be transferred to another device
		static const u32 cContentFlagNoProfileTransfer;		// Creates content that cannot be transferred to another use
		static const u32 cContentFlagAllowProfileTransfer;	// Allows creation of saved games that can be user transferred
		static const u32 cContentFlagStrongSigned;			// Only trusted content signed by LIVE will be opened
		static const u32 cContentFlagMoveOnlyTransfer;		// Moves only, no copies

		enum tState
		{
			cStateNull = 0,
			cStateCreating,
			cStateCreated,
			cStateCorrupt,
			cStateFailed,
			cStateFlushing,
			cStateClosing
		};

	public:

		tContent( );
		tContent( const tStringPtr & root ); // Initializes in the Created state
		~tContent( );

		tState fState( ) const { return mState; }
		
		// From Null state
		b32 fCreate( 
			u32 userIndex,						// [0, tUser::cMaxLocalUsers), tUser::cUserIndexNone
			const tStringPtr & rootName,		// Root to map the content to
			const XCONTENT_DATA & contentData,	// Content data retrieved from a tContentEnumerator
			u32 contentFlags );					// See cContentFlag* above

		b32 fFlush( ); // From Created state
		b32 fClose( ); // From Created, Corrupt, or Failed state

		// Only effective in Creating, Flushing, Closing state otherwise immediate true returns
		b32 fAdvance( );

	private:

#ifdef platform_xbox360
		class tContentOp : public XtlUtil::tOverlappedOp
		{	
		public:
			tContentOp( ) { }
			~tContentOp( ) { if( !fIsComplete( ) ) fWaitToComplete( ); }

			b32 fCreate( 
				u32 userIndex,
				const tStringPtr & rootName,
				const XCONTENT_DATA & contentData,
				u32 contentFlags );

			b32 fFlush( const tStringPtr & rootName );
			b32 fClose( const tStringPtr & rootName );

		private:
			XCONTENT_DATA mContentData;
		};
#else
		class tContentOp { };
#endif

	private:

		tState mState;
		tStringPtr mRootName;
		tContentOp mContentOp;
	};
}

#endif//__tContent__
