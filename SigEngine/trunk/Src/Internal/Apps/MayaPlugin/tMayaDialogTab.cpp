#include "MayaPluginPch.hpp"
#include "tMayaDialogTab.hpp"
#include "MayaUtil.hpp"

namespace Sig
{

	tMayaDialogTab::tMayaDialogTab( wxWindow* parent, const char* label )
		: tWxSlapOnPanel( parent, label )
		, mAlwaysVisible( true )
	{
		fCommonCtor( label );
	}

	tMayaDialogTab::tMayaDialogTab( wxNotebook* parent, const char* label )
		: tWxSlapOnPanel( parent, label )
		, mAlwaysVisible( true )
	{
		fCommonCtor( label );
	}

	void tMayaDialogTab::fCommonCtor( const char* label )
	{
		GetSizer( )->AddSpacer( 12 );

		if( !_stricmp( label, "Main" ) )
		{
			mAlwaysVisible = true;
		}
		else if( !_stricmp( label, "Geometry" ) )
		{
			mAlwaysVisible = false;
			fSetMayaObjectType( MFn::kMesh );
		}
		else if( !_stricmp( label, "Animation" ) )
		{
			mAlwaysVisible = false;
			fSetMayaObjectType( MFn::kJoint );
		}
		else if( !_stricmp( label, "Light" ) )
		{
			mAlwaysVisible = false;
			fSetMayaObjectType( MFn::kLight );
		}
		else if( !_stricmp( label, "Material" ) )
		{
			mAlwaysVisible = false;
			fSetMayaObjectType( MFn::kSurfaceShader );
		}
		else if( !_stricmp( label, "Effect" ) )
		{
			mAlwaysVisible = false;
			fSetMayaObjectType( MFn::kParticleCloud );
		}

		if( !mAlwaysVisible )
		{
			mOnMayaSelChanged.fReset( new tMayaEvent( tMayaEvent::fEventNameSelChanged( ),
				make_delegate_memfn( tMayaEvent::tCallback, tMayaDialogTab, fOnMayaSelChanged ) ) );

			fOnMayaSelChanged( );
		}
	}

	void tMayaDialogTab::fOnMayaSelChanged( )
	{
		sigassert( !mAlwaysVisible );

		mNumOfMyTypeSelected = 0;

		const u32 numSelected = MayaUtil::fForEachSelectedNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaDialogTab, fVisitSelectedNode ) );
		if( ( mNumOfMyTypeSelected > 0 ) /*&& ( mNumOfMyTypeSelected == numSelected )*/ )
			fShow( );
		else
			fHide( );
	}

	b32 tMayaDialogTab::fVisitSelectedNode( MDagPath& path, MObject& component )
	{
		if( fIsNodeMyType( path ) )
			++mNumOfMyTypeSelected;

		return true;
	}

}
