#include "SigEdPch.hpp"
#include "tEditorAppWindow.hpp"
#include "tCreateObjectPanel.hpp"
#include "tPlaceObjectCursor.hpp"
#include "Editor/tEditableTerrainEntity.hpp"
#include "Editor/tEditableLightEntity.hpp"
#include "Editor/tEditableAttachmentEntity.hpp"
#include "Editor/tEditableShapeEntity.hpp"
#include "Editor/tEditable3dGridEntity.hpp"
#include "Editor/tEditableWaypointEntity.hpp"
#include "Editor/tEditableCameraEntity.hpp"
#include "Editor/tEditableLightProbeEntity.hpp"
#include "Editor/tEditableTileCanvas.hpp"

namespace Sig
{
	class tTerrainObjectCursor : public tPlaceObjectCursor
	{
	public:
		tTerrainObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableTerrainEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "Place Object [Terrain]" );
		}
	};
	class tTerrainObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tTerrainObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjTypeTerrainSel" ), wxBitmap( "PlaceObjTypeTerrainDeSel" ), "Place terrain objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tTerrainObjectCursor( this ) );
		}
	};


	class tLightObjectCursor : public tPlaceObjectCursor
	{
	public:
		tLightObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableLightEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "Place Object [Light]" );
		}
	};
	class tLightObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tLightObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjTypeLightSel" ), wxBitmap( "PlaceObjTypeLightDeSel" ), "Place light objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tLightObjectCursor( this ) );
		}
	};



	class tAttachmentObjectCursor : public tPlaceObjectCursor
	{
	public:
		tAttachmentObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableAttachmentEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "Place Object [Attachment]" );
		}
	};
	class tAttachmentObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tAttachmentObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjTypeAttachmentSel" ), wxBitmap( "PlaceObjTypeAttachmentDeSel" ), "Place attachment objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tAttachmentObjectCursor( this ) );
		}
	};

	class tShapeObjectCursor : public tPlaceObjectCursor
	{
	public:
		tShapeObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableShapeEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "Place Object [Shape]" );
		}
	};
	class tShapeObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tShapeObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjTypeDummySel" ), wxBitmap( "PlaceObjTypeDummyDeSel" ), "Place shape objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tShapeObjectCursor( this ) );
		}
	};

	class tCameraObjectCursor : public tPlaceObjectCursor
	{
	public:
		tCameraObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableCameraEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "Place Object [Camera]" );
		}
	};
	class tCameraObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tCameraObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjTypeCameraSel" ), wxBitmap( "PlaceObjTypeCameraDeSel" ), "Place camera objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tCameraObjectCursor( this ) );
		}
	};


	class tLightProbeObjectCursor : public tPlaceObjectCursor
	{
	public:
		tLightProbeObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableLightProbeEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "Place Object [Light Probe]" );
		}
	};
	class tLightProbeObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tLightProbeObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjTypeLightProbeSel" ), wxBitmap( "PlaceObjTypeLightProbeDeSel" ), "Place light probe objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tLightProbeObjectCursor( this ) );
		}
	};

	class t3dGridObjectCursor : public tPlaceObjectCursor
	{
	public:
		t3dGridObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditable3dGridEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "Place Object [3d Grid]" );
		}
	};
	class t3dGridObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		t3dGridObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjType3dGridSel" ), wxBitmap( "PlaceObjType3dGridDeSel" ), "Place 3d grid objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new t3dGridObjectCursor( this ) );
		}
	};


	class tWaypointObjectCursor : public tPlaceObjectCursor
	{
	public:
		tWaypointObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableWaypointEntity( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ), Math::tVec3f(0.f,1.0f,0.f) )
		{
			fMainWindow( ).fSetStatus( "Place Object [Waypoint]" );
		}
	};
	class tWaypointObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tWaypointObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjTypeWaypointSel" ), wxBitmap( "PlaceObjTypeWaypointDeSel" ), "Place waypoint objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tWaypointObjectCursor( this ) );
		}
	};


	class tTileCanvasObjectCursor : public tPlaceObjectCursor
	{
	public:
		tTileCanvasObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditableTileCanvas( button->fGetParent()->fMainWindow().fGuiApp().fEditableObjects() ) ), Math::tVec3f(0.f,1.0f,0.f) )
		{
			fMainWindow( ).fSetStatus( "Place Object [Tile Canvas]" );
		}
	};
	class tTileCanvasObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tTileCanvasObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PlaceObjTypeTileCanvasSel" ), wxBitmap( "PlaceObjTypeTileCanvasDeSel" ), "Place tile canvas objects" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tTileCanvasObjectCursor( this ) );
		}
	};



	tCreateObjectPanel::tCreateObjectPanel( tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Create", "Create Object", "PlaceObj" )
	{
		tEditorCursorControllerButtonGroup* buttonGroup = new tEditorCursorControllerButtonGroup( this, "Object Type", false );

		new tTerrainObjectCursorButton( buttonGroup );
		new tLightObjectCursorButton( buttonGroup );
		new tAttachmentObjectCursorButton( buttonGroup );
		new tShapeObjectCursorButton( buttonGroup );
		new t3dGridObjectCursorButton( buttonGroup );
		new tWaypointObjectCursorButton( buttonGroup );
		new tCameraObjectCursorButton( buttonGroup );
		new tLightProbeObjectCursorButton( buttonGroup );
		new tTileCanvasObjectCursorButton( buttonGroup );

		buttonGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );
	}

}
