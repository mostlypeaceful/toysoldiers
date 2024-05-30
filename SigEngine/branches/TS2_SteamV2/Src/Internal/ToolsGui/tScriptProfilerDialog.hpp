#ifndef __tScriptProfilerDialog__
#define __tScriptProfilerDialog__

#include "wx/treectrl.h"
#include "wx/hyperlink.h"
#include "wx/regex.h"
#include "tScriptContextStack.hpp"
#include "Memory/tMemoryDump.hpp"
#include "tWxDrawPanel.hpp"

using namespace Sig::Memory;

namespace Sig { namespace ScriptData
{
	struct toolsgui_export tNodeData : wxTreeItemData
	{
		enum tType
		{
			cTypeRoot,
			cTypeHeap,
			cTypePage,
			cTypeAlloc,
			cTypeDiff,
		};

		u32 mType;

		tNodeData( u32 type ) : mType( type ) { }

		virtual std::string fToString( ) { return ""; }
	};

	struct toolsgui_export tProfilerNodeData : public tNodeData
	{
		Memory::tBaseDump*	mData;

		virtual std::string fToString( ) { return mData->fToString( ); }

		tProfilerNodeData( u32 type, Memory::tBaseDump* data )
			: tNodeData( type )
			, mData( data )
		{ }
	};

	struct tData
	{
		tAllocDump mAlloc;
		tGrowableArray<tAllocDump*> mRealAlloc;
		u32 mConsumed;

		tData( )
		{ }

		tData( tAllocDump& dump )
			: mAlloc( dump )
		{
			mRealAlloc.fPushBack( &dump );
		}

		b32 operator < ( const tData& right )
		{
			return mRealAlloc.fCount( ) > right.mRealAlloc.fCount( );
		}
	};

	struct tSortByType
	{
		tGrowableArray< tData > mBuckets;

		const tGrowableArray< tData >& fBuckets( ) const { return mBuckets; }

		tSortByType( tGrowableArray<tAllocDump>& allocs )
		{
			mBuckets.fSetCapacity( 0 );

			for( u32 i = 0; i < allocs.fCount( ); ++i )
				fInsertAllocation( allocs[ i ] );

			fSort( );
		}

		tSortByType( tMemoryDump* dump )
		{
			for( u32 i = 0; i < dump->mHeaps.fCount( ); ++i )
			{
				tHeapDump heap = dump->mHeaps[ i ];

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
				if( mBuckets[ i ].mAlloc.mStamp.mTypeName == "Page" || mBuckets[ i ].mAlloc.mStamp.mTypeName == "Page Header" )
					continue;

				total += mBuckets[ i ].mConsumed;
			}

			return total;
		}

		void fInsertAllocation( tAllocDump& alloc )
		{
			// Don't include pages or page headers in the allocation tracking
			if( alloc.mStamp.mTypeName == "Page" || alloc.mStamp.mTypeName == "Page Header" )
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

				wxString dataStr = alloc.fToSoftString( );
				dataStr += "\n Count: " + StringUtil::fToString( mBuckets[ i ].mRealAlloc.fCount( ) );

				wxTreeItemId node = tree->AppendItem( parent, dataStr );
				tree->SetItemData( node, new tProfilerNodeData( tNodeData::cTypeAlloc, mBuckets[ i ].mRealAlloc[ 0 ] ) );

				for( u32 s = 0; s < mBuckets[ i ].mRealAlloc.fCount( ); ++s )
				{
					Memory::tAllocDump* realAlloc = mBuckets[ i ].mRealAlloc[ s ];
					wxTreeItemId node2 = tree->AppendItem( node, realAlloc->fToSoftString( ) );
					tree->SetItemData( node2, new tProfilerNodeData( tNodeData::cTypeAlloc, realAlloc ) );
				}
			}
		}
	};

	struct tDiffData : tNodeData
	{
		s32 mDiffBytes;
		s32 mAllocDelta;
		tStampDump mStamp;

		inline f32 fDiffMB( ) { return ( mDiffBytes / ( 1024.f * 1024.f ) ); }

		tDiffData( ): tNodeData( tNodeData::cTypeDiff ) { }

		tDiffData( s32 diffBytes, s32 allocCount, tStampDump& stamp )
			:	tNodeData( tNodeData::cTypeDiff ),
			mDiffBytes( diffBytes ),
			mAllocDelta( allocCount ),
			mStamp( stamp )
		{
		}

		virtual std::string fToString( )
		{
			std::string output = mStamp.fToString( ) + "\n";
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

	class tScriptProfilerDialog;

	class toolsgui_export tMemoryDrawer : public tWxDrawPanel
	{
	public:
		tMemoryDrawer( tScriptProfilerDialog* owner );

		void fLimit( const wxTreeItemId& alloc, const wxTreeItemId& heap );

	protected:
		tScriptProfilerDialog* mOwner;
		wxTreeItemId mLimitAlloc;
		wxTreeItemId mLimitHeap;

		void fRenderRowBlock( wxDC& dc, u32 start, u32 end, u32 row );
		void fRenderHeap( wxDC& dc, const Memory::tHeapDump& heap );
		virtual void fRender( wxDC& dc );

		b32 fShouldDrawAlloc( wxTreeItemId& item, const Memory::tBaseDump* alloc, u32 type ) const;
	};

	class toolsgui_export tScriptProfilerDialog : public wxDialog
	{
	public:
		typedef tDelegate<void ( const tFilePathPtr& path )> tOpenDocCallback;
		typedef tDelegate<void ( u64 line )> tFocusAllOnLineCallback;

		tScriptProfilerDialog( wxWindow* parent );

		wxTreeCtrl* fTree( ) { return mTree; }

		void fShow( tOpenDocCallback fOpenDoc, tFocusAllOnLineCallback fFocusAllOnLine );

		const Memory::tMemoryDump& fDump( ) const { return mDump; }

	private:
		wxTreeCtrl* mTree;
		wxTextCtrl* mTextBox;
		wxHyperlinkCtrl* mHyperlink;
		tMemoryDrawer* mDrawPanel;
		Memory::tMemoryDump mDump;
		tOpenDocCallback fOpenDoc;
		tFocusAllOnLineCallback fFocusAllOnLine;

		wxComboBox* mShowOptions;
		wxComboBox* mSortOptions;

		wxTreeItemId mLastIdProcessed;

		void fClearStats( );
		void fWriteStat( const std::string& text );
		void fPopulateStats( wxTreeItemId item );

		void fPopTree( wxTreeItemId parent );
		void fPopTree( tGrowableArray< tDiffData >& diffData );

		void fLoadData( wxCommandEvent& event );

		wxFileDialog* fOpenDumpDialog( );
		void fTreeItemClicked( wxCommandEvent& event );
		void fHyperlinkClicked( wxHyperlinkEvent& event );
		void fOnItemRightClick( wxContextMenuEvent& event );
		void fDiffDump( wxCommandEvent& event );
		void fOutputDiff( tSortByType sort1, tSortByType sort2 );

		DECLARE_EVENT_TABLE()
	};
} }

#endif//__tScriptProfilerDialog__
