#include "ToolsGuiPch.hpp"
#include "FileSystem.hpp"
#include "tScriptProfilerDialog.hpp"
#include "wx/filedlg.h"
#include "wx/dcbuffer.h"
#include "Memory/tResourceMemoryProvider.hpp"
#include "Memory/tMainMemoryProvider.hpp"
#include "tPlatformDebugging.hpp"

using namespace Sig::Memory;

namespace Sig { namespace ScriptData
{

	namespace
	{
		const f32 cRowWidth = 500.f;
		const u32 cBytesPerRow = 1024 * 1024 * 0.33f;
		const f32 cRowHeight = 12.f;

		const f32 c1MB = 1024.f * 1024.f;

		const u32 cColorCount = 5;
		const wxBrush cColors[ cColorCount ] = 
		{ 
			*wxBLACK_BRUSH,  
			*wxBLUE_BRUSH,
			*wxCYAN_BRUSH,
			*wxGREEN_BRUSH,
			*wxRED_BRUSH,
		};

		enum tRightClickOptions
		{
			cDiffDump
		};

		// different ways to sort the data
		
		class tResourceSummary
		{
		public:
			void fReset( const tHeapDump& heap, const Memory::tMemoryDump& dump )
			{
				mDump = &dump;
				fComputeSummary( heap, dump );
			}

			void fPopulate( wxTreeCtrl& tree, wxTreeItemId& parent )
			{
				tree.DeleteChildren( parent );
				
				fPopulateCategory( tree, tree.AppendItem( parent, "By Type" ), mByType, *mDump );
				fPopulateCategory( tree, tree.AppendItem( parent, "By Tag" ), mByTag, *mDump );
			}

		private:
			// actually summary data
			struct tFileType
			{
				std::string mExtension;
				u32 mSize;
				tGrowableArray< const Memory::tAllocDump* > mFiles;

				tFileType( const std::string& ext = "" ) : mExtension( ext ), mSize( 0 ) { }
				b32 operator == ( const std::string& ext ) const { return (mExtension == ext); }
				b32 operator < ( const tFileType& other ) const { return (mSize > other.mSize); } // larger size first

				void fAddAlloc( const Memory::tAllocDump& alloc )
				{
					if( !mFiles.fFind( &alloc ) )
					{
						mFiles.fPushBack( &alloc );
						mSize += alloc.mStamp.mSize;
					}
				}

				void fSort( )
				{
					std::sort( mFiles.fBegin( ), mFiles.fEnd( ), fSortAllocs );
				}

				static b32 fSortAllocs( const Memory::tAllocDump* a, const Memory::tAllocDump* b ) { return (a->mStamp.mSize > b->mStamp.mSize); } // larger size first
			};

			const Memory::tMemoryDump* mDump;
			tGrowableArray< tFileType > mByType;
			tGrowableArray< tFileType > mByTag;

			void fPopulateCategory( wxTreeCtrl& tree, const wxTreeItemId& parent, const tGrowableArray< tFileType >& data, const Memory::tMemoryDump& dump )
			{
				for( u32 i = 0; i < data.fCount( ); ++i )
				{
					std::string header = data[ i ].mExtension + " - " + StringUtil::fToString( data[ i ].mSize / c1MB ) + "mb";
					wxTreeItemId type = tree.AppendItem( parent, header );
					for( u32 a = 0; a < data[ i ].mFiles.fCount( ); ++a )
					{
						const tAllocDump& alloc = *data[ i ].mFiles[ a ];
						wxTreeItemId node = tree.AppendItem( type, alloc.mStamp.fUserString( dump ) );
						tree.SetItemData( node, new tProfilerNodeData( tNodeData::cTypeAlloc, &alloc, dump ) );
					}
				}				
			}

