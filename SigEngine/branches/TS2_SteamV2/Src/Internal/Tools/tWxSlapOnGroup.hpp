#ifndef __tWxSlapOnGroup__
#define __tWxSlapOnGroup__
#include "tWxAutoDelete.hpp"

namespace Sig
{
	///
	/// \brief Represents a group of controls that has a label and a border.
	/// Can optionally act as a collapsible container.
	class tools_export tWxSlapOnGroup : public wxEvtHandler, public tWxAutoDelete
	{
	protected:
		wxString mLabel;
		wxPanel* mMainPanel;

		wxButton* mCollapseButton;
		wxStaticText * mLabelText;

		b32 mCollapsed;

	public:
		tWxSlapOnGroup( wxWindow* parent, const char* label, b32 collapsible );
		void fToggleCollapsed( );
		void fSetCollapsed( b32 collapsed );
		inline wxPanel* fGetMainPanel( ) { return mMainPanel; }

		const char * fLabel( ) { return mLabel; }
		void fSetLabel( const char * label );

	private:
		void fToggleCollapsed( wxCommandEvent& event );
	};
}


#endif//__tWxSlapOnGroup__
