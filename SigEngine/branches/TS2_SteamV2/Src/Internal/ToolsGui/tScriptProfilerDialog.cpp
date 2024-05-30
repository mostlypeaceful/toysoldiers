#include "ToolsGuiPch.hpp"
#include "FileSystem.hpp"
#include "tScriptProfilerDialog.hpp"
#include "wx/filedlg.h"

using namespace Sig::Memory;

namespace Sig { namespace ScriptData
{

	namespace
	{
		const f32 cRowWidth = 500.f;
		const u32 cBytesPerRow = 1024 * 1024 * 0.33f;
		const f32 cRowHeight = 12.f;

		const u32 cColorCount = 5;
		const wxBrush cColors[ cColorCount ] = 
		{ 
			*wxBLACK_BRUSH,  
			*wxBLUE_BRUSH,
			*wxCYAN_BRUSH,
			*wxGREEN_BRUSH,
			*wxRED_BRUSH,
		};

		enum tShowOptions
		{
			cShowAll,
			cShowOne,
			cShowFreed,
			cShowOptionsCount
		};

		enum tSortOptions
		{
			cSortAddress,
			cSortSize,
			cSortType,
			cSortOptionsCount
		};

		enum tRightClickOptions
		{
			cDiffDump
		};

		const wxString cShowOptionsStrs[ cShowOptionsCount ] = 
		{
			wxString( "All" ),
			wxString( "One" ),
			wxString( "Freed" ),
		};

		const wxString cSortOptionsStrs[ cSortOptionsCount ] = 
		{
			wxString( "Address" ),
			wxString( "Size" ),
			wxString( "Type" ),
		};

	}

	tMemoryDrawer::tMemoryDrawer( tScriptProfilerDialog* owner )
		: tWxDrawPanel( owner )
		, mOwner( owner )
	{ }


	void tMemoryDrawer::fLimit( const wxTreeItemId& alloc, const wxTreeItemId& heap )
	{
		mLimitAlloc = alloc;
		mLimitHeap = heap;
		Refresh( );
	}

	void tMemoryDrawer::fRenderRowBlock( wxDC& dc, u32 start, u32 end, u32 row )
	{
		f32 startPt = (f32)start / cBytesPerRow * cRowWidth;
		f32 endPt = (f32)end / cBytesPerRow * cRowWidth;

		dc.DrawRectangle( wxRect( startPt, row * cRowHeight, endPt - startPt, cRowHeight ) );	
	}

	b32 tMemoryDrawer::fShouldDrawAlloc( wxTreeItemId& item, const tBaseDump* alloc, u32 type ) const
	{
		if( !item.m_pItem )
			return true;
		else
		{
			tProfilerNodeData* nodeData = static_cast<tProfilerNodeData*> ( mOwner->fTree( )->GetItemData( item ) );
			if( !nodeData )
				return false;

			sigassert( nodeData->mData );

			if( nodeData->mType == type && (alloc == static_cast<Memory::tAllocDump*>( nodeData->mData )) )
				return true;
			else
			{
				wxTreeItemIdValue cookie;
				for( wxTreeItemId child = mOwner->fTree( )->GetFirstChild( item, cookie ); child.m_pItem; child = mOwner->fTree( )->GetNextChild( child, cookie ) )
				{
					if( fShouldDrawAlloc( child, alloc, type ) )
						return true;
				}

				return false;
			}
		}
	}

