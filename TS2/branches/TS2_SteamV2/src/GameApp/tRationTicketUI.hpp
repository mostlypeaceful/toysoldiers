// Ration Ticket UI for Script
#ifndef __tRationTicketUI__
#define __tRationTicketUI__
#include "Gui/tScriptedControl.hpp"

namespace Sig { 

	class tUser;

namespace Gui
{
	class tRationTicketUI : public tScriptedControl
	{
	public:
		explicit tRationTicketUI( );
		~tRationTicketUI( ) { }

	public:
		void fRationTicketProgress( u32 index, f32 progress, f32 max, tUser* user );
		void fAwardRationTicket( u32 index, tUser* user );
		void fFailRationTicket( u32 index, tUser* user );
		void fAwardNewRank( u32 rankIndex, tUser* user );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr< tRationTicketUI > tRationTicketUIPtr;
} }

#endif //__tRationTicketUI__