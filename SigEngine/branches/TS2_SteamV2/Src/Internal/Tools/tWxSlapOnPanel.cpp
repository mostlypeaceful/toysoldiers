#include "ToolsPch.hpp"
#include "tWxSlapOnPanel.hpp"

namespace Sig
{
	tWxSlapOnPanel::tWxSlapOnPanel( wxWindow* parent, const char* label )
		: wxScrolledWindow( parent, wxID_ANY )
		, mLabel( label )
		, mTabOwner( 0 )
	{
		fCommonCtor( );
	}

	tWxSlapOnPanel::tWxSlapOnPanel( wxNotebook* parent, const char* label )
		: wxScrolledWindow( parent, wxID_ANY )
		, mLabel( label )
		, mTabOwner( parent )
	{
		parent->AddPage( this, label );
		fCommonCtor( );
	}

	void tWxSlapOnPanel::fShow( )
	{
		if( !mTabOwner )
		{
			Show( true );
			return;
		}
		const s32 tabIndex = fGetTabPageIndex( );
		if( tabIndex < 0 )
		{
			mTabOwner->AddPage( this, mLabel, true );
			Show( true );
		}
	}

	void tWxSlapOnPanel::fHide( )
	{
		if( !mTabOwner )
		{
			Show( false );
			return;
		}
		const s32 tabIndex = fGetTabPageIndex( );
		if( tabIndex >= 0 )
		{
			Show( false );
			mTabOwner->RemovePage( tabIndex );
		}
	}

	s32 tWxSlapOnPanel::fGetTabPageIndex( )
	{
		if( !mTabOwner )
			return -1;
		for( u32 i = 0; i < mTabOwner->GetPageCount( ); ++i )
			if( mTabOwner->GetPage( i ) == this )
				return i;
		return -1;
	}

	void tWxSlapOnPanel::fCommonCtor( )
	{
		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		EnableScrolling( true, true );
		SetScrollbars( 20, 20, 50, 50 );
		SetBackgroundColour( GetParent( )->GetBackgroundColour( ) );
		SetForegroundColour( GetParent( )->GetForegroundColour( ) );
	}
}



// for a rainy day perhaps... although i can't tell any difference with it
//#include "wx/dcbuffer.h"
//namespace Sig
//{
//	class tWxDoubleBufferedPanel : public wxPanel
//	{
//	public:
//		tWxDoubleBufferedPanel( wxWindow* parent )
//			: wxPanel( parent )
//		{
//			fSetupDoubleBuffering( );
//		}
//		tWxDoubleBufferedPanel( wxWindow* parent, u32 styles, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize )
//			: wxPanel( parent, wxID_ANY, pos, size, styles )
//		{
//			fSetupDoubleBuffering( );
//		}
//	private:
//		void fSetupDoubleBuffering( )
//		{
//			SetBackgroundStyle( wxBG_STYLE_CUSTOM );
//			Connect( wxEVT_PAINT, wxPaintEventHandler( tWxDoubleBufferedPanel::fOnPaint ), NULL, this );
//		}
//		void fOnPaint( wxPaintEvent& event )
//		{
//			wxAutoBufferedPaintDC dc( this );
//		}
//	};
//}

