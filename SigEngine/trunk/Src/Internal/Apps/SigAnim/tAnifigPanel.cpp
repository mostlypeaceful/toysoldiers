#include "SigAnimPch.hpp"
#include "tAnifigPanel.hpp"
#include "tSceneGraphFile.hpp"

#include "tConfigurableBrowserTree.hpp"

#include "FileSystem.hpp"
#include "tSigAnimMainWindow.hpp"
#include "Anifig.hpp"

namespace Sig
{
	class tAnifigBrowser : public tConfigurableBrowserTree
	{
		tSigAnimMainWindow & mMainWindow;

	public:
		tAnifigBrowser(  wxWindow* parent, tSigAnimMainWindow & mainWindow, u32 minHeight )
			: tConfigurableBrowserTree( parent, Anifig::fIsAnifigFile, minHeight )
			, mMainWindow( mainWindow )
		{ }

	private:
		virtual void fOpenDoc( const tFilePathPtr& file )
		{
			mMainWindow.fOpenDoc( file );
		}
	};

	tAnifigPanel::tAnifigPanel( tWxToolsPanel * parent, tSigAnimMainWindow & mainWindow )
		: tWxToolsPanelTool( parent, "Anifig Browser", "Browse for anifig files", "Anifig" )
		, mBrowser( 0 )
		, mRefreshed( false )
	{
		// Panel adds itself to its parent's sizer for now.
		mBrowser = new tAnifigBrowser( fGetMainPanel( ), mainWindow, 200 );
		fGetMainPanel( )->GetSizer()->Add( mBrowser, 1, wxEXPAND | wxALL, 5 );
	}

	void tAnifigPanel::fOnTick( )
	{
		if( !mRefreshed )
		{
			mRefreshed = true;
			mBrowser->fRefresh( );
		}
	}
}
