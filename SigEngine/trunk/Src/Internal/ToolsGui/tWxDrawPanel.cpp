#include "ToolsGuiPch.hpp"
#include "tWxDrawPanel.hpp"

namespace Sig
{
	tWxDrawPanel::tWxDrawPanel( wxWindow* parent )
		: wxScrolledWindow( parent )
	{

	}

	tWxDrawPanel::~tWxDrawPanel( )
	{

	}

	//void tWxDrawPanel::fEventPaint( wxPaintEvent& event )
	//{
	//	//wxPaintDC dc(this);
	//	wxClientDC dc( this );
	//	dc.Clear( );
	//	fRender( dc );
	//}

	//void tWxDrawPanel::fRender( wxDC& dc )
	//{
	//	dc.DrawText( wxT( "Testing" ), 0, 0 ); 

	//	dc.SetBrush( *wxGREEN_BRUSH );
	//	dc.SetPen( wxPen( wxColor( 255, 0, 0), 5 ) );
	//	dc.DrawCircle( wxPoint( 0, 100 ), 25 );

	//	dc.SetBrush(*wxBLUE_BRUSH);
	//	dc.SetPen( wxPen( wxColor( 255, 175, 175 ), 10 ) );
	//	dc.DrawRectangle( 0, 100, 400, 200 );

	//	dc.SetPen( wxPen( wxColor( 0, 0, 0), 3 ) );
	//	dc.DrawLine( 0, 100, 700, 300 );
	//}

	//BEGIN_EVENT_TABLE( tWxDrawPanel, wxPanel )
	//	EVT_PAINT( tWxDrawPanel::fEventPaint )
	//END_EVENT_TABLE( )
}