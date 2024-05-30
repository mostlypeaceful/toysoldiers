#ifndef __tWxDirectoryBrowser__
#define __tWxDirectoryBrowser__
#include "Threads/tThread.hpp"
#include "Threads/tMutex.hpp"
#include "wx/treectrl.h"

namespace Sig
{

	class tools_export tWxDirectoryBrowser : public wxTreeCtrl
	{
	protected:

		struct tTreeViewFileData
		{
			u32 mSortValue;
			std::string mDisplayName;
			tFilePathPtr mXmlPath;
			tFilePathPtr mBinaryPath;

			tTreeViewFileData( ) : mSortValue( 0 ) { }

			inline b32 operator<( const tTreeViewFileData& other ) const
			{
				if( mSortValue == other.mSortValue )
					return mDisplayName < other.mDisplayName;
				return mSortValue < other.mSortValue;
			}
		};

		struct tTreeViewDirectoryData;

		typedef std::map< std::string, tTreeViewDirectoryData > tTreeViewDirectoryDataMap;

		struct tTreeViewDirectoryData
		{
			std::string mName;
			tGrowableArray< tTreeViewFileData > mFiles;
			tTreeViewDirectoryDataMap mSubFolders;
			inline b32 operator<( const tTreeViewDirectoryData& other ) const { return mName < other.mName; }

		};

	protected:

		tTreeViewDirectoryData		mResDir;
		wxTreeItemId				mRightClickItem;
		tFilePathPtrList			mAllPaths;
		tFilePathPtrList			mAllRelevantPaths;
		tGrowableArray<std::string>	mExpandedPaths;

		b16							mRefreshMT;
		b16							mRefreshMTComplete;
		Threads::tCriticalSection	mCritSec;
		Threads::tThread			mThread;

	public:

		class tools_export tFileEntryData : public wxTreeItemData
		{
			tWxDirectoryBrowser* mOwner;
			tFilePathPtr mXmlPath;
			tFilePathPtr mBinaryPath;
		public:
			tFileEntryData( tWxDirectoryBrowser* owner,  const tFilePathPtr& pathToXml, const tFilePathPtr& pathToBinary )
				: mOwner( owner ), mXmlPath( pathToXml ), mBinaryPath( pathToBinary ) { }
			inline const tFilePathPtr& fXmlPath( ) const { return mXmlPath; }
			inline const tFilePathPtr& fBinaryPath( ) const { return mBinaryPath; }
			wxColour fGetTextItemColor( ) const;
		};

		tWxDirectoryBrowser( wxWindow* parent, u32 minHeight );
		~tWxDirectoryBrowser( );
		virtual void fOnSelChanged( wxTreeEvent& event, const tFileEntryData& fileEntryData ) { }
		virtual wxColour fProvideCustomEntryColor( const tFileEntryData& fileEntryData ) { return wxColour( 0, 0, 0 ); }
		virtual b32 fFilterPath( const tFilePathPtr& path ) = 0; // i.e., return !Sigml::fIsSigmlFile( path );
		virtual std::string fConvertDisplayName( const std::string& simplePath, u32& sortValue ) { sortValue = 0; return simplePath; }
		virtual tFilePathPtr fXmlPathToBinaryPath( const tFilePathPtr& xmlPath ) = 0; // i.e., return Sigml::fSigmlPathToSigb( xmlPath );
		virtual tFilePathPtr fRootPath( ) const { return ToolsPaths::fGetCurrentProjectResFolder( ); } // must be res or a sub-folder of res

		void fOnSelChanged( wxTreeEvent& event );
		tFileEntryData* fIsFileItem( wxTreeItemId id ) const;
		void fRebuildResDir( );
		std::string fGetAbsoluteItemName( wxTreeItemId dir ) const;
		void fTrackExpanded( wxTreeItemId root );
		void fReExpand( wxTreeItemId root, const std::string& selectedItemName );
		void fRefreshAllPaths( );
		void fRefresh( b32 fullRefresh = true );
		void fSetRefreshMT( b32 mt ) { mRefreshMT = mt; }
		void fHandleRefreshMTCompletion( );
		wxTreeItemId fAddDirectory( wxTreeItemId curRoot, const tTreeViewDirectoryData& dir );
		const tFilePathPtrList& fGetAllPaths( ) const { return mAllPaths; }
		const tFilePathPtrList& fGetRelevantPaths( ) const { return mAllRelevantPaths; }
		tFilePathPtrList fGetRelevantPathsOfType( const tGrowableArray<const char*>& extList ) const;
	private:
		void fRefreshPartTwo( );
	};

}

#endif//__tWxDirectoryBrowser__

