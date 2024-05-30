#include "ToolsGuiPch.hpp"
#include "tManipulationGizmo.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tManipulationGizmoAction.hpp"
#include "Editor/tEditableObject.hpp"
#include "Input/tMouse.hpp"

namespace Sig
{
	tGizmoRenderable::tGizmoRenderable( const tGizmoGeometryPtr& geom )
		: mGeometry( geom )
		, mInstance( new Gfx::tRenderableEntity( mGeometry->fGetRenderBatch( ) ) )
	{
	}
	void tGizmoRenderable::fComputeLocalToWorld( const tManipulationGizmo& gizmo, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld )
	{
		Math::tMat3f xform;
		gizmo.fComputeLocalToWorld( xform, selectedObjects, camera, scale, orientToWorld );
		mInstance->fMoveTo( xform );
	}
	Gfx::tDrawCall tGizmoRenderable::fCreateDrawCall( const Gfx::tCamera& camera ) const
	{
		return mInstance->fGetDrawCall( camera );
	}




	void tDuplicateObjectsAction::fUndo( )
	{
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( mSavedSelection );
		for( u32 i = 0; i < mNewEntities.fCount( ); ++i )
			mNewEntities[ i ]->fDynamicCast< tEditableObject >( )->fRemoveFromWorld( );
	}
	void tDuplicateObjectsAction::fRedo( )
	{
		for( u32 i = 0; i < mNewEntities.fCount( ); ++i )
			mNewEntities[ i ]->fDynamicCast< tEditableObject >( )->fAddToWorld( );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( mNewEntities );
	}




	tManipulationGizmo::tManipulationGizmo( const tGizmoGeometryPtr& geom )
		: mGizmoGeometry( geom ) 
		, mDragging( false )
		, mDuplicated( false )
		, mX( false )
		, mY( false )
		, mZ( false )
		, mReferencePoint( Math::tVec3f::cZeroVector )
		, mOrientToWorld( false )
	{
	}

	void tManipulationGizmo::fUpdate( 
		tToolsGuiMainWindow& mainWindow,
		tEditorSelectionList& selectedObjects, 
		const Math::tMat3f& gizmoLocalToWorld,
		const Math::tRayf& pickRay, 
		const Gfx::tCamera& camera, 
		const Input::tMouse& mouse,
		const Input::tKeyboard& kb )
	{
		if( mDragging )
		{
			if( mouse.fButtonHeld( Input::tMouse::cButtonLeft ) )
			{
				// still dragging, update delta and move objects
				fOnDrag( mainWindow, selectedObjects, gizmoLocalToWorld, pickRay, camera, mouse, kb );
			}
			else
			{
				if( !mDuplicated )
				{
					// It's a normal translation.
					mainWindow.fGuiApp( ).fActionStack( ).fAddAction( mGizmoAction );
				}
				else if( mouse.fButtonDragging( Input::tMouse::cButtonLeft ) )
				{
					// Only add duplications if they have been dragged far enough.
					mainWindow.fGuiApp( ).fActionStack( ).fAddAction( mGizmoAction );
				}
				else
				{
					// Undo any failed duplications.
					mGizmoAction->fUndo( );
				}
				fFinishDrag( );
			}
		}
		else if( mouse.fButtonDown( Input::tMouse::cButtonLeft ) )
		{
			const Math::tMat3f worldToLocal = gizmoLocalToWorld.fInverse( );
			const Math::tRayf pickRayInLocal = pickRay.fTransform( worldToLocal );

			mGizmoGeometry->fRayCast( pickRayInLocal, mReferencePoint, mX, mY, mZ );
			if( mX || mY || mZ )
			{
				// we intersected some part of the gizmo, begin drag operation
				mDragging = true;

				// store the reference point (basically the intersection of the pick ray with the gizmo);
				// this is useful for constraining the motion to the plane of intersection
				mReferencePoint = gizmoLocalToWorld.fXformPoint( mReferencePoint );

				mDuplicated = 
					kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) ||
					kb.fButtonHeld( Input::tKeyboard::cButtonRShift );

				// create editor action(s) allowing for undo/redo
				if( mDuplicated )
				{
					mGizmoAction.fReset( new tDuplicateObjectsAction( mainWindow, selectedObjects ) );
				}
				else
					mGizmoAction.fReset( new tManipulationGizmoAction( mainWindow, selectedObjects ) );

				fOnDragStart( selectedObjects );
			}
		}
	}

	void tManipulationGizmo::fFinishDrag( )
	{
		// finished dragging
		mDragging = false;

		// tell action we're done, and release my ref count
		sigassert( !mGizmoAction.fNull( ) );
		mGizmoAction->fEnd( );
		mGizmoAction.fRelease( );

		fOnDragEnd( );
	}

	void tManipulationGizmo::fComputeLocalToWorld( Math::tMat3f& xform, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const
	{
		if( orientToWorld || selectedObjects.fCount( ) > 1 )
		{
			const Math::tVec3f avgPos = selectedObjects.fComputeAveragePosition( );
			xform = Math::tMat3f::cIdentity;
			xform.fSetTranslation( avgPos );
		}
		else if( selectedObjects.fCount( ) == 1 )
		{
			xform = selectedObjects[ 0 ]->fDynamicCast< tEditableObject >( )->fObjectToWorld( );
			xform.fNormalizeBasis( );
		}
		else
		{
			xform = Math::tMat3f::cIdentity;
		}

		fApplyCameraScaling( xform, camera, scale );
	}

	void tManipulationGizmo::fApplyCameraScaling( Math::tMat3f& xform, const Gfx::tCamera& camera, f32 scale )
	{
		const f32 depth = camera.fCameraDepth( xform.fGetTranslation( ) );
		xform.fScaleLocal( Math::tVec3f::cOnesVector * ( depth / ( 20.f * scale ) ) );
	}

}
