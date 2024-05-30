#include "SigFxPch.hpp"
#include "tReferenceEntityPanel.hpp"
#include "tSigFxMainWindow.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tWxDirectoryBrowser.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "FileSystem.hpp"
#include "iAssetPlugin.hpp"
#include "tAssetGenScanner.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "tSceneGraphFile.hpp"

#include <map>

namespace Sig
{
	enum tAction
	{
		cActionRunAssetGen = 1,
		cActionRefreshDirectory,
		//cActionOpenFile,
	};

	class tSceneRefObjectCursor : public tEditorCursorController, public tPlaceObjectCursorBase
	{
	public:
		tSceneRefObjectCursor( tToolsGuiMainWindow& mainWindow, const tResourceId& resourceId, const tResourcePtr& sigbResource )
			: tEditorCursorController( mainWindow.fGuiApp( ) )
			, tPlaceObjectCursorBase( mainWindow, tEntityPtr( new tEditableSgFileRefEntity( mainWindow.fGuiApp( ).fEditableObjects( ), resourceId, sigbResource ) ) )
		{
			mainWindow.fSetStatus( "Place Object [Sig Reference]" );
		}
		virtual void fOnTick( )
		{
			tPlaceObjectCursorBase::fOnTick( );
		}
		virtual b32 fComputePickRay( Math::tRayf& rayOut )
		{
			return tEditorCursorController::fComputePickRay( rayOut );
		}
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			if( mEntityMaster )
			{
				mEntityMaster->fDeleteImmediate( );
				mEntityMaster.fRelease( );
			}
		}
	};

	class tResourceBrowserTree : public tWxDirectoryBrowser
	{
	private:
		tSigFxMainWindow*			mEditorWindow;
		DECLARE_EVENT_TABLE()

	public:
		tResourceBrowserTree( tSigFxMainWindow* editorWindow, wxWindow* parent, u32 minHeight )
			: tWxDirectoryBrowser( parent, minHeight )
			, mEditorWindow( editorWindow )
		{			
			Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler(tResourceBrowserTree::fOnItemRightClick), 0, this );
		}
		virtual void fOnSelChanged( wxTreeEvent& event, const tFileEntryData& fileEntryData )
		{
			// this denotes an actual file was selected; load model and display as cursor

			// first check if it's the same file we're currently editing; we can't very well allow
			// placing a file within itself (infinite recursion):
			const tFilePathPtr sigmlFile = fileEntryData.fXmlPath( );
			if( sigmlFile == ToolsPaths::fMakeResRelative( tFilePathPtr( mEditorWindow->fGuiApp( ).fDocName( ) ) ) )
			{
				event.Veto( );
				wxMessageBox( "You are trying to place the current file you're editing into itself; this would make the universe explode.",
					  "File Placement Recursion", wxOK | wxICON_WARNING );
				mEditorWindow->SetFocus( );
				return;
			}

			const tResourceDepotPtr& resourceDepot = mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetResourceDepot( );
			const tResourceId rid = tResourceId::fMake<tSceneGraphFile>( fileEntryData.fBinaryPath( ) );
			tResourcePtr sigbResource = resourceDepot->fQuery( rid );
			mEditorWindow->fGuiApp( ).fSetCurrentCursor( tEditorCursorControllerPtr( new tSceneRefObjectCursor( *mEditorWindow, rid, sigbResource ) ) );
			SetItemTextColour( event.GetItem( ), fileEntryData.fGetTextItemColor( ) );
		}
		virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData )
		{
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".sigml" ) )
				return wxColour( 0x00, 0x77, 0x00 );
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".mshml" ) )
				return wxColour( 0x00, 0x00, 0x77 );
			return wxColour( 0, 0, 0 );
		}
		virtual b32 fFilterPath( const tFilePathPtr& path )
		{
			return !Sigml::fIsSigmlFile( path );
		}
		virtual std::string fConvertDisplayName( const std::string& simplePath, u32& sortValue )
		{
			std::stringstream displayName;

			if( StringUtil::fStrStrI( simplePath.c_str( ), ".mshml" ) )
			{
				displayName << "[mesh] ";
				sortValue = 1;
			}
			else
				displayName << "[scene] ";

			displayName << StringUtil::fStripExtension( simplePath.c_str( ) );

			return displayName.str( );
		}
		virtual tFilePathPtr fXmlPathToBinaryPath( const tFilePathPtr& xmlPath )
		{
			return Sigml::fSigmlPathToSigb( xmlPath );
		}
		void fOnItemRightClick( wxTreeEvent& event )
		{
			mRightClickItem = event.GetItem( );
			wxMenu menu;
			if( !fIsFileItem( event.GetItem( ) ) )
				menu.Append( cActionRefreshDirectory, _T("&Refresh directory"));
			else
			{
				menu.Append( cActionRunAssetGen, _T("Run &AssetGen on file and reload"));

				const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
				//if( fileEntryData && StringUtil::fCheckExtension( fileEntryData->fXmlPath( ).fCStr( ), ".sigml" ) )
				//	menu.Append( cActionOpenFile, _T("Open file in editor") );
			}
			PopupMenu(&menu, event.GetPoint( ).x, event.GetPoint( ).y);
			event.Skip( );
		}
		void fOnAction( wxCommandEvent& event )
		{
			switch( event.GetId( ) )
			{
			case cActionRunAssetGen:
				{
					if( !mRightClickItem.IsOk( ) )
						break;
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
					if( fileEntryData )
					{
						// run asset gen
						const tFilePathPtr pathToGenerate = fileEntryData->fXmlPath( );
						fRunAssetGen( mRightClickItem, fileEntryData, pathToGenerate );

						// now try to reload
						const tFilePathPtr pathToReload = fileEntryData->fBinaryPath( );
						const tResourceId rid = tResourceId::fMake<tSceneGraphFile>( pathToReload );
						tResourcePtr resource = mEditorWindow->fGuiApp( ).fEditableObjects( ).fGetResourceDepot( )->fQuery( rid );
						if( !resource->fLoading( ) && resource->fHasLoadCaller( ) )
						{
							// now we need to notify all entities that contain this resource or any sub-resources in this resource, and have them refresh
							tGrowableArray< tEditableSgFileRefEntity* > referenceEntities;
							mEditorWindow->fGuiApp( ).fEditableObjects( ).fCollectByType( referenceEntities );
							for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
								referenceEntities[ i ]->fRefreshDependents( resource, true );
							resource->fReload( false );
							for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
								referenceEntities[ i ]->fRefreshDependents( resource, false );
						}
					}

					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;

			case cActionRefreshDirectory:
				{
					if( !mRightClickItem.IsOk( ) )
						break;
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
					if( !fileEntryData )
					{
						// TODO refresh sub-directory only
						fRefresh( );
					}

					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;

			/*case cActionOpenFile:
				{
					if( !mRightClickItem.IsOk( ) )
						break;
					const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
					if( fileEntryData && StringUtil::fCheckExtension( fileEntryData->fXmlPath( ).fCStr( ), ".sigml" ) )
					{
						//UnselectAll( );
						//mEditorWindow->fOpenDoc( ToolsPaths::fMakeResAbsolute( fileEntryData->fXmlPath( ) ) );
					}

					// reset to null
					mRightClickItem = wxTreeItemId( );
				}
				break;
			*/
			default: { log_warning( "Unrecognized action!" ); }
				event.Skip( );
				break;
			}
		}
		void fRunAssetGen( wxTreeItemId id, const tFileEntryData* fileEntryData, const tFilePathPtr& path )
		{
			log_line( 0, "Running assetgen on [" << path << "]" );

			tAssetGenScanner::fProcessSingleFile( tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectResFolder( ), path ), false, 
				fPlatformIdFlag( cCurrentPlatform ) );// | fPlatformIdFlag( mEditorWindow->fPreviewPlatform( ) ) );

			SetItemTextColour( id, fileEntryData->fGetTextItemColor( ) );
		}
	};

	BEGIN_EVENT_TABLE(tResourceBrowserTree, wxTreeCtrl)
		EVT_MENU(				cActionRunAssetGen,			tResourceBrowserTree::fOnAction)
		EVT_MENU(				cActionRefreshDirectory,	tResourceBrowserTree::fOnAction)
		//EVT_MENU(				cActionOpenFile,			tResourceBrowserTree::fOnAction)
	END_EVENT_TABLE()


	tReferenceEntityPanel::tReferenceEntityPanel( tSigFxMainWindow* appWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Browse Reference Objects", "Reference Browser", "BrowseObj" )
		, mBrowser( 0 )
		, mCurSel( 0 )
		, mRefreshed( false )
	{
		mBrowser = new tResourceBrowserTree( appWindow, fGetMainPanel( ), 300 );
		fGetMainPanel( )->GetSizer( )->Add( mBrowser, 0, wxEXPAND | wxALL, 4 );
	}

	void tReferenceEntityPanel::fOnTick( )
	{
		if( !mRefreshed )
		{
			mRefreshed = true;
			mBrowser->fRefresh( );
		}

		wxTreeItemId curSel = mBrowser->GetSelection( );
		if( curSel != mCurSel )
		{
			mCurSel = curSel;
			fGuiApp( ).fMainWindow( ).SetFocus( );
		}

		//if( mRefreshTimer.fGetElapsedS( ) > 1.f )
		//{
		//	mRefreshTimer.fResetElapsedS( );
		//	mBrowser->fRefresh( );
		//}
	}


}
