#include "SigFxPch.hpp"
#include "tManipulateEffectPanel.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tToolsGuiMainWindow.hpp"

#include "tWxNumericText.hpp"
#include "tWxSlapOnCheckBox.hpp"

#include "tSelectObjectsCursor.hpp"
#include "tMouseFollowCursor.hpp"

#include "tTranslationGizmo.hpp"
#include "tRotationGizmo.hpp"
#include "tScaleGizmo.hpp"

#include "tTranslationGizmoGeometry.hpp"
#include "tRotationGizmoGeometry.hpp"
#include "tScaleGizmoGeometry.hpp"
#include "tMouseFollowGizmoGeometry.hpp"

#include "tEffectTranslationGizmo.hpp"
#include "tObjectScaleGizmo.hpp"
#include "tEffectRotationGizmo.hpp"

#include "tEffectPositionOffsetGizmo.hpp"
#include "tEffectScaleGizmo.hpp"
#include "tEffectFollowMouseGizmo.hpp"

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
		tWxCoordinate* mSpecifyX;
		tWxCoordinate* mSpecifyY; 
		tWxCoordinate* mSpecifyZ; 
	public:
		tSelectObjectCursorButton( tEditorCursorControllerButtonGroup* parent, tWxCoordinate* specifyX, tWxCoordinate* specifyY, tWxCoordinate* specifyZ )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateSelectionSel" ), wxBitmap( "ManipulateSelectionDeSel" ), "Select objects (Q)" )
			, mSpecifyX( specifyX )
			, mSpecifyY( specifyY )
			, mSpecifyZ( specifyZ )
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
		virtual void fOnCoordinateChanged( )
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
		}
		void fDisableCoordinates( )
		{
			mSpecifyX->fSetValue( 0.f, false ); mSpecifyX->Enable( false );
			mSpecifyY->fSetValue( 0.f, false ); mSpecifyY->Enable( false );
			mSpecifyZ->fSetValue( 0.f, false ); mSpecifyZ->Enable( false );
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
		tSigFxKeyline*			mTheKeyline;
	public:
		tTranslateObjectCursorButton( tEditorCursorControllerButtonGroup* parent, wxCheckBox* worldSpace, tSigFxKeyline* Keyline )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateTranslateSel" ), wxBitmap( "ManipulateTranslateDeSel" ), "Translate objects (W)" )
			, mWorldSpace( worldSpace )
			, mTheKeyline( Keyline )
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
				mGizmo.fReset( new tEffectTranslationGizmo( gizmoGeom, mTheKeyline ) );
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
		tManipulationGizmoPtr mGizmo;
	public:
		tRotateObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateRotateSel" ), wxBitmap( "ManipulateRotateDeSel" ), "Rotate objects (E)" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( new tRotationGizmoGeometry( 
					fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) ) );
					
				mGizmo.fReset( new tEffectRotationGizmo( gizmoGeom ) );
			}

			return tEditorCursorControllerPtr( new tSelectObjectsCursor( this, "Rotate Objects", mGizmo ) );
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
				mGizmo.fReset( new tObjectScaleGizmo( gizmoGeom ) );
			}
			return tEditorCursorControllerPtr( new tSelectObjectsCursor( this, "Scale Objects", mGizmo ) );
		}
	};


	class tPositionOffsetObjectCursorButton : public tEditorCursorControllerButton
	{
		tManipulationGizmoPtr mGizmo;
		tSigFxKeyline* mTheKeyline;
	public:
		tPositionOffsetObjectCursorButton( tEditorCursorControllerButtonGroup* parent, tSigFxKeyline *Keyline )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateTranslateSel" ), wxBitmap( "ManipulateTranslateDeSel" ), "Translation-over-Time(T)" )
			, mTheKeyline( Keyline )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( new tTranslationGizmoGeometry( 
					fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) ) );
				mGizmo.fReset( new tEffectPositionOffsetGizmo( gizmoGeom, mTheKeyline) );
			}
			return tEditorCursorControllerPtr( new tSelectObjectsCursor( this, "Translation-over-Time", mGizmo ) );
		}
	};
	class tScaleEffectCursorButton : public tEditorCursorControllerButton
	{
		tManipulationGizmoPtr mGizmo;
		tSigFxKeyline* mTheKeyline;
	public:
		tScaleEffectCursorButton( tEditorCursorControllerButtonGroup* parent, tSigFxKeyline *Keyline )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateScaleSel" ), wxBitmap( "ManipulateScaleDeSel" ), "Scale-over-Time (Y)" )
			, mTheKeyline( Keyline )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( new tScaleGizmoGeometry( 
					fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) ) );
				mGizmo.fReset( new tEffectScaleGizmo( gizmoGeom, mTheKeyline) );
			}
			return tEditorCursorControllerPtr( new tSelectObjectsCursor( this, "Scale-over-Time", mGizmo ) );
		}
	};

	class tEffectFollowMouseCursorButton : public tEditorCursorControllerButton
	{
		tManipulationGizmoPtr mGizmo;
		tSigFxKeyline* mTheKeyline;
	public:
		tEffectFollowMouseCursorButton( tEditorCursorControllerButtonGroup* parent, tSigFxKeyline *Keyline )
			: tEditorCursorControllerButton( parent, wxBitmap( "ManipulateTranslateSel" ), wxBitmap( "ManipulateTranslateDeSel" ), "Follow Mouse (U)" )
			, mTheKeyline( Keyline )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( new tMouseFollowGizmoGeometry( 
					fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), gfx->fGetSolidColorMaterial( ), gfx->fGetSolidColorGeometryAllocator( ), gfx->fGetSolidColorIndexAllocator( ) ) );
				mGizmo.fReset( new tEffectFollowMouseGizmo( gizmoGeom, mTheKeyline) );
			}
			return tEditorCursorControllerPtr( new tMouseFollowCursor( this, "Follow Mouse", mGizmo ) );
		}

		virtual void fOnNextCursorController( )
		{
			if( mGizmo )
			{
				tEffectFollowMouseGizmo* gizmo = dynamic_cast< tEffectFollowMouseGizmo* >( mGizmo.fGetRawPtr( ) );
				if( gizmo )
					gizmo->fEndThis( );
			}
			mGizmo.fRelease( );
		}
	};
	
	
	
	


	
	tManipulateEffectPanel::tManipulateEffectPanel( tWxToolsPanel* parent, tSigFxKeyline* Keyline )
		: tWxToolsPanelTool( parent, "Manipulate", "Manipulate Object", "Gizmo" )
		, mButtonGroup( 0 )
		, mSelectObjectButton( 0 )
		, mTheKeyline( Keyline )
	{
		mButtonGroup = new tEditorCursorControllerButtonGroup( this, "Gizmo Type", false, 4 );

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
		mButtonGroup->fGetMainPanel( )->GetSizer( )->Add( horizontalSizer, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5 );

		wxCheckBox* worldSpace = new wxCheckBox( mButtonGroup->fGetMainPanel( ), wxID_ANY, "Use World Axes" );
		worldSpace->SetValue( true );
		worldSpace->Enable( false );
		mButtonGroup->fGetMainPanel( )->GetSizer( )->Add( worldSpace, 0, wxALIGN_CENTER, 1 );

		tEditorHotKeyTable& hotKeyTable = fGuiApp( ).fHotKeys( );

		mSelectObjectButton = new tSelectObjectCursorButton( mButtonGroup, specifyX, specifyY, specifyZ );
		mSelectHotKey.fReset( 
			new tEnableCursorButtonHotKey( mSelectObjectButton, hotKeyTable, Input::tKeyboard::cButtonQ ) );
		mTranslateHotKey.fReset( 
			new tEnableCursorButtonHotKey( new tTranslateObjectCursorButton( mButtonGroup, worldSpace, mTheKeyline ), hotKeyTable, Input::tKeyboard::cButtonW ) );
		mRotateHotKey.fReset( 
			new tEnableCursorButtonHotKey( new tRotateObjectCursorButton( mButtonGroup ), hotKeyTable, Input::tKeyboard::cButtonE ) );
		mScaleHotKey.fReset( 
			new tEnableCursorButtonHotKey( new tScaleObjectCursorButton( mButtonGroup ), hotKeyTable, Input::tKeyboard::cButtonR ) );
		mPositionOffsetHotKey.fReset( 
			new tEnableCursorButtonHotKey( new tPositionOffsetObjectCursorButton( mButtonGroup, mTheKeyline ), hotKeyTable, Input::tKeyboard::cButtonT ) );
		tEffectScaleHotKey.fReset( 
			new tEnableCursorButtonHotKey( new tScaleEffectCursorButton( mButtonGroup, mTheKeyline ), hotKeyTable, Input::tKeyboard::cButtonY ) );
		tEffectFollowMouseHotKey.fReset(
			new tEnableCursorButtonHotKey( new tEffectFollowMouseCursorButton( mButtonGroup, mTheKeyline ), hotKeyTable, Input::tKeyboard::cButtonU ) );

		mButtonGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );
	}

	tManipulateEffectPanel::~tManipulateEffectPanel( )
	{
		delete mSelectObjectButton;
		delete mButtonGroup;		
	}

	void tManipulateEffectPanel::fSetSelectionCursor( )
	{
		mButtonGroup->fSetSelected( 0u );
	}

	void tManipulateEffectPanel::fOnTick( )
	{
		b32 hasFocus( false );

		if( !fGuiApp( ).fMainWindow( ).fDialogInputActive( ) )
			hasFocus = mSelectObjectButton->fUpdateCoordinates( );
		if( hasFocus )
			fGuiApp( ).fMainWindow( ).fSetDialogInputActive( );
	}

}
