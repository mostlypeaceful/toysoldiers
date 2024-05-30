#include "ToolsGuiPch.hpp"
#include "tSigAnimNodeControlPanel.hpp"
#include "tSigAnimNodeCanvas.hpp"
#include "tSigAnimDialog.hpp"
#include "tWxSlapOnControl.hpp"
#include "FileSystem.hpp"
#include "tMotionMap.hpp"

namespace Sig
{
	namespace 
	{
		const u32 cPanelWidth = 280;
		const u32 cPanelBorder = 8;
	}

	tSigAnimNodeControlPanel::tSigAnimNodeControlPanel( tSigAnimDialog* parent, tSigAnimNodeCanvas* canvas )
		: wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxSize( cPanelWidth, wxDefaultSize.y ), wxTAB_TRAVERSAL | wxBORDER_SIMPLE | wxVSCROLL )
		, mCanvas( canvas )
		, mMainWindow( parent )
		, mHeaderText( 0 )
		, mEventList( 0 )
		, mPropertyPanel( 0 )
	{
		SetMinSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetMaxSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetBackgroundColour( wxColour( 0x33, 0x33, 0x33 ) );
		SetForegroundColour( wxColour( 0x11, 0xff, 0x11 ) );

		mOnPropertyChanged.fFromMethod< tSigAnimNodeControlPanel, &tSigAnimNodeControlPanel::fOnPropertyChanged >( this );
		mCommonProps.mOnPropertyChanged.fAddObserver( &mOnPropertyChanged );

		tWxSlapOnControl::fSetLabelWidth( 80 );
		tWxSlapOnControl::fSetControlWidth( 150 );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mHeaderText = new wxStaticText( this, wxID_ANY, "Selection: None" );
		GetSizer( )->AddSpacer( 8 );
		GetSizer( )->Add( mHeaderText, 0, wxLEFT, 8 );
		GetSizer( )->AddSpacer( 8 );

		// Property panel
		mPropertyPanel = new wxScrolledWindow( this );
		mPropertyPanel->SetBackgroundColour( wxColour( 0x55, 0x55, 0x55 ) );
		mPropertyPanel->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
		GetSizer( )->Add( mPropertyPanel, 1, wxEXPAND | wxALL, 0 );

		mSliders = new tSigAnimSlidersPanel( *this, parent );
		GetSizer( )->Add( mSliders, 2, wxEXPAND | wxALL, 0 );
	}

	void tSigAnimNodeControlPanel::fOnCanvasSelectionChanged( const tDAGNodeList& nodes, const tDAGNodeConnectionList& conns, const tDAGNodeOutputList& outputs )
	{
		Freeze( );
		mPropertyPanel->DestroyChildren( );
		mCommonProps.fClearGui( );
		mSelectedNode.fRelease( );

		if( nodes.fCount( ) == 0 )
			mHeaderText->SetLabel( "Selection: None" );
		else if( nodes.fCount( ) > 1 )
			mHeaderText->SetLabel( "Selection: Multiple" );
		else
		{
			tAnimBaseNode* node = dynamic_cast<tAnimBaseNode*>( nodes.fFront( ).fGetRawPtr( ) );
			if( node )
			{
				mSelectedNode.fReset( node );
				//mSelectedNode->fSetControlPanel( this );

				mHeaderText->SetLabel( "Selection: " + node->fName( ) );
				mPropertyPanel->Layout( );

				mCommonProps = node->fProps( );
				mCommonProps.fCollectCommonPropertiesForGui( node->fProps( ) );
				
				tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps, false );

				mPropertyPanel->SetScrollbars( 0, 20, 1, 50 );
			}
			else
			{
				mHeaderText->SetLabel( "Selection: Invalid" );
			}
		}

		mPropertyPanel->Layout( );

		mSliders->fBuildKnobs( *mCanvas );

		Thaw( );
	}

	void tSigAnimNodeControlPanel::fRefreshProperties( )
	{
		//if( mSelectedNode )
		//{
		//	Freeze( );
		//	mPropertyPanel->DestroyChildren( );
		//	mCommonProps.fClearGui( );

		//	mCommonProps = mSelectedNode->fAIProps( );
		//	mCommonProps.fCollectCommonPropertiesForGui( mSelectedNode->fAIProps( ) );
		//	tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps, false );

		//	mPropertyPanel->Layout( );
		//	Thaw( );
		//}
	}

	void tSigAnimNodeControlPanel::fOnPropertyChanged( tEditableProperty& property )
	{
		mCanvas->fEditorActions( ).fForceSetDirty( true );

		if( mSelectedNode )
		{
			mSelectedNode->fApplyPropertyValues( );
			mCanvas->Refresh( );
		}

		mSliders->fBuildKnobs( *mCanvas );
	}

	//////////////////////////////////////////////////////////////////////////////////

	class tSigAnimKnobBase : public wxWindow, public Anim::tBlendReference
	{
	public:
		tSigAnimKnobBase( const std::string& name )
			: mName( name )
		{ }

		virtual void fCreateUI( wxWindow& parent, wxSizer& sizer ) { }
		virtual void fSetTrack( Anim::tSigAnimMoMap::tBlendData* track ) { }
		// return true if this knob is suitable for the specified blend.
		virtual b32 fConsider( tAnimBlendNode& blend ) { return false; }

		const std::string& fName( ) const { return mName; }

		// From tBlendReference
		virtual void fAnimEnded( ) { }

	protected:
		std::string mName;
	};

	class tSigAnimSliderKnob : public tSigAnimKnobBase
	{
	public:
		tSigAnimSliderKnob( tAnimBlendNode& blend );
		
		virtual b32 fConsider( tAnimBlendNode& blend );
		virtual void fCreateUI( wxWindow& parent, wxSizer& sizer );
		virtual void fSetTrack( Anim::tSigAnimMoMap::tBlendData* track );

		void fScrollChanged( wxScrollEvent& event );

	private:
		f32 mMin;
		f32 mMax;

		f32 mCurrentValue;
		b32 mControlTimeScale;

		static const u32 cPrecision = 100;

		wxSlider* mSlider;
		Anim::tSigAnimMoMap::tBlendData* mTrack;
	};

	tSigAnimSliderKnob::tSigAnimSliderKnob( tAnimBlendNode& blend )
		: tSigAnimKnobBase( blend.fBlendName( ) )
		, mMin( fMin( blend.fACurve( ), blend.fBCurve( ) ) )
		, mMax( fMax( blend.fACurve( ), blend.fBCurve( ) ) )
		, mCurrentValue( fClamp( 0.f, mMin, mMax ) )
		, mControlTimeScale( blend.fUIOnlyLinkTimeScale( ) )
		, mSlider( NULL )
		, mTrack( NULL )
	{
	}

	b32 tSigAnimSliderKnob::fConsider( tAnimBlendNode& blend )
	{
		if( blend.fUIBehaviorDigital( ) )
			return false;

		sigassert( mName == blend.fBlendName( ) );
		mMin = fMin( mMin, blend.fACurve( ), blend.fBCurve( ) );
		mMax = fMax( mMax, blend.fACurve( ), blend.fBCurve( ) );
		mCurrentValue = fClamp( 0.f, mMin, mMax );
		mControlTimeScale = (mControlTimeScale || blend.fUIOnlyLinkTimeScale( ));

		return true;
	}

	void tSigAnimSliderKnob::fCreateUI( wxWindow& parent, wxSizer& sizer )
	{
		wxStaticText* header = new wxStaticText( &parent, wxID_ANY, mName, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER );
		header->SetForegroundColour( wxColour( 255, 255, 255 ) );

		sizer.Add( header, 0, wxEXPAND );

		mSlider = new wxSlider( &parent, wxID_ANY, 0, mMin * cPrecision, mMax * cPrecision );
		mSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( tSigAnimSliderKnob::fScrollChanged ), NULL, this );
		mSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( tSigAnimSliderKnob::fScrollChanged ), NULL, this );
		sizer.Add( mSlider, 0, wxEXPAND | wxHORIZONTAL );
		sizer.AddSpacer( 8 );
	}

	void tSigAnimSliderKnob::fSetTrack( Anim::tSigAnimMoMap::tBlendData* track ) 
	{ 
		fScrollChanged( wxScrollEvent( ) );
		mTrack = track;
		mTrack->fSetBlendControl( &mCurrentValue );

		if( mControlTimeScale )
			mTrack->fSetTimeScaleControl( &mCurrentValue );
	}

	void tSigAnimSliderKnob::fScrollChanged( wxScrollEvent& event )
	{
		mCurrentValue = mSlider->GetValue( ) / (f32)cPrecision;
	}



	class tSigAnimButtonKnob : public tSigAnimKnobBase
	{
	public:
		tSigAnimButtonKnob( tAnimBlendNode& blend );

		virtual b32 fConsider( tAnimBlendNode& blend );
		virtual void fCreateUI( wxWindow& parent, wxSizer& sizer );
		virtual void fSetTrack( Anim::tSigAnimMoMap::tBlendData* track );

		void fClicked( wxCommandEvent& event );

		virtual void fAnimEnded( );

	private:
		wxCheckBox* mButton;
		Anim::tSigAnimMoMap::tBlendData* mTrack;
	};

	tSigAnimButtonKnob::tSigAnimButtonKnob( tAnimBlendNode& blend )
		: tSigAnimKnobBase( blend.fBlendName( ) )
		, mButton( NULL )
		, mTrack( NULL )
	{
	}

	b32 tSigAnimButtonKnob::fConsider( tAnimBlendNode& blend )
	{
		return blend.fBehaviorDigital( );
	}

	void tSigAnimButtonKnob::fCreateUI( wxWindow& parent, wxSizer& sizer )
	{
		mButton = new wxCheckBox( &parent, wxID_ANY, mName );
		mButton->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( tSigAnimButtonKnob::fClicked ), NULL, this );
		mButton->SetForegroundColour( wxColour( 0xFF, 0xFF, 0xFF ) );
		sizer.Add( mButton, 0, wxEXPAND | wxHORIZONTAL );
		sizer.AddSpacer( 8 );
	}

	void tSigAnimButtonKnob::fSetTrack( Anim::tSigAnimMoMap::tBlendData* track ) 
	{ 
		mTrack = track;
	}

	void tSigAnimButtonKnob::fClicked( wxCommandEvent& event )
	{
		if( mTrack )
		{
			if( mButton->GetValue( ) )
				mTrack->fAddReference( this );
			else
				mTrack->fRemoveReference( this );
		}
	}

	void tSigAnimButtonKnob::fAnimEnded( )
	{
		mButton->SetValue( false );
		fClicked( wxCommandEvent( ) );
	}




	class tSigAnimContextKnob : public tSigAnimKnobBase
	{
	public:
		tSigAnimContextKnob( const tProjectFile::tGameEnumeratedType* type );

		virtual void fCreateUI( wxWindow& parent, wxSizer& sizer );

		void fClicked( wxCommandEvent& event );

		void fSetDataOut( Anim::tSigAnimMoMap::tContextData* dataOut ) { mDataOut = dataOut; }

	private:
		wxComboBox* mBox;
		const tProjectFile::tGameEnumeratedType* mType;
		Anim::tSigAnimMoMap::tContextData* mDataOut;
	};

	tSigAnimContextKnob::tSigAnimContextKnob( const tProjectFile::tGameEnumeratedType* type )
		: tSigAnimKnobBase( type->mName )
		, mType( type )
		, mBox( NULL )
	{
	}

	void tSigAnimContextKnob::fCreateUI( wxWindow& parent, wxSizer& sizer )
	{
		mBox = new wxComboBox( &parent, wxID_ANY );
		mBox->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( tSigAnimContextKnob::fClicked ), NULL, this );
		sizer.Add( mBox, 0, wxEXPAND | wxHORIZONTAL );
		sizer.AddSpacer( 8 );

		sigassert( mType );
		for( u32 i = 0; i < mType->mValues.fCount( ); ++i )
			mBox->Append( mType->mValues[ i ].mName );
	}

	void tSigAnimContextKnob::fClicked( wxCommandEvent& event )
	{
		if( mDataOut )
		{
			mDataOut->fSetCurrentValue( mBox->GetSelection( ) );
		}
	}

	//////////////////////////////////////////////////////////////////////////////////

	tSigAnimSlidersPanel::tSigAnimSlidersPanel( wxWindow& parent, tSigAnimDialog* mainWindow )
		: wxScrolledWindow( &parent, wxID_ANY, wxDefaultPosition, wxSize( cPanelWidth, wxDefaultSize.y ), wxTAB_TRAVERSAL | wxBORDER_SIMPLE | wxVSCROLL )
		, mMainWindow( mainWindow )
	{

		SetScrollbars( 0, 20, 1, 50 );
	}

	tSigAnimSlidersPanel::~tSigAnimSlidersPanel( )
	{
	}

	void tSigAnimSlidersPanel::fClear( )
	{
		DestroyChildren( );
		mKnobs.fSetCount( 0 );
		mContexts.fSetCount( 0 );
	}

	namespace 
	{
		b32 fBlendNodeSort( const tAnimBlendNode* left, const tAnimBlendNode* right )
		{
			if( left->fUIBehaviorDigital( ) != right->fUIBehaviorDigital( ) )
			{
				return right->fUIBehaviorDigital( );
			}
			else
				return (strcmp( left->fBlendName( ).c_str( ), right->fBlendName( ).c_str( ) ) < 0);
		}
	}

	void tSigAnimSlidersPanel::fBuildKnobs( tSigAnimNodeCanvas& canvas )
	{
		fClear( );

		tGrowableArray< tAnimBlendNode* > blendNodes;
		for( u32 i = 0; i < canvas.fAllNodes( ).fCount( ); ++i )
		{
			tAnimBlendNode* node = dynamic_cast<tAnimBlendNode*>( canvas.fAllNodes( )[ i ].fGetRawPtr( ) );
			if( node )
				blendNodes.fPushBack( node );
		}

		std::sort( blendNodes.fBegin( ), blendNodes.fEnd( ), fBlendNodeSort );

		for( u32 i = 0; i < blendNodes.fCount( ); ++i )
		{
			tAnimBlendNode* bN = blendNodes[ i ];

			tSigAnimKnobBase* knob = NULL;
			for( u32 s = 0; s < mKnobs.fCount( ); ++s )
			{
				if( mKnobs[ s ]->fName( ) == bN->fBlendName( ) )
				{
					knob = mKnobs[ s ];
					break;
				}
			}

			if( !knob || !knob->fConsider( *bN ) )
			{
				if( bN->fUIBehaviorDigital( ) )
					knob = new tSigAnimButtonKnob( *bN );
				else
					knob = new tSigAnimSliderKnob( *bN );

				mKnobs.fPushBack( knob );
			}
		}

		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		wxStaticText* header = new wxStaticText( this, wxID_ANY, "Blend Preview Control Panel:" );
		header->SetForegroundColour( wxColour( 255, 255, 255 ) );
		GetSizer( )->Add( header, 0, wxEXPAND, 4 );

		wxButton* resetButton = new wxButton( this, wxID_ANY, "Reset" );
		resetButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigAnimSlidersPanel::fOnResetClicked ), NULL, this );
		GetSizer( )->Add( resetButton, 0, 0, 4 );

		for( u32 i = 0; i < mKnobs.fCount( ); ++i )
			mKnobs[ i ]->fCreateUI( *this, *GetSizer( ) );

		Layout( );
	}

	void tSigAnimSlidersPanel::fOnResetClicked( wxCommandEvent& )
	{
		mMomap.fReset( mMainWindow->fMakeMoMap( ) );	

		for( u32 i = 0; i < mKnobs.fCount( ); ++i )
		{
			mKnobs[ i ]->fSetTrack( mMomap ? mMomap->fFindBlendData( tStringPtr( mKnobs[ i ]->fName( ) ) ) : NULL );
		}

		if( !mContexts.fCount( ) && mMomap )
		{
			for( u32 i = 0; i < mMomap->fContextData( ).fCount( ); ++i )
			{
				u32 enumTypeValue = mMomap->fContextData( )[ i ].fEnumTypeValue( );
				const tProjectFile::tGameEnumeratedType* enumType = tProjectFile::fInstance( ).fFindEnumeratedTypeByKey( enumTypeValue );
				
				if( enumType )
				{
					tSigAnimContextKnob* knob = NEW tSigAnimContextKnob( enumType );
					knob->fSetDataOut( mMomap->fFindContextData( enumTypeValue ) );
					knob->fCreateUI( *this, *GetSizer( ) );
					mContexts.fPushBack( knob );
				}
				else
				{
					wxMessageBox( wxString( "Could not find enum type with key: " ) + enumTypeValue );
				}
			}

			Layout( );
		}
	}

}
