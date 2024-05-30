#if defined( platform_metro )
#ifndef __tImmersiveWindow__
#define __tImmersiveWindow__

#include <wrl.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>

namespace Sig
{
	///
	/// \brief Represents a single windows 8 metro-style "immersive window"
	class base_export tImmersiveWindow : public tUncopyable
	{
	public:
		tImmersiveWindow( );
		~tImmersiveWindow( );

		void fAssume( Windows::UI::Core::CoreWindow^ window );
		void fClose();
		void fOnTick() const;

		b32 fKeyDown( u32 vkey ) const;
		b32 fFocused() const;

		/// \brief If we're in the 320-wide metro snapped panel
		b32 fIsSnapped() const;

		b32 fIsVisible() const;

		IUnknown* fGetUnknownWindow() const;
		inline Windows::UI::Core::CoreWindow^ fGetCoreWindow() const { return Windows::UI::Core::CoreWindow::GetForCurrentThread(); }
	};
}

#endif//__tImmersiveWindow__
#endif//#if defined( platform_metro )
