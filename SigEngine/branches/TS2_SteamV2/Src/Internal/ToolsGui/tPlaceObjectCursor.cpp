#include "ToolsGuiPch.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "Math/tIntersectionRayPlane.hpp"


namespace Sig
{

	tPlaceObjectsAction::tPlaceObjectsAction( tToolsGuiMainWindow& mainWindow, const tEntityPtr& entity, b32 deleteObjs )
		: tEditorButtonManagedCursorAction( mainWindow )
		, mDeleteObjects( deleteObjs )
	{
		mEntities.fPushBack( entity );
		mSavedSelection.fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fRedo( );
	}
	tPlaceObjectsAction::tPlaceObjectsAction( tToolsGuiMainWindow& mainWindow, const tGrowableArray<tEntityPtr>& entities, b32 deleteObjs )
		: tEditorButtonManagedCursorAction( mainWindow )
		, mEntities( entities )
		, mDeleteObjects( deleteObjs )
	{
		mSavedSelection.fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fRedo( );
	}
	void tPlaceObjectsAction::fUndo( )
	{
		if( mDeleteObjects )
			fAddToWorld( );
		else
			fRemoveFromWorld( );
	}
	void tPlaceObjectsAction::fRedo( )
	{
		if( mDeleteObjects )
			fRemoveFromWorld( );
		else
			fAddToWorld( );
	}
	void tPlaceObjectsAction::fAddToWorld( )
	{
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fClear( );
		for( u32 i = 0; i < mEntities.fCount( ); ++i )
			mEntities[ i ]->fDynamicCast< tEditableObject >( )->fAddToWorld( );
	}
	void tPlaceObjectsAction::fRemoveFromWorld( )
	{
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( mSavedSelection );
		for( u32 i = 0; i < mEntities.fCount( ); ++i )
			mEntities[ i ]->fDynamicCast< tEditableObject >( )->fRemoveFromWorld( );
	}


	tPlaceObjectCursorBase::tPlaceObjectCursorBase( tToolsGuiMainWindow& mainWindow, const tEntityPtr& entityMaster, const Math::tVec3f& offset )
		: mMainWindow( mainWindow )
		, mEntityMaster( entityMaster )
		, mOffset( offset )
	{
	}

	tPlaceObjectCursorBase::~tPlaceObjectCursorBase( )
	{
		if( mEntityMaster )
			mEntityMaster->fDeleteImmediate( );
	}

	void tPlaceObjectCursorBase::fOnTick( )
	{
		if( !mEntityMaster )
			return;

		Math::tRayf pickRay;
		if( !fComputePickRay( pickRay ) )
			return;

		if( !mEntityMaster->fSceneGraph( ) )
			mEntityMaster->fSpawnImmediate( mMainWindow.fGuiApp( ).fSceneGraph( )->fRootEntity( ) );

		tWxRenderPanelContainer* gfx = mMainWindow.fRenderPanelContainer( );
		tWxRenderPanel* panel = gfx->fGetActiveRenderPanel( );

		Math::tVec3f p;

		{
			tEntity* ignoreSelf = mEntityMaster.fGetRawPtr( );

			f32 tOut = 0.f;	
			if( !panel->fSnapToGrid( ) && mMainWindow.fGuiApp( ).fEditableObjects( ).fPick( pickRay, &tOut, &ignoreSelf, 1 ) )
			{
				// use intersection with closest object ray-cast
				p = pickRay.fEvaluate( tOut );
			}
			else
			{
				// use lookat
				if( panel->fSnapToGrid( ) )
					p = panel->fGetGridCenter( );
				else
					p = panel->fGetScreen( )->fViewport( 0 )->fLogicCamera( ).fGetTripod( ).mLookAt;

				Math::tVec3f n;
				if( gfx->fPanelIsMain( panel ) )
				{
					// If the panel is the main panel, it needs to pick against a plane formed with the up vector.
					n = Math::tVec3f::cYAxis;
				}
				else // Otherwise, it's a top/front/right view and should pick against a plane orthogonal to the view vector.
					n = p - panel->fGetScreen( )->fViewport( 0 )->fLogicCamera( ).fGetTripod( ).mEye;

				// intersect ray with lookat plane, and use that intersection if possible
				Math::tIntersectionRayPlane<f32> intersect( pickRay, Math::tPlanef( n, p ) );
				if( intersect.fIntersects( ) && !intersect.fParallel( ) )
				{
					p = pickRay.fEvaluate( intersect.fT( ) );

					if( panel->fSnapToGrid( ) )
						panel->fSnapVertex( p );
				}
			}
		}

		mEntityMaster->fDynamicCast< tEditableObject >( )->fMoveTo( p + mOffset );

		const Input::tMouse& mouse = panel->fGetMouse( );
		if( !mMainWindow.fPriorityInputActive( ) && mouse.fButtonDown( Input::tMouse::cButtonLeft ) )
		{
			tEntityPtr newEntity = mEntityMaster->fDynamicCast< tEditableObject >( )->fClone( );
			mMainWindow.fGuiApp( ).fActionStack( ).fBeginCompoundAction( );
			tEditorActionPtr action( new tPlaceObjectsAction( mMainWindow, newEntity ) );
			fOnEntityPlaced( newEntity );
			mMainWindow.fGuiApp( ).fActionStack( ).fEndCompoundAction( action );
		}
	}

	tPlaceObjectCursor::tPlaceObjectCursor( tEditorCursorControllerButton* button, const tEntityPtr& entityMaster, const Math::tVec3f& offset )
		: tEditorButtonManagedCursorController( button )
		, tPlaceObjectCursorBase( button->fGetParent( )->fMainWindow( ), entityMaster, offset )
	{
	}

	tPlaceObjectCursor::tPlaceObjectCursor( tToolsGuiMainWindow& mainWindow, const tEntityPtr& entityMaster, const Math::tVec3f& offset )
		: tEditorButtonManagedCursorController( mainWindow.fGuiApp( ) )
		, tPlaceObjectCursorBase( mainWindow, entityMaster, offset )
	{
	}
	
	void tPlaceObjectCursor::fOnTick( )
	{
		tPlaceObjectCursorBase::fOnTick( );
	}

	b32 tPlaceObjectCursor::fComputePickRay( Math::tRayf& pickRay )
	{
		return tEditorCursorController::fComputePickRay( pickRay );
	}

	void tPlaceObjectCursor::fOnNextCursor( tEditorCursorController* nextController )
	{
		if( mEntityMaster )
		{
			mEntityMaster->fDeleteImmediate( );
			mEntityMaster.fRelease( );
		}

		tEditorButtonManagedCursorController::fOnNextCursor( nextController );
	}

}

