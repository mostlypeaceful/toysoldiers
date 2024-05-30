#ifndef __tSigAnimNodeCanvas__
#define __tSigAnimNodeCanvas__
#include "tDAGNodeCanvas.hpp"
#include "Momap.hpp"

namespace Sig
{
	class tCreateNodeContextAction;
	class tSigAnimDialog;

	class tSigAnimNodeCanvas : public tDAGNodeCanvas
	{
	private:
		tCreateNodeContextAction* mNodeCreator;
		Momap::tFile mClipboard;

		tEditorContextActionList mActionList;
		tSigAnimDialog* mMainWindow;

	public:
		explicit tSigAnimNodeCanvas( wxWindow* parent, tSigAnimDialog* mainWindow );

		void fAddDefaultNode( const wxPoint& p = wxPoint( -1, -1 ) );

		void fCopy( );
		void fPaste( );

		void fToFile( Momap::tMoState& file, b32 selectedOnly = false );
		void fFromFile( const Momap::tMoState& file, b32 addToScene = false );

		virtual void fOnMouseRightButtonUp( wxMouseEvent& event );
		virtual void fConnectionCreated( const tDAGNodeConnectionPtr& connection );
	};
}

#endif//__tSigAnimNodeCanvas__
