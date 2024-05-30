#ifndef __tManipulationGizmoAction__
#define __tManipulationGizmoAction__
#include "tEditorCursorControllerButton.hpp"
#include "Editor/tEditorSelectionList.hpp"

namespace Sig
{
	///
	/// \brief Represents an undoable/redoable gizmo action (i.e., translate, rotate, scale).
	/// Used internally by base gizmo and derived gizmos.
	class toolsgui_export tManipulationGizmoAction : public tEditorButtonManagedCursorAction
	{
		tEditorSelectionList mSelectedObjects;
		tDynamicArray< Math::tMat3f > mUndoXforms;
		tDynamicArray< Math::tMat3f > mRedoXforms;
	public:
		tManipulationGizmoAction( tToolsGuiMainWindow& mainWindow, const tEditorSelectionList& selectedObjects );
		virtual void fEnd( );
		virtual void fUndo( );
		virtual void fRedo( );
	private:
		void fSaveXforms( tDynamicArray< Math::tMat3f >& xforms );
		void fResetXforms( const tDynamicArray< Math::tMat3f >& xforms );
	};

}


#endif//__tManipulationGizmoAction__
