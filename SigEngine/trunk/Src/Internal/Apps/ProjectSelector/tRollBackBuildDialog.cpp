#include "ProjectSelectorPch.hpp"
#include "FileSystem.hpp"
#include "tRollBackBuildDialog.hpp"

namespace Sig
{
	enum
	{
		ID_DoubleClick = 1,
	};

	BEGIN_EVENT_TABLE(tRollBackBuildDialog, wxDialog)
		EVT_CLOSE(							tRollBackBuildDialog::fOnClose)
		END_EVENT_TABLE()

		tRollBackBuildDialog::tRollBackBuildDialog( wxWindow* parent, tFilePathPtr buildsDirectory )
		: wxDialog( parent, -1, "Roll Back Build", wxPoint( -1, -1 ), wxSize( -1, -1 ) )
		, mBuildsDirectory( buildsDirectory )
	{
		// Make sure that the file which holds the current build number exists
		mCurrentBuildFile = tFilePathPtr::fConstructPath( mBuildsDirectory, tFilePathPtr( "\\latest_build.txt" ) );
		if ( !FileSystem::fFileExists( mCurrentBuildFile ) )
		{
			std::string message( mCurrentBuildFile.fCStr( ) ); 
			message += " could not be found!";
			wxMessageDialog( this, message ).ShowModal( );
			return;
		}
		mCurrentBuild = fReadLineFromFile( mCurrentBuild, mCurrentBuildFile );
		if( mCurrentBuild.length( ) <= 0 )
		{
			wxMessageDialog( this, "The build file is empty?!" ).ShowModal( );
			return;
		}

		// Construct dialog interface
		wxBoxSizer* vSizer = new wxBoxSizer( wxVERTICAL );

		wxStaticText* staticText = new wxStaticText( this, wxID_ANY, "Existing Builds" );
		vSizer->Add( staticText, 0, wxALIGN_CENTER_HORIZONTAL );

		mBuildList = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString( ), wxLB_SINGLE );
		mBuildList->Connect(wxEVT_LEFT_DCLICK,	wxMouseEventHandler( tRollBackBuildDialog::fOnListBoxDoubleClick ), NULL, this);
		vSizer->Add( mBuildList, 1, wxALIGN_CENTER_HORIZONTAL | wxEXPAND );

		wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );

		wxButton* cancelButton = new wxButton( this, wxID_ANY, wxT("Close") );
		cancelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tRollBackBuildDialog::fOnCloseButtonClick ), NULL, this );
		hSizer->Add( cancelButton, 0, wxALIGN_RIGHT );

		vSizer->Add( hSizer, 0, wxALIGN_RIGHT );

		SetSizer( vSizer );

		fRefreshBuildsList( );
	}

	tRollBackBuildDialog::~tRollBackBuildDialog( )
	{
	}

	void tRollBackBuildDialog::fOnCloseButtonClick( wxCommandEvent& event )
	{
		fOnClose( wxCloseEvent( ) );
	}

	void tRollBackBuildDialog::fOnClose(wxCloseEvent& event)
	{
		Show( false );
	}

	void tRollBackBuildDialog::fOnListBoxDoubleClick(wxMouseEvent& event)
	{
		int index = mBuildList->GetSelection( );

		std::string build = mBuildList->GetString( index );

		// Add the star to the double-clicked item and remove it
		// from the previous item
		if( build[0] != '*' )
		{
			for( u32 i = 0; i < mBuildList->GetCount( ); ++i )
			{
				std::string listItem = mBuildList->GetString( i );
				if( listItem[0] == '*' )
				{
					mBuildList->SetString( i, listItem.substr( 1 ) );
					break;
				}
			}

			mBuildList->SetString( index, std::string( "*" ).append( build ) );
		}
		else
			build = build.substr( 1 ); // Remove the star

		if( build == mCurrentBuild )
			return;

		mCurrentBuild = build;
		fWriteLineToFile( build, mCurrentBuildFile );
	}

	std::string tRollBackBuildDialog::fReadLineFromFile( std::string& output, tFilePathPtr& path )
	{
		FileSystem::fReadFileToString( output, path );
		return StringUtil::fEatWhiteSpace( output );
	}

	void tRollBackBuildDialog::fWriteLineToFile( std::string& line, tFilePathPtr& path )
	{
		std::string data = line + "\n";
		FileSystem::fWriteBufferToFile( data.c_str( ), data.length( ), path );
	}

	void tRollBackBuildDialog::fRefreshBuildsList( )
	{
		mBuildList->Clear( );

		tFilePathPtrList dirNames;
		FileSystem::fGetFolderNamesInFolder( dirNames, mBuildsDirectory );
		for( u32 i = 0; i < dirNames.fCount( ); ++i )
		{
			std::string dirName = dirNames[i].fCStr( );

			// Is this the active build?
			if( StringUtil::fStricmp( dirNames[ i ].fCStr( ), mCurrentBuild.c_str( ) ) == 0 )
				dirName = "*" + dirName;

			mBuildList->Insert( dirName, 0 );
		}
	}
}
