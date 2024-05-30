
#include "Logic/LogicEventMacros.hpp"

define_logic_event( cOnSystemUiChange,				"ON_SYSTEM_UI_CHANGE",					ApplicationEvent::cApplicationEventFlag | 1  )
define_logic_event( cOnPartyMembersChange,			"ON_PARTY_MEMBERS_CHANGE",				ApplicationEvent::cApplicationEventFlag | 2  )

define_logic_event( cSessionCreated,				"SESSION_CREATED",						ApplicationEvent::cApplicationEventFlag | 3  )
define_logic_event( cSessionStarted,				"SESSION_STARTED",						ApplicationEvent::cApplicationEventFlag | 4  )
define_logic_event( cSessionEnded,					"SESSION_ENDED",						ApplicationEvent::cApplicationEventFlag | 5  )
define_logic_event( cSessionDeleted,				"SESSION_DELETED",						ApplicationEvent::cApplicationEventFlag | 6  )
define_logic_event( cSessionSearchComplete,			"SESSION_SEARCH_COMPLETE",				ApplicationEvent::cApplicationEventFlag | 7  )
define_logic_event( cSessionUsersChanged,			"SESSION_USERS_CHANGED",				ApplicationEvent::cApplicationEventFlag | 8  )
define_logic_event( cSessionFilled,					"SESSION_FILLED",						ApplicationEvent::cApplicationEventFlag | 9  )
define_logic_event( cSessionLoadLevel,				"SESSION_LOAD_LEVEL",					ApplicationEvent::cApplicationEventFlag | 10 )
define_logic_event( cSessionInviteAccepted,			"SESSION_INVITE_ACCEPTED",				ApplicationEvent::cApplicationEventFlag | 11 )
define_logic_event( cSessionInviteNeedFullVer,		"SESSION_INVITE_NEED_FULL_VERISON",		ApplicationEvent::cApplicationEventFlag | 12 )
define_logic_event( cSessionInviteRejected,			"SESSION_INVITE_REJECTED",				ApplicationEvent::cApplicationEventFlag | 13 )
define_logic_event( cSessionRejected,				"SESSION_REJECTED",						ApplicationEvent::cApplicationEventFlag | 14 )
define_logic_event( cSessionStatsLost,				"SESSION_STATS_LOST",					ApplicationEvent::cApplicationEventFlag | 15 )
define_logic_event( cOnDisconnect,					"ON_DISCONNECT",						ApplicationEvent::cApplicationEventFlag | 16 )
																															   
define_logic_event( cOnPlayerNoLive,				"ON_PLAYER_NO_LIVE",					ApplicationEvent::cApplicationEventFlag | 17 )
define_logic_event( cOnPlayerYesLive,				"ON_PLAYER_YES_LIVE",					ApplicationEvent::cApplicationEventFlag | 18 )
define_logic_event( cOnPlayerSignOut,				"ON_PLAYER_SIGN_OUT",					ApplicationEvent::cApplicationEventFlag | 19 )
define_logic_event( cOnPlayerSignIn,				"ON_PLAYER_SIGN_IN",					ApplicationEvent::cApplicationEventFlag | 20 )
define_logic_event( cOnPlayerLoseInput,				"ON_PLAYER_LOSE_INPUT",					ApplicationEvent::cApplicationEventFlag | 21 )
define_logic_event( cOnPlayerInputActivityChange,	"ON_PLAYER_INPUT_ACTIVITY_CHANGE",		ApplicationEvent::cApplicationEventFlag | 22 )
																															   
define_logic_event( cOnMenuStateChange,				"LOBBY_MENU_STATE_CHANGE",				ApplicationEvent::cApplicationEventFlag | 23 )
define_logic_event( cOnClientStateChange,			"LOBBY_CLIENT_STATE_CHANGE",			ApplicationEvent::cApplicationEventFlag | 24 )

define_logic_event( cProflieStorageDeviceRemoved,	"ON_PROFILE_STORAGE_DEVICE_REMOVED",	ApplicationEvent::cApplicationEventFlag | 25 )
define_logic_event( cOnUpgradeToFullVersion,		"ON_UPGRADE_TO_FULL_VERSION",			ApplicationEvent::cApplicationEventFlag | 26 )
