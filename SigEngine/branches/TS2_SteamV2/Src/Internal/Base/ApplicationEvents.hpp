
#include "Logic/LogicEventMacros.hpp"

define_logic_event( cOnSystemUiChange,			"ON_SYSTEM_UI_CHANGE",				Logic::tEvent::cApplicationEventFlag | 1  )
define_logic_event( cOnPartyMembersChange,		"ON_PARTY_MEMBERS_CHANGE",			Logic::tEvent::cApplicationEventFlag | 2  )

define_logic_event( cSessionCreated,			"SESSION_CREATED",					Logic::tEvent::cApplicationEventFlag | 3  )
define_logic_event( cSessionStarted,			"SESSION_STARTED",					Logic::tEvent::cApplicationEventFlag | 4  )
define_logic_event( cSessionEnded,				"SESSION_ENDED",					Logic::tEvent::cApplicationEventFlag | 5  )
define_logic_event( cSessionDeleted,			"SESSION_DELETED",					Logic::tEvent::cApplicationEventFlag | 6  )
define_logic_event( cSessionSearchComplete,		"SESSION_SEARCH_COMPLETE",			Logic::tEvent::cApplicationEventFlag | 7  )
define_logic_event( cSessionUsersChanged,		"SESSION_USERS_CHANGED",			Logic::tEvent::cApplicationEventFlag | 8  )
define_logic_event( cSessionFilled,				"SESSION_FILLED",					Logic::tEvent::cApplicationEventFlag | 9  )
define_logic_event( cSessionLoadLevel,			"SESSION_LOAD_LEVEL",				Logic::tEvent::cApplicationEventFlag | 10 )
define_logic_event( cSessionInviteAccepted,		"SESSION_INVITE_ACCEPTED",			Logic::tEvent::cApplicationEventFlag | 11 )
define_logic_event( cSessionInviteNeedFullVer,	"SESSION_INVITE_NEED_FULL_VERISON", Logic::tEvent::cApplicationEventFlag | 12 )
define_logic_event( cSessionInviteRejected,		"SESSION_INVITE_REJECTED",			Logic::tEvent::cApplicationEventFlag | 13 )
define_logic_event( cSessionRejected,			"SESSION_REJECTED",					Logic::tEvent::cApplicationEventFlag | 14 )
define_logic_event( cSessionStatsLost,			"SESSION_STATS_LOST",				Logic::tEvent::cApplicationEventFlag | 15 )
define_logic_event( cOnDisconnect,				"ON_DISCONNECT",					Logic::tEvent::cApplicationEventFlag | 16 )
																														   
define_logic_event( cOnPlayerNoLive,			"ON_PLAYER_NO_LIVE",				Logic::tEvent::cApplicationEventFlag | 17 )
define_logic_event( cOnPlayerYesLive,			"ON_PLAYER_YES_LIVE",				Logic::tEvent::cApplicationEventFlag | 18 )
define_logic_event( cOnPlayerSignOut,			"ON_PLAYER_SIGN_OUT",				Logic::tEvent::cApplicationEventFlag | 19 )
define_logic_event( cOnPlayerSignIn,			"ON_PLAYER_SIGN_IN",				Logic::tEvent::cApplicationEventFlag | 20 )
define_logic_event( cOnPlayerLoseInput,			"ON_PLAYER_LOSE_INPUT",				Logic::tEvent::cApplicationEventFlag | 21 )
																														   
define_logic_event( cOnMenuStateChange,			"LOBBY_MENU_STATE_CHANGE",			Logic::tEvent::cApplicationEventFlag | 22 )
define_logic_event( cOnClientStateChange,		"LOBBY_CLIENT_STATE_CHANGE",		Logic::tEvent::cApplicationEventFlag | 23 )

define_logic_event( cProflieStorageDeviceRemoved,	"ON_PROFILE_STORAGE_DEVICE_REMOVED",	Logic::tEvent::cApplicationEventFlag | 24 )
define_logic_event( cOnUpgradeToFullVersion,		"ON_UPGRADE_TO_FULL_VERSION",			Logic::tEvent::cApplicationEventFlag | 25 )
define_logic_event( cSessionInviteNeedSameVersion,	"SESSION_INVITE_NEED_SAME_VERSION", Logic::tEvent::cApplicationEventFlag | 26 )
