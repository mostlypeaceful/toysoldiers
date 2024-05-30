//------------------------------------------------------------------------------
// \file tNavGraphToolsPanel.cpp - 03 Dec 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigEdPch.hpp"
#include "tNavGraphToolsPanel.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tEditableNavGraphNodeEntity.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxToolsPanel.hpp"


namespace Sig
{
	class tConnectNodesAction : public tEditorButtonManagedCursorAction
	{
		tEditableNavGraphNodeEntityPtr mPrevNode, mNode;
	public:
		tConnectNodesAction( tToolsGuiMainWindow& mainWindow, tEditableNavGraphNodeEntity* prevNode, tEditableNavGraphNodeEntity* node )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mPrevNode( prevNode )
			, mNode( node )
		{
			fRedo( );
		}
		virtual void fUndo( )
		{
			mPrevNode->fConnect( mNode.fGetRawPtr( ), true );
		}
		virtual void fRedo( )
		{
			mPrevNode->fConnect( mNode.fGetRawPtr( ), false );
		}
	};

	class tDisconnectNodesAction : public tEditorButtonManagedCursorAction
	{
		tEditableNavGraphNodeEntityPtr mNode;
		tGrowableArray<tEditableNavGraphNodeEntityPtr> mConnections;
	public:
		tDisconnectNodesAction( tToolsGuiMainWindow& mainWindow, tEditableNavGraphNodeEntity* node )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mNode( node )
			, mConnections( node->fConnections( ) )
		{
			fRedo( );
		}
		virtual void fUndo( )
		{
			for( u32 i = 0; i < mConnections.fCount( ); ++i )
				mNode->fConnect( mConnections[ i ].fGetRawPtr( ) );
		}
		virtual void fRedo( )
		{
			mNode->fDisconnect( true );
		}
	};

	class tConnectNavNodeTool : public tEditorButtonManagedCursorController
	{
		tEditableNavGraphNodeEntityPtr mPrevNode;
	public:
		tConnectNavNodeTool( tEditorCursorControllerButton* button )
			: tEditorButtonManagedCursorController( button )
		{
			fMainWindow( ).fSetStatus( "Connect Nav Nodes" );
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
				mPrevNode.fRelease( );
				return;
			}
			tEditableNavGraphNodeEntity* node = mCurrentHoverObject->fDynamicCast< tEditableNavGraphNodeEntity >( );
			if( !node )
			{
				mPrevNode.fRelease( );
				return;
			}

			const b32 shiftDown = ( kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || kb.fButtonHeld( Input::tKeyboard::cButtonRShift ) );

			if( shiftDown )
			{
				fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tDisconnectNodesAction( fGuiApp( ).fMainWindow( ), node ) ) );

				mPrevNode.fRelease( );
			}
			else
			{
				if( mPrevNode )
					fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tConnectNodesAction( fGuiApp( ).fMainWindow( ), mPrevNode.fGetRawPtr( ), node ) ) );

				mPrevNode.fReset( node );
			}
		}
	};
	class tConnectNavNodeToolButton : public tEditorCursorControllerButton
	{
	public:
		tConnectNavNodeToolButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PathToolConnectPathSel" ), wxBitmap( "PathToolConnectPathDeSel" ), "Nav Node Connector - hold shift to break connections" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tConnectNavNodeTool* cursor = new tConnectNavNodeTool( this );
			return tEditorCursorControllerPtr( cursor );
		}
	};

	class tCreateNavNodeTool : public tPlaceObjectCursor
	{
		tEditableNavGraphNodeEntityPtr mPrevNode;
	public:
		tCreateNavNodeTool( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableNavGraphNodeEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ), Math::tVec3f(0.f,1.0f,0.f) )
		{
			fMainWindow( ).fSetStatus( "Create Nav Graph Nodes" );
		}
		virtual void fOnEntityPlaced( const tEntityPtr& placedEntity )
		{
			tEditableNavGraphNodeEntity* node = placedEntity->fDynamicCast< tEditableNavGraphNodeEntity >( );
			if( mPrevNode )
				fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tConnectNodesAction( fGuiApp( ).fMainWindow( ), node, mPrevNode.fGetRawPtr( ) ) ) );

			mPrevNode.fReset( node );
		}
	};
	class tCreateNavNodeToolButton : public tEditorCursorControllerButton
	{
	public:
		tCreateNavNodeToolButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PathToolCreatePathSel" ), wxBitmap( "PathToolCreatePathDeSel" ), "Place connected string of nav graph nodes" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tCreateNavNodeTool( this ) );
		}
	};

	class tCreateNavRootTool : public tPlaceObjectCursor
	{
	public:
		tCreateNavRootTool( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableNavGraphRootEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ), Math::tVec3f(0.f,1.0f,0.f) )
		{
			fMainWindow( ).fSetStatus( "Create Nav Graph Root" );
		}

		virtual void fOnEntityPlaced( const tEntityPtr& placedEntity )
		{
			tEditableNavGraphRootEntity* node = placedEntity->fDynamicCast< tEditableNavGraphRootEntity >( );
			const b32 exists = node != NULL;
		}
	};
	class tCreateNavRootToolButton : public tEditorCursorControllerButton
	{
	public:
		tCreateNavRootToolButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PathToolCreatePathSel" ), wxBitmap( "PathToolCreatePathDeSel" ), "Place a nav graph root" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tCreateNavRootTool( this ) );
		}
	};

	//------------------------------------------------------------------------------
	// tNavGraphToolsPanel
	//------------------------------------------------------------------------------
	tNavGraphToolsPanel::tNavGraphToolsPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Nav Graph Toolbox", "Nav Graph Toolbox", "NavGraphTools" )
		, mAppWindow( appWindow )
	{
		tEditorCursorControllerButtonGroup* brushGroup = new tEditorCursorControllerButtonGroup( this, "", false );
		new tCreateNavRootToolButton( brushGroup );
		new tCreateNavNodeToolButton( brushGroup );
		new tConnectNavNodeToolButton( brushGroup );
		brushGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );
	}

	void tNavGraphToolsPanel::fAddCursorHotKeys( tEditorButtonManagedCursorController* cursor )
	{
	}

	void tNavGraphToolsPanel::fUpdateParametersOnCursor( tEditorButtonManagedCursorController* cursor )
	{
	}
}
