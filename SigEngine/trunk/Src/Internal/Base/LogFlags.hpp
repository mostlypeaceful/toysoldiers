
#include "LogFlagsMacros.hpp"

define_log_flag(None,			"LOG_GROUP_NONE",			0);
define_log_flag(Rtti,			"LOG_GROUP_RTTI",			1);
define_log_flag(File,			"LOG_GROUP_FILE",			2);
define_log_flag(Thread,			"LOG_GROUP_THREAD",			3);
define_log_flag(Physics,		"LOG_GROUP_PHYSICS",		4);
define_log_flag(Input,			"LOG_GROUP_INPUT",			5);
define_log_flag(Graphics,		"LOG_GROUP_GRAPHICS",		6);
define_log_flag(Resource,		"LOG_GROUP_RESOURCE",		7);
define_log_flag(Script,			"LOG_GROUP_SCRIPT",			8);
define_log_flag(Simulation,		"LOG_GROUP_SIMULATION",		9);
define_log_flag(SceneGraph,		"LOG_GROUP_SCENEGRAPH",		10);
define_log_flag(DevMenu,		"LOG_GROUP_DEVMENU",		11);
define_log_flag(Audio,			"LOG_GROUP_AUDIO",			12);
define_log_flag(Network,		"LOG_GROUP_NETWORK",		13);
define_log_flag(Memory,			"LOG_GROUP_MEMORY",			14);
define_log_flag(Animation,		"LOG_GROUP_ANIMATION",		15);
define_log_flag(Localization,	"LOG_GROUP_LOCALIZATION",	16);
define_log_flag(Session,		"LOG_GROUP_SESSION",		17);
define_log_flag(LSP,			"LOG_GROUP_XLSP",			18);
define_log_flag(Database,		"LOG_GROUP_DATABASE",		19);
define_log_flag(Fui,			"LOG_GROUP_FUI",			20);
define_log_flag(Http,			"LOG_GROUP_HTTP",			21);

// *Warning* If you change which flag is the last flag, you must update cGameLogFlagStart

#if !defined( log_flag_register_types )
	static const u32 cGameLogFlagStart = 22;
#endif//#if !defined( log_flag_register_types )
