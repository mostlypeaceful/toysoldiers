#include "SigEdPch.hpp"
#include "tLayerPanel.hpp"
#include "Sigml.hpp"
#include "tWxSlapOnColorPicker.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tSelectObjectsCursor.hpp"
#include "Editor/tEditorAction.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tEditableSgFileRefEntity.hpp"
#include "tEditorAppWindow.hpp"

namespace Sig
{
	namespace
	{
		static const u32 cLayerHeight = 26;
		static const u32 cLayerHeightExpanded = 156;

		enum
		{
			cActionAddSelectedToLayer = 1,
			cActionSelectObjectsInLayer,
			cActionRemoveSelectedFromLayer,
			cActionMoveLayerUp,
			cActionMoveLayerDown,
			cActionDeleteIfEmpty,
			cActionDeleteAllEmpty,
			cActionReapplyVisibility,
		};
	}

	using Sigml::tObjectLayer;

	class tObjectLayerGui : public wxPanel, public tObjectLayer
	{
	private:
		tLayerPanel* mOwner;
		b32 mDefaultLayer;
		wxTextCtrl* mTextName;
		wxButton* mButtonState;
		wxButton* mButtonColor;
		wxButton* mButtonDelete;
		wxListBox* mItemList;
		wxColour mLayerColor;
		wxSizer* mColumnSizer;
		wxSizer* mRowSizer;
		wxSizer* mVSizer;
	public:
		tObjectLayerGui( tLayerPanel* owner, wxWindow* parent, wxSizer* containingSizer, const tObjectLayer& objectLayer, b32 defaultLayer, u32 layerIndex )
			: wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.x, cLayerHeight ), wxBORDER_SIMPLE | wxFIXED_MINSIZE )
			, tObjectLayer( objectLayer )
			, mOwner( owner )
			, mDefaultLayer( defaultLayer )
			, mTextName( 0 )
			, mButtonState( 0 )
			, mButtonColor( 0 )
			, mButtonDelete( 0 )
			, mColumnSizer( containingSizer )
			, mRowSizer( 0 )
			, mVSizer( 0 )
		{
			Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tObjectLayerGui::fOnMouseRightButtonUp ) );
			Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tObjectLayerGui::fOnAction ) );
			Connect( wxEVT_LEFT_DOWN, wxCommandEventHandler( tObjectLayerGui::fOnLeftClick ) );
			Connect( wxEVT_LEFT_UP, wxCommandEventHandler( tObjectLayerGui::fOnLeftUp ) );
			Connect( wxEVT_MOTION, wxCommandEventHandler( tObjectLayerGui::fOnMouseMove ) );

			mRowSizer = new wxBoxSizer( wxHORIZONTAL );
			if( defaultLayer )
			{
				mColumnSizer->Add( mRowSizer, 0, wxEXPAND | wxTOP, 4 );
			}
			else
			{
				mColumnSizer->Insert( fMax( ((s32)layerIndex) - 1, 0 ), mRowSizer, 0, wxEXPAND | wxTOP, 4 );
			}
			fApplyBGColor( false );
			mRowSizer->Add( this, 1, 0, 2 );

			// for top row and then object list
			mVSizer = new wxBoxSizer( wxVERTICAL );
			SetSizer( mVSizer );

			// for buttons across top
			wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
			mVSizer->Add( hSizer );

			mButtonState = new wxButton( this, wxID_ANY, "V", wxDefaultPosition, wxSize( 20, 20 ) );
			mButtonState->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tObjectLayerGui::fOnStateButtonPressed ), NULL, this );

			mButtonColor = new wxButton( this, wxID_ANY, "", wxDefaultPosition, wxSize( 20, 20 ) );
			mButtonColor->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tObjectLayerGui::fOnColorButtonPressed ), NULL, this );

			wxControl* textName = 0;
			if( defaultLayer )
				textName = new wxStaticText( this, wxID_ANY, "Default" );
			else
			{
				mTextName = new wxTextCtrl( this, wxID_ANY, fName( ), wxDefaultPosition, wxSize( 180, wxDefaultSize.y ), wxBORDER_NONE | wxTE_PROCESS_ENTER );
				mTextName->SetBackgroundColour( GetBackgroundColour( ) );
				mTextName->Connect( wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler( tObjectLayerGui::fOnEnterPressed ), NULL, this );
				mTextName->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( tObjectLayerGui::fOnTextFocus ), NULL, this );
				mTextName->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( tObjectLayerGui::fOnTextLostFocus ), NULL, this );
				textName = mTextName;
			}
			textName->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tObjectLayerGui::fOnMouseRightButtonUp ), NULL, this  );

			if( !defaultLayer )
			{
				mButtonDelete = new wxButton( this, wxID_ANY, "X", wxDefaultPosition, wxSize( 20, 20 ) );
				mButtonDelete->SetForegroundColour( wxColour( 0x99, 0x00, 0x00 ) );
				mButtonDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tObjectLayerGui::fOnDeleteButtonPressed ), NULL, this );
			}

			hSizer->Add( mButtonState, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4 );
			hSizer->Add( mButtonColor, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4 );
			hSizer->Add( textName, 0, wxALIGN_LEFT | wxALIGN_TOP | wxLEFT | wxTOP, 6 );

			if( mButtonDelete )
			{
				hSizer->AddStretchSpacer( );
				hSizer->Add( mButtonDelete, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4 );
			}

			// object list
			mItemList = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString( ), wxLB_SORT );
			mItemList->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( tObjectLayerGui::fOnItemDblClicked ), NULL, this );
			mVSizer->Add( mItemList, 3, wxEXPAND | wxALL, 2 );
			
			fExpand( );

			fUpdateColorButton( );
			fUpdateStateButton( );
		}
		~tObjectLayerGui( )
		{
			mColumnSizer->Remove( mRowSizer );
		}
		void fSetSize( b32 expanded )
		{
			u32 height = expanded ? cLayerHeightExpanded : cLayerHeight;
			SetSize( wxSize( wxDefaultSize.x, height ) );
			SetMinSize( wxSize( wxDefaultSize.x, height ) );

			bool show = expanded != 0;
			mItemList->Show( show );

			if( show )
			{
				fRefreshList( );
			}
			else
			{
				mVSizer->RecalcSizes( );
			}

			mVSizer->Layout( );
		}
		void fRefreshList( )
		{
			int selection = mItemList->GetSelection( );

			mItemList->Clear( );

			if( mItemList->IsShown( ) )
			{
				tGrowableArray<tEditableObject*> objectsInLayer;
				mOwner->fGetObjects( objectsInLayer, fEffectiveName( ) );

				for( u32 i = 0; i < objectsInLayer.fCount( ); ++i )
					fInsertObject( objectsInLayer[ i ] );

				mItemList->SetSelection( selection );
			}
		}
		void fInsertObject( tEditableObject* obj )
		{
			std::string name = obj->fGetSoftName( );
			mItemList->Append( name, obj );
		}
		const std::string& fEffectiveName( ) const
		{
			static const std::string cDefaultName("");
			return mDefaultLayer ? cDefaultName : fName( );
		}
		void fMoveRow( u32 layerIndex )
		{
			mColumnSizer->Detach( mRowSizer );
			mColumnSizer->Insert( layerIndex+2, mRowSizer, 1, wxEXPAND | wxTOP, 4 );
			mColumnSizer->Layout( );
		}
		void fOnMouseRightButtonUp( wxMouseEvent& event )
		{
			event.Skip( false );
			mOwner->fSetDragging( NULL, false );

			wxMenu menu;
			const wxPoint pos = event.GetPosition();

			tEditorSelectionList& selected = mOwner->fEditableObjects( ).fGetSelectionList( );

			// add actions to menu
			menu.Append( cActionSelectObjectsInLayer, wxString("Select Objects in \"") + fName( ) + "\"" );
			if( selected.fCount( ) > 0 )
			{
				menu.AppendSeparator( );
				menu.Append( cActionAddSelectedToLayer, wxString("Add Selected to \"") + fName( ) + "\"" );
				if( !mDefaultLayer )
					menu.Append( cActionRemoveSelectedFromLayer, wxString("Removed Selected from \"") + fName( ) + "\"" );
			}
			
			// add actions based on where they are in the layers.
			const u32 foundLayer = mOwner->fFindLayerIndex( this );
			if( foundLayer >= 1 )
			{
				const b32 isFirst = foundLayer == 1;
				const b32 isLast = foundLayer == mOwner->fNumLayers( )-1;

				// add separator if something is going to be added
				if( !isFirst || !isLast )
					menu.AppendSeparator( );

				if( !isFirst ) // don't let the top layer move up
					menu.Append( cActionMoveLayerUp, wxString("Move Layer Up") );
				if( !isLast ) // don't let the last layer fall down
					menu.Append( cActionMoveLayerDown, wxString("Move Layer Down") );
			}

			menu.AppendSeparator( );
			if( foundLayer >= 1 )
				menu.Append( cActionDeleteIfEmpty, wxString("Delete If Empty") );
			menu.Append( cActionDeleteAllEmpty, wxString("Delete All Empty") );

			menu.AppendSeparator( );
			menu.Append( cActionReapplyVisibility, wxString("ReApply Visibility") );			

			PopupMenu( &menu, pos.x, pos.y );
		}
		void fOnLeftClick( wxCommandEvent& event )
		{
			fExpand( );
			if( !mDefaultLayer )
				mOwner->fSetDragging( this, false );
		}
		void fOnLeftUp( wxCommandEvent& event )
		{
			mOwner->fSetDragging( NULL, true );
		}
		void fOnMouseMove( wxCommandEvent& event )
		{
			mOwner->fSetDragOver( this );
		}
		void fExpand( )
		{
			mOwner->fBeginBuildGui( );

			fSetSize( true );
			tGrowableArray<tObjectLayerGui*> uis = mOwner->fLayers( );

			//dont delete the first layer
			for( u32 i = 0; i < uis.fCount( ); ++i )
			{
				if( uis[ i ] != this )
					uis[ i ]->fSetSize( false );
			}

			mColumnSizer->Layout( );
			mOwner->fEndBuildGui( );
		}
		void fOnAction( wxCommandEvent& event )
		{
			const std::string& layerName = fEffectiveName( );
			switch( event.GetId( ) )
			{
			case cActionAddSelectedToLayer:
				{
					tEditorSelectionList& selected = mOwner->fEditableObjects( ).fGetSelectionList( );
					for( u32 i = 0; i < selected.fCount( ); ++i )
					{
						tEditableObject* eo = selected[ i ]->fDynamicCast<tEditableObject>( );
						mOwner->fAddToLayer( eo, layerName );

						if( fState( ) == cStateFrozen )
							eo->fFreeze( );
						else if( fState( ) == cStateHidden )
							eo->fHide( );
					}
					if( fState( ) == cStateFrozen || fState( ) == cStateHidden )
						selected.fClear( );
					fRefreshList( );
					
					tEditorAppWindow* window = static_cast<tEditorAppWindow*>(&mOwner->fGuiApp( ).fMainWindow( ));
					window->fRefreshObjectProperties( );
				}
				break;
			case cActionSelectObjectsInLayer:
				{
					tGrowableArray<tEditableObject*> objectsInLayer;
					mOwner->fGetObjects( objectsInLayer, layerName );

					tEditorSelectionList& selected = mOwner->fEditableObjects( ).fGetSelectionList( );
					tEditorSelectionList savedSelection = selected;
					selected.fClear( );
					for( u32 i = 0; i < objectsInLayer.fCount( ); ++i )
					{
						if( objectsInLayer[ i ]->fState( ) == tEditableObject::cStateShown )
							selected.fAdd( tEntityPtr( objectsInLayer[ i ] ) );
					}
					mOwner->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( mOwner->fGuiApp( ).fMainWindow( ), savedSelection ) ) );
				}
				break;
			case cActionRemoveSelectedFromLayer:
				{
					if( !mDefaultLayer )
					{
						// put objects from layer back into default layer
						tGrowableArray<tEditableObject*> objects;
						mOwner->fGetObjects( objects, fEffectiveName( ) );
						for( u32 i = 0; i < objects.fCount( ); ++i )
							if( objects[ i ]->fGetSelected( ) )
								mOwner->fRemoveFromLayer( objects[ i ], fEffectiveName( ) );
						fRefreshList( );
						tEditorAppWindow* window = static_cast<tEditorAppWindow*>(&mOwner->fGuiApp( ).fMainWindow( ));
						window->fRefreshObjectProperties( );
					}
				}
				break;
			case cActionDeleteIfEmpty:
				{
					tGrowableArray<tEditableObject*> objectsInLayer;
					mOwner->fGetObjects( objectsInLayer, layerName );
					if( objectsInLayer.fCount( ) == 0 )
						fOnDeleteButtonPressed( wxCommandEvent( ) );
				}
				break;
			case cActionDeleteAllEmpty:
				{
					tGrowableArray<tObjectLayerGui*> uis = mOwner->fLayers( );

					//dont delete the first layer
					for( u32 i = 1; i < uis.fCount( ); ++i )
						uis[ i ]->fOnAction( wxCommandEvent( 0, cActionDeleteIfEmpty ) );
				}
				break;
			case cActionReapplyVisibility:
				{
					tGrowableArray<tObjectLayerGui*> uis = mOwner->fLayers( );
					for( u32 i = 0; i < uis.fCount( ); ++i )
						uis[ i ]->fApplyState( );
				}
				break;
			case cActionMoveLayerUp:
				{
					mOwner->fBumpUp( this );
				}
				break;
			case cActionMoveLayerDown:
				{
					mOwner->fBumpDown( this );
				}
				break;
			default:
				event.Skip( );
				break;
			}
		}
		void fOnItemDblClicked( wxCommandEvent& e )
		{
			if( mItemList->GetSelection( ) != -1 )
			{
				tEditableObject* ptr = static_cast<tEditableObject*>( mItemList->GetClientData( mItemList->GetSelection( ) ) );
				if( ptr )
				{
					tEditorSelectionList& selected = mOwner->fEditableObjects( ).fGetSelectionList( );
					tEditorSelectionList savedSelection = selected;

					const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );

					if( kb.fButtonHeld( Input::tKeyboard::cButtonLCtrl ) || kb.fButtonHeld( Input::tKeyboard::cButtonRCtrl ) )
					{
						// holding control removes this from the selection
						selected.fRemove( tEntityPtr( ptr ) );
					}
					else
					{
						// Holding control will add to the selection.
						if( !kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) && !kb.fButtonHeld( Input::tKeyboard::cButtonRShift ) )
							selected.fClear( );

						selected.fAdd( tEntityPtr( ptr ) );
					}

					mOwner->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( new tModifySelectionAction( mOwner->fGuiApp( ).fMainWindow( ), savedSelection ) ) );
				}
			}
		}
		void fOnStateButtonPressed( wxCommandEvent& )
		{
			fCycleState( );
			fUpdateStateButton( );
			fApplyState( );
		}

		void fApplyState( )
		{
			mOwner->fMarkDirty( );

			const std::string& layerName = fEffectiveName( );
			tGrowableArray<tEditableObject*> objectsInLayer;
			mOwner->fGetObjects( objectsInLayer, layerName );

			tEditableObject::tState eoState;
			switch( fState( ) )
			{
			case cStateVisible: eoState = tEditableObject::cStateShown; break;
			case cStateHidden: eoState = tEditableObject::cStateHidden; break;
			case cStateFrozen: eoState = tEditableObject::cStateFrozen; break;
			default: sigassert( !"invalid state in tObjectLayerGui::fOnStateButtonPressed" ); break;
			}

			tEditorSelectionList& selected = mOwner->fEditableObjects( ).fGetSelectionList( );

			tFreezeHideAction* action = new tFreezeHideAction( mOwner->fGuiApp( ).fMainWindow( ), eoState );
			for( u32 i = 0; i < objectsInLayer.fCount( ); ++i )
			{
				if( fState( ) == cStateFrozen )
				{
					objectsInLayer[ i ]->fFreeze( true );
					selected.fRemove( tEntityPtr( objectsInLayer[ i ] ) );
				}
				else if( fState( ) == cStateHidden )
				{
					objectsInLayer[ i ]->fHide( true );
					selected.fRemove( tEntityPtr( objectsInLayer[ i ] ) );
				}
				else
					objectsInLayer[ i ]->fHide( false );
			}
			action->fFinishConstruction( );
			mOwner->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( action ) );
		}
		void fOnColorButtonPressed( wxCommandEvent& )
		{
			tColorPickerData cpd( fColorRgba( ).fXYZ( ) );
			if( tWxSlapOnColorPicker::fPopUpDialog( mButtonColor, cpd ) )
			{
				fSetColorRgba( cpd.fExpandRgba( ) );
				mOwner->fMarkDirty( );

				const std::string layerName = fEffectiveName( );
				mOwner->fEditableObjects( ).fLayerColors( )[ layerName ] = fColorRgba( );
				tGrowableArray<tEditableObject*> objects;
				mOwner->fGetObjects( objects, layerName );
				for( u32 i = 0; i < objects.fCount( ); ++i )
					objects[ i ]->fUpdateStateTint( );
			}
		}
		void fOnEnterPressed( wxCommandEvent& )
		{
			if( mOwner->fRenameLayer( this, mTextName->GetValue( ).c_str( ) ) )
				mOwner->fMarkDirty( );
			else
				mTextName->SetValue( fEffectiveName( ) );
			mOwner->fClearFocus( );
		}
		void fOnTextFocus( wxFocusEvent& )
		{
			mOwner->fSetDialogInputActive( true );
		}
		void fOnTextLostFocus( wxFocusEvent& )
		{
			mOwner->fSetDialogInputActive( false );
			if( mOwner->fRenameLayer( this, mTextName->GetValue( ).c_str( ) ) )
				mOwner->fMarkDirty( );
			else
				mTextName->SetValue( fEffectiveName( ) );
		}
		void fOnDeleteButtonPressed( wxCommandEvent& )
		{
			// put objects from layer back into default layer
			tGrowableArray<tEditableObject*> objects;
			mOwner->fGetObjects( objects, fEffectiveName( ) );
			for( u32 i = 0; i < objects.fCount( ); ++i )
				mOwner->fRemoveFromLayer( objects[ i ], fEffectiveName( ) );

			mOwner->fMarkDirty( );
			mOwner->fDeleteLayer( this );
		}
		void fUpdateStateButton( )
		{
			wxColour color;
			mButtonState->SetLabel( fStateText( color ) );
			mButtonState->SetForegroundColour( color );
		}
		void fUpdateColorButton( )
		{
			mButtonColor->SetBackgroundColour( wxColour( fRound<u8>( fColorRgba( ).x * 255.f ), fRound<u8>( fColorRgba( ).y * 255.f ), fRound<u8>( fColorRgba( ).z * 255.f ) ) );
			mOwner->fEditableObjects( ).fLayerColors( )[ fEffectiveName( ) ] = fColorRgba( );
		}
		void fUpdateLayer( const tObjectLayer& layer )
		{
			fSetColorRgba( layer.fColorRgba( ) );
			fSetState( layer.fState( ) );
			fUpdateColorButton( );
			fUpdateStateButton( );
		}
		void fApplyBGColor( b32 hovered )
		{	
			wxColour color;

			if( hovered )
			{
				color = wxColour( 0, 0xff, 0 );
			}
			else
			{
				if( mDefaultLayer )
					color = wxColour( 0xcc, 0xcc, 0xaa );
				else
					color =  wxColour( 0xff, 0xff, 0xbb );
			}

			SetBackgroundColour( color );
			if( mTextName )
				mTextName->SetBackgroundColour( color );
			Refresh( );
		}
	};

	namespace 
	{
		static const char* fName( b32 forVisibility )
		{
			if( forVisibility )
				return "Visibility";
			else
				return "Layers";
		}
	}

	tLayerPanel::tLayerPanel( tWxToolsPanel* parent, b32 forVisibility )
		: tWxToolsPanelTool( parent, fName( forVisibility ), fName( forVisibility ), "Layer" )
		, mLayerGroup( 0 )
		, mBaseLayerSizerIndex( 0 )
		, mDialogInputActive( false )
		, mDragging( false )
		, mForVisibility( forVisibility )
	{
		mLayerGroup = new tWxSlapOnGroup( fGetMainPanel( ), "", false );

		wxSizer* vSizer = mLayerGroup->fGetMainPanel( )->GetSizer( );

		wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
		vSizer->Add( buttonSizer, 0, wxALIGN_RIGHT, 2 );

		wxStaticText* addLayerText = new wxStaticText( mLayerGroup->fGetMainPanel( ), wxID_ANY, "Add Layer" );
		wxButton* addLayer = new wxButton( mLayerGroup->fGetMainPanel( ), wxID_ANY, "+", wxDefaultPosition, wxSize( 20, 20 ) );
		addLayer->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tLayerPanel::fOnAddLayerPressed ), NULL, this );

		buttonSizer->Add( addLayerText, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2 );
		buttonSizer->AddSpacer( 2 );
		buttonSizer->Add( addLayer, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2 );
		buttonSizer->AddSpacer( 4 );

		if( !mForVisibility )
			fAddDefaultLayer( );
	}
	void tLayerPanel::fOnTick( )
	{
		if( mDialogInputActive )
			fGuiApp( ).fMainWindow( ).fSetDialogInputActive( );
	}
	tEditableObjectContainer& tLayerPanel::fEditableObjects( )
	{
		return fGuiApp( ).fEditableObjects( );
	}
	const tEditableObjectContainer& tLayerPanel::fEditableObjects( ) const
	{
		return fGuiApp( ).fEditableObjects( );
	}
	void tLayerPanel::fReset( b32 subReset )
	{
		if( !subReset )
			fBeginBuildGui( );

		u32 minLayerCount = 0;

		if( !mForVisibility )
		{
			// update default layer
			mLayers[ 0 ]->fUpdateLayer( tObjectLayer( "Default" ) );
			mLayers[ 0 ]->fRefreshList();
			minLayerCount = 1;
		}

		// clear layers (all but default)
		while( mLayers.fCount( ) > minLayerCount )
			fDeleteLayer( mLayers.fBack( ), true );

		if( !subReset )
			fEndBuildGui( );
	}
	void tLayerPanel::fReset( const Sigml::tFile& file )
	{
		fBeginBuildGui( );

		fReset( true );

		// add layers from file

		const Sigml::tLayerList& list = mForVisibility ? file.mPotentialVisibility : file.mLayers;

		wxSizer* vSizer = mLayerGroup->fGetMainPanel( )->GetSizer( );
		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			if( i == 0 && !mForVisibility )
			{
				// update default layer
				mLayers[ 0 ]->fUpdateLayer( list[ 0 ] );
			}
			else
				mLayers.fPushBack( new tObjectLayerGui( this, mLayerGroup->fGetMainPanel( ), vSizer, list[ i ], false, mBaseLayerSizerIndex + i ) );
		}

		mParent->fUpdateScrollBars( );

		fEndBuildGui( );
	}
	void tLayerPanel::fMergeLayers( const Sigml::tFile& file )
	{
		fBeginBuildGui( );

		const Sigml::tLayerList& list = mForVisibility ? file.mPotentialVisibility : file.mLayers;

		wxSizer* vSizer = mLayerGroup->fGetMainPanel( )->GetSizer( );
		for( u32 fileIdx = 0; fileIdx < list.fCount( ); ++fileIdx )
		{
			const tObjectLayer& thisFileLayer = list[ fileIdx ];
			b32 found = false;

			// Look to see if this layer already exists.
			for( u32 layerIdx = 1; layerIdx < mLayers.fCount( ); ++layerIdx )
			{
				if( mLayers[ layerIdx ]->fEffectiveName( ) == thisFileLayer.fName( ) )
				{
					found = true;
					break;
				}
			}

			// Add unique layers.
			if( !found )
				mLayers.fPushBack( new tObjectLayerGui( this, mLayerGroup->fGetMainPanel( ), vSizer, list[ fileIdx ], false, mLayers.fCount( ) ) );
		}

		mParent->fUpdateScrollBars( );

		fEndBuildGui( );
	}
	void tLayerPanel::fSaveLayers( Sigml::tFile& file, const tEditorSelectionList* selected ) const
	{
		Sigml::tLayerList& list = mForVisibility ? file.mPotentialVisibility : file.mLayers;

		if( selected )
		{
			for( u32 l = 0; l < mLayers.fCount( ); ++l )
			{
				tObjectLayer& layer = static_cast<tObjectLayer&>( *mLayers[ l ] );

				tGrowableArray<tEditableObject*> objectsInLayer;
				fGetObjects( objectsInLayer, layer.fName( ) );

				if( objectsInLayer.fCount( ) )
					list.fPushBack( layer );
			}
		}
		else
		{
			list.fSetCount( mLayers.fCount( ) );
			for( u32 i = 0; i < mLayers.fCount( ); ++i )
				list[ i ] = static_cast<tObjectLayer&>( *mLayers[ i ] );
		}
	}
	void tLayerPanel::fDeleteLayer( tObjectLayerGui* layer, b32 subDelete )
	{
		u32 layerIdx = fFindLayerIndex(layer);
		mLayers.fFindAndEraseOrdered( layer );
		if( !subDelete )
			fBeginBuildGui( );
		layer->Destroy( );
		if( !subDelete )
			fEndBuildGui( );

		if (layerIdx >0)
		{
			mLayers[layerIdx-1]->fExpand();
		}
	}
	b32 tLayerPanel::fRenameLayer( tObjectLayerGui* layer, const std::string& name )
	{
		const std::string prevName = layer->fName( );
		if( prevName == name )
			return true; // name is the same

		// verify the name is legit
		u32 layerIndex = mLayers.fFind( layer ) - mLayers.fBegin( );
		if( fIsNameInUse( name.c_str( ), layerIndex ) )
		{
			wxMessageBox( "Specified layer name already exists", "Invalid layer name", wxOK | wxICON_WARNING | wxCENTRE );
			return false;
		}

		tGrowableArray<tEditableObject*> objects;
		fGetObjects( objects, prevName );

		for( u32 i = 0; i < objects.fCount( ); ++i )
			fRemoveFromLayer( objects[ i ], prevName );

		layer->fSetName( name );

		// update objects from layer with new name
		for( u32 i = 0; i < objects.fCount( ); ++i )
			fAddToLayer( objects[ i ], name );

		return true;
	}
	void tLayerPanel::fClearFocus( )
	{
		if( !mForVisibility )
			mLayers[ 0 ]->SetFocus( );
	}
	void tLayerPanel::fMarkDirty( )
	{
		fGuiApp( ).fActionStack( ).fForceSetDirty( true );
	}
	void tLayerPanel::fAddDefaultLayer( )
	{
		wxSizer* vSizer = mLayerGroup->fGetMainPanel( )->GetSizer( );
		mLayers.fPushBack( new tObjectLayerGui( this, mLayerGroup->fGetMainPanel( ), vSizer, tObjectLayer( "Default" ), true, mBaseLayerSizerIndex ) );

		wxSizerItemList& rows = vSizer->GetChildren( );
		mBaseLayerSizerIndex = ( u32 )rows.GetCount( );
	}
	void tLayerPanel::fBeginBuildGui( )
	{
		fGetMainPanel( )->GetParent( )->Freeze( );
	}
	void tLayerPanel::fEndBuildGui( )
	{
		fGetMainPanel( )->Layout( );
		fGetMainPanel( )->GetParent( )->Layout( );
		fGetMainPanel( )->GetParent( )->Refresh( );
		fGetMainPanel( )->GetParent( )->Thaw( );
	}
	void tLayerPanel::fOnAddLayerPressed( wxCommandEvent& )
	{
		fAddLayer( );
	}
	void tLayerPanel::fAddLayer( )
	{
		fBeginBuildGui( );

		wxSizer* vSizer = mLayerGroup->fGetMainPanel( )->GetSizer( );

		std::string layerName;
		for( u32 layerIdx = 0; true; ++layerIdx )
		{
			std::stringstream ss;
			ss << "Layer" << /*std::setfill( '0' ) << std::setw( 3 ) << */ layerIdx;
			layerName = ss.str( );

			if( !fIsNameInUse( layerName.c_str( ) ) )
				break;
		}

		mLayers.fPushBack( new tObjectLayerGui( this, mLayerGroup->fGetMainPanel( ), vSizer, tObjectLayer( layerName ), false, mBaseLayerSizerIndex + mLayers.fCount( ) ) );

		mParent->fUpdateScrollBars( );

		fEndBuildGui( );

		fMarkDirty( );
	}
	void tLayerPanel::fBumpUp( tObjectLayerGui* layer )
	{
		u32 foundLayer = fFindLayerIndex( layer );

		if( foundLayer == 1 || foundLayer >= mLayers.fCount( ) )
			return;

		mLayers.fEraseOrdered( foundLayer );
		layer->fMoveRow( --foundLayer );
		mLayers.fInsert( foundLayer, layer );

		for( u32 i = 0; i < mLayers.fCount( ); ++i )
			mLayers[ i ]->Refresh( );
	}
	void tLayerPanel::fBumpDown( tObjectLayerGui* layer )
	{
		u32 foundLayer = fFindLayerIndex( layer );

		if( foundLayer == 0 || foundLayer >= mLayers.fCount( )-1 )
			return;

		mLayers.fEraseOrdered( foundLayer );
		layer->fMoveRow( ++foundLayer );
		mLayers.fInsert( foundLayer, layer );

		for( u32 i = 0; i < mLayers.fCount( ); ++i )
			mLayers[ i ]->Refresh( );
	}
	u32 tLayerPanel::fFindLayerIndex( tObjectLayerGui* layer ) const
	{
		u32 foundLayer = 0;
		for( ; foundLayer < mLayers.fCount( ); ++foundLayer )
			if( mLayers[ foundLayer ] == layer )
				break;

		return foundLayer;
	}
	b32 tLayerPanel::fIsNameInUse( const char* name, u32 ignoreIndex ) const
	{
		for( u32 i = 0; i < mLayers.fCount( ); ++i )
		{
			if( i == ignoreIndex ) continue;
			if( _stricmp( name, mLayers[ i ]->fName( ).c_str( ) ) == 0 )
				return true;
		}
		return false;
	}
	void tLayerPanel::fSetDragging( tObjectLayerGui* obj, b32 drop )
	{
		if( obj )
		{
		}
		else if( drop )
		{
			if( mDragging && mDragOver && mDragging != mDragOver )
			{
				u32 destination = mLayers.fIndexOf( mDragOver );

				if( destination != 0 ) //dont switch places to default
				{
					fBeginBuildGui( );

					mLayers.fFindAndEraseOrdered( mDragging );

					mDragging->fMoveRow( destination );
					mLayers.fInsert( destination, mDragging );

					for( u32 i = 0; i < mLayers.fCount( ); ++i )
						mLayers[ i ]->Refresh( );

					fEndBuildGui( );
				}
			}
		}

		mDragOver = NULL;
		mDragging = obj;
		for( u32 i = 0; i < mLayers.fCount( ); ++i )
			mLayers[ i ]->fApplyBGColor( false );
	}
	void tLayerPanel::fSetDragOver( tObjectLayerGui* obj )
	{
		if( mDragging )
		{
			mDragOver = obj;
			for( u32 i = 1; i < mLayers.fCount( ); ++i )
			{
				b32 dragging = (mDragging == mLayers[ i ]);
				mLayers[ i ]->fApplyBGColor( !dragging && mLayers[ i ] == obj );
			}
		}
	}
	void tLayerPanel::fRefreshLists( )
	{
		for( u32 i = 0; i < mLayers.fCount( ); ++i )
			mLayers[ i ]->fRefreshList( );
	}


	void tLayerPanel::fAddToLayer( tEditableObject* eo, const std::string& name )
	{
		if( mForVisibility )
		{
			eo->fVisibilitySets( ).mSet.fFindOrAdd( name );
		}
		else
		{
			eo->fSetLayer( name );
		}
	}

	void tLayerPanel::fGetObjects( tGrowableArray<tEditableObject*>& objOut, const std::string& name ) const
	{
		if( mForVisibility )
		{
			fEditableObjects( ).fGetVisibilitySet( objOut, name );
		}
		else
		{
			fEditableObjects( ).fGetLayer( objOut, name );
		}
	}

	void tLayerPanel::fRemoveFromLayer( tEditableObject* eo, const std::string& name )
	{
		if( mForVisibility )
		{
			eo->fVisibilitySets( ).mSet.fFindAndEraseOrdered( name );
		}
		else
		{
			eo->fSetLayer( "" );
		}
	}
}


