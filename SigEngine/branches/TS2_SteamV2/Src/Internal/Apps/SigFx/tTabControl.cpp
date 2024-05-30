#include "SigFxPch.hpp"
#include "tTabControl.hpp"

namespace Sig
{

	tTabControl::tTabControl( wxWindow* parent, tToolsGuiMainWindow* toolswindow )
		: wxNotebook( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxCLIP_CHILDREN | wxNB_FIXEDWIDTH | wxNB_NOPAGETHEME | wxNB_BOTTOM )
		, mToolsWindow( toolswindow )
	{
		SetMinSize( wxSize( 0, 290 ) );
		SetBackgroundColour( wxColour( 145, 138, 127 ) );
		Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxCommandEventHandler( tTabControl::fOnPageChanged ) );
	}

	tTabControl::~tTabControl( )
	{

	}


	void tTabControl::fOnPageChanged( wxCommandEvent& event )
	{
		wxWindowList children = GetChildren( );
		for( u32 i = 0; i < children.GetCount( ); ++i )
		{
			tTabPanel *tab = static_cast< tTabPanel* >( children[ i ] );
			if( tab )
				tab->fBuildPageFromEntities( mToolsWindow->fGuiApp( ).fSelectionList( ) );
		}

		event.Skip( );
	}

	void tTabControl::fBuildPageFromEntities( tEditorSelectionList& selectedObjects )
	{
		wxWindowList children = GetChildren( );
		for( u32 i = 0; i < children.GetCount( ); ++i )
		{
			tTabPanel *tab = static_cast< tTabPanel* >( children[ i ] );
			if( tab )
				tab->fBuildPageFromEntities( selectedObjects );
		}
	}

	void tTabControl::fOnTickSelectedTab( f32 dt )
	{
		if( GetCurrentPage( ) )
		{
			tTabPanel *tab = static_cast< tTabPanel* >( GetCurrentPage( ) );
			tab->fOnTick( dt );
		}
	}
}

