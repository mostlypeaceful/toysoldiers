//------------------------------------------------------------------------------
// \file tWxSlapOnAddRemoveChoicesListBox.hpp - 14 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tWxSlapOnAddRemoveChoicesListBox__
#define __tWxSlapOnAddRemoveChoicesListBox__
#include "tWxSlapOnAddRemoveListBox.hpp"

class wxChoice;

namespace Sig
{
	///
	/// \class tWxSlapOnAddRemoveChoicesListBox
	/// \brief 
	class tools_export tWxSlapOnAddRemoveChoicesListBox : public tWxSlapOnAddRemoveListBox
	{
	public:

		tWxSlapOnAddRemoveChoicesListBox( 
			wxWindow* parent, 
			const char* label, 
			b32 sorted, 
			b32 uniqueItems,
			const wxString enumNames[], 
			u32 numEnumNames, 
			u32 defChoice = ~0);

		tWxSlapOnAddRemoveChoicesListBox( 
			wxWindow* parent, 
			const char* label, 
			b32 sorted, 
			b32 uniqueItems,
			const char * addLabel,
			const char * removeLabel,
			const wxString enumNames[], 
			u32 numEnumNames, 
			u32 defChoice = ~0 );

		virtual ~tWxSlapOnAddRemoveChoicesListBox( ) { }

		virtual void fDisableControl( );
		virtual void fEnableControl( );

	protected:
		
		virtual void fOnAdd( );

	private:

		void fSetupChoices( 
			wxWindow * parent,
			const wxString enumNames[], 
			u32 numEnumNames, 
			u32 defChoice );

	private:

		wxChoice * mChoice;
	};
}

#endif//__tWxSlapOnAddRemoveChoicesListBox__