	void tMemoryDrawer::fRenderHeap( wxDC& dc, const Memory::tHeapDump& heap )
	{
		u32 color = 0;

		for( u32 p = 0; p < heap.mPages.fCount( ); ++p )
		{
			const tPageDump& page = heap.mPages[ p ];
			for( u32 a = 0; a < page.mAllocations.fCount( ); ++a )
			{
				const tAllocDump& alloc = page.mAllocations[ a ];

				if( !fShouldDrawAlloc( mLimitAlloc, &alloc, tNodeData::cTypeAlloc ) )
					continue;

				++color;
				color = Math::fModulus( color, cColorCount );

				dc.SetBrush( cColors[ color ] );
				dc.SetPen( wxPen( wxColor(0,0,0), 0 ) );

				u32 addressOffset = alloc.mStamp.mAddress - page.mAddress;
				u32 row = addressOffset / cBytesPerRow;
				u32 start = addressOffset % cBytesPerRow;
				u32 end = start + alloc.mStamp.mSize;
				u32 overage = 0;

				if( end > cBytesPerRow )
				{
					overage = end - cBytesPerRow;
					end = cBytesPerRow;
				}

				fRenderRowBlock( dc, start, end, row );

				if( overage )
				{
					u32 fullBlocks = overage / cBytesPerRow;
					for( u32 i = 0 ; i < fullBlocks; ++i )
						fRenderRowBlock( dc, 0, cBytesPerRow, row + i );
					u32 extra = overage % cBytesPerRow;
					fRenderRowBlock( dc, 0, extra, row + fullBlocks );
				}				
			}
		}
	}

	void tMemoryDrawer::fRender( wxDC& dc )
	{
		for( u32 h = 0; h < mOwner->fDump( ).mHeaps.fCount( ); ++h )
		{
			const tHeapDump& heap = mOwner->fDump( ).mHeaps[ h ];
			if( heap.mType == tHeapDump::cTypeChunky )
			{
				if( !fShouldDrawAlloc( mLimitHeap, &heap, tNodeData::cTypeHeap ) )
					continue;

				fRenderHeap( dc, heap );

				if( !mLimitHeap.m_pItem )
					break; //if not limited only draw one heap
			}
		}
	}

	wxComboBox* fAddCombo( wxDialog* parent, wxBoxSizer* sizer, const wxString& name, u32 count, const wxString* strs )
	{
		wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );

		wxStaticText* label = new wxStaticText( parent, wxID_ANY, name );
		hSizer->Add( label, 0, wxEXPAND | wxALL );

		wxComboBox* comb = new wxComboBox( parent, wxID_ANY );
		for( u32 i = 0; i < count; ++i )
			comb->Append( strs[ i ] );

		comb->SetSelection( 0 );
		hSizer->Add( comb, 5, wxEXPAND | wxALL );

		sizer->Add( hSizer, 0, wxEXPAND | wxALL );
		return comb;
	}

	void tScriptProfilerDialog::fOutputDiff( tSortByType sort1, tSortByType sort2 )
	{
		tGrowableArray< tData > types = sort1.fBuckets( );
		tGrowableArray< tData > types2 = sort2.fBuckets( );
		tGrowableArray< tDiffData > diffData;

		for( u32 i = 0; i < types2.fCount( ); ++i )
		{
			// Find this type in the other dump
			for( u32 j = 0; j < types.fCount( ); ++j )
			{
				if( types2[ i ].mAlloc == types[ j ].mAlloc )
				{
					s32 diffBytes = types2[ i ].mConsumed - types[ j ].mConsumed;
					s32 diffAlloc = types2[ i ].mRealAlloc.fCount( ) - types[ j ].mRealAlloc.fCount( );

					if ( diffBytes > 0 )
						diffData.fPushBack( tDiffData( diffBytes, diffAlloc, types2[ i ].mAlloc.mStamp ) );

					types2.fEraseOrdered( i-- );
					break;
				}
			}
		}

		// Append remaining allocations
		tGrowableArray< tDiffData > remaining;
		for( u32 i = 0; i < types2.fCount( ); ++i )
		{
			tStampDump stamp = types2[ i ].mAlloc.mStamp;
			stamp.mFile = "NEW: " + stamp.mFile;

			remaining.fPushBack( tDiffData( types2[ i ].mConsumed, types2[ i ].mRealAlloc.fCount( ), stamp ) );
		}
		
		// Sort the two lists independently and then join them so that
		// the new allocations stay at the bottom
		std::sort( diffData.fBegin( ), diffData.fEnd( ), DiffDataDescending( ) );
		std::sort( remaining.fBegin( ), remaining.fEnd( ), DiffDataDescending( ) );
		diffData.fJoin( remaining );

		fPopTree( diffData );

		s32 totalConsumed = sort2.fTotalConsumed( ) - sort1.fTotalConsumed( );
		f32 totalConsumedMb = totalConsumed / ( 1024.f * 1024.f );
		std::string totalStr = "TOTAL CONSUMED DELTA: " + StringUtil::fToString( totalConsumed ) + " (" + StringUtil::fToString( totalConsumedMb ) + " MB)";
		std::string dashes = "\n" + std::string( totalStr.length( ), '-' ) + "\n";
		log_line( 0, dashes + totalStr + dashes );

		Thaw( );
	}