			void fComputeSummary( const tHeapDump& heap, const Memory::tMemoryDump& dump )
			{
				mByType.fSetCount( 0 );
				mByTag.fSetCount( 0 );

				for( u32 p = 0; p < heap.mPages.fCount( ); ++p )
				{
					const Memory::tPageDump& page = heap.mPages[ p ];

					for( u32 a = 0; a < page.mAllocations.fCount( ); ++a )
					{
						const Memory::tAllocDump& alloc = page.mAllocations[ a ];

						std::string resourcefileName = alloc.mStamp.fUserString( dump );

						if( resourcefileName.length( ) )
						{
							std::string ext = StringUtil::fGetExtension( resourcefileName.c_str( ) );
							std::string fileName = StringUtil::fNameFromPath( resourcefileName.c_str( ), true );
							tGrowableArray< std::string > frags;
							StringUtil::fSplit( frags, fileName.c_str( ), "_" );

							tFileType* byType = mByType.fFind( ext );
							if( !byType )
							{
								mByType.fPushBack( tFileType( ext ) );
								byType = &mByType.fBack( );
							}

							byType->fAddAlloc( alloc );

							for( u32 f = 0; f < frags.fCount( ); ++f )
							{
								const std::string& frag = frags[ f ];

								tFileType* byTag = mByTag.fFind( frag );
								if( !byTag )
								{
									mByTag.fPushBack( tFileType( frag ) );
									byTag = &mByTag.fBack( );
								}

								byTag->fAddAlloc( alloc );
							}
						}
					}
				}

				std::sort( mByTag.fBegin( ), mByTag.fEnd( ) );
				std::sort( mByType.fBegin( ), mByType.fEnd( ) );

				for( u32 i = 0; i < mByTag.fCount( ); ++i )
					mByTag[ i ].fSort( );
				for( u32 i = 0; i < mByType.fCount( ); ++i )
					mByType[ i ].fSort( );
			}

		};


		struct tSortByAddress
		{
			tSortByAddress( const tMemoryDump& dump, const tGrowableArray<tAllocDump>& allocs, wxTreeCtrl* tree, wxTreeItemId parent )
			{
				tGrowableArray< const tAllocDump* > ptrs;
				ptrs.fSetCount( allocs.fCount( ) );

				for( u32 i = 0; i < ptrs.fCount( ); ++i )
					ptrs[ i ] = &allocs[ i ];

				std::sort( ptrs.fBegin( ), ptrs.fEnd( ), fSort );

				for( u32 i = 0; i < ptrs.fCount( ); ++i )
				{
					const Memory::tAllocDump& alloc = *ptrs[ i ];
					wxTreeItemId node = tree->AppendItem( parent, alloc.fToSoftString( dump ) );
					tree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypeAlloc, ptrs[ i ], dump ) );
				}
			}

			static b32 fSort( const tAllocDump* a, const tAllocDump* b )
			{
				return (a->mStamp.mAddress < b->mStamp.mAddress);
			}
		};


