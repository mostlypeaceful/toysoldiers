#include "AtlasPch.hpp"
#include "tAtlasWindow.hpp"
#include "FileSystem.hpp"
#include "tAssetPluginDll.hpp"
#include "tAssetGenScanner.hpp"
#include "tStrongPtr.hpp"

namespace Sig
{
	enum
	{
		cActionNew = 1,
		cActionOpen,
		cActionSave,
		cActionSaveAs,
		cActionBuild,
		cActionAbout,
		cActionQuit,

		cActionMoveUp,
		cActionMoveDown,

		cActionSemanticDiffuse,
		cActionSemanticNormal,

		cActionFormatCompressed,
		cActionFormatUncompressed,
	};

	BEGIN_EVENT_TABLE(tAtlasWindow, wxFrame)
		EVT_CLOSE(									tAtlasWindow::fOnClose)
		EVT_MENU(				cActionNew,			tAtlasWindow::fOnAction)
		EVT_MENU(				cActionOpen,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionSave,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionSaveAs,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionBuild,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionAbout,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionQuit,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionMoveUp,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionMoveDown,	tAtlasWindow::fOnAction)
		EVT_MENU(				cActionSemanticDiffuse,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionSemanticNormal,		tAtlasWindow::fOnAction)
		EVT_MENU(				cActionFormatCompressed,	tAtlasWindow::fOnAction)
		EVT_MENU(				cActionFormatUncompressed,	tAtlasWindow::fOnAction)
	END_EVENT_TABLE()

	tAtlasWindow::tAtlasWindow( const wxString& title/*, const wxPoint& pos, const wxSize& size*/ )
	   : wxFrame( (wxFrame *)NULL, -1, title, wxPoint(-1,-1), wxSize(600,500) )
	   , mListBox( 0 )
	   , mEditMenu( 0 )
	   , mSubWidth( 0 )
	   , mSubHeight( 0 )
	   , mDirty( false )
	{
		// load asset plugins right-off the bat
		tAssetPluginDllDepot::fInstance( ).fLoadPluginsBasedOnCurrentProjectFile( );

		tWxSlapOnControl::fSetLabelWidth( 491 );
		tWxSlapOnControl::fSetControlWidth( 80 );

		//
		// set icon...
		//

		SetIcon(wxIcon("appicon"));

		//
		// add menu...
		//

		wxMenuBar *menuBar = new wxMenuBar;

		wxMenu *menuFile = new wxMenu;
		menuFile->Append( cActionNew, "&New\tCtrl+N" );
		menuFile->Append( cActionOpen, "&Open...\tCtrl+O" );
		menuFile->Append( cActionSave, "&Save\tCtrl+S" );
		menuFile->Append( cActionSaveAs, "&Save As" );
		menuFile->AppendSeparator();
		menuFile->Append( cActionBuild, "&Build\tCtrl+Shift+B" );
		menuFile->AppendSeparator();
		menuFile->Append( cActionAbout, "&About..." );
		menuFile->AppendSeparator();
		menuFile->Append( cActionQuit, "E&xit" );
		menuBar->Append( menuFile, "&File" );

		mEditMenu = new wxMenu;
		mEditMenu->Append( cActionMoveUp, "Move &Up\t-" );
		mEditMenu->Append( cActionMoveDown, "Move &Down\t+" );
		mEditMenu->AppendSeparator( );
		mEditMenu->AppendRadioItem( cActionSemanticDiffuse, "DiffuseMap" );
		mEditMenu->AppendRadioItem( cActionSemanticNormal, "NormalMap" );
		mEditMenu->AppendSeparator( );
		mEditMenu->AppendRadioItem( cActionFormatCompressed, "Compressed" );
		mEditMenu->AppendRadioItem( cActionFormatUncompressed, "Uncompressed" );
		menuBar->Append( mEditMenu, "&Edit" );

		SetMenuBar( menuBar );

		//
		// add status bar...
		//

		int statusWidths[] = { 0, 110, -1 };
		wxStatusBar* statusBar = CreateStatusBar( array_length(statusWidths), wxST_SIZEGRIP|wxFULL_REPAINT_ON_RESIZE, 0, wxT("Active Profile:") );
		SetStatusWidths( array_length(statusWidths), statusWidths );
		SetStatusText( "Texture Properties:", 1 );
		SetStatusText( "null", 2 );

		//
		// add main background panel...
		//

		wxPanel *panel = new wxPanel( this, wxID_ANY );

		wxBoxSizer *vbox = new wxBoxSizer( wxVERTICAL );
		panel->SetSizer( vbox );

		//
		// add Existing Profiles static text...
		//

		wxBoxSizer* hbox0 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox0, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

		wxStaticText* staticText = new wxStaticText( panel, wxID_ANY, wxT("Textures"), wxDefaultPosition, wxDefaultSize );
		hbox0->Add( staticText, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxLEFT, 5 );

		//
		// add List Box of profiles...
		//

		wxBoxSizer* hbox1 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox1, 1, wxGROW | wxALL, 0 );

