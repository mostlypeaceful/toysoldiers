#include "SigFxPch.hpp"
#include "tAttractorPropertiesPanel.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "tWxSlapOnChoice.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{
	using namespace FX;


	class tAttractorTypeChoice  : public tWxSlapOnChoice
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxAttractor* >* mList;
	public:
		tAttractorTypeChoice( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice = ~0 )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnChoice( parent, label, enumNames, numEnumNames, defChoice )
			, mList( 0 )	{	}

		void fSetList( tGrowableArray< tSigFxAttractor* >* list ) { mList = list; }

	protected:

		virtual void fOnControlUpdated( )
		{
			tForceType type = ( tForceType )fGetValue( );

			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetAttractor( )->fSetForceType( type );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};

	class tMustBeInRadiusCheckBox : public tWxSlapOnCheckBox
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxAttractor* >* mList;
	public:
		tMustBeInRadiusCheckBox( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnCheckBox( parent, label )
			, mList( 0 )	{	}

		virtual void fOnControlUpdated( )
		{
			if( mList)
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetAttractor( )->fParticleMustBeInRadius( fGetValue( ) );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxAttractor* >* list ) { mList = list; }
	};


	class tAffectsParticleDirectionCheckBox : public tWxSlapOnCheckBox
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxAttractor* >* mList;
	public:
		tAffectsParticleDirectionCheckBox( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnCheckBox( parent, label )
			, mList( 0 )	{	}

		virtual void fOnControlUpdated( )
		{
			if( mList)
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetAttractor( )->fSetAffectParticlesDirection( fGetValue( ) );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxAttractor* >* list ) { mList = list; }
	};



	class tAttractorFlagsCheckListBox :  public tWxSlapOnControl
	{
		tSigFxMainWindow* mSigFxMainWindow;
		wxCheckListBox* mCheckListBox;
		tGrowableArray< tSigFxAttractor* >* mList;
	public:

		tAttractorFlagsCheckListBox( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnControl( parent, label )
			, mCheckListBox( 0 )
		{
			mCheckListBox = new wxCheckListBox( parent, wxID_ANY, wxDefaultPosition, wxSize( fControlWidth( ), wxDefaultSize.y ), 0, wxLB_EXTENDED | wxLB_NEEDED_SB );
			fAddWindowToSizer( mCheckListBox, true );
			mCheckListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( tAttractorFlagsCheckListBox::fOnControlUpdated ), NULL, this );

			for( u32 i = 0; i < FX::cAttractorFlagCount; ++i )
				mCheckListBox->Insert( tToolAttractorData::mAttractorFlagNames[ i ].fCStr( ), i );
		}

		virtual void fEnableControl( )
		{
			mCheckListBox->Enable( );
		}

		virtual void fDisableControl( )
		{
			mCheckListBox->Disable( );
		}

		void fOnControlUpdated( wxCommandEvent& event )
		{
			for( u32 i = 0; i < mCheckListBox->GetCount( ); ++i )
			{
				for( u32 j = 0; j < mList->fCount( ); ++j )
				{
					( *mList )[ j ]->fGetAttractor( )->fRemoveFlag( ( 1 << i ) );
					if( mCheckListBox->IsChecked( i ) )
						( *mList )[ j ]->fGetAttractor( )->fAddFlag( ( 1 << i ) );
				}					
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxAttractor* >* list )
		{
			mList = list;
			fRefresh( );
		}

		void fRefresh( )
		{
			for( u32 i = 0; i < mCheckListBox->GetCount( ); ++i )
				mCheckListBox->Check( i, false );

			for( u32 i = 0; i < mCheckListBox->GetCount( ); ++i )
			{
				for( u32 j = 0; j < mList->fCount( ); ++j )
				{
					if( ( *mList )[ j ]->fGetAttractor( )->fHasFlag( ( 1 << i ) ) )
						mCheckListBox->Check( i );
				}
			}
		}
	};


	tAttractorPropertiesPanel::tAttractorPropertiesPanel( tSigFxMainWindow* fxWindow, tWxToolsPanel* parent )
		: mSigFxMainWindow( fxWindow )
		, tWxToolsPanelTool( parent, "Attractor Properties", "Attractor Properties", "PlaceObj" )
	{
		const wxString enumNames[ ] = {
			wxString( "Attract" ),
			wxString( "Repel" ),
			wxString( "Gravity" ),
			wxString( "Collide" ),
		};
		const u32 enumNameCount = array_length( enumNames );

		wxBoxSizer* container = new wxBoxSizer( wxVERTICAL );
		fGetMainPanel( )->GetSizer( )->Add( container, 1, wxALIGN_CENTER, 1 );
	
		wxPanel* group = new wxPanel( fGetMainPanel( ), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
		mAttractorTypeChoice = new tAttractorTypeChoice( mSigFxMainWindow, group, "Force Type", enumNames, enumNameCount, 0 );
		mParticleMustBeInRadiusCheckBox = new tMustBeInRadiusCheckBox( mSigFxMainWindow, group, "Must Be In Radius" );
		mAffectParticleDirection = new tAffectsParticleDirectionCheckBox( mSigFxMainWindow, group, "Affect Direction" );
		mFlagsCheckListBox = new tAttractorFlagsCheckListBox( mSigFxMainWindow, group, "Flags" );

		container->Add( group, 1, wxALIGN_LEFT | wxALL, 4 );
		container->AddSpacer( 8 );
	}


	void tAttractorPropertiesPanel::fUpdateSelectedList( tEditorSelectionList& list )
	{
		mAttractorList.fSetCount( 0 );
		list.fCullByType< tSigFxAttractor >( mAttractorList );

		if( !mAttractorList.fCount( ) )
			fSetCollapsed( true );
		else
			fSetCollapsed( false );

		mAttractorTypeChoice->fSetList( &mAttractorList );
		mAttractorTypeChoice->fSetList( &mAttractorList );
		mAffectParticleDirection->fSetList( &mAttractorList );
		mParticleMustBeInRadiusCheckBox->fSetList( &mAttractorList );
		mFlagsCheckListBox->fSetList( &mAttractorList );

		if( mAttractorList.fCount( ) )
		{
			u32 forceType = ( u32 )mAttractorList[ 0 ]->fGetAttractor( )->fForceType( );
			mAttractorTypeChoice->fSetValue( forceType );

			b32 particlemustbeinradius = mAttractorList[ 0 ]->fGetAttractor( )->fParticleMustBeInRadius( );
			mParticleMustBeInRadiusCheckBox->fSetValue( particlemustbeinradius != 0 );

			b32 affectparticledir = mAttractorList[ 0 ]->fGetAttractor( )->fAffectParticlesDirection( );
			mAffectParticleDirection->fSetValue( affectparticledir != 0 );
		}
	}


}