		struct tSortBySize
		{
			tSortBySize( const tMemoryDump& dump, const Memory::tHeapDump& heap, wxTreeCtrl* tree, wxTreeItemId parent )
			{
				tGrowableArray< const tAllocDump* > ptrs;

				for( u32 p = 0; p < heap.mPages.fCount( ); ++p )
					for( u32 i = 0; i < heap.mPages[ p ].mAllocations.fCount( ); ++i )
						ptrs.fPushBack( &heap.mPages[ p ].mAllocations[ i ] );

				std::sort( ptrs.fBegin( ), ptrs.fEnd( ), fSort );

				for( u32 i = 0; i < ptrs.fCount( ); ++i )
				{
					const Memory::tAllocDump& alloc = *ptrs[ i ];
					wxTreeItemId node = tree->AppendItem( parent, alloc.fToSoftString( dump ) );
					tree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypeAlloc, ptrs[ i ], dump ) );
				}
			}

			static b32 fSort( const tAllocDump* a, const tAllocDump* b )
			{
				return (a->mStamp.mSize > b->mStamp.mSize);
			}
		};


		struct tCategoryDump : public tBaseDump
		{
			enum tType
			{
				cSummarizedView,
				cByType,
				cBySize,
				cByAddress,
				cTypeCount
			};

			tCategoryDump( const tHeapDump* heap = NULL, u32 type = ~0 )
				: mHeap( heap )
				, mType( type )
				, mSummaryData( NULL )
			{ }

			std::string fName( ) const
			{
				switch( mType )
				{
				case cSummarizedView: return "Categorized"; break;
				case cByType: return "By Type"; break;
				case cBySize: return "By Size"; break;
				case cByAddress: return "By Address"; break;
				}

				return "Error";
			}


			const tHeapDump* mHeap;
			u32 mType;

			tResourceSummary* mSummaryData;
		};

		void fAddCategory( wxTreeCtrl& tree, wxTreeItemId parent, const Memory::tHeapDump* heap, u32 type, const Memory::tMemoryDump& fullDump )
		{
			tCategoryDump* cat = new tCategoryDump( heap, type );
			wxTreeItemId node =tree.AppendItem( parent, cat->fName( ) );
			tree.SetItemData( node, new tProfilerNodeData( tNodeData::cTypeCategory, cat, fullDump ) );

			if( type == tCategoryDump::cSummarizedView )
			{
				cat->mSummaryData = new tResourceSummary( );
				cat->mSummaryData->fReset( *heap, fullDump );
			}
		}
	}

	struct tSortByType
	{
		struct tData
		{
			tAllocDump mAlloc;
			tGrowableArray<const tAllocDump*> mRealAlloc;
			u32 mConsumed;

			tData( )
			{ }

			tData( const tAllocDump& dump )
				: mAlloc( dump )
			{
				mRealAlloc.fPushBack( &dump );
			}

			b32 operator < ( const tData& right )
			{
				return mRealAlloc.fCount( ) > right.mRealAlloc.fCount( );
			}
		};

		const tMemoryDump& mDump;
		tGrowableArray< tData > mBuckets;

		const tGrowableArray< tData >& fBuckets( ) const { return mBuckets; }

		tSortByType( const tHeapDump& heap, const tMemoryDump& dump )
			: mDump( dump )
		{
			mBuckets.fSetCapacity( 0 );

			for( u32 p = 0; p < heap.mPages.fCount( ); ++p )
			{
				for( u32 i = 0; i < heap.mPages[ p ].mAllocations.fCount( ); ++i )
					fInsertAllocation( heap.mPages[ p ].mAllocations[ i ] );
			}

			fSort( );
		}

		tSortByType( const tMemoryDump& dump )
			: mDump( dump )
		{
			for( u32 i = 0; i < dump.mHeaps.fCount( ); ++i )
			{
				tHeapDump heap = dump.mHeaps[ i ];

				for( u32 j = 0; j < heap.mPages.fCount( ); ++j )
				{
					tPageDump page = heap.mPages[ j ];

					for( u32 k = 0; k < page.mAllocations.fCount( ); ++k )
						fInsertAllocation( page.mAllocations[ k ] );
				}
			}

			fSort( );
		}

		s32 fTotalConsumed( ) const
		{
			s32 total = 0;
			for( u32 i = 0; i < mBuckets.fCount( ); ++i )
			{
				std::string typeName = mBuckets[ i ].mAlloc.mStamp.fTypeString( mDump );
				if( typeName == "Page" || typeName == "Page Header" )
					continue;

				total += mBuckets[ i ].mConsumed;
			}

			return total;
		}

		void fInsertAllocation( const tAllocDump& alloc )
		{
			// Don't include pages or page headers in the allocation tracking
			std::string typeName = alloc.mStamp.fTypeString( mDump );
			if( typeName == "Page" || typeName == "Page Header" )
				return;

			u32 bucket = ~0;
			for( u32 a = 0; a < mBuckets.fCount( ); ++a )
			{
				if( mBuckets[ a ].mAlloc.mStamp == alloc.mStamp )
				{
					bucket = a;
					break;
				}
			}
			if( bucket != ~0 )
			{
				// add to old bucket
				mBuckets[ bucket ].mRealAlloc.fPushBack( &alloc );
				mBuckets[ bucket ].mConsumed += alloc.mStamp.mSize;
			}
			else
			{
				// new bucket
				mBuckets.fPushBack( tData( alloc ) );
				mBuckets[ mBuckets.fCount( ) - 1 ].mConsumed = alloc.mStamp.mSize;
			}
		}

		void fSort( )
		{
			std::sort( mBuckets.fBegin( ), mBuckets.fEnd( ) );
		}

		void fPopTree( wxTreeCtrl* tree, wxTreeItemId parent )
		{
			for( u32 i = 0; i < mBuckets.fCount( ); ++i )
			{
				Memory::tAllocDump& alloc = mBuckets[ i ].mAlloc;

				wxString dataStr = alloc.fToSoftString( mDump );
				dataStr += "\n Count: " + StringUtil::fToString( mBuckets[ i ].mRealAlloc.fCount( ) ) + " Size: " + StringUtil::fToString( mBuckets[ i ].mConsumed / c1MB ) + " mb";

				wxTreeItemId node = tree->AppendItem( parent, dataStr );
				tree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypeAlloc, mBuckets[ i ].mRealAlloc[ 0 ], mDump ) );

				for( u32 s = 0; s < mBuckets[ i ].mRealAlloc.fCount( ); ++s )
				{
					const Memory::tAllocDump* realAlloc = mBuckets[ i ].mRealAlloc[ s ];
					wxTreeItemId node2 = tree->AppendItem( node, realAlloc->fToSoftString( mDump ) );
					tree->SetItemData( node2, new tProfilerNodeData( tNodeData::cTypeAlloc, realAlloc, mDump ) );
				}
			}
		}
	};

	struct tDiffData : tNodeData
	{
		s32 mDiffBytes;
		s32 mAllocDelta;
		tStampDump mStamp;
		b32 mNew;

		inline f32 fDiffMB( ) const { return ( mDiffBytes / c1MB ); }

		tDiffData( ) 
		{ }

		tDiffData( s32 diffBytes, s32 allocCount, const tStampDump& stamp, const tMemoryDump& dump, b32 isNew )
			: tNodeData( tNodeData::cTypeDiff, dump )
			, mDiffBytes( diffBytes )
			, mAllocDelta( allocCount )
			, mStamp( stamp )
			, mNew( isNew )
		{ }

		virtual std::string fToString( ) const
		{
			std::string output;
			if( mNew )
				output += "NEW: ";
			output += mStamp.fToString( *mDump ) + "\n";
			output += "Allocation Delta: " + StringUtil::fToString( mAllocDelta ) + "\n";
			output += StringUtil::fToString( mDiffBytes ) + " (" + StringUtil::fToString( fDiffMB( ) ) + ")\n";
			output += "******************************\n";

			return output;
		}
	};

	struct DiffDataDescending
	{
		inline bool operator()( const tDiffData& a, const tDiffData& b )
		{
			return a.mDiffBytes > b.mDiffBytes;
		}
	};


	tMemoryDrawer::tMemoryDrawer( tScriptProfilerDialog* owner )
		: tWxDrawPanel( owner )
		, mOwner( owner )
		, mLastWidth( 0 )
		, mStartingAddress( 0 )
	{
	}

	void tMemoryDrawer::fLimit( const wxTreeItemId& alloc, const wxTreeItemId& heap )
	{
		mLimitAlloc = alloc;
		mLimitHeap = heap;
		mDrawList.fSetCount( 0 );

		for( u32 h = 0; h < mOwner->fDump( ).mHeaps.fCount( ); ++h )
		{
			const tHeapDump& heap = mOwner->fDump( ).mHeaps[ h ];
			if( heap.mType == tHeapDump::cTypeChunky )
			{
				if( !fShouldDrawAlloc( mLimitHeap, &heap, tNodeData::cTypeHeap ) )
					continue;

				for( u32 p = 0; p < heap.mPages.fCount( ); ++p )
				{
					const tPageDump& page = heap.mPages[ p ];
					for( u32 a = 0; a < page.mAllocations.fCount( ); ++a )
					{
						const tAllocDump& alloc = page.mAllocations[ a ];

						if( !fShouldDrawAlloc( mLimitAlloc, &alloc, tNodeData::cTypeAlloc ) )
							continue;

						mStartingAddress = page.mAddress;
						mDrawList.fPushBack( &alloc );
					}
				}

				if( !mLimitHeap.m_pItem )
					break; //if not limited only draw one heap
			}
		}

		Refresh( );
	}

	void tMemoryDrawer::fRenderRowBlock( wxDC& dc, u32 start, u32 end, u32 row, u32 col )
	{
		s32 hscroll = GetScrollPos( wxHSCROLL );
		f32 startPt = (col + (f32)start / cBytesPerRow) * cRowWidth;
		f32 endPt = (col + (f32)end / cBytesPerRow) * cRowWidth;

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

			if( nodeData->mType == type && (alloc == static_cast<const Memory::tAllocDump*>( nodeData->mData )) )
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

	void tMemoryDrawer::fRenderList( wxDC& dc )
	{
		u32 color = 0;

		const u32 maxRows = GetSize( ).y / cRowHeight - 5;
		u32 columns = 0;

		for( u32 a = 0; a < mDrawList.fCount( ); ++a )
		{
			const tAllocDump& alloc = *mDrawList[ a ];

			++color;
			color = Math::fModulus( color, cColorCount );

			dc.SetBrush( cColors[ color ] );
			dc.SetPen( wxPen( wxColor(0,0,0), 0 ) );

			u32 addressOffset = alloc.mStamp.mAddress - mStartingAddress;
			u32 row = addressOffset / cBytesPerRow;
			u32 column = row / maxRows;
			u32 effectiveRow = row % maxRows;
			u32 start = addressOffset % cBytesPerRow;
			u32 end = start + alloc.mStamp.mSize;
			u32 overage = 0;

			columns = fMax( columns, column );

			if( end > cBytesPerRow )
			{
				overage = end - cBytesPerRow;
				end = cBytesPerRow;
			}

			fRenderRowBlock( dc, start, end, effectiveRow, column );

			if( overage )
			{
				u32 fullBlocks = overage / cBytesPerRow;
				for( u32 i = 0 ; i < fullBlocks; ++i )
				{
					u32 newRow = row + i;
					u32 column = newRow / maxRows;
					effectiveRow = newRow % maxRows;
					fRenderRowBlock( dc, 0, cBytesPerRow, effectiveRow, column );
				}

				u32 extra = overage % cBytesPerRow;
				u32 newRow = row + fullBlocks;
				u32 column = newRow / maxRows;
				effectiveRow = newRow % maxRows;
				columns = fMax( columns, column );
				fRenderRowBlock( dc, 0, extra, effectiveRow, column );
			}				
		}

		// Set scroll bars
		const u32 cScrollRate = 20;
		u32 totalWidth = (columns + 1) * cRowWidth + 1;
		if( totalWidth != mLastWidth )
		{
			mLastWidth = totalWidth;
			u32 scrollUnits = totalWidth / cScrollRate;
			SetScrollbars( cScrollRate, cScrollRate, scrollUnits, 0 );
		}

		//// draw border and column dividers
		//u32 cDividerBorderWidth = 2;
		////dc.SetBrush( wxBrush( wxColor(0,0,0), wxTRANSPARENT ) );
		////dc.SetPen( wxPen( wxColor(0,0,0), cDividerBorderWidth ) );

		//const u32 totalHeight = maxRows * cRowHeight;
		////dc.DrawRectangle( 0, 0, totalWidth, totalHeight );
		//for( u32 i = 1; i < columns - 1; ++i )
		//	dc.DrawLine( i * cRowWidth, 0, i * cRowWidth, totalHeight );
	}

	void tMemoryDrawer::OnDraw( wxDC& dc )
	{
		fRenderList( dc );
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

	void tScriptProfilerDialog::fOutputDiff( const tSortByType& sort1, const tSortByType& sort2 )
	{
		tGrowableArray< tSortByType::tData > types = sort1.fBuckets( );
		tGrowableArray< tSortByType::tData > types2 = sort2.fBuckets( );
		tGrowableArray< tDiffData > diffData;
		const tMemoryDump& dump = sort1.mDump;

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
						diffData.fPushBack( tDiffData( diffBytes, diffAlloc, types2[ i ].mAlloc.mStamp, sort1.mDump, false ) );

					types2.fEraseOrdered( i-- );
					break;
				}
			}
		}

		// Append remaining allocations
		tGrowableArray< tDiffData > remaining;
		for( u32 i = 0; i < types2.fCount( ); ++i )
		{
			const tStampDump& stamp = types2[ i ].mAlloc.mStamp;
			remaining.fPushBack( tDiffData( types2[ i ].mConsumed, types2[ i ].mRealAlloc.fCount( ), stamp, dump, true ) );
		}
		
		// Sort the two lists independently and then join them so that
		// the new allocations stay at the bottom
		std::sort( diffData.fBegin( ), diffData.fEnd( ), DiffDataDescending( ) );
		std::sort( remaining.fBegin( ), remaining.fEnd( ), DiffDataDescending( ) );
		diffData.fJoin( remaining );

		fPopTree( diffData );

		s32 totalConsumed = sort2.fTotalConsumed( ) - sort1.fTotalConsumed( );
		f32 totalConsumedMb = totalConsumed / c1MB;
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

			wxBoxSizer* loadButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

			wxButton* loadBut = new wxButton( this, wxID_ANY, "Load" );
			loadBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tScriptProfilerDialog::fLoadData ), NULL, this );
			loadButtonsSizer->Add( loadBut, 0, wxEXPAND | wxALL );

			wxButton* liveBut = new wxButton( this, wxID_ANY, "Live Dump" );
			liveBut->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tScriptProfilerDialog::fGetNetworkMemoryDump ), NULL, this );
			loadButtonsSizer->Add( liveBut, 0, wxEXPAND | wxALL );

			treeSizer->Add( loadButtonsSizer );

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
		dump2.fLoad( tFilePathPtr( openDumpFileDialog->GetPath( ).c_str( ) ) );

		tSortByType dump1Sorted( mDump );
		tSortByType dump2Sorted( dump2 );

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

		const tAllocDump* allocData = static_cast< const tAllocDump* > ( nodeData->mData );

		wxString file = allocData->mStamp.fSourceFileName( mDump );
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
				const Memory::tMemoryDump* dump = static_cast<const Memory::tMemoryDump*>( nodeData->mData );
				for( u32 i = 0; i < dump->mHeaps.fCount( ); ++i )
				{
					const Memory::tHeapDump& heap = dump->mHeaps[ i ];
					wxTreeItemId node = mTree->AppendItem( parent, heap.fToSoftString( mDump ) );
					mTree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypeHeap, &heap, mDump ) );
				}
			}
			break;
		case tNodeData::cTypeHeap:
			{
				const Memory::tHeapDump* dump = static_cast<const Memory::tHeapDump*>( nodeData->mData );

				const std::string heapName = dump->fName( mDump );
				if( heapName == tResourceMemoryProvider::fInstance( ).fName( ) || heapName == tHeap::cVramHeapName )
				{
					for( u32 i = 0; i < tCategoryDump::cTypeCount; ++i )
						fAddCategory( *mTree, parent, dump, i, mDump );
				}		
				else if( heapName == tMainMemoryProvider::fInstance( ).fName( ) )
				{
					fAddCategory( *mTree, parent, dump, tCategoryDump::cByType, mDump );
					fAddCategory( *mTree, parent, dump, tCategoryDump::cBySize, mDump );
					fAddCategory( *mTree, parent, dump, tCategoryDump::cByAddress, mDump );
				}
				else
				{
					fAddCategory( *mTree, parent, dump, tCategoryDump::cBySize, mDump );
					fAddCategory( *mTree, parent, dump, tCategoryDump::cByAddress, mDump );
				}
			}
			break;	
		case tNodeData::cTypeCategory:
			{
				const tCategoryDump* cat = static_cast<const tCategoryDump*>( nodeData->mData );
				const Memory::tHeapDump* dump = cat->mHeap;

				switch( cat->mType )					
				{
				case tCategoryDump::cSummarizedView:
					{
						sigassert( cat->mSummaryData );
						cat->mSummaryData->fPopulate( *mTree, parent );
					}
					break;
				case tCategoryDump::cByType:
					{
						tSortByType doit( *dump, mDump );
						doit.fPopTree( mTree, parent );
					}
					break;
				case tCategoryDump::cBySize:
					{
						tSortBySize doit( mDump, *dump, mTree, parent );
					}
					break;
				default:
					for( u32 i = 0; i < dump->mPages.fCount( ); ++i )
					{
						const Memory::tPageDump& page = dump->mPages[ i ];
						wxTreeItemId node = mTree->AppendItem( parent, page.fToSoftString( mDump ) );
						mTree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypePage, &page, mDump ) );
					}
				}
			}
			break;
		case tNodeData::cTypePage:
			{
				const Memory::tPageDump* dump = static_cast<const Memory::tPageDump*>( nodeData->mData );
				tSortByAddress doit( mDump, dump->mAllocations, mTree, parent );
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

	void tScriptProfilerDialog::fPopTree( const tGrowableArray< tDiffData >& diffData )
	{
		mTree->DeleteAllItems( );

		wxTreeItemId rootNode = mTree->AddRoot( "Root Node" );

		Freeze( );

		mTree->DeleteChildren( rootNode );

		for( u32 i = 0; i < diffData.fCount( ); ++i )
		{
			const tDiffData& data = diffData[ i ];
			sigcheckfail( data.mDump, continue );

			log_line( 0, data.fToString( ) );

			wxTreeItemId node = mTree->AppendItem( rootNode, data.mStamp.fSourceFileName( *data.mDump ) + ": " + data.mStamp.fLineNumString( ) );
			mTree->SetItemData( node, NEW_TYPED( tDiffData )( data ) );
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
		mTree->SetItemData( rootNode, new tProfilerNodeData( tNodeData::cTypeRoot, &mDump, mDump ) );

		mDump = Memory::tMemoryDump( );
		//mDump.fLoadXml( tFilePathPtr( openDumpFileDialog->GetPath( ) ) );
		mDump.fLoad( tFilePathPtr( openDumpFileDialog->GetPath( ) ) );

		fPopTree( rootNode );
	}

	void tScriptProfilerDialog::fLoadData( tDynamicArray<byte>& data )
	{
		mTree->DeleteAllItems( );

		wxTreeItemId rootNode = mTree->AddRoot( "Root Node" );
		mTree->SetItemData( rootNode, new tProfilerNodeData( tNodeData::cTypeRoot, &mDump, mDump ) );

		tGameArchiveLoad load( data.fBegin( ), data.fCount( ) );
		mDump.fLoad( data );

		fPopTree( rootNode );
	}

	
	void tScriptProfilerDialog::fGetNetworkMemoryDump( wxCommandEvent& event )
	{
		if( mMemoryDumpIP.Length( ) <= 0 )
			mMemoryDumpIP = wxGetTextFromUser( "Enter the IP address:", "Enter IP" );

		tDynamicArray<byte> results;

		const b32 status = tPlatformDebuggingTools::fGetMemoryDump( std::string( mMemoryDumpIP ), results );
		if( status )
		{
			if( results.fCount( ) )
				fLoadData( results );
		}
		else
		{
			// couldn't get a response, maybe wrong ip.
			mMemoryDumpIP.Clear( );
		}
		
	}

	BEGIN_EVENT_TABLE( tScriptProfilerDialog, wxDialog )
		EVT_HYPERLINK( wxID_ANY, tScriptProfilerDialog::fHyperlinkClicked )
		EVT_MENU( cDiffDump, tScriptProfilerDialog::fDiffDump )
	END_EVENT_TABLE()
} }
