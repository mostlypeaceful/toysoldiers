//------------------------------------------------------------------------------
// \file tRenderOnlyGizmo.cpp - 07 Oct 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsGuiPch.hpp"
#include "tRenderOnlyGizmo.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tToolsGuiApp.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "Editor/tEditableObject.hpp"

namespace Sig
{
	tRenderOnlyRenderable::tRenderOnlyRenderable( const tGizmoGeometryPtr& geom )
		: mGeometry( geom )
		, mInstance( new Gfx::tRenderableEntity( mGeometry->fGetRenderBatch( ) ) )
	{
	}
	void tRenderOnlyRenderable::fSetLocalToWorld( const Math::tMat3f& localToWorld )
	{
		mInstance->fMoveTo( localToWorld );
	}
	Gfx::tDrawCall tRenderOnlyRenderable::fCreateDrawCall( const Gfx::tCamera& camera ) const
	{
		return mInstance->fGetDrawCall( camera );
	}


	tRenderOnlyGizmo::tRenderOnlyGizmo( const tGizmoGeometryPtr& geom )
		: mGizmoGeometry( geom )
		, mActive( true )
		, mScreenScale( 1.f )
		, mWorldScale( 1.f )
		, mIsScreenSpace( true )
		, mGizmoRelativeScreenPos( 0.5f, 0.5f )
		, mPushForward( 1.f )
	{
	}

	void tRenderOnlyGizmo::fTick( tToolsGuiMainWindow& editorWindow )
	{
		if( !mActive )
			return;

		tWxRenderPanelContainer* gfx = editorWindow.fRenderPanelContainer( );

		// we need one gizmo renderable per viewable window, so make sure we have enough
		if( mGizmoPerWindow.fCount( ) != gfx->fRenderPanelCount( ) )
			mGizmoPerWindow.fSetCount( gfx->fRenderPanelCount( ) );

		// now go through each render panel window and adjust the gizmo for rendering in that viewport;
		// the reason for this messiness is that the gizmo needs to scale differently for each viewport,
		// requiring different renderable instances for each one
		u32 i = 0;
		for( tWxRenderPanel** ipanel = gfx->fRenderPanelsBegin( ); ipanel != gfx->fRenderPanelsEnd( ); ++ipanel, ++i )
		{
			if( !*ipanel || !(*ipanel)->fIsVisible( ) )
				continue;
			if( mGizmoPerWindow[ i ].fNull( ) )
				mGizmoPerWindow[ i ].fReset( new tRenderOnlyRenderable( fGetGizmoGeometry( ) ) );

			tWxRenderPanel* panel = *ipanel;

			// compute the local-to-world xform for this instance of the gizmo
			mGizmoPerWindow[ i ]->fSetLocalToWorld( fComputeLocalToWorld( editorWindow, panel ) );
			// add the draw call
			panel->fGetScreen( )->fAddWorldSpaceTopDrawCall( mGizmoPerWindow[ i ]->fGizmoRenderableEntity( ) );
		}
	}

	Math::tMat3f tRenderOnlyGizmo::fComputeLocalToWorld( tToolsGuiMainWindow& editorWindow, const tWxRenderPanel* panel )
	{
		Math::tMat3f ret( Math::tMat3f::cIdentity );

		if( mIsScreenSpace )
		{
			const Gfx::tCamera& camera = panel->fGetScreen( )->fViewport( 0 )->fLogicCamera( );

			Math::tVec2u windowRes( panel->GetSize( ).x, panel->GetSize( ).y );
			Math::tVec2u windowPos( mGizmoRelativeScreenPos.x * windowRes.x, mGizmoRelativeScreenPos.y * windowRes.y );
			Math::tMat4f projToWorld = camera.fGetWorldToProjection( ).fInverse( );
			Math::tVec3f worldPos = camera.fUnproject( windowPos, windowRes, projToWorld );

			if( camera.fGetLens( ).mProjectionType == Gfx::tLens::cProjectionPersp )
			{
				worldPos += (worldPos - camera.fGetTripod( ).mEye).fNormalize( ) * mPushForward;
				ret.fScaleLocal( Math::tVec3f( mScreenScale, mScreenScale, mScreenScale ) );
			}
			else
			{
				worldPos += camera.fZAxis( ) * mPushForward;
			}

			ret.fSetTranslation( worldPos );
		}
		else
		{
			tEditorSelectionList& selectedObjects = editorWindow.fGuiApp( ).fSelectionList( );

			if( selectedObjects.fCount( ) > 1 )
			{
				const Math::tVec3f avgPos = selectedObjects.fComputeAveragePosition( );
				ret.fSetTranslation( avgPos );
			}
			else if( selectedObjects.fCount( ) == 1 )
			{
				ret = selectedObjects[ 0 ]->fDynamicCast< tEditableObject >( )->fObjectToWorld( );
				ret.fNormalizeBasis( );
			}

			ret.fScaleLocal( Math::tVec3f( mWorldScale, mWorldScale, mWorldScale ) );
		}

		return ret;
	}
}
