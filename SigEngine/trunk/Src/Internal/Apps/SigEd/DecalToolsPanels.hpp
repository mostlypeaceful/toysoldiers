#ifndef __tPathDecalToolsPanel__
#define __tPathDecalToolsPanel__
#include "tWxToolsPanel.hpp"
#include "tEditorCursorControllerButton.hpp"

class wxStaticBitmap;

namespace Sig
{
	class tEditorAppWindow;
	class tTextureWrangler;
	class tWxSlapOnSpinner;

	class tPathDecalToolsPanel : public tWxToolsPanelTool
	{
		tEditorAppWindow* mAppWindow;
		tEditorHotKeyPtr mConnectHotKey;

		wxStaticBitmap* mDiffuseView;
		wxStaticBitmap* mNormalView;
		tTextureWrangler* mDiffuseTextureBox;
		tTextureWrangler* mNormalTextureBox;
		tWxSlapOnSpinner* mDecalWidth;

	public:
		tPathDecalToolsPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent );
		void fAddCursorHotKeys( tEditorButtonManagedCursorController* cursor );
		void fUpdateParametersOnCursor( tEditorButtonManagedCursorController* cursor );

		f32 fDefaultDecalWidth( ) const;
		std::string fDefaultDecalTexture( ) const;
		std::string fDefaultNormalTexture( ) const;

		void fOnBrowse( wxCommandEvent& evt );
		void fOnConfirm( wxCommandEvent& evt );
		void fRefreshTextureView( const char* path );
	};
}

#endif//__tPathDecalToolsPanel__
