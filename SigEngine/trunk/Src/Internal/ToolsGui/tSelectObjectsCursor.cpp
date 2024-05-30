#include "ToolsGuiPch.hpp"
#include "tSelectObjectsCursor.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "Editor\tEditableGroupEntity.hpp"

namespace Sig
{
	tFreezeHideAction::tFreezeHideAction( tToolsGuiMainWindow& mainWindow, tEditableObject::tState state )
		: tEditorButtonManagedCursorAction( mainWindow ), mState( state )
	{
		fMainWindow( ).fGuiApp( ).fEditableObjects( ).fCopyObjectSet( mOldSet );
	}
	void tFreezeHideAction::fFinishConstruction( )
	{
		fMainWindow( ).fGuiApp( ).fEditableObjects( ).fCopyObjectSet( mNewSet );
	}
	void tFreezeHideAction::fUndo( )
	{
		for( u32 i = 0; i < mOldSet[ mState ].fCount( ); ++i )
			mOldSet[ mState ][ i ]->fDynamicCast< tEditableObject >( )->fSetState( mState );
		for( u32 i = 0; i < mOldSet[ tEditableObject::cStateShown ].fCount( ); ++i )
			mOldSet[ tEditableObject::cStateShown ][ i ]->fDynamicCast< tEditableObject >( )->fSetState( tEditableObject::cStateShown );
	}
	void tFreezeHideAction::fRedo( )
	{
		for( u32 i = 0; i < mNewSet[ mState ].fCount( ); ++i )
		{
			fMainWindow( ).fGuiApp( ).fSelectionList( ).fRemove( mNewSet[ mState ][ i ] );
			mNewSet[ mState ][ i ]->fDynamicCast< tEditableObject >( )->fSetState( mState );
		}
		for( u32 i = 0; i < mNewSet[ tEditableObject::cStateShown ].fCount( ); ++i )
			mNewSet[ tEditableObject::cStateShown ][ i ]->fDynamicCast< tEditableObject >( )->fSetState( tEditableObject::cStateShown );
	}

	tModifySelectionAction::tModifySelectionAction( tToolsGuiMainWindow& mainWindow, const tEditorSelectionList& prevState )
		: tEditorButtonManagedCursorAction( mainWindow )
		, mUndoState( prevState )
		, mRedoState( mainWindow.fGuiApp( ).fSelectionList( ) ) 
	{
		fMainWindow( ).fGuiApp( ).fPrevSelectionList( ).fReset( prevState, false );
	}
	void tModifySelectionAction::fUndo( ) 
	{ 
		fMainWindow( ).fGuiApp( ).fPrevSelectionList( ).fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( mUndoState ); 
	}
	void tModifySelectionAction::fRedo( ) 
	{ 
		fMainWindow( ).fGuiApp( ).fPrevSelectionList( ).fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( mRedoState );

	}

	tDeleteSelectedObjectsAction::tDeleteSelectedObjectsAction( tToolsGuiMainWindow& mainWindow )
		: tEditorButtonManagedCursorAction( mainWindow )
	{
		// save list of currently selected objects; these are what get deleted
		mEntities.fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fRedo( );
	}
	void tDeleteSelectedObjectsAction::fUndo( )
	{
		// Note: the list is undone in reverse order so that re-inserting decal
		// nodes will function properly. And the order doesn't have an affect on
		// any other type of deletion.
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fClear( );
		for( s32 i = mEntities.fCount( )-1; i >= 0; --i )
		{
			mEntities[ i ]->fDynamicCast< tEditableObject >( )->fAddToWorld( );
			fMainWindow( ).fGuiApp( ).fSelectionList( ).fAdd( mEntities[ i ] );
		}
	}
	void tDeleteSelectedObjectsAction::fRedo( )
	{
		for( u32 i = 0; i < mEntities.fCount( ); ++i )
			mEntities[ i ]->fDynamicCast< tEditableObject >( )->fRemoveFromWorld( );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fClear( );
	}

