#ifndef __tPowerPoolUI__
#define __tPowerPoolUI__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { 

	struct tComboStatGroup;
	
namespace Gui
{

	class tPowerPoolUI : public tScriptedControl
	{
	public:
		explicit tPowerPoolUI( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tPowerPoolUI( );

		void fStep( f32 powerPoolPercent, f32 timerPercent, f32 dt );
		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }
		void fSetOverChargeActive( b32 active );
		void fShow( b32 show );
		void fShowBarrage( b32 show );
		void fSetCombo( u32 combo );

		b32 fIsShown( ) const { return mIsShown; }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr mUser;
		b32 mIsShown;
	};

	typedef tRefCounterPtr< tPowerPoolUI > tPowerPoolUIPtr;

}}

#endif//__tPowerPoolUI__
