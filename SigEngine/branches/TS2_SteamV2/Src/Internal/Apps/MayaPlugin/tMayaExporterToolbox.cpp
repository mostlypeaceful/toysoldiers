#include "MayaPluginPch.hpp"
#include "tMayaExporterToolbox.hpp"
#include "tExporterGuiFactory.hpp"

#include "MayaUtil.hpp"
#include "tMayaAttributeCheckBox.hpp"
#include "tMayaAttributeTextBox.hpp"
#include "tMayaAttributeEnumBox.hpp"
#include "tMayaAttributeMultiEnumBox.hpp"
#include "tMayaAttributeFloatBox.hpp"
#include "tMayaAttributeMaskBox.hpp"
#include "tMayaDialogGroupBox.hpp"
#include "tMayaDialogTab.hpp"
#include "tMayaSigmlQuickExport.hpp"
#include "tMayaCheckOutButton.hpp"
#include "tMayaTexturePathFixerButton.hpp"
#include "tMayaAttributeFileListBox.hpp"

namespace Sig
{

	class tMayaExporterGuiFactory : public tExporterGuiFactory
	{
		//// containers

		virtual tWxSlapOnDialog*		fCreateDialog( const char* title, wxWindow* parent=0 ) 
			{ sigassert( 0 ); return 0; }
		virtual tWxSlapOnTabSet*		fCreateTabSet( wxWindow* parent ) 
			{ return new tWxSlapOnTabSet( parent ); }
		virtual tWxSlapOnPanel*			fCreatePanel( wxWindow* parent, const char* title ) 
			{ return new tMayaDialogTab( parent, title ); }
		virtual tWxSlapOnPanel*			fCreatePanel( wxNotebook* parent, const char* title )
			{ return new tMayaDialogTab( parent, title ); }
		virtual tWxSlapOnGroup*			fCreateGroup( wxWindow* parent, const char* title, b32 collapsible )
			{ return new tMayaDialogGroupBox( parent, title, collapsible ); }

		//// controls

		virtual tWxSlapOnCheckBox*		fCreateCheckBoxAttribute( wxWindow* parent, const char* label )
			{ return new tMayaAttributeCheckBox( parent, label ); }
		virtual tWxSlapOnTextBox*		fCreateTextBoxAttribute( wxWindow* parent, const char* label )
			{ return new tMayaAttributeTextBox( parent, label ); }
		virtual tWxSlapOnChoice*		fCreateChoiceAttribute( wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defSlot )
			{ return new tMayaAttributeEnumBox( parent, label, enumNames, numEnumNames, defSlot ); }
		virtual tWxSlapOnControl*		fCreateMultiChoiceAttribute( wxWindow* parent, const char * label, const wxString enumNames[], u32 numEnumNames, u32 defSlot = ~0 )
			{ return new tMayaAttributeMultiEnumBox( parent, label, enumNames, numEnumNames, defSlot ); }
		virtual tWxSlapOnSpinner*		fCreateSpinnerAttribute( wxWindow* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision )
			{ return new tMayaAttributeFloatBox( parent, label, min, max, increment, precision ); }
		virtual tWxSlapOnMask*			fCreateMaskAttribute( wxWindow* parent, const char* label, u32 numBits, u32 defaultValue )
			{ return new tMayaAttributeMaskBox( parent, label, numBits, defaultValue ); };
		virtual tWxSlapOnQuickExport*	fCreateQuickExport( wxWindow* parent, const char* label )
			{ return new tMayaSigmlQuickExport( parent, label ); }
		virtual tWxSlapOnButton*		fCreateTexturePathFixerButton( wxWindow* parent, const char* label )
			{ return new tMayaTexturePathFixerButton( parent, label ); }
		virtual tWxSlapOnButton*		fCreateCheckOutButton( wxWindow* parent, const char* label )
			{ return new tMayaCheckOutButton( parent, label ); }
		virtual tWxSlapOnListBox*		fCreateAnimSourceSkeletonsListBox( wxWindow * parent, const char * label)
			{ return new tMayaAttributeFileListBox( parent, label, "Select Animation Source Skeleton", "*.sklml" ); }
	};


	tMayaExporterToolbox::tMayaExporterToolbox( const tFilePathPtr& iconBmpPath )
		: tExporterToolbox( "SigExporter in Maya", tStrongPtr<tExporterGuiFactory>( new tMayaExporterGuiFactory ) )
		, mNumGeomObjectsSelected( 0 )
		, mNumBoneObjectsSelected( 0 )
		, mNumLightObjectsSelected( 0 )
		, mNumMaterialsSelected( 0 )
	{
		wxIcon wxicon;
		wxicon.LoadFile( iconBmpPath.fCStr( ), wxBITMAP_TYPE_ICO );
		SetIcon( wxicon );

		// store maya's hwnd
		fSetRawParent( M3dView::applicationShell( ) );

		// create the maya event callbacks
		mOnMayaIdle.fReset( new tMayaEvent( 
			tMayaEvent::fEventNameIdle( ), 
			make_delegate_memfn( tMayaEvent::tCallback, tMayaExporterToolbox, fOnMayaIdle ) ) );
		mOnMayaSelChanged.fReset( new tMayaEvent( 
			tMayaEvent::fEventNameSelChanged( ),
			make_delegate_memfn( tMayaEvent::tCallback, tMayaExporterToolbox, fOnMayaSelChanged ) ) );

		// do this last to update all ui
		fOnMayaSelChanged( );

		fLoadLayout( );
	}

	tMayaExporterToolbox::~tMayaExporterToolbox( )
	{
		fSaveLayout( );
	}

	void tMayaExporterToolbox::fOnMayaIdle( )
	{
		if( fAutoHandleTopMost( ) )
			fSaveLayout( );
	}

	void tMayaExporterToolbox::fOnMayaSelChanged( )
	{
		mNumGeomObjectsSelected = 0;
		mNumBoneObjectsSelected = 0;
		mNumLightObjectsSelected = 0;
		mNumMaterialsSelected = 0;

		MayaUtil::fForEachSelectedNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaExporterToolbox, fCountObject ) );

		const u32 numTotal = 
			mNumGeomObjectsSelected +
			mNumBoneObjectsSelected +
			mNumLightObjectsSelected +
			mNumMaterialsSelected
			;

		std::stringstream ss;

		if( numTotal == 0 )
		{
			ss << "Nothing selected.";
		}
		else
		{
			if( mNumGeomObjectsSelected > 0 )
				ss << "[" << mNumGeomObjectsSelected << " geom(s)] ";
			if( mNumBoneObjectsSelected > 0 )
				ss << "[" << mNumBoneObjectsSelected << " bone(s)] ";
			if( mNumLightObjectsSelected > 0 )
				ss << "[" << mNumLightObjectsSelected << " light(s)] ";
			if( mNumMaterialsSelected > 0 )
				ss << "[" << mNumMaterialsSelected << " material(s)] ";
			ss << "selected.";
		}

		SetStatusText( ss.str( ).c_str( ), 1 );
	}

	b32 tMayaExporterToolbox::fCountObject( MDagPath& path, MObject& component )
	{
		MFnDependencyNode fnNode( path.node( ) );

		if( path.hasFn( MFn::kMesh ) )
			++mNumGeomObjectsSelected;
		if( path.hasFn( MFn::kJoint ) )
			++mNumBoneObjectsSelected;
		else if( path.hasFn( MFn::kLight ) )
			++mNumLightObjectsSelected;
		else if( fnNode.hasObj( MFn::kSurfaceShader ) )
			++mNumMaterialsSelected;

		return true;
	}


}
