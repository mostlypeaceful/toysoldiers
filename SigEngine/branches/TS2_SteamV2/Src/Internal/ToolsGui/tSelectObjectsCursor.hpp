#ifndef __tSelectObjectsCursor__
#define __tSelectObjectsCursor__
#include "Editor/tEditableObjectContainer.hpp"
#include "tManipulationGizmo.hpp"

namespace Sig
{
	class toolsgui_export tFreezeHideAction : public tEditorButtonManagedCursorAction
	{
	public:
		tEditableObject::tState					mState;
		tEditableObjectContainer::tObjectSet	mOldSet;
		tEditableObjectContainer::tObjectSet	mNewSet;
	public:
		tFreezeHideAction( tToolsGuiMainWindow& mainWindow, tEditableObject::tState state );
		void fFinishConstruction( );
		virtual void fUndo( );
		virtual void fRedo( );
	};

	class toolsgui_export tModifySelectionAction : public tEditorButtonManagedCursorAction
	{
		tEditorSelectionList mUndoState;
		tEditorSelectionList mRedoState;
	public:
		tModifySelectionAction( tToolsGuiMainWindow& mainWindow, const tEditorSelectionList& prevState );
		virtual void fUndo( );
		virtual void fRedo( );
		virtual b32 fDirtyingAction( ) const { return false; }
	};


	class toolsgui_export tDeleteSelectedObjectsAction : public tEditorButtonManagedCursorAction
	{
		tEditorSelectionList mEntities;
	public:
		tDeleteSelectedObjectsAction( tToolsGuiMainWindow& mainWindow );
		virtual void fUndo( );
		virtual void fRedo( );
	};


	class tWxRenderPanel;


	///
	/// \brief Primary cursor object for the editor, facilitates gizmos and selection,
	/// duplicating, deleting. This is the brick and mortar of object interaction.
	class toolsgui_export tSelectObjectsCursor : public tEditorButtonManagedCursorController
	{
	protected:
		typedef tEditableObjectContainer::tEntityMasterList		tEntityIntersectionList;
		typedef tGrowableArray< tGizmoRenderablePtr >			tGizmoRenderableList;

		Gfx::tSolidColorLinesPtr		mSelectionRectangle;
		tManipulationGizmoPtr			mGizmo;
		tGizmoRenderableList			mGizmoPerWindow;
		b32								mIsGizmoInWorldSpace;
		f32								mGizmoScale;
		tWxRenderPanel*					mSelectionPanel;
		b32								mSelectionStarted;
		Math::tVec2f					mCursorSelectStart;
		Time::tStamp					mStartStamp;

	public:
		tSelectObjectsCursor( 
			tEditorCursorControllerButton* button,
			const char* statusText,
			const tManipulationGizmoPtr& gizmo = tManipulationGizmoPtr( ) );
		virtual ~tSelectObjectsCursor( );
		virtual void fOnTick( );
		virtual void fOnNextCursor( tEditorCursorController* nextController );

		inline b32 fHasGizmo( ) const { return !mGizmo.fNull( ); }

		void fToggleFreezeOnHoverObject( );

		///
		/// \brief Explicitly specify the world coords; this will be interpreted
		/// based on the current gizmo type (i.e., translation, rotation, or scale).
		void fSetWorldCoords( const Math::tVec3f& worldCoords, b32 doX = true, b32 doY = true, b32 doZ = true );
		Math::tVec3f fGetWorldCoords( );

		b32 fGetDefaultCoords( Math::tVec3f& coords );

		inline b32  fGetIsGizmoWorldSpace( ) const { return mIsGizmoInWorldSpace; }
		inline void fSetIsGizmoWorldSpace( b32 is ) { mIsGizmoInWorldSpace = is; }

		inline f32  fGetGizmoScale( ) const { return mGizmoScale; }
		inline void fSetGizmoScale( f32 s ) { mGizmoScale = fClamp( s, 0.1f, 10.f ); }

	private:
		void fClearSelection( );
		virtual b32 fDoToolTipOverHoverObject( );
		void fHandleSelectionMouseDown( tWxRenderPanel* panel, const Input::tMouse& mouse, const Input::tKeyboard& kb );
		void fHandleSelectionMouseUp( tWxRenderPanel* panel, const Input::tMouse& mouse, const Input::tKeyboard& kb );
		void fUpdateSelectionDrag( tWxRenderPanel* panel, const Input::tMouse& mouse, const Input::tKeyboard& kb );
		void fAcquireIntersectedEntities( const Input::tMouse& mouse, tEntityIntersectionList& intersectedEntities );
	protected:
		void fHandleSelection( );
		void fHandleDelete( );
		void fHandleGizmo( );
	};

	typedef tRefCounterPtr< tSelectObjectsCursor > tSelectObjectsCursorPtr;

}

#endif//__tSelectObjectsCursor__
