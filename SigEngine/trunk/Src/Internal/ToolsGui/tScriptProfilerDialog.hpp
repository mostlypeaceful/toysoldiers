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
			cTypeCategory,
			cTypePage,
			cTypeAlloc,
			cTypeDiff,
		};

		u32 mType;
		const tMemoryDump* mDump;

		tNodeData( ) 
			: mDump( NULL ) 
		{ }

		tNodeData( u32 type, const tMemoryDump& dump ) 
			: mType( type )
			, mDump( &dump )
		{ }

		virtual std::string fToString( ) const { return ""; }
	};

	struct toolsgui_export tProfilerNodeData : public tNodeData
	{
		const Memory::tBaseDump*	mData;

		virtual std::string fToString( ) const { return mData->fToString( *mDump ); }

		tProfilerNodeData( u32 type, const Memory::tBaseDump* data, const tMemoryDump& dump )
			: tNodeData( type, dump )
			, mData( data )
		{ }
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
		u32 mLastWidth;
		tGrowableArray<const tAllocDump*> mDrawList;
		u32 mStartingAddress;

		void fRenderRowBlock( wxDC& dc, u32 start, u32 end, u32 row, u32 col );
		void fRenderList( wxDC& dc );
		virtual void OnDraw(wxDC& dc);

		b32 fShouldDrawAlloc( wxTreeItemId& item, const Memory::tBaseDump* alloc, u32 type ) const;
	};

	struct tDiffData;
	struct tSortByType;

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
		wxString mMemoryDumpIP;

		wxTreeItemId mLastIdProcessed;

		void fClearStats( );
		void fWriteStat( const std::string& text );
		void fPopulateStats( wxTreeItemId item );

		void fPopTree( wxTreeItemId parent );
		void fPopTree( const tGrowableArray< tDiffData >& diffData );

		void fLoadData( wxCommandEvent& event );
		void fLoadData( tDynamicArray<byte>& data );

		wxFileDialog* fOpenDumpDialog( );
		void fTreeItemClicked( wxCommandEvent& event );
		void fHyperlinkClicked( wxHyperlinkEvent& event );
		void fOnItemRightClick( wxContextMenuEvent& event );
		void fDiffDump( wxCommandEvent& event );
		void fOutputDiff( const tSortByType& sort1, const tSortByType& sort2 );

		void fGetNetworkMemoryDump( wxCommandEvent& event );

		DECLARE_EVENT_TABLE()
	};
} }

#endif//__tScriptProfilerDialog__
