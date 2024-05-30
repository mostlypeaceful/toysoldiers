#include "ToolsGuiPch.hpp"
#include "tScaleGizmo.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tManipulationGizmoAction.hpp"
#include "Editor/tEditableObject.hpp"
#include "Math/tIntersectionRayPlane.hpp"

namespace Sig
{
	namespace
	{
		Math::tVec3f fComputeScaleDelta( 
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
					const Math::tVec3f p = pickRay.fEvaluate( intersection.fT( ) );
					return Math::fProjectToUnit( translationAxis, p - referencePoint );
				}
			}

			return Math::tVec3f::cZeroVector;
		}
	}


	tScaleGizmo::tScaleGizmo( const tGizmoGeometryPtr& geom )
		: tManipulationGizmo( geom )
	{
	}

	void tScaleGizmo::fSpecifyWorldCoords( 
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
			Math::tMat3f editorXform = eo->fObjectToWorld( );

			Math::tVec3f scale = editorXform.fGetScale( );
			editorXform.fScaleLocal( 1.f / scale );

			if( eo->fUniformScaleOnly( ) )
			{
				if( !fEqual( scale.x, worldCoords.x ) )
					scale = worldCoords.x;
				else if( !fEqual( scale.y, worldCoords.y ) )
					scale = worldCoords.y;
				else if( !fEqual( scale.z, worldCoords.z ) )
					scale = worldCoords.z;
			}
			else
			{
				if( doX ) scale.x = worldCoords.x;
				if( doY ) scale.y = worldCoords.y;
				if( doZ ) scale.z = worldCoords.z;
			}

			editorXform.fScaleLocal( scale );
			eo->fMoveTo( editorXform );
		}

		action->fEnd( );
	}
	
	void tScaleGizmo::fExtractWorldCoords( 
		const tEditorSelectionList& selectedObjects,
		Math::tVec3f& worldCoords ) const
	{
		if( selectedObjects.fCount( ) > 0 )
		{
			const Math::tMat3f editorXform = selectedObjects[ 0 ]->fDynamicCast< tEditableObject >( )->fObjectToWorld( );
			worldCoords = editorXform.fGetScale( );
		}
	}

	void tScaleGizmo::fOnDrag( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects, 
		const Math::tMat3f& gizmoLocalToWorld,
		const Math::tRayf& pickRay, 
		const Gfx::tCamera& camera,
		const Input::tMouse& mouse,
		const Input::tKeyboard& kb )
	{
		sigassert( mX || mY || mZ );

		Math::tVec3f scale = Math::tVec3f::cZeroVector;

		if( mX )
		{
			const Math::tVec3f axis = gizmoLocalToWorld.fXAxis( ).fNormalizeSafe( );
			const Math::tVec3f delta = fComputeScaleDelta( 
				pickRay, axis, -camera.fZAxis( ), mReferencePoint );
			mReferencePoint += delta;
			scale.x			+= delta.fLength( ) * fSign( delta.fDot( axis ) );
		}
		if( mY )
		{
			const Math::tVec3f axis = gizmoLocalToWorld.fYAxis( ).fNormalizeSafe( );
			const Math::tVec3f delta = fComputeScaleDelta( 
				pickRay, axis, -camera.fZAxis( ), mReferencePoint );
			mReferencePoint += delta;
			scale.y			+= delta.fLength( ) * fSign( delta.fDot( axis ) );
		}
		if( mZ )
		{
			const Math::tVec3f axis = gizmoLocalToWorld.fZAxis( ).fNormalizeSafe( );
			const Math::tVec3f delta = fComputeScaleDelta( 
				pickRay, axis, -camera.fZAxis( ), mReferencePoint );
			mReferencePoint += delta;
			scale.z			+= delta.fLength( ) * fSign( delta.fDot( axis ) );
		}

		// find the component with the highest magnitude, but preserve its sign
		f32 maxMag = 0.f;
		for( u32 iaxis = 0; iaxis < 3; ++iaxis )
			if( fAbs( scale.fAxis( iaxis ) ) > fAbs( maxMag ) )
				maxMag = scale.fAxis( iaxis );

		// handle locked/combined axes
		if( mX && mY && mZ )
			scale = Math::tVec3f( maxMag );
		else if( mX && mY )
		{
			scale.x = maxMag;
			scale.y = maxMag;
		}
		else if( mX && mZ )
		{
			scale.x = maxMag;
			scale.z = maxMag;
		}
		else if( mY && mZ )
		{
			scale.y = maxMag;
			scale.z = maxMag;
		}

		// scale each selected object
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tEditableObject* editable = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
			if( !editable->fSupportsScale( ) ) continue;

			Math::tVec3f tempScale;
			if( editable->fUniformScaleOnly( ) )
			{
				const f32 max = scale.fMax( );
				const f32 min = scale.fMin( );
				tempScale = ( max == 0.f ) ? min : max;
			}
			else
				tempScale = scale;

			Math::tMat3f xform = editable->fObjectToWorld( );

			Math::tVec3f objScale = xform.fGetScale( );
			xform.fScaleLocal( 1.f / objScale );
			objScale += tempScale;

			const f32 minScale = 0.01f;
			const f32 maxScale = 9999999999.f;
			objScale.x = fClamp( objScale.x, minScale, maxScale );
			objScale.y = fClamp( objScale.y, minScale, maxScale );
			objScale.z = fClamp( objScale.z, minScale, maxScale );

			xform.fScaleLocal( objScale );
			editable->fMoveTo( xform );
		}
	}

}
