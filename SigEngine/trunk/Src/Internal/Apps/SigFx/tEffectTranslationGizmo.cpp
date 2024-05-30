#include "SigFxPch.hpp"
#include "tEffectTranslationGizmo.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tManipulationGizmoAction.hpp"
#include "Editor/tEditableObject.hpp"
#include "Math/tIntersectionRayPlane.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "FxEditor/tSigFxAttractor.hpp"
#include "FxEditor/tSigFxMeshSystem.hpp"
#include "tSigFxMainWindow.hpp"

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


	tEffectTranslationGizmo::tEffectTranslationGizmo( const tGizmoGeometryPtr& geom, tSigFxKeyline* keyline )
		: tManipulationGizmo( geom )
		, mTheKeyline( keyline )
	{
	}

	void tEffectTranslationGizmo::fSpecifyWorldCoords( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects,
		const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ )
	{
		// create gizmo editor action, allowing for undo/redo
		tEditorActionPtr action( new tManipulationGizmoAction( mainWindow, selectedObjects ) );
		mainWindow.fGuiApp( ).fActionStack( ).fClearDirty( );
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

		//mTheKeyline->fForceWholeSceneRefresh( );
	}

	void tEffectTranslationGizmo::fExtractWorldCoords( 
		const tEditorSelectionList& selectedObjects,
		Math::tVec3f& worldCoords ) const
	{
		if( selectedObjects.fCount( ) > 0 )
		{
			const Math::tMat3f editorXform = selectedObjects[ 0 ]->fDynamicCast< tEditableObject >( )->fObjectToWorld( );
			worldCoords = editorXform.fGetTranslation( );
		}
	}

	void tEffectTranslationGizmo::fOnDragStart( const tEditorSelectionList& selectedObjects )
	{
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tSigFxParticleSystem* fxps = selectedObjects[ i ]->fDynamicCast< tSigFxParticleSystem >( );
			if( fxps )
				fxps->fSetAttachmentEntity( 0 );
		}
	}

	void tEffectTranslationGizmo::fOnDragEnd( )
	{
		//mTheKeyline->fForceWholeSceneRefresh( );
		const tEditorSelectionList& selectedObjects = mTheKeyline->fMainWindow( )->fGuiApp( ).fSelectionList( );
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tSigFxParticleSystem* fxps = selectedObjects[ i ]->fDynamicCast< tSigFxParticleSystem >( );
			if( fxps )
			{
				tAttachmentEntity* attach = mTheKeyline->fMainWindow( )->fFxScene( )->fCheckForAttachmentToAttachmentEntities( fxps->fGetParticleSystem( ) );
				fxps->fSetAttachmentEntity( attach );
			}
		}
	}

	void tEffectTranslationGizmo::fOnDrag( 
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
			for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
			{
				tEditableObject* editable = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
				if( !editable->fSupportsTranslation( ) ) continue;
				editable->fTranslate( translate );
			}
		}
	}	

	
	Math::tVec3f fComputeCenterPosition( const tEditorSelectionList& selectedObjects )
	{
		Math::tVec3f averagePos = Math::tVec3f::cZeroVector;
	
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tEditableObject* editable = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
			const Math::tMat3f xform = editable->fObjectToWorld( );
			const Math::tMat3f xtrans( Math::tQuatf::cIdentity, xform.fGetTranslation( ) );
			tSigFxParticleSystem* fxps = editable->fDynamicCast< tSigFxParticleSystem >( );
			tSigFxAttractor* fxa = editable->fDynamicCast< tSigFxAttractor >( );

			if( fxps )
				averagePos += xform.fXformVector( fxps->fGetParticleSystem( )->fEmitterTranslation( ) );
			if( fxa )
				averagePos += xtrans.fXformVector( fxa->fGetAttractor( )->fCurrentPosition( ) );

			averagePos += xform.fGetTranslation( );
		}

		return averagePos / ( f32 ) selectedObjects.fCount( );
	}


}
