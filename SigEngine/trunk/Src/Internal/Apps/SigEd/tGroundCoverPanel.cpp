//------------------------------------------------------------------------------
// \file tGroundCoverPanel.cpp - 08 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigEdPch.hpp"
#include "tGroundCoverPanel.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "tHeightFieldPaintCursor.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "wx/listctrl.h"
#include "wx/radiobox.h"
#include "tConfigurableBrowserTree.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tEditableObjectContainer.hpp"
#include "tWxToolsPanelSlider.hpp"
#include "tEditorDialog.hpp"
#include "tEditorAppWindow.hpp"
#include "tWxSlapOnComboBox.hpp"
#include "tWxProgressDialog.hpp"
#include "tWxSlapOnCheckBox.hpp"

namespace Sig
{
	namespace
	{
		static const u32 cLayerHeight = 26;
		static const wxString cVisibleText = "V";
		static const wxString cInvisibleText = "H";
		static const wxString cNoSelectedObjectText = "[No Object]";
		static const wxColor cLayerSelectedColor( 0xff, 0xbb, 0xbb );
		static const wxColor cLayerUnselectedColor( 0xbb, 0xff, 0xbb );
		static const f32 cNudgeAmount = 0.1f;

		enum
		{
			cActionProperties = 1,
		};
	}

	class tGroundCoverLayerDialog;

