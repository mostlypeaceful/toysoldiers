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
		wxScrolledWindow* mMainPanel;

		wxButton* mCollapseButton;
		wxStaticText * mLabelText;

		const b32 mCollapsible;
		b32 mCollapsed;

	public:
		tWxSlapOnGroup( wxWindow* parent, const char* label, b32 collapsible, b32 scrollable = false, b32 expandToFit = false );
		void fToggleCollapsed( );
		void fSetCollapsed( b32 collapsed );
		inline wxScrolledWindow* fGetMainPanel( ) { return mMainPanel; }

		const char * fLabel( ) { return mLabel; }
		void fSetLabel( const char * label );

		void fRebuildGui( );

	private:
		void fToggleCollapsed( wxCommandEvent& event );
		void fForwarding( wxScrollWinEvent& event );
	};
}


#endif//__tWxSlapOnGroup__
