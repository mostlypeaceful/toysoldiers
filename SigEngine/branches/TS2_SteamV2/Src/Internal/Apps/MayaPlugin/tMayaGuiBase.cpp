#include "MayaPluginPch.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{
	tMayaGuiBase::tMayaGuiBase( )
		: mMayaObjectType( MFn::kBase )
	{
	}

	b32 tMayaGuiBase::fIsNodeMyType( MDagPath& path ) const
	{
		if( mMayaObjectType == MFn::kSurfaceShader )
		{
			// special check for surface shader types

			if( path.hasFn( MFn::kTransform ) )
				return false; // transform types are not materials

			// now see if the node implements surface shader
			MObject pathNode = path.node( );
			MFnDependencyNode fnNode( pathNode );
			return fnNode.hasObj( MFn::kSurfaceShader ) != 0;
		}

		return path.hasFn( mMayaObjectType ) != 0;
	}

	tMayaAttributeControlBase::tMayaAttributeControlBase( tMayaGuiBase* parent )
	{
		mOnMayaSelChanged.fReset( new tMayaEvent( 
			tMayaEvent::fEventNameSelChanged( ), 
			make_delegate_memfn( tMayaEvent::tCallback, tMayaAttributeControlBase, fOnMayaSelChanged ) ) );

		if( parent )
			fSetMayaObjectType( parent->fGetMayaObjectType( ) );
	}

	void tMayaAttributeControlBase::fOnMayaSelChanged( )
	{
	}

}
