//------------------------------------------------------------------------------
// \file tMayaAttributeMultiEnumBox.hpp - 14 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMayaAttributeMultiEnumBox__
#define __tMayaAttributeMultiEnumBox__
#include "tWxSlapOnAddRemoveChoicesListBox.hpp"
#include "tMayaAttributeListBox.hpp"

class wxChoice;

namespace Sig
{
	class tMayaAttributeMultiEnumBox : 
		public tWxSlapOnAddRemoveChoicesListBox,
		public tMayaAttributeListBoxBase
	{
	public:
		tMayaAttributeMultiEnumBox( wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defSlot = ~0 );

		virtual void fOnControlUpdated( ) 
		{
			tMayaAttributeListBoxBase::fOnControlUpdated( );
		}

		virtual void fDisableControl( )
		{
			tWxSlapOnAddRemoveChoicesListBox::fDisableControl( );
		}

		virtual void fEnableControl( )
		{
			tWxSlapOnAddRemoveChoicesListBox::fEnableControl( );
		}

	private:
		
	};
}

#endif//__tMayaAttributeMultiEnumBox__
