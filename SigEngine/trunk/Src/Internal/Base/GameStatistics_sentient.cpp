#include "BasePch.hpp"
#include "GameStatistics.hpp"
#include "SentientConfig.hpp"

#if defined( sig_use_sentient )
#include "tGameAppbase.hpp"

using namespace ::Microsoft::Games::Sentient;

namespace Sig { namespace GameStatistics
{
	namespace ImplementationDetails
	{
		/// Game metadata.  TODO: Hoist out of Base via e.g. fSetGameStatisticsMetadata or the like.
		Platform::String^ const gBuildNumber = "0.0.0.0";
		//const Platform::Guid gFullGameGuid(
		//const Platform::Guid gDlcGuids[] = { __uuidof(2), __uuidof(3), __uuidof(4) };

		// {94B08810-46E0-4577-B851-9929683F5AAF}
		static const GUID gFullGameGuid =  { 0x94b08810, 0x46e0, 0x4577, { 0xb8, 0x51, 0x99, 0x29, 0x68, 0x3f, 0x5a, 0xaf } };
		static const GUID gDlcGuids[] =
		{
			gFullGameGuid,
			{ 0x7c1ba72b, 0x3883, 0x49e9, { 0x9b, 0x14, 0xc1, 0x88, 0x63, 0x46, 0xc4, 0xac } },
			{ 0x9e5dc2f8, 0xefd0, 0x483d, { 0xa0, 0x93, 0x6d, 0x5c, 0xa9, 0x6d, 0x8b, 0xb4 } },
		};

		b32 fValidDlcIndex( u32 index ) { return index < array_length(gDlcGuids); }
		

		/// Current state tracking
		u32 gModelId = 0, gSubModelId = 0;
		u32 gLevelId = 0, gSubLevelId = 0;
		u32 gMultiplayerInstanceId = 0;
		LandscapeOrPortrait gPreviousOrientation = LandscapeOrPortrait::Undefined;
	}

	using namespace ImplementationDetails;




	///    Marketplace statistics
	///

	void fReportUpsellFullGame ( u32 promptId )
	{
		Events::UpsellPresented( gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId, promptId, gFullGameGuid );
	}

	void fReportUpsellFullGameResponse( tUpsellResponse response, u32 promptId )
	{
		switch( response )
		{
		case cUpsellResponseBought:		Events::UpsellResponded( gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId, promptId, gFullGameGuid, UpsellOutcome::Accepted ); break;
		case cUpsellResponseDeclined:	Events::UpsellResponded( gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId, promptId, gFullGameGuid, UpsellOutcome::Declined ); break;
		case cUpsellResponseIgnored:	/* No event. */ break;
		default:						log_assert( 0, !"Invalid tUpsellResponse!" ); break;
		}
	}

	void fReportUpsellDlc( u32 dlcid, u32 promptId )
	{
		if( !fValidDlcIndex( dlcid ) )
		{
			log_warning( 0, "Invalid DLC index!" );
			return;
		}

		Events::UpsellPresented( gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId, promptId, gDlcGuids[dlcid] );
	}

	void fReportUpsellDlcResponse( u32 dlcid, tUpsellResponse response, u32 promptId )
	{
		if( !fValidDlcIndex( dlcid ) )
		{
			log_warning( 0, "Invalid DLC index!" );
			return;
		}
		
		switch( response )
		{
		case cUpsellResponseBought:		Events::UpsellResponded( gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId, promptId, gDlcGuids[dlcid], UpsellOutcome::Accepted ); break;
		case cUpsellResponseDeclined:	Events::UpsellResponded( gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId, promptId, gDlcGuids[dlcid], UpsellOutcome::Declined ); break;
		case cUpsellResponseIgnored:	/* No event. */ break;
		default:						log_assert( 0, !"Invalid tUpsellResponse!" ); break;
		}
	}

	///    General state statistics
	///

