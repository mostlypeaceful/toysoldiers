
#ifdef define_log_flag
#	undef define_log_flag
#endif//define_log_flag

#ifdef log_flag_register_types
#	define define_log_flag( name, scriptName, id )		Log::tFlagsType( id, #name, "Log:"#name, scriptName );
#else//log_flag_register_types
#	define define_log_flag( name, scriptName, id )		static const u32 cFlag##name = id;
#endif//log_flag_register_types

