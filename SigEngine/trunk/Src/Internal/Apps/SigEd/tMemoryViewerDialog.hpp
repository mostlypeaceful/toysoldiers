#ifndef __tMemoryViewerDialog__
#define __tMemoryViewerDialog__
#include "tEditorDialog.hpp"
#include "Editor\tEditableObjectContainer.hpp"
#include "wx\treectrl.h"


namespace Sig
{
	struct tMemoryRecordNode;

	class tMemoryViewerDialog : public tEditorDialog
	{
		wxScrolledWindow*		mMainPanel;
		wxTreeCtrl*				mData;
		wxCheckBox*				mVMode;
		wxCheckBox*				mMMode;

		tGrowableArray< tMemoryRecordNode* > mRecords;

	public:
		tMemoryViewerDialog( tEditorAppWindow* editorWindow );

		// Public and called from the tEditorAppWindow update because the selection
		// changed event gets fired once per object added which is not optimal.
		void fOnSelectionChanged( tEditorSelectionList & list );

	private:
		void fDrawcursively( tGrowableArray< tMemoryRecordNode* >& records, wxTreeItemId& root );
		void fOnModeChanged( wxCommandEvent& event );

		void fTrackExpanded( tGrowableArray<tFilePathPtr>& expandedKeys, wxTreeItemId node );
		void fReExpand( tGrowableArray<tFilePathPtr>& expandedKeys, wxTreeItemId node );
	};

}

#endif//__tMemoryViewerDialog__
