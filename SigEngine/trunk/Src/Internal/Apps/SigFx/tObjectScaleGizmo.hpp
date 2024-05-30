#ifndef __tObjectScaleGizmo__
#define __tObjectScaleGizmo__
#include "tManipulationGizmo.hpp"
#include "tSigFxKeyline.hpp"

namespace Sig
{
	class tObjectScaleGizmo : public tManipulationGizmo
	{
		tEditorActionPtr mUndoAction;

	public:
		tObjectScaleGizmo( const tGizmoGeometryPtr& geom );
		virtual void fSpecifyWorldCoords( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects,
			const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ );
		virtual void fExtractWorldCoords( 
			const tEditorSelectionList& selectedObjects,
			Math::tVec3f& worldCoords ) const;
	protected:
		virtual void fOnDrag( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects, 
			const Math::tMat3f& gizmoLocalToWorld,
			const Math::tRayf& pickRay, 
			const Gfx::tCamera& camera,
			const Input::tMouse& mouse,
			const Input::tKeyboard& kb );
		virtual void fComputeLocalToWorld( Math::tMat3f& xform, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const;

	};

}

#endif	//__tObjectScaleGizmo__

