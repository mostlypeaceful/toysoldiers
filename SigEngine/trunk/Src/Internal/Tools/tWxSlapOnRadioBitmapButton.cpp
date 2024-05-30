#include "ToolsPch.hpp"
#include "tWxSlapOnRadioBitmapButton.hpp"

#include "wx/tooltip.h"


namespace Sig
{

	tWxSlapOnRadioBitmapButtonGroup::tWxSlapOnRadioBitmapButtonGroup( wxWindow* parent, const char* label, b32 collapsible, u32 maxButtonsPerRow )
		: tWxSlapOnGroup( parent, label, collapsible )
		, mMasterContainer( 0 )
		, mButtonContainer( 0 )
		, mMaxButtonsPerRow( fMax( 1u, maxButtonsPerRow ) )
		, mSelected( -1 )
	{
		mMasterContainer = new wxBoxSizer( wxVERTICAL );
		fGetMainPanel( )->GetSizer( )->Add( mMasterContainer, 1, wxALIGN_CENTER, 1 );
	}
	void tWxSlapOnRadioBitmapButtonGroup::fSetSelected( u32 ithButton, b32 callOnSelChanged )
	{
		if( ithButton == mSelected )
		{
			if ((u32)mSelected < mButtons.fCount( )  && !mButtons[mSelected]->fReselectable( ) )
				return;
		}

		if( callOnSelChanged )
			fGetMainPanel( )->Freeze( );

		fClearSelection( ithButton );
		if( ithButton < mButtons.fCount( ) )
		{
			mButtons[ ithButton ]->fUpdateAllBitmaps( mButtons[ ithButton ]->mSelected );
			mButtons[ ithButton ]->fOnSelected( );
			mSelected = ithButton;
		}

		// notify
		if( callOnSelChanged )
		{
			fOnSelChanged( );
			fGetMainPanel( )->Thaw( );
		}
	}
	void tWxSlapOnRadioBitmapButtonGroup::fSetSelected( tWxSlapOnRadioBitmapButton* button, b32 callOnSelChanged )
	{
		tWxSlapOnRadioBitmapButton** find = mButtons.fFind( button );
		if( find )
			fSetSelected( fPtrDiff( find, mButtons.fBegin( ) ), callOnSelChanged );
	}
	void tWxSlapOnRadioBitmapButtonGroup::fClearSelection( s32 ignoreThisIndex )
	{
		for( u32 i = 0; i < mButtons.fCount( ); ++i )
		{
			if( i == ignoreThisIndex ) continue;
			mButtons[ i ]->fUpdateAllBitmaps( mButtons[ i ]->mDeSelected );
			mButtons[ i ]->fOnDeselected( );
		}
		mSelected = -1;
	}
	void tWxSlapOnRadioBitmapButtonGroup::fDeleteButtons( )
	{
		for( u32 i = 0; i < mButtons.fCount( ); ++i )
			delete mButtons[ i ];
		mButtons.fSetCount( 0 );

		fGetMainPanel( )->GetSizer( )->Remove( mMasterContainer );
		mMasterContainer = new wxBoxSizer( wxVERTICAL );
		fGetMainPanel( )->GetSizer( )->Add( mMasterContainer, 1, wxALIGN_CENTER, 1 );
	}

	tWxSlapOnRadioBitmapButton::tWxSlapOnRadioBitmapButton( 
		tWxSlapOnRadioBitmapButtonGroup* parent, 
		const wxBitmap& selected, 
		const wxBitmap& deSelected,
		const char* toolTip )
		: wxBitmapButton( parent->fGetMainPanel( ), wxID_ANY, deSelected, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW )
		, mParent( parent )
		, mSelected( selected )
		, mDeSelected( deSelected )
	{
		SetBitmapHover( selected );
		parent->mButtons.fPushBack( this );

		if( parent->mButtons.fCount( ) % parent->mMaxButtonsPerRow == 1 )
		{
			parent->mButtonContainer = new wxBoxSizer( wxHORIZONTAL );
			parent->mMasterContainer->Add( parent->mButtonContainer, 1, wxALIGN_CENTER, 1 );
		}

		parent->mButtonContainer->Add( this, 0, wxALIGN_CENTER, 1 );
		Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tWxSlapOnRadioBitmapButton::fOnPressed ), NULL, this );

		if( toolTip )
		{
			SetToolTip( toolTip );
			Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( tWxSlapOnRadioBitmapButton::fOnHover ), NULL, this );
		}
	}

	u32 tWxSlapOnRadioBitmapButton::fButtonIndex( ) const
	{
		tWxSlapOnRadioBitmapButton* const* find = mParent->mButtons.fFind( this );
		if( find )
			return fPtrDiff( find, mParent->mButtons.fBegin( ) );
		return ~0;
	}

	void tWxSlapOnRadioBitmapButton::fOnPressed( wxCommandEvent& event )
	{
		mParent->fSetSelected( this );
	}

	void tWxSlapOnRadioBitmapButton::fOnHover( wxMouseEvent& event )
	{
		// For some reason wxWidgets tool tips are failing under certain circumstances;
		// I'm resorting to this because I'm tired of dealing with it.
		if( GetToolTip( ) )
			Win32Util::fDisplayToolTip( wxGetInstance( ), ( HWND )GetHWND( ), GetToolTip( )->GetTip( ).c_str( ) );
	}

	void tWxSlapOnRadioBitmapButton::fBuildSolidColor( )
	{
		wxImage blankImage( fMax( 16, fGetSelectedBitmap( ).GetWidth( ) ), fMax( 16, fGetSelectedBitmap( ).GetHeight( ) ) );

		for( s32 x = 0; x < blankImage.GetWidth( ); ++x )
		{
			for( s32 y = 0; y < blankImage.GetHeight( ); ++y )
				blankImage.SetRGB( x, y, mSolidColor.x * 255, mSolidColor.y * 255, mSolidColor.z * 255 ); 
		}

		// set the de-selected bitmap
		fGetDeSelectedBitmap( ) = wxBitmap( blankImage );

		// create band around the border for the "selected" bitmap
		const u32 bandPixelCount = 4;
		for( s32 y = 0; y < blankImage.GetHeight( ); ++y )
		{
			for( u32 iband = 0; iband < bandPixelCount; ++iband )
			{
				const u32 c = fMin<u32>( blankImage.GetHeight( ) - y - 1, y, iband ) * 64;
				blankImage.SetRGB( iband, y, c, c, 255u );
				blankImage.SetRGB( blankImage.GetWidth( ) - ( 1 + iband ), y, c, c, 255u );
			}
		}
		for( s32 x = 0; x < blankImage.GetWidth( ); ++x )
		{
			for( u32 iband = 0; iband < bandPixelCount; ++iband )
			{
				const u32 c = fMin<u32>( blankImage.GetWidth( ) - x - 1, x, iband ) * 64;
				blankImage.SetRGB( x, iband, c, c, 255u );
				blankImage.SetRGB( x, blankImage.GetHeight( ) - ( 1 + iband ), c, c, 255u );
			}
		}

		// set the selected bitmap
		fGetSelectedBitmap( ) = wxBitmap( blankImage );

		if( fIsSelected( ) )
			fUpdateAllBitmaps( fGetSelectedBitmap( ) );
		else
			fUpdateAllBitmaps( fGetDeSelectedBitmap( ) );
	}

	void tWxSlapOnRadioBitmapButton::fUpdateAllBitmaps( wxBitmap& bmp )
	{
		SetBitmapLabel( bmp );
		SetBitmapHover( mSelected );
	}


}

