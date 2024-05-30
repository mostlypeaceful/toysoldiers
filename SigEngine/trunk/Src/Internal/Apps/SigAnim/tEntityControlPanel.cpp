//------------------------------------------------------------------------------
// \file tEntityControlPanel.cpp - 26 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigAnimPch.hpp"
#include "tEntityControlPanel.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "Animation/tSkeletableSgFileRefEntity.hpp"
#include "Editor/tEditableObjectContainer.hpp"

namespace Sig
{
	namespace 
	{
		const wxString cPlayLabel( "|> Play" );
		const wxString cPauseLabel( "|| Pause" );
		const f32 cForceStepDefault = 1 / 60.f;
	}

	///
	/// \class tTimeScaleSpinner
	/// \brief 
	class tTimeScaleSpinner : public tWxSlapOnSpinner
	{
	public:
		tTimeScaleSpinner( wxWindow * parent, tEditableObjectContainer & container )
			: tWxSlapOnSpinner( parent, "Time Scale", 0.f, 100.f, 0.01f, 2, 100 )
			, mObjects( container )
		{
			fSetValue( 1.f );
		}

	protected:
		
		virtual void fOnControlUpdated( )
		{
			tWxSlapOnSpinner::fOnControlUpdated( );

			tGrowableArray<tSkeletableSgFileRefEntity *> ents;
			mObjects.fCollectSelectedOrOnly( ents, tSkeletableSgFileRefEntity::fIsCursor );

			const u32 count = ents.fCount( );
			for( u32 e = 0; e < count; ++e )
			{
				tSkeletableSgFileRefEntity * entity = ents[ e ];
				entity->fSetTimeScale( fGetValue( ) );
			}
		}

	private:
		tEditableObjectContainer & mObjects;
	};

	///
	/// \class tMotionRangeSpinner
	/// \brief 
	class tMotionRangeSpinner : public tWxSlapOnSpinner
	{
	public:
		tMotionRangeSpinner( wxWindow * parent, tEditableObjectContainer & container )
			: tWxSlapOnSpinner( parent, "Motion Range", 0.f, 1000.f, 0.01f, 2, 100 )
			, mObjects( container )
		{
			fSetValue( 20.f );
		}

	protected:
		
		virtual void fOnControlUpdated( )
		{
			tWxSlapOnSpinner::fOnControlUpdated( );

			tGrowableArray<tSkeletableSgFileRefEntity *> ents;
			mObjects.fCollectSelectedOrOnly( ents, tSkeletableSgFileRefEntity::fIsCursor );

			const u32 count = ents.fCount( );
			for( u32 e = 0; e < count; ++e )
			{
				tSkeletableSgFileRefEntity * entity = ents[ e ];
				entity->fSetMotionRange( fGetValue( ) );
			}
		}

	private:
		tEditableObjectContainer & mObjects;
	};

	///
	/// \class tForceStepSpinner
	/// \brief 
	class tForceStepSpinner : public tWxSlapOnSpinner
	{
	public:
		tForceStepSpinner( wxWindow * parent )
			: tWxSlapOnSpinner( parent, "Force Step", 0.001f, 1.f, 0.0001f, 5, 100 )
		{
			fSetValue( cForceStepDefault );
		}
	};

