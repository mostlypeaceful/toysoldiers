//------------------------------------------------------------------------------
// \file tEntityControlPanel.hpp - 26 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEntityControlPanel__
#define __tEntityControlPanel__
#include "tWxToolsPanel.hpp"
#include "Editor/tEditorSelectionList.hpp"

class wxCheckBox;
class wxButton;
class wxCommandEvent;

namespace Sig
{
	class tWxSlapOnSpinner;

	class tEntityControlPanel : public tWxToolsPanelTool
	{
	public:

		tEntityControlPanel( tWxToolsPanel * parent );

		void fSetPaused( b32 paused );
		b32 fPaused( ) const;
		f32 fStepSize( ) const;

	private:

		void fStepBack( wxCommandEvent & );
		void fStepForward( wxCommandEvent & );
		void fPlayPause( wxCommandEvent & );
		void fStep( f32 time );

		void fClearTracks( wxCommandEvent & );
		void fOnSelectionChanged( tEditorSelectionList & list );

		void fOnRenderSkeleton( wxCommandEvent & );
		void fOnApplyRefFrame( wxCommandEvent & );
		void fOnReversePlay( wxCommandEvent & );

	private:

		wxCheckBox * mRenderSkeleton;
		wxCheckBox * mApplyRefFrame;
		wxCheckBox * mReversePlay;
		wxCheckBox * mPartialsMatch;
		
		tWxSlapOnSpinner* mTimeScale;
		tWxSlapOnSpinner* mMotionRange;
		tWxSlapOnSpinner* mForceStep;

		wxButton * mPlayPauseButton;

		tEditorSelectionList::tOnSelectionChanged::tObserver mOnSelectionChanged;
	};
}


#endif//__tAnimationControlPanel__
