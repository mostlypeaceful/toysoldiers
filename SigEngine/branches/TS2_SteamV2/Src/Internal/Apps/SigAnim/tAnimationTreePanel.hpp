//------------------------------------------------------------------------------
// \file tAnimationTreePanel.hpp - 24 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tAnimationTreePanel__
#define __tAnimationTreePanel__
#include "tWxToolsPanel.hpp"
#include "tAnimPackData.hpp"

namespace Sig
{
	class tAnimPackTree;
	class tSigAnimEdDialog;
	class tSigAnimTimeline;

	///
	/// \class tAnimationTreePanel
	/// \brief 
	class tAnimationTreePanel : public tWxToolsPanelTool
	{
	public:

		tAnimationTreePanel( tWxToolsPanel * parent );
		
		const tAnimPackList & fAnimPackList( );

		void fMarkForRefresh( );
		void fToggelAnimEd( );
		void fWarnAndSaveAnyDirtyAnimPacks( );

		void fHandleDialogs( );

		virtual void fOnTick( );
		
		void fSetAnimationTimeline( tSigAnimTimeline* timeline );

	private:

		void fOnToggleAnimEdPressed( wxCommandEvent & );
		void fOnPartialsMatch( wxCommandEvent & );

	private:

		bool mRefresh;
		tAnimPackTree *		mAnimPackTree;
		tSigAnimTimeline*	mAnimTimeline;
	};
}

#endif//__tAnimationTreePanel__
