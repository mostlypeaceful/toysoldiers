#include "SigEdPch.hpp"
#include "tMemoryViewerDialog.hpp"
#include "tEditorAppWindow.hpp"
#include "tSceneRefEntity.hpp"
#include "tMeshEntity.hpp"

#include "Gfx\tTextureFile.hpp"
#include "Gfx\tGeometryFile.hpp"
#include "tMeshEntity.hpp"
#include "tMesh.hpp"
#include "tFxFileRefEntity.hpp"

namespace Sig
{
	namespace
	{
		static const f32 cOneMB = 1024.f * 1024.f;
	}

	struct tMemoryRecordNode
	{
		tMemoryRecordNode( ) : mMyBytesMain( 0 ), mBabyBytesMain( 0 ), mMyBytesVid( 0 ), mBabyBytesVid( 0 ) { }

		tFilePathPtr mKey;
		wxString	mName;
		u32			mMyBytesMain;
		u32			mBabyBytesMain;
		u32			mMyBytesVid;
		u32			mBabyBytesVid;

		tGrowableArray< tMemoryRecordNode* > mChildren;
	};

	class tTreeRecord : public wxTreeItemData
	{
	public:
		tTreeRecord( ) : mRecord( NULL ) { }
		tTreeRecord( const tMemoryRecordNode* const record ) : mRecord( record ) { }
		const tMemoryRecordNode* const mRecord;
	};

	b32 fCompareMainBytesAscending( const tMemoryRecordNode* a, const tMemoryRecordNode* b ) { return (a->mMyBytesMain + a->mBabyBytesMain) > (b->mMyBytesMain + b->mBabyBytesMain); }
	b32 fCompareVideoBytesAscending( const tMemoryRecordNode* a, const tMemoryRecordNode* b ) { return (a->mMyBytesVid + a->mBabyBytesVid) > (b->mMyBytesVid + b->mBabyBytesVid); }
	b32 fCompareTotalBytesAscending( const tMemoryRecordNode* a, const tMemoryRecordNode* b )
	{
		return (a->mMyBytesMain + a->mBabyBytesMain + a->mMyBytesVid + a->mBabyBytesVid) > (b->mMyBytesMain + b->mBabyBytesMain + b->mMyBytesVid + b->mBabyBytesVid);
	}

	tMemoryViewerDialog::tMemoryViewerDialog( tEditorAppWindow* editorWindow )
		: tEditorDialog( editorWindow, "MemoryViewerDialog" )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetTitle( "Asset Memory Viewer" );

		const int width = 400;
		SetMinSize( wxSize( width, -1 ) );
		SetSize( wxSize( width, -1 ) );

		mMainPanel = new wxScrolledWindow( this );
		mMainPanel->SetBackgroundColour( wxColour( 0xee, 0xee, 0xee ) );
		mMainPanel->SetForegroundColour( wxColour( 0x33, 0x33, 0x55 ) );
		mMainPanel->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		GetSizer( )->Add( mMainPanel, 1, wxEXPAND | wxALL, 0 );

		mData = new wxTreeCtrl( mMainPanel );
		mMainPanel->GetSizer( )->Add( mData, 1, wxEXPAND | wxALL, 10 );

		wxSizer* horizontalBottom = new wxBoxSizer( wxHORIZONTAL );

		mVMode = new wxCheckBox( mMainPanel, wxID_ANY, "VRAM" );
		mVMode->SetValue( true );
		horizontalBottom->Add( mVMode, 0, wxCENTER | wxBOTTOM | wxRIGHT, 10 );

		mMMode = new wxCheckBox( mMainPanel, wxID_ANY, "Main" );
		mMMode->SetValue( true );
		horizontalBottom->Add( mMMode, 0, wxCENTER | wxBOTTOM | wxLEFT, 10 );

		mMainPanel->GetSizer( )->Add( horizontalBottom, 0, wxCENTER );

