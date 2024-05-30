#include "ToolsGuiPch.hpp"
#include "tEditorCursorController.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "Editor/tEditableObjectContainer.hpp"

namespace Sig
{

	tEditorCursorController::tEditorCursorController( tToolsGuiApp& guiApp ) 
		: mGuiApp( guiApp )
		, mLastHoverIntersection( Math::tVec3f::cZeroVector )
	{
		// clear current cursor's hot keys
		if( !fGuiApp( ).fCurrentCursor( ).fNull( ) )
			fGuiApp( ).fCurrentCursor( )->mHotKeys.fDeleteArray( );

		tWxRenderPanelContainer* gfx = fGuiApp( ).fMainWindow( ).fRenderPanelContainer( );

		mToolTip.fSetDevFont( );
	}

	tEditorCursorController::~tEditorCursorController( )
	{
		if( !fGuiApp( ).fShuttingDown( ) )
		{
			if( mCurrentHoverObject )
				mCurrentHoverObject->fDynamicCast< tEditableObject >( )->fDisableSelectionBox( );
		}
	}

	b32 tEditorCursorController::fComputePickRay( Math::tRayf& rayOut )
	{
		tWxRenderPanelContainer* gfx = fGuiApp( ).fMainWindow( ).fRenderPanelContainer( );
		tWxRenderPanel* panel = gfx->fGetActiveRenderPanel( );
		if( !panel )
			return false; // cursor is not over any render panels, nothing to do

		const Gfx::tScreenPtr& screen = panel->fGetScreen( );
		if( screen->fGetViewportCount( ) != 1 )
			return false; // we only handle screens with one viewport

		const Input::tMouse& mouse = panel->fGetMouse( );
		const Gfx::tCamera& camera = screen->fViewport( 0 )->fLogicCamera( );
		const wxSize panelSize = panel->GetSize( );

		rayOut = camera.fComputePickRay( 
			Math::tVec2u( mouse.fGetState( ).mCursorPosX, mouse.fGetState( ).mCursorPosY ),
			Math::tVec2u( panelSize.x, panelSize.y ) );

		return true;
	}

	tEntityPtr tEditorCursorController::fPick( const Math::tRayf& ray, f32* bestTout, tEntity* const* ignoreList, u32 numToIgnore )
	{
		return fGuiApp( ).fEditableObjects( ).fPick( ray, bestTout, ignoreList, numToIgnore );
	}

	b32 tEditorCursorController::fDoToolTipOverHoverObject( )
	{
		tWxRenderPanel* renderPanel = fGuiApp( ).fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		return 
			renderPanel && 
			mCurrentHoverObject && 
			!fGuiApp( ).fMainWindow( ).fPriorityInputActive( );
	}

	void tEditorCursorController::fHandleHover( )
	{
		tEntityPtr newHoverObject;

		Math::tRayf pickRay;
		if( fComputePickRay( pickRay ) )
		{
			// see if we hit anything
			f32 tOut = 0.f;	
			newHoverObject = fPick( pickRay, &tOut );

			// store point of intersection
			if( newHoverObject )
				mLastHoverIntersection = pickRay.fPointAtTime( tOut );

			// see if derived type accepts this object type as a hoverable object
			newHoverObject = fFilterHoverObject( newHoverObject );
		}

		if( mCurrentHoverObject && ( newHoverObject != mCurrentHoverObject ) )
		{
			if( !mCurrentHoverObject->fDynamicCast< tEditableObject >( )->fGetSelected( ) )
				mCurrentHoverObject->fDynamicCast< tEditableObject >( )->fDisableSelectionBox( );
		}
		if( newHoverObject && !fGuiApp( ).fMainWindow( ).fPriorityInputActive( ) )
		{
			tEditableObject* eo = newHoverObject->fDynamicCast< tEditableObject >( );
			if( !eo->fIsFrozen( ) && eo->fIsSelectable() )
				eo->fEnableSelectionBox( );
		}
		mCurrentHoverObject = newHoverObject;

		tWxRenderPanel* renderPanel = fGuiApp( ).fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( );
		if( renderPanel )
		{
			if( fDoToolTipOverHoverObject( ) )
			{
				const std::string toolTip = mCurrentHoverObject->fDynamicCast< tEditableObject >( )->fGetToolTip( );
				if( toolTip.length( ) > 0 )
				{
					mToolTip.fBake( toolTip.c_str( ), 0, Gui::tText::cAlignLeft );
					mToolTip.fSetRgbaTint( Math::tVec4f( 1.f, 1.f, 1.f, 1.0f ) );
					mToolTip.fSetPosition( Math::tVec3f( renderPanel->fGetMouse( ).fGetState( ).mCursorPosX + 15, renderPanel->fGetMouse( ).fGetState( ).mCursorPosY, 0.f ) );
					renderPanel->fGetScreen( )->fAddScreenSpaceDrawCall( mToolTip.fDrawCall( ) );
				}
			}
		}
	}

	tToolsGuiMainWindow& tEditorCursorController::fMainWindow( ) { return mGuiApp.fMainWindow( ); }

}