	//------------------------------------------------------------------------------
	tEntityControlPanel::tEntityControlPanel( tWxToolsPanel * parent )
		: tWxToolsPanelTool( parent, "Entity Controls", "Entity Controls", "EntityCtrls" )
	{
		fGetMainPanel( )->SetForegroundColour( wxColor( 0xff, 0xff, 0xff ) );

		mOnSelectionChanged.fFromMethod<tEntityControlPanel, &tEntityControlPanel::fOnSelectionChanged>( this );
		parent->fGuiApp( ).fEditableObjects( ).fGetSelectionList( ).fGetSelChangedEvent( ).fAddObserver( &mOnSelectionChanged );

		{
			wxBoxSizer * sizer = new wxBoxSizer( wxHORIZONTAL );

			mRenderSkeleton = new wxCheckBox( fGetMainPanel( ), wxID_ANY, "Render Skeleton", wxDefaultPosition, wxDefaultSize, wxCHK_3STATE );
			mRenderSkeleton->Connect( 
				wxEVT_COMMAND_CHECKBOX_CLICKED, 
				wxCommandEventHandler( tEntityControlPanel::fOnRenderSkeleton ), NULL, this );
			mApplyRefFrame = new wxCheckBox( fGetMainPanel( ), wxID_ANY, "Apply Ref Frame", wxDefaultPosition, wxDefaultSize, wxCHK_3STATE );
			mApplyRefFrame->Connect( 
				wxEVT_COMMAND_CHECKBOX_CLICKED, 
				wxCommandEventHandler( tEntityControlPanel::fOnApplyRefFrame ), NULL, this );
			sizer->Add( mRenderSkeleton, 1, wxEXPAND | wxALL, 3 );
			sizer->Add( mApplyRefFrame, 1, wxEXPAND | wxALL, 3 );

			fGetMainPanel( )->GetSizer( )->Add(sizer, 1, wxEXPAND | wxALL, 3 );
		}

		{
			wxBoxSizer * sizer = new wxBoxSizer( wxHORIZONTAL );

			mReversePlay = new wxCheckBox( fGetMainPanel( ), wxID_ANY, "Reverse Play", wxDefaultPosition, wxDefaultSize, wxCHK_3STATE );
			mReversePlay->Connect( 
				wxEVT_COMMAND_CHECKBOX_CLICKED, 
				wxCommandEventHandler( tEntityControlPanel::fOnReversePlay ), NULL, this );
			//mPartialsMatch = new wxCheckBox( fGetMainPanel( ), wxID_ANY, "Match Partials", wxDefaultPosition, wxDefaultSize, wxCHK_3STATE );
			//mPartialsMatch->Connect( 
			//	wxEVT_COMMAND_CHECKBOX_CLICKED, 
			//	wxCommandEventHandler( tEntityControlPanel::fOnPartialsMatch ), NULL, this );
			sizer->Add( mReversePlay, 1, wxEXPAND | wxALL, 3 );
			//sizer->Add( mPartialsMatch, 1, wxEXPAND | wxALL, 3 );

			fGetMainPanel( )->GetSizer( )->Add(sizer, 1, wxEXPAND | wxALL, 3 );
		}

		mTimeScale = new tTimeScaleSpinner( fGetMainPanel( ), fParent( )->fGuiApp( ).fEditableObjects( ) );
		mMotionRange = new tMotionRangeSpinner( fGetMainPanel( ), fParent( )->fGuiApp( ).fEditableObjects( ) );
		mForceStep = new tForceStepSpinner( fGetMainPanel( ) );

		{
			wxBoxSizer* wSizer = new wxBoxSizer( wxHORIZONTAL );
			fGetMainPanel( )->GetSizer( )->Add( wSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 45 );

			wxButton* stepBack = new wxButton( fGetMainPanel( ), wxID_ANY, "<", wxDefaultPosition, wxSize( 15, wxDefaultSize.y ) );
			stepBack->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEntityControlPanel::fStepBack ), NULL, this );
			wSizer->Add( stepBack, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxBOTTOM, 10 );

