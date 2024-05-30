#include "SigEdPch.hpp"
#include "DecalToolsPanels.hpp"
#include "Editor/tEditableWaypointEntity.hpp"
#include "Editor/tEditablePathDecalWaypointEntity.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tEditorAppWindow.hpp"
#include "tWxSlapOnTextBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tWxRenderPanelContainer.hpp"

namespace Sig
{

	class tAddOnePathDecalWaypointAction : public tEditorButtonManagedCursorAction
	{
		tEditablePathDecalWaypointPtr mLastInLineWaypoint, mAddedWaypoint;
	public:
		tAddOnePathDecalWaypointAction( tToolsGuiMainWindow& mainWindow, tEditablePathDecalWaypoint* prevWaypoint, tEditablePathDecalWaypoint* waypoint )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mLastInLineWaypoint( prevWaypoint )
			, mAddedWaypoint( waypoint )
		{
			fRedo( );
		}
		virtual void fUndo( )
		{
			mAddedWaypoint->fRemoveFromWorld( );
		}
		virtual void fRedo( )
		{
			mLastInLineWaypoint->fGetParent( )->fAddBack( mAddedWaypoint.fGetRawPtr( ) );
		}
	};

	class tJoinPathDecalAction : public tEditorButtonManagedCursorAction
	{
		tEditablePathDecalWaypointPtr mReceivingPoint, mAddedPoint;
		tEditablePathDecalEntityPtr mReceivingDecal, mDestroyedDecal;
		tGrowableArray< tEditablePathDecalWaypointPtr > mWaypointsSwitching;

	public:
		tJoinPathDecalAction( tToolsGuiMainWindow& mainWindow, tEditablePathDecalWaypoint* prevWaypoint, tEditablePathDecalWaypoint* waypoint )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mReceivingPoint( prevWaypoint )
			, mAddedPoint( waypoint )
			, mReceivingDecal( prevWaypoint->fGetParent( ) )
			, mDestroyedDecal( waypoint->fGetParent( ) )
		{
			mWaypointsSwitching = mDestroyedDecal->fGetWaypoints( );

			fRedo( );
		}
		virtual void fUndo( )
		{
			mReceivingDecal->fRemoveWaypoints( mWaypointsSwitching );
			mDestroyedDecal->fAddToWorld( );
			mDestroyedDecal->fAddWaypoints( mWaypointsSwitching );
		}
		virtual void fRedo( )
		{
			mReceivingDecal->fJoin( mReceivingPoint.fGetRawPtr( ), mAddedPoint.fGetRawPtr( ) );
		}
	};

	class tDisconnectPathDecalWaypointAction : public tEditorButtonManagedCursorAction
	{
		tEditablePathDecalWaypointPtr mSplitAfterPoint;
		tEditablePathDecalEntityPtr mExistingDecal, mCreatedDecal;
		tGrowableArray< tEditablePathDecalWaypointPtr > mWaypointsSwitching;
	public:
		tDisconnectPathDecalWaypointAction( tToolsGuiMainWindow& mainWindow, tEditablePathDecalWaypoint& splitAfterThis )
			: tEditorButtonManagedCursorAction( mainWindow )
			, mSplitAfterPoint( &splitAfterThis )
			, mExistingDecal( splitAfterThis.fGetParent( ) )
		{
			mCreatedDecal.fReset( mExistingDecal->fSplit( &splitAfterThis ) );
			if( !mCreatedDecal )
				return;

			mWaypointsSwitching = mCreatedDecal->fGetWaypoints( );
		}
		virtual void fUndo( )
		{
			if( !mCreatedDecal )
				return;

			mCreatedDecal->fRemoveWaypoints( mWaypointsSwitching );
			mCreatedDecal->fRemoveFromWorld( );
			mExistingDecal->fAddWaypoints( mWaypointsSwitching );
		}
		virtual void fRedo( )
		{
			if( !mCreatedDecal )
				return;

			mExistingDecal->fRemoveWaypoints( mWaypointsSwitching );
			mCreatedDecal->fAddToWorld( );
			mCreatedDecal->fAddWaypoints( mWaypointsSwitching );
		}
	};

	class tCreatePathDecalTool : public tPlaceObjectCursor
	{
		tEditablePathDecalWaypointPtr mPrevWaypoint;
		tPathDecalToolsPanel* mButtonContainer;

