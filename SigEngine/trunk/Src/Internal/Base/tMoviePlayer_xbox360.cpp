#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tMoviePlayer.hpp"
#include "FileSystem.hpp"
#include "Gfx/tDevice.hpp"
#include "tGameAppBase.hpp"
#include "tSceneGraph.hpp"

namespace Sig
{
	tMoviePlayer::tMoviePlayer( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad )
		: mXmvPlayer( 0 )
		, mLooping( loop )
		, mAudio( NULL )
		, mMasteringVoice( NULL )
	{
		fCreateMovie( device, system, path, t, l, b, r, loop, preLoad );
	}
	tMoviePlayer::~tMoviePlayer( )
	{
		fReleasePlayer( );
	}
	void tMoviePlayer::fCreateMovie( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad )
	{
		IDirect3DDevice9* d3ddev = device.fGetDevice( );

		HRESULT hr = XAudio2Create( &mAudio /*, flags, hwThread*/ );
		sigassert( mAudio );

		if( FAILED( hr ) || !mAudio )
			return;

		// Create a mastering voice
		hr = mAudio->CreateMasteringVoice( &mMasteringVoice );
		if( FAILED( hr ) || !mMasteringVoice )
		{
			mAudio->Release( );
			return;
		}

		// The movie volume should be based off the user music volume.
		mMasteringVoice->SetVolume( system.fUserMusicVolume() );

		sigassert( d3ddev );

        XMEDIA_XMV_CREATE_PARAMETERS xmvParams;
		fZeroOut( xmvParams );

		xmvParams.dwFlags =  XMEDIA_CREATE_SHARE_IO_CACHE;
		xmvParams.dwAudioDecoderCpu = cCPU;
		xmvParams.dwVideoDecoderCpu = cCPU;
		xmvParams.dwAudioRendererCpu = cCPU;
		xmvParams.dwVideoRendererCpu = cCPU;

		tGameAppBase::fInstance( ).fSceneGraph( )->fDisallowLogicThread( cCPU );

        // Use the default audio and video streams.
        // If using a wmv file with multiple audio or video streams
        // (such as different audio streams for different languages)
        // the dwAudioStreamId & dwVideoStreamId parameters can be used 
        // to select which audio (or video) stream will be played back

        xmvParams.dwAudioStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;
        xmvParams.dwVideoStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;

		// Specify that this should be a looping movie.
		if( loop )
			xmvParams.dwFlags |= XMEDIA_CREATE_FOR_LOOP;

		tFilePathPtr absPath = tFilePathPtr::fConstructPath( tApplication::fInstance( ).fGameRoot( ), path );

		if( !FileSystem::fFileExists( absPath ) )
		{
			log_warning( "Video file doesnt exist: " << absPath );
			return;
		}

		if( false )
		{
			if( !FileSystem::fReadFileToBuffer( mInMemoryBuffer, absPath ) )
				return;

            xmvParams.createType = XMEDIA_CREATE_FROM_MEMORY;
            xmvParams.createFromMemory.pvBuffer = mInMemoryBuffer.fBegin( );
            xmvParams.createFromMemory.dwBufferSize = mInMemoryBuffer.fCount( );
		}
		else
		{
            // Set the parameters to load the movie from a file.
            xmvParams.createType = XMEDIA_CREATE_FROM_FILE;
            xmvParams.createFromFile.szFileName = absPath.fCStr( );
			xmvParams.createFromFile.dwIoBlockSize = DWORD( 0.5 * 1024 * 1024 );
			xmvParams.createFromFile.dwIoBlockCount = 2;
			xmvParams.createFromFile.dwIoBlockJitter = 2;
		}

		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextMovie ) );
		hr = XMedia2CreateXmvPlayer( d3ddev, mAudio, &xmvParams, &mXmvPlayer );
		Memory::tHeap::fResetVramContext( );

		if( FAILED( hr ) )
		{
			log_warning( "Movie player couldn't be created HR: " << hr );
			fReleasePlayer( );
			return;
		}

		fRefreshVideoRect( t, l, b, r );
	}
	void tMoviePlayer::fPause( b32 pause )
	{
		if( mXmvPlayer )
		{
			XMEDIA_PLAYBACK_STATUS playbackStatus;
			if( SUCCEEDED( mXmvPlayer->GetStatus( &playbackStatus ) ) )
			{
				if( !pause )
				{
					if( XMEDIA_PLAYER_PAUSED == playbackStatus.Status )
						mXmvPlayer->Resume();
				}
				else
				{
					if( XMEDIA_PLAYER_PAUSED != playbackStatus.Status )
						mXmvPlayer->Pause();
				}
			}
		}
	}
	b32 tMoviePlayer::fPaused( ) const
	{
		if( mXmvPlayer )
		{
			XMEDIA_PLAYBACK_STATUS playbackStatus;
			if( SUCCEEDED( mXmvPlayer->GetStatus( &playbackStatus ) ) )
			{
				if( playbackStatus.Status == XMEDIA_PLAYER_PAUSING ||
					playbackStatus.Status == XMEDIA_PLAYER_PAUSED )
					return true;
			}
		}

		return false;
	}
	void tMoviePlayer::fCancel( )
	{
		if( !mXmvPlayer )
			return;
		mXmvPlayer->Stop( XMEDIA_STOP_IMMEDIATE );
		fReleasePlayer( );
	}
	void tMoviePlayer::fRefreshVideoRect( u32 t, u32 l, u32 b, u32 r )
	{
		if( !mXmvPlayer )
			return;

		//const u32 width = r - l;
		//const u32 height = b - t;
		//const s32 hWidth = width / 2;
		//const s32 hHeight = height / 2;

		const f32 xScale = 1.f; // TODO another way to parameterize
		const f32 yScale = 1.f; // TODO another way to parameterize

		// Scale the output width.
		const float left = ( f32 )l;
		const float right = ( f32 )r;
		const float top = ( f32 )t;
		const float bottom = ( f32 )b;

		const f32 angle = 0.f; // TODO another way to parameterize
		const float cosTheta = std::cos( angle );
		const float sinTheta = std::sin( angle );

		// Apply the scaling and rotation.
		mVideoScreen.aVertices[ 0 ].fX = left;//hWidth + ( left * cosTheta - top * sinTheta );
		mVideoScreen.aVertices[ 0 ].fY = top;//hHeight + ( top * cosTheta + left * sinTheta );
		mVideoScreen.aVertices[ 0 ].fZ = 0;

		mVideoScreen.aVertices[ 1 ].fX = right;//hWidth + ( right * cosTheta - top * sinTheta );
		mVideoScreen.aVertices[ 1 ].fY = top;//hHeight + ( top * cosTheta + right * sinTheta );
		mVideoScreen.aVertices[ 1 ].fZ = 0;

		mVideoScreen.aVertices[ 2 ].fX = left;//hWidth + ( left * cosTheta - bottom * sinTheta );
		mVideoScreen.aVertices[ 2 ].fY = bottom;//hHeight + ( bottom * cosTheta + left * sinTheta );
		mVideoScreen.aVertices[ 2 ].fZ = 0;

		mVideoScreen.aVertices[ 3 ].fX = right;//hWidth + ( right * cosTheta - bottom * sinTheta );
		mVideoScreen.aVertices[ 3 ].fY = bottom;//hHeight + ( bottom * cosTheta + right * sinTheta );
		mVideoScreen.aVertices[ 3 ].fZ = 0;

		// Always leave the UV coordinates at the default values.
		const f32 uvFudge = 0.05f;
		mVideoScreen.aVertices[ 0 ].fTu = 0 + uvFudge;
		mVideoScreen.aVertices[ 0 ].fTv = 0 + uvFudge;
		mVideoScreen.aVertices[ 1 ].fTu = 1 - uvFudge;
		mVideoScreen.aVertices[ 1 ].fTv = 0 + uvFudge;
		mVideoScreen.aVertices[ 2 ].fTu = 0 + uvFudge;
		mVideoScreen.aVertices[ 2 ].fTv = 1 - uvFudge;
		mVideoScreen.aVertices[ 3 ].fTu = 1 - uvFudge;
		mVideoScreen.aVertices[ 3 ].fTv = 1 - uvFudge;

		// Tell the XMV player to use the new settings.
		// This locks the vertex buffer so it may cause stalls if called every frame.
		mXmvPlayer->SetVideoScreen( &mVideoScreen );
	}
	void tMoviePlayer::fRenderFrame( Gfx::tDevice& device )
	{
		if( !mXmvPlayer )
			return;

        // If RenderNextFrame does not return S_OK then the frame was not
        // rendered (perhaps because it was cancelled) so a regular frame
        // buffer should be rendered before calling present.
		const HRESULT hr = mXmvPlayer->RenderNextFrame( mLooping ? XMEDIA_PLAY_LOOP : 0, NULL );

		device.fInvalidateLastRenderState( );

		IDirect3DDevice9* d3ddev = device.fGetDevice( );
		sigassert( d3ddev );

        // Reset our cached view of what pixel and vertex shaders are set, because
        // it is no longer accurate, since XMV will have set their own shaders.
        // This avoids problems when the shader cache thinks it knows what shader
        // is set and it is wrong.
        d3ddev->SetVertexShader( 0 );
        d3ddev->SetPixelShader( 0 );
        d3ddev->SetVertexDeclaration( 0 );

        // Movie playback changes various D3D states, so you should reset the
        // states that you need after movie playback is finished.
        d3ddev->SetRenderState( D3DRS_VIEWPORTENABLE, TRUE );

        if( FAILED( hr ) || hr == ( HRESULT )XMEDIA_W_EOF )
			fReleasePlayer( );
	}
	void tMoviePlayer::fReleasePlayer( )
	{
		tGameAppBase::fInstance( ).fSceneGraph( )->fAllowLogicThread( cCPU );

		if( mXmvPlayer ) mXmvPlayer->Release( );
		mXmvPlayer = NULL;

		if( mMasteringVoice ) mMasteringVoice->DestroyVoice( );
		mMasteringVoice = NULL;
		
		if( mAudio ) mAudio->Release( );
		mAudio = NULL;

		mInMemoryBuffer.fDeleteArray( );
	}
}
#endif
