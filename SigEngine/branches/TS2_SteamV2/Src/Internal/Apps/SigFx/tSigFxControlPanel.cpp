#include "SigFxPch.hpp"
#include "tFileSystem.hpp"
#include "tSigFxControlPanel.hpp"
#include "tSigFxMainWindow.hpp"
#include "WxUtil.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxRenderPanel.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tSceneGraphFile.hpp"
#include "tAssetGenScanner.hpp"


namespace Sig
{
	tSigFxControlPanel::tSigFxControlPanel( tSigFxMainWindow* mainWindow )
		: wxScrolledWindow( mainWindow, wxID_ANY, wxDefaultPosition, wxSize( 320, wxDefaultSize.y ), wxBORDER_THEME | wxVSCROLL )
		, mMainWindow( mainWindow )
		, mRegKeyName( mainWindow->fGuiApp( ).fRegKeyName( ) + "\\ControlPanel" )
	{
		SetBackgroundColour( wxColour( 0xaa, 0xff, 0xaa ) );

		SetMinSize( wxSize( GetSize( ) ) );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		//SetScrollbars( 0, 20, 0, 0 );

		fBuildGui( );

		fUpdateWindowTitle( );
	}

	tSigFxControlPanel::~tSigFxControlPanel( )
	{
		fSave( );
	}

	void tSigFxControlPanel::fOnTick( f32 dt )
	{
	}

	void tSigFxControlPanel::fLoadSettings( )
	{
		fLoad( );
	}

	void tSigFxControlPanel::fResetSolidColorObjects( 
		const Gfx::tDevicePtr& device,
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
				
		
	}

	void tSigFxControlPanel::fSaveInternal( HKEY hKey )
	{
		// save settings to registry
	}
	
	void tSigFxControlPanel::fLoadInternal( HKEY hKey )
	{
		// load settings from registry
	}

	void tSigFxControlPanel::fBuildGui( )
	{
		// add wx widgets controls
	}

	void tSigFxControlPanel::fUpdateWindowTitle( )
	{
		std::stringstream ss;
		ss << "SigFx";

		// add extra stuff to title (file name, whatever)

		mMainWindow->SetTitle( ss.str( ) );
	}

}

