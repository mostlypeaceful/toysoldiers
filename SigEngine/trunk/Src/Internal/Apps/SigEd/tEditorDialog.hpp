#ifndef __tEditorDialog__
#define __tEditorDialog__
#include "tWxSlapOnDialog.hpp"

namespace Sig
{
	class tEditorAppWindow;

	class siged_export tEditorDialog : public tWxSlapOnDialog
	{
		tEditorAppWindow*		mEditorWindow;

	public:
		tEditorAppWindow*		fEditorWindow( ) const { return mEditorWindow; }
		tEditorDialog( tEditorAppWindow* editorWindow, const char* regKeyName );
		void fOnTick( );
	};

}


#endif//__tEditorDialog__
