#include "SigScriptPch.hpp"
#include "tLocmlControlPanel.hpp"
#include "tStrongPtr.hpp"
#include "Locml.hpp"
#include "tScriptNotebook.hpp"
#include "tWxTextEditor.hpp"
#include <wx/progdlg.h>

namespace Sig
{
	tLocmlControlPanel::tLocmlControlPanel( wxWindow* parent,  tScriptNotebook* notebook )
		: wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL )
		, mNotebook( notebook )
	{
		SetBackgroundColour( wxColour( 0xee, 0xee, 0xee ) );
		SetForegroundColour( wxColour( 0x00, 0x00, 0x00 ) );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mLangTargets.fSetCount( tLocmlConvertDictionary::cNumLangTargets );
		mLangTargets[ tLocmlConvertDictionary::cEN ] = new tLocmlTargetUI( this, wxT("en"), tFilePathPtr("Loc\\en\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cEN ]->fSetSource();
		mLangTargets[ tLocmlConvertDictionary::cBR ] = new tLocmlTargetUI( this, wxT("br"), tFilePathPtr("Loc\\br\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cCHS ] = new tLocmlTargetUI( this, wxT("chs"), tFilePathPtr("Loc\\chs\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cDE ] = new tLocmlTargetUI( this, wxT("de"), tFilePathPtr("Loc\\de\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cES ] = new tLocmlTargetUI( this, wxT("es"), tFilePathPtr("Loc\\es\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cFR ] = new tLocmlTargetUI( this, wxT("fr"), tFilePathPtr("Loc\\fr\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cIT ] = new tLocmlTargetUI( this, wxT("it"), tFilePathPtr("Loc\\it\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cJA ] = new tLocmlTargetUI( this, wxT("ja"), tFilePathPtr("Loc\\ja\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cKO ] = new tLocmlTargetUI( this, wxT("ko"), tFilePathPtr("Loc\\ko\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cRU ] = new tLocmlTargetUI( this, wxT("ru"), tFilePathPtr("Loc\\ru\\text.locml") );
		mLangTargets[ tLocmlConvertDictionary::cZH ] = new tLocmlTargetUI( this, wxT("zh"), tFilePathPtr("Loc\\zh\\text.locml") );

		wxButton* pushChanges = new wxButton( this, wxID_ANY, wxT("Push Changes") );
		pushChanges->SetToolTip( "Copies the English file's formatting changes\n to all selected non-English locmls." );
		pushChanges->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tLocmlControlPanel::fOnPush ), NULL, this );
		GetSizer()->Add( pushChanges, 0, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 4 );

		//wxButton* findErrors = new wxButton( this, wxID_ANY, wxT("Find Errors") );
		//findErrors->SetToolTip( "Displays any discrepancies in the gathered localization data." );
		//findErrors->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tLocmlControlPanel::fOnFindErrors ), NULL, this );
		//GetSizer()->Add( findErrors, 0, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 4 );

		wxButton* rebuildDictionary = new wxButton( this, wxID_ANY, wxT("Refresh Localization Dictionary") );
		rebuildDictionary->SetToolTip( "Rebuilds the data used to push changes\n to non-English files. This will only parse SAVED\n changes to locml files." );
		rebuildDictionary->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tLocmlControlPanel::fOnRebuild ), NULL, this );
		GetSizer()->Add( rebuildDictionary, 0, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, 4 );
	}

	tLocmlControlPanel::~tLocmlControlPanel()
	{
		// wxEvtHandler isn't auto-deleted.
		for( u32 i = 0; i < mLangTargets.fCount(); ++i )
			delete mLangTargets[i];

		mLangTargets.fDeleteArray();
	}

	void tLocmlControlPanel::fBuildDict( )
	{
		// Must clear out any previous data to prevent conflicts and duplicates.
		if( mDict.fBuilt() )
			mDict.fClear();

		wxProgressDialog progress( wxString( "Extracting Localization Data" ), wxEmptyString, mLangTargets.fCount(), NULL, wxDEFAULT_DIALOG_STYLE | wxPD_AUTO_HIDE | wxSTAY_ON_TOP | wxPD_CAN_ABORT );
		progress.SetSize( 300, progress.GetSize( ).y );

		b32 proceed = progress.Update( 0, "" );

		for( u32 i = 0; i < mLangTargets.fCount() && proceed; ++i )
		{
			proceed = progress.Update( i, wxString( "Extracting [" + mLangTargets[i]->fGetLang() + "]..." ) );

			Locml::tFile targ;
			targ.fLoadXml( mLangTargets[i]->fGetFullPath() );
			mDict.fExtractFromLocml( mLangTargets[i]->fGetLang(), targ );

			proceed = progress.Update( i+1, wxString( "Extracting [" + mLangTargets[i]->fGetLang() + "]..." ) );
		}

		// Discard everything if we didn't fully succeed.
		if( !proceed )
			mDict.fClear();
	}

	void tLocmlControlPanel::fOnPush( wxCommandEvent& event )
	{
		//const Time::tStamp topStart = Time::fGetStamp();

		// Ensure dictionary is built at least once
		if( !mDict.fBuilt() )
			fBuildDict();

		// This checks if the build was canceled.
		if( !mDict.fBuilt() )
			return;

		//const f32 buildTime = Time::fGetElapsedMs( topStart, Time::fGetStamp() );
		//log_warning( "Build Time ms: " << buildTime );

		// Get the EN notebook page.
		const u32 engIdx = mNotebook->fOpenOrFind( ToolsPaths::fMakeResAbsolute( mLangTargets[ tLocmlConvertDictionary::cEN ]->fGetFullPath() ) );
		if( engIdx == -1 )
		{
			log_warning( "Something went really wrong trying to fOnPush some locmls" );
			return;
		}

		tScriptNotebookPage* enPage = mNotebook->fGetPage( engIdx );
		enPage->fGetEditor()->SelectAll();
		wxString enDocument = enPage->fGetEditor()->GetSelectedText();

		wxProgressDialog progress( wxString( "Formatting Target Files" ), wxEmptyString, mLangTargets.fCount(), NULL, wxDEFAULT_DIALOG_STYLE | wxPD_AUTO_HIDE | wxSTAY_ON_TOP | wxPD_CAN_ABORT );
		progress.SetSize( 300, progress.GetSize( ).y );

		b32 proceed = progress.Update( 0, "" );

		// Push changes to each target file.
		for( u32 i = tLocmlConvertDictionary::cEN+1; i < mLangTargets.fCount() && proceed; ++i )
		{
			proceed = progress.Update( i, wxString( "Formatting [" + mLangTargets[i]->fGetLang() + "]..." ) );

			if( !mLangTargets[i]->fActive() )
				continue;

			// Get target script notebook page.
			const u32 idx = mNotebook->fOpenOrFind( mLangTargets[i]->fGetFullPath() );
			if( idx == -1 )
			{
				log_warning( "Something went really wrong trying to fOnPush some locmls" );
				return;
			}

			// Make sure the English page is show, this speeds up the editing considerably.
			mNotebook->SetSelection( engIdx );

			tScriptNotebookPage* targPage = mNotebook->fGetPage( idx );

			targPage->fGetEditor()->BeginUndoAction();

				// Paste formatting from EN.
				targPage->fGetEditor()->SetText( enDocument );

				//const Time::tStamp convertingStamp = Time::fGetStamp();

				// Replace all the things.
				targPage->fConvertLocmlText( mDict, mLangTargets[i]->fGetLang() );

				//const f32 convertingTime = Time::fGetElapsedMs( convertingStamp, Time::fGetStamp() );
				//log_warning( "Conv " << mLangTargets[i]->fGetLang() << " Time ms: " << convertingTime );

			targPage->fGetEditor()->EndUndoAction();

			proceed = progress.Update( i+1, wxString( "Formatting [" + mLangTargets[i]->fGetLang() + "]..." ) );
		}

		//const f32 totalTime = Time::fGetElapsedMs( topStart, Time::fGetStamp() );
		//log_warning( "Total Time ms: " << totalTime );
	}

	void tLocmlControlPanel::fOnFindErrors( wxCommandEvent& event )
	{
		// TODO: Missing keys
		// TODO: Records where entries != cNumTargets+1
	}

	void tLocmlControlPanel::fOnRebuild( wxCommandEvent& event )
	{
		fBuildDict();
	}

	tLocmlControlPanel::tLocmlTargetUI::tLocmlTargetUI( wxWindow* parent, wxString lang, tFilePathPtr defaultPath )
		: mParent( parent )
		, mActive( NULL )
		, mFilePath( NULL )
		, mLang( lang )
	{
		// Builds a horizontal sizer for itself and inserts into any parent's existing sizer.
		wxBoxSizer* mySizer = new wxBoxSizer( wxHORIZONTAL );

		// Checkbox
		mActive = new wxCheckBox( parent, wxID_ANY, lang );
		mActive->SetValue( true );
		mySizer->Add( mActive, 0, wxALIGN_CENTER_VERTICAL );

		// File path
		// TODO: save/load paths
		mFilePath = new wxTextCtrl( parent, wxID_ANY, wxString( defaultPath.fCStr() ) );
		mySizer->Add( mFilePath, 1, wxLEFT, 2 );

		// Browse button
		wxButton* browse = new wxButton( parent, wxID_ANY, "...", wxDefaultPosition, wxSize( 28, -1 ) );
		browse->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tLocmlControlPanel::tLocmlTargetUI::fOnBrowse ), NULL, this );
		mySizer->Add( browse, 0, wxLEFT, 2 );

		parent->GetSizer()->Add( mySizer, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 4 );
	}

	void tLocmlControlPanel::tLocmlTargetUI::fSetSource( )
	{
		mActive->SetValue( false );
		mActive->Disable();
	}

	b32 tLocmlControlPanel::tLocmlTargetUI::fActive() const
	{
		return mActive->GetValue();
	}

	tFilePathPtr tLocmlControlPanel::tLocmlTargetUI::fGetFullPath() const
	{
		return ToolsPaths::fMakeResAbsolute( tFilePathPtr( mFilePath->GetValue().c_str() ) );
	}

	void tLocmlControlPanel::tLocmlTargetUI::fOnBrowse( wxCommandEvent& event )
	{
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mParent, 
			"Open Locml",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "" ),
			wxString( "*.locml" ),
			wxFD_OPEN ) );

		if( openFileDialog->ShowModal( ) != wxID_OK )
			return;

		std::string path = ToolsPaths::fMakeResRelative( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ), true ).fCStr( );

		mFilePath->SetValue( wxString( path.c_str() ) );
	}
}
