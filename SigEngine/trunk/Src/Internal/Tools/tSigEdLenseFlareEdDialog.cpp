#include "ToolsPch.hpp"
#include "tSigEdLenseFlareEdDialog.hpp"
#include "tApplication.hpp"
#include "Gfx/tRenderContext.hpp"
#include "FileSystem.hpp"
#include "Editor\tEditablePropertyColor.hpp"

namespace Sig
{
	namespace
	{
		tEditablePropertyPtr cNullProperty( new tEditablePropertyString( "cDummy" ) );

		const u32 cRenderWidth = 600;
		const u32 cRenderHeight = cRenderWidth * (720/1024.f);

		static const char* cPropertyFilename = "Flare.Filename";
		static const char* cPropertyPosition = "Flare.Position";
		static const char* cPropertyScale = "Flare.Scale";
		static const char* cPropertyTint = "Flare.Tint";
		static const char* cPropertyAlpha = "Flare.Alpha";
	}

	class tLenseFlareEditPanel : public wxWindow
	{
	public:
		tLenseFlareEditPanel( wxWindow* parent, Gfx::tLightEntity* lightToControl )
			: wxWindow( parent, wxID_ANY )
			, mLightToControl( lightToControl )
		{
			SetSizer( new wxBoxSizer( wxVERTICAL ) );
			SetMaxSize( wxSize( 600, 200 ) );

			fSetupUI( );

			mCachedProjectData = fProjectFlares( );

			mChanged = false;
			fPopulateStyles( );

			Layout( );
		}

		tGrowableArray<tProjectFile::tLenseFlare>& fProjectFlares( )
		{
			return tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mLenseFlares;
		}

		void fPopulateStyles( )
		{
			mType->Clear( );
			for( u32 i = 0; i < fProjectFlares( ).fCount( ); ++i )
				mType->Append( fProjectFlares( )[ i ].mName );

			fPopulateStyle( wxCommandEvent( ) );
		}

		void fPopulateStyle( wxCommandEvent& )
		{
			u32 currentSel = mFlareList->GetSelection( );
			mFlareList->Clear( );

			tProjectFile::tLenseFlare* flare = fEditingStyle( );

			if( flare )
			{
				sigassert( mLightToControl );
				mLightToControl->mLenseFlareKey = flare->mKey;

				tDynamicArray< Gfx::tLenseFlareData::tFlare >& flares = flare->mData.mFlares;
				std::sort( flares.fBegin( ), flares.fEnd( ) );

				for( u32 i = 0; i < flares.fCount( ); ++i )
				{
					std::string name = StringUtil::fToString( flares[ i ].mPosition );
					name += " ";
					name += flares[ i ].mTexture.fCStr( );
					mFlareList->Append( name );
				}

				mFlareList->SetSelection( currentSel );
			}
			else
				mLightToControl->mLenseFlareKey = ~0;
					
			fPopulateFlare( wxCommandEvent( ) );
		}

		void fPopulateFlare( wxCommandEvent& )
		{
			mEditingFlareProps.fClearGui( );
			mPropertyPanel->DestroyChildren( );

			Gfx::tLenseFlareData::tFlare* flare = fEditingFlare( );

			if( flare )
			{
				// move data over to UI.
				mEditingFlare = tFlareEditableData( );
				mEditingFlare.fFromFlare( *flare );

				mEditingFlareProps = mEditingFlare.mTable;
				mEditingFlareProps.fCollectCommonPropertiesForGui( mEditingFlare.mTable );
				tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mEditingFlareProps, false );
			}

			mPropertyPanel->Layout( );
		}

		b32 fReadyToClose( )
		{
			if( mChanged )
			{
				u32 msgResult = wxMessageBox("Save your changes??",
					"Please confirm",
					wxICON_QUESTION | wxYES_NO | wxCANCEL);

				if( msgResult == wxYES )
					fSave( wxCommandEvent( ) );
				else if( msgResult == wxNO )
					fCancel( wxCommandEvent( ) );
			}

			return !mChanged;
		}

