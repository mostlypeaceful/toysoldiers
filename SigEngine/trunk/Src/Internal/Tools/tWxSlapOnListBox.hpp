//------------------------------------------------------------------------------
// \file tWxSlapOnListBox.hpp - 17 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tWxSlapOnListBox__
#define __tWxSlapOnListBox__
#include "tWxSlapOnControl.hpp"

class wxListBox;

namespace Sig
{
	///
	/// \class tWxSlapOnListBox
	/// \brief 
	class tools_export tWxSlapOnListBox : public tWxSlapOnControl
	{

	public:

		tWxSlapOnListBox( wxWindow* parent, const char* label, b32 sorted, b32 uniqueItems, b32 checkable );

		virtual void fEnableControl( );
		virtual void fDisableControl( );

		void fSetToolTip( const wxString& toolTip );

		void fAddItem( const char * item );
		void fAddItems( u32 itemCount, const char * items[] );
		
		void fClearItems( );
		void fClearSelectedItems( );

		b32 fIsChecked( u32 index ) const;
		void fSetChecked( u32 index, b32 checked );
		

		u32 ItemCount( ) const;
		void Item( u32 index, wxString & itemOut ) const;

		wxListBox * fListBox( ) const { return mListBox; }

	protected:

		wxWindow * fGetParentWindow( );

	protected:

		b32 mUniqueItems;
		wxListBox * mListBox;
		b32 mCheckable;
	};
}

#endif//__tWxSlapOnListBox__
