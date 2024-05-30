//------------------------------------------------------------------------------
// \file tManipulateObjectPanel.cpp - 23 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigAnimPch.hpp"
#include "tManipulateObjectPanel.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "tSelectObjectsCursor.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tTranslationGizmo.hpp"
#include "tTranslationGizmoGeometry.hpp"
#include "tRotationGizmo.hpp"
#include "tRotationGizmoGeometry.hpp"
#include "tScaleGizmo.hpp"
#include "tScaleGizmoGeometry.hpp"
#include "tWxNumericText.hpp"


namespace Sig
{
	///
	/// \class tWxCoordinate
	/// \brief 
	class tWxCoordinate : public tWxNumericText
	{
	public:

		class tOwner
		{
		public:
			virtual void fOnCoordinateChanged( ) = 0;
		};

		tWxCoordinate( 
			wxWindow* parent, 
			const wxPoint& pos = wxDefaultPosition, 
			const wxSize& size = wxDefaultSize )
			: tWxNumericText( parent, -99999999.f, +99999999.f, 2, pos, size ), mOwner( 0 ) { }
		
		inline void fSetOwner( tOwner * owner ) { mOwner = owner; }
		inline tOwner * fOwner( ) const { return mOwner; }

		virtual void fOnValueUpdated( )
		{
			if( mOwner )
				mOwner->fOnCoordinateChanged( );
		}

	private:

		tOwner * mOwner;

	};

	///
	/// \class tWxOwnedButton
	/// \brief 
	class tWxOwnedButton : public wxButton
	{
	public:
		
		class tOwner
		{
		public:
			virtual void fOnButtonPressed( ) = 0;
		};
		
		tWxOwnedButton( 
			wxWindow * parent, 
			const char * label, 
			const wxPoint& pos = wxDefaultPosition, 
			const wxSize& size = wxDefaultSize  )
			: wxButton( parent, wxID_ANY, label, pos, size ), mOwner( 0 )
		{
			Connect( 
				wxEVT_COMMAND_BUTTON_CLICKED, 
				wxCommandEventHandler( tWxOwnedButton::fOnPressed ), 
				NULL, 
				this );
		}

		inline void fSetOwner( tOwner * owner ) { mOwner = owner; }
		inline tOwner * fOwner( ) const { return mOwner; }

	private:

		virtual void fOnPressed( wxCommandEvent & )
		{
			if( mOwner )
				mOwner->fOnButtonPressed( );
		}

	private:
		
		tOwner * mOwner;
	};

	///
	/// \class tGizmoCursorButton
	/// \brief 
	class tGizmoCursorButton :
		public tEditorCursorControllerButton, 
		public tWxCoordinate::tOwner,
		public tWxOwnedButton::tOwner
	{

	public:

		tGizmoCursorButton( 
			tEditorCursorControllerButtonGroup* parent,
			const char * selBitmap,
			const char * deselBitmap,
			const char * toolTip,
			tWxCoordinate * x, 
			tWxCoordinate * y, 
			tWxCoordinate * z,
			tWxOwnedButton * reset )
			: tEditorCursorControllerButton( 
				parent, 
				wxBitmap( selBitmap ), 
				wxBitmap( deselBitmap ), 
				toolTip )
			, mWasDragging( false )
		{
			mCoords[0] = x;
			mCoords[1] = y;
			mCoords[2] = z;
			mReset = reset;

			mOnSelChanged.fFromMethod<tGizmoCursorButton, &tGizmoCursorButton::fOnSelectionChanged>( this );

			tEditableObjectContainer & container = fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( );
			container.fGetSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelChanged );
		}

		virtual void fOnSelected( )
		{
			fUpdateValues( );

			for( u32 i = 0; i < array_length( mCoords ); ++i )
			{
				mCoords[ i ]->fSetOwner( this );
				mCoords[ i ]->Enable( );
			}

			mReset->fSetOwner( this );
			mReset->Enable( );

			mWasDragging = false;
		}

		virtual void fOnDeselected( )
		{
			for( u32 i = 0; i < array_length( mCoords ); ++i )
			{
				mCoords[ i ]->Disable( );
				mCoords[ i ]->fSetOwner( 0 );
			}

			mReset->fSetOwner( 0 );
			mReset->Disable( );
		}

		virtual void fOnCoordinateChanged( )
		{
			tSelectObjectsCursor* cursor = fGetCursor( );
			if( cursor )
			{
				b32 ind[ array_length( mCoords ) ];
				Math::tVec3f coord;
				for( u32 i = 0; i < array_length( mCoords ); ++i )
				{
					ind[ i ] = mCoords[ i ]->fIsIndeterminate( );
					coord[ i ] = mCoords[ i ]->fGetValue( );
				}

				cursor->fSetWorldCoords( coord, !ind[ 0 ], !ind[ 1 ], !ind[ 2 ] );
			}

			fGetParent( )->fMainWindow( ).SetFocus( );
		}

