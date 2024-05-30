#ifndef __tCachedScoreWriterSession__
#define __tCachedScoreWriterSession__

#include "tGameSession.hpp"

namespace Sig
{
	class tGameApp;
	class tGameSession;
	class tGameSessionSearch;
	class tGameArchiveLoad;
	class tPlayer;

	///
	/// \class tCachedScoreWriterSession
	/// \brief 
	class tCachedScoreWriterSession : public tRefCounter
	{
	public:
		tCachedScoreWriterSession( );
		~tCachedScoreWriterSession( );

		void fReset( );

		void fQueueStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] );
		b32 fCanWriteStats( );
		b32 fStartSession( const tUserPtr& user );
		void fOnUserSignInChange( const tUserSigninInfo oldStates[], const tUserSigninInfo newStates[] );
		void fTick( );

	private:
		struct tStatsWrite : public tRefCounter
		{
			tPlatformUserId mUserId;
			u32 mViewId;
			tDynamicArray< tUserProperty > mProperties;
		};
		typedef tRefCounterPtr< tStatsWrite > tStatsWritePtr;

	private:
		void fGameSessionCallback( tGameSession& gameSession, u32 oldState, b32 success );

		tRingBuffer< tStatsWritePtr > mQueuedWrites;
		tGameSessionPtr mSession;
		tUserPtr mUser;

		u32 mStatsWritten;
	};

	typedef tRefCounterPtr<tCachedScoreWriterSession> tCachedScoreWriterSessionPtr;
}

#endif//__tCachedScoreWriterSession__