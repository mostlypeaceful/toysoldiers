//------------------------------------------------------------------------------
// \file tAchievements.hpp - 20 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tAchievements__
#define __tAchievements__
#include "tUser.hpp"
#include "Gui/tSaveUI.hpp"

namespace Sig
{
	///
	/// \class tAchievementsWriter
	/// \brief 
	class tAchievementsWriter : public Gui::tSaveUI::tSaveInstance
	{
	public:

		tAchievementsWriter( u32 localUserIndex, u32 count, const u32 achievements[] );
		~tAchievementsWriter( );

		virtual void fBegin( );
		virtual b32 fFinished( );
		virtual void fFinish( b32 wait );

		// no UI specifically for achievement writes.
		virtual b32 fWantsUI( ) const { return false; }

	private:

#ifdef platform_xbox360 
		struct tWriteData
		{
			tWriteData( );
			~tWriteData( );
			b32 fOverlapComplete( b32 & success );

			XOVERLAPPED mOverlapped;
		};
#else
		struct tWriteData { };
#endif 

		enum tState
		{
			cStateNull,
			cStateWriting,
			cStateSuccess,
			cStateFail
		};

	private:

		void fSetupData( u32 count, const u32 achievements[] );

	private:
		
		
		tState mState;
		tWriteData mWriteData;
		tDynamicBuffer mToWrite;
	};

	struct tAchievementData
	{
	public:
		tAchievementData( ) : mImageId( ~0u ) { }
		~tAchievementData( ) { }

		tLocalizedString mLabel;
		tLocalizedString mDescription;
		u32 mImageId;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	///
	/// \class tAchievementsReader
	/// \brief 
	class tAchievementsReader
	{
	public:

		static const u32 cDetailsLabel; // The display label
		static const u32 cDetailsDescription; // The display description
		static const u32 cDetailsUnachieved; // The display description for unachieved achievements

	public:

		enum tState
		{
			cStateNull = 0,
			cStateReading,
			cStateSuccess,
			cStateFailure
		};

	public:

		tAchievementsReader( );
		~tAchievementsReader( );

		inline tState fState( ) const { return mState; }

		void fSelectUser( u32 localHwIndex );

		// Reading
		void	fRead( u32 startIdx, u32 toRead, u32 details, tPlatformUserId target = tUser::cInvalidUserId );
		void	fCancelRead( );
		b32		fAdvanceRead( );
		void	fWait( );

		// Access
		b32		fIsAwarded( u32 id );
		b32		fGetData( u32 id, tAchievementData& out );

	private:

#ifdef platform_xbox360

		struct tReadData
		{
			tReadData( );
			~tReadData( );

			void fCloseEnumerator( );
			b32 fOverlapComplete( b32 & success, u32 & count );

			HANDLE mEnumerator;
			tDynamicArray<u8> mBuffer;
			XOVERLAPPED mOverlapped;
		};
#else
		struct tReadData{ };
#endif

	private:

		u32 mUserIndex;
		tState mState;
		tReadData mReadData;

		u32 mReadCount;
	};

}

#endif//__tAchievements__
