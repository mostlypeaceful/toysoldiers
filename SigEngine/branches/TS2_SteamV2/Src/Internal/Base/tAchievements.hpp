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
#if defined( use_steam )
	namespace Achievements
	{
		extern tDelegate< const tStringPtr& ( u32 ) > sAchievementIdToValueString;
	}
#endif

	///
	/// \class tAchievementsWriter
	/// \brief 
	class tAchievementsWriter : public Gui::tSaveUI::tSaveInstance
	{
	public:

		tAchievementsWriter( u32 localUserIndex, u32 count, const u32 achievements[] );

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
			b32 fOverlapComplete( b32 & success );

			XOVERLAPPED mOverlapped;
		};
#elif defined( use_steam )
		class tWriteData
		{
		public:
			tWriteData();
			virtual ~tWriteData();

			void fCancel( );
			b32 fOverlapComplete( b32 & success );
			void fWrite( tDynamicArray< tStringPtr >& achievements );

		private:
			enum tState
			{
				cStateNone,
				cStateWriting,
				cStateDone,
				cStateError,
			};

			tState mState;

			void fClear( );
			STEAM_CALLBACK( tWriteData, fOnUserStatsStored, UserStatsStored_t, mCallbackUserStatsStored);
			STEAM_CALLBACK( tWriteData, fOnUserAchievementStored, UserAchievementStored_t, mCallbackUserAchievementStored);
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
#if defined( platform_xbox360 )
		tDynamicBuffer mToWrite;
#elif defined( use_steam )
		tDynamicArray< tStringPtr > mToWrite;
#endif
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
	class tAchievementsReader : public tRefCounter
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

			void fCloseEnumerator( );
			b32 fOverlapComplete( b32 & success, u32 & count );

			HANDLE mEnumerator;
			tDynamicArray<u8> mBuffer;
			XOVERLAPPED mOverlapped;
		};
#elif defined( use_steam )
		class tReadData
		{
		public:
			tReadData();
			virtual ~tReadData();

			void fCancel( );
			b32 fOverlapComplete( b32 & success, u32 & count );
			void fRead( tPlatformUserId target );

		private:
			enum tState
			{
				cStateNone,
				cStateReading,
				cStateDone,
				cStateError,
			};

			tState mState;

			void fClear( );
			//void fOnUserStatsReceived( UserStatsReceived_t *pResult, bool bIOFailure );
			//CCallResult< tReadData, UserStatsReceived_t > mCallbackUserStatsReceived;
			STEAM_CALLBACK( tReadData, fOnUserStatsReceived, UserStatsReceived_t, mCallbackUserStatsReceived);
		};
#else
		struct tReadData { };
#endif

	private:

		u32 mUserIndex;
		tState mState;
		tReadData mReadData;

		u32 mReadCount;
	};

	typedef tRefCounterPtr< tAchievementsWriter > tAchievementsWriterPtr;
	typedef tRefCounterPtr< tAchievementsReader > tAchievementsReaderPtr;
}

#endif//__tAchievements__