			mPlayPauseButton = new wxButton( fGetMainPanel( ), wxID_ANY, cPlayLabel );
			mPlayPauseButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEntityControlPanel::fPlayPause ), NULL, this );
			wSizer->Add( mPlayPauseButton, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 10 );

			wxButton* stepForward = new wxButton( fGetMainPanel( ), wxID_ANY, ">", wxDefaultPosition, wxSize( 15, wxDefaultSize.y ) );
			stepForward->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEntityControlPanel::fStepForward ), NULL, this );
			wSizer->Add( stepForward, 0, wxALIGN_CENTER_HORIZONTAL | wxRIGHT | wxTOP | wxBOTTOM, 10 );
		}

		wxButton * clear = new wxButton( fGetMainPanel( ), wxID_ANY, "T-Pose" );
		clear->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEntityControlPanel::fClearTracks ), NULL, this );
		fGetMainPanel( )->GetSizer( )->Add( clear, 0, wxEXPAND | wxALL, 3 );

		fOnSelectionChanged( parent->fGuiApp( ).fEditableObjects( ).fGetSelectionList( ) );

	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fStepBack( wxCommandEvent & )
	{
		fStep( -mForceStep->fGetValue( ) );
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fStepForward( wxCommandEvent & )
	{
		fStep( mForceStep->fGetValue( ) );
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fPlayPause( wxCommandEvent & )
	{
		fSetPaused( fPaused( ) );
	}

	b32 tEntityControlPanel::fPaused( ) const
	{
		return mPlayPauseButton->GetLabel( ) == cPauseLabel;
	}

	f32 tEntityControlPanel::fStepSize( ) const
	{
		return mForceStep->fGetValue( );
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fStep( f32 time )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( 
			ents, tSkeletableSgFileRefEntity::fIsCursor );

		const u32 count = ents.fCount( );
		for( u32 e = 0; e < count; ++e )
		{
			tSkeletableSgFileRefEntity * entity = ents[ e ];
			entity->fForceStep( time );
		}

		if( mPlayPauseButton->GetLabel( ) != cPlayLabel )
			mPlayPauseButton->SetLabel( cPlayLabel );
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fClearTracks( wxCommandEvent & )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( 
			ents, tSkeletableSgFileRefEntity::fIsCursor );

		const u32 count = ents.fCount( );
		for( u32 e = 0; e < count; ++e )
		{
			const Anim::tAnimatedSkeletonPtr & skeleton = ents[ e ]->fSkeleton( );
			if( !skeleton )
				continue;

			skeleton->fClearTracks( );
			skeleton->fSetToIdentity( );
		}
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fOnSelectionChanged( tEditorSelectionList & list )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( 
			ents, tSkeletableSgFileRefEntity::fIsCursor );

		s32 paused = 0;

		b32 reversed = false; b32 reversedConflict = false;
		b32 renderSkeleton = false; b32 renderSkeletonConflict = false;
		b32 applyRefFrame = false; b32 applyRefFrameConflict = false;

		f32 timeScale = 1; b32 timeScaleConflict = false;
		f32 motion = 20; b32 motionConflict = false;
		
		if( ents.fCount( ) )
		{
			tSkeletableSgFileRefEntity * entity = ents[ 0 ];
			paused += entity->fPaused( ) ? 1 : -1;

			reversed = entity->fReverseTime( );
			renderSkeleton = entity->fRenderSkeleton( );
			applyRefFrame = entity->fApplyRefFrame( );
			timeScale = entity->fTimeScale( );
			motion = entity->fMotionRange( );
		}

		const u32 count = ents.fCount( );
		for( u32 e = 1; e < count; ++e )
		{
			tSkeletableSgFileRefEntity * entity = ents[ e ];

			paused += entity->fPaused( ) ? 1 : -1;
			if( entity->fReverseTime( ) != reversed )
				reversedConflict = true;
			if( entity->fRenderSkeleton( ) != renderSkeleton )
				renderSkeletonConflict = true;
			if( entity->fTimeScale( ) != timeScale )
				timeScaleConflict = true;
			if( entity->fMotionRange( ) != motion )
				motionConflict = true;
			if( entity->fApplyRefFrame( ) != applyRefFrame )
				applyRefFrameConflict = true;
		}

		mPlayPauseButton->SetLabel( paused > 0 ? cPlayLabel : cPauseLabel );
		
		if( reversedConflict )
			mReversePlay->Set3StateValue( wxCHK_UNDETERMINED );
		else
			mReversePlay->SetValue( reversed ? true : false );

		if( renderSkeletonConflict )
			mRenderSkeleton->Set3StateValue( wxCHK_UNDETERMINED );
		else
			mRenderSkeleton->SetValue( renderSkeleton ? true : false );

		if( applyRefFrameConflict )
			mApplyRefFrame->Set3StateValue( wxCHK_UNDETERMINED );
		else
			mApplyRefFrame->SetValue( applyRefFrame ? true : false );

		if( timeScaleConflict )
			mTimeScale->fSetIndeterminateNoEvent( );
		else
			mTimeScale->fSetValueNoEvent( timeScale );

		if( motionConflict )
			mMotionRange->fSetIndeterminateNoEvent( );
		else
			mMotionRange->fSetValueNoEvent( motion );
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fOnRenderSkeleton( wxCommandEvent & )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( 
			ents, tSkeletableSgFileRefEntity::fIsCursor );

		const u32 count = ents.fCount( );
		for( u32 e = 0; e < count; ++e )
		{
			tSkeletableSgFileRefEntity * entity = ents[ e ];
			entity->fSetRenderSkeleton( mRenderSkeleton->GetValue( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fOnApplyRefFrame( wxCommandEvent & )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( 
			ents, tSkeletableSgFileRefEntity::fIsCursor );

		const u32 count = ents.fCount( );
		for( u32 e = 0; e < count; ++e )
		{
			tSkeletableSgFileRefEntity * entity = ents[ e ];
			entity->fSetApplyRefFrame( mApplyRefFrame->GetValue( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fOnReversePlay( wxCommandEvent & )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( 
			ents, tSkeletableSgFileRefEntity::fIsCursor );

		const u32 count = ents.fCount( );
		for( u32 e = 0; e < count; ++e )
		{
			tSkeletableSgFileRefEntity * entity = ents[ e ];
			entity->fSetReverseTime( mReversePlay->GetValue( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tEntityControlPanel::fSetPaused( b32 paused )
	{
		tGrowableArray<tSkeletableSgFileRefEntity *> ents;
		fParent( )->fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( ents, tSkeletableSgFileRefEntity::fIsCursor );
		
		const u32 count = ents.fCount( );
		for( u32 e = 0; e < count; ++e )
		{
			tSkeletableSgFileRefEntity * entity = ents[ e ];
			entity->fSetPaused( paused );
		}

		if( count )
			mPlayPauseButton->SetLabel( paused ? cPlayLabel : cPauseLabel );
	}

}
