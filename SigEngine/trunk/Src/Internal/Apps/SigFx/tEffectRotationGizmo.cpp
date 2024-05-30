#include "SigFxPch.hpp"
#include "tEffectRotationGizmo.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tManipulationGizmoAction.hpp"
#include "Editor/tEditableObject.hpp"
#include "Math/tIntersectionRayPlane.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "tFxEditorActions.hpp"
#include "tEffectTranslationGizmo.hpp"		// for fComputeCenterPosition( )

namespace Sig
{

	tEffectRotationGizmo::tEffectRotationGizmo( const tGizmoGeometryPtr& geom )
		: tManipulationGizmo( geom )
	{
	}

	void tEffectRotationGizmo::fSpecifyWorldCoords( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects,
		const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ )
	{
		// create gizmo editor action, allowing for undo/redo
		tEditorActionPtr action( new tManipulationGizmoAction( mainWindow, selectedObjects ) );
		mainWindow.fGuiApp( ).fActionStack( ).fAddAction( action );

		const b32 rotateAroundReferenceFrame = selectedObjects.fCount( ) > 1;

		const Math::tVec3f avgPos = selectedObjects.fComputeAveragePosition( );
		mReferenceFrame = Math::tMat3f::cIdentity;
		mReferenceFrame.fSetTranslation( avgPos );

		const Math::tMat3f refFrameInv = mReferenceFrame.fInverse( );

		if( rotateAroundReferenceFrame ) // rotate reference frame
		{
			Math::tEulerAnglesf ea = Math::tEulerAnglesf( Math::tQuatf( mReferenceFrame ) );

			if( doX ) ea.x = Math::fToRadians( worldCoords.x );
			if( doY ) ea.y = Math::fToRadians( worldCoords.y );
			if( doZ ) ea.z = Math::fToRadians( worldCoords.z );
			const Math::tQuatf r( ea );

			r.fToMatrix( mReferenceFrame );

			// forcefully preserve previous scale of matrix (avoids floating point drift)
			mReferenceFrame.fNormalizeBasis( );
		}

		// now explicitly set all transforms
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tEditableObject* eo = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );

			if( rotateAroundReferenceFrame )
			{
				const Math::tMat3f refFrameRel = refFrameInv * eo->fObjectToWorld( );

				// reset the transform
				eo->fMoveTo( mReferenceFrame * refFrameRel );
			}
			else
			{
				Math::tMat3f editorXform = eo->fObjectToWorld( );
				const Math::tVec3f scale = editorXform.fGetScale( );

				editorXform.fScaleLocal( 1.f / scale );
				Math::tEulerAnglesf ea = Math::tEulerAnglesf( Math::tQuatf( editorXform ) );

				if( doX ) ea.x = Math::fToRadians( worldCoords.x );
				if( doY ) ea.y = Math::fToRadians( worldCoords.y );
				if( doZ ) ea.z = Math::fToRadians( worldCoords.z );
				const Math::tQuatf r( ea );

				r.fToMatrix( editorXform );
				editorXform.fScaleLocal( scale );

				eo->fMoveTo( editorXform );
			}
		}

		action->fEnd( );
	}

	void tEffectRotationGizmo::fExtractWorldCoords( 
		const tEditorSelectionList& selectedObjects,
		Math::tVec3f& worldCoords ) const
	{
		if( selectedObjects.fCount( ) > 0 )
		{
			Math::tMat3f editorXform = selectedObjects[ 0 ]->fDynamicCast< tEditableObject >( )->fObjectToWorld( );
			editorXform.fNormalizeBasis( );
			const Math::tEulerAnglesf euler = Math::tEulerAnglesf( Math::tQuatf( editorXform ) );
			worldCoords = Math::fToDegrees( Math::tVec3f( 
				euler.x < 0.f ? euler.x + Math::c2Pi : euler.x,
				euler.y < 0.f ? euler.y + Math::c2Pi : euler.y,
				euler.z < 0.f ? euler.z + Math::c2Pi : euler.z ) );
			worldCoords.x = fRound<f32>( worldCoords.x );
			worldCoords.y = fRound<f32>( worldCoords.y );
			worldCoords.z = fRound<f32>( worldCoords.z );
		}
	}

	void tEffectRotationGizmo::fComputeLocalToWorld( Math::tMat3f& localToWorldOut, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const
	{
		if( mDragging && selectedObjects.fCount( ) > 1 )
		{
			localToWorldOut = mReferenceFrame;
			fApplyCameraScaling( localToWorldOut, camera, scale );
		}
		else
			tManipulationGizmo::fComputeLocalToWorld( localToWorldOut, selectedObjects, camera, scale, orientToWorld );
	}

	void tEffectRotationGizmo::fOnDragStart( const tEditorSelectionList& selectedObjects )
	{
		const Math::tVec3f avgPos = selectedObjects.fComputeAveragePosition( );
		mReferenceFrame = Math::tMat3f::cIdentity;
		mReferenceFrame.fSetTranslation( avgPos );
	}


	void tEffectRotationGizmo::fOnDrag( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects, 
		const Math::tMat3f& gizmoLocalToWorld,
		const Math::tRayf& pickRay, 
		const Gfx::tCamera& camera,
		const Input::tMouse& mouse,
		const Input::tKeyboard& kb )
	{
		sigassert( mX || mY || mZ );

		if( mX )
		{
			mReferencePoint = fApplyRotationDelta( 
				selectedObjects, pickRay, 1, gizmoLocalToWorld.fYAxis( ).fNormalizeSafe( ), gizmoLocalToWorld.fGetTranslation( ), mReferencePoint );
		}
		if( mY )
		{
			mReferencePoint = fApplyRotationDelta( 
				selectedObjects, pickRay, 2, gizmoLocalToWorld.fZAxis( ).fNormalizeSafe( ), gizmoLocalToWorld.fGetTranslation( ), mReferencePoint );
		}
		if( mZ )
		{
			mReferencePoint = fApplyRotationDelta( 
				selectedObjects, pickRay, 0, gizmoLocalToWorld.fXAxis( ).fNormalizeSafe( ), gizmoLocalToWorld.fGetTranslation( ), mReferencePoint );
		}
	}

	namespace
	{
		const f32 cRotationAxisLength = 0.1f;
	}

	Math::tVec3f tEffectRotationGizmo::fApplyRotationDelta( 
		const tEditorSelectionList& selectedObjects, 
		const Math::tRayf& pickRay,
		u32 iaxis,
		const Math::tVec3f& planeNormal,
		const Math::tVec3f& rotationCenter,
		const Math::tVec3f& referencePoint )
	{
		if( planeNormal.fIsZero( ) )
			return referencePoint;

		// create plane containing reference point with normal the axis of rotation
		const Math::tPlanef plane( planeNormal, referencePoint );

		// intersect the pick ray with the plane
		Math::tIntersectionRayPlane<f32> intersection( plane, pickRay );
		if( !intersection.fIntersects( ) || intersection.fParallel( ) )
			return referencePoint;

		const b32 rotateAroundReferenceFrame = selectedObjects.fCount( ) > 1;

		// evaluate point of intersection with reference plane
		const Math::tVec3f p = pickRay.fPointAtTime( intersection.fT( ) );

		const Math::tVec3f toRefPoint = referencePoint - rotationCenter;
		const Math::tVec3f v = cRotationAxisLength * ( p - rotationCenter ).fNormalizeSafe( );
		const Math::tVec3f x = toRefPoint.fCross( v );

		// compute angle
		const f32 sint = x.fLength( ) * ( x.fDot( planeNormal ) >= 0.f ? 1.f : -1.f );
		const f32 cost = toRefPoint.fDot( v );
		const f32 theta = std::atan2( sint, cost );

		// only rotate all the objects if angle is beyond a certain threshold
		if( fAbs( theta ) > 0.001f )
		{
			const Math::tMat3f refFrameInv = mReferenceFrame.fInverse( );

			if( rotateAroundReferenceFrame ) // rotate reference frame
			{
				// create rotation quaternion
				const Math::tQuatf rot( Math::tAxisAnglef( mReferenceFrame.fGetCol( iaxis ).fNormalizeSafe( ), theta ) );

				// rotate each axis by incremental rotation
				mReferenceFrame.fXAxis( rot.fRotate( mReferenceFrame.fXAxis( ) ) );
				mReferenceFrame.fYAxis( rot.fRotate( mReferenceFrame.fYAxis( ) ) );
				mReferenceFrame.fZAxis( rot.fRotate( mReferenceFrame.fZAxis( ) ) );

				// forcefully preserve previous scale of matrix (avoids floating point drift)
				mReferenceFrame.fNormalizeBasis( );
			}

			for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
			{
				tEditableObject* editable = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
				if( !editable->fSupportsRotation( ) ) continue;

				if( rotateAroundReferenceFrame )
				{
					const Math::tMat3f refFrameRel = refFrameInv * editable->fObjectToWorld( );

					// reset the transform
					editable->fMoveTo( mReferenceFrame * refFrameRel );
				}
				else
				{
					// retrieve object's current transform
					Math::tMat3f xform = editable->fObjectToWorld( );

					// save scale (so we can forcefully preserve it)
					const Math::tVec3f scale = xform.fGetScale( );

					// create rotation quaternion
					const Math::tQuatf rot( Math::tAxisAnglef( xform.fGetCol( iaxis ).fNormalizeSafe( ), theta ) );

					// rotate each axis by incremental rotation
					xform.fXAxis( rot.fRotate( xform.fXAxis( ) ) );
					xform.fYAxis( rot.fRotate( xform.fYAxis( ) ) );
					xform.fZAxis( rot.fRotate( xform.fZAxis( ) ) );

					// forcefully preserve previous scale of matrix (avoids floating point drift)
					xform.fNormalizeBasis( );
					xform.fScaleLocal( scale );

					// reset the transform
					editable->fMoveTo( xform );
				}
			}

			// return new reference point
			return rotationCenter + v;
		}

		return referencePoint;
	}
}