	tGroupSelectedObjectsAction::tGroupSelectedObjectsAction( tToolsGuiMainWindow& mainWindow )
		: tEditorButtonManagedCursorAction( mainWindow )
	{
		mEntities.fReset( fMainWindow().fGuiApp().fEditableObjects().fGetSelectionList(), false );

		tEditableGroupEntity* master = new tEditableGroupEntity( fMainWindow().fGuiApp().fEditableObjects() );
		mMaster.fReset( master );

		fRedo( );
	}
	void tGroupSelectedObjectsAction::fUndo( )
	{
		tEditableGroupEntity* master = mMaster->fDynamicCast< tEditableGroupEntity >( );
		master->fEmptyObjects( );
		master->fRemoveFromWorld( );

		fMainWindow( ).fGuiApp( ).fPrevSelectionList( ).fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( mEntities );
	}
	void tGroupSelectedObjectsAction::fRedo( )
	{
		tEditableGroupEntity* master = mMaster->fDynamicCast< tEditableGroupEntity >( );
		master->fAddToWorld( );
		master->fAddObjects( mEntities );

		fMainWindow( ).fGuiApp( ).fPrevSelectionList( ).fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fClear( );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fAdd( mMaster );
	}

	tBreakSelectedGroupsAction::tBreakSelectedGroupsAction( tToolsGuiMainWindow& mainWindow )
		: tEditorButtonManagedCursorAction( mainWindow )
	{
		tGrowableArray< tEditableGroupEntity* > groupEntities;
		fMainWindow().fGuiApp().fEditableObjects().fGetSelectionList().fCullByType< tEditableGroupEntity >( groupEntities );

		mRecords.fSetCount( groupEntities.fCount() );
		for( u32 i = 0; i < groupEntities.fCount(); ++i )
		{
			mRecords[i].mMaster.fReset( groupEntities[i] );
			mRecords[i].mEntities.fReset( groupEntities[i]->fGetObjects() );
		}

		fRedo();
	}
	void tBreakSelectedGroupsAction::fUndo( )
	{
		fMainWindow( ).fGuiApp( ).fPrevSelectionList( ).fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fClear( );

		tEditorSelectionList newList;
		for( u32 i = 0; i < mRecords.fCount(); ++i )
		{
			tGroupRecord& thisRecord = mRecords[i];
			tEditableGroupEntity* master = mRecords[i].mMaster->fDynamicCast< tEditableGroupEntity >( );
			master->fAddToWorld( );
			master->fAddObjects( mRecords[i].mEntities );
			
			newList.fAdd( tEntityPtr(master), false );
		}

		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( newList );
	}
	void tBreakSelectedGroupsAction::fRedo( )
	{
		fMainWindow( ).fGuiApp( ).fPrevSelectionList( ).fReset( fMainWindow( ).fGuiApp( ).fSelectionList( ), false );
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fClear( );

		tEditorSelectionList newList;
		for( u32 i = 0; i < mRecords.fCount(); ++i )
		{
			tGroupRecord& thisRecord = mRecords[i];
			tEditableGroupEntity* master = mRecords[i].mMaster->fDynamicCast< tEditableGroupEntity >( );
			master->fEmptyObjects( );
			master->fRemoveFromWorld( );

			for( u32 ent = 0; ent < thisRecord.mEntities.fCount(); ++ent )
				newList.fAdd( thisRecord.mEntities[ent], false );
		}

		fMainWindow( ).fGuiApp( ).fSelectionList( ).fReset( newList );
	}

	class tToggleFreezeOnHoverObjectHotKey : public tEditorHotKey
	{
		tSelectObjectsCursor* mSelectObjectsCursor;
	public:
		tToggleFreezeOnHoverObjectHotKey( tEditorHotKeyTable& table, tSelectObjectsCursor* selectObjectsCursor ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonU, 0 ), mSelectObjectsCursor( selectObjectsCursor ) { }
		virtual void fFire( ) const { mSelectObjectsCursor->fToggleFreezeOnHoverObject( ); }
	};


	tSelectObjectsCursor::tSelectObjectsCursor( tEditorCursorControllerButton* button, const char* statusText, const tManipulationGizmoPtr& gizmo )
		: tEditorButtonManagedCursorController( button )
		, mSelectionRectangle( new Gfx::tSolidColorLines( ) )
		, mGizmo( gizmo )
		, mIsGizmoInWorldSpace( false )
		, mGizmoScale( 1.f )
		, mSelectionPanel( 0 )
		, mSelectionStarted( false )
		, mCursorSelectStart( Math::tVec2f::cZeroVector )
	{
		fMainWindow( ).fSetStatus( statusText );

		tWxRenderPanelContainer* gfx = fMainWindow( ).fRenderPanelContainer( );
		mSelectionRectangle->fResetDeviceObjects( 
			fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) );

