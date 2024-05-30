#ifndef __tSigAIControlPanel__
#define __tSigAIControlPanel__
#include "tSigAINodeCanvas.hpp"
#include "Editor/tEditableProperty.hpp"
#include "tProjectFile.hpp"
#include "tAINode.hpp"
#include "Momap.hpp"

namespace Sig
{
	class tEditablePropertyCustomString;

	class tSigAIControlPanel : public wxScrolledWindow, public tAINodeControlPanel
	{
	public:
		typedef tDAGNodeCanvas::tDAGNodeList tDAGNodeList;
		typedef tDAGNodeCanvas::tDAGNodeConnectionList tDAGNodeConnectionList;
		typedef tDAGNodeCanvas::tDAGNodeOutputList tDAGNodeOutputList;
	private:
		tSigAINodeCanvas*	mCanvas;
		wxStaticText*		mHeaderText;
		wxScrolledWindow*	mPropertyPanel;
		wxComboBox*			mEventCombo;
		wxListBox*			mGoalList;

		tEditablePropertyTable mCommonProps;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnPropertyChanged;

		tFilePathPtr mMoMapFileName;
		Momap::tFile mMoMapFile;

		tGrowableArray< tProjectFile::tEvent > mEventList;
		tGoalAINodePtr mSelectedNode;
		tDAGNodeConnectionPtr mSelectedConn;
		tGoalEventHandlerPtr mSelectedHandler;

		tEditablePropertyTable mConnectionProps;
		tEditablePropertyTable mEventHandlerProps;
	public:
		tSigAIControlPanel( wxWindow* parent, tSigAINodeCanvas* canvas );

		void fOnEventSelectionChanged( const tDAGNodeList& nodes, const tDAGNodeConnectionList& conns, const tDAGNodeOutputList& outputs );
		virtual void fRefreshProperties( );

		static const std::string cNoMoMapText;
		const tFilePathPtr& fMoMapPath( ) const { return mMoMapFileName; }
		const Momap::tFile& fMoMapFile( ) const { return mMoMapFile; }

		void fLoadMoMap( const tFilePathPtr& path );
		b32 fChooseMoState( tEditablePropertyCustomString& prop, std::string& newValue );
	private:
		void fFillEvents( );
		void fPopulateGoals( );
		void fResetEventCombo( );
		void fOnPropertyChanged( tEditableProperty& property );
		void fOnEventHandlerSelected( wxCommandEvent& event );
		void fOnInsertPriority( wxCommandEvent& );
		void fOnRemovePriority( wxCommandEvent& );
		void fShiftPriorities( s32 value, tGoalAINode* goal, u32 selectedPriority );
		void fAddAIFlagHandler( tGoalAINode* goal );
	};
}

#endif//__tSigAIControlPanel__
