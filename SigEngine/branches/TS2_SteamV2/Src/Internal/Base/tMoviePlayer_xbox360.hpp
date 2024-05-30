#ifndef __tMoviePlayer_xbox360__
#define __tMoviePlayer_xbox360__
#include "xmedia2.h"

namespace Sig
{
	class tMoviePlayer : public tRefCounter
	{
		IXMedia2XmvPlayer*		mXmvPlayer;
		XMEDIA_VIDEO_SCREEN		mVideoScreen;
		tDynamicBuffer			mInMemoryBuffer;
		b32						mLooping;
		IXAudio2*				mAudio;
		IXAudio2MasteringVoice* mMasteringVoice;
	public:
		enum { cIsSupported = true };
	public:
		tMoviePlayer( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad = false );
		~tMoviePlayer( );
		b32	 fFinishedPlaying( ) const { return mXmvPlayer==0; }
		void fPause( b32 unpause = false );
		void fCancel( );
		void fRefreshVideoRect( u32 t, u32 l, u32 b, u32 r );
		void fRenderFrame( Gfx::tDevice& device );
	private:
		void fCreateMovie( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad );
		void fReleasePlayer( );

		static const u32 cCPU = 3;
	};
}

#endif//__tMoviePlayer_xbox360__
