#include "ToolsGuiPch.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxToolsPanel.hpp"
#include "tWxToolsPanelContainer.hpp"

namespace Sig
{

	tWxToolsPanelTool::tWxToolsPanelTool( 
		tWxToolsPanel* parent,
		const char* label,
		const wxString& iconToolTip, 
		const wxString& iconBitmapResource )
		: tWxSlapOnGroup( parent, label, true )
		, mParent( parent )
		, mRegKeyName( parent->fGuiApp( ).fRegKeyName( ) + "\\ToolsPanelTool\\" + label )
		, mIconName( label )
		, mIconToolTip( iconToolTip )
		, mIconBitmapResource( iconBitmapResource )
		, mActionId( 0 )
		, mShowByDefault( true )
	{
		sigassert( parent );
		parent->mTools.fPushBack( this );
	}

	void tWxToolsPanelTool::fShow( wxToolBar* toolBar, b32 show )
	{
		if( show != ( fGetMainPanel( )->IsShown( )!=0 ) )
		{
			fHandleAction( mActionId );
			toolBar->ToggleTool( mActionId, fGetMainPanel( )->IsShown( ) );
		}
	}

	void tWxToolsPanelTool::fAddToToolBar( wxToolBar* toolBar, u32& currentActionId )
	{
		mActionId = currentActionId++;
		toolBar->AddCheckTool( mActionId, mIconName, wxBitmap( mIconBitmapResource ), wxNullBitmap, mIconToolTip );

		if( mShowByDefault )
		{
			toolBar->ToggleTool( mActionId, true );
			++mParent->mNumToolsOn;
		}
		else
		{
			toolBar->ToggleTool( mActionId, false );
			fGetMainPanel( )->Show( false );
		}
	}

	b32 tWxToolsPanelTool::fHandleAction( u32 actionId )
	{
		if( mActionId == actionId )
		{
			fGetMainPanel( )->Show( !fGetMainPanel( )->IsShown( ) );
			if( fGetMainPanel( )->IsShown( ) )
				++mParent->mNumToolsOn;
			else
				--mParent->mNumToolsOn;
			fSave( );
			return true;
		}

		return false;
	}

	tToolsGuiApp& tWxToolsPanelTool::fGuiApp( )
	{
		return mParent->fGuiApp( );
	}

	const tToolsGuiApp& tWxToolsPanelTool::fGuiApp( ) const
	{
		return mParent->fGuiApp( );
	}

	std::string tWxToolsPanelTool::fRegistryKeyName( ) const
	{
		return mRegKeyName;
	}

	void tWxToolsPanelTool::fSaveInternal( HKEY hKey )
	{
		Win32Util::fSetRegistryKeyValue( hKey, ( b32 )( fGetMainPanel( )->IsShown( )!=0 ), "Show" );
	}
	void tWxToolsPanelTool::fLoadInternal( HKEY hKey )
	{
		Win32Util::fGetRegistryKeyValue( hKey, mShowByDefault, "Show" );
	}


	tWxToolsPanel::tWxToolsPanel( tWxToolsPanelContainer* parent, u32 width, wxColour bkgndColor, wxColour frgndColor )
		: wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxSize( width, wxDefaultSize.y ), wxBORDER_SIMPLE /*| wxTAB_TRAVERSAL */ )
		, mContainer( parent )
		, mNumToolsOn( 0 )
	{
		SetMaxSize( wxSize( width, -1 ) );
		SetScrollbars( 0, 20, 0, 0 );

		SetBackgroundColour( bkgndColor );
		SetForegroundColour( frgndColor );

		sigassert( parent->GetSizer( ) );
		parent->GetSizer( )->Add( this, 0, wxEXPAND | wxTOP | wxBOTTOM | wxALIGN_RIGHT | wxFIXED_MINSIZE, 0 );
		parent->mToolsPanels.fPushBack( this );
	}

	void tWxToolsPanel::fAddToToolBar( wxToolBar* toolBar, u32& currentActionId )
	{
		for( u32 i = 0; i < mTools.fCount( ); ++i )
		{
			mTools[ i ]->fLoad( );
			mTools[ i ]->fAddToToolBar( toolBar, currentActionId );
		}
	}

	b32 tWxToolsPanel::fHandleAction( u32 actionId )
	{
		for( u32 i = 0; i < mTools.fCount( ); ++i )
		{
			if( mTools[ i ]->fHandleAction( actionId ) )
			{
				if( IsShown( ) && mNumToolsOn == 0 )
					Show( false );
				else if( !IsShown( ) && mNumToolsOn > 0 )
					Show( true );
				fUpdateScrollBars( );
				return true;
			}
		}

		return false;
	}

	void tWxToolsPanel::fOnTick( )
	{
		fCheckForCursorFocus( );
		for( u32 i = 0; i < mTools.fCount( ); ++i )
			mTools[ i ]->fOnTick( );
	}

	tToolsGuiApp& tWxToolsPanel::fGuiApp( )
	{
		return mContainer->fGuiApp( );
	}

	namespace
	{
		static const u32 cPixelsPerScrollUnit = 20;
	}

	void tWxToolsPanel::fUpdateScrollBars( )
	{
		if( mNumToolsOn == 0 )
			return;

		SetScrollbars( 0, cPixelsPerScrollUnit, 0, fComputeScrollHeight( ), 0, GetScrollPos( wxVERTICAL ) );
	}

	u32 tWxToolsPanel::fComputeScrollHeight( )
	{
		u32 o = 0;
		for( u32 i = 0; i < mTools.fCount( ); ++i )
			if( mTools[ i ]->fGetMainPanel( )->IsShown( ) )
				o += mTools[ i ]->fGetMainPanel( )->GetSize( ).y;

		if( ( s32 )o < GetParent( )->GetSize( ).y - 200 )
			return 0;
		else
			return o / ( cPixelsPerScrollUnit / 2 + cPixelsPerScrollUnit / 3 ) + 3;
	}

	void tWxToolsPanel::fCheckForCursorFocus( )
	{
		if( fGuiApp( ).fMainWindow( ).fPriorityInputActive( ) )
			return;
		POINT p;
		GetCursorPos( &p );
		RECT r;
		GetWindowRect( ( HWND )GetHWND( ), &r );
		if( fInBounds( p.x, r.left, r.right ) && fInBounds( p.y, r.top, r.bottom ) )
			SetFocus( );
	}

	tWxUnifiedToolsPanel::tWxUnifiedToolsPanel( 
		tWxToolsPanelContainer* parent, 
		const wxString& iconName, 
		const wxString& iconToolTip, 
		const wxString& iconBitmapResource, 
		u32 width, wxColour bkgndColor)
		: tWxToolsPanel( parent, width, bkgndColor )
		, mIconName( iconName )
		, mIconToolTip( iconToolTip )
		, mIconBitmapResource( iconBitmapResource )
	{
	}

	void tWxUnifiedToolsPanel::fAddToToolBar( wxToolBar* toolBar, u32& currentActionId )
	{
		mActionId = currentActionId++;
		toolBar->AddCheckTool( mActionId, mIconName, wxBitmap( mIconBitmapResource ), wxNullBitmap, mIconToolTip );
		toolBar->ToggleTool( mActionId, true );
	}

	b32 tWxUnifiedToolsPanel::fHandleAction( u32 actionId )
	{
		if( mActionId == actionId )
		{
			Show( !IsShown( ) );
			return true;
		}

		return false;
	}

}


