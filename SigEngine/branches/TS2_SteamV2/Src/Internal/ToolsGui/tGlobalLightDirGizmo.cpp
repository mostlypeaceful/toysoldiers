//------------------------------------------------------------------------------
// \file tGlobalLightDirGizmo.cpp - 08 Oct 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsGuiPch.hpp"
#include "tGlobalLightDirGizmo.hpp"
#include "tWxRenderPanel.hpp"
#include "tToolsGuiApp.hpp"
#include "Editor\tEditorSelectionList.hpp"

namespace Sig
{
	tGlobalLightDirGizmo::tGlobalLightDirGizmo( const tGizmoGeometryPtr& geom )
		: tRenderOnlyGizmo( geom )
		, mLightDir( 1.f, 0.f, 0.f )
	{
		mScreenScale = 0.03f;
		mWorldScale = 5.f;

		mGizmoRelativeScreenPos = Math::tVec2f( 0.95f, 0.05f );
	}

	Math::tMat3f tGlobalLightDirGizmo::fComputeLocalToWorld( tToolsGuiMainWindow& editorWindow, const tWxRenderPanel* panel )
	{
		mIsScreenSpace = editorWindow.fGuiApp( ).fSelectionList( ).fCount( ) == 0;

		Math::tMat3f ret = tRenderOnlyGizmo::fComputeLocalToWorld( editorWindow, panel );
		ret.fOrientXAxis( mLightDir );

		// Rotating resets the scaling.
		if( mIsScreenSpace )
		{
			if( panel->fGetScreen( )->fViewport( 0 )->fLogicCamera( ).fGetLens( ).mProjectionType != Gfx::tLens::cProjectionOrtho )
				ret.fScaleLocal( Math::tVec3f( mScreenScale, mScreenScale, mScreenScale ) );
		}
		else
		{
			ret.fSetTranslation( ret.fGetTranslation( ) + mLightDir * -30.f );
			ret.fScaleLocal( Math::tVec3f( mWorldScale, mWorldScale, mWorldScale ) );
		}

		return ret;
	}
}
