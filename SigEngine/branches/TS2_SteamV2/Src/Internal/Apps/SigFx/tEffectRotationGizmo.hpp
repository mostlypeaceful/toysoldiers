#ifndef __tEffectRotationGizmo__
#define __tEffectRotationGizmo__
#include "tManipulationGizmo.hpp"
#include "tSigFxKeyline.hpp"

namespace Sig
{
	class tEffectRotationGizmo : public tManipulationGizmo
	{
		Math::tMat3f mReferenceFrame;

	public:
		tEffectRotationGizmo( const tGizmoGeometryPtr& geom );
		virtual void fSpecifyWorldCoords( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects,
			const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ );
		virtual void fExtractWorldCoords( 
			const tEditorSelectionList& selectedObjects,
			Math::tVec3f& worldCoords ) const;
	protected:
		virtual void fComputeLocalToWorld( Math::tMat3f& localToWorldOut, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const;

		virtual void fOnDragStart( const tEditorSelectionList& selectedObjects );

		virtual void fOnDrag( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects, 
			const Math::tMat3f& gizmoLocalToWorld,
			const Math::tRayf& pickRay, 
			const Gfx::tCamera& camera,
			const Input::tMouse& mouse,
			const Input::tKeyboard& kb );
		Math::tVec3f fApplyRotationDelta( 
			const tEditorSelectionList& selectedObjects, 
			const Math::tRayf& pickRay,
			u32 iaxis,
			const Math::tVec3f& planeNormal,
			const Math::tVec3f& rotationCenter,
			const Math::tVec3f& referencePoint );
	};
}

#endif//__tEffectRotationGizmo__

