#ifndef __tShadeControlPanel__
#define __tShadeControlPanel__
#include "tShadeNodeCanvas.hpp"
#include "Editor/tEditableProperty.hpp"

namespace Sig
{
	class tEditablePropertyCustomString;

	class tShadeControlPanel : public wxScrolledWindow
	{
	public:
		typedef tDAGNodeCanvas::tDAGNodeList tDAGNodeList;
	private:
		tShadeNodeCanvas* mCanvas;
		wxStaticText* mHeaderText;
		wxScrolledWindow* mPropertyPanel;
		tEditablePropertyTable mCommonProps;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnPropertyChanged;
	public:
		tShadeControlPanel( wxWindow* parent, tShadeNodeCanvas* canvas );
		void fOnSelectionChanged( const tDAGNodeList& selected );
	private:
		void fOnPropertyChanged( tEditableProperty& property );
	};
}

#endif//__tShadeControlPanel__
