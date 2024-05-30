#include "ToolsPch.hpp"
#include "tWxSlapOnGroup.hpp"


namespace Sig
{

	tWxSlapOnGroup::tWxSlapOnGroup( wxWindow* parent, const char* label, b32 collapsible )
		: tWxAutoDelete( parent )
		, mLabel( label )
		, mMainPanel( 0 )
		, mCollapseButton( 0 )
		, mLabelText( 0 )
		, mCollapsed( false )
	{
		const u32 borderFlag = wxBORDER_SIMPLE;//collapsible ? wxBORDER_RAISED : wxBORDER_SIMPLE;
		mMainPanel = new wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, /*wxTAB_TRAVERSAL | */borderFlag );
		mMainPanel->SetBackgroundColour( parent->GetBackgroundColour( ) );
		mMainPanel->SetForegroundColour( parent->GetForegroundColour( ) );

		if( !parent->GetSizer( ) )
			parent->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		parent->GetSizer( )->Add( mMainPanel, 0, wxEXPAND | wxALL, collapsible ? 1 : 5 );

		mMainPanel->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		if( collapsible )
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
			if( label && strlen( label ) > 0 )
			{
				mLabelText = new wxStaticText( 
					mMainPanel, 
					wxID_ANY, 
					label, 
					wxDefaultPosition, 
					wxDefaultSize,
					wxALIGN_LEFT );

				mMainPanel->GetSizer( )->Add( mLabelText, 0, wxEXPAND | wxALL, 2 );
			}
			else
				mMainPanel->GetSizer( )->AddSpacer( 4 );
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

	void tWxSlapOnGroup::fToggleCollapsed( wxCommandEvent& event )
	{
		fToggleCollapsed( );
	}

}
