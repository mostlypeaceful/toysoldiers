#include "MayaPluginPch.hpp"
#include "tMayaAttributeMaskBox.hpp"
#include "MayaUtil.hpp"
#include "tMayaDialogTab.hpp"

namespace Sig
{


	tMayaAttributeMaskBox::tMayaAttributeMaskBox( wxWindow* parent, const char* label, u32 numBits, u32 defaultVal )
		: tWxSlapOnMask( parent, label, numBits )
		, tMayaAttributeControlBase( dynamic_cast< tMayaGuiBase* >( parent ) )
		, mDefaultValue( defaultVal )
	{
		fDisableControl( );
	}

	void tMayaAttributeMaskBox::fOnControlUpdated( )
	{
		MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeMaskBox, fSetEachSelectedObject ) );
	}

	void tMayaAttributeMaskBox::fOnMayaSelChanged( )
	{
		// TODO need to fire this whenever a node's attribute is changed from maya's built-in attribute editors
		// in order to update our display of the selected objects' attributes

		mAttributeTracker.fStartTracking( );

		const u32 numSelectedNodes = MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeMaskBox, fOnMayaSelChangedEachObject ) );

		if( mAttributeTracker.fIsValueFound( ) )
		{
			fEnableControl( );
			if( mAttributeTracker.fIsValueConsistent( ) )
				fSetValue( (u32)mAttributeTracker.fGetValue( ), false );
			else
				fSetValue( mDefaultValue, false );
		}
		else
			fDisableControl( );
	}

	b32 tMayaAttributeMaskBox::fOnMayaSelChangedEachObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		MFnDependencyNode nodeFn( component );

		s32 value = (s32)mDefaultValue;
		b32 found = MayaUtil::fGetIntAttribute( component, nodeFn, value, fGetLabelText( ).c_str( ) );
		if( !found )
		{
			MayaUtil::fUpdateIntAttribute(
				component, nodeFn, value, fGetLabelText( ).c_str( ), mDefaultValue, std::numeric_limits<s32>::min( ), std::numeric_limits<s32>::max( ) );
			found = true;
		}

		return mAttributeTracker.fUpdate( found, value );
	}

	b32 tMayaAttributeMaskBox::fSetEachSelectedObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		const s32 value = (s32)fGetValue( );

		MFnDependencyNode nodeFn( component );

		MayaUtil::fUpdateIntAttribute(
			component, nodeFn, value, fGetLabelText( ).c_str( ), mDefaultValue, std::numeric_limits<s32>::min( ), std::numeric_limits<s32>::max( ) );

		return true;
	}

}