	private:
		struct tFlareEditableData
		{
			tFlareEditableData( )
			{
				mTable.fInsert( tEditablePropertyPtr( NEW tEditablePropertyFileNameString( cPropertyFilename ) ) );
				mTable.fInsert( tEditablePropertyPtr( NEW tEditablePropertyFloat( cPropertyPosition, -1.f, -99.f, 99.f, 0.01f, 2 ) ) );
				mTable.fInsert( tEditablePropertyPtr( NEW tEditablePropertyVec2f( cPropertyScale, 1.f, -99.f, 99.f, 0.01f, 2 ) ) );		
				mTable.fInsert( tEditablePropertyPtr( NEW tEditablePropertyColor( cPropertyTint ) ) );		
				mTable.fInsert( tEditablePropertyPtr( NEW tEditablePropertyFloat( cPropertyAlpha, 0.2f, 0.f, 1.f, 0.01f, 2 ) ) );		
			}

			void fFromFlare( Gfx::tLenseFlareData::tFlare& flare )
			{
				mTable.fSetData( cPropertyFilename, std::string( flare.mTexture.fCStr( ) ? flare.mTexture.fCStr( ) : "" ) );
				mTable.fSetData( cPropertyPosition, flare.mPosition );
				mTable.fSetData( cPropertyScale, flare.mScale );
				mTable.fSetData( cPropertyTint, tColorPickerData( flare.mColor.fXYZ( ) ) );
				mTable.fSetData( cPropertyAlpha, flare.mColor.w );
			}

			void fToFlare( Gfx::tLenseFlareData::tFlare& flare )
			{
				flare.mTexture = tFilePathPtr( mTable.fGetValue( cPropertyFilename, std::string( "" ) ) ); 
				flare.mPosition = mTable.fGetValue( cPropertyPosition, -1.f );
				flare.mScale = mTable.fGetValue( cPropertyScale, Math::tVec2f( 1 ) );
					
				tColorPickerData color = mTable.fGetValue( cPropertyTint, tColorPickerData( ) );
				f32 alpha = mTable.fGetValue( cPropertyAlpha, 0.2f );
				flare.mColor = Math::tVec4f( color.mRgb, alpha );
			}

			tEditablePropertyTable mTable;
		};

		wxComboBox* mType;
		wxListBox* mFlareList;
		wxPanel* mPropertyPanel;

		Gfx::tLightEntity* mLightToControl;

		tGrowableArray<tProjectFile::tLenseFlare> mCachedProjectData;
		tFlareEditableData mEditingFlare;
		tEditablePropertyTable mEditingFlareProps;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnUserPropertyChanged;

		b32 mChanged;

