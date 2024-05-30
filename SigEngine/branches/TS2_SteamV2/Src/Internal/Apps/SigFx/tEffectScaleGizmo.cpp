#include "SigFxPch.hpp"
#include "tEffectScaleGizmo.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
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


	tEffectScaleGizmo::tEffectScaleGizmo( const tGizmoGeometryPtr& geom, tSigFxKeyline* Keyline )
		: tManipulationGizmo( geom )
		, mTheKeyline( Keyline )
		, mUndoAction( 0 )
		, mInitialCollect( true )
	{
		mPreviousState = mTheKeyline->fPaused( );
		mOnSelChanged.fFromMethod< tEffectScaleGizmo, &tEffectScaleGizmo::fOnSelChanged >( this );
		mTheKeyline->fMainWindow( )->fGuiApp( ).fSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelChanged );
	}

	void tEffectScaleGizmo::fOnSelChanged( tEditorSelectionList& selectedObjects )
	{
		mInitialCollect = true;
	}

	void tEffectScaleGizmo::fTick( tToolsGuiMainWindow& editorWindow, tEditorSelectionList& selectedObjects )
	{
		if( mInitialCollect )
		{
			mInitialCollect = false;
			fCollectEntities( selectedObjects, true );
		}

		if( mFxEditables.fCount( ) )
		{
			const tFxEditableGizmoInfo* es = &mFxEditables[ 0 ];
			mTheKeyline->fGraphline( )->fSetActiveGraph( *es->mGraph );
		}
	}

	void tEffectScaleGizmo::fSpecifyWorldCoords( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects,
		const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ )
	{
		// Save the state of our particle systems before they're changed!
		tEditorActionPtr action( new tSaveParticleSystemGraphsAction( selectedObjects ) );
		mainWindow.fGuiApp( ).fActionStack( ).fAddAction( action );

		fCollectEntities( selectedObjects );

		// get all the particle systems and scale them. Actually with the particle systems
		// we're just scaling the emitter, not the actual xform.

		for( u32 i = 0; i < mFxEditables.fCount( ); ++i )
		{
			tFxEditableGizmoInfo* es = &mFxEditables[ i ];
			Math::tVec3f scale = es->mKeyframe->fValue< Math::tVec3f >( );

			if( es->mEditable->fUniformScaleOnly( ) )
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

			es->mKeyframe->fSetValue< Math::tVec3f >( scale );
			( *es->mGraph )->fBuildValues( );
		}

		action->fEnd( );
		mTheKeyline->fForceWholeSceneRefresh( );
		fCollectEntities( selectedObjects );
	}
	
	void tEffectScaleGizmo::fExtractWorldCoords( const tEditorSelectionList& selectedObjects, Math::tVec3f& worldCoords ) const
	{
		if( mFxEditables.fCount( ) )
		{
			f32 delta = mTheKeyline->fDelta( );
			const tFxEditableGizmoInfo* es = &mFxEditables[ 0 ];
			worldCoords = ( *es->mGraph )->fSample< Math::tVec3f >( 0, delta );
		}
	}


	void tEffectScaleGizmo::fOnDrag( 
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

		for( u32 i = 0; i < mFxEditables.fCount( ); ++i )
		{
			tFxEditableGizmoInfo* es = &mFxEditables[ i ];

			Math::tVec3f tempScale;
			if( es->mEditable->fUniformScaleOnly( ) )
			{
				const f32 max = scale.fMax( );
				const f32 min = scale.fMin( );
				tempScale = ( max == 0.f ) ? min : max;
			}
			else
				tempScale = scale;

			Math::tVec3f objScale( es->mKeyframe->fValue< Math::tVec3f > ( ) );
			objScale += tempScale;

			const f32 minScale = 0.01f;
			const f32 maxScale = 9999999999.f;
			objScale.x = fClamp( objScale.x, minScale, maxScale );
			objScale.y = fClamp( objScale.y, minScale, maxScale );
			objScale.z = fClamp( objScale.z, minScale, maxScale );

			es->mKeyframe->fSetValue< Math::tVec3f >( objScale );
			( *es->mGraph )->fBuildValues( );
		}
		

		mTheKeyline->fGraphline( )->fFrame( );
		mTheKeyline->fForceWholeSceneRefresh( );
	}


	void tEffectScaleGizmo::fOnDragStart( const tEditorSelectionList& selectedObjects )
	{
		mPreviousState = mTheKeyline->fPaused( );
		if( !mTheKeyline->fPaused( ) )
			mTheKeyline->fSetPaused( true );

		// Save the state of our particle systems before they're changed!
		mUndoAction.fReset( new tSaveParticleSystemGraphsAction( selectedObjects ) );
		mTheKeyline->fMainWindow( )->fGuiApp( ).fActionStack( ).fAddAction( mUndoAction );

		fCollectEntities( selectedObjects );

		mTheKeyline->fImmediateUpdate( true );

		for( u32 i = 0; i < mFxEditables.fCount( ); ++i )
			mTheKeyline->fSelectKeyframes( mFxEditables[ i ].mKeyframe );
	}
		
	
	void tEffectScaleGizmo::fOnDragEnd( )
	{
		if( mUndoAction )
			mUndoAction->fEnd( );

		mFxEditables.fSetCount( 0 );
		mTheKeyline->fSetPaused( mPreviousState );
		fCollectEntities( mTheKeyline->fMainWindow( )->fGuiApp( ).fSelectionList( ), true );
	}

	void tEffectScaleGizmo::fComputeLocalToWorld( Math::tMat3f& xform, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const
	{
		Math::tVec3f averagePos = fComputeCenterPosition( selectedObjects );

		xform = Math::tMat3f::cIdentity;
		xform.fSetTranslation( averagePos );

		if( !orientToWorld )
			xform.fNormalizeBasis( );

		fApplyCameraScaling( xform, camera, scale );
	}

	void tEffectScaleGizmo::fCollectEntities( const tEditorSelectionList& selectedObjects, b32 noNewKeyframes )
	{
		f32 delta = mTheKeyline->fDelta( );
		mFxEditables.fSetCount( 0 );

		for( u32 i = 0; i < selectedObjects.fCount( ); ++i )
		{
			tFxEditableGizmoInfo es;
			es.mEditable = selectedObjects[ i ]->fDynamicCast< tEditableObject >( );
			
			tSigFxParticleSystem* fxps = es.mEditable->fDynamicCast< tSigFxParticleSystem >( );
			tSigFxAttractor* fxa = es.mEditable->fDynamicCast< tSigFxAttractor >( );

			if( !fxps && !fxa )
				continue;

			if( fxps )
			{
				tParticleSystemPtr ps = fxps->fGetParticleSystem( );
				es.mGraph = &ps->fState( ).fToolState( ).mEmissionGraphs[ cEmitterScaleGraph ];

				if( !noNewKeyframes )
				{
					es.mKeyframe = ( *es.mGraph )->fCopyPreviousKey( delta, 0.025f );
					es.mKeyframe->fSetValue< Math::tVec3f >( ( *es.mGraph )->fSample< Math::tVec3f >( ps->fSystemRandomNumberGenerator( ), delta ) );
				}
			}
			else if( fxa )
			{
				tParticleAttractorPtr attractor = fxa->fGetAttractor( );
				Attractors::tAttractorDataPtr data = attractor->fGetAttractorData( );

				es.mGraph = &data->fGraph( cAttractorScaleGraph );

				if( !noNewKeyframes )
				{
					es.mKeyframe = ( *es.mGraph )->fCopyPreviousKey( delta, 0.025f );
					es.mKeyframe->fSetValue< f32 >( ( *es.mGraph )->fSample< f32 >( attractor->fRandomNumberGenerator( ), delta ) );
				}
			}

			if( !noNewKeyframes )
			{
				( *es.mGraph )->fAddKeyframe( es.mKeyframe );
				( *es.mGraph )->fBuildValues( );
			}

			mFxEditables.fPushBack( es );
		}
	}


}
