#include "ShadePch.hpp"
#include "tShadeControlPanel.hpp"
#include "tShadeNode.hpp"
#include "tWxSlapOnControl.hpp"
#include "tWxSelectStringDialog.hpp"
#include "Editor/tEditablePropertyTypes.hpp"

namespace Sig
{
	tShadeControlPanel::tShadeControlPanel( wxWindow* parent, tShadeNodeCanvas* canvas )
		: wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxSize( 280, wxDefaultSize.y ), wxTAB_TRAVERSAL | wxBORDER_SIMPLE | wxVSCROLL )
		, mCanvas( canvas )
		, mHeaderText( 0 )
		, mPropertyPanel( 0 )
	{
		SetMinSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetMaxSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetBackgroundColour( wxColour( 0x33, 0x33, 0x33 ) );
		SetForegroundColour( wxColour( 0x11, 0xff, 0x11 ) );

		mOnPropertyChanged.fFromMethod< tShadeControlPanel, &tShadeControlPanel::fOnPropertyChanged >( this );
		mCommonProps.mOnPropertyChanged.fAddObserver( &mOnPropertyChanged );

		tWxSlapOnControl::fSetLabelWidth( 80 );
		tWxSlapOnControl::fSetControlWidth( 150 );

		// set global filename browser filters
		tEditablePropertyFileNameString::fAddFilter( tShadeNode::cNamePropertiesDefault, "*.tga" );
		tEditablePropertyFileNameString::fAddFilter( tShadeNode::cNamePropertiesDefault, "*.png" );
		tEditablePropertyFileNameString::fAddFilter( tShadeNode::cNamePropertiesDefault, "*.bmp" );
		tEditablePropertyFileNameString::fAddFilter( tShadeNode::cNamePropertiesDefault, "*.jpg" );
		tEditablePropertyFileNameString::fAddFilter( tShadeNode::cNamePropertiesDefault, "*.dds" );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mHeaderText = new wxStaticText( this, wxID_ANY, "Selection: None" );
		GetSizer( )->AddSpacer( 8 );
		GetSizer( )->Add( mHeaderText, 0, wxLEFT, 8 );
		GetSizer( )->AddSpacer( 8 );

		mPropertyPanel = new wxScrolledWindow( this );
		mPropertyPanel->SetBackgroundColour( wxColour( 0x55, 0x55, 0x55 ) );
		mPropertyPanel->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
		GetSizer( )->Add( mPropertyPanel, 1, wxEXPAND | wxALL, 0 );
	}

	void tShadeControlPanel::fOnSelectionChanged( const tDAGNodeList& selected )
	{
		Freeze( );
		mPropertyPanel->DestroyChildren( );
		mCommonProps.fClearGui( );

		if( selected.fCount( ) == 0 )
			mHeaderText->SetLabel( "Selection: None" );
		else if( selected.fCount( ) > 1 )
			mHeaderText->SetLabel( "Selection: Multiple" );
		else
		{
			tShadeNode* shadeNode = dynamic_cast<tShadeNode*>( selected.fFront( ).fGetRawPtr( ) );
			if( shadeNode )
			{
				mHeaderText->SetLabel( "Selection: " + shadeNode->fName( ) );

				mCommonProps = shadeNode->fShadeProps( );
				mCommonProps.fCollectCommonPropertiesForGui( shadeNode->fShadeProps( ) );
				tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps, false );
			}
			else
			{
				mHeaderText->SetLabel( "Selection: Invalid" );
			}
		}

		mPropertyPanel->Layout( );
		Thaw( );
	}

	void tShadeControlPanel::fOnPropertyChanged( tEditableProperty& property )
	{
		mCanvas->fEditorActions( ).fForceSetDirty( true );
	}

}
