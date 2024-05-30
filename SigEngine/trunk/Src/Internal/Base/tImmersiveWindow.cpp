#include "BasePch.hpp"
#if defined( platform_metro )
#include "tImmersiveWindow.hpp"
#include "GameStatistics.hpp"

using namespace Windows::UI::ViewManagement;

namespace Sig
{
	tImmersiveWindow::tImmersiveWindow( )
	{
	}
	tImmersiveWindow::~tImmersiveWindow( )
	{
	}

	void tImmersiveWindow::fAssume( Windows::UI::Core::CoreWindow^ window )
	{
	}

	void tImmersiveWindow::fClose()
	{
		fGetCoreWindow()->Close();
	}

	void tImmersiveWindow::fOnTick() const
	{
		fGetCoreWindow()->Dispatcher->ProcessEvents( Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent );
		auto bounds = fGetCoreWindow()->Bounds;

		using namespace GameStatistics;

		if( !fIsVisible() )
			fReportWindowLayoutChanged( (u32)bounds.Width, (u32)bounds.Height, cWindowStyleMetroMinimized );
		else switch( ApplicationView::Value )
		{
		case ApplicationViewState::Filled:				fReportWindowLayoutChanged( (u32)bounds.Width, (u32)bounds.Height, cWindowStyleMetroPartial );
		case ApplicationViewState::FullScreenLandscape:	fReportWindowLayoutChanged( (u32)bounds.Width, (u32)bounds.Height, cWindowStyleMetroFullscreen );
		case ApplicationViewState::FullScreenPortrait:	fReportWindowLayoutChanged( (u32)bounds.Width, (u32)bounds.Height, cWindowStyleMetroFullscreen );
		case ApplicationViewState::Snapped:				fReportWindowLayoutChanged( (u32)bounds.Width, (u32)bounds.Height, cWindowStyleMetroSnapped );
		default:
			break;
		}
	}

	b32 tImmersiveWindow::fKeyDown( u32 key ) const
	{
		using Windows::UI::Core::CoreVirtualKeyStates;
		return (fGetCoreWindow()->GetAsyncKeyState( (Windows::System::VirtualKey)key ) & CoreVirtualKeyStates::Down) != CoreVirtualKeyStates::None;
	}

	b32 tImmersiveWindow::fFocused() const
	{
		//return Windows::UI::Core::CoreWindow::Current == mWindow; // DP4 throwing NotImplementedException here.
		return fGetCoreWindow()->IsInputEnabled; // closest equivalent I can think of
	}

	b32 tImmersiveWindow::fIsSnapped() const
	{
		b32 result = ApplicationView::Value == ApplicationViewState::Snapped;
		return result;
	}

	b32 tImmersiveWindow::fIsVisible() const
	{
		using namespace Windows::UI::ViewManagement;
		b32 result = fGetCoreWindow()->Visible;
		return result;
	}

	IUnknown* tImmersiveWindow::fGetUnknownWindow() const
	{
		// Voodoo from metro directx sample (D3DRenderer.cpp).
		// Seems to translate a given Platform::Object^ -> IUnknown*
		// 
		Platform::Object^ o = fGetCoreWindow();
		return reinterpret_cast<IUnknown*>(o);
	}
}
#endif // defined( platform_metro )
