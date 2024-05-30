#ifndef __tScaleGizmo__
#define __tScaleGizmo__
#include "tManipulationGizmo.hpp"

namespace Sig
{
	class toolsgui_export tScaleGizmo : public tManipulationGizmo
	{
	public:
		tScaleGizmo( const tGizmoGeometryPtr& geom );
		virtual void fSpecifyWorldCoords( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects,
			const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ );
		virtual void fExtractWorldCoords( 
			const tEditorSelectionList& selectedObjects,
			Math::tVec3f& worldCoords ) const;
		virtual Math::tVec3f fGetDefaultCoords( ) const { return Math::tVec3f( 1.f, 1.f, 1.f ); }
	protected:
		virtual void fOnDrag( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects, 
			const Math::tMat3f& gizmoLocalToWorld,
			const Math::tRayf& pickRay, 
			const Gfx::tCamera& camera,
			const Input::tMouse& mouse,
			const Input::tKeyboard& kb );
	};

}

#endif//__tScaleGizmo__

