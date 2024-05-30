#ifndef __tManipulationGizmo__
#define __tManipulationGizmo__
#include "tGizmoGeometry.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "Gfx/tRenderableEntity.hpp"

namespace Sig
{
	class tManipulationGizmo;
	class tEditorSelectionList;

	namespace Input
	{
		class tMouse;
		class tKeyboard;
	}

	///
	/// \brief Represents an individual renderable instance of a gizmo object. Manages
	/// screen-space scaling of gizmo transform, in order to maintain constant size
	/// in screen-space as camera zooms in/out.
	class toolsgui_export tGizmoRenderable : public tRefCounter
	{
		tGizmoGeometryPtr				mGeometry;
		Gfx::tRenderableEntityPtr		mInstance;
	public:

		tGizmoRenderable( const tGizmoGeometryPtr& geom );

		const Math::tMat3f& fLocalToWorld( ) const { return mInstance->fObjectToWorld( ); }
		void fComputeLocalToWorld( const tManipulationGizmo& gizmo, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale = 1.f, b32 orientToWorld = false );
		Gfx::tDrawCall fCreateDrawCall( const Gfx::tCamera& camera ) const;
		const Gfx::tRenderableEntityPtr& fGizmoRenderableEntity( ) const { return mInstance; }
	};
	typedef tRefCounterPtr<tGizmoRenderable> tGizmoRenderablePtr;

	///
	/// \brief Core logic of gizmo manipulation, this class acts as the base type
	/// for the derived gizmos (translate, scale, rotate). This type represents a single
	/// logical gizmo object in the world; manages begin/end drag functionality.
	class toolsgui_export tManipulationGizmo : public tRefCounter
	{
	protected:
		tGizmoGeometryPtr mGizmoGeometry;
		b32 mDragging;
		b32 mX, mY, mZ;
		Math::tVec3f mReferencePoint;
		tEditorActionPtr mGizmoAction;

	public:
		tManipulationGizmo( const tGizmoGeometryPtr& geom );
		virtual ~tManipulationGizmo( ) { }

		const tGizmoGeometryPtr&	fGetGizmoGeometry( ) const { return mGizmoGeometry; }
		b32							fGetDragging( ) const { return mDragging; }

		virtual void fSpecifyWorldCoords( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects,
			const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ ) = 0;
		virtual void fExtractWorldCoords( 
			const tEditorSelectionList& selectedObjects,
			Math::tVec3f& worldCoords ) const = 0;

		virtual Math::tVec3f fGetDefaultCoords( ) const { return Math::tVec3f( 0.f, 0.f, 0.f ); }

		virtual void fUpdate( 
			tToolsGuiMainWindow& editorWindow,
			tEditorSelectionList& selectedObjects, 
			const Math::tMat3f& gizmoLocalToWorld,
			const Math::tRayf& pickRay, 
			const Gfx::tCamera& camera,
			const Input::tMouse& mouse,
			const Input::tKeyboard& kb );

		void fFinishDrag( );

		virtual void fTick( tToolsGuiMainWindow& editorWindow, tEditorSelectionList& selectedObjects ) { }
		virtual void fComputeLocalToWorld( Math::tMat3f& localToWorldOut, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const;

	protected:

		virtual void fOnDragStart( const tEditorSelectionList& selectedObjects ) { }
		virtual void fOnDragEnd( ) { }

		virtual void fOnDrag( 
			tToolsGuiMainWindow& mainWindow,
			const tEditorSelectionList& selectedObjects, 
			const Math::tMat3f& gizmoLocalToWorld,
			const Math::tRayf& pickRay, 
			const Gfx::tCamera& camera,
			const Input::tMouse& mouse,
			const Input::tKeyboard& kb ) = 0;

		static void fApplyCameraScaling( Math::tMat3f& xform, const Gfx::tCamera& camera, f32 scale );
	};
	typedef tRefCounterPtr<tManipulationGizmo> tManipulationGizmoPtr;

}



#endif//__tManipulationGizmo__