		virtual void fOnButtonPressed( )
		{
			tSelectObjectsCursor* cursor = fGetCursor( );
			if( cursor )
			{
				Math::tVec3f defaultCoords;
				if( cursor->fGetDefaultCoords( defaultCoords ) )
				{
					b32 ind[ array_length( mCoords ) ];
					for( u32 i = 0; i < array_length( mCoords ); ++i )
					{
						ind[ i ] = mCoords[ i ]->fIsIndeterminate( );
						mCoords[ i ]->fSetValue( defaultCoords[ i ], false );
					}

					cursor->fSetWorldCoords( defaultCoords, !ind[ 0 ], !ind[ 1 ], !ind[ 2 ] );
				}
			}

			fGetParent( )->fMainWindow( ).SetFocus( );
		}

		virtual void fUpdate( )
		{
			// No gizmo means no change
			if( mGizmo.fNull( ) )
				return;

			// Only if we own the controls do we affect them
			if( mReset->fOwner( ) != this )
				return;

			b32 dragging = mGizmo->fGetDragging( );
			if( dragging || mWasDragging )
			{
				fUpdateValues( );
				mWasDragging = dragging;
			}
		}

	protected:

		void fUpdateValues( )
		{
			tSelectObjectsCursor* cursor = fGetCursor( );
			Math::tVec3f coord = cursor ? cursor->fGetWorldCoords( ) : Math::tVec3f::cZeroVector;

			for( u32 i = 0; i < array_length( mCoords ); ++i )
				mCoords[ i ]->fSetValue( coord[ i ], false );
		}

		virtual void fOnSelectionChanged( tEditorSelectionList & )
		{
			if( mReset->fOwner( ) == this )
				fUpdateValues( );
		}

		tSelectObjectsCursor * fGetCursor( )
		{
			return dynamic_cast< tSelectObjectsCursor* >( 
				fGetParent( )->fMainWindow( ).fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );
		}


	protected:

		tManipulationGizmoPtr mGizmo;
		tWxCoordinate * mCoords[ 3 ];
		tWxOwnedButton * mReset;

	private:

