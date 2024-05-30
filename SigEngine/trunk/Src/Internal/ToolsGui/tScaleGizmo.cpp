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
					const Math::tVec3f p = pickRay.fPointAtTime( intersection.fT( ) );
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
				if( eo->fUniformScaleInXZ( ) )
				{
					if( !fEqual( scale.x, worldCoords.x ) )
						scale = worldCoords.x;
					else if( !fEqual( scale.z, worldCoords.z ) )
						scale = worldCoords.z;

					scale.y = worldCoords.y;
				}
				else
				{
					if( !fEqual( scale.x, worldCoords.x ) )
						scale = worldCoords.x;
					else if( !fEqual( scale.y, worldCoords.y ) )
						scale = worldCoords.y;
					else if( !fEqual( scale.z, worldCoords.z ) )
						scale = worldCoords.z;
				}
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
		scale = fHandleLockedAxes( scale, maxMag );

		Math::tAabbf biggestObjBox;
		Math::tVec3f biggestBoxDims;
		const f32 maxBoxDim = fDetermineLargestScaleFactor( selectedObjects, biggestObjBox, biggestBoxDims );

		// scale each selected object
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tEditableObject* editable = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
			if( !editable->fSupportsScale( ) ) continue;

			Math::tVec3f boxDims( biggestBoxDims );

			Math::tVec3f tempScale;
			if( editable->fUniformScaleOnly( ) )
			{
				if( editable->fUniformScaleInXZ( ) )
				{
					const f32 max = scale.fXZ( ).fMax( );
					const f32 min = scale.fXZ( ).fMin( );
					tempScale = ( max == 0.f ) ? min : max;
					tempScale.y = scale.y;

					boxDims = maxBoxDim;
					boxDims.y = biggestBoxDims.y;
				}
				else
				{
					const f32 max = scale.fMax( );
					const f32 min = scale.fMin( );
					tempScale = ( max == 0.f ) ? min : max;

					// Consider the box to be max dim. Prevents uniform scale from
					// causing uneven scaling on each axis.
					boxDims = maxBoxDim;
				}
			}
			else
				tempScale = scale;

			// Adjust this object's scale speed based on the scale factor.
			tempScale /= boxDims;

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

	Math::tVec3f tScaleGizmo::fHandleLockedAxes( const Math::tVec3f& vec, const f32 maximum ) const
	{
		Math::tVec3f ret = vec;

		if( mX && mY && mZ )
			ret = Math::tVec3f( maximum );
		else if( mX && mY )
		{
			ret.x = maximum;
			ret.y = maximum;
		}
		else if( mX && mZ )
		{
			ret.x = maximum;
			ret.z = maximum;
		}
		else if( mY && mZ )
		{
			ret.y = maximum;
			ret.z = maximum;
		}

		return ret;
	}

	// This finds the biggest union of all selected objects and uses that to scale the rate at which the gizmo scales.
	f32 tScaleGizmo::fDetermineLargestScaleFactor( const tEditorSelectionList& selectedObjects, Math::tAabbf& outObjBox, Math::tVec3f& outScaleFactor ) const
	{
		outObjBox = Math::tAabbf( Math::tVec3f( -0.01f ), Math::tVec3f( 0.01f ) );

		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tEditableObject* editable = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
			if( !editable->fSupportsScale( ) ) continue;

			// Get this object's box to investigate.
			Math::tAabbf insideBox = editable->fObjectSpaceBox( );
			const f32 maxBoxDim = insideBox.fMaxAxisLength( );

			if( editable->fUniformScaleOnly() )
			{
				if( editable->fUniformScaleInXZ() )
				{
					// Use the max for X/Z and preserve the box height.
					const f32 halfMax = maxBoxDim / 2.f;
					Math::tVec3f description( halfMax, insideBox.fHeight() / 2.f, halfMax );
					insideBox = Math::tAabbf( -description, description );
				}
				else
				{
					// Consider the box to be max dim. Prevents uniform scale from
					// causing uneven scaling on each axis.
					const f32 halfMax = maxBoxDim / 2.f;
					insideBox = Math::tAabbf( Math::tVec3f(-halfMax), Math::tVec3f(halfMax) );
				}
			}

			// Keep the union of the boxes so far.
			outObjBox |= insideBox;
		}

		outScaleFactor = Math::tVec3f( outObjBox.fWidth(), outObjBox.fHeight(), outObjBox.fDepth() );

		// Ensure that none of our scale factors are zero.
		if( outScaleFactor.x == 0.f )
			outScaleFactor.x = 1.f;

		if( outScaleFactor.y == 0.f )
			outScaleFactor.y = 1.f;

		if( outScaleFactor.z == 0.f )
			outScaleFactor.z = 1.f;

		// NOTE: Making a note here that this isn't using the same sign-preservation way that the
		// scale vector handles locked axes, but ideally (is this true?) our object boxes are not
		// going to have negative widths.
		outScaleFactor = fHandleLockedAxes( outScaleFactor, outScaleFactor.fMaxMagnitude() );

		return outObjBox.fMaxAxisLength();
	}

}
