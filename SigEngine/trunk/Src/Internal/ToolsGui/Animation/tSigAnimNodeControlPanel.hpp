#ifndef __tSigAnimNodeControlPanel__
#define __tSigAnimNodeControlPanel__

#include "tDAGNodeCanvas.hpp"
#include "DerivedAnimNodes.hpp"
#include "Editor/tEditableProperty.hpp"
#include "tProjectFile.hpp"

namespace Sig
{

	class tSigAnimNodeCanvas;
	class tSigAnimSlidersPanel;
	class tSigAnimDialog;

	class toolsgui_export tSigAnimNodeControlPanel : public wxScrolledWindow
	{
	public:
		typedef tDAGNodeCanvas::tDAGNodeList tDAGNodeList;
		typedef tDAGNodeCanvas::tDAGNodeConnectionList tDAGNodeConnectionList;
		typedef tDAGNodeCanvas::tDAGNodeOutputList tDAGNodeOutputList;

	private:
		tSigAnimDialog* mMainWindow;
		tSigAnimNodeCanvas*	mCanvas;
		wxStaticText*		mHeaderText;
		wxScrolledWindow*	mPropertyPanel;
		tSigAnimSlidersPanel* mSliders;

		tEditablePropertyTable mCommonProps;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnPropertyChanged;

		tGrowableArray< tProjectFile::tEvent > mEventList;
		tAnimBaseNodePtr mSelectedNode;
		tDAGNodeConnectionPtr mSelectedConn;

	public:
		tSigAnimNodeControlPanel( tSigAnimDialog* parent, tSigAnimNodeCanvas* canvas );

		void fOnCanvasSelectionChanged( const tDAGNodeList& nodes, const tDAGNodeConnectionList& conns, const tDAGNodeOutputList& outputs );
		virtual void fRefreshProperties( );

		tSigAnimSlidersPanel* fSliders( ) { return mSliders; }
	
	private:
		void fOnPropertyChanged( tEditableProperty& property );
	};

	class tSigAnimKnobBase;
	namespace Anim { class tSigAnimMoMap; }
	class tSkeletableSgFileRefEntity;

	class toolsgui_export tSigAnimSlidersPanel : public wxScrolledWindow
	{
	public:
		tSigAnimSlidersPanel( wxWindow& parent, tSigAnimDialog* mainWindow );
		~tSigAnimSlidersPanel( );

		void fClear( );
		void fBuildKnobs( tSigAnimNodeCanvas& canvas );
		void fOnResetClicked( wxCommandEvent& );

		tGrowableArray<tSigAnimKnobBase*> mKnobs;
		tGrowableArray<tSigAnimKnobBase*> mContexts;

		tRefCounterPtr<Anim::tSigAnimMoMap> mMomap;
		tSigAnimDialog* mMainWindow;
	};


}

#endif//__tSigAnimNodeControlPanel__
