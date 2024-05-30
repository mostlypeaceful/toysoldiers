#include "SigFxPch.hpp"
#include "tEffectPositionOffsetGizmo.hpp"
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


	tEffectPositionOffsetGizmo::tEffectPositionOffsetGizmo( const tGizmoGeometryPtr& geom, tSigFxKeyline* Keyline )
		: tManipulationGizmo( geom )
		, mTheKeyline( Keyline )
		, mInitialCollect( true )
	{
		mPreviousState = mTheKeyline->fPaused( );
		mOnSelChanged.fFromMethod< tEffectPositionOffsetGizmo, &tEffectPositionOffsetGizmo::fOnSelChanged >( this );
		mTheKeyline->fMainWindow( )->fGuiApp( ).fSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelChanged );
	}

	void tEffectPositionOffsetGizmo::fOnSelChanged( tEditorSelectionList& selectedObjects )
	{
		mInitialCollect = true;
	}

	void tEffectPositionOffsetGizmo::fTick( tToolsGuiMainWindow& editorWindow, tEditorSelectionList& selectedObjects )
	{
		if( mInitialCollect )
		{
			mInitialCollect = false;
			fCollectEntities( selectedObjects, true );
		}

		for( u32 i = 0; i < mFxEditables.fCount( ); ++i )
		{
			const tFxEditableGizmoInfo* es = &mFxEditables[ i ];

			if( i == 0 )				
				mTheKeyline->fGraphline( )->fSetActiveGraph( *es->mGraph );

			const Math::tMat3f xform = es->mEditable->fObjectToWorld( );
			const Math::tVec3f editablePos = xform.fGetTranslation( );

			for( u32 j = 0; j < ( *es->mGraph )->fNumKeyframes( ) - 1; ++j )
			{
				tKeyframePtr key1 = ( *es->mGraph )->fKeyframe( j );
				tKeyframePtr key2 = ( *es->mGraph )->fKeyframe( j+1 );

				Math::tVec3f p1 = ( editablePos + xform.fXformVector( key1->fValue< Math::tVec3f >( ) ) );
				Math::tVec3f p2 = ( editablePos + xform.fXformVector( key2->fValue< Math::tVec3f >( ) ) );

				tPair< Math::tVec3f, Math::tVec3f > line = fMakePair< Math::tVec3f, Math::tVec3f >( p1, p2 );
				
				const Math::tVec4f lineColor( 0.54f, 0.8f, 1.f, 0.75f );
				const Math::tVec4f endpointColor( 0.54f, 0.8f, 1.f, 0.75f );
				const f32 endpointRadius = 0.0666f;

				editorWindow.fGuiApp( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( line, lineColor );
				editorWindow.fGuiApp( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tSpheref( p1, endpointRadius ), endpointColor );

				if( j == ( *es->mGraph )->fNumKeyframes( ) - 2 ) 
					editorWindow.fGuiApp( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tAabbf( Math::tSpheref( p2, endpointRadius ) ), endpointColor );
			}
		}
	}

	void tEffectPositionOffsetGizmo::fSpecifyWorldCoords( 
		tToolsGuiMainWindow& mainWindow,
		const tEditorSelectionList& selectedObjects,
		const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ )
	{
		// Save the state of our particle systems before they're changed!
		tEditorActionPtr action( new tSaveParticleSystemGraphsAction( selectedObjects ) );
		mainWindow.fGuiApp( ).fActionStack( ).fAddAction( action );

		fCollectEntities( selectedObjects );

		for( u32 i = 0; i < mFxEditables.fCount( ); ++i )
		{
			tFxEditableGizmoInfo* es = &mFxEditables[ i ];
			Math::tVec3f t = es->mKeyframe->fValue< Math::tVec3f >( );
			if( doX ) t.x = worldCoords.x;
			if( doY ) t.y = worldCoords.y;
			if( doZ ) t.z = worldCoords.z;
			es->mKeyframe->fSetValue< Math::tVec3f >( t );
			( *es->mGraph )->fBuildValues( );
		}

		action->fEnd( );
		mTheKeyline->fForceWholeSceneRefresh( );
	}

	void tEffectPositionOffsetGizmo::fExtractWorldCoords( 
		const tEditorSelectionList& selectedObjects,
		Math::tVec3f& worldCoords ) const
	{
		if( mFxEditables.fCount( ) )
		{
			f32 delta = mTheKeyline->fDelta( );
			const tFxEditableGizmoInfo* es = &mFxEditables[ 0 ];
			worldCoords = ( *es->mGraph )->fSample< Math::tVec3f >( 0, delta );
		}

		//worldCoords = fComputeCenterPosition( selectedObjects );
	}

	void tEffectPositionOffsetGizmo::fOnDrag( 
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
			for( u32 i = 0; i < mFxEditables.fCount( ); ++i )
			{
				tFxEditableGizmoInfo* es = &mFxEditables[ i ];
				const Math::tMat3f xform = es->mEditable->fObjectToWorld( );
				const Math::tVec3f scale = xform.fGetScale( );
				translate *= ( 1.f / scale );	// take into account that our worldspace might be scaled so if that's the case then scale our translated value so it keeps up with the mouse position.
				
				if( es->mKeyframe )
				{
					Math::tVec3f val = translate + es->mKeyframe->fValue< Math::tVec3f >( );
					es->mKeyframe->fSetValue< Math::tVec3f >( val );
					( *es->mGraph )->fBuildValues( );
				}
			}
			
			mTheKeyline->fGraphline( )->fFrame( );
			mTheKeyline->fForceWholeSceneRefresh( );
		}
	}	

	void tEffectPositionOffsetGizmo::fOnDragStart( const tEditorSelectionList& selectedObjects )
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

	void tEffectPositionOffsetGizmo::fOnDragEnd( )
	{
		if( mUndoAction )
			mUndoAction->fEnd( );

		mFxEditables.fSetCount( 0 );
		mTheKeyline->fSetPaused( mPreviousState );
		fCollectEntities( mTheKeyline->fMainWindow( )->fGuiApp( ).fSelectionList( ), true );
	}

	void tEffectPositionOffsetGizmo::fComputeLocalToWorld( Math::tMat3f& xform, const tEditorSelectionList& selectedObjects, const Gfx::tCamera& camera, f32 scale, b32 orientToWorld ) const
	{
		Math::tVec3f averagePos = fComputeCenterPosition( selectedObjects );

		xform = Math::tMat3f::cIdentity;
		xform.fSetTranslation( averagePos );

		if( !orientToWorld )
			xform.fNormalizeBasis( );

		fApplyCameraScaling( xform, camera, scale );
	}


	void tEffectPositionOffsetGizmo::fCollectEntities( const tEditorSelectionList& selectedObjects, b32 noNewKeyframes )
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
				continue;		// we only want to handle particle systems and attractors.

			if( fxps )
			{
				tParticleSystemPtr ps = fxps->fGetParticleSystem( );
				es.mGraph = &ps->fState( ).fToolState( ).mEmissionGraphs[ cEmitterTranslationGraph ];

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

				es.mGraph = &data->fGraph( cAttractorPositionGraph );
				if( !noNewKeyframes )
				{
					es.mKeyframe = ( *es.mGraph )->fCopyPreviousKey( delta, 0.025f );
					es.mKeyframe->fSetValue< Math::tVec3f >( ( *es.mGraph )->fSample< Math::tVec3f >( attractor->fRandomNumberGenerator( ), delta ) );
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
