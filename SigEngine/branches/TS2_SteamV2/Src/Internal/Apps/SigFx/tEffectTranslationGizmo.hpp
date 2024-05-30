#ifndef __tEffectTranslationGizmo__
#define __tEffectTranslationGizmo__
#include "tManipulationGizmo.hpp"
#include "FX/tFxKeyframe.hpp"
#include "tSigFxKeyline.hpp"

namespace Sig
{
	class tEffectTranslationGizmo : public tManipulationGizmo
	{
		tSigFxKeyline* mTheKeyline;

	public:
		tEffectTranslationGizmo( const tGizmoGeometryPtr& geom, tSigFxKeyline* keyline );
		
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

		virtual void fOnDragStart( const tEditorSelectionList& selectedObjects );
		virtual void fOnDragEnd( );

		virtual void fComputeLocalToWorld( Math::tMat3f& xform, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const;
	};




	Math::tVec3f fComputeCenterPosition( const tEditorSelectionList& selectedObjects );

}

#endif//__tEffectTranslationGizmo__