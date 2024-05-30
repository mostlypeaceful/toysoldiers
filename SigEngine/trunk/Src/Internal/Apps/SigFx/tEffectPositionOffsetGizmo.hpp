#ifndef __tEffectPositionOffsetGizmo__
#define __tEffectPositionOffsetGizmo__
#include "tManipulationGizmo.hpp"
#include "FX/tFxKeyframe.hpp"
#include "tSigFxKeyline.hpp"
#include "tSigFxGraph.hpp"
#include "tFxEditableGizmoInfo.hpp"

namespace Sig
{
	class tEffectPositionOffsetGizmo : public tManipulationGizmo
	{
		tSigFxKeyline* mTheKeyline;
		b32 mPreviousState;
		b32 mInitialCollect;
		tEditorSelectionList::tOnSelectionChanged::tObserver mOnSelChanged;
				
		tGrowableArray< tFxEditableGizmoInfo > mFxEditables;
		tEditorActionPtr mUndoAction;
		
	public:
		tEffectPositionOffsetGizmo( const tGizmoGeometryPtr& geom, tSigFxKeyline* Keyline );
		
		virtual void fSpecifyWorldCoords( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects,
			const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ );
		virtual void fExtractWorldCoords( 
			const tEditorSelectionList& selectedObjects,
			Math::tVec3f& worldCoords ) const;

		virtual void fOnDragStart( const tEditorSelectionList& selectedObjects );
		virtual void fOnDragEnd( );
		virtual void fTick( tToolsGuiMainWindow& editorWindow, tEditorSelectionList& selectedObjects );

		void fOnSelChanged( tEditorSelectionList& selectedObjects );
		void fCollectEntities( const tEditorSelectionList& selectedObjects, b32 noNewKeyframes = false );

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

#endif	// __tEffectPositionOffsetGizmo__