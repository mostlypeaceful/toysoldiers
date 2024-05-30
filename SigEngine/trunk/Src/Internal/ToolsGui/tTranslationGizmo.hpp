#ifndef __tTranslationGizmo__
#define __tTranslationGizmo__
#include "tManipulationGizmo.hpp"

namespace Sig
{
	class toolsgui_export tTranslationGizmo : public tManipulationGizmo
	{
	public:
		tTranslationGizmo( const tGizmoGeometryPtr& geom );
		virtual void fSpecifyWorldCoords( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects,
			const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ );
		virtual void fExtractWorldCoords( 
			const tEditorSelectionList& selectedObjects,
			Math::tVec3f& worldCoords ) const;

		virtual void fComputeLocalToWorld( 
			Math::tMat3f& localToWorldOut, 
			const tEditorSelectionList& selectedObjects, 
			const Gfx::tCamera& camera, 
			f32 scale, 
			b32 orientToWorld ) const;
	protected:
		virtual void fOnDragStart( const tEditorSelectionList& selectedObjects );

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

#endif//__tTranslationGizmo__

