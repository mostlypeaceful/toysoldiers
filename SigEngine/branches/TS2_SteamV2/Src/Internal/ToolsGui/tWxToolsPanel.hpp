#ifndef __tWxToolsPanel__
#define __tWxToolsPanel__
#include "tWxSlapOnGroup.hpp"

namespace Sig
{
	class tToolsGuiApp;
	class tWxToolsPanelTool;
	class tWxToolsPanel;
	class tWxToolsPanelContainer;

	///
	/// \brief Provides base functionality for a "tool". A "tool" is basically
	/// a relatively small, collapsible window which can be toggled on and off
	/// for display. Generally, the derived types provide a smattering of useful 
	/// buttons and other controls for interacting with the application in some
	/// way; these controls live on the tWxToolsPanelTool object.
	class toolsgui_export tWxToolsPanelTool : public tWxSlapOnGroup, public Win32Util::tRegistrySerializer
	{
	protected:
		tWxToolsPanel* mParent; 
		std::string mRegKeyName;
		wxString mIconName;
		wxString mIconToolTip;
		wxString mIconBitmapResource;
		u32 mActionId;
		b32 mShowByDefault;
	public:
		tWxToolsPanelTool( 
			tWxToolsPanel* parent, 
			const char* label, 
			const wxString& iconToolTip = "", 
			const wxString& iconBitmapResource = "" );
		inline tWxToolsPanel* fParent( ) const { return mParent; } 
		inline b32 fGetShowByDefault( ) const { return mShowByDefault; }
		void fShow( wxToolBar* toolBar, b32 show );
		virtual void fAddToToolBar( wxToolBar* toolBar, u32& currentActionId );
		virtual b32 fHandleAction( u32 actionId );
		virtual void fOnTick( ) { }

		tToolsGuiApp& fGuiApp( );
		const tToolsGuiApp& fGuiApp( ) const;

	protected:
		virtual std::string fRegistryKeyName( ) const;
		virtual void fSaveInternal( HKEY hKey );
		virtual void fLoadInternal( HKEY hKey );
	};

	///
	/// \brief A basic window panel which contains a dynamic list of "tools" (tWxToolsPanelTool).
	/// Basically this is just a container/group wrapper around these objects.
	class toolsgui_export tWxToolsPanel : public wxScrolledWindow
	{
		friend class tWxToolsPanelTool;
	protected:
		tWxToolsPanelContainer* mContainer;
		tGrowableArray< tWxToolsPanelTool* > mTools;
		u32 mNumToolsOn;
	public:
		tWxToolsPanel( tWxToolsPanelContainer* parent, u32 width = 280, wxColour bkgndColor = wxColour(0xbb,0xbb,0xbb), wxColour frgndColor = wxColour( 0x00, 0x00, 0x00 ) );
		virtual void fAddToToolBar( wxToolBar* toolBar, u32& currentActionId );
		virtual b32 fHandleAction( u32 actionId );
		virtual void fOnTick( );
		tToolsGuiApp& fGuiApp( );
		void fUpdateScrollBars( );
	private:
		u32 fComputeScrollHeight( );
		void fCheckForCursorFocus( );
	};

	class toolsgui_export tWxUnifiedToolsPanel : public tWxToolsPanel
	{
		wxString mIconName;
		wxString mIconToolTip;
		wxString mIconBitmapResource;
		u32 mActionId;
	public:
		tWxUnifiedToolsPanel( tWxToolsPanelContainer* parent, 
			const wxString& iconName, 
			const wxString& iconToolTip, 
			const wxString& iconBitmapResource, 
			u32 width = 280, wxColour bkgndColor = wxColour(0xbb,0xbb,0xbb) );
		virtual void fAddToToolBar( wxToolBar* toolBar, u32& currentActionId );
		virtual b32 fHandleAction( u32 actionId );
	};

}

#endif//__tWxToolsPanel__

