//------------------------------------------------------------------------------
// \file tLoadObjectPanel.cpp - 19 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigAnimPch.hpp"
#include "tLoadObjectPanel.hpp"
#include "tWxSlapOnRadioBitmapButton.hpp"
#include "WxUtil.hpp"
#include "tBrowseControl.hpp"
#include "tStrongPtr.hpp"
#include "FileSystem.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tEditorCursorController.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "tSkeletableSgFileRefEntity.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tSkeletonFile.hpp"
#include "tSigAnimMainWindow.hpp"
#include "tConfigurableBrowserTree.hpp"

namespace Sig
{
	
	///
	/// \class tLoadObjectCursor
	/// \brief 
	class tLoadObjectCursor : public tPlaceObjectCursor
	{
	public:

		tLoadObjectCursor( tToolsGuiMainWindow & mainWindow, const tEntityPtr & entity)
			: tPlaceObjectCursor( mainWindow, entity )
		{
			mainWindow.fSetStatus( "Place Object" );
		}
	};
	
	///
	/// \class tSigmlBrowser
	/// \brief 
	class tSigmlBrowser : public tConfigurableBrowserTree
	{
	public:

		tSigmlBrowser( tWxToolsPanelTool * tool, u32 minHeight )
			: tConfigurableBrowserTree( tool->fGetMainPanel( ), Sigml::fIsSigmlFile, minHeight, true )
			, mMainWindow( tool->fGuiApp( ).fMainWindow( ) )
			, mRefreshing( false )
		{

		}

		virtual void fOpenDoc( const tFilePathPtr& file ) 
		{
			tFilePathPtr resRelFile = ToolsPaths::fMakeResRelative( file );
			tResourcePtr sgFile = mMainWindow.fGuiApp( ).fResourceDepot( )->fQuery( 
				tResourceId::fMake<tSceneGraphFile>( 
					tResourceConvertPath<tSceneGraphFile>::fConvertToBinary( 
						 resRelFile) ) );

			tSkeletableSgFileRefEntity * entity = 
				new tSkeletableSgFileRefEntity( mMainWindow.fGuiApp( ).fEditableObjects( ), sgFile );

			entity->fSetIsCursor( true );

			mMainWindow.fGuiApp( ).fSetCurrentCursor(
				tEditorCursorControllerPtr( 
					new tLoadObjectCursor( mMainWindow, tEntityPtr( entity ) ) ) );
		}

		virtual void fOnSelChanged( wxTreeEvent& event, const tFileEntryData& fileEntryData )
		{
			if( !mRefreshing )
				fSimulateActivation( event );
		}

		void fRefresh( )
		{
			mRefreshing = true;
			tConfigurableBrowserTree::fRefresh( );
			mRefreshing = false;
		}

	private:

		tToolsGuiMainWindow & mMainWindow;
		b32 mRefreshing;

	};

	//------------------------------------------------------------------------------
	// tLoadObjectPanel
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tLoadObjectPanel::tLoadObjectPanel( tWxToolsPanel * parent )
		: tWxToolsPanelTool( parent, "New Object Browser", "Load entities or mesh/skeleton pairs", "LoadObj" )
		, mRefreshed( false )
	{
		fGetMainPanel( )->SetForegroundColour( wxColor( 0xff, 0xff, 0xff ) );
		mBrowser = new tSigmlBrowser( this, 300 );
		fGetMainPanel( )->GetSizer( )->Add( mBrowser, 1, wxEXPAND | wxALL, 5 );
	}

	//------------------------------------------------------------------------------
	void tLoadObjectPanel::fMarkForRefresh( )
	{
		mRefreshed = false;
	}

	//------------------------------------------------------------------------------
	void tLoadObjectPanel::fOnTick( )
	{
		if( !mRefreshed )
		{
			mRefreshed = true;
			mBrowser->fRefresh( );
		}
	}
}