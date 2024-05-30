
#ifdef define_logic_event
#	undef define_logic_event
#endif//define_logic_event

#ifdef logic_event_script_export
#	define define_logic_event( cppName, scriptName, id )		vm.fConstTable( ).Const( scriptName, ( int )( id ) );
#else//logic_event_script_export
#	define define_logic_event( cppName, scriptName, id )		static const u32 cppName = id;
#endif//logic_event_script_export

