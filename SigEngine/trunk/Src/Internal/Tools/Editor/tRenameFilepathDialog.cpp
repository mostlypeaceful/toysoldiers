#include "ToolsPch.hpp"
#include "tRenameFilepathDialog.hpp"
#include "FileSystem.hpp"
#include "Sigml.hpp"
#include "tScriptFileConverter.hpp"
#include "tFileWriter.hpp"

namespace Sig
{
	static const u32 cOutsideBorderSpacer	= 6;

	tRenameFilepathDialog::tRenameFilepathDialog( wxWindow* parent, const tFilePathPtr& originalPath, b32 onlySameType )
		: wxDialog( parent, wxID_ANY, "Rename file and replace references.", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxRESIZE_BORDER | wxTAB_TRAVERSAL | wxSTAY_ON_TOP )
		, mOriginalPath( originalPath )
		, mOnlySameType( onlySameType )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		wxStaticText* sText = new wxStaticText( this, wxID_ANY, "Enter new name:" );
		GetSizer( )->Add( sText, 0, wxALL | wxEXPAND, cOutsideBorderSpacer );

		mProgress = new wxProgressDialog( "Searching..", "super longgggggggggggggggggggggggggggggggggggggggg file path", 100, this );
		
		wxBoxSizer* horizSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( horizSizer, 0, wxEXPAND, 0 );

		mText = new wxTextCtrl( this, wxID_ANY );
		mText->SetValue( originalPath.fCStr( ) );
		horizSizer->Add( mText, 1, wxALL | wxEXPAND, cOutsideBorderSpacer );

		wxButton *button = new wxButton( this, wxID_ANY, "Rename" );
		horizSizer->Add( button, 0, wxRIGHT | wxBOTTOM, cOutsideBorderSpacer );

		button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tRenameFilepathDialog::fOnAccept ), NULL, this );

		mListBox = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_MULTIPLE | wxLB_NEEDED_SB );
		GetSizer( )->Add( mListBox, 1, wxEXPAND, 0 );
		
		SetSize( 800, 500 );

		if( fPopulateListBox( ) )
		{
			Layout( );
			ShowModal( );
		}
		else
			Close( );
	}

	void tRenameFilepathDialog::fOnAccept( wxCommandEvent& event )
	{
		tFilePathPtr newPath( mText->GetValue( ).c_str( ) );
		tFilePathPtr absOld = ToolsPaths::fMakeResAbsolute( mOriginalPath );
		tFilePathPtr absNew = ToolsPaths::fMakeResAbsolute( newPath );

		if( absOld == absNew )
		{
			wxMessageBox( "Trying to rename the file to the same name.", "Oops!", wxOK, this );
			return;
		}

		if( !FileSystem::fFileExists( absNew ) 
			|| wxMessageBox( "File already exists. Continue?", "Oops!", wxYES_NO, this ) == wxYES )
		{
			FileSystem::fCreateDirectory( tFilePathPtr( StringUtil::fDirectoryFromPath( absNew.fCStr( ) ) ) );
			FileSystem::fCopyFile( absOld, absNew );
			FileSystem::fDeleteFile( absOld );

			for( u32 i = 0; i < mListBox->GetCount( ); ++i )
			{
				if( mListBox->IsChecked( i ) )
					mEntries[ i ]->fReplace( mOriginalPath, newPath );
			}

			wxMessageBox( "Success!\nYou'll need to run assetgen to build the modified files.", "Oops!", wxOK, this );
			Close( true );
		}
	}

	namespace 
	{
		struct tSigmlEntry : public tRenameFilepathDialog::tEntry
		{
			Sigml::tFile* mFile;
			tFilePathPtr mPath;

			tSigmlEntry( const tFilePathPtr& path, const tFilePathPtr& replace )
				: mFile( NULL ), mPath( path )
			{
				mFile = new Sigml::tFile( );
				mFile->fLoadXml( path );

				mOccurances = mFile->fReplaceReferences( replace, replace );
			}

			~tSigmlEntry( ) { delete mFile; }

			virtual void fReplace( const tFilePathPtr& replace, const tFilePathPtr& with )
			{
				sigassert( mFile );
				mFile->fReplaceReferences( replace, with );
				mFile->fSaveXml( mPath, true );
			}
		};

		struct tTextEntry : public tRenameFilepathDialog::tEntry
		{
			tFilePathPtr mPath;
			std::string mScript;

			tTextEntry( const tFilePathPtr& path, const tFilePathPtr& replace )
				: mPath( path )
			{
				if( FileSystem::fReadFileToString( mScript, path ) )
				{
					mOccurances = tScriptFileConverter::fReplaceFilePathReferences( mScript, replace, replace );
				}
				else
					mScript = "";
			}

			virtual void fReplace( const tFilePathPtr& replace, const tFilePathPtr& with )
			{
				sigassert( mScript.length() && mOccurances > 0 );

				tScriptFileConverter::fReplaceFilePathReferences( mScript, replace, with );

				// write it back out
				tFileWriter ofile;
				ofile.fOpen( mPath );
				ofile( mScript.c_str( ), mScript.length( ) );
			}
		};
	}

	b32 tRenameFilepathDialog::fPopulateListBox( )
	{
		mEntries.fSetCount( 0 );
		mListBox->Clear( );

		bool cancel = false;
		mProgress->Update( 25, "Finding files to parse...", &cancel );

		tFilePathPtrList files;
		FileSystem::fGetFileNamesInFolder( files, ToolsPaths::fGetCurrentProjectResFolder( ), true, true );
		FileSystem::fGetFileNamesInFolder( files, ToolsPaths::fGetCurrentProjectSrcFolder( ), true, true );

		const std::string originalExtension = StringUtil::fGetExtension( mOriginalPath.fCStr( ) );

		for( u32 i = 0; !cancel && i < files.fCount( ); ++i )
		{
			std::string ext = StringUtil::fGetExtension( files[ i ].fCStr( ) );
			tEntry *e = NULL;

			if( mOnlySameType && ext != originalExtension )
				continue;

			mProgress->Update( f32(i)/files.fCount( ), "Parsing: " + StringUtil::fToString( i ) + "/" + StringUtil::fToString( files.fCount( ) ) + "\n" + files[ i ].fCStr( ), &cancel );

			if( ext == ".sigml" ) e = new tSigmlEntry( files[ i ], mOriginalPath );
			else if( ext == ".nut" || ext == ".hpp" || ext == ".h" || ext == ".cpp" ) e = new tTextEntry( files[ i ], mOriginalPath );
			// TODO SEARCH .TABS FOR OCCURANCES AND WARN USER THAT/WHICH TABLES MUST BE UPDATED MANUALLY

			if( e && e->fValid( ) )
			{
				mEntries.fPushBack( tEntryPtr( e ) );
				mListBox->Append( std::string( files[ i ].fCStr( ) ) + " (" + StringUtil::fToString( e->mOccurances ) +")" );
				mListBox->Check( mListBox->GetCount( ) - 1 );
			}
			else 
				delete e;
		}
		mProgress->Hide( );

		if( cancel )
		{
			return false;
		}
		else if( mListBox->GetCount( ) == 0 )
		{
			wxMessageBox( "No occurrences of this file found in resource tree. " );
			return false;
		}

		return true;
	}

}