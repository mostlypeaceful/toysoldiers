#ifndef __tSigEdMainWindow__
#define __tSigEdMainWindow__
#include "tEditorAppWindow.hpp"

namespace Sig
{
	///
	/// \brief Top level, parent frame window containing all sub windows for the application.
	/// Basically a simple container that delegates to its children.
	class tSigEdMainWindow : public tEditorAppWindow, public Win32Util::tRegistrySerializer
	{
		wxChoice* mPlatformComboBox;
	public:
		tSigEdMainWindow( tToolsGuiApp& guiApp );
		~tSigEdMainWindow( );
		virtual void fSetupRendering( );
		virtual void fOnTick( );

	private:
		void fAddMenus( );
		void fAddToolbar( );
		void fAddTools( );
		void fOnClose(wxCloseEvent& event);
		void fOnAction(wxCommandEvent& event);
		void fOnPlatformChanged(wxCommandEvent& event);
		virtual void fOnRightClick( wxWindow* window, wxMouseEvent& event );
		virtual std::string fRegistryKeyName( ) const;
		virtual void fSaveInternal( HKEY hKey );
		virtual void fLoadInternal( HKEY hKey );
		void fOnFilesDropped(wxDropFilesEvent& event);
		DECLARE_EVENT_TABLE()
	};

}

#endif//__tSigEdMainWindow__
