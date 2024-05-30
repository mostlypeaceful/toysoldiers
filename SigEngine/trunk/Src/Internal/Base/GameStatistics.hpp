#ifndef __GameStatistics__
#define __GameStatistics__

/// For reporting gameplay statistics (to e.g. Microsoft via the Sentient SDK as apparently now mandated for Metro/MoLIVE titles)
/// Trying to make this as self-contained and API-agnostic as possible

namespace Sig { namespace GameStatistics
{
	///    Marketplace statistics
	///
	/// We report upsells when we showed an upsell notice/screen/other bit of UI.  Explicit event to help track ignored offers.
	/// We also report the "response" to each upsell
	///
	/// promptId = optional game defined identifier for which upsell dialog/prompt we enticed the user with
	/// dlcid = game defined identifier

	enum tUpsellResponse
	{
		cUpsellResponseBought,		///< User bought it
		cUpsellResponseDeclined,	///< User closed an upsell window/dialog/prompt
		cUpsellResponseIgnored,		///< User navigated away from a UI which contained upsell prompts but wasn't exclusively upsell prompts, or the upsell faded away after a timeout.
	};

	void base_export fReportUpsellFullGame( u32 promptId = ~0u );
	void base_export fReportUpsellFullGameResponse( tUpsellResponse response, u32 promptId = ~0u );

	void base_export fReportUpsellDlc( u32 dlcid, u32 promptId = ~0u );
	void base_export fReportUpsellDlcResponse( u32 dlcid, tUpsellResponse, u32 promptId = ~0u );

	///    General state statistics
	///

	enum tGameEvent
	{
		cGameEventStarted,		///< Process starting
		cGameEventShutdown,		///< Process shutting down / being killed
		cGameEventSuspending,	///< Process being suspended (e.g. Metro/iOS application suspension notifications, Windows hibernating, etc)
		cGameEventResuming,		///< Process being resumed (e.g. Metro application resumed from hibernation, Windows restored from hibernation, etc).  N.B. Sentient initializes here
	};
	void base_export fReportGameEvent( tGameEvent event );	///< N.B. Sentient initializes on some of these events

	enum tWindowStyle
	{
		cWindowStyleNone,					/// Platform doesn't have any form of multitasking.
		cWindowStyleUnknown,				/// Something funky happened.

		cWindowStyleFullscreen,				///< Prefer cWindowStyle{Metro,Pc}Fullscreen on platforms with actual windows.
		cWindowStyleMinimized,				///< Prefer cWindowStyle{Metro,Pc}Minimized on platforms with actual windows.

		cWindowStyleMetroFullscreen,		///< "100%" (fullscreen)
		cWindowStyleMetroPartial,			///< "70%"	(fullscreen - 320px wide)
		cWindowStyleMetroSnapped,			///< "30%"	(320px wide)
		cWindowStyleMetroMinimized,			///< Not currently visible

		cWindowStylePcFullscreenExclusive,	///< Full screen exclusive mode
		cWindowStylePcFullscreenWindowed,	///< Full screen non-exclusive borderless mode
		cWindowStylePcWindowed,				///< Traditional bordered windowed mode
		cWindowStylePcWindowedBorderless,	///< Window in non-bordered mode
		cWindowStylePcMinimized,			///< Not currently visible
	};

	void base_export fReportWindowLayoutChanged( u32 width, u32 height, tWindowStyle windowStyle );
	void base_export fReportLogin( u32 hwIndex, u64 userId );	///< N.B. Sentient initializes here
	void base_export fReportLogout( u32 hwIndex );

	///    Gameplay statistics

	void base_export fReportAchievementUnlocked( u32 achievementId );

	void base_export fReportLevelStarted( u32 levelType, u32 levelId );	///< Started from scratch
	void base_export fReportLevelResumed( u32 saveid );					///< Resumed from a save game/point
	u32  base_export fReportLevelSaved( u32 overwriting = ~0u );		///< Assumes a level is in progress
	void base_export fReportLevelRestarted( );							///< Assumes a level is in progress

	enum tLevelEvent
	{
		cLevelEventWon,		///< User won the level
		cLevelEventLost,	///< User lost the level
		cLevelEventQuit,	///< User quit the level before winning it or losing it
	};
	void base_export fReportLevelEvent( tLevelEvent event );			///< Assumes a level is in progress

	void base_export fExportScriptInterface( tScriptVm& vm );
}}

#endif //ndef __GameStatistics__
