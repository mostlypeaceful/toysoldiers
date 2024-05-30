#include "BasePch.hpp"
#include "GameStatistics.hpp"

namespace Sig { namespace GameStatistics
{
	namespace
	{
		void fReportUpsellFullGameResponseFromScript( u32 response, u32 promptId )
		{
			fReportUpsellFullGameResponse( (tUpsellResponse)response, promptId );
		}

		void fReportUpsellDlcResponseFromScript( u32 dlcid, u32 response, u32 promptId )
		{
			fReportUpsellDlcResponse( dlcid, (tUpsellResponse)response, promptId );
		}

		void fReportEventFromScript( u32 event )
		{
			fReportGameEvent((tGameEvent)event);
		}

		void fReportLevelEventFromScript( u32 event )
		{
			fReportLevelEvent( (tLevelEvent)event );
		}
	}



	void fExportScriptInterface( tScriptVm& vm )
	{
		vm.fNamespace(_SC("GameStatistics"))
			.Func(_SC("ReportUpsellFullGame"),&fReportUpsellFullGame)
			.Func(_SC("ReportUpsellFullGameResponse"),&fReportUpsellFullGameResponseFromScript)
			.Func(_SC("ReportUpsellDlc"),&fReportUpsellDlc)
			.Func(_SC("ReportUpsellDlcResponse"),&fReportUpsellDlcResponseFromScript)

			.Func(_SC("ReportLevelStarted"),&fReportLevelStarted)
			.Func(_SC("ReportLevelResumed"),&fReportLevelResumed)
			.Func(_SC("ReportLevelSaved"),&fReportLevelSaved)
			.Func(_SC("ReportLevelRestarted"),&fReportLevelRestarted)
			.Func(_SC("ReportLevelEvent"),&fReportLevelEventFromScript)
			;

		vm.fConstTable( )
			.Const(_SC("GAME_STATS_UPSELL_RESPONSE_BOUGHT"),	cUpsellResponseBought)
			.Const(_SC("GAME_STATS_UPSELL_RESPONSE_DECLINED"),	cUpsellResponseDeclined)
			.Const(_SC("GAME_STATS_UPSELL_RESPONSE_IGNORED"),	cUpsellResponseIgnored)

			.Const(_SC("GAME_STATS_LEVEL_EVENT_WON"),	cLevelEventWon)
			.Const(_SC("GAME_STATS_LEVEL_EVENT_LOST"),	cLevelEventLost)
			.Const(_SC("GAME_STATS_LEVEL_EVENT_QUIT"),	cLevelEventQuit)
			;
	}
}}
