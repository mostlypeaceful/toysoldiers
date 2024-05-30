//------------------------------------------------------------------------------
// \file tWxSlapOnAddRemoveListBox.hpp - 18 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tWxSlapOnAddRemoveListBox__
#define __tWxSlapOnAddRemoveListBox__
#include "tWxSlapOnListBox.hpp"

namespace Sig
{
	///
	/// \class tWxSlapOnAddRemoveListBox
	/// \brief 
	class tools_export tWxSlapOnAddRemoveListBox : public tWxSlapOnListBox
	{
	public:

		tWxSlapOnAddRemoveListBox( 
			wxWindow* parent, 
			const char* label, 
			b32 sorted, 
			b32 uniqueItems );

		tWxSlapOnAddRemoveListBox( 
			wxWindow* parent, 
			const char* label, 
			b32 sorted, 
			b32 uniqueItems,
			const char * addLabel,
			const char * removeLabel );

		virtual ~tWxSlapOnAddRemoveListBox( ) { }

		virtual void fDisableControl( );
		virtual void fEnableControl( );

	protected:

		virtual void fOnAdd( ) { }
		virtual void fOnRemove( );

	private:

		void fSetupButtons( wxWindow * parent, const char * addLabel, const char * removeLabel );
		void fOnAddInternal( wxCommandEvent& );
		void fOnRemoveInternal( wxCommandEvent& );

	private:

		wxBoxSizer * mButtonSizer;
		wxButton * mAddButton;
		wxButton * mRemoveButton;

	};

}

#endif//__tWxSlapOnAddRemoveListBox__
