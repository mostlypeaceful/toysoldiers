#include "ToolsGuiPch.hpp"
#include "tTranslationGizmo.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tManipulationGizmoAction.hpp"
#include "Editor/tEditableObject.hpp"
#include "Math/tIntersectionRayPlane.hpp"
#include "tWxRenderPanelContainer.hpp"

namespace Sig
{

	namespace
	{
		Math::tVec3f fComputeTranslationDelta( 
			const Math::tRayf& pickRay,
			const Math::tVec3f& translationAxis,
			const Math::tVec3f& cameraLook,
			const Math::tVec3f& referencePoint )
		{
			const Math::tVec3f x = cameraLook.fCross( translationAxis ).fNormalizeSafe( );
			const Math::tVec3f n = translationAxis.fCross( x ).fNormalizeSafe( );

			if( !n.fIsZero( ) )
			{
				const Math::tPlanef plane( n, referencePoint );
				Math::tIntersectionRayPlane<f32> intersection( plane, pickRay );
				if( intersection.fIntersects( ) && !intersection.fParallel( ) )
				{
					const Math::tVec3f p = pickRay.fPointAtTime( intersection.fT( ) );
					return Math::fProjectToUnit( translationAxis, p - referencePoint );
				}
			}

			return Math::tVec3f::cZeroVector;
		}
	}


	tTranslationGizmo::tTranslationGizmo( const tGizmoGeometryPtr& geom )
		: tManipulationGizmo( geom )
	{
	}

	void tTranslationGizmo::fSpecifyWorldCoords( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects,
		const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ )
	{
		// create gizmo editor action, allowing for undo/redo
		tEditorActionPtr action( new tManipulationGizmoAction( mainWindow, selectedObjects ) );
		mainWindow.fGuiApp( ).fActionStack( ).fAddAction( action );

		// now explicitly set all transforms
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tEditableObject* eo = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
			Math::tVec3f t = eo->fObjectToWorld( ).fGetTranslation( );
			if( doX ) t.x = worldCoords.x;
			if( doY ) t.y = worldCoords.y;
			if( doZ ) t.z = worldCoords.z;
			eo->fMoveTo( t );
		}

		action->fEnd( );
	}

	void tTranslationGizmo::fExtractWorldCoords( 
		const tEditorSelectionList& selectedObjects,
		Math::tVec3f& worldCoords ) const
	{
		if( selectedObjects.fCount( ) > 0 )
		{
			const Math::tMat3f editorXform = selectedObjects[ 0 ]->fDynamicCast< tEditableObject >( )->fObjectToWorld( );
			worldCoords = editorXform.fGetTranslation( );
		}
	}

	void tTranslationGizmo::fComputeLocalToWorld( 
			Math::tMat3f& localToWorldOut, 
			const tEditorSelectionList& selectedObjects, 
			const Gfx::tCamera& camera, 
			f32 scale, 
			b32 orientToWorld ) const
	{
		if( Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonLCtrl ) || 
			Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonRCtrl ) ) 
			orientToWorld = !orientToWorld;

		return tManipulationGizmo::fComputeLocalToWorld( localToWorldOut, selectedObjects, camera, scale, orientToWorld );
	}

	void tTranslationGizmo::fOnDragStart( const tEditorSelectionList& selectedObjects )
	{
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tEditableObject* editable = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
			if( !editable->fSupportsTranslation( ) ) continue;
			editable->fTransition( ) = editable->fObjectToWorld( );
		}
	}

	void tTranslationGizmo::fOnDrag( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects, 
		const Math::tMat3f& gizmoLocalToWorld,
		const Math::tRayf& pickRay, 
		const Gfx::tCamera& camera,
		const Input::tMouse& mouse,
		const Input::tKeyboard& kb )
	{
		sigassert( mX || mY || mZ );

		Math::tVec3f translate = Math::tVec3f::cZeroVector;

		if( mX )
		{
			const Math::tVec3f delta = fComputeTranslationDelta( 
				pickRay, gizmoLocalToWorld.fXAxis( ).fNormalizeSafe( ), -camera.fZAxis( ), mReferencePoint );
			if( !delta.fIsZero( ) )
			{
				mReferencePoint += delta;
				translate		+= delta;
			}
		}
		if( mY )
		{
			const Math::tVec3f delta = fComputeTranslationDelta( 
				pickRay, gizmoLocalToWorld.fYAxis( ).fNormalizeSafe( ), -camera.fZAxis( ), mReferencePoint );
			if( !delta.fIsZero( ) )
			{
				mReferencePoint += delta;
				translate		+= delta;
			}
		}
		if( mZ )
		{
			const Math::tVec3f delta = fComputeTranslationDelta( 
				pickRay, gizmoLocalToWorld.fZAxis( ).fNormalizeSafe( ), -camera.fZAxis( ), mReferencePoint );
			if( !delta.fIsZero( ) )
			{
				mReferencePoint += delta;
				translate		+= delta;
			}
		}

		if( !translate.fIsZero( ) )
		{
			tWxRenderPanelContainer* gfx = mainWindow.fRenderPanelContainer( );
			tWxRenderPanel* panel = gfx->fGetActiveRenderPanel( );

			for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
			{
				tEditableObject* editable = selectedObjects[ i ]->fDynamicCast< tEditableObject >();
				if( !editable->fSupportsTranslation() || !editable->fIsTranslatable() ) continue;

				if( panel->fSnapToGrid( ) )
				{
					editable->fTransition( ).fTranslateGlobal( translate );
					Math::tVec3f pos = editable->fTransition( ).fGetTranslation( );
					panel->fSnapVertex( pos );
					editable->fMoveTo( pos );
				}
				else
					editable->fTranslate( translate );
			}

			selectedObjects.fSnapToGround( true );
		}
	}

}
