#ifndef __tWxColumnListBox__
#define __tWxColumnListBox__
#include <wx/listctrl.h>

namespace Sig
{
	class toolsgui_export tWxColumnListBox : public wxListView
	{
	public:
		tWxColumnListBox( wxWindow *parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, long style );

		/// 
		/// \brief Pushes a column back to the right side of the table.
		void fCreateColumn( wxString columnHeader );

		/// 
		/// \brief Creates a row with only the first column's text.
		/// returns the index of the added row.
		u64 fCreateRow( wxString firstColumnText )
		{
			return InsertItem( GetItemCount( ), firstColumnText );
		}

		/// 
		/// \brief Creates a row with full data for all expected columns.
		/// returns the index of the added row.
		u64 fCreateRow( const tGrowableArray< wxString >& data );

		void fAutosizeAllColumns( );
	};
}

#endif//__tWxColumnListBox__
