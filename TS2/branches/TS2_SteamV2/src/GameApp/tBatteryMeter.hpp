/// \file   tBatteryMeter.hpp
/// \author Randall Knapp
/// \par    Email:\n rknapp\@signalstudios.net
/// \date   November 29, 2010 - 11:39
/// \par    Copyright:\n &copy; Signal Studios 2010-2011
/// \brief  Battery meter for vehicles
#ifndef __tBatteryMeter__
#define __tBatteryMeter__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"
#include "tVehicleLogic.hpp"

namespace Sig { namespace Gui
{
	class tBatteryMeter : public tScriptedControl
	{
	public:
		explicit tBatteryMeter( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tBatteryMeter( );

		void fSet( f32 percent );
		void fFadeIn( );
		void fFadeOut( );

		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }
		tVehicleLogic* fVehicle( ) const { return mVehicle.fGetRawPtr( ); }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr mUser;
		tRefCounterPtr< tVehicleLogic > mVehicle;
	};

	typedef tRefCounterPtr< tBatteryMeter > tBatteryMeterPtr;
}}

#endif //__tBatteryMeter__