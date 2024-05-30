#ifndef __tMoviePlayer__
#define __tMoviePlayer__

namespace Sig
{
	namespace Gfx { class tDevice; }
	namespace Audio { class tSystem; }
}

#if defined( platform_xbox360 )

#	include "tMoviePlayer_xbox360.hpp"

#else// generic platform with no movie implementation

namespace Sig
{
	class tMoviePlayer : public tRefCounter
	{
	public:
		enum { cIsSupported = false };
	public:
		tMoviePlayer( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad = false ) { }
		b32	 fFinishedPlaying( ) const { return true; }
		void fPause( b32 pause ) { }
		b32 fPaused( ) const { return false; }
		void fCancel( ) { }
		void fRefreshVideoRect( u32 t, u32 l, u32 b, u32 r ) { }
		void fRenderFrame( Gfx::tDevice& device ) { }
	};
}

#endif// generic platform with no movie implementation

namespace Sig
{
	typedef tRefCounterPtr< tMoviePlayer > tMoviePlayerPtr;
}

#endif//__tMoviePlayer__
