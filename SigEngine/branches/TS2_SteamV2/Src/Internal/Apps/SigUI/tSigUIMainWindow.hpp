#ifndef __tSigUIMainWindow__
#define __tSigUIMainWindow__
#include "tToolsGuiMainWindow.hpp"

namespace Sig
{
	class tWxToolsPanelContainer;

	///
	/// \brief Top level, parent frame window containing all sub windows for the application.
	/// Basically a simple container that delegates to its children.
	class tSigUIMainWindow : public tToolsGuiMainWindow, public Win32Util::tRegistrySerializer
	{
		tPlatformId mPreviewPlatform;
		wxChoice* mPlatformComboBox;
		wxBoxSizer* mMainSizer;
		tWxToolsPanelContainer* mToolsPanelContainer;
	public:
		tSigUIMainWindow( tToolsGuiApp& guiApp );
		~tSigUIMainWindow( );
		virtual void fSetupRendering( );
		virtual void fOnTick( );

	private:
		void fAddMenus( );
		void fAddToolbar( );
		void fAddTools( );
		void fNewDoc( );
		void fOnClose(wxCloseEvent& event);
		void fOnAction(wxCommandEvent& event);
		void fOnPlatformChanged(wxCommandEvent& event);
		virtual std::string fRegistryKeyName( ) const;
		virtual void fSaveInternal( HKEY hKey );
		virtual void fLoadInternal( HKEY hKey );
		DECLARE_EVENT_TABLE()
	};

}

#endif//__tSigUIMainWindow__
