#include "ToolsPch.hpp"
#include "tWxSlapOnColorPicker.hpp"
#include "wx/colordlg.h"


namespace Sig
{
	void tColorPickerData::fNormalize( )
	{
		const Math::tVec4f rgba = fExpandRgba( );
		const f32 max = rgba.fMaxMagnitude( );
		if( max > 1.f )
			mRgbScale = max;
		else
			mRgbScale = 1.f;
		mRgb = rgba.fXYZ( ) / mRgbScale;
		mAlpha = rgba.w;
	}


	tFixedArray<wxColour,16> tWxSlapOnColorPicker::gCustomColors;

	b32 tWxSlapOnColorPicker::fPopUpDialog( 
		wxButton* button, 
		tColorPickerData& value,
		const tColorPickerData& min, 
		const tColorPickerData& max )
	{
		wxColourDialog colorDialog( button, 0 );

		wxColour col = wxColour( fRound<u8>( value.mRgb.x * 255.f ), fRound<u8>( value.mRgb.y * 255.f ), fRound<u8>( value.mRgb.z * 255.f ) );
		colorDialog.GetColourData( ).SetChooseFull( true );
		colorDialog.GetColourData( ).GetColour( ) = col;

		for( u32 i = 0; i < gCustomColors.fCount( ); ++i )
			colorDialog.GetColourData( ).SetCustomColour( i, gCustomColors[ i ] );

		const s32 screenWidth = ( s32 )Win32Util::fGetDesktopWidth( );
		const s32 screenHeight = ( s32 )Win32Util::fGetDesktopHeight( );
		wxPoint screenPos = button->GetScreenPosition( );
		if( screenPos.x > screenWidth / 2 )
			screenPos.x -= colorDialog.GetSize( ).x;
		if( screenPos.y > screenHeight / 2 )
			screenPos.y -= colorDialog.GetSize( ).y;
		colorDialog.SetPosition( screenPos );
		if( colorDialog.ShowModal( ) == wxID_OK )
		{
			for( u32 i = 0; i < gCustomColors.fCount( ); ++i )
				gCustomColors[ i ] = colorDialog.GetColourData( ).GetCustomColour( i );

			wxColourData retData = colorDialog.GetColourData( );

			col = retData.GetColour( );
			value.mRgb = Math::tVec3f( col.Red( ) / 255.f, col.Green( ) / 255.f, col.Blue( ) / 255.f );
			value = fClamp( value, min, max );

			col = wxColour( fRound<u8>( value.mRgb.x * 255.f ), fRound<u8>( value.mRgb.y * 255.f ), fRound<u8>( value.mRgb.z * 255.f ) );
			button->SetBackgroundColour( col );
			button->Refresh( );

			return true;
		}

		return false;
	}


	tWxSlapOnColorPicker::tWxSlapOnColorPicker( 
		wxWindow* parent, 
		const char* label,
		const tColorPickerData& initState, 
		const tColorPickerData& min,
		const tColorPickerData& max )
		: tWxSlapOnControl( parent, label )
		, mValue( initState )
		, mMin( min )
		, mMax( max )
	{
		const s32 buttonHeight = 24;

		mButton = new wxButton(
			parent,
			wxID_ANY,
			"",
			wxDefaultPosition,
			wxSize( fControlWidth( ), buttonHeight ) );

		fAddWindowToSizer( mButton, true );

		mButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tWxSlapOnColorPicker::fOnButtonPressedInternal ), NULL, this );

		fSetValue( initState );
	}

	void tWxSlapOnColorPicker::fEnableControl( )
	{
		mButton->Enable( true );
	}

	void tWxSlapOnColorPicker::fDisableControl( )
	{
		mButton->Enable( false );
	}

	void tWxSlapOnColorPicker::fOnButtonPressedInternal( wxCommandEvent& )
	{
		if( fPopUpDialog( mButton, mValue, mMin, mMax ) )
			fOnControlUpdated( );
	}

	tColorPickerData tWxSlapOnColorPicker::fGetValue( ) const
	{
		return mValue;
	}

	void tWxSlapOnColorPicker::fSetValue( const tColorPickerData& colorData )
	{
		mValue = fClamp( colorData, mMin, mMax );

		wxColour col = wxColour( fRound<u8>( colorData.mRgb.x * 255.f ), fRound<u8>( colorData.mRgb.y * 255.f ), fRound<u8>( colorData.mRgb.z * 255.f ) );
		mButton->SetBackgroundColour( col );
		mButton->Refresh( );
	}

}