		b32 mWasDragging;
		tEditorSelectionList::tOnSelectionChanged::tObserver mOnSelChanged;

	};


	///
	/// \class tScaleObjectsCursorButton
	/// \brief 
	class tScaleObjectsCursorButton : public tGizmoCursorButton
	{
	public:
		
		tScaleObjectsCursorButton( 
			tEditorCursorControllerButtonGroup* parent,
			tWxCoordinate * x, 
			tWxCoordinate * y, 
			tWxCoordinate * z,
			tWxOwnedButton * reset )
			: tGizmoCursorButton( 
				parent, 
				"ManipScaleSel", 
				"ManipScaleDeSel", 
				"Scale objects (R)", 
				x, y, z, reset )
		{
		}

		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( 
					new tScaleGizmoGeometry( 
						fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), 
						gfx->fGetSolidColorMaterial( ), 
						gfx->fGetSolidColorGeometryAllocator( ), 
						gfx->fGetSolidColorIndexAllocator( ) ) );

				mGizmo.fReset( new tScaleGizmo( gizmoGeom ) );
			}

			return tEditorCursorControllerPtr( 
				new tSelectObjectsCursor( this, "Scale Objects", mGizmo ) );
		}

	};

	///
	/// \class tRotateObjectsCursorButton
	/// \brief 
	class tRotateObjectsCursorButton : public tGizmoCursorButton
	{
	public:

		tRotateObjectsCursorButton( 
			tEditorCursorControllerButtonGroup* parent,
			tWxCoordinate * x, 
			tWxCoordinate * y, 
			tWxCoordinate * z,
			tWxOwnedButton * reset )
			: tGizmoCursorButton( 
				parent, 
				"ManipRotateSel", 
				"ManipRotateDeSel", 
				"Rotate objects (E)",
				x, y, z, reset )
		{
		}

		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer* gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( 
					new tRotationGizmoGeometry( 
						fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), 
						gfx->fGetSolidColorMaterial( ), 
						gfx->fGetSolidColorGeometryAllocator( ), 
						gfx->fGetSolidColorIndexAllocator( ) ) );

				mGizmo.fReset( new tRotationGizmo( gizmoGeom ) );
			}

			return tEditorCursorControllerPtr( 
				new tSelectObjectsCursor( this, "Rotate Objects", mGizmo ) );
		}

	};

	///
	/// \class tTranslateObjectsCursorButton
	/// \brief 
	class tTranslateObjectsCursorButton : public tGizmoCursorButton
	{
	public:
		tTranslateObjectsCursorButton( 
			tEditorCursorControllerButtonGroup* parent,
			tWxCoordinate * x, 
			tWxCoordinate * y, 
			tWxCoordinate * z,
			tWxOwnedButton * reset )
			: tGizmoCursorButton( 
				parent, 
				"ManipTranslateSel", 
				"ManipTranslateDeSel", 
				"Translate objects (W)",
				x, y, z, reset )
		{
		}

		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			if( mGizmo.fNull( ) )
			{
				tWxRenderPanelContainer * gfx = fGetParent( )->fMainWindow( ).fRenderPanelContainer( );
				tGizmoGeometryPtr gizmoGeom( 
					new tTranslationGizmoGeometry( 
						fGetParent( )->fMainWindow( ).fGuiApp( ).fGfxDevice( ), 
						gfx->fGetSolidColorMaterial( ), 
						gfx->fGetSolidColorGeometryAllocator( ), 
						gfx->fGetSolidColorIndexAllocator( ) ) );

				mGizmo.fReset( new tTranslationGizmo( gizmoGeom ) );
			}

			tSelectObjectsCursor* cursor = new tSelectObjectsCursor( this, "Translate Objects", mGizmo );
			cursor->fSetIsGizmoWorldSpace( true );

			return tEditorCursorControllerPtr( cursor );
		}

	};

	///
	/// \class tSelectObjectsCursorButton
	/// \brief 
	class tSelectObjectsCursorButton : public tEditorCursorControllerButton
	{
	public:

		tSelectObjectsCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( 
				parent, 
				wxBitmap( "ManipSelectSel" ), 
				wxBitmap( "ManipSelectDeSel" ), 
				"Select objects (Q)" )
		{
		}

		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( 
				new tSelectObjectsCursor( this, "Select Objects" ) );
		}
	};

	//------------------------------------------------------------------------------
	// tManipulateObjectPanel
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	tManipulateObjectPanel::tManipulateObjectPanel( tWxToolsPanel * parent )
		: tWxToolsPanelTool( parent, "Manipulate", "Manipulate Objects", "Gizmo" )
	{
		fGetMainPanel( )->SetForegroundColour( wxColor( 0xff, 0xff, 0xff ) );
		mButtonGroup = new tEditorCursorControllerButtonGroup( this, "Gizmo Type", false );

		tEditorHotKeyTable& hotKeyTable = fGuiApp( ).fHotKeys( );

		wxBoxSizer * sizer = new wxBoxSizer( wxHORIZONTAL );

		const s32 coordSize = 40;
		tWxCoordinate * xCoord = new tWxCoordinate( 
			fGetMainPanel( ), wxDefaultPosition, wxSize( coordSize, wxDefaultSize.y ) );
		tWxCoordinate * yCoord = new tWxCoordinate( 
			fGetMainPanel( ), wxDefaultPosition, wxSize( coordSize, wxDefaultSize.y ) );
		tWxCoordinate * zCoord = new tWxCoordinate( 
			fGetMainPanel( ), wxDefaultPosition, wxSize( coordSize, wxDefaultSize.y ) );
		tWxOwnedButton * reset = new tWxOwnedButton( 
			fGetMainPanel( ), "Reset", wxDefaultPosition, wxSize( coordSize, wxDefaultSize.y ) );

		sizer->Add( xCoord, 0, wxALL, 4 );
		sizer->Add( yCoord, 0, wxALL, 4 );
		sizer->Add( zCoord, 0, wxALL, 4 );
		sizer->Add( reset, 0, wxALL, 4 );

		mSelectHotKey.fReset( 
			new tEnableCursorButtonHotKey( 
				new tSelectObjectsCursorButton( mButtonGroup ), 
				hotKeyTable, 
				Input::tKeyboard::cButtonQ ) );

		mGizmos[ 0 ] = new tTranslateObjectsCursorButton( mButtonGroup, xCoord, yCoord, zCoord, reset );
		mGizmos[ 1 ] = new tRotateObjectsCursorButton( mButtonGroup, xCoord, yCoord, zCoord, reset );
		mGizmos[ 2 ] = new tScaleObjectsCursorButton( mButtonGroup, xCoord, yCoord, zCoord, reset );

		mTranslateHotKey.fReset( 
			new tEnableCursorButtonHotKey( mGizmos[ 0 ], hotKeyTable, Input::tKeyboard::cButtonW ) );
		mRotateHotKey.fReset( 
			new tEnableCursorButtonHotKey( mGizmos[ 1 ], hotKeyTable, Input::tKeyboard::cButtonE ) );
		mScaleHotKey.fReset( 
			new tEnableCursorButtonHotKey( mGizmos[ 2 ], hotKeyTable, Input::tKeyboard::cButtonR ) );

		fGetMainPanel( )->GetSizer( )->Add( sizer, 0, wxALL | wxALIGN_CENTER, 4 );
	}

	//------------------------------------------------------------------------------
	void tManipulateObjectPanel::fOnTick( )
	{
		// Set the cursor to the select cursor if not exists
		if( !fGuiApp( ).fCurrentCursor( ) )
			mSelectHotKey->fFire( );

		for( u32 i = 0; i < array_length( mGizmos ); ++i )
			mGizmos[ i ]->fUpdate( );
	}

	void tManipulateObjectPanel::fSetSelectionCursor( )
	{
		mButtonGroup->fSetSelected( 0u );
	}
}