		mListBox = new wxListBox( panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString(), wxLB_SINGLE );
		hbox1->Add( mListBox, 1, wxGROW | wxALL, 5 );

		mListBox->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler(tAtlasWindow::fOnListBoxDClick), NULL, this );
		mListBox->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler(tAtlasWindow::fOnListBoxRUp), NULL, this );
		mListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(tAtlasWindow::fOnSelChanged), NULL, this );

		//
		// add Buttons for adding/removing textures...
		//

		wxBoxSizer* hbox2 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox2, 0, wxALIGN_RIGHT | wxALL, 0 );

		wxStaticText* addRemoveText = new wxStaticText( panel, wxID_ANY, "Add/Remove Textures" );
		hbox2->Add( addRemoveText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

		wxButton* addTex = new wxButton( panel, wxID_ANY, "+", wxDefaultPosition, wxSize( 20, 20 ) );
		wxButton* subTex = new wxButton( panel, wxID_ANY, "-", wxDefaultPosition, wxSize( 20, 20 ) );
		hbox2->AddStretchSpacer( 1 );
		hbox2->Add( addTex, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5 );
		hbox2->Add( subTex, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5 );

		addTex->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAtlasWindow::fOnAddTexPressed ), NULL, this );
		subTex->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tAtlasWindow::fOnSubTexPressed ), NULL, this );

		//
		// add sub texture width/height controls
		//
		mSubWidth = new tSpinner( this, panel, "Individual Texture Width", 64.f, 2048.f, 4.f, 0 );
		mSubHeight = new tSpinner( this, panel, "Individual Texture Height", 64.f, 2048.f, 4.f, 0 );
		mSubWidth->fSetValueNoEvent( 512.f );
		mSubHeight->fSetValueNoEvent( 512.f );

		vbox->AddSpacer( 12 );

		//
		// center the window
		//

		fMarkDirty( false );
		Centre( );
	}

	tAtlasWindow::~tAtlasWindow( )
	{
	}

	b32 tAtlasWindow::fPathExistsAlready( const tFilePathPtr& path ) const
	{
		for( u32 i = 0; i < mListBox->GetCount( ); ++i )
		{
			if( fGetPathAt( i ) == path )
				return true;
		}
		return true;
	}

	tFilePathPtr tAtlasWindow::fGetPathAt( int index ) const
	{
		if( index >= 0 && index < ( s32 )mListBox->GetCount( ) )
			return tFilePathPtr( mListBox->GetString( index ) );
		return tFilePathPtr( );
	}

	void tAtlasWindow::fUpdateTitle( )
	{
		std::stringstream ss;
		ss << "SigAtlas ~ ";
		if( mCurrentDoc.fNull( ) )
			ss << "(untitled)";
		else
			ss << mCurrentDoc.fCStr( );
		if( mDirty )
			ss << "*";
		SetTitle( wxString( ss.str( ).c_str( ) ) );

		Gfx::tTextureFile::tSemantic s; Gfx::tTextureFile::tFormat f;
		const std::string formatText = fFigureSemanticAndFormat( s, f );
		ss.str( "" );
		ss << mListBox->GetCount( ) << " textures";
		const int selIndex = mListBox->GetSelection( );
		if( selIndex >= 0 && selIndex < ( int )mListBox->GetCount( ) )
			ss << ", selected = " << selIndex;
		ss << " | " << formatText;
		SetStatusText( ss.str( ).c_str( ), 2 );
	}

	b32 tAtlasWindow::fClearDoc( )
	{
		if( mDirty )
		{
			const int result = wxMessageBox( "You have unsaved changes - would you like to save them before resetting?",
						  "Save Changes?", wxYES | wxNO | wxCANCEL | wxICON_WARNING );

			if(			result == wxYES )			{ if( !fSaveDoc( false ) ) return false; }
			else if(	result == wxNO )			{ }
			else if(	result == wxCANCEL )		{ return false; }
			else									{ log_warning( "Unknown result returned from Message Box" ); }
		}

		mCurrentDoc = tFilePathPtr( );
		mListBox->Clear( );
		mEditMenu->Check( cActionSemanticDiffuse, true );
		mEditMenu->Check( cActionFormatCompressed, true );
		mSubWidth->fSetValueNoEvent( 512.f );
		mSubHeight->fSetValueNoEvent( 512.f );
		fMarkDirty( false );
		return true;
	}

	b32 tAtlasWindow::fSaveDoc( b32 saveAs )
	{
		if( saveAs || mCurrentDoc.fNull( ) )
		{
			const std::string ext = Tatml::fGetFileExtension( );

			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				this, 
				"Save Scene As",
				wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
				wxString( "untitled" + ext ),
				wxString( "*" + ext ),
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

			if( openFileDialog->ShowModal( ) != wxID_OK )
				return false; // cancelled

			mCurrentDoc = tFilePathPtr( openFileDialog->GetPath( ).c_str( ) );
//fAddRecentFile( mCurrentDoc );
//fMainWindow( ).fUpdateRecentFileMenu( );
		}
		else
		{
			// not doing a save as; if we're not dirty, then skip
			if( !mDirty )
				return true;
		}

		fSerialize( mCurrentDoc );

		fMarkDirty( false );
		return true;
	}

	void tAtlasWindow::fBuildDoc( )
	{
		if( mCurrentDoc.fNull( ) )
		{
			wxMessageBox( "You must save your file before building or previewing it.", "Save First", wxOK | wxICON_WARNING );
		}
		else if( mDirty )
		{
			fSaveDoc( false );
		}

		tAssetGenScanner::fProcessSingleFile( mCurrentDoc, true );
	}

	void tAtlasWindow::fOpenDoc( )
	{
		if( mDirty )
		{
			if( !fClearDoc( ) )
				return; // user cancelled, don't try to open new file
		}

		const std::string ext = Tatml::fGetFileExtension( );

		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			this, 
			"Open Texture Atlas File",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "untitled" + ext ),
			wxString( "*" + ext ),
			wxFD_OPEN ) );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			fOpenDoc( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
		}
	}

	void tAtlasWindow::fOpenDoc( const tFilePathPtr& file )
	{
		if( !fClearDoc( ) )
			return; // user cancelled, don't try to open new file

		if( FileSystem::fFileExists( file ) )
		{
			if( !fDeserialize( file ) )
			{
				wxMessageBox( "The specified .sigml file is corrupt or out of date; open failed.", "Invalid File", wxOK | wxICON_WARNING );
				return;
			}

			// set up new scene
			mCurrentDoc = file;
			fUpdateTitle( );
//fGuiApp( ).fAddRecentFile( file );
//fUpdateRecentFileMenu( );
		}
		else
		{
			wxMessageBox( "The specified file can not be found; open failed.", "File Not Found", wxOK | wxICON_WARNING );
		}
	}

	void tAtlasWindow::fSerialize( const tFilePathPtr& file )
	{
		Tatml::tFile tatml;

		const u32 numEntries = mListBox->GetCount( );
		for( u32 i = 0; i < numEntries; ++i )
		{
			const tFilePathPtr path = fGetPathAt( i );
			if( !path.fNull( ) ) tatml.mTexturePaths.fPushBack( path );
		}

		tatml.mSubWidth = mSubWidth->fGetValue( );
		tatml.mSubHeight = mSubHeight->fGetValue( );

		fFigureSemanticAndFormat( tatml.mSemantic, tatml.mFormat );

		tatml.fSaveXml( file, true );
	}

	b32 tAtlasWindow::fDeserialize( const tFilePathPtr& file )
	{
		Tatml::tFile tatml;
		if( !tatml.fLoadXml( file ) )
			return false;

		for( u32 i = 0; i < tatml.mTexturePaths.fCount( ); ++i )
		{
			const tFilePathPtr relPath = ToolsPaths::fMakeResRelative( tatml.mTexturePaths[ i ] );
			mListBox->Append( wxString( relPath.fCStr( ) ) );
		}

		mSubWidth->fSetValueNoEvent( ( f32 )tatml.mSubWidth );
		mSubHeight->fSetValueNoEvent( ( f32 )tatml.mSubHeight );

		switch( tatml.mSemantic )
		{
		case Gfx::tTextureFile::cSemanticNormal:
			mEditMenu->Check( cActionSemanticNormal, true );
			mEditMenu->Check( cActionFormatCompressed, true );
			break;
		case Gfx::tTextureFile::cSemanticDiffuse:
		default:
			mEditMenu->Check( cActionSemanticDiffuse, true );
			if( tatml.mFormat == Gfx::tTextureFile::cFormatDXT1 )
				mEditMenu->Check( cActionFormatCompressed, true );
			else
				mEditMenu->Check( cActionFormatUncompressed, true );
			break;
		}

		return true;
	}

	void tAtlasWindow::fMoveSelection( s32 dir )
	{
		const int curIdx = mListBox->GetSelection( );
		if( curIdx < 0 || curIdx >= ( int )mListBox->GetCount( ) )
			return; // hmmm, current selection not valid, abort

		const wxString selText = mListBox->GetString( curIdx );
		const int newIdx = fRound<int>( fWrap( ( f32 )curIdx + dir, 0.f, ( f32 )mListBox->GetCount( ) ) );
		sigassert( newIdx >= 0 && newIdx < ( int )mListBox->GetCount( ) );

		mListBox->Delete( curIdx );
		mListBox->Insert( selText, newIdx );
		mListBox->Select( newIdx );

		fMarkDirty( );
	}

	std::string tAtlasWindow::fFigureSemanticAndFormat( Gfx::tTextureFile::tSemantic& semantic, Gfx::tTextureFile::tFormat& format )
	{
		std::stringstream ss;

		b32 compressed = true;
		if( mEditMenu->IsChecked( cActionFormatUncompressed ) )
			compressed = false;

		b32 doDefault = false;
		if( mEditMenu->IsChecked( cActionSemanticDiffuse ) )
		{
			doDefault = true;
			mEditMenu->Enable( cActionFormatUncompressed, true );
		}
		else if( mEditMenu->IsChecked( cActionSemanticNormal ) )
		{
			mEditMenu->Check( cActionFormatCompressed, true );
			mEditMenu->Enable( cActionFormatUncompressed, false );

			semantic = Gfx::tTextureFile::cSemanticNormal;
			format = Gfx::tTextureFile::cFormatDXT5;
			ss << "NormalMap, DXT5";
		}
		else
		{
			doDefault = true;
			log_warning( "invalid state" );
		}

		if( doDefault )
		{
			semantic = Gfx::tTextureFile::cSemanticDiffuse;
			if( compressed )
			{
				format = Gfx::tTextureFile::cFormatDXT1;
				ss << "DiffuseMap, DXT1 (default) / DXT5 (if alpha)";
			}
			else
			{
				format = Gfx::tTextureFile::cFormatA8R8G8B8;
				ss << "DiffuseMap, 32-bit ARGB";
			}
		}

		return ss.str( );
	}

	void tAtlasWindow::fMarkDirty( b32 dirty )
	{
		mDirty = dirty;
		fUpdateTitle( );
	}

	void tAtlasWindow::fOnAction(wxCommandEvent& event)
	{
		switch( event.GetId( ) )
		{
		case cActionNew:	fClearDoc( ); break;
		case cActionOpen:	fOpenDoc( ); break;
		case cActionSave:	fSaveDoc( false ); break;
		case cActionSaveAs: fSaveDoc( true ); break;
		case cActionBuild:  fBuildDoc( ); break;
		case cActionAbout:	wxMessageBox( "This application allows you to create and edit texture atlases.", "About SigAtlas", wxOK | wxICON_INFORMATION ); break;
		case cActionQuit:	Close( true ); break;

		case cActionMoveUp:		fMoveSelection( -1 ); break;
		case cActionMoveDown:	fMoveSelection( +1 ); break;

		case cActionSemanticDiffuse:
		case cActionSemanticNormal:
		case cActionFormatCompressed:
		case cActionFormatUncompressed:
			fMarkDirty( ); break;
		}
	}

	void tAtlasWindow::fOnClose(wxCloseEvent& event)
	{
		if( !fClearDoc( ) )
			event.Veto( );
		else
			event.Skip( );
	}

	namespace
	{
		static void fBrowseForTextures( wxWindow* parent, tFilePathPtrList& paths, b32 multiple = true )
		{
			const std::string ext = "*.bmp;*.jpg;*.png;*.tga;*.dds";

			// browse for a new path
			tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
				parent, 
				"Select Texture",
				wxEmptyString,
				wxEmptyString,
				wxString( "*" + ext ),
				wxFD_OPEN | ( multiple ? wxFD_MULTIPLE : 0 ) ) );

			if( openFileDialog->ShowModal( ) == wxID_OK )
			{
				wxArrayString wxpaths;
				openFileDialog->GetPaths( wxpaths );

				for( u32 i = 0; i < wxpaths.Count( ); ++i )
				{
					const tFilePathPtr absPath = tFilePathPtr( wxpaths[ i ].c_str( ) );
					const tFilePathPtr relPath = ToolsPaths::fMakeResRelative( absPath );
					paths.fPushBack( relPath );
				}
			}
		}
	}

	void tAtlasWindow::fOnAddTexPressed(wxCommandEvent& event)
	{
		tFilePathPtrList paths;
		fBrowseForTextures( this, paths );
		for( u32 i = 0; i < paths.fCount( ); ++i )
		{
			mListBox->Append( wxString( paths[ i ].fCStr( ) ) );
			fMarkDirty( );
		}
	}

	void tAtlasWindow::fOnSubTexPressed(wxCommandEvent& event)
	{
		const int index = mListBox->GetSelection( );
		if( index >= 0 && index < ( s32 )mListBox->GetCount( ) )
		{
			mListBox->Delete( index );
			if( mListBox->GetCount( ) > 0 )
				mListBox->Select( fMin<s32>( index, mListBox->GetCount( ) - 1 ) );
			fMarkDirty( );
		}
	}

	void tAtlasWindow::fOnListBoxDClick(wxMouseEvent& event)
	{
		const int index = mListBox->GetSelection( );
		if( index >= 0 && index < ( s32 )mListBox->GetCount( ) )
		{
			tFilePathPtrList paths;
			fBrowseForTextures( this, paths, false );
			if( paths.fCount( ) > 0 )
			{
				mListBox->SetString( index, wxString( paths.fFront( ).fCStr( ) ) );
				fMarkDirty( );
			}
		}
	}

	void tAtlasWindow::fOnListBoxRUp(wxMouseEvent& event)
	{
		wxPoint pos = event.GetPosition();

		wxMenu menu;

		menu.Append(cActionMoveUp, _T("Move &Up\t-"));
		menu.Append(cActionMoveDown, _T("Move &Down\t+"));

		PopupMenu(&menu, pos.x, pos.y);
	}

	void tAtlasWindow::fOnSelChanged(wxCommandEvent& event)
	{
		fUpdateTitle( );
	}

}
