#include "BasePch.hpp"
#include "tCanvasFrameOutline.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig { namespace Gui
{
	devvar( bool, Debug_UI_CanvasFrameOutlines, false );
	devvar( bool, Debug_UI_CanvasFrameOutlinesTestHierDepth, false );
	devvar( s32, Debug_UI_CanvasFrameOutlinesHierDepth, 3 );

	b32 tCanvasFrameOutline::fDebugDrawEnabled( )
	{
		return Debug_UI_CanvasFrameOutlines;
	}

	tCanvasFrameOutline::tCanvasFrameOutline( )
	{
		fRI_SetObjectToWorld( &Math::tMat3f::cIdentity ); //important! prevents verts from being transformed by my parents worldxform
	}

	void tCanvasFrameOutline::fOnMoved( )
	{
		tLineList::fOnMoved( );
		fUpdateVerts( );
	}

	void tCanvasFrameOutline::fOnParentMoved( )
	{
		tLineList::fOnParentMoved( );
		fUpdateVerts( );
	}

	void tCanvasFrameOutline::fOnTintChanged( )
	{
		tLineList::fOnTintChanged( );
		fUpdateVerts( );
	}

	void tCanvasFrameOutline::fOnParentTintChanged( )
	{
		tLineList::fOnParentTintChanged( );
		fUpdateVerts( );
	}

	void tCanvasFrameOutline::fUpdateVerts( )
	{
		if( Debug_UI_CanvasFrameOutlinesTestHierDepth && fParent( ) )
		{
			int depth = 0;			
			tCanvasFrame* parent = fParent( );
			while( parent )
			{
				++depth;
				parent = parent->fParent( );
			}
			if( depth != Debug_UI_CanvasFrameOutlinesHierDepth )
			{
				tGrowableArray<Gfx::tSolidColorRenderVertex> verts;
				fSetGeometry( verts );
				return;
			}
		}

		tGrowableArray<Gfx::tSolidColorRenderVertex> verts( 8 );
		const float zDepth = 1.0f;
		const u32 clr = 0xffffffff;
		const Math::tRect& r = fParent( )->fWorldXform( ).mRect;

		const Math::tVec3f tL( r.mL, r.mT, zDepth );
		const Math::tVec3f tR( r.mR, r.mT, zDepth );
		const Math::tVec3f bR( r.mR, r.mB, zDepth );
		const Math::tVec3f bL( r.mL, r.mB, zDepth );

		verts[ 0 ] = Gfx::tSolidColorRenderVertex( tL, clr );
		verts[ 1 ] = Gfx::tSolidColorRenderVertex( tR, clr );

		verts[ 2 ] = Gfx::tSolidColorRenderVertex( tR, clr );
		verts[ 3 ] = Gfx::tSolidColorRenderVertex( bR, clr );

		verts[ 4 ] = Gfx::tSolidColorRenderVertex( bR, clr );
		verts[ 5 ] = Gfx::tSolidColorRenderVertex( bL, clr );

		verts[ 6 ] = Gfx::tSolidColorRenderVertex( bL, clr );
		verts[ 7 ] = Gfx::tSolidColorRenderVertex( tL, clr );


		fSetGeometry( verts );
	}

}};
