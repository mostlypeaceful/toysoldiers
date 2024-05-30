#ifndef __tConfigurableBrowserTree__
#define __tConfigurableBrowserTree__
#include "tWxDirectoryBrowser.hpp"

namespace Sig
{
	class tToolsGuiMainWindow;

	typedef b32 (*tFilterFunction)( const tFilePathPtr& path );
	typedef tFilePathPtr (*tXmlPathToBinaryPath)( const tFilePathPtr& xmlPath );

	class toolsgui_export tMenuOption : public tRefCounter
	{
	protected:
		s32 mActionId;
	public:

		virtual ~tMenuOption( ) { }

		/// 
		/// \brief This should only be specified by the tConfigurableBrowserTree.
		void fSetActionId( s32 actionId ) { mActionId = actionId; }
		s32 fActionId( ) const { return mActionId; }

		virtual void tAddOption( const tWxDirectoryBrowser::tFileEntryData* entryData, wxTreeEvent& event, wxMenu& menu ) = 0;
		virtual void tExecuteOption( const tWxDirectoryBrowser::tFileEntryData* entryData, wxCommandEvent& event ) = 0;
	};
	define_smart_ptr( toolsgui_export, tRefCounterPtr, tMenuOption );

	/// 
	/// \brief
	/// A browser tree that can be set up to search for specific file types. Additional
	/// functionality should be added to allow better control over this browser.
	class toolsgui_export tConfigurableBrowserTree : public tWxDirectoryBrowser
	{
	private:
		tFilterFunction			mFilterFn;
		b16						mProcessActivation;
		b16						mShowRenameInRightClick;
		s32						mLastActionId;

		tGrowableArray< tMenuOptionPtr > mMenuOptions;

	public:
		// When passing in the filter function, remember that things that return negative from that function
		// will be removed from the browser. ProcessActivation means the tree will respond to double clicks and
		// other item activation events.
		tConfigurableBrowserTree( wxWindow* parent, tFilterFunction filter, u32 minHeight, b32 processActivation = false, b32 showRenameInRightClick = false );

		void fAddMenuOption( tMenuOptionPtr& newOption );

	protected:
		void fSimulateActivation( wxTreeEvent & event ) 
		{
			fOnDoubleClick( event );
		}

	private:
		virtual void fOpenDoc( const tFilePathPtr& file ) { }

		/// 
		/// \brief
		/// Default behavior at this level is to return blue text for actual files. The tree
		/// text is controlled elsewhere and will show as black.
		virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData )
		{
			return wxColour( 0x00, 0x00, 0xFF );
		}

		virtual b32 fFilterPath( const tFilePathPtr& path );

		virtual std::string fConvertDisplayName( const std::string& simplePath, u32& sortValue )
		{
			return StringUtil::fStripExtension( simplePath.c_str( ) );
		}

		virtual tFilePathPtr fXmlPathToBinaryPath( const tFilePathPtr& xmlPath )
		{
			return tFilePathPtr();
		}

		void fOnItemRightClick( wxTreeEvent& event );
		void fOnAction( wxCommandEvent& event );
		void fOnDoubleClick( wxTreeEvent& event ); // Responds to double click and other activation events.

		DECLARE_EVENT_TABLE()
	};
}

#endif
