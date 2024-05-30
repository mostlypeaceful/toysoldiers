#include "BasePch.hpp"
#include "GameStatistics.hpp"
#include "SentientConfig.hpp"

#if !defined( sig_use_sentient )

namespace Sig { namespace GameStatistics
{
	namespace
	{
		void fWarnNotImplemented()
		{
			log_warning_once( "Sig::GameStatistics not implemented/enabled for this platform" );
		}
	}

	///    Marketplace statistics
	///

	void fReportUpsellFullGame( u32 promptId ) { fWarnNotImplemented(); }
	void fReportUpsellFullGameResponse( tUpsellResponse response, u32 promptId ) { fWarnNotImplemented(); }

	void fReportUpsellDlc( u32 dlcid, u32 promptId ) { fWarnNotImplemented(); }
	void fReportUpsellDlcResponse( u32 dlcid,  tUpsellResponse response,u32 promptId ) { fWarnNotImplemented(); }

	///    General state statistics
	///

	void fReportGameEvent( tGameEvent event ) { fWarnNotImplemented(); }
	void fReportWindowLayoutChanged( u32 width, u32 height, tWindowStyle windowStyle ) { fWarnNotImplemented(); }
	void fReportLogin( u32 hwIndex, u64 userId ) { fWarnNotImplemented(); }
	void fReportLogout( u32 hwIndex ) { fWarnNotImplemented(); }

	///    Gameplay statistics

	void fReportAchievementUnlocked( u32 achievementId ) { fWarnNotImplemented(); }

	void fReportLevelStarted( u32 levelType, u32 levelId ) { fWarnNotImplemented(); }
	void fReportLevelResumed( u32 saveid ) { fWarnNotImplemented(); }
	void fReportLevelRestarted( ) { fWarnNotImplemented(); }
	u32  fReportLevelSaved( u32 overwriting )
	{
		fWarnNotImplemented();
		return ~0u;
	}

	void fReportLevelEvent( tLevelEvent event ) { fWarnNotImplemented(); }
}}

#endif // !defined( sig_use_sentient )