	tScriptProfilerDialog::tScriptProfilerDialog( wxWindow* parent )
		: wxDialog( parent, wxID_ANY, "Browser", wxDefaultPosition, wxSize( 1200, 600 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX )
	{
		wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL );

		// Tree Panel
		{
			wxBoxSizer* treeSizer = new wxBoxSizer( wxVERTICAL );

			mShowOptions = fAddCombo( this, treeSizer, "Show:", cShowOptionsCount, cShowOptionsStrs );
			mSortOptions = fAddCombo( this, treeSizer, "Sort:", cSortOptionsCount, cSortOptionsStrs );

			wxButton* loadBut = new wxButton( this, wxID_ANY, "Load" );
			loadBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tScriptProfilerDialog::fLoadData ), NULL, this );
			treeSizer->Add( loadBut, 0, wxEXPAND | wxALL );

			mTree = new wxTreeCtrl( this );
			mTree->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxCommandEventHandler( tScriptProfilerDialog::fTreeItemClicked ), NULL, this );
			mTree->Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxContextMenuEventHandler( tScriptProfilerDialog::fOnItemRightClick ) );
			treeSizer->Add( mTree, 10, wxEXPAND | wxALL );

			sizer->Add( treeSizer, 2, wxEXPAND | wxALL );
		}

		// Text panel
		{
			wxBoxSizer* hyperlinkSizer = new wxBoxSizer( wxVERTICAL );

			mTextBox = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_RICH | wxTE_AUTO_URL );
			hyperlinkSizer->Add( mTextBox, 1, wxEXPAND | wxALL );

			mHyperlink = new wxHyperlinkCtrl( this, wxID_ANY, "Go To Source", "", wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE | wxHL_CONTEXTMENU | wxHL_ALIGN_CENTRE );
			hyperlinkSizer->Add( mHyperlink, 0.1, wxEXPAND | wxALL );

			sizer->Add( hyperlinkSizer, 1, wxEXPAND | wxALL );
		}

		mDrawPanel = new tMemoryDrawer( this );
		sizer->Add( mDrawPanel, 3, wxEXPAND | wxALL );

		SetSizer( sizer );
	}

	wxFileDialog* tScriptProfilerDialog::fOpenDumpDialog( )
	{
		return new wxFileDialog( this, "Open Dump File", "", "", "*.dmpml", wxFD_OPEN | wxFD_FILE_MUST_EXIST );
	}

	void tScriptProfilerDialog::fShow( tOpenDocCallback fOpenDoc, tFocusAllOnLineCallback fFocusAllOnLine )
	{
		this->fOpenDoc = fOpenDoc;
		this->fFocusAllOnLine = fFocusAllOnLine;

		Show( );
	}

	void tScriptProfilerDialog::fTreeItemClicked( wxCommandEvent& event )
	{
		wxTreeItemId selection = mTree->GetSelection( );
		if( !selection.IsOk( ) )
			return;

		if ( selection == mLastIdProcessed )
			return;
		mLastIdProcessed = selection;

		fClearStats( );

		fPopulateStats( selection );

		if( mTree->GetChildrenCount( selection, false ) == 0 )
		{
			fPopTree( selection );
		}
	}

	void tScriptProfilerDialog::fDiffDump( wxCommandEvent& event )
	{
		wxFileDialog* openDumpFileDialog = fOpenDumpDialog( );
		if( openDumpFileDialog->ShowModal( ) != wxID_OK )
			return;

		Memory::tMemoryDump dump2 = Memory::tMemoryDump( );
		dump2.fLoadXml( tFilePathPtr( openDumpFileDialog->GetPath( ).c_str( ) ) );

		tSortByType dump1Sorted( &mDump );
		tSortByType dump2Sorted( &dump2 );

		fOutputDiff( dump1Sorted, dump2Sorted );
	}

	void tScriptProfilerDialog::fHyperlinkClicked( wxHyperlinkEvent& event )
	{
		wxTreeItemId selection = mTree->GetSelection( );
		if( !selection.IsOk( ) )
			return;

		tNodeData* nodeDataPtr = static_cast<tNodeData*> ( mTree->GetItemData( selection ) );
		if( nodeDataPtr->mType != tNodeData::cTypeAlloc )
			return;

		tProfilerNodeData* nodeData = static_cast<tProfilerNodeData*> ( nodeDataPtr );
		if( !nodeData )
			return;

		tAllocDump* allocData = static_cast< tAllocDump* > ( nodeData->mData );

		wxString file = allocData->mStamp.mFile;
		u32 line = allocData->mStamp.mLineNum;

		// Replace leading "." character with fully-qualified path
		if ( file.SubString(0, 1 ) == "." )
			file = ToolsPaths::fGetCurrentProjectSrcFolder( ).fCStr( ) + file.SubString(1, file.Length( ) - 1 );

		tFilePathPtr filePath( file );
		if ( !FileSystem::fFileExists( filePath ) )
		{
			// If the source file isn't in the project, it should be in the engine
			wxString srcPath = "\\Src\\Internal\\Base\\";
			tFilePathPtr altFilePath( ToolsPaths::fGetEngineRootFolder( ).fCStr( ) + srcPath + file );
			filePath = altFilePath;
		}

		fOpenDoc( filePath );
		fFocusAllOnLine( line );
	}

	void tScriptProfilerDialog::fOnItemRightClick( wxContextMenuEvent& event )
	{
		wxMenu menu;
		menu.Append( cDiffDump, "Diff Dump", "Diff this dump against a second dump.", wxITEM_NORMAL );
		PopupMenu( &menu );
		event.Skip( );
	}

	void tScriptProfilerDialog::fClearStats( )
	{
		mTextBox->Clear( );
	}

	void tScriptProfilerDialog::fWriteStat( const std::string& text )
	{
		mTextBox->WriteText( text );
	}

	void tScriptProfilerDialog::fPopulateStats( wxTreeItemId item )
	{
		tNodeData* nodeData = static_cast<tNodeData*> ( mTree->GetItemData( item ) );
		if( !nodeData )
			return;

		fWriteStat( nodeData->fToString( ) );

		if( nodeData->mType == tNodeData::cTypeAlloc )
		{
			wxTreeItemId p;
			for( p = mTree->GetItemParent( item ); p.m_pItem; p = mTree->GetItemParent( p ) )
			{
				tProfilerNodeData* nodeData = static_cast<tProfilerNodeData*> ( mTree->GetItemData( p ) );
				if( !nodeData )
				{
					p = wxTreeItemId( );
					break;
				}

				if( nodeData->mType == tNodeData::cTypeHeap )
					break;
			}

			mDrawPanel->fLimit( item, p );
		}
		else if( nodeData->mType == tNodeData::cTypeHeap )
		{
			mDrawPanel->fLimit( wxTreeItemId( ), item );
		}
	}

	struct tSortByAddress
	{
		tSortByAddress( tGrowableArray<tAllocDump>& allocs, wxTreeCtrl* tree, wxTreeItemId parent )
		{
			for( u32 i = 0; i < allocs.fCount( ); ++i )
			{
				Memory::tAllocDump& alloc = allocs[ i ];
				wxTreeItemId node = tree->AppendItem( parent, alloc.fToSoftString( ) );
				tree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypeAlloc, &allocs[ i ] ) );
			}
		}
	};

	void tScriptProfilerDialog::fPopTree( wxTreeItemId parent )
	{
		Freeze( );

		tProfilerNodeData* nodeData = static_cast<tProfilerNodeData*> ( mTree->GetItemData( parent ) );
		if( !nodeData )
			return;

		mTree->DeleteChildren( parent );

		sigassert( nodeData->mData );

		switch( nodeData->mType )
		{
		case tNodeData::cTypeRoot:
			{
				Memory::tMemoryDump* dump = static_cast<Memory::tMemoryDump*>( nodeData->mData );
				for( u32 i = 0; i < dump->mHeaps.fCount( ); ++i )
				{
					Memory::tHeapDump& heap = dump->mHeaps[ i ];
					wxTreeItemId node = mTree->AppendItem( parent, heap.fToSoftString( ) );
					mTree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypeHeap, &heap ) );
				}
			}
			break;
		case tNodeData::cTypeHeap:
			{
				Memory::tHeapDump* dump = static_cast<Memory::tHeapDump*>( nodeData->mData );
				for( u32 i = 0; i < dump->mPages.fCount( ); ++i )
				{
					Memory::tPageDump& page = dump->mPages[ i ];
					wxTreeItemId node = mTree->AppendItem( parent, page.fToSoftString( ) );
					mTree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypePage, &page ) );
				}
			}
			break;
		case tNodeData::cTypePage:
			{
				Memory::tPageDump* dump = static_cast<Memory::tPageDump*>( nodeData->mData );

				if( mSortOptions->GetSelection( ) == cSortType )
				{
					tSortByType doit( dump->mAllocations );
					doit.fPopTree( mTree, parent );
				}
				else
					tSortByAddress doit( dump->mAllocations, mTree, parent );
			}
			break;
		default:
			{
				Thaw( );
				return;
			}
		}

		mTree->CollapseAll( );
		mTree->EnsureVisible( parent );
		mTree->Expand( parent );
		mTree->SelectItem( parent );

		Thaw( );
	}

	void tScriptProfilerDialog::fPopTree( tGrowableArray< tDiffData >& diffData )
	{
		mTree->DeleteAllItems( );

		wxTreeItemId rootNode = mTree->AddRoot( "Root Node" );

		Freeze( );

		mTree->DeleteChildren( rootNode );

		for( u32 i = 0; i < diffData.fCount( ); ++i )
		{
			tDiffData data = diffData[ i ];

			log_line( 0, data.fToString( ) );

			wxTreeItemId node = mTree->AppendItem( rootNode, data.mStamp.mFile + ": " + StringUtil::fToString( data.mStamp.mLineNum ) );
			mTree->SetItemData( node, new tDiffData( data.mDiffBytes, data.mAllocDelta, data.mStamp ) );
		}

		mTree->CollapseAll( );
		mTree->EnsureVisible( rootNode );
		mTree->Expand( rootNode );
		mTree->SelectItem( rootNode );
	}

	void tScriptProfilerDialog::fLoadData( wxCommandEvent& event )
	{
		wxFileDialog* openDumpFileDialog = fOpenDumpDialog( );
		if( openDumpFileDialog->ShowModal( ) != wxID_OK )
			return;

		mTree->DeleteAllItems( );

		wxTreeItemId rootNode = mTree->AddRoot( "Root Node" );
		mTree->SetItemData( rootNode, new tProfilerNodeData( tNodeData::cTypeRoot, &mDump ) );

		mDump = Memory::tMemoryDump( );
		mDump.fLoadXml( tFilePathPtr( openDumpFileDialog->GetPath( ) ) );

		fPopTree( rootNode );
	}

	BEGIN_EVENT_TABLE( tScriptProfilerDialog, wxDialog )
		EVT_HYPERLINK( wxID_ANY, tScriptProfilerDialog::fHyperlinkClicked )
		EVT_MENU( cDiffDump, tScriptProfilerDialog::fDiffDump )
		END_EVENT_TABLE()
} }
