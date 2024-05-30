//------------------------------------------------------------------------------
// \file tNetUI.hpp - 31 May 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tNetUI__
#define __tNetUI__

#include "Gui/tScriptedControl.hpp"

namespace Sig { namespace Gui
{
	///
	/// \class tNetUI
	/// \brief 
	class tNetUI : public Gui::tScriptedControl
	{

	public:

		tNetUI( );
		~tNetUI( );

		void fShowLagUI( f32 dt );
		void fHideLagUI( );

		void fShowNetWaitUI( );
		void fHideNetWaitUI( );

		void fShowDesyncUI( );
		void fHideDesyncUI( );

	public:

		static void fExportScriptInterface( tScriptVm& vm );

	private:

		f32 mLagUITimer;
		b32 mLaggedUIActive;
		b32 mNetWaitUIActive;
		b32 mDesyncUIActive;

	};

	typedef tRefCounterPtr< tNetUI > tNetUIPtr;
} }

#endif//__tNetUI__
