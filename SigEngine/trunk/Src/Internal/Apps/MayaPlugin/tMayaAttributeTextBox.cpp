#include "MayaPluginPch.hpp"
#include "tMayaAttributeTextBox.hpp"
#include "MayaUtil.hpp"

namespace Sig
{


	tMayaAttributeTextBox::tMayaAttributeTextBox( wxWindow* parent, const char* label )
		: tWxSlapOnTextBox( parent, label )
		, tMayaAttributeControlBase( dynamic_cast< tMayaGuiBase* >( parent ) )
		, mAttributeTracker( "" )
	{
		fDisableControl( );
	}

	void tMayaAttributeTextBox::fOnControlUpdated( )
	{
		MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeTextBox, fSetEachSelectedObject ) );
	}

	void tMayaAttributeTextBox::fOnMayaSelChanged( )
	{
		// TODO need to fire this whenever a node's attribute is changed from maya's built-in attribute editors

		mAttributeTracker.fStartTracking( );

		const u32 numSelectedNodes = MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeTextBox, fOnMayaSelChangedEachObject ) );

		if( mAttributeTracker.fIsValueFound( ) )
		{
			fEnableControl( );
			if( mAttributeTracker.fIsValueConsistent( ) )
				fSetValue( mAttributeTracker.fGetValue( ) );
			else
				fSetValue( "" );
		}
		else
			fDisableControl( );
	}

	b32 tMayaAttributeTextBox::fOnMayaSelChangedEachObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		MFnDependencyNode nodeFn( component );

		std::string value = "";
		b32 found = MayaUtil::fGetStringAttribute( component, nodeFn, value, fGetLabelText( ).c_str( ) );
		if( !found )
		{
			MayaUtil::fUpdateStringAttribute(
				component, nodeFn, value, fGetLabelText( ).c_str( ) );
			found = true;
		}

		return mAttributeTracker.fUpdate( found, value );
	}

	b32 tMayaAttributeTextBox::fSetEachSelectedObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		const std::string value = fGetValue( );

		MFnDependencyNode nodeFn( component );

		MayaUtil::fUpdateStringAttribute(
			component, nodeFn, value, fGetLabelText( ).c_str( ) );

		return true;
	}

}
