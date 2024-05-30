#ifndef __tFpsText__
#define __tFpsText__
#include "tFpsCounter.hpp"
#include "tText.hpp"

namespace Sig { namespace Gui
{
	///
	/// \brief Simple utility class that wraps a text object
	/// and an fps counter to make printing fps text very simple.
	class base_export tFpsText : public tText
	{
	private:
		tFpsCounter mCounter;
	public:
		tFpsText( );
		explicit tFpsText( Gfx::tDefaultAllocators& allocators );
		void fPreRender( Gfx::tScreen& screen, f32 x = 10.f, f32 y = 10.f );
		void fPostRender( );
		inline f32 fFps( ) const { return mCounter.fFps( ); }
	};

	typedef tRefCounterPtr<tFpsText> tFpsTextPtr;
}}

#endif//__tFpsText__
