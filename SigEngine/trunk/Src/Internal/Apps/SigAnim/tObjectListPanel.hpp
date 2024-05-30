//------------------------------------------------------------------------------
// \file tObjectListPanel.hpp - 25 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tObjectListPanel__
#define __tObjectListPanel__

#include "tWxToolsPanel.hpp"

namespace Sig
{
	///
	/// \class tObjectListPanel
	/// \brief 
	class tObjectListPanel : public tWxToolsPanelTool
	{
	public:

		tObjectListPanel( tWxToolsPanel * parent );

		virtual void fOnTick( );

	private:

		class tObjectListBox * mListBox;
	};
}

#endif//__tObjectListPanel__
