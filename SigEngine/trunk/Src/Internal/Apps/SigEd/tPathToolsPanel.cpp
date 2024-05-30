#include "SigEdPch.hpp"
#include "tPathToolsPanel.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tWxToolsPanelSlider.hpp"
#include "tEditorAppWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "Editor/tEditableWaypointEntity.hpp"

namespace Sig
{

	class tConnectWaypointsAction : public tEditorButtonManagedCursorAction
	{
		tEditableWaypointEntityPtr mPrevWaypoint, mWaypoint;
	public:
		tConnectWaypointsAction( tToolsGuiMainWindow& mainWindow, tEditableWaypointEntity& prevWaypoint, tEditableWaypointEntity& waypoint )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mPrevWaypoint( &prevWaypoint )
			, mWaypoint( &waypoint )
		{
			fRedo( );
		}
		virtual void fUndo( )
		{
			mPrevWaypoint->fConnect( mWaypoint.fGetRawPtr( ), true );
		}
		virtual void fRedo( )
		{
			mPrevWaypoint->fConnect( mWaypoint.fGetRawPtr( ), false );
		}
	};

	class tDisconnectWaypointAction : public tEditorButtonManagedCursorAction
	{
		tEditableWaypointBasePtr mWaypoint;
		tGrowableArray<tEditableWaypointBasePtr> mConnections;
	public:
		tDisconnectWaypointAction( tToolsGuiMainWindow& mainWindow, tEditableWaypointEntity& waypoint )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mWaypoint( &waypoint )
			, mConnections( waypoint.fConnections( ) )
		{
			fRedo( );
		}
		virtual void fUndo( )
		{
			for( u32 i = 0; i < mConnections.fCount( ); ++i )
				mWaypoint->fConnect( mConnections[ i ].fGetRawPtr( ) );
		}
		virtual void fRedo( )
		{
			mWaypoint->fDisconnect( true );
		}
	};

	class tCreatePathTool : public tPlaceObjectCursor
	{
		tEditableWaypointEntityPtr mPrevWaypoint;
	public:
		tCreatePathTool( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableWaypointEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ), Math::tVec3f(0.f,1.0f,0.f) )
		{
			fMainWindow( ).fSetStatus( "Create Path" );
		}
		virtual void fOnEntityPlaced( const tEntityPtr& placedEntity )
		{
			tEditableWaypointEntity* waypoint = placedEntity->fDynamicCast< tEditableWaypointEntity >( );
			if( mPrevWaypoint )
			{
				const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );
				if( kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || kb.fButtonHeld( Input::tKeyboard::cButtonRShift ) )
					fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tConnectWaypointsAction( fGuiApp( ).fMainWindow( ), *waypoint, *mPrevWaypoint ) ) );
				else
					fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tConnectWaypointsAction( fGuiApp( ).fMainWindow( ), *mPrevWaypoint, *waypoint ) ) );
			}

			mPrevWaypoint.fReset( waypoint );
		}
	};
	class tCreatePathToolButton : public tEditorCursorControllerButton
	{
	public:
		tCreatePathToolButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PathToolCreatePathSel" ), wxBitmap( "PathToolCreatePathDeSel" ), "Place connected series of waypoints - hold shift to reverse direction" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tCreatePathTool( this ) );
		}
	};


	class tPathToolConnect : public tEditorButtonManagedCursorController
	{
		tEditableWaypointEntityPtr mPrevWaypoint;
	public:
		tPathToolConnect( tEditorCursorControllerButton* button )
			: tEditorButtonManagedCursorController( button )
		{
			fMainWindow( ).fSetStatus( "Connect Waypoints" );
		}
		virtual void fOnTick( )	
		{
			fHandleHover( );

			tWxRenderPanelContainer* gfx = fMainWindow( ).fRenderPanelContainer( );
			tWxRenderPanel* panel = gfx->fGetActiveRenderPanel( );
			if( !panel )
				return;
			const Input::tMouse& mouse = panel->fGetMouse( );
			const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );

			if( mouse.fButtonDown( Input::tMouse::cButtonLeft ) )
				fHandleMouseDown( panel, mouse, kb );
		}
		void fHandleMouseDown( tWxRenderPanel* panel, const Input::tMouse& mouse, const Input::tKeyboard& kb )
		{
			if( !mCurrentHoverObject )
			{
				mPrevWaypoint.fRelease( );
				return;
			}
			tEditableWaypointEntity* waypoint = mCurrentHoverObject->fDynamicCast< tEditableWaypointEntity >( );
			if( !waypoint )
			{
				mPrevWaypoint.fRelease( );
				return;
			}

			const b32 shiftDown = ( kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || kb.fButtonHeld( Input::tKeyboard::cButtonRShift ) );

			if( shiftDown )
			{
				fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tDisconnectWaypointAction( fGuiApp( ).fMainWindow( ), *waypoint ) ) );

				// TODO track previous waypoint as part of action stack
				mPrevWaypoint.fRelease( );
			}
			else
			{
				if( mPrevWaypoint )
					fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tConnectWaypointsAction( fGuiApp( ).fMainWindow( ), *mPrevWaypoint, *waypoint ) ) );

				// TODO track previous waypoint as part of action stack
				mPrevWaypoint.fReset( waypoint );
			}
		}
	};
	class tPathToolConnectButton : public tEditorCursorControllerButton
	{
		tPathToolsPanel* mButtonContainer;
	public:
		tPathToolConnectButton( tPathToolsPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PathToolConnectPathSel" ), wxBitmap( "PathToolConnectPathDeSel" ), "Waypoint Connector - hold shift to break connections" )
			, mButtonContainer( buttonContainer )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tPathToolConnect* cursor = new tPathToolConnect( this );
			mButtonContainer->fUpdateParametersOnCursor( cursor );
			mButtonContainer->fAddCursorHotKeys( cursor );
			return tEditorCursorControllerPtr( cursor );
		}
	};


	tPathToolsPanel::tPathToolsPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Path Toolbox", "Path Toolbox", "PathTools" )
		, mAppWindow( appWindow )
	{
		tEditorCursorControllerButtonGroup* brushGroup = new tEditorCursorControllerButtonGroup( this, "", false );
		new tCreatePathToolButton( brushGroup );
		tPathToolConnectButton* connect = new tPathToolConnectButton( this, brushGroup );
		brushGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		tEditorHotKeyTable& hotKeyTable = fGuiApp( ).fHotKeys( );
		mConnectHotKey.fReset( 
			new tEnableCursorButtonHotKey( connect, hotKeyTable, Input::tKeyboard::cButtonT, 0, true ) );
	}

	void tPathToolsPanel::fAddCursorHotKeys( tEditorButtonManagedCursorController* cursor )
	{
		if( !cursor )
			return;
	}

	void tPathToolsPanel::fUpdateParametersOnCursor( tEditorButtonManagedCursorController* cursor )
	{
		if( !cursor )
			return;
	}

}
