#ifndef __tObjectBrowserDialog__
#define __tObjectBrowserDialog__
#include "tEditorDialog.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tEditableSgFileRefEntity;
	class tWxColumnListBox;
	struct tListRowData;
	class tEditableObject;

	class tObjectBrowserDialog : public tEditorDialog
	{
		wxScrolledWindow*		mMainPanel;

		// Term to search
		wxTextCtrl*				mSearchBox;

		// Type filters
		wxCheckBox*				mMshmlBox;
		wxCheckBox*				mSigmlBox;
		wxCheckBox*				mFxmlBox;
		wxCheckBox*				mTerrainBox;
		wxCheckBox*				mLightBox;
		wxCheckBox*				mLightProbeBox;
		wxCheckBox*				mAttachBox;
		wxCheckBox*				mShapeBox;
		wxCheckBox*				mWaypointBox;
		wxCheckBox*				mDecalsBox;
		wxCheckBox*				mNavBox;
		wxCheckBox*				mCameraBox;
		wxCheckBox*				mTileCanvasBox;
		wxCheckBox*				mTilePropsBox;
		wxCheckBox*				mGroupsBox;

		// Field filters
		wxCheckBox*				mNameBox;
		wxCheckBox*				mFilePathBox;
		wxCheckBox*				mScriptBox;
		wxCheckBox*				mGamePropsBox;

		// Results
		tWxColumnListBox*		mResultsBox;

		wxCheckBox*				mFindDuplicatesBox;
		wxCheckBox*				mSearchSelection;
		wxCheckBox*				mSearchHiddensBox;
		wxCheckBox*				mFrameSelectedBox;

	public:
		tObjectBrowserDialog( tEditorAppWindow* editorWindow );
	    virtual bool Show( bool show = true );

		void fClear( );

	private:
		void fBuildGui( );
		void fOnSelectPressed( wxCommandEvent& event );
		void fSelectItem( );
		void fOnSearchPressed( wxCommandEvent& event );
		void fOnListBoxDClick( wxMouseEvent& event );

		template< class T >
		b32 fProcessType( tGrowableArray< tListRowData >& rowPackages, tEditableObject* editObjPtr, b32 skip, b32 fptOn, b32 namOn, b32 scpOn, b32 prpOn, const wxString filePathOrType );

		b32 fFilterFields( const tListRowData& rowData, b32 testFilePathName, b32 testRefName, b32 testScriptName, b32 testGameProps );
		void fRefreshList( );
	};

}

#endif//__tObjectBrowserDialog__
