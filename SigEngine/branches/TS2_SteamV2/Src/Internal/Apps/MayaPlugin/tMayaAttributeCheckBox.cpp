#include "MayaPluginPch.hpp"
#include "tMayaAttributeCheckBox.hpp"
#include "MayaUtil.hpp"

namespace Sig
{


	tMayaAttributeCheckBox::tMayaAttributeCheckBox( wxWindow* parent, const char* label )
		: tWxSlapOnCheckBox( parent, label )
		, tMayaAttributeControlBase( dynamic_cast< tMayaGuiBase* >( parent ) )
	{
		fDisableControl( );
	}

	void tMayaAttributeCheckBox::fOnControlUpdated( )
	{
		MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeCheckBox, fSetEachSelectedObject ) );
	}

	void tMayaAttributeCheckBox::fOnMayaSelChanged( )
	{
		// TODO need to fire this whenever a node's attribute is changed from maya's built-in attribute editors

		mAttributeTracker.fStartTracking( );

		const u32 numSelectedNodes = MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeCheckBox, fOnMayaSelChangedEachObject ) );

		if( mAttributeTracker.fIsValueFound( ) )
		{
			fEnableControl( );
			if( mAttributeTracker.fIsValueConsistent( ) )
				fSetValue( mAttributeTracker.fGetValue( ) ? cTrue : cFalse );
			else
				fSetValue( cGray );
		}
		else
			fDisableControl( );
	}

	b32 tMayaAttributeCheckBox::fOnMayaSelChangedEachObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		MFnDependencyNode nodeFn( component );

		b32 value = false;
		b32 found = MayaUtil::fGetBoolAttribute( component, nodeFn, value, fGetLabelText( ).c_str( ) );
		if( !found )
		{
			MayaUtil::fUpdateBoolAttribute(
				component, nodeFn, value, fGetLabelText( ).c_str( ), false );
			found = true;
		}

		return mAttributeTracker.fUpdate( found, value );
	}

	b32 tMayaAttributeCheckBox::fSetEachSelectedObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		const b32 value = fGetValue( )==cTrue;

		MFnDependencyNode nodeFn( component );

		MayaUtil::fUpdateBoolAttribute(
			component, nodeFn, value, fGetLabelText( ).c_str( ), false );

		return true;
	}

}
