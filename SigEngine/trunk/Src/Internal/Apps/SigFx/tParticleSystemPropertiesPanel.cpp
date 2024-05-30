#include "SigFxPch.hpp"
#include "tParticleSystemPropertiesPanel.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "wx/tooltip.h"
#include "tWxToolsPanelSlider.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{
	using namespace FX;

	class tEmitterShapeChoice  : public tWxSlapOnChoice
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
	public:
		tEmitterShapeChoice( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice = ~0 )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnChoice( parent, label, enumNames, numEnumNames, defChoice )
			, mList( 0 )	{	}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }

	protected:

		virtual void fOnControlUpdated( )
		{
			tEmitterType type = ( tEmitterType) fGetValue( );

			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )								
					( *mList )[ i ]->fGetParticleSystem( )->fSetEmitterType( type );
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};


	class tBlendOpChoice  : public tWxSlapOnChoice
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
	public:
		tBlendOpChoice( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice = ~0 )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnChoice( parent, label, enumNames, numEnumNames, defChoice )
			, mList( 0 )	{	}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }

	protected:

		virtual void fOnControlUpdated( )
		{
			const u32 blendOpIndex = fGetValue( );

			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
					( *mList )[ i ]->fGetParticleSystem( )->fSetBlendOpFromIndex( blendOpIndex );
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};
	class tSrcBlendChoice : public tWxSlapOnChoice
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
	public:
		tSrcBlendChoice( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice = ~0 )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnChoice( parent, label, enumNames, numEnumNames, defChoice )
			, mList( 0 )	{	}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }

	protected:

		virtual void fOnControlUpdated( )
		{
			const u32 srcBlend = fGetValue( );

			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
					( *mList )[ i ]->fGetParticleSystem( )->fSetSrcBlend( srcBlend );
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};
	class tDstBlendChoice : public tWxSlapOnChoice
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
	public:
		tDstBlendChoice( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice = ~0 )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnChoice( parent, label, enumNames, numEnumNames, defChoice )
			, mList( 0 )	{	}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }

	protected:

		virtual void fOnControlUpdated( )
		{
			const u32 dstBlend = fGetValue( );

			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
					( *mList )[ i ]->fGetParticleSystem( )->fSetDstBlend( dstBlend );
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};


	class tSortModeChoice  : public tWxSlapOnChoice
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
	public:
		tSortModeChoice( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice = ~0 )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnChoice( parent, label, enumNames, numEnumNames, defChoice )
			, mList( 0 )	{	}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }

	protected:

		virtual void fOnControlUpdated( )
		{
			tParticleSortMode sortMode = ( tParticleSortMode) fGetValue( );

			if( mList )
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
					( *mList )[ i ]->fGetParticleSystem( )->fSetSortMode( sortMode );
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};

	class tCameraDepthSpinner : public tWxToolsPanelSlider
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
		f32 mMinDepth;
		f32 mMaxDepth;
	public:
		tCameraDepthSpinner( tSigFxMainWindow* fxWindow, wxWindow* windowParent, const char* labelName, f32 initialValue, f32 minDepth, f32 maxDepth )
			: mSigFxMainWindow( fxWindow )
			, tWxToolsPanelSlider( windowParent, labelName, &fxWindow->fGuiApp( ).fMainWindow( ), initialValue, true )
			, mMinDepth( minDepth )
			, mMaxDepth( maxDepth )
			, mList( 0 )	{	}

		void fSetCameraDepthSliderValue( f32 depth )
		{
			f32 lerpValue = fClamp( ( depth - mMinDepth ) / ( mMaxDepth - mMinDepth ), 0.f, 1.f );
			fSetValue( lerpValue );
			fSetDisplayValueText( depth );
		}

		virtual void fOnScrollingChanged( )
		{
			if( mList)
			{
				const f32 value = Math::fLerp( mMinDepth, mMaxDepth, fGetValue( ) );
				fSetDisplayValueText( value );
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetParticleSystem( )->fSetCameraDepthOffset( value );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }
	};

	class tUpdateSpeedSpinner : public tWxToolsPanelSlider
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
		f32 mMinUpdateSpeed;
		f32 mMaxUpdateSpeed;
	public:
		tUpdateSpeedSpinner( tSigFxMainWindow* fxWindow, wxWindow* windowParent, const char* labelName, f32 initialValue, f32 minUpdateSpeed, f32 maxUpdateSpeed )
			: mSigFxMainWindow( fxWindow )
			, tWxToolsPanelSlider( windowParent, labelName, &fxWindow->fGuiApp( ).fMainWindow( ), initialValue, true )
			, mMinUpdateSpeed( minUpdateSpeed )
			, mMaxUpdateSpeed( maxUpdateSpeed )
			, mList( 0 )	{	}

		void fSetUpdateSpeedValue( f32 speed )
		{
			f32 lerpValue = fClamp( ( speed - mMinUpdateSpeed ) / ( mMaxUpdateSpeed - mMinUpdateSpeed ), 0.f, 1.f );
			fSetValue( lerpValue );
			fSetDisplayValueText( speed );
		}

		virtual void fOnScrollingChanged( )
		{
			if( mList)
			{
				const f32 value = Math::fLerp( mMinUpdateSpeed, mMaxUpdateSpeed, fGetValue( ) );
				fSetDisplayValueText( value );
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetParticleSystem( )->fSetUpdateSpeedMultiplier( value );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }


	};

	class tLodFactorSlider : public tWxToolsPanelSlider
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
	public:
		tLodFactorSlider( tSigFxMainWindow* fxWindow, wxWindow* windowParent, const char* labelName, f32 initialValue = 0.5f )
			: mSigFxMainWindow( fxWindow )
			, tWxToolsPanelSlider( windowParent, labelName, &fxWindow->fGuiApp( ).fMainWindow( ), initialValue, true )
			, mList( 0 )	{	}

		virtual void fOnScrollingChanged( )
		{
			if( mList)
			{
				const f32 value = fMax( 0.1f, fGetValue( ) );
				fSetDisplayValueText( value );
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetParticleSystem( )->fSetLodFactor( value );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetLodFactorValue( f32 lodFactor )
		{
			u32 lod = ( u32 ) lodFactor;
			fSetValue( lodFactor );
			fSetDisplayValueText( lodFactor );
		}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }
	};

	class tGhostParticleFrequencySpinner : public tWxToolsPanelSlider
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
		f32 mMinFrequency;
		f32 mMaxFrequency;
	public:
		tGhostParticleFrequencySpinner( tSigFxMainWindow* fxWindow, wxWindow* windowParent, const char* labelName, f32 initialValue, f32 minFreq, f32 maxFreq )
			: mSigFxMainWindow( fxWindow )
			, tWxToolsPanelSlider( windowParent, labelName, &fxWindow->fGuiApp( ).fMainWindow( ), initialValue, true )
			, mMinFrequency( minFreq )
			, mMaxFrequency( maxFreq )
			, mList( 0 )	{	}

		void fSetGhostParticleFrequency( f32 freq )
		{
			f32 steps = 1.f / 256.f;
			f32 best = Math::cInfinity;
			for( f32 f = 0.f; f <= 1.f; f += steps )
			{
				f32 val = Math::fLerp( mMinFrequency, mMaxFrequency, f*f );
				f32 diff = fAbs( val - freq );
				if( diff < best )
				{
					best = diff;
					if( best < 1.f )
					{
						fSetValue( f );
						fSetDisplayValueText( freq );
					}
				}
			}
		}

		virtual void fOnScrollingChanged( )
		{
			if( mList)
			{
				const f32 value = Math::fLerp( mMinFrequency, mMaxFrequency, fGetValue( )*fGetValue( ) );
				fSetDisplayValueText( value );
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetParticleSystem( )->fSetGhostParticleFrequency( value );
				}				
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }
	};

	class tGhostParticleLifetimeSpinner : public tWxToolsPanelSlider
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxParticleSystem* >* mList;
		f32 mMinFrequency;
		f32 mMaxFrequency;
	public:
		tGhostParticleLifetimeSpinner( tSigFxMainWindow* fxWindow, wxWindow* windowParent, const char* labelName, f32 initialValue, f32 minFreq, f32 maxFreq )
			: mSigFxMainWindow( fxWindow )
			, tWxToolsPanelSlider( windowParent, labelName, &fxWindow->fGuiApp( ).fMainWindow( ), initialValue, true )
			, mMinFrequency( minFreq )
			, mMaxFrequency( maxFreq )
			, mList( 0 )	{	}

		void fSetGhostParticleLifetime( f32 freq )
		{
			f32 steps = 1.f / 256.f;
			f32 best = Math::cInfinity;
			for( f32 f = 0.f; f <= 1.f; f += steps )
			{
				f32 val = Math::fLerp( mMinFrequency, mMaxFrequency, f*f );
				f32 diff = fAbs( val - freq );
				if( diff < best )
				{
					best = diff;
					if( best < 1.f )
					{
						fSetValue( f );
						fSetDisplayValueText( freq );
					}
				}
			}
		}

		virtual void fOnScrollingChanged( )
		{
			if( mList)
			{
				const f32 value = Math::fLerp( mMinFrequency, mMaxFrequency, fGetValue( )*fGetValue( ) );
				fSetDisplayValueText( value );
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetParticleSystem( )->fSetGhostParticleLifetime( value );
				}				
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }
	};


	/*
	class tBurstModeCheckBox : public tWxSlapOnCheckBox
	{
		tGrowableArray< tSigFxParticleSystem* >* mList;
	public:
		tBurstModeCheckBox( wxWindow* parent, const char* label )
			: tWxSlapOnCheckBox( parent, label )
			, mList( 0 )	{	}

		virtual void fOnControlUpdated( )
		{
			if( mList)
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fGetParticleSystem( )->fSetBurstMode( fGetValue( ) == true );
				}
			}
		}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list ) { mList = list; }
	};
	*/


	class tSystemFlagsCheckListBox :  public tWxSlapOnControl
	{
		tSigFxMainWindow* mSigFxMainWindow;
		wxCheckListBox* mCheckListBox;
		tGrowableArray< tSigFxParticleSystem* >* mList;
	public:

		tSystemFlagsCheckListBox( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnControl( parent, label )
			, mCheckListBox( 0 )
		{
			mCheckListBox = new wxCheckListBox( parent, wxID_ANY, wxDefaultPosition, wxSize( fControlWidth( ), wxDefaultSize.y ), 0, wxLB_EXTENDED | wxLB_NEEDED_SB );
			fAddWindowToSizer( mCheckListBox, true );
			mCheckListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( tSystemFlagsCheckListBox::fOnControlUpdated ), NULL, this );

			for( u32 i = 0; i < FX::cParticleSystemFlagsCount; ++i )
				mCheckListBox->Insert( tToolParticleSystemState::mParticleSystemFlagStrings[ i ].fCStr( ), i );
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
				const u32 flag = ( 1 << i );

				for( u32 j = 0; j < mList->fCount( ); ++j )
				{
					const b32 hasCurFlag = ( *mList )[ j ]->fHasFlag( flag );
					( *mList )[ j ]->fRemoveFlag( flag );

					if( mCheckListBox->IsChecked( i ) )
					{
						if( flag == FX::cGhostImage && !hasCurFlag )
						{
							if( ( *mList )[ j ]->fGetParticleSystem( )->fParticleCount( ) > 500 )
							{
								wxMessageDialog msgBox( this->fParent( ), "Are you sure you want to enable ghost particles on this particle-heavy-system? Selecting 'yes' could possibly hang the program...", "Enable Ghost Particles?", wxCENTRE | wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION );
								if( msgBox.ShowModal( ) != wxID_YES )
								{
									mCheckListBox->Check( i, false );
									continue;
								}
								else
								{
									( *mList )[ j ]->fGetParticleSystem( )->fResetLastGhostParticleEmissionTime( );
								}
							}
						}

						( *mList )[ j ]->fAddFlag( flag );
					}
				}					
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxParticleSystem* >* list )
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
					if( ( *mList )[ j ]->fGetParticleSystem( )->fHasFlag( ( 1 << i ) ) )
						mCheckListBox->Check( i );
				}
			}
		}
	};


	tIgnoreAttractorChecklist::tIgnoreAttractorChecklist( wxWindow* parent, const char* label, tEditableObjectContainer& list, tSigFxMainWindow* window )
		: tWxSlapOnControl( parent, label )
		, mCheckListBox( 0 )
		, mObjectList( list )
		, mSigFxMainWindow( window )
	{
		mCheckListBox = new wxCheckListBox( parent, wxID_ANY, wxDefaultPosition, wxSize( fControlWidth( ), wxDefaultSize.y ), 0, wxLB_EXTENDED | wxLB_NEEDED_SB );
		fAddWindowToSizer( mCheckListBox, true );
		fRefresh( );
		mCheckListBox->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( tIgnoreAttractorChecklist::fOnControlUpdated ), NULL, this );
	}

	void tIgnoreAttractorChecklist::fEnableControl( )
	{
		mCheckListBox->Enable( );
	}

	void tIgnoreAttractorChecklist::fDisableControl( )
	{
		mCheckListBox->Disable( );
	}

	void tIgnoreAttractorChecklist::fOnControlUpdated( wxCommandEvent& event )
	{
		tGrowableArray< tSigFxParticleSystem* > systems;
		mObjectList.fCollectAllByType< tSigFxParticleSystem >( systems );

		tGrowableArray< tSigFxMeshSystem* > meshes;
		mObjectList.fCollectAllByType< tSigFxMeshSystem >( meshes );

		tGrowableArray< tSigFxAttractor* > attractors;
		mObjectList.fCollectAllByType< tSigFxAttractor >( attractors );
	
		for( u32 i = 0; i < systems.fCount( ); ++i )
		{
			if( systems[ i ]->fGetSelected( ) )
			{
				systems[ i ]->fClearAttractorIgnoreIds( );
				systems[ i ]->fGetParticleSystem( )->fSyncAttractorIgnoreIds( );
				systems[ i ]->fGetParticleSystem( )->fSetAttractors( mSigFxMainWindow->fFxScene( )->fGetAttractorsList( ) );
			}
		}
		for( u32 i = 0; i < meshes.fCount( ); ++i )
		{
			if( meshes[ i ]->fGetSelected( ) )
			{
				meshes[ i ]->fFxMeshSystem( )->fFxMeshSystemData( )->fClearAttractorIgnoreIds( );
				meshes[ i ]->fFxMeshSystem( )->fSetAttractors( mSigFxMainWindow->fFxScene( )->fGetAttractorsList( ) );
			}
		}

		for( u32 i = 0; i < mCheckListBox->GetCount( ); ++i )
		{
			for( u32 j = 0; j < attractors.fCount( ); ++j )
			{
				if( mCheckListBox->IsChecked( i ) )
				{
					const char* s1 = attractors[ j ]->fAttractorName( ).fCStr( );
					wxString s2 = mCheckListBox->GetItem( i )->GetName( );
					if( !strcmp( s1, s2 ) )
					{
						for( u32 s = 0; s < systems.fCount( ); ++s )
						{
							if( systems[ s ]->fGetSelected( ) )
							{
								systems[ s ]->fAddAttractorIgnoreId( attractors[ j ]->fGetAttractor( )->fId( ) );
								systems[ s ]->fGetParticleSystem( )->fSyncAttractorIgnoreIds( );
								systems[ s ]->fGetParticleSystem( )->fSetAttractors( mSigFxMainWindow->fFxScene( )->fGetAttractorsList( ) );
							}
						}
						for( u32 s = 0; s < meshes.fCount( ); ++s )
						{
							if( meshes[ s ]->fGetSelected( ) )
							{
								meshes[ s ]->fFxMeshSystem( )->fFxMeshSystemData( )->fAddAttractorIgnoreId( attractors[ j ]->fGetAttractor( )->fId( ) );
								meshes[ s ]->fFxMeshSystem( )->fSetAttractors( mSigFxMainWindow->fFxScene( )->fGetAttractorsList( ) );
							}
						}
					}
				}
			}
		}

		mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		mSigFxMainWindow->fSync( );
	}

	void tIgnoreAttractorChecklist::fRefresh( )
	{
		mCheckListBox->Clear( );

		tGrowableArray< tSigFxAttractor* > attractors;
		mObjectList.fCollectAllByType< tSigFxAttractor >( attractors );
					
		tGrowableArray< tSigFxParticleSystem* > systems;
		mObjectList.fCollectAllByType< tSigFxParticleSystem >( systems );

		tGrowableArray< tSigFxMeshSystem* > meshes;
		mObjectList.fCollectAllByType< tSigFxMeshSystem >( meshes );

		for( u32 i = 0; i < attractors.fCount( ); ++i )
		{
			tSigFxAttractor* attractor = attractors[ i ]->fDynamicCast< tSigFxAttractor >( );

			if( attractor )
			{
				u32 id = attractor->fGetAttractor( )->fId( );
				mCheckListBox->Insert( attractor->fAttractorName( ).fCStr( ), i );
				mCheckListBox->Check( i, false );

				for( u32 j = 0; j < systems.fCount( ); ++j )
				{
					if( systems[ j ]->fGetSelected( ) )
					{
						for( u32 x = 0; x < systems[ j ]->fGetParticleSystem( )->fState( ).fAttractorIgnoreListCount( ); ++x )
						{
							if( systems[ j ]->fGetParticleSystem( )->fState( ).fAttractorIgnoreId( x ) == id )
								mCheckListBox->Check( i );
						}
					}
				}

				for( u32 j = 0; j < meshes.fCount( ); ++ j )
				{
					if( meshes[ j ]->fGetSelected( ) )
					{
						for( u32 x = 0; x < meshes[ j ]->fFxMeshSystem( )->fFxMeshSystemData( )->fAttractorIgnoreListCount( ); ++x )
						{
							if( meshes[ j ]->fFxMeshSystem( )->fFxMeshSystemData( )->fAttractorIgnoreId( x ) == id )
								mCheckListBox->Check( i );
						}
					}
				}
			}
		}
	}

	tParticleSystemPropertiesPanel::tParticleSystemPropertiesPanel( tWxToolsPanel* parent, tEditableObjectContainer& list, tSigFxMainWindow* window )
		: tWxToolsPanelTool( parent, "Particle System Properties", "Particle System Properties", "PlaceObj" )
		, mObjectList( list )
		, mSigFxMainWindow( window )
	{				
		wxString shapeEnums[ ] = { wxString( "Point" ),
								 wxString( "Sphere" ),
								 wxString( "Box" ),
								 wxString( "Fountain" ),
								 wxString( "Shockwave" ),
								 wxString( "Cylinder" ),
		};

		wxString blendOpEnums[ ] = { wxString( "Add" ),
								 wxString( "Subtract" ),
								 wxString( "ReverseSubtract" ),
								 wxString( "Min" ),
								 wxString( "Max" ) };

		wxString blendModeEnums[ ] = { wxString( "One" ),
								 wxString( "Zero" ),
								 wxString( "SrcAlpha" ),
								 wxString( "OneMinusSrcAlpha" ),
								 wxString( "SrcColor" ),
								 wxString( "OneMinusSrcColor" ),
								 wxString( "DstAlpha" ),
								 wxString( "OneMinusDstAlpha" ),
								 wxString( "DstColor" ),
								 wxString( "OneMinusDstColor" ) };

		wxString sortEnums[ ] = { wxString( "None" ),
								 wxString( "Distance" ),
								 wxString( "First Born" ),
								 wxString( "Last Born" ),
								 wxString( "Alpha" ) };

		wxBoxSizer* container = new wxBoxSizer( wxVERTICAL );

		wxPanel* group = new wxPanel( fGetMainPanel( ), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

		mEmitterShapeChoice = new tEmitterShapeChoice( mSigFxMainWindow, group, "Emitter Shape", shapeEnums, array_length( shapeEnums ), 0 );
		
		mBlendOpChoice = new tBlendOpChoice( mSigFxMainWindow, group, "Blend Op", blendOpEnums, array_length( blendOpEnums ), 0 );
		mSrcBlendChoice = new tSrcBlendChoice( mSigFxMainWindow, group, "Src Blend", blendModeEnums, array_length( blendModeEnums ), 2 );
		mDstBlendChoice = new tDstBlendChoice( mSigFxMainWindow, group, "Dst Blend", blendModeEnums, array_length( blendModeEnums ), 0 );

		mSortModeChoice = new tSortModeChoice( mSigFxMainWindow, group, "Sort Mode", sortEnums, array_length( sortEnums ), 0 ); 
		mUpdateSpeedSpinner = new tUpdateSpeedSpinner( mSigFxMainWindow, group, "System Update Rate", 0.5f, 0.1f, 10.f );
		mCameraDepthSpinner = new tCameraDepthSpinner( mSigFxMainWindow, group, "Camera Depth Offset", 0.5f, -100.f, 100.f );
		mLodFactorSpinner = new tLodFactorSlider( mSigFxMainWindow, group, "System LOD Factor", 1.f );
		mGhostParticleFrequencySpinner = new tGhostParticleFrequencySpinner( mSigFxMainWindow, group, "Ghost Particle Freq", 0.25f, 30.f, 2000.f );
		mGhostParticleLifetimeSpinner = new tGhostParticleLifetimeSpinner( mSigFxMainWindow, group, "Ghost Particle Life", 0.2f, 0.1f, 10.f );

		//mBurstModeCheckBox = new tBurstModeCheckBox( group, wxString( "Burst Mode" ));
		mSystemFlagsCheckList = new tSystemFlagsCheckListBox( mSigFxMainWindow, group, wxString( "System Flags" ) );
		mAttractorIgnoreChecklist = new tIgnoreAttractorChecklist( group, "Ignore Attractors", mObjectList, mSigFxMainWindow );


		container->Add( group, 0, wxALIGN_LEFT , 4 );

		container->AddSpacer( 8 );
		fGetMainPanel( )->GetSizer( )->Add( container, 0, wxALIGN_CENTER, 0 );
	}
	

	void tParticleSystemPropertiesPanel::fUpdateSelectedList( tEditorSelectionList& list )
	{
		mParticleSystemsList.fSetCount( 0 );
		list.fCullByType< tSigFxParticleSystem >( mParticleSystemsList );

		if( !mParticleSystemsList.fCount( ) )
			fSetCollapsed( true );
		else
			fSetCollapsed( false );

		mCameraDepthSpinner->fSetList( &mParticleSystemsList );
		mUpdateSpeedSpinner->fSetList( &mParticleSystemsList );
		mLodFactorSpinner->fSetList( &mParticleSystemsList );
		mGhostParticleFrequencySpinner->fSetList( &mParticleSystemsList );
		mGhostParticleLifetimeSpinner->fSetList( &mParticleSystemsList );

//		mBurstModeCheckBox->fSetList( &mParticleSystemsList );
		mSystemFlagsCheckList->fSetList( &mParticleSystemsList );
		mBlendOpChoice->fSetList( &mParticleSystemsList );
		mSrcBlendChoice->fSetList( &mParticleSystemsList );
		mDstBlendChoice->fSetList( &mParticleSystemsList );
		mSortModeChoice->fSetList( &mParticleSystemsList );
		mEmitterShapeChoice->fSetList( &mParticleSystemsList );
		mAttractorIgnoreChecklist->fRefresh( );

		f32 offset( 0.f );
		f32 updateSpeed( 1.f );
		f32 lodFactor( 1.f );
		f32 ghostParticleFrequency( 60.f );
		f32 ghostParticleLifetime( 0.5f );	// as a percent of the parents' life
		b32 invalid = false;
		for( u32 i = 0; i < mParticleSystemsList.fCount( ) && !invalid; ++i )
		{
			FX::tParticleSystemPtr ps = mParticleSystemsList[ i ]->fGetParticleSystem( );
			if( i == 0 )
			{
				offset = ps->fCameraDepthOffset( );
				updateSpeed = ps->fUpdateSpeedMultiplier( );
				lodFactor = ps->fLodFactor( );
				ghostParticleFrequency = ps->fGhostParticleFrequency( );
				ghostParticleLifetime = ps->fGhostParticleLifetime( );
			}
			else
			{
				f32 off = ps->fCameraDepthOffset( );
				f32 us = ps->fUpdateSpeedMultiplier( );
				f32 lf = ps->fLodFactor( );
				f32 gpf = ps->fGhostParticleFrequency( );
				f32 gpl = ps->fGhostParticleLifetime( );

				if( off != offset || us != updateSpeed || lf != lodFactor || ghostParticleFrequency != gpf || ghostParticleLifetime != gpl )
					invalid = true;
			}

			u32 shape = ( u32 ) ps->fGetEmitterType( );
			mEmitterShapeChoice->fSetValue( shape );

			u32 blendOp = ( u32 ) ps->fRenderState( ).fGetBlendOpAsIndex( );
			mBlendOpChoice->fSetValue( blendOp );
			u32 srcBlend = ( u32 ) ps->fRenderState( ).fGetSrcBlendMode( );
			mSrcBlendChoice->fSetValue( srcBlend );
			u32 dstBlend = ( u32 ) ps->fRenderState( ).fGetDstBlendMode( );
			mDstBlendChoice->fSetValue( dstBlend );

			u32 sort = ( u32 ) ps->fSortMode( );
			mSortModeChoice->fSetValue( sort );

			//b32 burstMode = ps->fBurstMode( );
			//mBurstModeCheckBox->fSetValue( burstMode ? true : false );
		}

		if( invalid )
		{
			//mCameraDepthOffsetSpinner->fInvalidText( );
		}
		else
		{
			if( mParticleSystemsList.fCount( ) )
			{
				mCameraDepthSpinner->fSetCameraDepthSliderValue( offset );
				mUpdateSpeedSpinner->fSetUpdateSpeedValue( updateSpeed );
				mLodFactorSpinner->fSetLodFactorValue( lodFactor );
				mGhostParticleFrequencySpinner->fSetGhostParticleFrequency( ghostParticleFrequency );
				mGhostParticleLifetimeSpinner->fSetGhostParticleLifetime( ghostParticleLifetime );
			}
		}
	}
}


