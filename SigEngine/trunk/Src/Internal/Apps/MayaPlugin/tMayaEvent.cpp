#include "MayaPluginPch.hpp"
#include "tMayaEvent.hpp"

namespace Sig
{

	void tMayaEvent::fLogEventNames( )
	{
		log_line( 0, "BEGIN Maya Event Names-----------------------" );

		MStringArray names;
		MEventMessage::getEventNames( names );
		for( u32 i = 0; i < names.length( ); ++i )
		{
			log_line( 0, "\t" << names[i].asChar( ) );
		}

		log_line( 0, "END Maya Event Names-----------------------" );
	}

}
