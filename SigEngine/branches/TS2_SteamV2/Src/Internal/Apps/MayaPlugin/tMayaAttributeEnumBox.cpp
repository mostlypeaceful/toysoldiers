#include "MayaPluginPch.hpp"
#include "tMayaAttributeEnumBox.hpp"
#include "MayaUtil.hpp"

namespace Sig
{

	tMayaAttributeEnumBox::tMayaAttributeEnumBox( wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defSlot )
		: tWxSlapOnChoice( parent, label, enumNames, numEnumNames, defSlot )
		, tMayaAttributeControlBase( dynamic_cast< tMayaGuiBase* >( parent ) )
		, mDefSlot( defSlot )
	{
		// store the enum names
		mEnumNames.fSetCount( numEnumNames );
		for( u32 i = 0; i < mEnumNames.fCount( ); ++i )
			mEnumNames[i] = enumNames[i];

		fDisableControl( );
	}

	void tMayaAttributeEnumBox::fOnControlUpdated( )
	{
		MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeEnumBox, fSetEachSelectedObject ) );
	}

	void tMayaAttributeEnumBox::fOnMayaSelChanged( )
	{
		// TODO need to fire this whenever a node's attribute is changed from maya's built-in attribute editors
		// in order to update our display of the selected objects' attributes

		mAttributeTracker.fStartTracking( );

		const u32 numSelectedNodes = MayaUtil::fForEachSelectedNode( 
			make_delegate_memfn( MayaUtil::tForEachObject, tMayaAttributeEnumBox, fOnMayaSelChangedEachObject ) );

		if( mAttributeTracker.fIsValueFound( ) )
		{
			fEnableControl( );
			if( mAttributeTracker.fIsValueConsistent( ) )
				fSetValue( mAttributeTracker.fGetValue( ) );
			else
				fSetValue( -1 );
		}
		else
			fDisableControl( );
	}

	b32 tMayaAttributeEnumBox::fOnMayaSelChangedEachObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		MFnDependencyNode nodeFn( component );

		u32 value = ( ( mDefSlot != ~0 ) ? mDefSlot : 0 );
		b32 found = MayaUtil::fGetEnumAttribute( component, nodeFn, value, fGetLabelText( ).c_str( ) );
		if( !found )
		{
			MayaUtil::fUpdateEnumAttribute(
				component, nodeFn, value, fGetLabelText( ).c_str( ), mEnumNames.fBegin( ), mEnumNames.fCount( ), 0 );
			found = true;
		}

		return mAttributeTracker.fUpdate( found, value );
	}

	b32 tMayaAttributeEnumBox::fSetEachSelectedObject( MDagPath& path, MObject& component )
	{
		if( !fIsNodeMyType( path ) )
			return true;

		const u32 value = fGetValue( );

		MFnDependencyNode nodeFn( component );

		MayaUtil::fUpdateEnumAttribute(
			component, nodeFn, value, fGetLabelText( ).c_str( ), mEnumNames.fBegin( ), mEnumNames.fCount( ), 0 );

		return true;
	}

}

