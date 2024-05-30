#include "SigFxPch.hpp"
#include "tCreateNewEffectPanel.hpp"
#include "tPlaceObjectCursor.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "FxEditor/tSigFxAttractor.hpp"
#include "FxEditor/tSigFxMeshSystem.hpp"
#include "FxEditor/tSigFxLight.hpp"
#include "tWxSlapOnChoice.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tSelectObjectsCursor.hpp"
#include "tSigFxMainWindow.hpp"
#include "tSigFxMatEd.hpp"

namespace Sig
{
	class tNewParticleSystemObjectCursor : public tPlaceObjectCursor
	{
	public:
		tNewParticleSystemObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( new tSigFxParticleSystem( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			tSigFxMainWindow* mainWindow = dynamic_cast< tSigFxMainWindow* >( &fMainWindow( ) );
			sigassert( mainWindow );

			mainWindow->fSigFxMatEd( )->fSetDefaultShader( mEntityMaster );
			mainWindow->fSetStatus( "New Effect [Particle System]" );
		}

		virtual void fOnEntityPlaced( const tEntityPtr& placedEntity )
		{
			mEntityMaster->fDeleteImmediate( );
			mEntityMaster.fReset( new tSigFxParticleSystem( mMainWindow.fGuiApp( ).fEditableObjects( ) ) );
			tSigFxMainWindow* mainWindow = dynamic_cast< tSigFxMainWindow* >( &fMainWindow( ) );
			sigassert( mainWindow );
			mainWindow->fSigFxMatEd( )->fSetDefaultShader( mEntityMaster );
		}

		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			fGetButton( )->fGetParent( )->fClearSelection( );
			tPlaceObjectCursor::fOnNextCursor( nextController );
		}
	};
	class tNewParticleSystemObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tNewParticleSystemObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "ParticleSystemPlaceSel" ), wxBitmap( "ParticleSystemPlace" ), "New Particle System" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tNewParticleSystemObjectCursor( this ) );
		}
	};


	class tNewAttractorObjectCursor : public tPlaceObjectCursor
	{
		tSigFxAttractor* mAttractor;
	public:
		tNewAttractorObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( mAttractor = new tSigFxAttractor( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "New Attractor" );
		}
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			fGetButton( )->fGetParent( )->fClearSelection( );
			tPlaceObjectCursor::fOnNextCursor( nextController );
		}
	};
	class tNewAttractorObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tNewAttractorObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "AttractorPlaceSel" ), wxBitmap( "AttractorPlace" ), "New Attractor" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tNewAttractorObjectCursor( this ) );
		}
	};


	class tNewLightObjectCursor : public tPlaceObjectCursor
	{
		tSigFxLight* mLight;
	public:
		tNewLightObjectCursor( tEditorCursorControllerButton* button )
			: tPlaceObjectCursor( button, tEntityPtr( mLight = new tSigFxLight( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
		{
			fMainWindow( ).fSetStatus( "New Light" );
		}
		virtual void fOnNextCursor( tEditorCursorController* nextController )
		{
			fGetButton( )->fGetParent( )->fClearSelection( );
			tPlaceObjectCursor::fOnNextCursor( nextController );
		}
	};
	class tNewLightObjectCursorButton : public tEditorCursorControllerButton
	{
	public:
		tNewLightObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
			: tEditorCursorControllerButton( parent, wxBitmap( "AttractorPlaceSel" ), wxBitmap( "AttractorPlace" ), "New Light" )
		{
		}
		virtual tEditorCursorControllerPtr fCreateCursorController( )
		{
			return tEditorCursorControllerPtr( new tNewLightObjectCursor( this ) );
		}
	};

	//class tNewFxMeshSystemObjectCursor : public tPlaceObjectCursor
	//{
	//	tSigFxMeshSystem* mMeshSystem;
	//public:
	//	tNewFxMeshSystemObjectCursor( tEditorCursorControllerButton* button )
	//		: tPlaceObjectCursor( button, tEntityPtr( mMeshSystem = new tSigFxMeshSystem( button->fGetParent( )->fMainWindow( ).fGuiApp( ).fEditableObjects( ) ) ) )
	//	{
	//		fMainWindow( ).fSetStatus( "New Mesh System" );
	//	}
	//	virtual void fOnEntityPlaced( const tEntityPtr& placedEntity )
	//	{
	//		tPlaceObjectCursor::fOnEntityPlaced( placedEntity );
	//	}
	//	virtual void fOnNextCursor( tEditorCursorController* nextController )
	//	{
	//		fGetButton( )->fGetParent( )->fClearSelection( );
	//		tPlaceObjectCursor::fOnNextCursor( nextController );
	//	}
	//};
	//class tNewMeshSystemObjectCursorButton : public tEditorCursorControllerButton
	//{
	//public:
	//	tNewMeshSystemObjectCursorButton( tEditorCursorControllerButtonGroup* parent )
	//		: tEditorCursorControllerButton( parent, wxBitmap( "MeshSystemPlaceSel" ), wxBitmap( "MeshSystemPlace" ), "New Mesh System" )
	//	{
	//	}
	//	virtual tEditorCursorControllerPtr fCreateCursorController( )
	//	{
	//		return tEditorCursorControllerPtr( new tNewFxMeshSystemObjectCursor( this ) );
	//	}
	//};



	class tPlacedEffectsListBox :  public tWxSlapOnControl
	{
		wxCheckListBox* mListBox;
		tSigFxMainWindow& mMainWindow;
		b32 mAllowSelectionRefresh;

	public:

		tPlacedEffectsListBox( wxWindow* parent, const char* label, tEditableObjectContainer& list, tSigFxMainWindow& window )
			: tWxSlapOnControl( parent, label )
			, mListBox( 0 )
			, mMainWindow( window )
			, mAllowSelectionRefresh( true )
		{
			mListBox = new wxCheckListBox( parent, wxID_ANY, wxDefaultPosition, wxSize( fControlWidth( ), wxDefaultSize.y ), 0, wxLB_EXTENDED | wxLB_NEEDED_SB );
			fAddWindowToSizer( mListBox, true );
			fRefresh( );
			mListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( tPlacedEffectsListBox::fOnControlUpdated ), NULL, this );
			mListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( tPlacedEffectsListBox::fOnChecksUpdated ), NULL, this );
			mListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( tPlacedEffectsListBox::fOnListBoxDoubleClicked ), NULL, this );
		}

		virtual void fEnableControl( )
		{
			mListBox->Enable( );
		}
		virtual void fDisableControl( )
		{
			mListBox->Disable( );
		}

		void fShowHideAll( b32 hideAll )
		{
			for( u32 i = 0; i < mListBox->GetCount( ); ++i )
			{
				mListBox->Check( i, hideAll == true );
			}

			fOnChecksUpdated( wxCommandEvent( ) );
		}

		void fOnChecksUpdated( wxCommandEvent& event )
		{
			tGrowableArray< tEditableObject* > list;
			mMainWindow.fGuiApp( ).fEditableObjects( ).fCollectAllByType< tEditableObject >( list );
			//mMainWindow.fGuiApp( ).fEditableObjects( ).fUnhideAll( );

			for( u32 i = 0; i < mListBox->GetCount( ); ++i )
			{
				wxString& string = mListBox->GetString( i );
				b32 checked = mListBox->IsChecked( i );
				for( u32 j = 0; j < list.fCount( ); ++j )
				{
					tSigFxParticleSystem* system = list[ j ]->fDynamicCast< tSigFxParticleSystem >( );
					tSigFxAttractor* attractor = list[ j ]->fDynamicCast< tSigFxAttractor >( );
					tSigFxLight* light = list[ i ]->fDynamicCast< tSigFxLight >( );

					if( checked )
					{
						if( system && !strcmp( system->fParticleSystemName( ).fCStr( ), string.c_str( ) ) )
							system->fGetParticleSystem( )->fSetDisabled( true );
						if( attractor && !strcmp( attractor->fAttractorName( ).fCStr( ), string.c_str( ) ) )
							attractor->fSetHidden( true );
						if( light && !strcmp( light->fGetName( ).c_str( ), string.c_str( ) ) )
							light->fHide( true );
					}
					else
					{
						if( system && !strcmp( system->fParticleSystemName( ).fCStr( ), string.c_str( ) ) )
							system->fGetParticleSystem( )->fSetDisabled( false );
						if( attractor && !strcmp( attractor->fAttractorName( ).fCStr( ), string.c_str( ) ) )
							attractor->fSetHidden( false );
						if( light && !strcmp( light->fGetName( ).c_str( ), string.c_str( ) ) )
							light->fHide( false );
					}
				}
			}
		}

		class tRenameWindow : public wxDialog
		{
			wxTextCtrl* mText;
			tSigFxMainWindow* mSigFxMainWindow;

		public:
			tRenameWindow( tSigFxMainWindow* sigFxWindow, const tStringPtr& text )
				: wxDialog( 0, wxID_ANY, "Rename", wxDefaultPosition, wxSize( 333, 90 ), wxDEFAULT_DIALOG_STYLE )
				, mSigFxMainWindow( sigFxWindow )
			{
				mText = new wxTextCtrl( this, wxID_ANY, "nothing", wxDefaultPosition, wxDefaultSize );
				mText->SetValue( wxString( text.fCStr( ) ) );
				
				wxBoxSizer *sizer = new wxBoxSizer( wxVERTICAL );
				wxSizer* buttonSizer = CreateButtonSizer( wxOK | wxCANCEL );

				sizer->Add( mText, 1, wxEXPAND | wxALL, 4 );
				sizer->Add( buttonSizer, 1, wxEXPAND | wxALL, 4 );
				
				SetSizer( sizer );

				Centre( );
			}

			tStringPtr fText( )
			{
				return tStringPtr( mText->GetValue( ).c_str( ) );
			}
		};

		void fOnListBoxDoubleClicked( wxCommandEvent& event )
		{
			tGrowableArray< tEditableObject* > list;
			mMainWindow.fGuiApp( ).fEditableObjects( ).fCollectAllByType< tEditableObject >( list );

			for( u32 i = 0; i < mListBox->GetCount( ); ++i )
			{
				if( mListBox->IsSelected( i ) )
				{
					wxString& string = mListBox->GetString( i );
					tEditableObject* obj = 0;
					for( u32 j = 0; j < list.fCount( ); ++j )
					{
						tSigFxParticleSystem* system = list[ j ]->fDynamicCast< tSigFxParticleSystem >( );
						tSigFxAttractor* attractor = list[ j ]->fDynamicCast< tSigFxAttractor >( );
						tSigFxMeshSystem* meshSystem = list[ j ]->fDynamicCast< tSigFxMeshSystem >( );
						tSigFxLight* light = list[ j ]->fDynamicCast< tSigFxLight >( );
						if( system && !strcmp( system->fParticleSystemName( ).fCStr( ), string.c_str( ) ) )
							obj = system;
						else if( attractor && !strcmp( attractor->fAttractorName( ).fCStr( ), string.c_str( ) ) )
							obj = attractor;
						else if( meshSystem && !strcmp( meshSystem->fFxMeshSystemName( ).fCStr( ), string.c_str( ) ) )
							obj = meshSystem;
						else if( light && !strcmp( light->fGetName( ).c_str( ), string.c_str( ) ) )
							obj = light;
					}

					if( obj )
					{
						tSigFxParticleSystem* system = obj->fDynamicCast< tSigFxParticleSystem >( );
						tSigFxAttractor* attractor = obj->fDynamicCast< tSigFxAttractor >( );
						tSigFxMeshSystem* meshSystem = obj->fDynamicCast< tSigFxMeshSystem >( );
						tSigFxLight* light = obj->fDynamicCast< tSigFxLight >( );
						tStringPtr text( "" );

						if( system )
							text = system->fParticleSystemName( );
						else if( attractor )
							text = attractor->fAttractorName( );
						else if( meshSystem )
							text = meshSystem->fFxMeshSystemName( );
						else if( light )
							text = tStringPtr( light->fGetName( ) );

						tRenameWindow* renamer = new tRenameWindow( &mMainWindow, text );
						if( renamer->ShowModal( ) == wxID_OK )
						{
							if( system )
								system->fSetParticleSystemName( renamer->fText( ) );
							else if( attractor )
								attractor->fSetAttractorName( renamer->fText( ) );
							else if( meshSystem )
								meshSystem->fSetMeshSystemName( renamer->fText( ) );
							else if( light )
								light->fSetEditableProperty( std::string( renamer->fText( ).fCStr( ) ), Sigml::tObject::fEditablePropObjectName( ) );

							mMainWindow.fGuiApp( ).fActionStack( ).fForceSetDirty( true );
						}
					}
				}
			}
			fRefresh( );
		}

		void fOnControlUpdated( wxCommandEvent& event )
		{
			mAllowSelectionRefresh = false;

			tGrowableArray< tEditableObject* > list;
			mMainWindow.fGuiApp( ).fEditableObjects( ).fCollectAllByType< tEditableObject >( list );

			// first deselect all
			mMainWindow.fGuiApp( ).fEditableObjects( ).fGetSelectionList( ).fClear( );
			
			for( u32 i = 0; i < mListBox->GetCount( ); ++i )
			{
				wxString& string = mListBox->GetString( i );

				b32 checked = mListBox->IsChecked( i );
				b32 selected = mListBox->IsSelected( i );

				if( selected )
				{
					for( u32 j = 0; j < list.fCount( ); ++j )
					{
						mAllowSelectionRefresh = false;

						tSigFxParticleSystem* system = list[ j ]->fDynamicCast< tSigFxParticleSystem >( );
						tSigFxAttractor* attractor = list[ j ]->fDynamicCast< tSigFxAttractor >( );
						tSigFxMeshSystem* meshSystem = list[ j ]->fDynamicCast< tSigFxMeshSystem >( );
						tSigFxLight* light = list[ j ]->fDynamicCast< tSigFxLight >( );

						if( system && !strcmp( system->fParticleSystemName( ).fCStr( ), string.c_str( ) ) )
							mMainWindow.fGuiApp( ).fEditableObjects( ).fGetSelectionList( ).fAdd( tEntityPtr( system ) );
						else if( attractor && !strcmp( attractor->fAttractorName( ).fCStr( ), string.c_str( ) ) )
							mMainWindow.fGuiApp( ).fEditableObjects( ).fGetSelectionList( ).fAdd( tEntityPtr( attractor ) );
						else if( meshSystem && !strcmp( meshSystem->fFxMeshSystemName( ).fCStr( ), string.c_str( ) ) )
							mMainWindow.fGuiApp( ).fEditableObjects( ).fGetSelectionList( ).fAdd( tEntityPtr( meshSystem ) );
						else if( light && !strcmp( light->fGetName( ).c_str( ), string.c_str( ) ) )
							mMainWindow.fGuiApp( ).fEditableObjects( ).fGetSelectionList( ).fAdd( tEntityPtr( light ) );
					}
				}
				
			}
		}

		void fUpdateSelectedList( tEditorSelectionList& list )
		{
			if( !mAllowSelectionRefresh )
			{
				mAllowSelectionRefresh = true;
				return;
			}
			for( u32 i = 0; i < mListBox->GetCount( ); ++i )
			{
				wxString& string = mListBox->GetString( i );
				mListBox->Deselect( i );

				for( u32 j = 0; j < list.fCount( ); ++j )
				{
					tSigFxParticleSystem* system = list[ j ]->fDynamicCast< tSigFxParticleSystem >( );
					tSigFxAttractor* attractor = list[ j ]->fDynamicCast< tSigFxAttractor >( );
					tSigFxMeshSystem* meshSystem = list[ j ]->fDynamicCast< tSigFxMeshSystem >( );
					tSigFxLight* light = list[ j ]->fDynamicCast< tSigFxLight >( );

					if( system && !strcmp( system->fParticleSystemName( ).fCStr( ), string.c_str( ) ) )
						mListBox->Select( i );
					else if( attractor && !strcmp( attractor->fAttractorName( ).fCStr( ), string.c_str( ) ) )
						mListBox->Select( i );
					else if( meshSystem && !strcmp( meshSystem->fFxMeshSystemName( ).fCStr( ), string.c_str( ) ) )
						mListBox->Select( i );			
					else if( light && !strcmp( light->fGetName( ).c_str( ), string.c_str( ) ) )
						mListBox->Select( i );
				}
			}

		}

		void fRefresh( )
		{
			mListBox->Clear( );

			tGrowableArray< tEditableObject* > list;
			mMainWindow.fGuiApp( ).fEditableObjects( ).fCollectAllByType< tEditableObject >( list );

			u32 index( 0 );
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				tSigFxParticleSystem* system = list[ i ]->fDynamicCast< tSigFxParticleSystem >( );
				tSigFxAttractor* attractor = list[ i ]->fDynamicCast< tSigFxAttractor >( );
				tSigFxMeshSystem* meshSystem = list[ i ]->fDynamicCast< tSigFxMeshSystem >( );
				tSigFxLight* light = list[ i ]->fDynamicCast< tSigFxLight >( );

				if( system )
				{
					mListBox->Insert( system->fParticleSystemName( ).fCStr( ), index++ );
					wxOwnerDrawn* od = mListBox->GetItem( index - 1 );
					od->SetTextColour( wxColour( 156, 60, 26 ) );
				}
				else if( attractor )
				{			
					mListBox->Insert( attractor->fAttractorName( ).fCStr( ), index++ );
					wxOwnerDrawn* od = mListBox->GetItem( index - 1 );
					od->SetTextColour( wxColour( 26, 122, 156 ) );
				}
				else if ( meshSystem )
				{
					mListBox->Insert( meshSystem->fFxMeshSystemName( ).fCStr( ), index++ );
					wxOwnerDrawn* od = mListBox->GetItem( index - 1 );
					od->SetTextColour( wxColour( 94, 120, 20 ) );
				}
				else if( light )
				{
					mListBox->Insert( light->fGetName( ).c_str( ), index++ );
					wxOwnerDrawn* od = mListBox->GetItem( index - 1 );
					od->SetTextColour( wxColour( 94, 120, 20 ) );
				}

				if( list[ i ]->fGetSelected( ) )
					mListBox->Select( index-1 );
			}

			for( u32 i = 0; i < mListBox->GetCount( ); ++i )
			{
				wxString& string = mListBox->GetString( i );

				for( u32 j = 0; j < list.fCount( ); ++j )
				{
					tSigFxParticleSystem* system = list[ j ]->fDynamicCast< tSigFxParticleSystem >( );
					tSigFxAttractor* attractor = list[ j ]->fDynamicCast< tSigFxAttractor >( );
					tSigFxMeshSystem* meshSystem = list[ j ]->fDynamicCast< tSigFxMeshSystem >( );
					tSigFxLight* light = list[ j ]->fDynamicCast< tSigFxLight >( );

					if( system && !strcmp( system->fParticleSystemName( ).fCStr( ), string.c_str( ) ) && system->fGetParticleSystem( )->fDisabled( ) )
						mListBox->Check( i );
					else if( attractor && !strcmp( attractor->fAttractorName( ).fCStr( ), string.c_str( ) ) && attractor->fHidden( ) )
						mListBox->Check( i );
					else if( meshSystem && !strcmp( meshSystem->fFxMeshSystemName( ).fCStr( ), string.c_str( ) ) && meshSystem->fHidden( ) )
						mListBox->Check( i );
					else if( light && !strcmp( light->fGetName( ).c_str( ), string.c_str( ) ) && light->fIsHidden( ) )
						mListBox->Check( i );
				}
			}

			// Yes that's right we need to go up 3 levels and then refresh the layout to keep the 'Placed Effects' window to size as new effects are added...
			mListBox->GetParent( )->GetParent( )->GetParent( )->Layout( );
			mListBox->GetParent( )->GetParent( )->GetParent( )->Refresh( );
			mListBox->GetParent( )->GetParent( )->GetParent( )->Update( );
		}
	};


	tCreateNewEffectPanel::tCreateNewEffectPanel( tWxToolsPanel* parent, tEditableObjectContainer& list, tSigFxMainWindow& mainWindow )
		: tWxToolsPanelTool( parent, "New Effects", "Create Effect", "PlaceObj" )
	{
		tEditorCursorControllerButtonGroup* buttonGroup = new tEditorCursorControllerButtonGroup( this, "Place New Effect...", false );

		new tNewParticleSystemObjectCursorButton( buttonGroup );
		new tNewAttractorObjectCursorButton( buttonGroup );
		new tNewLightObjectCursorButton( buttonGroup );

		wxBoxSizer* container = new wxBoxSizer( wxVERTICAL );
		wxPanel* group = new wxPanel( fGetMainPanel( ), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
		mEffectsChoice = new tPlacedEffectsListBox( group, "Placed Effects:  ", list, mainWindow );
		container->Add( group, 0, wxALIGN_LEFT, 4 );

		wxCheckBox* checkAll = new wxCheckBox( fGetMainPanel( ), wxID_ANY, "Show/Hide All", wxDefaultPosition, wxDefaultSize );
		checkAll->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( tCreateNewEffectPanel::fOnShowHideAll ), NULL, this );
		container->Add( checkAll, 0, wxALIGN_RIGHT | wxTOP, 4 );

		container->AddSpacer( 8 );
		fGetMainPanel( )->GetSizer( )->Add( container, 0, wxALIGN_CENTER, 0 );
		buttonGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );
	}

	void tCreateNewEffectPanel::fOnShowHideAll( wxCommandEvent& event )
	{
		if( mEffectsChoice )
			mEffectsChoice->fShowHideAll( event.IsChecked( ) );
	}

	void tCreateNewEffectPanel::fRefresh( )
	{
		if( mEffectsChoice )
			mEffectsChoice->fRefresh( );
	}

	void tCreateNewEffectPanel::fUpdateSelectedList( tEditorSelectionList& list )
	{
		if( mEffectsChoice )
			mEffectsChoice->fUpdateSelectedList( list );
	}
}


