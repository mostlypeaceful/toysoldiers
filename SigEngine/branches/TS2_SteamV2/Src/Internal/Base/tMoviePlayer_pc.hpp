#ifndef __tMoviePlayer_pc__
#define __tMoviePlayer_pc__

#if defined(target_game)
#define USE_THEORA
#endif

#if defined(USE_THEORA)
#include <ogg/ogg.h>
#include <theora/theoradec.h>
#include <vorbis/codec.h>
#include <map>
#include "Gfx/tDeviceResource.hpp"
#include "Gfx/tDevice.hpp"

namespace Sig
{
	class tMoviePlayer : public tRefCounter, public Gfx::tDeviceResource
	{
		u32 mTop, mLeft, mRight, mBottom;
		b32 mLoop;
		b32 mPreload;
		b32 mPagesFinished;
		b32 mComplete;

		std::map<int, class tOggStream *> mStreams;
		ogg_sync_state mOggSyncState;
		HANDLE hFile;
		double mPlayTime;

	public:
		enum { cIsSupported = true };
		void fTick( );
		void fDraw( );

	public:
		tMoviePlayer( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad = false );
		virtual ~tMoviePlayer( );
		b32	 fFinishedPlaying( ) const { return mComplete; }
		void fPause( b32 unpause = false );
		b32 fPaused( ) const;
		void fCancel( );
		void fRefreshVideoRect( u32 t, u32 l, u32 b, u32 r );
		void fRenderFrame( Gfx::tDevice& device );
		void fOnGraphEvent( );
		static tMoviePlayer* fInstance( );

		virtual void fOnDeviceLost( Gfx::tDevice* device );
		virtual void fOnDeviceReset( Gfx::tDevice* device );

	private:
		void fReadPage();
		bool fAnyStreamFinishedHeader();
	};
}

#else

struct IGraphBuilder;
struct IMediaControl;
struct IMediaEventEx;
struct IVMRWindowlessControl;
struct IBaseFilter;
struct IVMRSurfaceAllocatorNotify9;

namespace Sig
{
	class tMoviePresenter;
	class tMoviePlayer : public tRefCounter
	{
		IGraphBuilder* mGraphBuilder;
		IMediaControl* mMediaControl;
		IMediaEventEx* mMediaEvent;
		IBaseFilter* mVMR;
		IVMRSurfaceAllocatorNotify9* mNotify;
		tMoviePresenter *mMoviePresenter;
		b32 mComplete;

	public:
		enum { cIsSupported = true };

	public:
		tMoviePlayer( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad = false );
		virtual ~tMoviePlayer( );
		b32	 fFinishedPlaying( ) const;
		void fPause( b32 unpause = false );
		b32 fPaused( ) const;
		void fCancel( );
		void fRefreshVideoRect( u32 t, u32 l, u32 b, u32 r );
		void fRenderFrame( Gfx::tDevice& device );
		void fOnGraphEvent( );
		static tMoviePlayer* fInstance( );
		void fTick( ) { }

	private:
		b32 tMoviePlayer::fFailed( HRESULT hr, const char *msg );
		void fCreateMovie( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad );
		void fReleasePlayer( );
	};
}
#endif

#endif //__tMoviePlayer_pc__
