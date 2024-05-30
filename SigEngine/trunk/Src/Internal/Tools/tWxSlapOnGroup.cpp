#include "ToolsPch.hpp"
#include "tWxSlapOnGroup.hpp"


namespace Sig
{

	tWxSlapOnGroup::tWxSlapOnGroup( wxWindow* parent, const char* label, b32 collapsible, b32 scrollable, b32 expandToFit )
		: tWxAutoDelete( parent )
		, mLabel( label )
		, mMainPanel( 0 )
		, mCollapseButton( 0 )
		, mLabelText( 0 )
		, mCollapsible( collapsible )
		, mCollapsed( false )
	{
		const u32 borderFlag = scrollable ? wxBORDER_SIMPLE | wxVSCROLL : wxBORDER_SIMPLE;
		mMainPanel = new wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, /*wxTAB_TRAVERSAL | */borderFlag );
		mMainPanel->SetBackgroundColour( parent->GetBackgroundColour( ) );
		mMainPanel->SetForegroundColour( parent->GetForegroundColour( ) );

		if( !parent->GetSizer( ) )
			parent->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		// Note: expandToFit is actually an integer weight but I don't think we've ever given it anything other than 1 anywhere.
		parent->GetSizer( )->Add( mMainPanel, expandToFit, wxEXPAND | wxALL, collapsible ? 1 : 5 );

		mMainPanel->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		fRebuildGui();

		if( !scrollable )
		{
			// This isn't ideal but preferable to internally splitting mMainPanel into wxPanel and wxScrolledWindow members.
			mMainPanel->Connect( wxEVT_SCROLLWIN_TOP, wxScrollWinEventHandler(tWxSlapOnGroup::fForwarding) );
			mMainPanel->Connect( wxEVT_SCROLLWIN_BOTTOM, wxScrollWinEventHandler(tWxSlapOnGroup::fForwarding) );
			mMainPanel->Connect( wxEVT_SCROLLWIN_LINEUP, wxScrollWinEventHandler(tWxSlapOnGroup::fForwarding) );
			mMainPanel->Connect( wxEVT_SCROLLWIN_LINEDOWN, wxScrollWinEventHandler(tWxSlapOnGroup::fForwarding) );
			mMainPanel->Connect( wxEVT_SCROLLWIN_PAGEUP, wxScrollWinEventHandler(tWxSlapOnGroup::fForwarding) );
			mMainPanel->Connect( wxEVT_SCROLLWIN_PAGEDOWN, wxScrollWinEventHandler(tWxSlapOnGroup::fForwarding) );
			mMainPanel->Connect( wxEVT_SCROLLWIN_THUMBTRACK, wxScrollWinEventHandler(tWxSlapOnGroup::fForwarding) );
			mMainPanel->Connect( wxEVT_SCROLLWIN_THUMBRELEASE, wxScrollWinEventHandler(tWxSlapOnGroup::fForwarding) );
		}
	}

	void tWxSlapOnGroup::fToggleCollapsed( )
	{
		mCollapsed = !mCollapsed;
		if( mCollapsed && mCollapseButton )
			mCollapseButton->SetLabel( mLabel + " >>>" );
		else if( mCollapseButton )
			mCollapseButton->SetLabel( mLabel + " <<<" );

		// toggle visibility of all children
		{
			wxWindowList& children = mMainPanel->GetChildren( );
			for( u32 i = 1; i < children.size( ); ++i ) // start at 1 to skip label/button
				children[ i ]->Show( !mCollapsed );
		}

		mMainPanel->GetParent( )->Layout( );
		mMainPanel->GetParent( )->Refresh( );
		mMainPanel->GetParent( )->Update( );
	}

	void tWxSlapOnGroup::fSetCollapsed( b32 collapsed )
	{
		if( collapsed == mCollapsed )
			return;
		fToggleCollapsed( );
	}

	void tWxSlapOnGroup::fSetLabel( const char * label )
	{
		mLabel = label;

		if( mCollapseButton )
		{
			if( mCollapsed )
				mCollapseButton->SetLabel( mLabel + " >>>" );
			else
				mCollapseButton->SetLabel( mLabel + " <<<" );
		}

		if( mLabelText )
			mLabelText->SetLabel( mLabel );
	}

	void tWxSlapOnGroup::fRebuildGui( )
	{
		if( mCollapsible )
		{
			mCollapseButton = new wxButton(
				mMainPanel,
				wxID_ANY,
				mLabel + " <<<",
				wxDefaultPosition,
				wxDefaultSize/*,
				wxTAB_TRAVERSAL | wxWANTS_CHARS*/ );
			mCollapseButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tWxSlapOnGroup::fToggleCollapsed ), NULL, this );

			mMainPanel->GetSizer( )->Add( mCollapseButton, 0, wxEXPAND | wxALL, 2 );
		}
		else
		{
			if( mLabel && mLabel.length() )
			{
				mLabelText = new wxStaticText( 
					mMainPanel, 
					wxID_ANY, 
					mLabel, 
					wxDefaultPosition, 
					wxDefaultSize,
					wxALIGN_LEFT );

				mMainPanel->GetSizer( )->Add( mLabelText, 0, wxEXPAND | wxALL, 2 );
			}
			else
				mMainPanel->GetSizer( )->AddSpacer( 4 );
		}
	}

	void tWxSlapOnGroup::fToggleCollapsed( wxCommandEvent& event )
	{
		fToggleCollapsed( );
	}

	void tWxSlapOnGroup::fForwarding( wxScrollWinEvent& event )
	{
		// Send the event up higher since we don't care about this.
		event.Skip( true );
		event.ResumePropagation(3);
	}
}
