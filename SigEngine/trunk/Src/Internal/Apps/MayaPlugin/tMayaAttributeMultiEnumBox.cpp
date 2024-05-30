//------------------------------------------------------------------------------
// \file tMayaAttributeMultiEnumBox.cpp - 14 Apr 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "MayaPluginPch.hpp"
#include "tMayaAttributeMultiEnumBox.hpp"
#include "tWxSlapOnChoice.hpp"

namespace Sig
{
	tMayaAttributeMultiEnumBox::tMayaAttributeMultiEnumBox( 
		wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defSlot )
		: tWxSlapOnAddRemoveChoicesListBox( parent, label, true, true, enumNames, numEnumNames, defSlot )
		, tMayaAttributeListBoxBase( dynamic_cast< tMayaGuiBase* >( parent ), this )
	{
		fDisableControl( );
	}
}
