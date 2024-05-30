//------------------------------------------------------------------------------
// \file tMayaAttributeListBox.cpp - 17 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "MayaPluginPch.hpp"
#include "tMayaAttributeListBox.hpp"
#include "MayaUtil.hpp"

namespace Sig
{

	//------------------------------------------------------------------------------
	// tMayaAttributeListBoxBase
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tMayaAttributeListBoxBase::tMayaAttributeListBoxBase( 
		tMayaGuiBase * parent, tWxSlapOnListBox * listBox )
		: tMayaAttributeControlBase( parent )
		, mListBox( listBox )
		, mUpdatingSelection( false )
	{
	}

	//------------------------------------------------------------------------------
	void tMayaAttributeListBoxBase::fOnControlUpdated( )
	{
		// Don't update the nodes if we're in the process of updating the list box
		// because the selected nodes changed
		if( mUpdatingSelection )
			return;

		MayaUtil::fForEachSelectedNode(
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeListBoxBase, fSetEachSelectedObject ) );
	}

	//------------------------------------------------------------------------------
	void tMayaAttributeListBoxBase::fOnMayaSelChanged( )
	{
		mUpdatingSelection = true;

		mListBox->fClearItems( );
		const u32 numSelectedNodes = MayaUtil::fForEachSelectedNode(
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeListBoxBase, fOnMayaSelChangedEachObject ) );
		
		
		// We only support adding to one node at a time
		if( numSelectedNodes > 1 )
			fDisableControl( );
		else
			fEnableControl( );

		mUpdatingSelection = false;
	}

	//------------------------------------------------------------------------------
	b32 tMayaAttributeListBoxBase::fOnMayaSelChangedEachObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		MFnDependencyNode nodeFn( component );

		const wxString label = mListBox->fGetLabelText( );

		s32 itemCount = 0;
		b32 found = MayaUtil::fGetIntAttribute( component, nodeFn, itemCount, label.c_str( ) );
		if( !found )
			MayaUtil::fUpdateIntAttribute( component, nodeFn, 0, label.c_str( ), 0, 0, 0 );

		for( u32 i = 0; i < (u32)itemCount; ++i )
		{
			wxString longName;
			longName.Printf( "%s%d", label.c_str( ), i );

			std::string item;
			b32 found = MayaUtil::fGetStringAttribute( component, nodeFn, item, longName.c_str( ) );
			if( found )
				mListBox->fAddItem( wxString( item.c_str( ) ) );
			else
				log_warning( "tMayaAttributeListBox has out of sync items" );
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tMayaAttributeListBoxBase::fSetEachSelectedObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		MFnDependencyNode nodeFn( component );

		const wxString label = mListBox->fGetLabelText( );
		const u32 itemCount = mListBox->ItemCount( );

		MayaUtil::fUpdateIntAttribute(
			component, nodeFn, itemCount, label.c_str( ), 0, 0, 0 );

		// No more processing if we've got em all
		if( !itemCount )
			return true;
		
		for( u32 i = 0; i < itemCount; ++i )
		{
			wxString longName;
			longName.Printf( "%s%d", label.c_str( ), i );
			
			wxString str;
			mListBox->Item( i, str );
			MayaUtil::fUpdateStringAttribute(
				component, nodeFn, std::string( str.c_str( ) ), longName.c_str( ) );
		}

		return true;
	}

	//------------------------------------------------------------------------------
	// tMayaAttributeListBox
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tMayaAttributeListBox::tMayaAttributeListBox( wxWindow* parent, const char* label )
		: tWxSlapOnListBox( parent, label, true, true, false )
		, tMayaAttributeListBoxBase( dynamic_cast<tMayaGuiBase *>( parent ), this )
	{
		fDisableControl( );
	}
}