	public:
		tCreatePathDecalTool( tPathDecalToolsPanel* buttonContainer, tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tEditablePathDecalWaypoint( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
			, mButtonContainer( buttonContainer )
		{
			fMainWindow( ).fSetStatus( "Create Path Decal" );
		}
		virtual void fOnEntityPlaced( const tEntityPtr& placedEntity )
		{
			tEditablePathDecalWaypoint* waypoint = placedEntity->fDynamicCast< tEditablePathDecalWaypoint >( );

			// Scale the placed waypoints up to a larger size.
			Math::tMat3f xForm = waypoint->fObjectToWorld();
			xForm.fSetDiagonal( Math::tVec3f( mButtonContainer->fDefaultDecalWidth( ), 4.f, 2.f ) );
			waypoint->fMoveTo( xForm );

			// If the last waypoint doesn't exist or was removed from the scene.
			if( !mPrevWaypoint || !mPrevWaypoint->fInContainer( ) )
			{
				tEditablePathDecalEntity* newDecal = new tEditablePathDecalEntity( waypoint->fGetContainer( ) );
				newDecal->fAddToWorld( );

				waypoint->fSetParent( newDecal );
				waypoint->fGetParent( )->fAddBack( waypoint );
			}
			else
			{
				fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tAddOnePathDecalWaypointAction( fGuiApp( ).fMainWindow( ), mPrevWaypoint.fGetRawPtr( ), waypoint ) ) );
			}

			waypoint->fGetParent( )->fGetEditableProperties( ).fSetData( fEditablePropDiffuseTextureFilePath( ), mButtonContainer->fDefaultDecalTexture( ) );
			waypoint->fGetParent( )->fGetEditableProperties( ).fSetData( fEditablePropNormalMapFilePath( ), mButtonContainer->fDefaultNormalTexture( ) );

			mPrevWaypoint.fReset( waypoint );
		}
	};
	class tCreatePathDecalToolButton : public tEditorCursorControllerButton
	{
		tPathDecalToolsPanel* mButtonContainer;
	public:
		tCreatePathDecalToolButton( tPathDecalToolsPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PathToolCreatePathSel" ), wxBitmap( "PathToolCreatePathDeSel" ), "Place connected decal path" )
			, mButtonContainer( buttonContainer )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tCreatePathDecalTool( mButtonContainer, this ) );
		}
	};


	class tPathDecalToolConnect : public tEditorButtonManagedCursorController
	{
		tEditablePathDecalWaypointPtr mPrevWaypoint;
	public:
		tPathDecalToolConnect( tEditorCursorControllerButton* button )
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
			tEditablePathDecalWaypoint* waypoint = mCurrentHoverObject->fDynamicCast< tEditablePathDecalWaypoint >( );
			if( !waypoint )
			{
				mPrevWaypoint.fRelease( );
				return;
			}

			const b32 shiftDown = ( kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || kb.fButtonHeld( Input::tKeyboard::cButtonRShift ) );

			if( shiftDown )
			{
				if( waypoint->fGetParent( )->fCanSplit( waypoint ) )
					fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tDisconnectPathDecalWaypointAction( fGuiApp( ).fMainWindow( ), *waypoint ) ) );

				mPrevWaypoint.fRelease( );
			}
			else
			{
				if( mPrevWaypoint )
				{
					// Don't allow loops.
					if( mPrevWaypoint->fGetParent( )->fCanJoin( mPrevWaypoint.fGetRawPtr( ), waypoint ) )
						fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tJoinPathDecalAction( fGuiApp( ).fMainWindow( ), mPrevWaypoint.fGetRawPtr( ), waypoint ) ) );
				}

				mPrevWaypoint.fReset( waypoint );
			}
		}
	};
	class tPathDecalToolConnectButton : public tEditorCursorControllerButton
	{
		tPathDecalToolsPanel* mButtonContainer;
	public:
		tPathDecalToolConnectButton( tPathDecalToolsPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "PathToolConnectPathSel" ), wxBitmap( "PathToolConnectPathDeSel" ), "Waypoint Connector - hold shift to break connections" )
			, mButtonContainer( buttonContainer )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tPathDecalToolConnect* cursor = new tPathDecalToolConnect( this );
			mButtonContainer->fUpdateParametersOnCursor( cursor );
			mButtonContainer->fAddCursorHotKeys( cursor );
			return tEditorCursorControllerPtr( cursor );
		}
	};

	
	class tTextureWrangler : public tWxSlapOnTextBox
	{
		wxStaticBitmap* mBitmap;

	public:
		tTextureWrangler( wxWindow* parent, const char* label, s32 widthOverride, wxStaticBitmap* bitmap )
			: tWxSlapOnTextBox( parent, label, widthOverride )
			, mBitmap( bitmap )
		{ }

		void fOnBrowse( wxCommandEvent& evt )
		{
			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				this->fParent( ),
				"Select Texture",
				wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
				wxEmptyString,
				wxFileSelectorDefaultWildcardStr,
				wxFD_OPEN ) );

			if( openFileDialog->ShowModal( ) == wxID_OK )
				fRefreshTextureView( openFileDialog->GetPath( ).c_str( ) );
		}

	protected:
		virtual void fOnControlUpdated( )
		{
			fRefreshTextureView( fGetValue( ).c_str( ) );
		}

		void fRefreshTextureView( const char* path )
		{
			const tFilePathPtr absPath = tFilePathPtr( path );
			const tFilePathPtr relPath = ToolsPaths::fMakeResRelative( absPath );
			fSetValue( relPath.fCStr( ) );

			wxImage::AddHandler( new wxPNGHandler );
			wxImage::AddHandler( new wxTGAHandler );

			wxImage* image = NULL;
			
			if( StringUtil::fCheckExtension( path, ".tga" ) )
				image = new wxImage( absPath.fCStr( ), wxBITMAP_TYPE_TGA ); 
			else if( StringUtil::fCheckExtension( path, ".png" ) )
				image = new wxImage( absPath.fCStr( ), wxBITMAP_TYPE_ANY ); 
			else
			{
				log_warning( "Unsupported texture file type." );
				return;
			}

			mBitmap->SetBitmap( image->Scale(64, 64) );
			mBitmap->Refresh( );
		}
	};


	tPathDecalToolsPanel::tPathDecalToolsPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Decal Toolbox", "Decal Toolbox", "DecalTools" )
		, mAppWindow( appWindow )
	{
		tEditorCursorControllerButtonGroup* pathDecalTools = new tEditorCursorControllerButtonGroup( this, "", false );
		new tCreatePathDecalToolButton( this, pathDecalTools );
		tPathDecalToolConnectButton* connect = new tPathDecalToolConnectButton( this, pathDecalTools );

		mDecalWidth = new tWxSlapOnSpinner( pathDecalTools->fGetMainPanel( ), "Default Decal Width", -9999.f, +9999.f, 0.1f, 1, 100 );
		mDecalWidth->fSetValue( 2.f );

		wxBitmap bmp;
		mDiffuseView = new wxStaticBitmap( pathDecalTools->fGetMainPanel( ), wxID_ANY, bmp, wxDefaultPosition, wxSize(64, 64) );
		mNormalView = new wxStaticBitmap( pathDecalTools->fGetMainPanel( ), wxID_ANY, bmp, wxDefaultPosition, wxSize(64, 64) );

		mDiffuseTextureBox =  new tTextureWrangler( pathDecalTools->fGetMainPanel( ), "Default Diffuse", 150, mDiffuseView );
		wxButton* browseForTex = new wxButton( pathDecalTools->fGetMainPanel( ), wxID_ANY, "...", wxDefaultPosition, wxSize( 22, 20 ) );
		browseForTex->SetForegroundColour( wxColour( 0x22, 0x22, 0xff ) );
		browseForTex->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTextureWrangler::fOnBrowse ), NULL, mDiffuseTextureBox );
		mDiffuseTextureBox->fAddWindowToSizer( browseForTex, true );

		mNormalTextureBox =  new tTextureWrangler( pathDecalTools->fGetMainPanel( ), "Default Normal", 150, mNormalView );
		wxButton* browseForNorm = new wxButton( pathDecalTools->fGetMainPanel( ), wxID_ANY, "...", wxDefaultPosition, wxSize( 22, 20 ) );
		browseForNorm->SetForegroundColour( wxColour( 0x22, 0x22, 0xff ) );
		browseForNorm->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTextureWrangler::fOnBrowse ), NULL, mNormalTextureBox );
		mNormalTextureBox->fAddWindowToSizer( browseForNorm, true );

		pathDecalTools->fGetMainPanel( )->GetSizer( )->AddSpacer( 4 );

		wxBoxSizer* horiz = new wxBoxSizer( wxHORIZONTAL );
		horiz->Add( mDiffuseView, 0 );
		horiz->Add( mNormalView, 0, wxLEFT, 4 );

		pathDecalTools->fGetMainPanel( )->GetSizer( )->Add( horiz, 0, wxLEFT, 110 );

		pathDecalTools->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );
	}

	void tPathDecalToolsPanel::fAddCursorHotKeys( tEditorButtonManagedCursorController* cursor )
	{
		if( !cursor )
			return;
	}

	void tPathDecalToolsPanel::fUpdateParametersOnCursor( tEditorButtonManagedCursorController* cursor )
	{
		if( !cursor )
			return;
	}

	f32 tPathDecalToolsPanel::fDefaultDecalWidth( ) const
	{
		return mDecalWidth->fGetValue( );
	}

	std::string tPathDecalToolsPanel::fDefaultDecalTexture( ) const
	{
		return mDiffuseTextureBox->fGetValue( );
	}

	std::string tPathDecalToolsPanel::fDefaultNormalTexture( ) const
	{
		return mNormalTextureBox->fGetValue( );
	}

}
