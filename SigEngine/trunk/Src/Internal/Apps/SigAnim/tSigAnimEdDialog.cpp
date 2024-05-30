#include "SigAnimPch.hpp"
#include "tSigAnimEdDialog.hpp"
#include "tWxSlapOnTextBox.hpp"
#include "tWxSlapOnChoice.hpp"
#include "tSigAnimMainWindow.hpp"
#include "tProjectFile.hpp"

namespace Sig
{
	namespace
	{
		static const wxColour cBackColor = wxColour( 0x33, 0x33, 0x44 );
		static const wxColour cTextColor = wxColour( 0xff, 0xff, 0xaa );
		static const wxColour cGroupTitleColour = wxColour( 0xff, 0xff, 0xff );

		enum
		{
			cActionSaveAnimPacks = 1,
		};
	}

	// Duplicated in tSigAnimTimeline.cpp
	std::string fKeyframeEventStringFromKey( u32 key )
	{
		std::string eventType = "Error";
		const tProjectFile::tEvent* event = tProjectFile::fInstance( ).fFindKeyframeEventByKey( key );
		if( event )
			eventType = event->mName;
		else
			log_warning( "Could not find key frame event. key: " << key );
		return eventType;
	}


	tSigAnimEdDialog::tSigAnimEdDialog( tOwner & owner, wxWindow * parent )
		: tWxSlapOnDialog( "AnimEd", parent, ToolsPaths::fGetSignalRegistryKeyName( ) + "SigAnim\\AnimEdDialog" )
		, mOwner( owner )
		, mMainPanel( 0 )
		, mCurrentAnimPackText( 0 )
		, mCurrentAnimText( 0 )
		, mCurrentAnimFlagsText( 0 )
		, mCurrentAnimLengthText( 0 )
		, mDisableCompression( 0 )
		, mCompressionErrorP( 0 )
		, mCompressionErrorR( 0 )
		, mCompressionErrorS( 0 )
		, mEventTagListBox( 0 )
		, mAnimMetaData( 0 )
	{
		SetIcon( wxIcon( "appicon" ) );

		const int width = 390;
		const int height = 560; // wxDefaultSize.y
		SetSize( wxSize( width, height ) );
		SetMinSize( wxSize( width, height ) );
		SetMaxSize( wxSize( width, height ) );

		SetBackgroundColour( cBackColor );
		SetForegroundColour( cTextColor );

		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tSigAnimEdDialog::fOnAction ) );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		wxToolBar* toolBar = new wxToolBar( this, wxID_ANY );
		toolBar->AddTool( cActionSaveAnimPacks, "Save Anim Packs", wxBitmap( "saveanipks" ), wxNullBitmap, wxITEM_NORMAL, "Save All Loaded Anim Packs" );
		toolBar->Realize( );

		mMainPanel = new wxScrolledWindow( this );
		GetSizer( )->Add( toolBar, 0, wxEXPAND | wxALL, 0 );
		GetSizer( )->Add( mMainPanel, 1, wxEXPAND | wxALL, 0 );
		fBuildGui( );
		fClearAnim( );
	}

	void tSigAnimEdDialog::fClearAnim( )
	{
		mCurrentAnimPackText->SetLabel( "[null]" );
		mCurrentAnimText->SetLabel( "[null]" );
		mCurrentAnimFlagsText->SetLabel( "None" );
		mCurrentAnimLengthText->SetLabel( "0" );
		mAnimMetaData = 0;
		mDisableCompression->fSetDataPtr( 0 );
		mDisableCompression->fSetValue( tWxSlapOnCheckBox::cFalse );
		mDisableCompression->fDisableControl( );
		mCompressionErrorP->fSetDataPtr( 0 );
		mCompressionErrorP->fSetValue( 1.f/100.f );
		mCompressionErrorP->fDisableControl( );
		mCompressionErrorR->fSetDataPtr( 0 );
		mCompressionErrorR->fSetValue( 1.f/1000.f );
		mCompressionErrorR->fDisableControl( );
		mCompressionErrorS->fSetDataPtr( 0 );
		mCompressionErrorS->fSetValue( 1.f/100.f );
		mCompressionErrorS->fDisableControl( );
		mEventTagListBox->Clear( );
	}

	void tSigAnimEdDialog::fSetAnim( const std::string& animPkLabel, const std::string& animName, Anipk::tFile& anipkFile, const tKeyFrameAnimation* kfAnim )
	{
		mCurrentAnimPackText->SetLabel( animPkLabel );
		mCurrentAnimText->SetLabel( animName );
		std::string flags;
		if( kfAnim->fPartial( ) )
			if( kfAnim->fContainsRefFrame( ) )	flags = "PARTIAL | REFFRAME";
			else								flags = "PARTIAL";
		else									flags = "COMPLETE";
		mCurrentAnimFlagsText->SetLabel( flags );
		wxString length;
		length.Printf( "%f", kfAnim->fLength( true ) );
		mCurrentAnimLengthText->SetLabel( length );
		mAnimMetaData = &anipkFile.fFindOrAddAnimMetaData( animName );

		mDisableCompression->fEnableControl( );
		mDisableCompression->fSetDataPtr( &mAnimMetaData->mDisableCompression );
		mCompressionErrorP->fEnableControl( );
		mCompressionErrorP->fSetDataPtr( &mAnimMetaData->mCompressionErrorP );
		mCompressionErrorR->fEnableControl( );
		mCompressionErrorR->fSetDataPtr( &mAnimMetaData->mCompressionErrorR );
		mCompressionErrorS->fEnableControl( );
		mCompressionErrorS->fSetDataPtr( &mAnimMetaData->mCompressionErrorS );

		fSyncEventListBoxFromMetaData( );
	}

	void tSigAnimEdDialog::fOnAnimValueModified( )
	{
		mOwner.fMarkCurrentAnimPackDirty( );
		SetTitle( "AnimEd*" );
	}

	void tSigAnimEdDialog::fClearDirty( )
	{
		SetTitle( "AnimEd" );
	}

	void tSigAnimEdDialog::fOnTick( )
	{
		if( mAnimMetaData && mAnimMetaData->fDirty() )
		{
			fSyncEventListBoxFromMetaData( );
			fOnAnimValueModified( );
			mAnimMetaData->fClearDirty( );
		}
	}

	namespace
	{
		wxStaticText* fAddStaticText( wxWindow* group, s32 leftMargin, const char* textTitle, const char* defaultText )
		{
			wxBoxSizer* wSizer = new wxBoxSizer( wxHORIZONTAL );
			group->GetSizer( )->Add( wSizer, 0, wxEXPAND | wxALL, 0 );

			wSizer->AddSpacer( leftMargin );

			wxStaticText* title = new wxStaticText( group, wxID_ANY, textTitle );
			wSizer->Add( title, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxLEFT | wxTOP, 5 );

			wxStaticText* text = new wxStaticText( group, wxID_ANY, defaultText );
			wSizer->Add( text, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxTOP, 5 );
			return text;
		}
	}

	void tSigAnimEdDialog::fOnAction( wxCommandEvent& event )
	{
		switch( event.GetId( ) )
		{
		case cActionSaveAnimPacks:
			mOwner.fSaveAllAnimPacks( );
			break;
		default:
			event.Skip( );
			break;
		}
	}

	namespace
	{
		class tCreateKeyframeEventDialog : public wxDialog
		{
			tWxSlapOnSpinner* mTime;
			tWxSlapOnTextBox* mText;
			tWxSlapOnChoice*  mEventType;
		public:
			tCreateKeyframeEventDialog( wxWindow* parent, f32 maxTime, f32 defaultTime, const std::string& type, const std::string& defaultTag )
				: wxDialog( parent, wxID_ANY, "Create Keyframe Event", wxDefaultPosition, wxSize( 380, 150 ) )
				, mTime( 0 )
				, mText( 0 )
				, mEventType( 0 )
			{
				SetSizer( new wxBoxSizer( wxVERTICAL ) );
				GetSizer( )->AddSpacer( 8 );

				mTime = new tWxSlapOnSpinner( this, "Time", 0.f, maxTime, 0.0001f, 4 );
				mTime->fSetValueNoEvent( defaultTime );

				u32 typeIndex = 0;
				const tProjectFile& pf = tProjectFile::fInstance( );
				tDynamicArray<wxString> eventNames( pf.mKeyFrameEvents.fCount( ) );
				for( u32 i = 0; i < pf.mKeyFrameEvents.fCount( ); ++i )
				{
					eventNames[ i ] = pf.mKeyFrameEvents[ i ].mName;
					if( eventNames[ i ] == type ) typeIndex = i;
				}

				mEventType = new tWxSlapOnChoice( this, "Type", eventNames.fBegin( ), pf.mKeyFrameEvents.fCount( ) );
				mEventType->fSetValue( typeIndex );

				mText = new tWxSlapOnTextBox( this, "Tag" );
				mText->fSetValue( defaultTag );

				GetSizer( )->AddSpacer( 8 );
				GetSizer( )->Add( CreateStdDialogButtonSizer( wxOK | wxCANCEL ), 1, wxEXPAND | wxALL, 5 );

				// This dialog box is explicitly sized.
				//  If you add any more controls you will need to adjust the dialog size in tCreateKeyframeEventDialog( )

				Layout( );
				Center( );
			}
			f32 fGetTime( ) const { return mTime->fGetValue( ); }
			std::string fGetText( ) const { return mText->fGetValue( ); }
			std::string fGetEventType( ) const
			{
				return std::string( mEventType->fGetValueString( ) );
			}
		};
	}

	void tSigAnimEdDialog::fOnAddEventTagPressed( wxCommandEvent& )
	{
		f64 length = 0.f;
		mCurrentAnimLengthText->GetLabel( ).ToDouble( &length );

		const std::string currentAnimName = std::string( mCurrentAnimText->GetLabel( ).c_str( ) );

		tCreateKeyframeEventDialog dlg( this, ( f32 )length, mOwner.fGetAnimTime( currentAnimName ), "", "" );
		if( dlg.ShowModal( ) == wxID_OK )
		{
			fAddKeyframeEventToListBox( dlg.fGetTime( ), dlg.fGetEventType( ), dlg.fGetText( ) );
			fSyncCurrentAnimToEventListBox( );
			fOnAnimValueModified( );
		}
	}

	void tSigAnimEdDialog::fOnSubEventTagPressed( wxCommandEvent& )
	{
		const s32 sel = mEventTagListBox->GetSelection( );
		if( sel >= 0 )
		{
			mEventTagListBox->Delete( sel );
			fSyncCurrentAnimToEventListBox( );
			fOnAnimValueModified( );
		}
		const s32 numEntries = mEventTagListBox->GetCount( );
		if( numEntries > 0 )
			mEventTagListBox->Select( fMax<int>( 0, fMin<int>( numEntries-1, sel ) ) );
	}

	void tSigAnimEdDialog::fOnModifyEventTag( wxCommandEvent& )
	{
		const s32 sel = mEventTagListBox->GetSelection( );
		if( sel < 0 )
			return;

		f64 length = 0.f;
		mCurrentAnimLengthText->GetLabel( ).ToDouble( &length );

		Anipk::tKeyFrameTag kft;
		fExtractKeyFrameTag( sel, kft );

		tCreateKeyframeEventDialog dlg( this, ( f32 )length, kft.mTime, fKeyframeEventStringFromKey( kft.mEventTypeKey ), kft.mTag );
		if( dlg.ShowModal( ) == wxID_OK )
		{
			mEventTagListBox->Delete( sel );
			fAddKeyframeEventToListBox( dlg.fGetTime( ), dlg.fGetEventType( ), dlg.fGetText( ) );
			fSyncCurrentAnimToEventListBox( );
			fOnAnimValueModified( );
		}
	}

	void tSigAnimEdDialog::fAddKeyframeEventToListBox( f32 time, const std::string& type, const std::string& tag )
	{
		std::stringstream entryText;
		entryText << std::setprecision( 4 ) << std::fixed << time << ": " << type << ": " << ( tag.length( ) ? tag : "NO_TAG" );
		mEventTagListBox->Append( entryText.str( ) );
	}

	b32 tSigAnimEdDialog::fExtractKeyFrameTag( s32 i, Anipk::tKeyFrameTag& kft ) const
	{
		tGrowableArray<std::string> frags;
		StringUtil::fSplit( frags, mEventTagListBox->GetString( i ).c_str( ), ": ", true );

		if( frags.fCount( ) != 3 )
		{
			log_warning( "Wrong number of frags in fExtractKeyFrameTag" );
			return false;
		}


		const tProjectFile::tEvent* event = tProjectFile::fInstance( ).fFindKeyframeEventByName( frags[ 1 ] );
		if( !event )
		{
			log_warning( "could not find event in fExtractKeyFrameTag, name: " << frags[ 1 ] );
			return false;
		}

		kft.mTime = ( f32 )atof( frags[ 0 ].c_str( ) );
		kft.mEventTypeKey = event->mKey;
		kft.mTag = frags[ 2 ] == "NO_TAG" ? "" : frags[ 2 ];

		return true;
	}

	void tSigAnimEdDialog::fSyncCurrentAnimToEventListBox( )
	{
		if( !mAnimMetaData )
			return;
		mAnimMetaData->mKeyFrameTags.fSetCount( 0 );

		const s32 numEntries = mEventTagListBox->GetCount( );
		if( numEntries <= 0 )
			return;

		for( s32 i = 0; i < numEntries; ++i )
		{
			Anipk::tKeyFrameTag kft;
			if( fExtractKeyFrameTag( i, kft ) )
				mAnimMetaData->mKeyFrameTags.fPushBack( kft );
		}
	}

	void tSigAnimEdDialog::fSyncEventListBoxFromMetaData( )
	{
		mEventTagListBox->Clear( );
		for( u32 i = 0; i < mAnimMetaData->mKeyFrameTags.fCount( ); ++i )
			fAddKeyframeEventToListBox( mAnimMetaData->mKeyFrameTags[ i ].mTime, fKeyframeEventStringFromKey( mAnimMetaData->mKeyFrameTags[ i ].mEventTypeKey ), mAnimMetaData->mKeyFrameTags[ i ].mTag );
	}

	void tSigAnimEdDialog::fBuildGui( )
	{
		sigassert( mMainPanel );
		mMainPanel->SetScrollbars( 0, 20, 0, 0 );
		mMainPanel->SetBackgroundColour( cBackColor );
		mMainPanel->SetForegroundColour( cTextColor );

		mMainPanel->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		mMainPanel->GetSizer( )->AddSpacer( 5 );

		// add group for anim stats/info
		{
			wxBoxSizer* wSizer = new wxBoxSizer( wxHORIZONTAL );
			mMainPanel->GetSizer( )->Add( wSizer, 0, wxEXPAND | wxALL, 0 );

			wxPanel* group = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
			group->SetForegroundColour( cTextColor );
			wSizer->Add( group, 1, wxEXPAND | wxALL, 5 );
			group->SetSizer( new wxBoxSizer( wxVERTICAL ) );

			{
				wxBoxSizer* wSizer = new wxBoxSizer( wxHORIZONTAL );
				group->GetSizer( )->Add( wSizer, 0, wxEXPAND | wxALL, 0 );

				wxStaticText* animInfo = new wxStaticText( group, wxID_ANY, "Animation Info" );
				animInfo->SetForegroundColour( cGroupTitleColour );
				wSizer->Add( animInfo, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxLEFT | wxTOP, 5 );
			}

			const s32 leftMargin = 20;
			mCurrentAnimPackText = fAddStaticText( group, leftMargin, "Pack: ", "[null]" );
			mCurrentAnimText = fAddStaticText( group, leftMargin, "Name: ", "[null]" );
			mCurrentAnimFlagsText = fAddStaticText( group, leftMargin, "Flags: ", "None" );
			mCurrentAnimLengthText = fAddStaticText( group, leftMargin, "Length: ", "0" );

			group->GetSizer( )->AddSpacer( 10 );
		}

		// add group for compression options
		{
			wxBoxSizer* wSizer = new wxBoxSizer( wxHORIZONTAL );
			mMainPanel->GetSizer( )->Add( wSizer, 0, wxEXPAND | wxALL, 0 );

			wxPanel* group = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
			group->SetForegroundColour( cTextColor );
			wSizer->Add( group, 1, wxEXPAND | wxALL, 5 );
			group->SetSizer( new wxBoxSizer( wxVERTICAL ) );

			{
				wxBoxSizer* wSizer = new wxBoxSizer( wxHORIZONTAL );
				group->GetSizer( )->Add( wSizer, 0, wxEXPAND | wxALL, 0 );

				wxStaticText* options = new wxStaticText( group, wxID_ANY, "Compression Error Tolerances" );
				options->SetForegroundColour( cGroupTitleColour );
				wSizer->Add( options, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxLEFT | wxTOP, 5 );
			}

			mDisableCompression = new tCheckBox( *this, group, "Disable All" );
			mDisableCompression->fSetValue( tWxSlapOnCheckBox::cTrue );
			mCompressionErrorP = new tSpinner( *this, group, "Translation", 0.f, 1.f, 0.00001f, 5 );
			mCompressionErrorP->fSetValue( 0.001f );
			mCompressionErrorR = new tSpinner( *this, group, "Rotation", 0.f, 1.f, 0.00001f, 5 );
			mCompressionErrorR->fSetValue( 0.001f );
			mCompressionErrorS = new tSpinner( *this, group, "Scale", 0.f, 1.f, 0.00001f, 5 );
			mCompressionErrorS->fSetValue( 0.001f );

			group->GetSizer( )->AddSpacer( 10 );
		}

		// add group for keyframe event tags
		{
			wxBoxSizer* wSizer = new wxBoxSizer( wxHORIZONTAL );
			mMainPanel->GetSizer( )->Add( wSizer, 0, wxEXPAND | wxALL, 0 );

			wxPanel* group = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
			group->SetForegroundColour( cTextColor );
			wSizer->Add( group, 1, wxEXPAND | wxALL, 5 );
			group->SetSizer( new wxBoxSizer( wxVERTICAL) );

			{
				wxBoxSizer* wSizer = new wxBoxSizer( wxHORIZONTAL );
				group->GetSizer( )->Add( wSizer, 0, wxEXPAND | wxALL, 0 );

				wxStaticText* animPacksTitle = new wxStaticText( group, wxID_ANY, "Keyframe Event Tags" );
				animPacksTitle->SetForegroundColour( cGroupTitleColour );
				wSizer->Add( animPacksTitle, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5 );

				wxButton* addEventTag = new wxButton( group, wxID_ANY, "+", wxDefaultPosition, wxSize( 20, 20 ) );
				wxButton* subEventTag = new wxButton( group, wxID_ANY, "-", wxDefaultPosition, wxSize( 20, 20 ) );
				wSizer->AddStretchSpacer( 1 );
				wSizer->Add( addEventTag, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT | wxTOP, 5 );
				wSizer->Add( subEventTag, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT | wxTOP, 5 );

				addEventTag->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigAnimEdDialog::fOnAddEventTagPressed ), NULL, this );
				subEventTag->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigAnimEdDialog::fOnSubEventTagPressed ), NULL, this );
			}

			mEventTagListBox = new wxListBox( group, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.x, 180 ), wxArrayString(), wxLB_SINGLE | wxLB_SORT );
			mEventTagListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( tSigAnimEdDialog::fOnModifyEventTag ), NULL, this );
			group->GetSizer( )->Add( mEventTagListBox, 1, wxEXPAND | wxALL, 5 );
		}
	}

}

