#include "MayaPluginPch.hpp"
#include "tMayaAttributeFloatBox.hpp"
#include "MayaUtil.hpp"
#include "tMayaDialogTab.hpp"

namespace Sig
{


	tMayaAttributeFloatBox::tMayaAttributeFloatBox( wxWindow* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision )
		: tWxSlapOnSpinner( parent, label, min, max, increment, precision )
		, tMayaAttributeControlBase( dynamic_cast< tMayaGuiBase* >( parent ) )
	{
		fDisableControl( );
	}

	void tMayaAttributeFloatBox::fOnControlUpdated( )
	{
		MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeFloatBox, fSetEachSelectedObject ) );
	}

	void tMayaAttributeFloatBox::fOnMayaSelChanged( )
	{
		// TODO need to fire this whenever a node's attribute is changed from maya's built-in attribute editors
		// in order to update our display of the selected objects' attributes

		mAttributeTracker.fStartTracking( );

		const u32 numSelectedNodes = MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeFloatBox, fOnMayaSelChangedEachObject ) );

		if( mAttributeTracker.fIsValueFound( ) )
		{
			fEnableControl( );
			if( mAttributeTracker.fIsValueConsistent( ) )
				fSetValueNoEvent( mAttributeTracker.fGetValue( ) );
			else
				fSetIndeterminateNoEvent( );
		}
		else
			fDisableControl( );
	}

	b32 tMayaAttributeFloatBox::fOnMayaSelChangedEachObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		MFnDependencyNode nodeFn( component );

		f32 value = fClamp( 0.f, mMin, mMax );
		b32 found = MayaUtil::fGetFloatAttribute( component, nodeFn, value, fGetLabelText( ).c_str( ) );
		if( !found )
		{
			MayaUtil::fUpdateFloatAttribute(
				component, nodeFn, value, fGetLabelText( ).c_str( ), 0.f, mMin, mMax, mIncrement, mPrecision );
			found = true;
		}

		return mAttributeTracker.fUpdate( found, value );
	}

	b32 tMayaAttributeFloatBox::fSetEachSelectedObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		const f32 value = fGetValue( );

		MFnDependencyNode nodeFn( component );

		MayaUtil::fUpdateFloatAttribute(
			component, nodeFn, value, fGetLabelText( ).c_str( ), 0.f, mMin, mMax, mIncrement, mPrecision );

		return true;
	}

}
