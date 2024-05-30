#include "ToolsGuiPch.hpp"
#include "tRotationGizmo.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tManipulationGizmoAction.hpp"
#include "Editor/tEditableObject.hpp"
#include "Math/tIntersectionRayPlane.hpp"

namespace Sig
{

	tRotationGizmo::tRotationGizmo( const tGizmoGeometryPtr& geom )
		: tManipulationGizmo( geom )
		, mThetaDelta( 0 )
		, mReferenceFrame( Math::tMat3f::cIdentity )
	{
	}

	void tRotationGizmo::fSpecifyWorldCoords( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects,
		const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ )
	{
		// create gizmo editor action, allowing for undo/redo
		tEditorActionPtr action( new tManipulationGizmoAction( mainWindow, selectedObjects ) );
		mainWindow.fGuiApp( ).fActionStack( ).fAddAction( action );

		const b32 rotateAroundReferenceFrame = (mOrientToWorld || selectedObjects.fCount( ) > 1) && !worldCoords.fIsZero( );

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

	void tRotationGizmo::fExtractWorldCoords( 
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

	void tRotationGizmo::fComputeLocalToWorld( Math::tMat3f& localToWorldOut, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const
	{
		if( Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonLCtrl ) || 
			Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonRCtrl ) ) 
			orientToWorld = !orientToWorld;

		if( mDragging && (orientToWorld || selectedObjects.fCount( ) > 1) )
		{
			localToWorldOut = mReferenceFrame;
			fApplyCameraScaling( localToWorldOut, camera, scale );
		}
		else
			tManipulationGizmo::fComputeLocalToWorld( localToWorldOut, selectedObjects, camera, scale, orientToWorld );
	}

	void tRotationGizmo::fOnDragStart( const tEditorSelectionList& selectedObjects )
	{
		const Math::tVec3f avgPos = selectedObjects.fComputeAveragePosition( );
		mReferenceFrame = Math::tMat3f::cIdentity;
		mReferenceFrame.fSetTranslation( avgPos );
	}

	void tRotationGizmo::fOnDrag( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects, 
		const Math::tMat3f& gizmoLocalToWorld,
		const Math::tRayf& pickRay, 
		const Gfx::tCamera& camera,
		const Input::tMouse& mouse,
		const Input::tKeyboard& kb )
	{
		sigassert( mX || mY || mZ );

		b32 rotationSnap = kb.fButtonHeld( kb.cButtonJ );
		u32 rotationSnapAmount = mainWindow.fRotationGizmoSettings( )->fValue( );		

		if( mX )
		{
			mReferencePoint = fApplyRotationDelta( 
				selectedObjects, pickRay, 1, gizmoLocalToWorld.fYAxis( ).fNormalizeSafe( ), gizmoLocalToWorld.fGetTranslation( ), mReferencePoint, rotationSnap, rotationSnapAmount );
		}
		if( mY )
		{
			mReferencePoint = fApplyRotationDelta( 
				selectedObjects, pickRay, 2, gizmoLocalToWorld.fZAxis( ).fNormalizeSafe( ), gizmoLocalToWorld.fGetTranslation( ), mReferencePoint, rotationSnap, rotationSnapAmount );
		}
		if( mZ )
		{
			mReferencePoint = fApplyRotationDelta( 
				selectedObjects, pickRay, 0, gizmoLocalToWorld.fXAxis( ).fNormalizeSafe( ), gizmoLocalToWorld.fGetTranslation( ), mReferencePoint, rotationSnap, rotationSnapAmount );
		}
	}

	namespace
	{
		const f32 cRotationAxisLength = 0.1f;
	}

	Math::tVec3f tRotationGizmo::fApplyRotationDelta( 
		const tEditorSelectionList& selectedObjects, 
		const Math::tRayf& pickRay,
		u32 iaxis,
		const Math::tVec3f& planeNormal,
		const Math::tVec3f& rotationCenter,
		const Math::tVec3f& referencePoint,
		b32 snap,
		u32 snapDegree)
	{
		if( planeNormal.fIsZero( ) )
			return referencePoint;

		// create plane containing reference point with normal the axis of rotation
		const Math::tPlanef plane( planeNormal, referencePoint );

		// intersect the pick ray with the plane
		Math::tIntersectionRayPlane<f32> intersection( plane, pickRay );
		if( !intersection.fIntersects( ) || intersection.fParallel( ) )
			return referencePoint;

		b32 orientToWorld = mOrientToWorld;
		if( Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonLCtrl ) || 
			Input::tKeyboard::fInstance( ).fButtonHeld( Input::tKeyboard::cButtonRCtrl ) ) 
			orientToWorld = !orientToWorld;

		const b32 rotateAroundReferenceFrame = orientToWorld || selectedObjects.fCount( ) > 1;

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
			f32 theta2 = theta;
			if( snap )
			{
				// Round to nearest 15 degree
				f32 theta3 = theta + mThetaDelta;

				// Cleaner to do this math as degrees and transform back to radians
				s32 percent = Math::fToDegrees( theta3 );
				s32 signOfPercent = fabs(theta3) / theta3;

				// if(((abs( degree ))%snap) > (snap/2)) then round up else round down
				s32 absolutePercent = percent * signOfPercent;
				s32 modPercent = absolutePercent%snapDegree;
				if( modPercent > f32( snapDegree / 2.0f ) )
					absolutePercent += snapDegree;
				absolutePercent -= modPercent;

				// Transform back to radians
				theta2 = Math::fToRadians( f32( absolutePercent ) ) * signOfPercent;

				// Remember how much of theta was rounded off
				mThetaDelta = theta3 - theta2;
			}

			const Math::tMat3f refFrameInv = mReferenceFrame.fInverse( );

			if( rotateAroundReferenceFrame ) // rotate reference frame
			{
				// create rotation quaternion
				const Math::tQuatf rot( Math::tAxisAnglef( mReferenceFrame.fGetCol( iaxis ).fNormalizeSafe( ), theta2 ) );

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
					const Math::tQuatf rot( Math::tAxisAnglef( xform.fGetCol( iaxis ).fNormalizeSafe( ), theta2 ) );

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

