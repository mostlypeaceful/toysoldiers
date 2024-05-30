#include "SigFxPch.hpp"
#include "tEffectFollowMouseGizmo.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tManipulationGizmoAction.hpp"
#include "Editor/tEditableObject.hpp"
#include "Math/tIntersectionRayPlane.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "tFxEditorActions.hpp"
#include "tEffectTranslationGizmo.hpp"		// for fComputeCenterPostion( )
#include "tSigFxGraphline.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{
	using namespace FX;

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
					const Math::tVec3f p = pickRay.fEvaluate( intersection.fT( ) );
					return Math::fProjectToUnit( translationAxis, p - referencePoint );
				}
			}

			return Math::tVec3f::cZeroVector;
		}
	}


	tEffectFollowMouseGizmo::tEffectFollowMouseGizmo( const tGizmoGeometryPtr& geom, tSigFxKeyline* Keyline )
		: tManipulationGizmo( geom )
		, mTheKeyline( Keyline )
		, mInitialCollect( true )
		, mPreviousPos( Math::tVec3f::cZeroVector )
	{
		mPreviousState = mTheKeyline->fPaused( );
		tEditableObjectContainer& editables = mTheKeyline->fMainWindow( )->fGuiApp( ).fEditableObjects( );
		mAction.fReset( new tManipulationGizmoAction( *mTheKeyline->fMainWindow( ), editables.fGetSelectionList( ) ) );
		mAction->fEnd( );
	}

	tEffectFollowMouseGizmo::~tEffectFollowMouseGizmo( )
	{

	}

	void tEffectFollowMouseGizmo::fEndThis( )
	{
		mAction->fUndo( );

		tEditableObjectContainer& editables = mTheKeyline->fMainWindow( )->fGuiApp( ).fEditableObjects( );
		tGrowableArray< tSigFxParticleSystem* > systems;
		editables.fCollectByType< tSigFxParticleSystem >( systems );
		for( u32 i = 0; i < systems.fCount( ); ++i )
		{
			systems[ i ]->fGetParticleSystem( )->fClear( );
			//systems[ i ]->fGetParticleSystem( )->fSetLocalSpace( true );		// currently we're keeping all particle systems out of local space, so no need to swap back and forth here.
		}
	}

	void tEffectFollowMouseGizmo::fOnSelChanged( tEditorSelectionList& selectedObjects )
	{
		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tSigFxParticleSystem* fxps = selectedObjects[ i ]->fDynamicCast< tSigFxParticleSystem >( );
			if( fxps )
			{
				fxps->fGetParticleSystem( )->fClear( );
				//fxps->fGetParticleSystem( )->fSetLocalSpace( false );		// see above local-space comment
			}
		}
		mInitialCollect = false;
	}

	void tEffectFollowMouseGizmo::fTick( tToolsGuiMainWindow& editorWindow, tEditorSelectionList& selectedObjects )
	{
			
	}

	void tEffectFollowMouseGizmo::fUpdate( 
			tToolsGuiMainWindow& editorWindow,
			tEditorSelectionList& selectedObjects, 
			const Math::tMat3f& gizmoLocalToWorld,
			const Math::tRayf& pickRay, 
			const Gfx::tCamera& camera,
			const Input::tMouse& mouse,
			const Input::tKeyboard& kb )
	{
		if( mInitialCollect )
		{
			fOnSelChanged( selectedObjects );
		}

		tWxRenderPanelContainer* gfx = editorWindow.fRenderPanelContainer( );
		tWxRenderPanel* panel = gfx->fGetActiveRenderPanel( );

		// use lookat
		Math::tVec3f p = panel->fGetScreen( )->fViewport( 0 )->fLogicCamera( ).fGetTripod( ).mLookAt;

		// intersect ray with lookat plane, and use that intersection if possible
		Math::tIntersectionRayPlane<f32> intersect( pickRay, Math::tPlanef( Math::tVec3f::cYAxis, p ) );
		if( intersect.fIntersects( ) && !intersect.fParallel( ) )
			p = pickRay.fEvaluate( intersect.fT( ) );

		const Math::tVec3f difference = p - mPreviousPos;
		mPreviousPos = p;

		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			selectedObjects[ i ]->fTranslate( difference );
		}
	}

	void tEffectFollowMouseGizmo::fSpecifyWorldCoords( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects,
		const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ )
	{
		
	}

	void tEffectFollowMouseGizmo::fExtractWorldCoords( 
		const tEditorSelectionList& selectedObjects,
		Math::tVec3f& worldCoords ) const
	{
		worldCoords = fComputeCenterPosition( selectedObjects );
	}

	void tEffectFollowMouseGizmo::fOnDrag( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects, 
		const Math::tMat3f& gizmoLocalToWorld,
		const Math::tRayf& pickRay, 
		const Gfx::tCamera& camera,
		const Input::tMouse& mouse,
		const Input::tKeyboard& kb )
	{
		
	}	

	void tEffectFollowMouseGizmo::fOnDragStart( const tEditorSelectionList& selectedObjects )
	{
		
	}

	void tEffectFollowMouseGizmo::fOnDragEnd( )
	{
		
	}

	void tEffectFollowMouseGizmo::fComputeLocalToWorld( Math::tMat3f& xform, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const
	{
		Math::tVec3f averagePos = fComputeCenterPosition( selectedObjects );

		xform = Math::tMat3f::cIdentity;
		xform.fSetTranslation( averagePos );

		if( !orientToWorld )
			xform.fNormalizeBasis( );

		fApplyCameraScaling( xform, camera, scale );
	}


	void tEffectFollowMouseGizmo::fCollectEntities( const tEditorSelectionList& selectedObjects, b32 noNewKeyframes )
	{
		
	}
}