	///
	/// \class tTerrainCursorGroundCoverRenderGridHotKey
	/// \brief 
	class tTerrainCursorGroundCoverRenderGridHotKey : public tEditorHotKey
	{
		tGroundCoverPanel * mOwner;
	public:
		tTerrainCursorGroundCoverRenderGridHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton6, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fToggleCursorGrid( ); }
	};

	///
	/// \class tTerrainCursorGroundCoverSizeIncHotKey
	/// \brief 
	class tTerrainCursorGroundCoverSizeIncHotKey : public tEditorHotKey
	{
		tGroundCoverPanel * mOwner;
	public:
		tTerrainCursorGroundCoverSizeIncHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton1, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorSize( +cNudgeAmount ); }
	};

	///
	/// \class tTerrainCursorGroundCoverSizeDecHotKey
	/// \brief 
	class tTerrainCursorGroundCoverSizeDecHotKey : public tEditorHotKey
	{
		tGroundCoverPanel* mOwner;
	public:
		tTerrainCursorGroundCoverSizeDecHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton1, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorSize( -cNudgeAmount ); }
	};

	///
	/// \class tTerrainCursorGroundCoverStrengthIncHotKey
	/// \brief 
	class tTerrainCursorGroundCoverStrengthIncHotKey : public tEditorHotKey
	{
		tGroundCoverPanel* mOwner;
	public:
		tTerrainCursorGroundCoverStrengthIncHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton2, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorStrength( +cNudgeAmount ); }
	};

	///
	/// \class tTerrainCursoGroundCoverStrengthDecHotKey
	/// \brief 
	class tTerrainCursorGroundCoverStrengthDecHotKey : public tEditorHotKey
	{
		tGroundCoverPanel* mOwner;
	public:
		tTerrainCursorGroundCoverStrengthDecHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton2, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorStrength( -cNudgeAmount ); }
	};

	///
	/// \class tTerrainCursorGroundCoverFalloffIncHotKey
	/// \brief 
	class tTerrainCursorGroundCoverFalloffIncHotKey : public tEditorHotKey
	{
		tGroundCoverPanel* mOwner;
	public:
		tTerrainCursorGroundCoverFalloffIncHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton3, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorFalloff( +cNudgeAmount ); }
	};

	///
	/// \class tTerrainCursorGroundCoverFalloffDecHotKey
	/// \brief 
	class tTerrainCursorGroundCoverFalloffDecHotKey : public tEditorHotKey
	{
		tGroundCoverPanel* mOwner;
	public:
		tTerrainCursorGroundCoverFalloffDecHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton3, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorFalloff( -cNudgeAmount ); }
	};

	///
	/// \class tTerrainCursorGroundCoverShapeIncHotKey
	/// \brief 
	class tTerrainCursorGroundCoverShapeIncHotKey : public tEditorHotKey
	{
		tGroundCoverPanel* mOwner;
	public:
		tTerrainCursorGroundCoverShapeIncHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton4, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorShape( +cNudgeAmount ); }
	};

	///
	/// \class tTerrainCursorGroundCoverShapeDecHotKey
	/// \brief 
	class tTerrainCursorGroundCoverShapeDecHotKey : public tEditorHotKey
	{
		tGroundCoverPanel* mOwner;
	public:
		tTerrainCursorGroundCoverShapeDecHotKey( tGroundCoverPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton4, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorShape( -cNudgeAmount ); }
	};

	///
	/// \class tGroundCoverEditLayerAction
	/// \brief 
	class tGroundCoverEditLayerAction : public tEditorAction
	{
	public:

		tGroundCoverEditLayerAction( u32 id, tGroundCoverPanel * panel );
		
		virtual void fUndo( );
		virtual void fRedo( );
		virtual void fEnd( );

	private:

		struct tEntityEdit
		{
			tEntityPtr mEntity;
			tTerrainGeometry::tGroundCover mCover;
		};

		tGroundCoverPanel * mPanel;
		Sigml::tGroundCoverLayer mStart, mEnd;
		tDynamicArray< tEntityEdit > mEntities;
	};

	///
	/// \class tGroundCoverLayerVisibilityAction
	/// \brief 
	class tGroundCoverLayerVisibilityAction : public tEditorAction
	{
	public:

		tGroundCoverLayerVisibilityAction(u32 id, tGroundCoverPanel * panel, b32 _far );

		virtual void fUndo( );
		virtual void fRedo( );
		virtual void fEnd( );

	private:
		u32 mId;
		tGroundCoverPanel * mPanel;
		b32 mFar;

		
		u32 mStart, mEnd;
	};

	///
	/// \class tGroundCoverLayerToggleVisibleAction
	/// \brief 
	class tGroundCoverLayerToggleVisibleAction : public tEditorAction
	{
	public:
		tGroundCoverLayerToggleVisibleAction( u32 id, tGroundCoverPanel * panel );

		virtual void fUndo( );
		virtual void fRedo( );

	private:

		u32 mId;
		tGroundCoverPanel * mPanel;
		b32 mStart;
	};

	///
	/// \class tGroundCoverLayerTitleAction
	/// \brief 
	class tGroundCoverLayerTitleAction : public tEditorAction
	{
	public:

		tGroundCoverLayerTitleAction( u32 id, tGroundCoverPanel * panel );
		virtual void fUndo( );
		virtual void fRedo( );
		virtual void fEnd( );

	private:

		u32 mId;
		tGroundCoverPanel * mPanel;
		std::string mStart, mEnd;
	};

	///
	/// \class tGroundCoverBrushSlider
	/// \brief 
	class tGroundCoverBrushSlider : public tWxToolsPanelSlider
	{
		tGroundCoverPanel * mOwner;
	public:
		tGroundCoverBrushSlider( 
			tGroundCoverPanel * owner, 
			wxWindow* parent, 
			const char* labelName, 
			f32 initialValue )
			: tWxToolsPanelSlider( parent, labelName, &owner->fGuiApp( ).fMainWindow( ), initialValue )
			, mOwner( owner ) { }

		virtual void fOnValueChanged( ) { mOwner->fOnSlidersChanged( ); }
	};

	///
	/// \class tGroundCoverRefreshHeightsDialog
	/// \brief 
	class tGroundCoverRefreshHeightsDialog : public tWxProgressDialog
	{
	public:
		tGroundCoverRefreshHeightsDialog( wxWindow * parent )
			: tWxProgressDialog( parent, "Refreshing Heights", false ) { SetSize( 300, GetSize( ).y ); }
	};

	///
	/// \class tGroundCoverLayerGui
	/// \brief 
	class tGroundCoverLayerGui : public wxPanel, public Sigml::tGroundCoverLayer
	{
	public:

		tGroundCoverLayerGui( 
			tGroundCoverPanel * owner,
			wxWindow * parent,
			const tGroundCoverLayer & layer,
			u32 sizerIndex )
			: wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.x, cLayerHeight ), wxBORDER_SIMPLE )
			, tGroundCoverLayer( layer )
			, mOwner( owner )
			, mSelected( false )
		{
			fCommonConstruct( sizerIndex );
		}

		tGroundCoverLayerGui( 
			tGroundCoverPanel * owner, 
			wxWindow * parent, 
			u32 uniqueId,
			const char * title, 
			u32 sizerIndex )
			: wxPanel( 
				parent, 
				wxID_ANY, 
				wxDefaultPosition, 
				wxSize( wxDefaultSize.x, cLayerHeight ), 
				wxBORDER_SIMPLE )
			, tGroundCoverLayer( uniqueId, title )
			, mOwner( owner )
			, mSelected( false )
		{
			fCommonConstruct( sizerIndex );
		}

		void fSetTitle( const char * title, b32 setUI ) 
		{ 
			fStartTitleEdit( );

			fSetName( title );
			if( setUI )
				mTextName->SetValue( title );

			fEndEdit( );
		};

		void fSetTitleNoAction( const char * title )
		{
			fSetName( title );
			mTextName->SetValue( title );
		}

		void fSetUnitSize( f32 size ) 
		{
			fStartEdit( );
			tGroundCoverLayer::fSetUnitSize( size );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetPaintUnits( u32 units ) 
		{ 
			fStartEdit( );
			tGroundCoverLayer::fSetPaintUnits( units );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetRotation( u32 rotation )
		{
			fStartEdit( );
			tGroundCoverLayer::fSetRotation( rotation );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetYRotationScale( f32 scale )
		{
			fStartEdit( );
			tGroundCoverLayer::fSetYRotationScale( scale );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetXZRotationScale( f32 scale )
		{
			fStartEdit( );
			tGroundCoverLayer::fSetXZRotationScale( scale );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetTranslation( u32 translation )
		{
			fStartEdit( );
			tGroundCoverLayer::fSetTranslation( translation );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetXZTranslationScale( f32 scale )
		{
			fStartEdit( );
			tGroundCoverLayer::fSetXZTranslationScale( scale );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetYTranslationScale( f32 scale )
		{
			fStartEdit( );
			tGroundCoverLayer::fSetYTranslationScale( scale );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetScaleRangeAdjustor( f32 adj )
		{
			fStartEdit( );
			tGroundCoverLayer::fSetScaleRangeAdjustor( adj );
			fEndEdit( );
			
			fUpdateGroundCover( );
		}

		void fSetFarVisibility( u32 vis, b32 isEdit )
		{
			if( isEdit )
				fStartVisibilityEdit( true );

			tGroundCoverLayer::fSetFarVisibility( vis );

			if( isEdit )
				fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetNearVisibility( u32 vis, b32 isEdit )
		{
			if( isEdit )
				fStartVisibilityEdit( false );

			tGroundCoverLayer::fSetNearVisibility( vis );

			if( isEdit )
				fEndEdit( );

			fUpdateGroundCover( );
		}

		tFilePathPtr fAddSigml( const tFilePathPtr & file )
		{
			tFilePathPtr resRelFile = ToolsPaths::fMakeResRelative( file );
			if( tGroundCoverLayer::fFindElement( resRelFile ) )
				return tFilePathPtr( );

			// Add the element
			fStartEdit( );
			tGroundCoverLayer::fAddElement( resRelFile );
			fEndEdit( );

			fUpdateGroundCover( );
			return resRelFile;
		}

		void fRemoveElement( const tFilePathPtr & path )
		{
			fStartEdit( );
			tGroundCoverLayer::fRemoveElement( path );
			fEndEdit( );

			fUpdateGroundCover( );
		}

		void fSetLayer( const Sigml::tGroundCoverLayer & layer )
		{
			// Note: this is not an edit action
			*static_cast<Sigml::tGroundCoverLayer *>( this ) = layer;
			fUpdateGroundCover( );

			// Update ui for any changes
			mVisibleButton->SetLabel( fVisible( ) ? cVisibleText : cInvisibleText );
			mTextName->SetValue( fName( ).c_str( ) );
		}

		void fSetSelected( b32 selected )
		{
			mSelected = selected;

			wxColor color = selected ? cLayerSelectedColor : cLayerUnselectedColor;
			SetBackgroundColour( color );
			mTextName->SetBackgroundColour( color );
		}

		b32 fIsSelected( ) const { return mSelected; }

		void fSetVisible( b32 visible, b32 isEdit )
		{
			if( visible == fVisible( ) )
				return;

			if( isEdit )
				fStartVisibleEdit( );

			Sigml::tGroundCoverLayer::fSetVisible( visible );
			mVisibleButton->SetLabel( visible ? cVisibleText : cInvisibleText );

			tGrowableArray< tEditableTerrainGeometry * > ents;
			mOwner->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

			const u32 entCount = ents.fCount( );
			for( u32 e = 0; e < entCount; ++e )
				ents[ e ]->fUpdateGroundCoverVisiblity( *this );

			if( isEdit )
			{
				fEndEdit( );
				mOwner->fMarkDirty( );
			}
		}

	private:

		void fStartEdit( )
		{
			sigassert( mCurrentAction.fNull( ) );
			mCurrentAction.fReset( new tGroundCoverEditLayerAction( fUniqueId( ), mOwner ) );
			mOwner->fGuiApp( ).fActionStack( ).fAddAction( mCurrentAction );
		}

		void fStartVisibilityEdit( b32 _far )
		{
			sigassert( mCurrentAction.fNull( ) );
			mCurrentAction.fReset( new tGroundCoverLayerVisibilityAction( fUniqueId( ), mOwner, _far ) );
			mOwner->fGuiApp( ).fActionStack( ).fAddAction( mCurrentAction );
		}

		void fStartVisibleEdit( )
		{
			sigassert( mCurrentAction.fNull( ) );
			mCurrentAction.fReset( new tGroundCoverLayerToggleVisibleAction( fUniqueId( ), mOwner ) );
			mOwner->fGuiApp( ).fActionStack( ).fAddAction( mCurrentAction );
		}

		void fStartTitleEdit( )
		{
			sigassert( mCurrentAction.fNull( ) );
			mCurrentAction.fReset( new tGroundCoverLayerTitleAction( fUniqueId( ), mOwner ) );
			mOwner->fGuiApp( ).fActionStack( ).fAddAction( mCurrentAction );
		}

		void fEndEdit( )
		{
			sigassert( !mCurrentAction.fNull( ) );
			mCurrentAction->fEnd( );
			mCurrentAction.fRelease( );
		}

		void fUpdateGroundCover( )
		{
			tGrowableArray< tEditableTerrainGeometry * > ents;
			mOwner->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

			tWxProgressDialog * progress = new tWxProgressDialog( 
				&mOwner->fGuiApp( ).fMainWindow( ), "Updating Ground Cover...", false );
			progress->SetSize( 300, progress->GetSize( ).y );

			const u32 entCount = ents.fCount( );
			for( u32 i = 0; i < entCount; ++i )
			{
				std::stringstream str;
				str << "Terrain Entity " << i + 1 << "/" << entCount;
				progress->fUpdate( i / (f32)entCount, str.str( ).c_str( ) );
				ents[ i ]->fUpdateGroundCover( *this );
				
			}

			delete progress;

			mOwner->fMarkDirty( );
		}

		void fCommonConstruct( u32 sizerIndex )
		{
			SetMinSize( wxSize( wxDefaultSize.x, cLayerHeight ) );
			SetMaxSize( wxSize( wxDefaultSize.x, cLayerHeight ) );

			SetBackgroundColour( cLayerUnselectedColor );

			Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tGroundCoverLayerGui::fOnContextMenu ) );
			Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tGroundCoverLayerGui::fOnAction ) );
			Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( tGroundCoverLayerGui::fOnFocus ) );

			// Add ourselves to the parent
			GetParent( )->GetSizer( )->Insert( sizerIndex, this, 1, wxEXPAND | wxTOP, 4 );

			// Build our sizer
			wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
			SetSizer( hSizer );

			// Build our ui
			mVisibleButton	= new wxButton( 
				this, 
				wxID_ANY, 
				fVisible( ) ? cVisibleText : cInvisibleText, 
				wxDefaultPosition, 
				wxSize( 20, 20 ) );
			mVisibleButton->Connect( 
				wxEVT_COMMAND_BUTTON_CLICKED, 
				wxCommandEventHandler( tGroundCoverLayerGui::fOnToggleVisible ), 
				NULL, this );

			mTextName = new wxTextCtrl( 
				this, 
				wxID_ANY, 
				fName( ).c_str( ), 
				wxDefaultPosition, 
				wxSize( 180, wxDefaultSize.y ), 
				wxBORDER_NONE | wxTE_PROCESS_ENTER );
			mTextName->SetBackgroundColour( GetBackgroundColour( ) );
			mTextName->Connect( 
				wxEVT_COMMAND_TEXT_ENTER, 
				wxTextEventHandler( tGroundCoverLayerGui::fOnNameEntered ), NULL, this );
			mTextName->Connect( 
				wxEVT_SET_FOCUS, 
				wxFocusEventHandler( tGroundCoverLayerGui::fOnNameGotFocus ), NULL, this );
			mTextName->Connect( 
				wxEVT_KILL_FOCUS, 
				wxFocusEventHandler( tGroundCoverLayerGui::fOnNameLostFocus ), NULL, this );
			mTextName->Connect( 
				wxEVT_RIGHT_UP, 
				wxMouseEventHandler( tGroundCoverLayerGui::fOnContextMenu ), NULL, this  );
			
			wxButton * buttonDelete	= new wxButton( this, wxID_ANY, "X", wxDefaultPosition, wxSize( 20, 20 ) );
			buttonDelete->SetForegroundColour( wxColour( 0x99, 0x00, 0x00 ) );
			buttonDelete->Connect( 
				wxEVT_COMMAND_BUTTON_CLICKED, 
				wxCommandEventHandler( tGroundCoverLayerGui::fOnDelete ), 
				NULL, this );

			// Add our ui
			hSizer->Add( mVisibleButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4 );
			hSizer->Add( mTextName, 0, wxALIGN_LEFT | wxALIGN_TOP | wxLEFT | wxTOP, 6 );
			hSizer->AddStretchSpacer( );
			hSizer->Add( buttonDelete, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4 );
		}

		void fOnToggleVisible( wxCommandEvent & )
		{
			fSetVisible( mVisibleButton->GetLabelText( ) != cVisibleText, true );
		}


		void fOnDelete( wxCommandEvent & )
		{
			mOwner->fDeleteLayer( this );
			// Return immediately;
		}

		void fOnNameEntered( wxCommandEvent & )
		{
			fRename( );
		}

		void fOnNameGotFocus( wxFocusEvent & )
		{
			fOnFocus( );
			mOwner->fSetDialogInputActive( true );
		}

		void fOnNameLostFocus( wxFocusEvent & )
		{
			mOwner->fSetDialogInputActive( false );
			fRename( );
		}

		void fRename( )
		{
			if( !mOwner->fRenameLayer( this, std::string( mTextName->GetValue( ) ) ) )
				mTextName->SetValue( fName( ).c_str( ) );
		}

		void fOnContextMenu( wxMouseEvent & e)
		{
			e.Skip( false );

			wxMenu menu;
			menu.Append( cActionProperties, "Properties...");

			PopupMenu( &menu, e.GetPosition( ) );
		}

		void fOnAction( wxCommandEvent& e )
		{
			switch( e.GetId( ) )
			{
			case cActionProperties:
				mOwner->fShowProperties( this );
				break;
			}
		}

		void fOnFocus( wxFocusEvent & )
		{
			fOnFocus( );
		}

		void fOnFocus( )
		{
			mOwner->fSelectLayer( this );
		}

	private:

		tEditorActionPtr mCurrentAction;
		tGroundCoverPanel * mOwner;
		wxTextCtrl * mTextName;
		wxButton * mVisibleButton;
		b32 mSelected;
	};

	//------------------------------------------------------------------------------
	// tGroundCoverEditLayerAction
	//------------------------------------------------------------------------------
	tGroundCoverEditLayerAction::tGroundCoverEditLayerAction( u32 id, tGroundCoverPanel * panel )
		: mPanel( panel )
		, mStart( *panel->fFindLayer( id ) )
	{
		fSetIsLive( true );

		tGrowableArray< tEditableTerrainGeometry * > ents;
		mPanel->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

		const u32 entCount = ents.fCount( );
		mEntities.fNewArray( entCount );

		for( u32 e = 0; e < entCount; ++e )
		{
			tEntityEdit & edit = mEntities[ e ];
			edit.mEntity.fReset( ents[ e ] );
			
			edit.mCover.mCoverId = id;
			ents[ e ]->fSaveGroundCover( edit.mCover );
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverEditLayerAction::fUndo( )
	{
		mPanel->fRestoreLayer( mStart );
		
		const u32 entCount = mEntities.fCount( );
		for( u32 e = 0; e < entCount; ++e )
		{
			const tEntityEdit & edit = mEntities[ e ];
			tEditableTerrainGeometry * ent = edit.mEntity->fDynamicCast< tEditableTerrainGeometry >( );
			ent->fRestoreGroundCover( edit.mCover );
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverEditLayerAction::fRedo( )
	{
		mPanel->fRestoreLayer( mEnd );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverEditLayerAction::fEnd(  )
	{
		mEnd = *mPanel->fFindLayer( mStart.fUniqueId( ) );
		tEditorAction::fEnd( );
	}

	//------------------------------------------------------------------------------
	// tGroundCoverLayerVisibilityAction
	//------------------------------------------------------------------------------
	tGroundCoverLayerVisibilityAction::tGroundCoverLayerVisibilityAction(u32 id, tGroundCoverPanel * panel, b32 _far )
		: mId( id )
		, mPanel( panel )
		, mFar( _far )
	{
		if( mFar ) mStart = panel->fFindLayer( id )->fFarVisibility( );
		else mStart = panel->fFindLayer( id )->fNearVisibility( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayerVisibilityAction::fUndo( )
	{
		if( mFar ) mPanel->fFindLayer( mId )->fSetFarVisibility( mStart, false );
		else mPanel->fFindLayer( mId )->fSetNearVisibility( mStart, false );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayerVisibilityAction::fRedo( )
	{
		if( mFar ) mPanel->fFindLayer( mId )->fSetFarVisibility( mEnd, false );
		else mPanel->fFindLayer( mId )->fSetNearVisibility( mEnd, false );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayerVisibilityAction::fEnd( )
	{
		if( mFar ) mEnd = mPanel->fFindLayer( mId )->fFarVisibility( );
		else mEnd = mPanel->fFindLayer( mId )->fNearVisibility( );
	}

	//------------------------------------------------------------------------------
	// tGroundCoverLayerToggleVisibleAction
	//------------------------------------------------------------------------------
	tGroundCoverLayerToggleVisibleAction::tGroundCoverLayerToggleVisibleAction( u32 id, tGroundCoverPanel * panel )
		: mId( id )
		, mPanel( panel )
	{
		mStart = panel->fFindLayer( id )->fVisible( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayerToggleVisibleAction::fUndo( )
	{
		mPanel->fFindLayer( mId )->fSetVisible( mStart, false );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayerToggleVisibleAction::fRedo( )
	{
		mPanel->fFindLayer( mId )->fSetVisible( !mStart, false );
	}

	//------------------------------------------------------------------------------
	// tGroundCoverLayerTitleAction
	//------------------------------------------------------------------------------
	tGroundCoverLayerTitleAction::tGroundCoverLayerTitleAction( u32 id, tGroundCoverPanel * panel )
		: mId( id )
		, mPanel( panel )
	{
		mStart = panel->fFindLayer( id )->fName( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayerTitleAction::fUndo( )
	{
		mPanel->fFindLayer( mId )->fSetTitleNoAction( mStart.c_str( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayerTitleAction::fRedo( )
	{
		mPanel->fFindLayer( mId )->fSetTitleNoAction( mEnd.c_str( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverLayerTitleAction::fEnd( )
	{
		mEnd = mPanel->fFindLayer( mId )->fName( );
		tEditorAction::fEnd( );
	}

	///
	/// \class tSigmlBrowser
	/// \brief 
	class tSigmlBrowser : public tConfigurableBrowserTree
	{
	public:

		tSigmlBrowser( wxWindow * parent, tGroundCoverLayerDialog * owner )
			: tConfigurableBrowserTree( parent, Sigml::fIsSigmlFile, 400, true, false )
			, mOwner( owner )
		{
		}

	private:

		virtual void fOpenDoc( const tFilePathPtr& file );

		virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData )
		{
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".sigml" ) )
				return wxColour( 0x00, 0x77, 0x00 );
			if( StringUtil::fStrStrI( fileEntryData.fXmlPath( ).fCStr( ), ".mshml" ) )
				return wxColour( 0x00, 0x00, 0x77 );

			return wxColour( 0, 0, 0 );
		}


	private:

		tGroundCoverLayerDialog * mOwner;
	};

	///
	/// \class tWxSlapOnLayerSpinner
	/// \brief 
	class tWxSlapOnLayerSpinner : public tWxSlapOnSpinner
	{
	public:

		tWxSlapOnLayerSpinner( 
			wxWindow * parent, 
			const char * label, 
			f32 min, f32 max, 
			f32 increment, u32 precision )
			: tWxSlapOnSpinner( parent, label, min, max, increment, precision ) { }

		void fSetLayer( tGroundCoverLayerGui * layer )
		{
			mLayer = layer;
			if( layer )
				fSetValueNoEvent( fGetLayerValue( ) );
		}
		
	protected:

		virtual void fOnControlUpdated( )
		{
			if( mLayer )
				fSetLayerValue( fGetValue( ) );
		}

		virtual f32 fGetLayerValue( ) const = 0;
		virtual void fSetLayerValue( f32 value ) = 0;

		tGroundCoverLayerGui * mLayer;
	};

	///
	/// \class tWxSlapOnUnitSizeSpinner
	/// \brief 
	class tWxSlapOnUnitSizeSpinner : public tWxSlapOnLayerSpinner
	{
	public:
		tWxSlapOnUnitSizeSpinner( wxWindow * parent )
			: tWxSlapOnLayerSpinner( parent, "Unit Size", 0.01f, 100.f, 0.01f, 4 ) { }

	protected:

		virtual f32 fGetLayerValue( ) const
		{
			return mLayer->fUnitSize( );
		}

		virtual void fSetLayerValue( f32 value )
		{
			mLayer->fSetUnitSize( value );
		}
	};

	///
	/// \class tWxSlapOnPaintUnitSpinner
	/// \brief 
	class tWxSlapOnPaintUnitSpinner : public tWxSlapOnLayerSpinner
	{
	public:
		tWxSlapOnPaintUnitSpinner( wxWindow * parent )
			: tWxSlapOnLayerSpinner( parent, "Paint Units", 1, 20, 1, 1 ) { }

	protected:

		virtual f32 fGetLayerValue( ) const
		{
			return (f32)mLayer->fPaintUnits( );
		}

		virtual void fSetLayerValue( f32 value )
		{
			mLayer->fSetPaintUnits( (u32)value );
		}
	};

	///
	/// \class tWxLayerSlider
	/// \brief 
	class tWxLayerSlider : public tWxToolsPanelSlider
	{
	public:

		tWxLayerSlider( wxWindow * parent, wxWindow * editorWindow, const char * label )
			: tWxToolsPanelSlider( parent, label, editorWindow ) { }

		void fSetLayer( tGroundCoverLayerGui * layer )
		{
			mLayer = layer;
			if( layer )
				fSetValue( fGetLayerValue( ) );
		}
		
	protected:

		virtual f32 fGetLayerValue( ) const = 0;
		virtual void fSetLayerValue( f32 value ) = 0;

		tGroundCoverLayerGui * mLayer;

	private:

		virtual void fOnValueChanged( )
		{
			fSetLayerValue( fGetValue( ) );
		}

	};

	///
	/// \class tWxYRotationLayerSlider
	/// \brief 
	class tWxYRotationLayerSlider : public tWxLayerSlider
	{
	public:

		tWxYRotationLayerSlider( wxWindow * parent, wxWindow * editorWindow ) 
			: tWxLayerSlider( parent, editorWindow, "Y Rotation Scale" ) { }

	protected:

		virtual f32 fGetLayerValue( ) const
		{
			return mLayer->fYRotationScale( );
		}

		virtual void fSetLayerValue( f32 value )
		{
			mLayer->fSetYRotationScale( value );
		}

	};

	///
	/// \class tWxXZRotationLayerSlider
	/// \brief 
	class tWxXZRotationLayerSlider : public tWxLayerSlider
	{
	public:

		tWxXZRotationLayerSlider( wxWindow * parent, wxWindow * editorWindow ) 
			: tWxLayerSlider( parent, editorWindow, "XZ Rotation Scale" ) { }

	protected:

		virtual f32 fGetLayerValue( ) const
		{
			return mLayer->fXZRotationScale( );
		}

		virtual void fSetLayerValue( f32 value )
		{
			mLayer->fSetXZRotationScale( value );
		}

	};

	///
	/// \class tWxXZOffsetLayerSlider
	/// \brief 
	class tWxXZOffsetLayerSlider : public tWxLayerSlider
	{
	public:

		tWxXZOffsetLayerSlider( wxWindow * parent, wxWindow * editorWindow ) 
			: tWxLayerSlider( parent, editorWindow, "XZ Translation Scale" ) { }

	protected:

		virtual f32 fGetLayerValue( ) const
		{
			return mLayer->fXZTranslationScale( );
		}

		virtual void fSetLayerValue( f32 value )
		{
			mLayer->fSetXZTranslationScale( value );
		}

	};

	///
	/// \class tWxYOffsetLayerSlider
	/// \brief 
	class tWxYOffsetLayerSlider : public tWxLayerSlider
	{
	public:

		tWxYOffsetLayerSlider( wxWindow * parent, wxWindow * editorWindow ) 
			: tWxLayerSlider( parent, editorWindow, "Y Translation Scale" ) { }

	protected:

		virtual f32 fGetLayerValue( ) const
		{
			return mLayer->fYTranslationScale( );
		}

		virtual void fSetLayerValue( f32 value )
		{
			mLayer->fSetYTranslationScale( value );
		}

	};

	///
	/// \class tWxScaleRangeLayerSlider
	/// \brief 
	class tWxScaleRangeLayerSlider : public tWxLayerSlider
	{
	public:
		tWxScaleRangeLayerSlider( wxWindow * parent, wxWindow * editorWindow )
			: tWxLayerSlider( parent, editorWindow, "Scale Range" ) { }

	protected:

		virtual f32 fGetLayerValue( ) const
		{
			return mLayer->fScaleRangeAdjustor( );
		}

		virtual void fSetLayerValue( f32 value )
		{
			mLayer->fSetScaleRangeAdjustor( value );
		}
	};

	///
	/// \class tWxLayerVisibilityComboBox
	/// \brief 
	class tWxLayerVisibilityComboBox : public tWxSlapOnComboBox
	{
	public:

		tWxLayerVisibilityComboBox( wxWindow * parent, b32 _far )
			: tWxSlapOnComboBox( parent, _far ? "Far Fade" : "Near Fade", 0, wxEXPAND )
			, mFar( _far )
			, mLayer( 0 )
		{
			fAddString( "Near" );
			fAddString( "Medium" );
			fAddString( "Far" );
			fAddString( "NoFade" );
		}

		void fSetLayer( tGroundCoverLayerGui * layer )
		{
			if( layer == mLayer )
				return;

			mLayer = layer;
			if( layer )
				fSetSelection( mFar ? layer->fFarVisibility( ) : layer->fNearVisibility( ) );
		}

	private:

		virtual void fOnControlUpdated( )
		{
			if( mLayer )
			{
				if( mFar ) mLayer->fSetFarVisibility( fGetSelection( ), true );
				else mLayer->fSetNearVisibility( fGetSelection( ), true );
			}
		}

	private:

		b32 mFar;
		tGroundCoverLayerGui * mLayer;

	};

	///
	/// \class tWxRenderShadowsCheckBox
	/// \brief 
	class tWxRenderShadowsCheckBox : public tWxSlapOnCheckBox
	{
	public:

		tWxRenderShadowsCheckBox( tGroundCoverPanel * panel, wxWindow * parent ) 
			: tWxSlapOnCheckBox( parent, "Cast Shadows" )
			, mPanel( panel )
		{
			fDisableControl( );
		}

		void fSetElement( const tFilePathPtr & path )
		{
			mPath = path;

			if( path.fNull( ) )
			{
				fSetValue( tWxSlapOnCheckBox::cFalse );
				fDisableControl( );
			}
			else
			{
				fEnableControl( );

				const Sigml::tGroundCoverLayer::tElement * element = 
					mPanel->fSelectedLayer( )->fFindElement( path );
				sigassert( element );
				
				fSetValue( 
					element->mCastsShadow ? 
					tWxSlapOnCheckBox::cTrue : 
					tWxSlapOnCheckBox::cFalse );
			}
		}

	protected:

		virtual void fOnControlUpdated( )
		{
			tGroundCoverLayerGui * layer = mPanel->fSelectedLayer( );
			Sigml::tGroundCoverLayer::tElement * element = layer->fFindElement( mPath );
			sigassert( element );

			element->mCastsShadow = fGetValueBool( );

			// Update the ground cover
			tGrowableArray< tEditableTerrainGeometry * > ents;
			mPanel->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

			const u32 entCount = ents.fCount( );
			for( u32 i = 0; i < entCount; ++i )
				ents[ i ]->fUpdateGroundCoverShadows( *layer );

			mPanel->fMarkDirty( );
		}

		tFilePathPtr mPath;
		tGroundCoverPanel * mPanel;
	};

	///
	/// \class tWxFrequencySlider
	/// \brief 
	class tWxFrequencySlider : public tWxToolsPanelSlider
	{
	public:

		tWxFrequencySlider( tGroundCoverPanel * panel, wxWindow * parent )
			: tWxToolsPanelSlider( parent, "Frequency", &panel->fGuiApp( ).fMainWindow( ), 1 )
			, mPanel( panel )
		{
		}

		void fSetElement( const tFilePathPtr & path )
		{
			mPath = path;

			if( path.fNull( ) )
			{
				fSetValue( 1 );
				Disable( );
			}
			else
			{
				Enable( );

				const Sigml::tGroundCoverLayer::tElement * element = 
					mPanel->fSelectedLayer( )->fFindElement( path );
				sigassert( element );
				
				fSetValue( element->mFrequency );
			}
		}

	protected:

		virtual void fOnValueChanged( )
		{
			tGroundCoverLayerGui * layer = mPanel->fSelectedLayer( );
			Sigml::tGroundCoverLayer::tElement * element = layer->fFindElement( mPath );
			sigassert( element );

			element->mFrequency = fGetValue( );

			tWxProgressDialog * progress = new tWxProgressDialog( 
				&mPanel->fGuiApp( ).fMainWindow( ), "Updating Ground Cover...", false );
			progress->SetSize( 300, progress->GetSize( ).y );

			// Update the ground cover
			tGrowableArray< tEditableTerrainGeometry * > ents;
			mPanel->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

			const u32 entCount = ents.fCount( );
			for( u32 i = 0; i < entCount; ++i )
			{
				std::stringstream str;
				str << "Terrain Entity " << i + 1 << "/" << entCount;
				progress->fUpdate( i / (f32)entCount, str.str( ).c_str( ) );

				ents[ i ]->fUpdateGroundCoverFrequency( *layer );
			}

			delete progress;

			mPanel->fMarkDirty( );
		}

	private:

		tFilePathPtr mPath;
		tGroundCoverPanel * mPanel;
	};

	///
	/// \class tWxSlapOnPaintUnitSpinner
	/// \brief 
	class tWxSpawnCountSpinner : public tWxSlapOnSpinner
	{
	public:
		tWxSpawnCountSpinner( tGroundCoverPanel * panel, wxWindow * parent )
			: tWxSlapOnSpinner( parent, "Spawn Count", 1, 100, 1, 1 ) 
			, mPanel( panel ) 
		{ 
		}

		void fSetElement( const tFilePathPtr & path )
		{
			mPath = path;

			if( path.fNull( ) )
			{
				fSetValueNoEvent( 1 );
				fDisableControl( );
			}
			else
			{
				fEnableControl( );

				const Sigml::tGroundCoverLayer::tElement * element = 
					mPanel->fSelectedLayer( )->fFindElement( path );
				sigassert( element );
				
				fSetValueNoEvent( element->mSpawnCount );
			}
		}

	private:

		virtual void fOnControlUpdated( )
		{
			tGroundCoverLayerGui * layer = mPanel->fSelectedLayer( );
			Sigml::tGroundCoverLayer::tElement * element = layer->fFindElement( mPath );
			sigassert( element );

			element->mSpawnCount = (u32)fGetValue( );

			tWxProgressDialog * progress = new tWxProgressDialog( 
				&mPanel->fGuiApp( ).fMainWindow( ), "Updating Ground Cover...", false );
			progress->SetSize( 300, progress->GetSize( ).y );

			// Update the ground cover
			tGrowableArray< tEditableTerrainGeometry * > ents;
			mPanel->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

			const u32 entCount = ents.fCount( );
			for( u32 i = 0; i < entCount; ++i )
			{
				std::stringstream str;
				str << "Terrain Entity " << i + 1 << "/" << entCount;
				progress->fUpdate( i / (f32)entCount, str.str( ).c_str( ) );

				ents[ i ]->fUpdateGroundCoverSpawnCount( *layer );
			}

			delete progress;

			mPanel->fMarkDirty( );
		}

	private:

		tFilePathPtr mPath;
		tGroundCoverPanel * mPanel;
	};


	///
	/// \class tGroundCoverLayerDialog
	/// \brief 
	class tGroundCoverLayerDialog : public tEditorDialog
	{
	public:
		tGroundCoverLayerDialog( tEditorAppWindow * window, tGroundCoverPanel * owner, wxWindow * parent )
			: tEditorDialog( window, "GroundCoverDialog" )
			, mOwner( owner )
		{
			
			SetSizer( new wxBoxSizer( wxVERTICAL ) );

			wxScrolledWindow * mainPanel = new wxScrolledWindow( this );
			GetSizer( )->Add( mainPanel, 1, wxEXPAND | wxALL, 0 );

			mainPanel->SetScrollbars( 0, 20, 1, 50 );

			// World
			{
				tWxSlapOnGroup * group = new tWxSlapOnGroup( mainPanel, "World", true );

				wxString rotStrs[] = { "None", "About Y", "About XYZ" };
				mRotationBox = new wxRadioBox( 
					group->fGetMainPanel( ), 
					wxID_ANY, 
					"Random Rotation", 
					wxDefaultPosition, 
					wxDefaultSize, 
					array_length( rotStrs ), 
					rotStrs );
				mRotationBox->Connect(
					wxEVT_COMMAND_RADIOBOX_SELECTED,
					wxCommandEventHandler( tGroundCoverLayerDialog::fOnRandomRotation ), NULL, this );
				group->fGetMainPanel( )->GetSizer( )->Add( mRotationBox, 1, wxEXPAND | wxALL, 5 );
				mYRotationSlider = new tWxYRotationLayerSlider( group->fGetMainPanel( ), &owner->fGuiApp( ).fMainWindow( ) );
				mXZRotationSlider = new tWxXZRotationLayerSlider( group->fGetMainPanel( ), &owner->fGuiApp( ).fMainWindow( ) );
				

				wxString transStrs[] = { "None", "Along XZ", "Along XYZ" };
				mTranslationBox = new wxRadioBox( 
					group->fGetMainPanel( ), 
					wxID_ANY, 
					"Random Translation", 
					wxDefaultPosition, 
					wxDefaultSize, 
					array_length( transStrs ), 
					transStrs );
				mTranslationBox->Connect(
					wxEVT_COMMAND_RADIOBOX_SELECTED,
					wxCommandEventHandler( tGroundCoverLayerDialog::fOnRandomTranslation ), NULL, this );
				group->fGetMainPanel( )->GetSizer( )->Add( mTranslationBox, 1, wxEXPAND | wxALL, 5 );

				mXZOffsetSlider = new tWxXZOffsetLayerSlider( group->fGetMainPanel( ), &owner->fGuiApp( ).fMainWindow( ) );
				mYOffsetSlider = new tWxYOffsetLayerSlider( group->fGetMainPanel( ), &owner->fGuiApp( ).fMainWindow( ) );
				mScaleRangeSlider = new tWxScaleRangeLayerSlider( group->fGetMainPanel( ), &owner->fGuiApp( ).fMainWindow( ) );
			}

			// Density
			{

				tWxSlapOnGroup * group = new tWxSlapOnGroup( mainPanel, "Density", true );
				
				mUnitSizeSpinner = new tWxSlapOnUnitSizeSpinner( group->fGetMainPanel( ) );
				mPaintUnitSpinner = new tWxSlapOnPaintUnitSpinner( group->fGetMainPanel( ) );
				//mPaintUnitSpinner->fDisableControl( );
			}

			// Visibility
			{
				tWxSlapOnGroup * group = new tWxSlapOnGroup( mainPanel, "Visiblity", true );
				mFarVisBox = new tWxLayerVisibilityComboBox( group->fGetMainPanel( ), true );
				mNearVisBox = new tWxLayerVisibilityComboBox( group->fGetMainPanel( ), false );
			}
			
			// Objects
			{
				tWxSlapOnGroup * group = new tWxSlapOnGroup( mainPanel, "Elements", true );

				// Properties
				{
					mObjectProperties = new tWxSlapOnGroup( group->fGetMainPanel( ), cNoSelectedObjectText, false );

					mFrequencySlider = new tWxFrequencySlider( owner, mObjectProperties->fGetMainPanel( ) );
					mSpawnCounterSpinner = new tWxSpawnCountSpinner( owner, mObjectProperties->fGetMainPanel( ) );
					mRenderShadowsBox = new tWxRenderShadowsCheckBox( owner, mObjectProperties->fGetMainPanel( ) );
				}

				mCoverObjectList = new wxListBox( group->fGetMainPanel( ), wxID_ANY );
				mCoverObjectList->Connect(
					wxEVT_COMMAND_LISTBOX_SELECTED,
					wxCommandEventHandler( tGroundCoverLayerDialog::fOnObjectSelected ), NULL, this );
				mCoverObjectList->Connect( 
					wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, 
					wxCommandEventHandler( tGroundCoverLayerDialog::fOnRemoveObject ), NULL, this );


				mBrowser = new tSigmlBrowser( group->fGetMainPanel( ), this );

				group->fGetMainPanel( )->GetSizer( )->Add( mCoverObjectList, 1, wxEXPAND | wxALL, 5 );
				group->fGetMainPanel( )->GetSizer( )->Add( mBrowser, 2, wxEXPAND | wxALL, 5 );
			}


			Fit( );
			fLoad( );
		}

		void fSetLayer( tGroundCoverLayerGui * layer )
		{
			// Skip needless updating
			if( layer == mLayer )
				return;

			mLayer = layer;

			if( !mLayer )
			{
				if( IsShown( ) )
					Show( false );

				return;
			}

			mUnitSizeSpinner->fSetLayer( layer );
			mPaintUnitSpinner->fSetLayer( layer );
			mYRotationSlider->fSetLayer( layer );
			mXZRotationSlider->fSetLayer( layer );
			mXZOffsetSlider->fSetLayer( layer );
			mYOffsetSlider->fSetLayer( layer );
			mScaleRangeSlider->fSetLayer( layer );
			mFarVisBox->fSetLayer( layer );
			mNearVisBox->fSetLayer( layer );

			// Object properties
			fSetObjectProperties( -1 );

			Freeze( );

			mRotationBox->SetSelection( mLayer->fRotation( ) );
			mTranslationBox->SetSelection( mLayer->fTranslation( ) );

			std::stringstream ss;
			ss << "Ground Cover (" << layer->fName( ) << ") Settings...";

			SetTitle( ss.str( ) );

			mCoverObjectList->Clear( );
			const u32 count = layer->fElementCount( );
			const Sigml::tGroundCoverLayer::tElement * elements = layer->fElements( );
			for( u32 e = 0; e < count; ++e )
				fAddSigmlUi( elements[ e ].mSgPath );

			if( count )
			{
				mCoverObjectList->SetSelection( 0);
				fSetObjectProperties( 0 );
			}

			Thaw( );
		}

		void fUpdateLayer( tGroundCoverLayerGui * layer )
		{
			if( layer != mLayer )
				return;

			mLayer = 0;
			fSetLayer( layer );
		}
		
		void fRefresh( )
		{
			mBrowser->fRefresh( );
		}

		void fAddSigml( const tFilePathPtr & file )
		{
			tFilePathPtr resRelFile = mLayer->fAddSigml( file );
			if( resRelFile.fNull( ) )
				return;

			Freeze( );
			fAddSigmlUi( resRelFile );
			Thaw( );
		}

	private:

		struct tWxPath : public wxClientData, public tFilePathPtr 
		{ 
			tWxPath( const tFilePathPtr & path ) : tFilePathPtr( path ) { }
		};

		void fAddSigmlUi( const tFilePathPtr & resRelFile )
		{
			// Build the list box str
			std::string name = StringUtil::fNameFromPath( resRelFile.fCStr( ), true );
			std::stringstream str;
			str << "[" 
				<< name
				<< "] "
				<< StringUtil::fStripExtension( resRelFile.fCStr( ) );

			mCoverObjectList->Append( str.str( ).c_str( ), new tWxPath( resRelFile ) );
		}

		void fOnRandomRotation( wxCommandEvent & e )
		{
			if( mLayer->fRotation( ) == e.GetInt( ) )
				return;

			mLayer->fSetRotation( e.GetInt( ) );
		}

		void fOnRandomTranslation( wxCommandEvent & e )
		{
			if( mLayer->fTranslation( ) == e.GetInt( ) )
				return;

			mLayer->fSetTranslation( e.GetInt( ) );
		}

		void fOnObjectSelected( wxCommandEvent & e )
		{
			fSetObjectProperties( e.GetInt( ) );
		}

		void fSetObjectProperties( s32 id )
		{
			if( id < 0 )
			{
				mObjectProperties->fSetLabel( cNoSelectedObjectText );
				mFrequencySlider->fSetElement( tFilePathPtr( ) );
				mSpawnCounterSpinner->fSetElement( tFilePathPtr( ) );
				mRenderShadowsBox->fSetElement( tFilePathPtr( ) );
				return;
			}

			tWxPath * path = (tWxPath *)mCoverObjectList->GetClientObject( id );

			mObjectProperties->fSetLabel( mCoverObjectList->GetString( id ) );
			mFrequencySlider->fSetElement( *path );
			mSpawnCounterSpinner->fSetElement( *path );
			mRenderShadowsBox->fSetElement( *path );
		}

		void fOnRemoveObject( wxCommandEvent & e )
		{
			mLayer->fRemoveElement( *(tWxPath *)e.GetClientObject( ) );

			if( mCoverObjectList->IsSelected( e.GetInt( ) ) )
				fSetObjectProperties( -1 );

			Freeze( );
			mCoverObjectList->Delete( e.GetInt( ) );
			Thaw( );
		}

	private:

		tGroundCoverPanel * mOwner;

		tWxSlapOnLayerSpinner *	mUnitSizeSpinner;
		tWxSlapOnLayerSpinner *	mPaintUnitSpinner;
		
		tWxLayerSlider *		mYRotationSlider;
		tWxLayerSlider *		mXZRotationSlider;
		tWxLayerSlider *		mXZOffsetSlider;
		tWxLayerSlider *		mYOffsetSlider;
		tWxLayerSlider *		mScaleRangeSlider;
		wxRadioBox *			mRotationBox;
		wxRadioBox *			mTranslationBox;

		tWxLayerVisibilityComboBox * mFarVisBox;
		tWxLayerVisibilityComboBox * mNearVisBox;

		// Object properties
		tWxSlapOnGroup * mObjectProperties;
		tWxFrequencySlider * mFrequencySlider;
		tWxSpawnCountSpinner * mSpawnCounterSpinner;
		tWxRenderShadowsCheckBox * mRenderShadowsBox;


		wxListBox * mCoverObjectList;
		tSigmlBrowser * mBrowser;
		tGroundCoverLayerGui * mLayer;
	};

	//------------------------------------------------------------------------------
	void tSigmlBrowser::fOpenDoc( const tFilePathPtr& file )
	{
		mOwner->fAddSigml( file );
	}
	

	///
	/// \class tGroundCoverPaintDensity
	/// \brief 
	class tGroundCoverPaintDensity : public tHeightFieldGroundCoverPaintCursor
	{
	public:

		tGroundCoverPaintDensity( 
			tEditorCursorControllerButton* button, 
			wxCheckBox * renderCursorGrid,
			tGroundCoverLayerGui ** selectedLayerPtr,
			f32 scale )
			: tHeightFieldGroundCoverPaintCursor( button )
			, mScale( scale )
			, mSelectedLayerPtr( selectedLayerPtr )
			, mRenderCursorGridCheckBox( renderCursorGrid )
		{
			fMainWindow( ).fSetStatus( "Paint Ground Cover Density" );
		}

	protected:

		virtual void fOnTick( )
		{
			
			mPaintLayer = *mSelectedLayerPtr;
			if( mPaintLayer )
			{
				mRadiusRange.x = fMin( 1.f, 0.25f * mPaintLayer->fUnitSize( ) );
				mRadiusRange.y = fMax( 60.f, mPaintLayer->fUnitSize( ) * mPaintLayer->fPaintUnits( ) * 3 );
			}
			fSetRenderCursorGrid( mRenderCursorGridCheckBox->GetValue( ) );
			tHeightFieldGroundCoverPaintCursor::fOnTick( );
		}

		virtual void fModifyDensityTexel( tPaintGroundCoverArgs & args )
		{
			if( args.mPaintStrength <= 0 )
				return;

			f32 val = args.mMask.fIndex( args.mX, args.mZ );
			val += args.mPaintStrength * args.mDt * mScale * 10;
			args.mMask.fIndex( args.mX, args.mZ ) = fClamp( val, 0.f, 1.f );
		}

	private:

		f32 mScale;
		tGroundCoverLayerGui ** mSelectedLayerPtr;
		wxCheckBox * mRenderCursorGridCheckBox;
		
	};

	///
	/// \class tGroundCoverPaintDensityButton
	/// \brief 
	class tGroundCoverPaintDensityButton : public tEditorCursorControllerButton
	{

	public:

		tGroundCoverPaintDensityButton( 
			tGroundCoverPanel * owner,
			tEditorCursorControllerButtonGroup* parent,
			const char * selIcon,
			const char * deselIcon,
			const char * toolTip,
			wxCheckBox * renderCursorGrid,
			tGroundCoverLayerGui ** selectedLayerPtr,
			f32 scale )	
			: tEditorCursorControllerButton( 
				parent, 
				wxBitmap( selIcon ), 
				wxBitmap( deselIcon ), 
				toolTip )
			, mOwner( owner )
			, mSelectedLayerPtr( selectedLayerPtr )
			, mRenderCursorGridCheckBox( renderCursorGrid )
			, mScale( scale )
		{
		}

		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			tGroundCoverPaintDensity * terrainCursor = 
				new tGroundCoverPaintDensity( this, mRenderCursorGridCheckBox, mSelectedLayerPtr, mScale );

			mOwner->fUpdateParametersOnCursor( terrainCursor );
			mOwner->fAddCursorHotKeys( terrainCursor );

			return tEditorCursorControllerPtr( terrainCursor );
		}

	private:

		tGroundCoverPanel * mOwner;
		tGroundCoverLayerGui ** mSelectedLayerPtr;
		wxCheckBox * mRenderCursorGridCheckBox;

		f32 mScale;
	};

	

	//------------------------------------------------------------------------------
	tGroundCoverPanel::tGroundCoverPanel( tEditorAppWindow * window, tWxToolsPanel * parent )
		: tWxToolsPanelTool( 
			parent, 
			"Ground Cover", 
			"Create and manage layers of ground cover", 
			"GroundCover" )
		, mDialogInputActive( false )
		, mRefreshDialog( true )
		, mSelectedLayer( 0 )
	{
		mPropertiesDialog = new tGroundCoverLayerDialog( window, this, parent );
		if( mWantsDialog = mPropertiesDialog->IsShown( ) )
			mPropertiesDialog->fToggle( );

		window->fAddEditorDialog( mPropertiesDialog );

		tEditorCursorControllerButtonGroup * brushGroup = new tEditorCursorControllerButtonGroup( this, "GC Brush Type", false );
		{
			mRenderCursorGridCheckBox = new wxCheckBox( brushGroup->fGetMainPanel( ), wxID_ANY, "Render Cursor Grid" );

			new tGroundCoverPaintDensityButton( 
				this, 
				brushGroup, 
				"PaintTerrainGCIncreaseSel", 
				"PaintTerrainGCIncreaseDeSel", 
				"Increase GC density",
				mRenderCursorGridCheckBox,
				&mSelectedLayer,
				-1.f );
			new tGroundCoverPaintDensityButton( 
				this, 
				brushGroup, 
				"PaintTerrainGCDecreaseSel", 
				"PaintTerrainGCDecreaseDeSel", 
				"Decrease GC density",
				mRenderCursorGridCheckBox,
				&mSelectedLayer,
				1.f );

			brushGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 10 );
			brushGroup->fGetMainPanel( )->GetSizer( )->Add( mRenderCursorGridCheckBox, 0, wxALL, 4 );
		}

		tWxSlapOnGroup * propGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Brush Properties", false );
		{
			mSizeSlider = new tGroundCoverBrushSlider( this, propGroup->fGetMainPanel( ), "Size", 0.25f );
			mStrengthSlider = new tGroundCoverBrushSlider( this, propGroup->fGetMainPanel( ), "Strength", 0.50f );
			mFalloffSlider = new tGroundCoverBrushSlider( this, propGroup->fGetMainPanel( ), "Focus", 0.25f );
			mShapeSlider = new tGroundCoverBrushSlider( this, propGroup->fGetMainPanel( ), "Shape", 0.00f );
		}

		mLayerGroup = new tWxSlapOnGroup( fGetMainPanel( ), "GC Layers", false );
		{
			wxBoxSizer * buttonSizer = new wxBoxSizer( wxHORIZONTAL );
			mLayerGroup->fGetMainPanel( )->GetSizer( )->Add( buttonSizer, 0, wxALIGN_RIGHT, 2 );

			wxStaticText* addLayerText = new wxStaticText( mLayerGroup->fGetMainPanel( ), wxID_ANY, "Add GC Layer" );
			wxButton* addLayer = new wxButton( mLayerGroup->fGetMainPanel( ), wxID_ANY, "+", wxDefaultPosition, wxSize( 20, 20 ) );
			addLayer->Connect( 
				wxEVT_COMMAND_BUTTON_CLICKED, 
				wxCommandEventHandler( tGroundCoverPanel::fOnAddLayerPressed ), 
				NULL, this );

			buttonSizer->Add( addLayerText, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2 );
			buttonSizer->AddSpacer( 2 );
			buttonSizer->Add( addLayer, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2 );
			buttonSizer->AddSpacer( 4 );
		}

		tWxSlapOnGroup * utilGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Utilities", false );
		{
			wxButton * button = new wxButton( utilGroup->fGetMainPanel( ), wxID_ANY, "Refresh Heights" );
			button->Connect( 
				wxEVT_COMMAND_BUTTON_CLICKED, 
				wxCommandEventHandler( tGroundCoverPanel::fOnRefreshHeights ), 
				NULL, this );

			utilGroup->fGetMainPanel( )->GetSizer( )->Add( button, 0, wxEXPAND | wxALIGN_CENTER, 4 );
		}

		mBaseLayerSizerIndex = 
			( u32 )mLayerGroup->fGetMainPanel( )->GetSizer( )->GetChildren( ).GetCount( );

		mOnObjectAdded.fFromMethod<tGroundCoverPanel, &tGroundCoverPanel::fOnObjectAdded>( this );
		fParent( )->fGuiApp( ).fEditableObjects( ).fGetObjectAddedEvent( ).fAddObserver( &mOnObjectAdded );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fDeleteLayer( tGroundCoverLayerGui * layer )
	{
		if( layer == mSelectedLayer )
			fSelectLayer( 0 );

		tGrowableArray< tEditableTerrainGeometry * > ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

		const u32 entCount = ents.fCount( );
		for( u32 e = 0; e < entCount; ++e )
			ents[ e ]->fRemoveGroundCover( layer->fUniqueId( ) );

		mLayers.fFindAndEraseOrdered( layer );
		fBeginBuildUI( );
		layer->Destroy( );
		fEndBuildUI( );
		mLayerGroup->fGetMainPanel( )->GetSizer( )->Layout( );
		fMarkDirty( );
	}

	//------------------------------------------------------------------------------
	b32 tGroundCoverPanel::fRenameLayer( tGroundCoverLayerGui * layer, const std::string & title )
	{
		if( !_stricmp( layer->fName( ).c_str( ), title.c_str( ) ) )
			return true;

		if( !fTitleIsAvailable( title.c_str( ), layer ) )
		{
			wxMessageBox( 
				"Specified name already exists", "Invalid GC layer name", wxOK | wxICON_WARNING | wxCENTRE );
			return false;
		}

		layer->fSetTitle( title.c_str( ), false );
		fMarkDirty( );
		return true;
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fShowProperties( tGroundCoverLayerGui * layer )
	{
		if( layer != mSelectedLayer )
			fSelectLayer( layer );

		mPropertiesDialog->Show( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fSelectLayer( tGroundCoverLayerGui * layer )
	{
		// Skip needless updates
		if( mSelectedLayer == layer )
			return;

		fBeginBuildUI( );

		mSelectedLayer = layer;

		const u32 layerCount = mLayers.fCount( );
		for( u32 i = 0; i < layerCount; ++i )
			mLayers[ i ]->fSetSelected( mLayers[ i ] == layer );

		mPropertiesDialog->fSetLayer( layer );

		fEndBuildUI( );
	}

	//------------------------------------------------------------------------------
	tGroundCoverLayerGui * tGroundCoverPanel::fFindLayer( u32 id )
	{
		const u32 layerCount = mLayers.fCount( );
		for( u32 l = 0; l < layerCount; ++l )
		{
			if( mLayers[ l ]->fUniqueId( ) == id )
				return mLayers[ l ];
		}

		return 0;
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fOnTick( )
	{
		if( mDialogInputActive )
			fGuiApp( ).fMainWindow( ).fSetDialogInputActive( );

		if( mRefreshDialog )
		{
			mPropertiesDialog->fRefresh( );
			mRefreshDialog = false;
		}
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fReset( )
	{
		fBeginBuildUI( );

		tGrowableArray< tEditableTerrainGeometry * > ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );
		const u32 entCount = ents.fCount( );

		const u32 count = mLayers.fCount( );
		for( u32 l = 0; l < count; ++l )
		{
			for( u32 e = 0; e < entCount; ++e )
				ents[ e ]->fRemoveGroundCover( mLayers[ l ]->fUniqueId( ) );

			mLayers[ l ]->Destroy( );
		}

		mLayers.fSetCount( 0 );
		fSelectLayer( 0 );
		
		mParent->fUpdateScrollBars( );
		fEndBuildUI( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fReset( const Sigml::tFile& file )
	{
		fBeginBuildUI( );

		tGrowableArray< tEditableTerrainGeometry * > ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );
		const u32 entCount = ents.fCount( );

		mSelectedLayer = 0;

		u32 count = mLayers.fCount( );
		for( u32 l = 0; l < count; ++l )
		{
			for( u32 e = 0; e < entCount; ++e )
				ents[ e ]->fRemoveGroundCover( mLayers[ l ]->fUniqueId( ) );

			mLayers[ l ]->Destroy( );
		}

		mLayers.fSetCount( 0 );

		count = file.mGroundCovers.fCount( );
		for( u32 l = 0; l < count; ++l )
		{
			mLayers.fPushBack(
				new tGroundCoverLayerGui(
					this, 
					mLayerGroup->fGetMainPanel( ), 
					file.mGroundCovers[ l ], 
					mBaseLayerSizerIndex + mLayers.fCount( ) ) );

			for( u32 e = 0; e < entCount; ++e )
				ents[ e ]->fRestoreGroundCover( *mLayers[ l ] );
		}

		for( u32 e = 0; e < entCount; ++e )
			ents[ e ]->fSetGroundCoverInitialized( );

		if( mLayers.fCount( ) && mWantsDialog )
		{
			mWantsDialog = false;
			fSelectLayer( mLayers[ 0 ] );
			if( !mPropertiesDialog->IsShown( ) )
				mPropertiesDialog->Show( true );
		}

		mParent->fUpdateScrollBars( );
		fEndBuildUI( );	
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fSave( Sigml::tFile & file )
	{
		const u32 count = mLayers.fCount( );
		file.mGroundCovers.fSetCount( count );

		for( u32 l = 0; l < count; ++l )
			file.mGroundCovers[ l ] = *mLayers[ l ];
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fMarkDirty( )
	{
		fGuiApp( ).fActionStack( ).fForceSetDirty( true );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fRestoreLayer( const Sigml::tGroundCoverLayer & layer )
	{
		tGroundCoverLayerGui * layerGui = fFindLayer( layer.fUniqueId( ) );
		sigassert( layerGui );
		layerGui->fSetLayer( layer );

		mPropertiesDialog->fUpdateLayer( layerGui );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fUpdateParametersOnCursor( tHeightFieldPaintCursor* cursor )
	{
		if( !cursor )
			return;

		cursor->fSetSize( mSizeSlider->fGetValue( ) );
		cursor->fSetStrength( mStrengthSlider->fGetValue( ) );
		cursor->fSetFalloff( mFalloffSlider->fGetValue( ) );
		cursor->fSetShape( mShapeSlider->fGetValue( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fNudgeCursorSize( f32 delta )
	{
		mSizeSlider->fSetValue( mSizeSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fNudgeCursorStrength( f32 delta )
	{
		mStrengthSlider->fSetValue( mStrengthSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fNudgeCursorFalloff( f32 delta )
	{
		mFalloffSlider->fSetValue( mFalloffSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fNudgeCursorShape( f32 delta )
	{
		mShapeSlider->fSetValue( mShapeSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fToggleCursorGrid( )
	{
		mRenderCursorGridCheckBox->SetValue( !mRenderCursorGridCheckBox->GetValue( ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fOnRefreshHeights( wxCommandEvent & )
	{
		tGroundCoverRefreshHeightsDialog * dialog = new tGroundCoverRefreshHeightsDialog( &
			fGuiApp( ).fMainWindow( ) );

		tGrowableArray< tEditableTerrainGeometry * > ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

		f32 progress = 0;
		const f32 progressStep = 1.f / ( ents.fCount( ) * mLayers.fCount( ) );

		const u32 entCount = ents.fCount( );
		for( u32 e = 0; e < entCount; ++e )
		{
			const u32 layerCount = mLayers.fCount( );
			for( u32 l = 0; l < layerCount; ++l, progress += progressStep )
			{
				std::stringstream str;
				str << "Terrain (" << e + 1 << "/" << entCount << ")" 
					<< " Layer (" << l + 1 << "/" << layerCount << ")";

				dialog->fUpdate( progress, str.str( ).c_str( ));
				ents[ e ]->fRefreshGroundCoverHeights( *mLayers[ l ] );
			}
		}

		//dialog->Show( false );
		delete dialog;

		fMarkDirty( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fOnAddLayerPressed( wxCommandEvent & )
	{
		std::string layerTitle;
		for( u32 i = 0; true; ++i )
		{
			std::stringstream str;
			str << "GC Layer " << i;

			layerTitle = str.str( );
			if( fTitleIsAvailable( layerTitle.c_str( ) ) )
				break;
		}

		fBeginBuildUI( );
		
		mLayers.fPushBack( 
			new tGroundCoverLayerGui( 
				this, 
				mLayerGroup->fGetMainPanel( ), 
				mLayers.fCount( ) ? mLayers.fBack( )->fUniqueId( ) + 1 : 0, // Increment the highest layer id
				layerTitle.c_str( ), 
				mBaseLayerSizerIndex + mLayers.fCount( ) ) );


		tGrowableArray< tEditableTerrainGeometry * > ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectAllByType( ents );

		const u32 entCount = ents.fCount( );
		for( u32 e = 0; e < entCount; ++e )
			ents[ e ]->fAddGroundCover( *mLayers.fBack( ) );

		mParent->fUpdateScrollBars( );
		fEndBuildUI( );

		fMarkDirty( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fOnObjectAdded( tEditableObjectContainer &, const tEntityPtr & e )
	{
		tEditableTerrainGeometry * terrain = e->fDynamicCast<tEditableTerrainGeometry>( );
		if( !terrain )
			return;

		// Must be a state change
		if( !terrain->fInitializeGroundCover( ) )
			return;

		const u32 gcCount = mLayers.fCount( );
		for( u32 gc = 0; gc < gcCount; ++gc )
			terrain->fAddGroundCover( *mLayers[ gc ] );

		terrain->fSetGroundCoverInitialized( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fBeginBuildUI( )
	{
		fGetMainPanel( )->GetParent( )->Freeze( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fEndBuildUI( )
	{
		fGetMainPanel( )->Layout( );
		fGetMainPanel( )->GetParent( )->Layout( );
		fGetMainPanel( )->GetParent( )->Refresh( );
		fGetMainPanel( )->GetParent( )->Thaw( );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fAddCursorHotKeys( tHeightFieldPaintCursor* cursorBase )
	{
		tGroundCoverPaintDensity* cursor = dynamic_cast< tGroundCoverPaintDensity* >( cursorBase );
		if( !cursor )
			return;

		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverRenderGridHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverSizeIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverSizeDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverStrengthIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverStrengthDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverFalloffIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverFalloffDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverShapeIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorGroundCoverShapeDecHotKey( this ) ) );
	}

	//------------------------------------------------------------------------------
	void tGroundCoverPanel::fOnSlidersChanged( )
	{
		tEditorCursorControllerPtr cursor = fGuiApp( ).fCurrentCursor( );
		fUpdateParametersOnCursor( dynamic_cast<tHeightFieldPaintCursor*>( cursor.fGetRawPtr( ) ) );
	}

	
	//------------------------------------------------------------------------------
	b32 tGroundCoverPanel::fTitleIsAvailable( 
		const char * title, tGroundCoverLayerGui * ignore )
	{
		const u32 layerCount = mLayers.fCount( );
		for( u32 l = 0; l < layerCount; ++l )
		{
			if( mLayers[ l ] == ignore) continue;
			if( !_stricmp( mLayers[ l ]->fName( ).c_str( ), title ) )
				return false;
		}

		return true;
	}
}
