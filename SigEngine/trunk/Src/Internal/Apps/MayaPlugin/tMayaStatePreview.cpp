#include "MayaPluginPch.hpp"
#include "tMayaStatePreview.hpp"
#include "tExporterToolbox.hpp"
#include "MayaUtil.hpp"
#include "tWxSlapOnButton.hpp"

namespace Sig
{
	tMayaStatePreview::tMayaStatePreview( wxWindow* parent, const char* label )
		: tWxSlapOnSpinner( parent, label, -1, 256, 1, 0 )
	{
		fSetValueNoEvent( -1 );
		wxButton* button = new wxButton( parent, wxID_ANY, "All", wxDefaultPosition, wxSize( 22, 20 ) );
		fAddWindowToSizer( button, true );
		button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tMayaStatePreview::fShowAll ), NULL, this );
	}

	void tMayaStatePreview::fShowAll( wxCommandEvent& )
	{
		fSetValue( -1 );
	}

	void tMayaStatePreview::fOnControlUpdated( )
	{
		mShowList.clear( );
		mHideList.clear( );

		MayaUtil::fForEachTransformNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaStatePreview, fVisitNode ) );

		log_line( 0, "Hide: " << mHideList.length( ) << " Show: " << mShowList.length( ) );
		MGlobal::setActiveSelectionList( mHideList );
		MGlobal::executeCommand( MString( "HideSelectedObjects;" ) );
		MGlobal::setActiveSelectionList( mShowList );
		MGlobal::executeCommand( MString( "ShowSelectedObjects;" ) );
		MGlobal::clearSelectionList( );
	}

	b32 tMayaStatePreview::fVisitNode( MDagPath& path, MObject& component )
	{
		MFnDagNode nodeFn( path );
		MObject mobject = path.node( );

		if( path.hasFn( MFn::kMesh ) )
		{
			s32 stateMaskTemp = 0;	
			if( MayaUtil::fGetIntAttribute( mobject, nodeFn, stateMaskTemp, tExporterToolbox::fNameStateMask( ) ) )
			{
				u32 stateMask = stateMaskTemp;
				if( fGetValue( ) == -1 || (stateMask & (1<<(s32)fGetValue( ))) )
					mShowList.add( path, component );
				else
					mHideList.add( path, component );
			}
		}

		return true;
	}
}