		mHotKeys.fPushBack( tEditorHotKeyPtr( new tToggleFreezeOnHoverObjectHotKey( fMainWindow( ).fGuiApp( ).fHotKeys( ), this ) ) );
	}

	tSelectObjectsCursor::~tSelectObjectsCursor( )
	{
	}

	void tSelectObjectsCursor::fOnTick( )
	{
		fHandleGizmo( );
		fHandleHover( );
		fHandleSelection( );
		fHandleDelete( );
	}

	void tSelectObjectsCursor::fOnNextCursor( tEditorCursorController* nextController )
	{
		// clear selection list if current cursor is not a selection-based cursor
		tSelectObjectsCursor* currentCursor = dynamic_cast<tSelectObjectsCursor*>( nextController );
		if( !currentCursor )
			fClearSelection( );
		tEditorButtonManagedCursorController::fOnNextCursor( nextController );
	}

	void tSelectObjectsCursor::fToggleFreezeOnHoverObject( )
	{
		tWxRenderPanel* renderPanel = fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		if( !renderPanel ) return;
		if( !renderPanel->fGetMouse( ).fButtonHeld( Input::tMouse::cButtonLeft ) ) return;
		if( mCurrentHoverObject.fNull( ) ) return;

		tEditableObject* eo = mCurrentHoverObject->fDynamicCast< tEditableObject >( );
		sigassert( eo );

		if( !mGizmo.fNull( ) && mGizmo->fGetDragging( ) )
			mGizmo->fFinishDrag( );

		if( eo->fGetSelected( ) )
			fMainWindow( ).fGuiApp( ).fSelectionList( ).fRemove( mCurrentHoverObject );

		tFreezeHideAction* action = new tFreezeHideAction( fMainWindow( ), tEditableObject::cStateFrozen );
		eo->fFreeze( !eo->fIsFrozen( ) );
		action->fFinishConstruction( );
		fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
	}

	void tSelectObjectsCursor::fSetWorldCoords( const Math::tVec3f& worldCoords, b32 doX, b32 doY, b32 doZ )
	{
		if( !mGizmo.fNull( ) && fMainWindow( ).fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			mGizmo->fSpecifyWorldCoords( fMainWindow( ), fMainWindow( ).fGuiApp( ).fSelectionList( ), worldCoords, doX, doY, doZ );
	}

	Math::tVec3f tSelectObjectsCursor::fGetWorldCoords( )
	{
		Math::tVec3f worldCoords = Math::tVec3f( 0.f );
		if( !mGizmo.fNull( ) && fMainWindow( ).fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			mGizmo->fExtractWorldCoords( fMainWindow( ).fGuiApp( ).fSelectionList( ), worldCoords );
		return worldCoords;
	}

	b32 tSelectObjectsCursor::fGetDefaultCoords( Math::tVec3f& coords )
	{
		if( !mGizmo.fNull( ) && fMainWindow( ).fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
		{
			coords = mGizmo->fGetDefaultCoords( );
			return true;
		}

		return false;
	}

	void tSelectObjectsCursor::fClearSelection( )
	{
		// N.B.! this method deliberately DOES NOT
		// track using an editor action; this is meant
		// for intermediary operations while doing
		// larger selection set modifications

		// de-select currently selected objects
		fMainWindow( ).fGuiApp( ).fSelectionList( ).fClear( );
	}

	b32 tSelectObjectsCursor::fDoToolTipOverHoverObject( )
	{
		return tEditorCursorController::fDoToolTipOverHoverObject( ) &&
				!mSelectionStarted && 
				( mGizmo.fNull( ) || !mGizmo->fGetDragging( ) );
	}

	void tSelectObjectsCursor::fHandleSelectionMouseDown( tWxRenderPanel* panel, const Input::tMouse& mouse, const Input::tKeyboard& kb )
	{
		mSelectionStarted = true;
		mSelectionPanel = panel;
		mCursorSelectStart.x = ( f32 )mouse.fGetState( ).mCursorPosX;
		mCursorSelectStart.y = ( f32 )mouse.fGetState( ).mCursorPosY;
		mStartStamp = Time::fGetStamp( );
	}

	void tSelectObjectsCursor::fHandleSelectionMouseUp( tWxRenderPanel* panel, const Input::tMouse& mouse, const Input::tKeyboard& kb )
	{
		if( !mSelectionStarted )
			return;
		mSelectionStarted = false;

		tEntityIntersectionList intersection;
		intersection.fSetCapacity( 256 );
 		fAcquireIntersectedEntities( mouse, intersection );

		if( intersection.fCount( ) > 0 )
		{
			// button was pressed to select/deselect
			if( kb.fButtonHeld( Input::tKeyboard::cButtonLCtrl ) ||
				kb.fButtonHeld( Input::tKeyboard::cButtonRCtrl ) )
			{
				// save selection state
				tEditorSelectionList savedSelection = fMainWindow( ).fGuiApp( ).fSelectionList( );

				// track whether we actually change the selection set
				b32 selModified = false;

				// remove these objects from selection
				for( u32 i = 0; i < intersection.fCount( ); ++i )
				{
					if( intersection[ i ]->fDynamicCast< tEditableObject >( )->fGetSelected( ) )
					{
						// mark selection as modified
						selModified = true;

						// remove object
						fMainWindow( ).fGuiApp( ).fSelectionList( ).fRemove( intersection[ i ] );
					}
				}

				if( selModified )
				{
					// add action for undo/redo (only if we actually changed anything)
					fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( fMainWindow( ), savedSelection ) ) );
				}
			}
			else
			{
				// save selection state
				tEditorSelectionList savedSelection = fMainWindow( ).fGuiApp( ).fSelectionList( );

				// track whether we actually change the selection set
				b32 selModified = false;

				if( !kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) &&
					!kb.fButtonHeld( Input::tKeyboard::cButtonRShift ) )
				{
					// user is not adding object to selection, so clear it
					fClearSelection( );

					// mark selection modified
					selModified = true;
				}

				// add these objects to selection
				for( u32 i = 0; i < intersection.fCount( ); ++i )
				{
					tEditableObject* eo = intersection[ i ]->fDynamicCast< tEditableObject >( );
					if( !eo->fGetSelected( ) && !eo->fIsFrozen( ) )
					{
						// mark selection as modified
						selModified = true;

						// select current hover object
						fMainWindow( ).fGuiApp( ).fSelectionList( ).fAdd( intersection[ i ] );
					}
				}

				if( selModified )
				{
					// add action for undo/redo (only if we actually changed anything)
					fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( fMainWindow( ), savedSelection ) ) );
				}
			}
		}
		else if( fMainWindow( ).fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
		{
			// save selection state
			tEditorSelectionList savedSelection = fMainWindow( ).fGuiApp( ).fSelectionList( );

			fClearSelection( );

			// add action for undo/redo
			fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( fMainWindow( ), savedSelection ) ) );
		}
	}

	void tSelectObjectsCursor::fUpdateSelectionDrag( tWxRenderPanel* panel, const Input::tMouse& mouse, const Input::tKeyboard& kb )
	{
		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		verts.fSetCapacity( 128 );

		const u32 color = Gfx::tVertexColor( 0xff, 0x00, 0x00, 0xff ).fForGpu( );

		Math::tVec2f startPoint = mCursorSelectStart;
		Math::tVec2f endPoint( ( f32 )mouse.fGetState( ).mCursorPosX, ( f32 )mouse.fGetState( ).mCursorPosY );

		if( endPoint.x < startPoint.x )
			fSwap( endPoint.x, startPoint.x );
		if( endPoint.y < startPoint.y )
			fSwap( endPoint.y, startPoint.y );

		// dx and dy are in pixels (or should be, as long as the screen space camera projection is)
		const f32 dx = 5.f;
		const f32 dy = 5.f;

		for( f32 x = startPoint.x; x + dx < endPoint.x; x += 2.f * dx )
		{
			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( x, startPoint.y, 0.1f ), color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( x + dx, startPoint.y, 0.1f ), color ) );

			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( x, endPoint.y, 0.1f ), color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( x + dx, endPoint.y, 0.1f ), color ) );
		}

		for( f32 y = startPoint.y; y + dy < endPoint.y; y += 2.f * dy )
		{
			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( startPoint.x, y, 0.1f ), color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( startPoint.x, y + dy, 0.1f ), color ) );

			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( endPoint.x, y, 0.1f ), color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( endPoint.x, y + dy, 0.1f ), color ) );
		}

		if( verts.fCount( ) == 0 )
			return;

		// bake the verts
		mSelectionRectangle->fBake( ( Sig::byte* )verts.fBegin( ), verts.fCount( ), false );

		// add the draw call
		panel->fGetScreen( )->fAddScreenSpaceDrawCall( Gfx::tDrawCall( *mSelectionRectangle, 0.5f ) );
	}

	void tSelectObjectsCursor::fAcquireIntersectedEntities( const Input::tMouse& mouse, tEntityIntersectionList& intersection )
	{
		if( mCurrentHoverObject )
			intersection.fPushBack( mCurrentHoverObject );

		Math::tVec2f startPoint = mCursorSelectStart;
		Math::tVec2f endPoint( ( f32 )mouse.fGetState( ).mCursorPosX, ( f32 )mouse.fGetState( ).mCursorPosY );
		if( endPoint.x < startPoint.x )
			fSwap( endPoint.x, startPoint.x );
		if( endPoint.y < startPoint.y )
			fSwap( endPoint.y, startPoint.y );

		// ensure it's not a degenerate box
		endPoint.x += 1;
		endPoint.y += 1;

		// clip the start/end to the window size.
		const f32 panelX = (f32)mSelectionPanel->GetSize( ).x, panelY = (f32)mSelectionPanel->GetSize( ).y;
		startPoint.x = fMax( startPoint.x, 0.f );
		startPoint.y = fMax( startPoint.y, 0.f );
		startPoint.x = fMin( startPoint.x, panelX );
		startPoint.y = fMin( startPoint.y, panelY );

		endPoint.x = fMax( endPoint.x, 0.f );
		endPoint.y = fMax( endPoint.y, 0.f );
		endPoint.x = fMin( endPoint.x, panelX );
		endPoint.y = fMin( endPoint.y, panelY );

		const Math::tVec2f delta = endPoint - startPoint;

		const f32 minDragPixels = 5.f;
		if( fAbs( delta.x ) < minDragPixels && fAbs( delta.y ) < minDragPixels )
			return; // drag was small enough to ignore the resulting frustum

		// If this was a super fast click, disregard it because it probably wasn't
		// meant to be a frustum selection. 150ms is really generous for click length.
		const f32 maxSweetPixels = 20.f;
		if( fAbs( delta.x ) < maxSweetPixels && fAbs( delta.y ) < maxSweetPixels && Time::fGetElapsedMs( mStartStamp, Time::fGetStamp( ) ) < 150 )
			return;

		// build frustum to use for intersecting against objects

		const Gfx::tCamera& worldCamera = mSelectionPanel->fGetScreen( )->fViewport( 0 )->fLogicCamera( );
		const Math::tVec2u panelSize( mSelectionPanel->GetSize( ).x, mSelectionPanel->GetSize( ).y );

		Math::tFrustumf frustum;
		worldCamera.fComputePickFrustum( frustum,
				Math::tVec2u( ( u32 )startPoint.x,	( u32 )startPoint.y ),
				Math::tVec2u( ( u32 )endPoint.x,	( u32 )startPoint.y ),
				Math::tVec2u( ( u32 )startPoint.x,	( u32 )endPoint.y ),
				Math::tVec2u( ( u32 )endPoint.x,	( u32 )endPoint.y ),
				panelSize );

		intersection.fSetCount( 0 );
		fMainWindow( ).fGuiApp( ).fEditableObjects( ).fIntersect( frustum, intersection );
	}

	void tSelectObjectsCursor::fHandleSelection( )
	{
		tWxRenderPanelContainer* gfx = fMainWindow( ).fRenderPanelContainer( );
		if( !gfx->fGetActiveRenderPanel( ) )
			return; // no panels are active (means cursor is no where in any viewports)
		tWxRenderPanel* panel = gfx->fGetFocusRenderPanel( );
		if( !panel )
			return; // cursor is not currently over a render panel
		if( !mGizmo.fNull( ) && mGizmo->fGetDragging( ) )
			return; // don't change selection while dragging
		if( fMainWindow( ).fPriorityInputActive( ) )
			return; // camera is currently handling input, come back later

		const Input::tMouse& mouse = panel->fGetMouse( );
		const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );

		if( mouse.fButtonDown( Input::tMouse::cButtonLeft ) )
			fHandleSelectionMouseDown( panel, mouse, kb );
		else if( mouse.fButtonUp( Input::tMouse::cButtonLeft ) )
			fHandleSelectionMouseUp( panel, mouse, kb );
		else if( mSelectionStarted )
			fUpdateSelectionDrag( panel, mouse, kb );
	}

	void tSelectObjectsCursor::fHandleDelete( )
	{
		tWxRenderPanelContainer* gfx = fMainWindow( ).fRenderPanelContainer( );
		tWxRenderPanel* panel = gfx->fGetActiveRenderPanel( );
		if( !panel )
			return;
		if( !mGizmo.fNull( ) && mGizmo->fGetDragging( ) )
			return; // don't delete selection while dragging

		const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );

		if( !fMainWindow( ).fPriorityInputActive( ) &&
			kb.fButtonDown( Input::tKeyboard::cButtonDelete ) )
		{
			fMainWindow( ).fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tDeleteSelectedObjectsAction( fMainWindow( ) ) ) );
		}
	}

	void tSelectObjectsCursor::fHandleGizmo( )
	{
		if( mGizmo.fNull( ) )
			return; // no gizmo
		
		mGizmo->fTick( fMainWindow( ), fMainWindow( ).fGuiApp( ).fSelectionList( ) );

		if( fMainWindow( ).fGuiApp( ).fSelectionList( ).fCount( ) == 0 )
			return; // no selected objects

		tWxRenderPanelContainer* gfx = fMainWindow( ).fRenderPanelContainer( );

		// we need one gizmo renderable per viewable window, so make sure we have enough
		if( mGizmoPerWindow.fCount( ) != gfx->fRenderPanelCount( ) )
			mGizmoPerWindow.fSetCount( gfx->fRenderPanelCount( ) );

		// now go through each render panel window and adjust the gizmo for rendering in that viewport;
		// the reason for this messiness is that the gizmo needs to scale differently for each viewport,
		// requiring different renderable instances for each one
		u32 i = 0;
		for( tWxRenderPanel** ipanel = gfx->fRenderPanelsBegin( ); ipanel != gfx->fRenderPanelsEnd( ); ++ipanel, ++i )
		{
			if( !*ipanel || !(*ipanel)->fIsVisible( ) )
				continue;
			if( mGizmoPerWindow[ i ].fNull( ) )
				mGizmoPerWindow[ i ].fReset( new tGizmoRenderable( mGizmo->fGetGizmoGeometry( ) ) );

			tWxRenderPanel* panel = *ipanel;
			const Gfx::tCamera& camera = panel->fGetScreen( )->fViewport( 0 )->fLogicCamera( );

			// compute the local-to-world xform for this instance of the gizmo
			mGizmoPerWindow[ i ]->fComputeLocalToWorld( *mGizmo, fMainWindow( ).fGuiApp( ).fSelectionList( ), camera, mGizmoScale, mIsGizmoInWorldSpace );
			// add the draw call
			panel->fGetScreen( )->fAddWorldSpaceTopDrawCall( mGizmoPerWindow[ i ]->fGizmoRenderableEntity( ) );

			// now we actually handle picking/dragging/etc. for the gizmo
			if( !fMainWindow( ).fPriorityInputActive( ) &&
				 panel->fGetMouse( ).fCursorInClientArea( ) )
			{
				Math::tRayf pickRay;
				if( fComputePickRay( pickRay ) )
				{
					mGizmo->fUpdate( 
						fMainWindow( ),
						fMainWindow( ).fGuiApp( ).fSelectionList( ), 
						mGizmoPerWindow[ i ]->fLocalToWorld( ), 
						pickRay, 
						camera, 
						panel->fGetMouse( ), 
						Input::tKeyboard::fInstance( ) );
				}
			}
		}
	}
}

