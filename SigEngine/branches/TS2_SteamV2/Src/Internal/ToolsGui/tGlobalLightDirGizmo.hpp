//------------------------------------------------------------------------------
// \file tGlobalLightDirGizmo.hpp - 08 Oct 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGlobalLightDirGizmo__
#define __tGlobalLightDirGizmo__
#include "tRenderOnlyGizmo.hpp"

namespace Sig
{
	class tWxRenderPanel;

	class toolsgui_export tGlobalLightDirGizmo : public tRenderOnlyGizmo
	{
		Math::tVec3f mLightDir;

	public:
		tGlobalLightDirGizmo( const tGizmoGeometryPtr& geom );

		void fSetDirection( const Math::tVec3f& lightDir ) { mLightDir = lightDir; }

		virtual Math::tMat3f fComputeLocalToWorld( tToolsGuiMainWindow& editorWindow, const tWxRenderPanel* panel );

	private:
	};
	typedef tRefCounterPtr<tGlobalLightDirGizmo> tGlobalLightDirGizmoPtr;
}

#endif __tGlobalLightDirGizmo__
