//------------------------------------------------------------------------------
// \file tLoadObjectPanel.cpp - 19 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigAnimPch.hpp"
#include "tLoadObjectPanel.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tEditorCursorController.hpp"
#include "Animation/tSkeletableSgFileRefEntity.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tConfigurableBrowserTree.hpp"
#include "tSearchableOpenFilesDialog.hpp"

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
			tResourceId rid = tResourceId::fMake<tSceneGraphFile>( tSceneGraphFile::fConvertToBinary( resRelFile ) );
			tResourcePtr sgFile = mMainWindow.fGuiApp( ).fResourceDepot( )->fQuery( rid );

			tSkeletableSgFileRefEntity * entity = 
				new tSkeletableSgFileRefEntity( mMainWindow.fGuiApp( ).fEditableObjects( ), rid, sgFile );

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

		// Add browser.
		mBrowser = new tSigmlBrowser( this, 300 );
		fGetMainPanel( )->GetSizer( )->Add( mBrowser, 1, wxEXPAND | wxALL, 5 );

		// Add search button.
		wxButton* findObject = new wxButton( fGetMainPanel(), wxID_ANY, "Find Object (Searchable Dialog)" );
		findObject->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tLoadObjectPanel::fOnFindClicked ), NULL, this );
		fGetMainPanel()->GetSizer()->Add( findObject, 0, wxEXPAND | wxALL, 5 );

		mFindDialog = new tSearchableOpenFilesDialog( fGetMainPanel(), ToolsPaths::fGetCurrentProjectResFolder(), wxLC_SINGLE_SEL );

		tGrowableArray<const char*> additionalFilters;
		additionalFilters.fPushBack( ".sigml" );
		mFindDialog->fSetAdditionalExtensionFilters( additionalFilters );
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

	void tLoadObjectPanel::fOnFindClicked( wxCommandEvent& evt )
	{
		tFilePathPtr file = mFindDialog->fGetSelectedFile();

		tFilePathPtr resRelFile = ToolsPaths::fMakeResRelative( file );
		tResourceId rid = tResourceId::fMake<tSceneGraphFile>( tSceneGraphFile::fConvertToBinary( resRelFile ) );
		tResourcePtr sgFile = fGuiApp().fResourceDepot()->fQuery( rid );

		tSkeletableSgFileRefEntity * entity = 
			new tSkeletableSgFileRefEntity( fGuiApp().fEditableObjects(), rid, sgFile );

		entity->fSetIsCursor( true );

		fGuiApp().fSetCurrentCursor(
			tEditorCursorControllerPtr( 
			new tLoadObjectCursor( fGuiApp().fMainWindow(), tEntityPtr(entity) ) ) );

		fGuiApp().fMainWindow().SetFocus();
	}
}