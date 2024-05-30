#include "ToolsGuiPch.hpp"
#include "tWxColumnListBox.hpp"

namespace Sig
{
	tWxColumnListBox::tWxColumnListBox( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) 
		: wxListView(parent, id, pos, size, style)
	{ 
	}

	void tWxColumnListBox::fCreateColumn( wxString columnHeader )
	{
		wxListItem itemCol;
		itemCol.SetText( columnHeader );
		itemCol.SetImage( -1 );
		InsertColumn( GetColumnCount( ), itemCol );
	}

	u64 tWxColumnListBox::fCreateRow( const tGrowableArray< wxString >& data )
	{
		sigassert( data.fCount( ) > 0 && data.fCount( ) == GetColumnCount( ) );

		// Create the row and the first column entry.
		const u64 rowAddedIdx = fCreateRow( data[0] );

		// Add all subsequent rows.
		const u64 numRows = data.fCount( );
		for( u64 i = 1; i < numRows; ++i )
			SetItem( rowAddedIdx, i, data[i] );

		return rowAddedIdx;
	}

	void tWxColumnListBox::fAutosizeAllColumns( )
	{
		for( s32 i = 0; i < GetColumnCount( ); ++i )
			SetColumnWidth( i, wxLIST_AUTOSIZE_USEHEADER );
	}
}

