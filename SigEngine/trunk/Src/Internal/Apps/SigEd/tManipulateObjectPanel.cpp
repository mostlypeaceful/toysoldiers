#include "SigEdPch.hpp"
#include "tManipulateObjectPanel.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tToolsGuiMainWindow.hpp"

#include "tWxNumericText.hpp"
#include "tWxSlapOnCheckBox.hpp"

#include "tSelectObjectsCursor.hpp"

#include "tTranslationGizmo.hpp"
#include "tRotationGizmo.hpp"
#include "tScaleGizmo.hpp"

#include "tTranslationGizmoGeometry.hpp"
#include "tRotationGizmoGeometry.hpp"
#include "tScaleGizmoGeometry.hpp"


namespace Sig
{
	class tSelectObjectCursorButton;

	class tWxCoordinate : public tWxNumericText
	{
		tSelectObjectCursorButton* mOwner;
	public:
		tWxCoordinate( wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize )
			: tWxNumericText( parent, -99999999.f, +99999999.f, 2, pos, size ), mOwner( 0 ) { }
		inline void fSetOwner( tSelectObjectCursorButton* owner ) { mOwner = owner; }
		virtual void fOnValueUpdated( );
	};


	class tSelectObjectCursorButton : public tEditorCursorControllerButton
	{
		tWxCoordinate*	mSpecifyX;
		tWxCoordinate*	mSpecifyY; 
		tWxCoordinate*	mSpecifyZ;
		wxButton*		mResetButton;
	public:
		tSelectObjectCursorButton( tEditorCursorControllerButtonGroup* parent, tWxCoordinate* specifyX, tWxCoordinate* specifyY, tWxCoordinate* specifyZ, wxButton* resetButton )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateSelectionSel" ), wxBitmap( "ManipulateSelectionDeSel" ), "Select objects (Q)" )
			, mSpecifyX( specifyX )
			, mSpecifyY( specifyY )
			, mSpecifyZ( specifyZ )
			, mResetButton( resetButton )
		{
			mSpecifyX->fSetOwner( this );
			mSpecifyY->fSetOwner( this );
			mSpecifyZ->fSetOwner( this );
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tSelectObjectsCursor( this, "Select Objects" ) );
		}
		virtual void fOnSelected( )
		{
			fDisableCoordinates( );
		}
		virtual void fOnDeselected( )
		{
			fEnableCoordinates( Math::tVec3f::cZeroVector, false );
		}
		void fOnCoordinateChanged( )
		{
			tSelectObjectsCursor* cursor = 
				dynamic_cast< tSelectObjectsCursor* >( fGetParent( )->fMainWindow( ).fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
			if( cursor )
			{
				cursor->fSetWorldCoords( Math::tVec3f( mSpecifyX->fGetValue( ), mSpecifyY->fGetValue( ), mSpecifyZ->fGetValue( ) ),
					!mSpecifyX->fIsIndeterminate( ), !mSpecifyY->fIsIndeterminate( ), !mSpecifyZ->fIsIndeterminate( ) );
			}
			fGetParent( )->fMainWindow( ).SetFocus( );
		}
		void fOnResetFieldsPressed( wxCommandEvent& event )
		{
			tSelectObjectsCursor* cursor = 
				dynamic_cast< tSelectObjectsCursor* >( fGetParent( )->fMainWindow( ).fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
			if( cursor )
			{
				Math::tVec3f defaultCoords;
				if( cursor->fGetDefaultCoords( defaultCoords ) )
				{
					mSpecifyX->fSetValue( defaultCoords.x, false );
					mSpecifyY->fSetValue( defaultCoords.y, false );
					mSpecifyZ->fSetValue( defaultCoords.z, false );

					cursor->fSetWorldCoords( defaultCoords, !mSpecifyX->fIsIndeterminate( ), !mSpecifyY->fIsIndeterminate( ), !mSpecifyZ->fIsIndeterminate( ) );
				}
			}
			fGetParent( )->fMainWindow( ).SetFocus( );
		}
		b32 fUpdateCoordinates( )
		{
			const b32 hasFocus = 
				( GetFocus( ) == ( HWND )mSpecifyX->GetHWND( ) ) ||
				( GetFocus( ) == ( HWND )mSpecifyY->GetHWND( ) ) ||
				( GetFocus( ) == ( HWND )mSpecifyZ->GetHWND( ) );
			if( hasFocus )
				return true; // don't forcefully update while we're trying to edit

			tSelectObjectsCursor* cursor = dynamic_cast< tSelectObjectsCursor* >( fGetParent( )->fMainWindow( ).fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
			Math::tVec3f coords = Math::tVec3f::cZeroVector;
			if( cursor && cursor->fHasGizmo( ) && fGetParent( )->fMainWindow( ).fGuiApp( ).fSelectionList( ).fCount( ) > 0 )
				fEnableCoordinates( cursor->fGetWorldCoords( ), fGetParent( )->fMainWindow( ).fGuiApp( ).fSelectionList( ).fCount( ) > 1 );
			else
				fDisableCoordinates( );

			return false;
		}

	private:
		void fEnableCoordinates( const Math::tVec3f& v, b32 indeterminate )
		{
			mSpecifyX->Enable( true ); if( indeterminate ) mSpecifyX->fSetIndeterminate( ); else mSpecifyX->fSetValue( v.x, false );
			mSpecifyY->Enable( true ); if( indeterminate ) mSpecifyY->fSetIndeterminate( ); else mSpecifyY->fSetValue( v.y, false );
			mSpecifyZ->Enable( true ); if( indeterminate ) mSpecifyZ->fSetIndeterminate( ); else mSpecifyZ->fSetValue( v.z, false );

			mResetButton->Enable( true );
		}
		void fDisableCoordinates( )
		{
			mSpecifyX->fSetValue( 0.f, false ); mSpecifyX->Enable( false );
			mSpecifyY->fSetValue( 0.f, false ); mSpecifyY->Enable( false );
			mSpecifyZ->fSetValue( 0.f, false ); mSpecifyZ->Enable( false );

			mResetButton->Enable( false );
		}
	};

	void tWxCoordinate::fOnValueUpdated( )
	{
		mOwner->fOnCoordinateChanged( );
	}


	class tTranslateObjectCursorButton : public tEditorCursorControllerButton
	{
		tManipulationGizmoPtr	mGizmo;
		wxCheckBox*				mWorldSpace;
	public:
		tTranslateObjectCursorButton( tEditorCursorControllerButtonGroup* parent, wxCheckBox* worldSpace )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateTranslateSel" ), wxBitmap( "ManipulateTranslateDeSel" ), "Translate objects (W)" )
			, mWorldSpace( worldSpace )
		{
			mWorldSpace->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( tTranslateObjectCursorButton::fOnCheckBoxChecked ), NULL, this );
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( new tTranslationGizmoGeometry( 
					fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) ) );
				mGizmo.fReset( new tTranslationGizmo( gizmoGeom ) );
			}

			tSelectObjectsCursor* cursor = new tSelectObjectsCursor( this, "Translate Objects", mGizmo );
			cursor->fSetIsGizmoWorldSpace( mWorldSpace->GetValue( )!=0 );
			return tEditorCursorControllerPtr( cursor );
		}
		virtual void fOnSelected( )
		{
			mWorldSpace->Enable( true );
		}
		virtual void fOnDeselected( )
		{
			mWorldSpace->Enable( false );
		}
	private:
		void fOnCheckBoxChecked( wxCommandEvent& event )
		{
			tSelectObjectsCursor* cursor = 
				dynamic_cast< tSelectObjectsCursor* >( fGetParent( )->fMainWindow( ).fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
			if( cursor )
				cursor->fSetIsGizmoWorldSpace( mWorldSpace->GetValue( )!=0 );
			fGetParent( )->fMainWindow( ).SetFocus( );
		}
	};
	class tRotateObjectCursorButton : public tEditorCursorControllerButton
	{
		define_dynamic_cast( tRotateObjectCursorButton, tWxSlapOnRadioBitmapButton );
		tManipulationGizmoPtr mGizmo;
		wxCheckBox*				mWorldSpace;
	public:
		tRotateObjectCursorButton( tEditorCursorControllerButtonGroup* parent, wxCheckBox* worldSpace )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateRotateSel" ), wxBitmap( "ManipulateRotateDeSel" ), "Rotate objects (E)" )
			, mWorldSpace( worldSpace )
		{
			mWorldSpace->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( tRotateObjectCursorButton::fOnCheckBoxChecked ), NULL, this );
		}
		virtual void fOnSelected( )
		{
			mWorldSpace->Enable( true );
		}
		virtual void fOnDeselected( )
		{
			mWorldSpace->Enable( false );
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( new tRotationGizmoGeometry( 
					fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) ) );
				mGizmo.fReset( new tRotationGizmo( gizmoGeom ) );
			}

			tSelectObjectsCursor* cursor = new tSelectObjectsCursor( this, "Rotate Objects", mGizmo );
			cursor->fSetIsGizmoWorldSpace( mWorldSpace->GetValue( )!=0 );
			return tEditorCursorControllerPtr( cursor );
		}
	private:
		void fOnCheckBoxChecked( wxCommandEvent& event )
		{
			tSelectObjectsCursor* cursor = 
				dynamic_cast< tSelectObjectsCursor* >( fGetParent( )->fMainWindow( ).fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
			if( cursor )
				cursor->fSetIsGizmoWorldSpace( mWorldSpace->GetValue( )!=0 );
			fGetParent( )->fMainWindow( ).SetFocus( );
		}
	};
	class tScaleObjectCursorButton : public tEditorCursorControllerButton
	{
		tManipulationGizmoPtr mGizmo;
	public:
		tScaleObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateScaleSel" ), wxBitmap( "ManipulateScaleDeSel" ), "Scale objects (R)" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( new tScaleGizmoGeometry( 
					fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) ) );
				mGizmo.fReset( new tScaleGizmo( gizmoGeom ) );
			}
			return tEditorCursorControllerPtr( new tSelectObjectsCursor( this, "Scale Objects", mGizmo ) );
		}
	};

	//------------------------------------------------------------------------------
	// tManipulateObjectPanel
	//------------------------------------------------------------------------------
	tManipulateObjectPanel::tManipulateObjectPanel( tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Manipulate", "Manipulate Object", "Gizmo" )
		, mButtonGroup( 0 )
		, mSelectObjectButton( 0 )
	{
		mButtonGroup = new tEditorCursorControllerButtonGroup( this, "Gizmo Type", false );

		const s32 textBoxWidth = 60;
		tWxCoordinate* specifyX = new tWxCoordinate( mButtonGroup->fGetMainPanel( ), wxDefaultPosition, wxSize( textBoxWidth, wxDefaultSize.y ) );
		tWxCoordinate* specifyY = new tWxCoordinate( mButtonGroup->fGetMainPanel( ), wxDefaultPosition, wxSize( textBoxWidth, wxDefaultSize.y ) );
		tWxCoordinate* specifyZ = new tWxCoordinate( mButtonGroup->fGetMainPanel( ), wxDefaultPosition, wxSize( textBoxWidth, wxDefaultSize.y ) );

		specifyX->fSetValue( 0.f, false ); specifyX->Enable( false );
		specifyY->fSetValue( 0.f, false ); specifyY->Enable( false );
		specifyZ->fSetValue( 0.f, false ); specifyZ->Enable( false );

		wxBoxSizer* horizontalSizer = new wxBoxSizer( wxHORIZONTAL );
		horizontalSizer->Add( specifyX, 0, wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 4 );
		horizontalSizer->Add( specifyY, 0, wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 4 );
		horizontalSizer->Add( specifyZ, 0, wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 4 );

		wxButton* clearButton = new wxButton( mButtonGroup->fGetMainPanel( ), wxID_ANY, wxT("Reset"), wxDefaultPosition, wxSize( 40, wxDefaultSize.y ) );
		horizontalSizer->Add( clearButton, 0, wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 4 );

		mButtonGroup->fGetMainPanel( )->GetSizer( )->Add( horizontalSizer, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5 );

		mWorldSpace = new wxCheckBox( mButtonGroup->fGetMainPanel( ), wxID_ANY, "Use World Axes" );
		mWorldSpace->Enable( false );
		mButtonGroup->fGetMainPanel( )->GetSizer( )->Add( mWorldSpace, 0, wxALIGN_CENTER, 1 );

		tEditorHotKeyTable& hotKeyTable = fGuiApp( ).fHotKeys( );

		mSelectObjectButton = new tSelectObjectCursorButton( mButtonGroup, specifyX, specifyY, specifyZ, clearButton );
		mSelectHotKey.fReset( 
			new tEnableCursorButtonHotKey( mSelectObjectButton, hotKeyTable, Input::tKeyboard::cButtonQ ) );
		mTranslateHotKey.fReset( 
			new tEnableCursorButtonHotKey( new tTranslateObjectCursorButton( mButtonGroup, mWorldSpace ), hotKeyTable, Input::tKeyboard::cButtonW ) );
		mRotateHotKey.fReset( 
			new tEnableCursorButtonHotKey( new tRotateObjectCursorButton( mButtonGroup, mWorldSpace ), hotKeyTable, Input::tKeyboard::cButtonE ) );
		mScaleHotKey.fReset( 
			new tEnableCursorButtonHotKey( new tScaleObjectCursorButton( mButtonGroup ), hotKeyTable, Input::tKeyboard::cButtonR ) );

		mButtonGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		clearButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tSelectObjectCursorButton::fOnResetFieldsPressed), NULL, mSelectObjectButton );
		mWorldSpace->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tManipulateObjectPanel::fOnWorldAxesClicked), NULL, this );
	}

	void tManipulateObjectPanel::fSetSelectionCursor( )
	{
		mButtonGroup->fSetSelected( 0u );
	}

	void tManipulateObjectPanel::fOnTick( )
	{
		b32 hasFocus = false;

		if( !fGuiApp( ).fMainWindow( ).fDialogInputActive( ) )
			hasFocus = mSelectObjectButton->fUpdateCoordinates( );
		if( hasFocus )
			fGuiApp( ).fMainWindow( ).fSetDialogInputActive( );
	}

	void tManipulateObjectPanel::fOnWorldAxesClicked( wxCommandEvent& event )
	{
		// Save all the data to the registry.
		fSave( );
		event.Skip();
	}

	void tManipulateObjectPanel::fSaveInternal( HKEY hKey )
	{
		Win32Util::fSetRegistryKeyValue( hKey, ( b32 )( mWorldSpace->GetValue() ), "UseWorldAxes" );

		tWxToolsPanelTool::fSaveInternal( hKey );
	}
	void tManipulateObjectPanel::fLoadInternal( HKEY hKey )
	{
		b32 tempUseWorldAxes = true;
		Win32Util::fGetRegistryKeyValue( hKey, tempUseWorldAxes, "UseWorldAxes" );
		mWorldSpace->SetValue( tempUseWorldAxes == 1 );

		tWxToolsPanelTool::fLoadInternal( hKey );
	}

}
