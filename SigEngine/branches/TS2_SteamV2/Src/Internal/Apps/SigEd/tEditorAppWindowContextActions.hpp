#ifndef __tEditorAppWindowContextActions__
#define __tEditorAppWindowContextActions__
#include "Editor/tEditableTerrainEntity.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "Editor/tEditableAttachmentEntity.hpp"
#include "Editor/tEditableWaypointEntity.hpp"
#include "Editor/tEditableShapeEntity.hpp"
#include "Editor/tEditablePathDecalWaypointEntity.hpp"
#include "Editor/tEditableNavGraphNodeEntity.hpp"
#include "tEditableObjectProperties.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tSelectObjectsCursor.hpp"

namespace Sig
{
	class tHideObjectsContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mIdHideSelected, mIdHideUnselected, mIdUnhideAll;
	public:
		tHideObjectsContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mIdHideSelected( fNextUniqueActionId( ) )
			, mIdHideUnselected( fNextUniqueActionId( ) )
			, mIdUnhideAll( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			const b32 someObjectsSelected = mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0;
			const b32 someObjectsHidden = mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetHiddenCount( ) > 0;
			const b32 someObjectsUnselected = ( mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetShownCount( ) - mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) ) > 0;
			if( someObjectsSelected )
				menu.Append( mIdHideSelected, _T("Hide Selected\tCtrl+H") );
			if( someObjectsUnselected )
				menu.Append( mIdHideUnselected, _T("Hide Unselected") );
			if( someObjectsHidden )
				menu.Append( mIdUnhideAll, _T("Unhide All") );
			if( someObjectsSelected || someObjectsUnselected || someObjectsHidden )
			{
				menu.AppendSeparator( );
				return true;
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mIdHideSelected )
				mEditorWindow->fHideSelected( );
			else if( actionId == mIdHideUnselected )
				mEditorWindow->fHideUnselected( );
			else if( actionId == mIdUnhideAll )
				mEditorWindow->fUnhideAll( );
			else
				return false;
			return true;
		}
	};

	class tFreezeObjectsContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mIdFreezeSelected, mIdFreezeUnselected, mIdUnfreezeAll;
	public:
		tFreezeObjectsContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mIdFreezeSelected( fNextUniqueActionId( ) )
			, mIdFreezeUnselected( fNextUniqueActionId( ) )
			, mIdUnfreezeAll( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			const b32 someObjectsSelected = mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0;
			const b32 someObjectsFrozen = mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetFrozenCount( ) > 0;
			const b32 someObjectsUnselected = ( mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetShownCount( ) - mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) ) > 0;
			if( someObjectsSelected )
				menu.Append( mIdFreezeSelected, _T("Freeze Selected") );
			if( someObjectsUnselected )
				menu.Append( mIdFreezeUnselected, _T("Freeze Unselected") );
			if( someObjectsFrozen )
				menu.Append( mIdUnfreezeAll, _T("Unfreeze all") );
			if( someObjectsSelected || someObjectsUnselected || someObjectsFrozen )
			{
				menu.AppendSeparator( );
				return true;
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mIdFreezeSelected )
				mEditorWindow->fFreezeSelected( );
			else if( actionId == mIdFreezeUnselected )
				mEditorWindow->fFreezeUnselected( );
			else if( actionId == mIdUnfreezeAll )
				mEditorWindow->fUnfreezeAll( );
			else
				return false;
			return true;
		}
	};

	class tSnapObjectsContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mIdSnapUp, mIdSnapDown, mIdSnapClosest;
	public:
		tSnapObjectsContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mIdSnapClosest( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				menu.Append( mIdSnapClosest, _T("Snap to Ground"));
				menu.AppendSeparator( );
				return true;
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mIdSnapClosest )
				mEditorWindow->fSnapToGround( false );
			else
				return false;
			return true;
		}
	};

	class tChangeShadeModeContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mIdShadeSmooth, mIdShadeWireFrame, mIdEdgedFaces;
		Gfx::tScreen* mScreen;
	public:
		tChangeShadeModeContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mIdShadeSmooth( fNextUniqueActionId( ) )
			, mIdShadeWireFrame( fNextUniqueActionId( ) )
			, mIdEdgedFaces( fNextUniqueActionId( ) )
			, mScreen( 0 )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			mScreen = mEditorWindow->fRenderPanelContainer( )->fGetActiveScreen( );
			sigassert( mScreen );
			wxMenu* shadeMode = new wxMenu;
			wxMenuItem* smooth = shadeMode->AppendCheckItem( mIdShadeSmooth, _T("Default") );
			wxMenuItem* wireframe = shadeMode->AppendCheckItem( mIdShadeWireFrame, _T("Wireframe") );
			wxMenuItem* edgedFaces = shadeMode->AppendCheckItem( mIdEdgedFaces, _T("Edged Faces") );
			switch( mScreen->fGetGlobalFillMode( ) )
			{
			case Gfx::tRenderState::cGlobalFillWire:		wireframe->Check( true ); break;
			case Gfx::tRenderState::cGlobalFillEdgedFace:	edgedFaces->Check( true ); break;
			default:										smooth->Check( true ); break;
			}
			menu.AppendSubMenu( shadeMode, "Shade Mode" );
			menu.AppendSeparator( );
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			b32 handled = true;
			if( actionId == mIdShadeSmooth )
				mScreen->fSetGlobalFillMode( Gfx::tRenderState::cGlobalFillDefault );
			else if( actionId == mIdShadeWireFrame )
				mScreen->fSetGlobalFillMode( Gfx::tRenderState::cGlobalFillWire );
			else if( actionId == mIdEdgedFaces )
				mScreen->fSetGlobalFillMode( Gfx::tRenderState::cGlobalFillEdgedFace );
			else
				handled = false;
			mScreen = 0;
			return handled;
		}
	};

	class tSaveHeightMapBitmapsContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;
	public:
		tSaveHeightMapBitmapsContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				tGrowableArray< tEditableTerrainGeometry* > terrainObjs;
				mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableTerrainGeometry >( terrainObjs );
				if( terrainObjs.fCount( ) > 0 )
				{
					menu.Append( mId, _T("Save Selected HeightMaps..."));
					menu.AppendSeparator( );
					return true;
				}
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mId )
			{
				tGrowableArray< tEditableTerrainGeometry* > terrainObjs;
				mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableTerrainGeometry >( terrainObjs );
				if( terrainObjs.fCount( ) > 0 )
				{
					tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
						mEditorWindow->fRenderPanelContainer( )->fGetActiveRenderPanel( ), 
						"Save HeightMap As...",
						wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
						wxString( "heightmap.bmp" ),
						wxString( "*.bmp" ),
						wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

					mEditorWindow->SetFocus( );

					if( openFileDialog->ShowModal( ) == wxID_OK )
					{
						const std::string docNameGeneric = openFileDialog->GetPath( );

						for( u32 i = 0; i < terrainObjs.fCount( ); ++i )
						{
							const std::string docName = 
								i > 0 ? 
									( StringUtil::fAppend( StringUtil::fStripExtension( docNameGeneric.c_str( ) ), i ) + ".bmp" ) : 
									docNameGeneric;
							terrainObjs[ i ]->fSaveHeightFieldToBitmap( tFilePathPtr( docName.c_str( ) ) );
						}
					}
				}
			}
			else
				return false;
			return true;
		}
	};


	class tOpenSigmlContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;
	public:
		tOpenSigmlContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				tGrowableArray< tEditableSgFileRefEntity* > sigmls;
				mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableSgFileRefEntity >( sigmls );
				if( sigmls.fCount( ) == 1 && sigmls.fFront( )->fIsEditable( ) )
				{
					menu.Append( mId, _T("Open .sigml file in editor"));
					menu.AppendSeparator( );
					return true;
				}
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mId )
			{
				tGrowableArray< tEditableSgFileRefEntity* > sigmls;
				mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableSgFileRefEntity >( sigmls );
				if( sigmls.fCount( ) == 1 && sigmls.fFront( )->fIsEditable( ) )
				{
					const tFilePathPtr path = Sigml::fSigbPathToSigml( sigmls.fFront( )->fResourcePath( ) );
					mEditorWindow->fOpenDoc( ToolsPaths::fMakeResAbsolute( path ) );
				}
			}
			else
				return false;
			return true;
		}
	};

	class tDisplayObjectPropertiesContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mObjectPropsId, mGlobalPropsId;
	public:
		tDisplayObjectPropertiesContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mObjectPropsId( fNextUniqueActionId( ) )
			, mGlobalPropsId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				menu.Append( mObjectPropsId, _T("Object Properties...") );
			}
			menu.Append( mGlobalPropsId, _T("Global Properties...") );
			menu.AppendSeparator( );
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mObjectPropsId )
				mEditorWindow->fToggleObjectProperties( );
			else if( actionId == mGlobalPropsId )
				mEditorWindow->fToggleGlobalProperties( );
			else
				return false;
			return true;
		}
	};


	class tConvertAttachmentPointsToWayPointsContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;
	public:
		tConvertAttachmentPointsToWayPointsContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				tGrowableArray< tEditableAttachmentEntity* > attachmentPoints;
				mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableAttachmentEntity >( attachmentPoints );
				if( attachmentPoints.fCount( ) > 0 )
				{
					menu.Append( mId, _T("Convert to Waypoints"));
					menu.AppendSeparator( );
					return true;
				}
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId != mId )
				return false;

			tGrowableArray< tEditableAttachmentEntity* > attachmentPoints;
			mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableAttachmentEntity >( attachmentPoints );
			if( attachmentPoints.fCount( ) > 0 )
			{
				tGrowableArray<tEntityPtr> oldEntities;
				tGrowableArray<tEntityPtr> newEntities;
				for( u32 i = 0; i < attachmentPoints.fCount( ); ++i )
				{
					oldEntities.fPushBack( tEntityPtr( attachmentPoints[ i ] ) );

					tEditableWaypointEntity* waypoint = new tEditableWaypointEntity( mEditorWindow->fGuiApp( ).fEditableObjects( ) );
					waypoint->fSetLayer( attachmentPoints[ i ]->fLayer( ) );
					waypoint->fMoveTo( attachmentPoints[ i ]->fObjectToWorld( ) );
					waypoint->fGetEditableProperties( ).fUnion( attachmentPoints[ i ]->fGetEditableProperties( ), true );
					newEntities.fPushBack( tEntityPtr( waypoint ) );
				}
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tPlaceObjectsAction( *mEditorWindow, oldEntities, true ) ) );
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tPlaceObjectsAction( *mEditorWindow, newEntities ) ) );
			}

			return true;
		}
	};

	class tConvertToAttachmentPointsContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;
	public:
		tConvertToAttachmentPointsContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				menu.Append( mId, _T("Convert to Attachment Points"));
				menu.AppendSeparator( );
				return true;
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId != mId )
				return false;

			tEditorSelectionList& selList = mEditorWindow->fGuiApp( ).fSelectionList( );
			if( selList.fCount( ) > 0 )
			{
				tGrowableArray<tEntityPtr> oldEntities;
				tGrowableArray<tEntityPtr> newEntities;
				for( u32 i = 0; i < selList.fCount( ); ++i )
				{
					oldEntities.fPushBack( selList[ i ] );

					tEditableObject* eo = selList[ i ]->fDynamicCast< tEditableObject >( );

					tEditableAttachmentEntity* newEnt = new tEditableAttachmentEntity( mEditorWindow->fGuiApp( ).fEditableObjects( ) );
					newEnt->fSetLayer( eo->fLayer( ) );
					newEnt->fMoveTo( eo->fObjectToWorld( ) );
					newEnt->fGetEditableProperties( ).fUnion( eo->fGetEditableProperties( ), true );
					newEntities.fPushBack( tEntityPtr( newEnt ) );
				}
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tPlaceObjectsAction( *mEditorWindow, oldEntities, true ) ) );
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tPlaceObjectsAction( *mEditorWindow, newEntities ) ) );
			}

			return true;
		}
	};

	class tConvertToShapesContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;
	public:
		tConvertToShapesContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				menu.Append( mId, _T("Convert to Shapes"));
				menu.AppendSeparator( );
				return true;
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId != mId )
				return false;

			tEditorSelectionList& selList = mEditorWindow->fGuiApp( ).fSelectionList( );
			if( selList.fCount( ) > 0 )
			{
				tGrowableArray<tEntityPtr> oldEntities;
				tGrowableArray<tEntityPtr> newEntities;
				for( u32 i = 0; i < selList.fCount( ); ++i )
				{
					oldEntities.fPushBack( selList[ i ] );

					tEditableObject* eo = selList[ i ]->fDynamicCast< tEditableObject >( );

					tEditableShapeEntity* newEnt = new tEditableShapeEntity( mEditorWindow->fGuiApp( ).fEditableObjects( ) );
					newEnt->fSetLayer( eo->fLayer( ) );
					Math::tMat3f newXform = eo->fObjectToWorld( );
					newXform.fScaleLocal( 0.5f );
					newEnt->fMoveTo( newXform );
					newEnt->fGetEditableProperties( ).fUnion( eo->fGetEditableProperties( ), true );
					newEntities.fPushBack( tEntityPtr( newEnt ) );
				}
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tPlaceObjectsAction( *mEditorWindow, oldEntities, true ) ) );
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tPlaceObjectsAction( *mEditorWindow, newEntities ) ) );
			}

			return true;
		}
	};

	class tConvertToNavGraphContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;

		// These need to be cleared for each new operation.
		tGrowableArray<tEntityPtr> mOldEntities;
		tGrowableArray<tEntityPtr> mNewEntities;
		tHashTable<u32, tEditableNavGraphNodeEntity*> mWpGuidToNn;
	public:
		tConvertToNavGraphContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				tGrowableArray< tEditableWaypointEntity* > waypoints;
				mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableWaypointEntity >( waypoints );
				if( waypoints.fCount( ) > 0 )
				{
					menu.Append( mId, _T("Convert To Nav Graph"));
					menu.AppendSeparator( );
					return true;
				}
			}
			return false;
		}
		tEditableNavGraphNodeEntity* fFindOrCreate( tEditableObjectContainer& container, const tEditableWaypointBase* wp )
		{
			// Look and see if this waypoint has already been converted.
			const u32 wpGuid = wp->fGuid( );
			tEditableNavGraphNodeEntity** foundNode = mWpGuidToNn.fFind( wpGuid );
			if( foundNode )
				return *foundNode;

			// If one hasn't been, create a new one.
			tEditableNavGraphNodeEntity* newNav = new tEditableNavGraphNodeEntity( mEditorWindow->fGuiApp( ).fEditableObjects( ) );
			newNav->fSetLayer( wp->fLayer( ) );
			Math::tMat3f newXform = wp->fObjectToWorld( );
			newNav->fMoveTo( newXform );
			newNav->fGetEditableProperties( ).fUnion( wp->fGetEditableProperties( ), true );

			mNewEntities.fPushBack( tEntityPtr( newNav ) );

			// Record it.
			mWpGuidToNn.fInsert( wpGuid, newNav );

			return newNav;
		}

		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId != mId )
				return false;

			tGrowableArray< tEditableWaypointEntity* > waypoints;
			mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableWaypointEntity >( waypoints );
			if( waypoints.fCount( ) > 0 )
			{
				for( u32 i = 0; i < waypoints.fCount( ); ++i )
				{
					mOldEntities.fPushBack( tEntityPtr( waypoints[ i ] ) );

					tEditableWaypointEntity* wp = waypoints[ i ];

					tEditableNavGraphNodeEntity* newNav = fFindOrCreate( mEditorWindow->fGuiApp( ).fEditableObjects( ), wp );

					const tGrowableArray<tEditableWaypointBasePtr>& conns = wp->fConnections( );
					for( u32 i = 0; i < conns.fCount( ); ++i )
					{
						if( !conns[i]->fSceneGraph( ) )
							continue;

						tEditableNavGraphNodeEntity* newConn = fFindOrCreate( mEditorWindow->fGuiApp( ).fEditableObjects( ), conns[i].fGetRawPtr( ) );
						newNav->fConnect( newConn );
					}

					const tGrowableArray<tEditableWaypointBase*>& backs = wp->fBackConnections( );
					for( u32 i = 0; i < backs.fCount( ); ++i )
					{
						if( !backs[i]->fSceneGraph( ) )
							continue;

						tEditableNavGraphNodeEntity* newConn = fFindOrCreate( mEditorWindow->fGuiApp( ).fEditableObjects( ), backs[i] );
						newConn->fConnect( newNav );
					}
				}
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tPlaceObjectsAction( *mEditorWindow, mOldEntities, true ) ) );
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tPlaceObjectsAction( *mEditorWindow, mNewEntities ) ) );
			}

			// Clear containers.
			mOldEntities.fDeleteArray( );
			mNewEntities.fDeleteArray( );
			mWpGuidToNn.fClear( 1 );

			return true;
		}
	};

	class tSelectEntireNavGraphContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;
	public:
		tSelectEntireNavGraphContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				tGrowableArray< tEditableNavGraphNodeEntity* > navGraphNodes;
				mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableNavGraphNodeEntity >( navGraphNodes );
				if( navGraphNodes.fCount( ) > 0 )
				{
					menu.Append( mId, _T("Select Entire Nav Graph"));
					menu.AppendSeparator( );
					return true;
				}
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId != mId )
				return false;

			tGrowableArray< tEditableNavGraphNodeEntity* > navGraphNodes;
			mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableNavGraphNodeEntity >( navGraphNodes );
			if( navGraphNodes.fCount( ) > 0 )
			{
				tEditorSelectionList& selected = mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetSelectionList( );
				tEditorSelectionList savedSelection = selected;
				selected.fClear( );
				tGrowableArray<tEditableNavGraphNodeEntity*> entirePath;
				for( u32 i = 0; i < navGraphNodes.fCount( ); ++i )
					navGraphNodes[ i ]->fAcquireEntireGraph( entirePath );
				for( u32 i = 0; i < entirePath.fCount( ); ++i )
					selected.fAdd( tEntityPtr( entirePath[ i ] ) );
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( mEditorWindow->fGuiApp( ).fMainWindow( ), savedSelection ) ) );
			}

			return true;
		}
	};

	class tSelectPathDecalContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;
	public:
		tSelectPathDecalContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) == 0 )
				return false;

			tGrowableArray< tEditablePathDecalWaypoint* > wayPoints;
			mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditablePathDecalWaypoint >( wayPoints );
			if( wayPoints.fCount( ) == 0 )
				return false;

			menu.Append( mId, _T("Select Decal"));
			menu.AppendSeparator( );
			return true;
		}
		virtual b32 fHandleAction( u32 actionId )
		{
			if( actionId != mId )
				return false;

			tGrowableArray< tEditablePathDecalWaypoint* > wayPoints;
			mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditablePathDecalWaypoint >( wayPoints );
			if( wayPoints.fCount( ) > 0 )
			{
				tEditorSelectionList& selected = mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetSelectionList( );
				tEditorSelectionList savedSelection = selected;
				selected.fClear( );
				tGrowableArray<tEditablePathDecalWaypoint*> entirePath;
				for( u32 i = 0; i < wayPoints.fCount( ); ++i )
					wayPoints[ i ]->fAcquireEntireDecal( entirePath );
				for( u32 i = 0; i < entirePath.fCount( ); ++i )
					selected.fAdd( tEntityPtr( entirePath[ i ] ) );
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( mEditorWindow->fGuiApp( ).fMainWindow( ), savedSelection ) ) );
			}

			return true;
		}
	};

	class tSelectEntirePathContextAction : public tEditorContextAction
	{
		tEditorAppWindow* mEditorWindow;
		u32 mId;
	public:
		tSelectEntirePathContextAction( tEditorAppWindow* editorWindow )
			: mEditorWindow( editorWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mEditorWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
			{
				tGrowableArray< tEditableWaypointEntity* > wayPoints;
				mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableWaypointEntity >( wayPoints );
				if( wayPoints.fCount( ) > 0 )
				{
					menu.Append( mId, _T("Select Path(s)"));
					menu.AppendSeparator( );
					return true;
				}
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId != mId )
				return false;

			tGrowableArray< tEditableWaypointEntity* > wayPoints;
			mEditorWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableWaypointEntity >( wayPoints );
			if( wayPoints.fCount( ) > 0 )
			{
				tEditorSelectionList& selected = mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetSelectionList( );
				tEditorSelectionList savedSelection = selected;
				selected.fClear( );
				tGrowableArray<tEditableWaypointBase*> entirePath;
				for( u32 i = 0; i < wayPoints.fCount( ); ++i )
					wayPoints[ i ]->fAcquireEntirePath( entirePath );
				for( u32 i = 0; i < entirePath.fCount( ); ++i )
					selected.fAdd( tEntityPtr( entirePath[ i ] ) );
				mEditorWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( mEditorWindow->fGuiApp( ).fMainWindow( ), savedSelection ) ) );
			}

			return true;
		}
	};

}

#endif//__tEditorAppWindowContextActions__