		void fSetupUI( )
		{
			// type drop down
			{
				wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
				GetSizer( )->Add( hSizer );

				wxStaticText* staticT = new wxStaticText( this, wxID_ANY, "Lense Flare Style: " );
				staticT->SetForegroundColour( *wxWHITE );
				hSizer->Add( staticT );

				mType = new wxComboBox( this, wxID_ANY );
				mType->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(tLenseFlareEditPanel::fPopulateStyle), NULL, this );
				hSizer->Add( mType );

				wxButton* plusBut = new wxButton( this, wxID_ANY, "+" );
				plusBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tLenseFlareEditPanel::fAddStyle), NULL, this );
				hSizer->Add( plusBut );

				wxButton* copyBut = new wxButton( this, wxID_ANY, "Copy" );
				copyBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tLenseFlareEditPanel::fCopyStyle), NULL, this );
				hSizer->Add( copyBut );
			}

			wxStaticText* staticT = new wxStaticText( this, wxID_ANY, "Flare Element Properties: " );
			staticT->SetForegroundColour( *wxWHITE );
			GetSizer( )->Add( staticT );

			// flare list
			{
				wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
				GetSizer( )->Add( hSizer );

				mFlareList = new wxListBox( this, wxID_ANY );
				mFlareList->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(tLenseFlareEditPanel::fPopulateFlare), NULL, this );
				hSizer->Add( mFlareList, 2, wxEXPAND | wxVERTICAL );

				// buttons
				{
					wxSizer* vSizer = new wxBoxSizer( wxVERTICAL );
					hSizer->Add( vSizer );

					wxButton* plusBut = new wxButton( this, wxID_ANY, "+" );
					plusBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tLenseFlareEditPanel::fAddElement), NULL, this );
					vSizer->Add( plusBut );

					wxButton* clone = new wxButton( this, wxID_ANY, "Copy" );
					clone->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tLenseFlareEditPanel::fCloneElement), NULL, this );
					vSizer->Add( clone );

					wxButton* minusBut = new wxButton( this, wxID_ANY, "-" );
					minusBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tLenseFlareEditPanel::fRemoveElement), NULL, this );
					vSizer->Add( minusBut );
				}

				mPropertyPanel = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxSize( 400, 270 ), wxBORDER_SIMPLE | wxVSCROLL );
				hSizer->Add( mPropertyPanel );

				mOnUserPropertyChanged.fFromMethod< tLenseFlareEditPanel, &tLenseFlareEditPanel::fOnUserPropertyChanged >( this );
				mEditingFlareProps.mOnPropertyChanged.fAddObserver( &mOnUserPropertyChanged );		
			}

			// confirm buttons
			{
				wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
				GetSizer( )->Add( hSizer );

				wxButton* saveBut = new wxButton( this, wxID_ANY, "Save" );
				saveBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tLenseFlareEditPanel::fSave), NULL, this );
				hSizer->Add( saveBut );

				wxButton* cancelBut = new wxButton( this, wxID_ANY, "Cancel" );
				cancelBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tLenseFlareEditPanel::fCancel), NULL, this );
				hSizer->Add( cancelBut );
			}
		}

		void fOnUserPropertyChanged( tEditableProperty& property )
		{
			Gfx::tLenseFlareData::tFlare* flare = fEditingFlare( );

			if( flare )
			{
				mChanged = true;
				mEditingFlare.fToFlare( *flare );

				if( property.fGetName( ) == cPropertyPosition )
				{
					// Only refresh flare list when position changes.
					//  That's the only one that needs to re sort/build/display the list.
					fPopulateStyle( wxCommandEvent( ) );
				}
			}
		}

		tProjectFile::tLenseFlare* fEditingStyle( )
		{
			u32 typeSel = mType->GetCurrentSelection( );
				
			if( typeSel < fProjectFlares( ).fCount( ) )
			{
				tProjectFile::tLenseFlare& flare = fProjectFlares( )[ typeSel ];
				return &flare;
			}

			return NULL;
		}

		Gfx::tLenseFlareData::tFlare* fEditingFlare( )
		{
			tProjectFile::tLenseFlare* flare = fEditingStyle( );

			if( flare )
			{
				u32 selection = mFlareList->GetSelection( );
				if( selection < flare->mData.mFlares.fCount( ) )
				{
					Gfx::tLenseFlareData::tFlare& f = flare->mData.mFlares[ selection ];
					return &f;
				}
			}

			return NULL;
		}

		void fAddStyle( wxCommandEvent& )
		{
			wxString str = wxGetTextFromUser( "Enter Flare Style Name: ", "Add Flare Style.", "", this );
			std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
			if( result.length( ) > 0 )
			{
				mChanged = true;
				tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.fAddFlare( result );
				fPopulateStyles( );
				mType->Select( mType->GetCount( ) - 1 );
				fPopulateStyle( wxCommandEvent( ) );
			}
		}

		void fCopyStyle( wxCommandEvent& )
		{
			tProjectFile::tLenseFlare* selected = fEditingStyle( );
			if( selected )
			{
				wxString str = wxGetTextFromUser( "Enter Flare Style Name: ", "Copy Flare Style.", "", this );
				std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
				if( result.length( ) > 0 )
				{
					mChanged = true;
					tProjectFile::tLenseFlare* flare = tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.fAddFlare( result );
					flare->mData = selected->mData;

					fPopulateStyles( );
					mType->Select( mType->GetCount( ) - 1 );
					fPopulateStyle( wxCommandEvent( ) );
				}
			}
		}			

		void fAddElement( wxCommandEvent& )
		{
			tProjectFile::tLenseFlare* style = fEditingStyle( );
			if( style )
			{
				mChanged = true;
				style->mData.mFlares.fPushBack( Gfx::tLenseFlareData::tFlare( tFilePathPtr( "" ), -1.f, Math::tVec2f::cOnesVector, Math::tVec4f::cOnesVector ) );
				fPopulateStyle( wxCommandEvent( ) );
			}
		}

		void fRemoveElement( wxCommandEvent& )
		{
			tProjectFile::tLenseFlare* style = fEditingStyle( );
			if( style )
			{
				u32 selection = mFlareList->GetSelection( );
				if( selection < style->mData.mFlares.fCount( ) )
				{
					mChanged = true;
					style->mData.mFlares.fEraseOrdered( selection );
				}
				fPopulateStyle( wxCommandEvent( ) );
			}
		}

		void fCloneElement( wxCommandEvent& )
		{
			tProjectFile::tLenseFlare* style = fEditingStyle( );
			if( style )
			{
				u32 selection = mFlareList->GetSelection( );
				if( selection < style->mData.mFlares.fCount( ) )
				{
					mChanged = true;
					style->mData.mFlares.fPushBack( style->mData.mFlares[ selection ] );
				}
				fPopulateStyle( wxCommandEvent( ) );
			}
		}

		void fSave( wxCommandEvent& )
		{
			tProjectFile::fInstance( ).fSaveXml( ToolsPaths::fGetCurrentProjectFilePath( ), true );
			mChanged = false;
		}

		void fCancel( wxCommandEvent& )
		{
			tProjectFile::tLenseFlare* style = fEditingStyle( );
			if( style )
			{
				if ( wxMessageBox("Destroy all changes?",
					"Please confirm",
					wxICON_QUESTION | wxYES_NO) == wxYES )
				{
					tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mLenseFlares = mCachedProjectData;
					mChanged = false;
					fPopulateStyles( );
					mType->Select( 0 );
					fPopulateStyle( wxCommandEvent( ) );
				}
			}
		}
	};

	tSigEdLenseFlareEdDialog* tSigEdLenseFlareEdDialog::gDialog = NULL;

	tSigEdLenseFlareEdDialog::tSigEdLenseFlareEdDialog( wxWindow* parent, Gfx::tDevicePtr device, tEditorActionStack& actionStack, const std::string& regKeyName )
		: tMatEdMainWindow( parent, actionStack, regKeyName, false, false, false, cRenderWidth, cRenderHeight )
		, mDefaultTextureBrowserPathSet( false )
	{
		gDialog = this;

		SetIcon( wxIcon( "appicon" ) );

		fSetupPreviewWindow( device );

		mPreviewBundle.fReset( new tMaterialPreviewBundle( fDevice( ), HlslGen::cVshMeshModel ) );
		fSetPreviewBundle( mPreviewBundle );
		fPreviewPanel( )->fSetCurrentPreviewGeometry( tMaterialPreviewBundle::cPreviewGeometryModeSphereAndShadowedPlane );

		fSetDefaultShader( );

		// disable real lighting.
		//fDefaultLight( )->fSetColor( Gfx::tLight::cColorTypeFront, Math::tVec4f::cZeroVector );
		//fDefaultLight( )->fSetColor( Gfx::tLight::cColorTypeBack, Math::tVec4f::cZeroVector );
		//fDefaultLight( )->fSetColor( Gfx::tLight::cColorTypeAmbient, Math::tVec4f::cZeroVector );
		//fDefaultLight( )->fSetColor( Gfx::tLight::cColorTypeRim, Math::tVec4f::cZeroVector );

		mLenseFlarePanel = new tLenseFlareEditPanel( this, mPreviewBundle->fLenseFlareLight( ) );
		GetSizer( )->Add( mLenseFlarePanel, 1, wxEXPAND | wxALL, 0 );
	}

	tSigEdLenseFlareEdDialog::~tSigEdLenseFlareEdDialog( )
	{
		gDialog = NULL;
	}

	b32 tSigEdLenseFlareEdDialog::fOnTick( )
	{
		//fAutoHandleTopMost( ( HWND )mSigEd->GetHWND( ) );

		if( fIsActive( ) )
		{
			const b32 o = tMatEdMainWindow::fOnTick( );
			//mSigEd->fSetDialogInputActive( );
			if( !mDefaultTextureBrowserPathSet )
				mDefaultTextureBrowserPathSet = fSetDefaultBrowseDirectory( tFilePathPtr( "tools/lighting/" ) );
		}

		return true;
	}

	void tSigEdLenseFlareEdDialog::fSetDefaultShader( )
	{
		// todo get this from the tools res folder.
		const tFilePathPtr cFileName( "shaders/standard/basic.derml" );
		fOnShaderSelected( cFileName );
	}

	void tSigEdLenseFlareEdDialog::fOnShaderSelected( const tFilePathPtr& shaderPath )
	{
		Derml::tFile dermlFile;
		dermlFile.fLoadXml( ToolsPaths::fMakeResAbsolute( shaderPath ) );

		Derml::tMtlFile mtlFile;
		mtlFile.fFromShaderFile( dermlFile, shaderPath );

		mPreviewBundle->fGenerateShaders( dermlFile, HlslGen::cToolTypeDefault );
		mPreviewBundle->fUpdateMaterial( mtlFile, *fTextureCache( ) );

		fFromDermlMtlFile( mtlFile );
	}

	void tSigEdLenseFlareEdDialog::fShow( b32 show )
	{
		if( gDialog )
		{
			gDialog->Show( (show != false) );
			gDialog->Layout( );
		}
	}

	//void tSigEdLenseFlareEdDialog::fAddProperty( tEditableProperty& prop, const tEditorSelectionList* selection )
	//{
	//	if( !gDialog )
	//		return;

	//	tSigEdLenseFlareEdDialog& diag = *gDialog;

	//	//// Find owner of property.
	//	//diag.mEditingEnt.fRelease( );

	//	//sigassert( selection );
	//	//for( u32 i = 0; i < selection->fCount( ); ++i )
	//	//{
	//	//	tEditableObject* obj = (*selection)[ i ]->fStaticCast<tEditableObject>( );

	//	//	if( obj->fGetEditableProperties( ).fContainsProperty( prop ) )
	//	//	{
	//	//		diag.mEditingEnt.fReset( obj );
	//	//		break;
	//	//	}
	//	//}

	//	//sigassert( diag.mEditingEnt );	


	//	//// Populate Dialog.
	//	//prop.fGetData( diag.mEditingData );
	//	//diag.mEditingProperty.fReset( &prop );

	//	//diag.fSetLightProbeProperties( diag.mEditingData.mUserOptions );

	//	//if( diag.mEditingData.fCubeMapFileName( ).length( ) )
	//	//	diag.fRefreshCubemapFile( );
	//	//else if( !diag.mEditingData.mComputed )
	//	//{
	//	//	// No data yet, so render some initial values.
	//	//	diag.fRender( );
	//	//	diag.mEditingData.mComputed = true;
	//	//	diag.fOnLightPropertyChanged( *cNullProperty );
	//	//}

	//	diag.Show( );
	//}

	//void tSigEdLenseFlareEdDialog::fRefreshProperty( tEditableProperty& prop )
	//{
	//	//TODO ?
	//}

	//void tSigEdLenseFlareEdDialog::fRemoveProperty( tEditableProperty& prop )
	//{
	//	if( !gDialog )
	//		return;

	//	tSigEdLenseFlareEdDialog& diag = *gDialog;
	//	//diag.fSetLightProbeProperties( tEditablePropertyTable( ) );

	//	diag.mEditingProperty.fRelease( );
	//	diag.Hide( );
	//}	

	void tSigEdLenseFlareEdDialog::fOnPropertyChanged( tEditableProperty& property )
	{
		tMatEdMainWindow::fOnUserPropertyChanged( property );
		
		//if( property.fGetName( ) == tEditableLightProbeData::fEditablePropCubeMapButtons( ) )
		//{
		//	tEditablePropertyButtons& prop = static_cast<tEditablePropertyButtons&>( property );
		//	std::string val;
		//	prop.fGetData( val );
		//	if( val == tEditableLightProbeData::cCommandRender )
		//		fRender( );
		//	else if( val == tEditableLightProbeData::cCommandSave )
		//		fSave( );
		//	else if( val == tEditableLightProbeData::cCommandLoad )
		//		fLoad( );
		//	else if( val == tEditableLightProbeData::cCommandRefresh )
		//		fRefreshCubemapFile( );
		//}
		//else if( property.fGetName( ) == tEditableLightProbeData::fEditablePropCubeMapFile( ) )
		//	fRefreshCubemapFile( );

		//fRefreshHarmonics( );

		sigassert( mEditingProperty );
		mEditingProperty->fSetData( mEditingData );
		
	}


	void tSigEdLenseFlareEdDialog::fOnClose( wxCloseEvent& event )
	{
		if( mLenseFlarePanel->fReadyToClose( ) )
			tMatEdMainWindow::fOnClose( event );
	}

}

