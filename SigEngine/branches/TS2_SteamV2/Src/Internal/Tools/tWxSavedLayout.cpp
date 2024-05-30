#include "ToolsPch.hpp"
#include "tWxSavedLayout.hpp"

namespace Sig
{
	tWxSavedLayout::tWxSavedLayout( const std::string& regKeyName )
		: mRegKeyName( regKeyName )
		, mVisible( false )
		, mMinimized( false )
		, mMaximized( false )
		, mXPos(wxDefaultPosition.x)
		, mYPos(wxDefaultPosition.y)
		, mXSize(wxDefaultSize.x)
		, mYSize(wxDefaultSize.y)
	{
	}

	b32 tWxSavedLayout::operator==( const tWxSavedLayout& other ) const
	{
		return	mVisible == other.mVisible &&
				mMinimized == other.mMinimized &&
				mMaximized == other.mMaximized &&
				mXPos == other.mXPos &&
				mYPos == other.mYPos &&
				mXSize == other.mXSize &&
				mYSize == other.mYSize;
	}

	void tWxSavedLayout::fFromWxWindow( wxWindow* window )
	{
		const HWND thisHwnd = ( HWND )window->GetHWND( );
		mMaximized = IsZoomed( thisHwnd ) != 0;
		mMinimized = IsIconic( thisHwnd ) != 0;
		mVisible = (window->IsShown( )!=0) && !mMinimized;
		if( !mMinimized && !mMaximized )
		{
			mXPos = window->GetPosition( ).x;
			mYPos = window->GetPosition( ).y;
			mXSize = window->GetSize( ).x;
			mYSize = window->GetSize( ).y;
		}
	}

	void tWxSavedLayout::fToWxWindow( wxWindow* window )
	{
		if( !Win32Util::fIsVisibleOnConnectedMonitor( mXPos, mYPos ) )
			mXPos = mYPos = 2;
		window->SetPosition( wxPoint( mXPos, mYPos ) );
		window->SetSize( wxSize( mXSize, mYSize ) );
		window->Show( mVisible!=0 );
	}

	b32 tWxSavedLayout::fIsInBounds( s32 maxWinWidth )
	{
		if( !mMinimized &&
			mXPos != wxDefaultPosition.x && 
			mYPos != wxDefaultPosition.y && 
			fInBounds( mXSize, 1, maxWinWidth ) && 
			fInBounds( mYSize, 1, maxWinWidth ) )
		{
			return true;
		}

		return false;
	}

	void tWxSavedLayout::fSaveInternal( HKEY hKey )
	{
		Win32Util::fSetRegistryKeyValue( hKey, mVisible, fVisibleName( ) );
		Win32Util::fSetRegistryKeyValue( hKey, mMinimized, fMinimizedName( ) );
		Win32Util::fSetRegistryKeyValue( hKey, mMaximized, fMaximizedName( ) );
		Win32Util::fSetRegistryKeyValue( hKey, mXPos, fXPosName( ) );
		Win32Util::fSetRegistryKeyValue( hKey, mYPos, fYPosName( ) );
		Win32Util::fSetRegistryKeyValue( hKey, mXSize, fXSizeName( ) );
		Win32Util::fSetRegistryKeyValue( hKey, mYSize, fYSizeName( ) );
	}

	void tWxSavedLayout::fLoadInternal( HKEY hKey )
	{
		Win32Util::fGetRegistryKeyValue( hKey, reinterpret_cast<s32&>( mVisible ), fVisibleName( ) );
		Win32Util::fGetRegistryKeyValue( hKey, reinterpret_cast<s32&>( mMinimized ), fMinimizedName( ) );
		Win32Util::fGetRegistryKeyValue( hKey, reinterpret_cast<s32&>( mMaximized ), fMaximizedName( ) );
		Win32Util::fGetRegistryKeyValue( hKey, mXPos, fXPosName( ) );
		Win32Util::fGetRegistryKeyValue( hKey, mYPos, fYPosName( ) );
		Win32Util::fGetRegistryKeyValue( hKey, mXSize, fXSizeName( ) );
		Win32Util::fGetRegistryKeyValue( hKey, mYSize, fYSizeName( ) );
	}

}

