#include "ToolsPch.hpp"
#include "tWxProgressDialog.hpp"
#include "tWin32Window.hpp"

namespace Sig
{
	tWxProgressDialog::tWxProgressDialog( wxWindow* parent, const char* title, b32 pumpMessages )
		: wxProgressDialog( title, "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_SMOOTH )
		, mProgress( 0.f )
		, mPump( pumpMessages )
	{
		Fit( );
		Center( );
	}

	tWxProgressDialog::~tWxProgressDialog( )
	{
		Time::tStopWatch timer;
		while( timer.fGetElapsedMs( ) < 500.f )
		{
			wxProgressDialog::Update( 100 );
			wxProgressDialog::Show( );
			fPump( );
		}
	}

	void tWxProgressDialog::fUpdate( f32 progressZeroToOne, const char* message )
	{
		sigassert( fInBounds( progressZeroToOne, 0.f, 1.f ) );
		mProgress = fMax( mProgress, progressZeroToOne );
		Update( fRound<u32>( mProgress * 99 ), message );
		fPump( );
	}

	void tWxProgressDialog::fPump( )
	{
		if( mPump )
			tWin32Window::fMessagePump( );
		else
		{
			Refresh( );
			Update( );
		}
	}

}