	void fReportGameEvent( tGameEvent event )
	{
		switch( event )
		{
		case cGameEventStarted: ///< Sentient not initialized on startup: Requires user to be logged in.
		case cGameEventShutdown:
		case cGameEventSuspending:
			break;
		case cGameEventResuming:
			Sentient::Initialize( gBuildNumber );
			break;
		default:
			break;
		}
	}
	void fReportWindowLayoutChanged( u32 width, u32 height, tWindowStyle windowStyle )
	{
		LandscapeOrPortrait newOrientation = (width>=height) ? LandscapeOrPortrait::Landscape : LandscapeOrPortrait::Portrait;
		if( gPreviousOrientation != newOrientation )
			Events::GameViewChanged( gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId, newOrientation );
		gPreviousOrientation = newOrientation;
	}
	void fReportLogin( u32 hwIndex, u64 userId )
	{
		Sentient::Initialize(gBuildNumber);
	}
	void fReportLogout( u32 hwIndex )
	{
	}

	///    Gameplay statistics

	void fReportAchievementUnlocked( u32 achievementId )
	{
		Events::AchievementUnlocked( gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId, achievementId, 0 ); /// ignoring the gamerscore param for now
	}
	void fReportLevelStarted( u32 levelType, u32 levelId )
	{
		gModelId = 100 * levelType;
		gLevelId = 100 * levelType + levelId;

		Events::LevelStart(
			gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId,
			gMultiplayerInstanceId==0 ? SingleOrMultiplayer::Single : SingleOrMultiplayer::Multiplayer_Live,
			FriendOrMatch::Undefined,
			CompeteOrCoop::Undefined,
			DifficultyLevel::Undefined,
			1,									// local players
			gMultiplayerInstanceId==0 ? 0 : 1,	// online players
			tGameAppBase::fInstance( ).fIsFullVersion() ? LicenseLevel::Purchased : LicenseLevel::Trial,
			GameControls::Undefined,
			AudioSettings::Undefined,
			gPreviousOrientation );

	}
	void fReportLevelResumed( u32 saveid )
	{
		Events::LevelResume(
			gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId,
			gMultiplayerInstanceId==0 ? SingleOrMultiplayer::Single : SingleOrMultiplayer::Multiplayer_Live,
			FriendOrMatch::Undefined,
			CompeteOrCoop::Undefined,
			DifficultyLevel::Undefined,
			1,									// local players
			gMultiplayerInstanceId==0 ? 0 : 1,	// online players
			tGameAppBase::fInstance( ).fIsFullVersion() ? LicenseLevel::Purchased : LicenseLevel::Trial,
			GameControls::Undefined,
			AudioSettings::Undefined,
			gPreviousOrientation,
			saveid );
	}
	u32  fReportLevelSaved( u32 overwriting )
	{
		if( overwriting == ~0u )
			log_warning( 0, "Sentient expects game-defined save IDs / overwriting" );

		Events::LevelSaveOrCheckpoint(
			gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId,
			0, // exit progression stat 1
			0, // exit progression stat 2
			0, // duration in seconds
			overwriting );

		return overwriting;
	}

	void fReportLevelRestarted( )
	{
		log_warning_unimplemented(0);
	}

	void fReportLevelEvent( tLevelEvent event )
	{
		LevelExitStatus status = LevelExitStatus::Undefined;

		switch( event )
		{
		case cLevelEventWon:	status = LevelExitStatus::Succeeded; break;
		case cLevelEventLost:	status = LevelExitStatus::Failed; break;
		case cLevelEventQuit:	status = LevelExitStatus::Exited; break;
		default:				log_assert( 0, "Invalid tLevelEvent!" ); break;
		}

		Events::LevelExit(
			gModelId, gSubModelId, gLevelId, gSubLevelId, gMultiplayerInstanceId,
			status,
			0, // exit progression stat 1
			0, // exit progression stat 2
			0 ); // duration in seconds
	}
}}

#endif // defined( sig_use_sentient )
