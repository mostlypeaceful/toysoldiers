#include "ToolsPch.hpp"
#include "tWxSlapOnDialog.hpp"

namespace Sig
{
	const u32 tWxSlapOnDialog::cDefaultStyles = wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxRESIZE_BORDER | wxTAB_TRAVERSAL | wxWANTS_CHARS | wxFRAME_NO_TASKBAR;

	tWxSlapOnDialog::tWxSlapOnDialog( const char* title, wxWindow* parent, const std::string regKeyName )
		: wxFrame( parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, cDefaultStyles )
		, mRawParent( 0 )
		, mWasTopMost( false )
		, mRegKeyName( regKeyName )
	{
		Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( tWxSlapOnDialog::fOnClose ) );
	}

	std::string tWxSlapOnDialog::fRegistryKeyName( ) const
	{
		return mRegKeyName;
	}

	void tWxSlapOnDialog::fSetRawParent( HWND rawParent )
	{
		mRawParent = rawParent;
	}

	void tWxSlapOnDialog::fSave( )
	{
		tWxSavedLayout::fFromWxWindow( this );
		if( fIsInBounds( 2048 ) )
			tWxSavedLayout::fSave( );
	}

	void tWxSlapOnDialog::fLoad( )
	{
		if( tWxSavedLayout::fLoad( ) )
		{
			tWxSavedLayout::fToWxWindow( this );
			if( !fIsInBounds( 2048 ) )
			{
				SetPosition( wxPoint( 100, 100 ) );
				SetSize( wxSize( 300, 500 ) );
			}
		}
	}

	b32 tWxSlapOnDialog::fIsActive( ) const
	{
		if( !IsShown( ) )
			return false;

		const HWND thisHwnd = ( HWND )GetHWND( );

		RECT rect;
		GetWindowRect( thisHwnd, &rect );

		POINT cursor;
		GetCursorPos( &cursor );

		b32 isActive = fInBounds<s32>( cursor.x, rect.left, rect.right ) && fInBounds<s32>( cursor.y, rect.top, rect.bottom );

		if( !isActive )
		{
			// HACK not a fan of this, but it works (check if the user clicked the mouse outside the bounds of the dialog window)
			const b32 clickedOutside = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) || ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 );
			if( !clickedOutside )
			{
				const HWND focusHwnd = GetFocus( );
				isActive = ( focusHwnd != thisHwnd ) && IsChild( thisHwnd, focusHwnd );
			}
		}

		return isActive;
	}

	b32 tWxSlapOnDialog::fIsMinimized( ) const
	{
		const HWND thisHwnd = ( HWND )GetHWND( );

		return IsIconic( thisHwnd ) != 0;
	}

	void tWxSlapOnDialog::fRestoreFromMinimized( )
	{
		const HWND thisHwnd = ( HWND )GetHWND( );

		ShowWindow( thisHwnd, SW_RESTORE );
	}

	void tWxSlapOnDialog::fShow( )
	{
		if( fIsMinimized( ) )
			fRestoreFromMinimized( );
		else
			Show( true );
	}

	void tWxSlapOnDialog::fToggle( )
	{
		if( fIsMinimized( ) )
			fRestoreFromMinimized( );
		else
			Show( !IsShown( ) );
	}

	void tWxSlapOnDialog::fSetTopMost( b32 makeTopMost )
	{
		if( mWasTopMost != makeTopMost )
		{
			if( mRawParent )
			{
				const HWND thisHwnd = ( HWND )GetHWND( );
				if( makeTopMost )
					SetWindowPos( thisHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
				else
					SetWindowPos( thisHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE );
			}
			else
			{
				SetWindowStyle( ( makeTopMost ? wxSTAY_ON_TOP : 0 ) | cDefaultStyles );
			}

			mWasTopMost = makeTopMost;
		}
	}

	b32 tWxSlapOnDialog::fAutoHandleTopMost( HWND parent )
	{
		if( !parent && !mRawParent )
			return mWasTopMost;
		if( !parent )
			parent = mRawParent;

		// we have to manually handle updating the "topmost-ness" of the dialog,
		// as its not a true child window of the "parent"... so, if the parent or 
		// any of the windows in the parent process are currently on top of the window chain, 
		// then make our window topmost...

		const HWND thisHwnd = ( HWND )GetHWND( );
		const HWND foregroundWindow = GetForegroundWindow( );

		DWORD procIdOfForegroundWindow;
		GetWindowThreadProcessId( foregroundWindow, &procIdOfForegroundWindow );

		DWORD procIdOfParent;
		GetWindowThreadProcessId( parent, &procIdOfParent );

		const b32 focusIsInProcess = procIdOfParent == procIdOfForegroundWindow;
		const b32 parentIsMinimized = IsIconic( parent );
		const b32 setTopMost = focusIsInProcess && !parentIsMinimized;

		fSetTopMost( setTopMost );

		return mWasTopMost;
	}

	void tWxSlapOnDialog::fOnClose( wxCloseEvent& event )
	{
		if( event.CanVeto( ) )
		{
			Show( false );
			event.Veto( );
		}
		else
			wxFrame::Destroy( );
	}

}