		mVMode->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tMemoryViewerDialog::fOnModeChanged), NULL, this );
		mMMode->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tMemoryViewerDialog::fOnModeChanged), NULL, this );

		Layout( );
		Refresh( );
	}

	// So many fun little helper funx
	b32 fFindExistingRecord( const tFilePathPtr& searchKey, tGrowableArray< tMemoryRecordNode* >& records )
	{
		for( u32 i = 0; i < records.fCount(); ++i )
		{
			if( records[i]->mKey == searchKey )
				return true;
		}

		return false;
	}

	void fScrapeSubs( tResourcePtrList& subs, tGrowableArray< tMemoryRecordNode* >& records )
	{
		for( u32 s = 0; s < subs.fCount(); ++s )
		{
			tResource* res = subs[ s ].fGetRawPtr( );

			// Exclude duplicates on a sibling level.
			if( fFindExistingRecord( res->fGetPath(), records ) )
				continue;

			// Otherwise, make a new record and get it's data.
			tMemoryRecordNode* newRecord = NEW_TYPED( tMemoryRecordNode )( );

			newRecord->mKey = res->fGetPath( );
			newRecord->mName = StringUtil::fNameFromPath( res->fGetPath().fCStr() );

			if( res->fIsType<Gfx::tGeometryFile>() )
			{
				Gfx::tGeometryFile* geo = res->fCast<Gfx::tGeometryFile>( );

				newRecord->mMyBytesMain = geo->fMainUsage( );
				u32 dummya, dummyb, dummyc, dummyd;
				newRecord->mMyBytesVid = geo->fVramUsage( dummya, dummyb, dummyc, dummyd );
			}
			else if( res->fIsType<Gfx::tTextureFile>() )
			{
				Gfx::tTextureFile* tex = res->fCast<Gfx::tTextureFile>( );

				newRecord->mMyBytesMain = tex->fMainUsage( );
				newRecord->mMyBytesVid = tex->fVramUsage( );
			}
			else
			{
				newRecord->mMyBytesMain = res->fGetResourceBufferSize( );
			}

			records.fPushBack( newRecord );
		}
	}

	void fRegistercursively( tGrowableArray< tMemoryRecordNode* >& records, tEntity* entity )
	{
		const u32 num = entity->fChildCount( );
		for( u32 i = 0; i < num; ++i )
		{
			tEntity* baseEntity = entity->fChild( i ).fGetRawPtr( );

			tResource* res = NULL;

			tSceneRefEntity* sceneRefEntity = baseEntity->fDynamicCast< tSceneRefEntity >( );
			if( sceneRefEntity )
				res = sceneRefEntity->fSgResource( ).fGetRawPtr( );

			FX::tFxFileRefEntity* fxEntity = baseEntity->fDynamicCast< FX::tFxFileRefEntity >( );
			if( fxEntity )
				res = fxEntity->fFxResource( ).fGetRawPtr( );

			if( !res )
				continue;

			// Exclude duplicates on a sibling level.
			if( fFindExistingRecord( res->fGetPath(), records ) )
				continue;

			tMemoryRecordNode* newRecord = NEW_TYPED( tMemoryRecordNode )( );

			newRecord->mKey = res->fGetPath( );
			newRecord->mName = StringUtil::fNameFromPath( res->fGetPath().fCStr() );
			newRecord->mMyBytesMain = res->fGetResourceBufferSize( );

			fRegistercursively( newRecord->mChildren, baseEntity );

			tLoadInPlaceFileBase* lipFile = (tLoadInPlaceFileBase*)res->fGetResourceBuffer( );
			tResourcePtrList subs;
			lipFile->fGatherSubResources( subs );

			if( subs.fCount() )
				fScrapeSubs( subs, newRecord->mChildren );

			for( u32 child = 0; child < newRecord->mChildren.fCount(); ++child )
			{
				newRecord->mBabyBytesMain += newRecord->mChildren[ child ]->mBabyBytesMain + newRecord->mChildren[ child ]->mMyBytesMain;
				newRecord->mBabyBytesVid += newRecord->mChildren[ child ]->mBabyBytesVid + newRecord->mChildren[ child ]->mMyBytesVid;
			}

			records.fPushBack( newRecord );
		}
	}

	void fDeletecursively( tGrowableArray< tMemoryRecordNode* >& records )
	{
		for( u32 i = 0; i < records.fCount(); ++i )
		{
			fDeletecursively( records[i]->mChildren );
			delete records[i];
		}

		records.fDeleteArray( );
	}

	void tMemoryViewerDialog::fDrawcursively( tGrowableArray< tMemoryRecordNode* >& records, wxTreeItemId& root )
	{
		// Sort based on what's visible
		if( mVMode->GetValue() && mMMode->GetValue() )
		{
			std::sort( records.fBegin(), records.fEnd(), fCompareTotalBytesAscending );
		}
		else if( mVMode->GetValue() )
		{
			std::sort( records.fBegin(), records.fEnd(), fCompareVideoBytesAscending );
		}
		else if( mMMode->GetValue() )
		{
			std::sort( records.fBegin(), records.fEnd(), fCompareMainBytesAscending );
		}

		for( u32 i = 0; i < records.fCount(); ++i )
		{
			const tMemoryRecordNode* thisNode = records[i];

			// Get the byte information we care about based on selected flags.
			u32 thisBytes = 0;
			u32 babyBytes = 0;

			if( mMMode->GetValue() )
			{
				thisBytes += thisNode->mMyBytesMain;
				babyBytes += thisNode->mBabyBytesMain;
			}

			if( mVMode->GetValue() )
			{
				thisBytes += thisNode->mMyBytesVid;
				babyBytes += thisNode->mBabyBytesVid;
			}

			// Lead off with my own size record.
			wxString label;
			label << wxString::Format( "%.2f", (thisBytes / cOneMB) ) << "mb";

			// Optionally show accumulated underneath guys.
			if( babyBytes > 0.f )
			{
				const f32 totalUnderneath = babyBytes / cOneMB;
				label << " / " << wxString::Format( "%.2f", totalUnderneath ) << "mb";
			}

			// Cap it with a name.
			label << " - " + records[i]->mName;

			// Add it to a tree.
			wxTreeItemId newChild = mData->AppendItem( root, label );	

			tTreeRecord* newTreeRecord = new tTreeRecord( records[i] );
			mData->SetItemData( newChild, newTreeRecord );

			fDrawcursively( records[i]->mChildren, newChild );
		}
	}

	void tMemoryViewerDialog::fOnSelectionChanged( tEditorSelectionList & list )
	{
		if( list.fCount() == 0 )
			return;

		mData->Freeze( );

		fDeletecursively( mRecords );

		const u32 num = list.fCount();
		for( u32 i = 0; i < num; ++i )
			fRegistercursively( mRecords, list[i].fGetRawPtr() );

		// DUPLICATED BELOW
		mData->DeleteAllItems();
		wxTreeItemId root = mData->AddRoot( "Selections" );
		fDrawcursively( mRecords, root );
		mData->Expand( root );

		mData->Layout( );
		mData->Thaw( );
	}

	void tMemoryViewerDialog::fTrackExpanded( tGrowableArray<tFilePathPtr>& expandedKeys, wxTreeItemId node )
	{
		if( !node.IsOk( ) ) 
		{
			log_warning( "tMemoryViewerDialog:: Node isn't ok trying to track expanded." );
			return;
		}

		const tTreeRecord* const data = (const tTreeRecord* const)mData->GetItemData( node );
		if( data && data->mRecord )
		{
			if( mData->IsExpanded( node ) )
			{
				expandedKeys.fFindOrAdd( data->mRecord->mKey );
			}
			else
			{
				expandedKeys.fFindAndErase( data->mRecord->mKey );
			}
		}

		wxTreeItemIdValue id;
		for( wxTreeItemId ichild = mData->GetFirstChild( node, id ); ichild.IsOk( ); ichild = mData->GetNextChild( ichild, id ) )
			fTrackExpanded( expandedKeys, ichild );
	}

	void tMemoryViewerDialog::fReExpand( tGrowableArray<tFilePathPtr>& expandedKeys, wxTreeItemId node )
	{
		if( !node.IsOk( ) )
		{
			log_warning( "tMemoryViewerDialog:: Node isn't ok trying to re-expand." );
			return;
		}

		const tTreeRecord* const data = (const tTreeRecord* const)mData->GetItemData( node );
		if( data && data->mRecord )
		{
			if( expandedKeys.fFind( data->mRecord->mKey ) )
				mData->Expand( node );
		}

		wxTreeItemIdValue id;
		for( wxTreeItemId ichild = mData->GetFirstChild( node, id ); ichild.IsOk( ); ichild = mData->GetNextChild( ichild, id ) )
			fReExpand( expandedKeys, ichild );
	}

	void tMemoryViewerDialog::fOnModeChanged( wxCommandEvent& event )
	{
		tGrowableArray< tFilePathPtr > expandedKeys;

		fTrackExpanded( expandedKeys, mData->GetRootItem() );

		// DUPLICATED ABOVE
		mData->DeleteAllItems();
		wxTreeItemId root = mData->AddRoot( "Selections" );
		fDrawcursively( mRecords, root );
		mData->Expand( root );

		fReExpand( expandedKeys, root );
	}
}
