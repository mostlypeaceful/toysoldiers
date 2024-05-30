//------------------------------------------------------------------------------
// \file tLoadObjectPanel.hpp - 19 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tLoadObjectPanel__
#define __tLoadObjectPanel__
#include "tWxToolsPanel.hpp"

namespace Sig
{
	class tSigmlBrowser;

	///
	/// \class tLoadObjectPanel
	/// \brief 
	class tLoadObjectPanel : public tWxToolsPanelTool
	{
	public:

		tLoadObjectPanel( tWxToolsPanel * parent );

		void fMarkForRefresh( );
		virtual void fOnTick( );

	private:
		
		tSigmlBrowser * mBrowser;
		b32 mRefreshed;
	};
}

#endif//__tLoadObjectPanel__
