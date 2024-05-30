#include "ToolsPch.hpp"
#include "tWxDirectoryBrowser.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	wxColour tWxDirectoryBrowser::tFileEntryData::fGetTextItemColor( ) const
	{
		if( mBinaryPath.fLength( ) > 0 )
		{
			if( !FileSystem::fFileExists( mBinaryPath ) ||
				FileSystem::fIsAMoreRecentThanB( tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectResFolder( ), mXmlPath ), mBinaryPath ) )
			{
				// binary file either doesn't exist or is out of date
				return wxColour( 0xff, 0x00, 0x00 );
			}
		}
		return mOwner->fProvideCustomEntryColor( *this );
	}

	namespace
	{
		static u32 thread_call fRefreshAllPathsMT( void* param )
		{
			tWxDirectoryBrowser* dirBrowser = ( tWxDirectoryBrowser* )param;
			dirBrowser->fRefreshAllPaths( );
			return 0;
		}
	}

	tWxDirectoryBrowser::tWxDirectoryBrowser( wxWindow* parent, u32 minHeight )
		: wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.x, minHeight ) )
		, mRefreshMT( false )
		, mRefreshMTComplete( false )
	{
		SetBackgroundColour( wxColour( 0xee, 0xee, 0xee ) );
		SetForegroundColour( wxColour( 0x00, 0x00, 0x00 ) );
		Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler(tWxDirectoryBrowser::fOnSelChanged), 0, this );
		Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(tWxDirectoryBrowser::fOnSelChanged), 0, this );	
	}

	tWxDirectoryBrowser::~tWxDirectoryBrowser( )
	{
		if( mRefreshMT )
		{
			while( mThread.fRunning( ) )
				fSleep( 1 );
		}
	}

	void tWxDirectoryBrowser::fOnSelChanged( wxTreeEvent& event )
	{
		const wxTreeItemId id = event.GetItem( );
		if( !id.IsOk( ) )
			return;
		const wxString name = GetItemText( id );
		const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( id ) );
		if( !fileEntryData )
			return;

		fOnSelChanged( event, *fileEntryData );
	}

	tWxDirectoryBrowser::tFileEntryData* tWxDirectoryBrowser::fIsFileItem( wxTreeItemId id ) const
	{
		return( dynamic_cast< tFileEntryData* >( GetItemData( id ) ) );
	}

	void tWxDirectoryBrowser::fRebuildResDir( )
	{
		const tFilePathPtr currentRootFolder = fRootPath( );

		const char slashString[] = { fPlatformFilePathSlash( cCurrentPlatform ), '\0' };

		mResDir = tTreeViewDirectoryData( );
		mResDir.mName = StringUtil::fNameFromPath( currentRootFolder.fCStr( ) );

		// first, we need to bucketize the list by folder
		mAllRelevantPaths.fDeleteArray( );
		for( u32 i = 0; i < mAllPaths.fCount( ); ++i )
		{
			if( ToolsPaths::fIsUnderSourceFolder( mAllPaths[ i ] ) )
				continue;
			if( fFilterPath( mAllPaths[ i ] ) )
				continue;

			// record the paths that match this filter.
			mAllRelevantPaths.fPushBack( mAllPaths[ i ] );

			const tFilePathPtr file = ToolsPaths::fMakeResRelative( mAllPaths[ i ] );

			tGrowableArray< std::string > subStrings;
			StringUtil::fSplit( subStrings, file.fCStr( ), slashString );

			tTreeViewDirectoryData* currentDir = &mResDir;
			for( u32 isub = 0; isub < subStrings.fCount( ) - 1; ++isub )
			{
				tTreeViewDirectoryData* nextDir = &currentDir->mSubFolders[ subStrings[ isub ] ];
				nextDir->mName = subStrings[ isub ];
				currentDir = nextDir;
			}

			tTreeViewFileData fileData;
			fileData.mDisplayName = fConvertDisplayName( subStrings.fBack( ), fileData.mSortValue );
			fileData.mXmlPath = ToolsPaths::fMakeResRelative( file );
			fileData.mBinaryPath = fXmlPathToBinaryPath( fileData.mXmlPath );
			currentDir->mFiles.fPushBack( fileData );

			std::sort( currentDir->mFiles.fBegin( ), currentDir->mFiles.fEnd( ) );
		}
	}

	std::string tWxDirectoryBrowser::fGetAbsoluteItemName( wxTreeItemId dir ) const
	{
		if( !dir.IsOk( ) ) return "";

		std::string name = GetItemText( dir ).c_str( );

		for( wxTreeItemId parent = GetItemParent( dir ); parent.IsOk( ); parent = GetItemParent( parent ) )
			name = GetItemText( parent ) + "." + name;

		return name;
	}

	void tWxDirectoryBrowser::fTrackExpanded( wxTreeItemId root )
	{
		if( !root.IsOk( ) ) 
			return;
		if( fIsFileItem( root ) ) 
			return;

		const std::string name = fGetAbsoluteItemName( root );
		if( IsExpanded( root ) )
		{
			mExpandedPaths.fFindOrAdd( name );
		}
		else
		{
			mExpandedPaths.fFindAndErase( name );
		}

		wxTreeItemIdValue cookie;
		for( wxTreeItemId ichild = GetFirstChild( root, cookie ); ichild.IsOk( ); ichild = GetNextChild( ichild, cookie ) )
			fTrackExpanded( ichild );
	}

	void tWxDirectoryBrowser::fReExpand( wxTreeItemId root, const std::string& selectedItemName )
	{
		if( !root.IsOk( ) ) return;

		const std::string name = fGetAbsoluteItemName( root );
		if( mExpandedPaths.fFind( name ) )
			Expand( root );

		if( name == selectedItemName )
			SelectItem( root );

		wxTreeItemIdValue cookie;
		for( wxTreeItemId ichild = GetFirstChild( root, cookie ); ichild.IsOk( ); ichild = GetNextChild( ichild, cookie ) )
			fReExpand( ichild, selectedItemName );
	}

	void tWxDirectoryBrowser::fRefreshAllPaths( )
	{
		const tFilePathPtr currentRootFolder = fRootPath( );
		mAllPaths.fDeleteArray( );
		FileSystem::fGetFileNamesInFolder( 
			mAllPaths,
			currentRootFolder,
			true,
			true );

		if( mRefreshMT )
		{
			Threads::tMutex lock( mCritSec );
			mRefreshMTComplete = true;
		}
	}

	void tWxDirectoryBrowser::fRefresh( b32 fullRefresh )
	{
		if( fullRefresh )
		{
			if( mRefreshMT )
			{
				mRefreshMTComplete = false;

				// kick off thread
				mThread.fStart( fRefreshAllPathsMT, "fRefreshAllPathsMT", this );

				return;
			}
			else
				fRefreshAllPaths( );
		}

		fRefreshPartTwo( );
	}

	void tWxDirectoryBrowser::fRefreshPartTwo( )
	{
		Freeze( );

		// save off selected item name
		const std::string selectedName = fGetAbsoluteItemName( GetSelection( ) );

		// save off the expanded directory names
		fTrackExpanded( GetRootItem( ) );

		DeleteAllItems( );
		fRebuildResDir( );
		Expand( fAddDirectory( wxTreeItemId( ), mResDir ) );

		// now re-expand saved directories
		fReExpand( GetRootItem( ), selectedName );

		Thaw( );
	}

	void tWxDirectoryBrowser::fHandleRefreshMTCompletion( )
	{
		Threads::tMutex lock( mCritSec );

		if( !mRefreshMTComplete )
			return;
		mRefreshMTComplete = false;
		fRefreshPartTwo( );
	}

	wxTreeItemId tWxDirectoryBrowser::fAddDirectory( wxTreeItemId curRoot, const tTreeViewDirectoryData& dir )
	{
		if( !curRoot )
			curRoot = AddRoot( dir.mName.c_str( ) );
		else
			curRoot = AppendItem( curRoot, dir.mName.c_str( ), -1, -1, 0 );

		for( u32 ifile = 0; ifile < dir.mFiles.fCount( ); ++ifile )
		{
			tFileEntryData* fileEntryData = new tFileEntryData( this, dir.mFiles[ ifile ].mXmlPath, dir.mFiles[ ifile ].mBinaryPath );
			wxTreeItemId fileId = AppendItem( curRoot, dir.mFiles[ ifile ].mDisplayName.c_str( ), -1, -1, fileEntryData );
			SetItemTextColour( fileId, fileEntryData->fGetTextItemColor( ) );
		}
		for( tTreeViewDirectoryDataMap::const_iterator isub = dir.mSubFolders.begin( ); isub != dir.mSubFolders.end( ); ++isub )
			fAddDirectory( curRoot, isub->second );

		return curRoot;
	}

	tFilePathPtrList tWxDirectoryBrowser::fGetRelevantPathsOfType( const tGrowableArray<const char*>& extList ) const 
	{ 
		tFilePathPtrList filteredPaths;
		for( u32 i = 0; i < mAllRelevantPaths.fCount( ); ++i )
		{
			const tFilePathPtr& thisPath = mAllRelevantPaths[ i ];

			b32 incompatible = false;
			for( u32 j = 0; j < extList.fCount( ); ++j )
			{
				if( !StringUtil::fCheckExtension( thisPath.fCStr( ), extList[j] ) )
				{
					incompatible = true;
					break;
				}
			}

			if( !incompatible )
				filteredPaths.fPushBack( thisPath );
		}

		return filteredPaths; 
	}

}
