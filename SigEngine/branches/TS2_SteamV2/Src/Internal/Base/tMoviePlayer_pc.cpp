#include "BasePch.hpp"
#if defined( platform_pcdx )
#include "tMoviePlayer.hpp"
#include "FileSystem.hpp"
#include "Gfx/tDevice.hpp"
#include "tGameAppBase.hpp"
#include "tSceneGraph.hpp"
#include "tApplication.hpp"
#include <dshow.h>
#include <map>

#if defined(USE_THEORA)

#include <mmsystem.h>
#include <dsound.h>

namespace Sig
{
	class tOggDecoder
	{
	public:
		// return FALSE if this is not a header packet
		virtual ~tOggDecoder() {}
		virtual bool fDecodeHeader(ogg_packet &packet) { return false; }
		virtual void fDecodeBody(ogg_packet &packet) { }
		virtual void fFinishStream() { }
		virtual void fOnDeviceLost() {}
		virtual void fOnDeviceReset() {}

		// milliseconds from start-of-play when we should call fDecodeBody next
		double mNextDecodeTime;
	};

	class tOggDecoderTheora : public tOggDecoder
	{
	public:
		tOggDecoderTheora( const th_info &theoraInfo, th_comment &theoraComment, th_setup_info *theoraSetup )
		{
			memcpy(&mInfo, &theoraInfo, sizeof(th_info));
			memcpy(&mComment, &theoraComment, sizeof(th_comment));
			mSetup = theoraSetup;
			mNextDecodeTime = 0;
			mHeaderDecoded = false;
		}

		~tOggDecoderTheora()
		{
			th_decode_free(mDecodeContext);
			th_comment_clear(&mComment);
			th_info_clear(&mInfo);
			mTexture->Release();
			mTexture = 0;
		}

		void fOnDeviceLost()
		{
			if (mTexture)
			{
				mTexture->Release();
				mTexture = 0;
			}
		}

		void fCreateRenderTexture( )
		{
			IDirect3DDevice9 *d3d = Gfx::tDevice::fGetDefaultDevice( )->fGetDevice( );
			HRESULT res = d3d->CreateTexture(mInfo.frame_width, mInfo.frame_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &mTexture, 0);
		}

		void fOnDeviceReset() 
		{
			if( mHeaderDecoded )
			{
				fCreateRenderTexture( );
			}
		}

		virtual bool fDecodeHeader(ogg_packet &packet)
		{
			if (th_decode_headerin( &mInfo, &mComment, &mSetup, &packet ) > 0)
			{
				log_line(0, "THEORA HEADER");
				return true;
			}

			// header finished, process it...
			mDecodeContext = th_decode_alloc(&mInfo, mSetup);
			log_line(0, "Theora stream: " << mInfo.pic_width << "x" << mInfo.pic_height << " " << (double)mInfo.fps_numerator/mInfo.fps_denominator << "fps");

			mSecondsPerFrame =  (double)mInfo.fps_denominator/(double)mInfo.fps_numerator;
			mPxfmt=mInfo.pixel_fmt;
			switch(mInfo.pixel_fmt)
			{
			case TH_PF_420: log_line(0," 4:2:0 video"); break;
			case TH_PF_422: log_line(0, " 4:2:2 video"); break;
			case TH_PF_444: log_line(0, " 4:4:4 video"); break;
			case TH_PF_RSVD:
			default:
				log_line(0," video  (UUNKNOWN Chroma sampling!)");
				break;
			}

			if(mInfo.pic_width != mInfo.frame_width || mInfo.pic_height != mInfo.frame_height)
				printf("  Frame content is %dx%d with offset (%d,%d).\n",mInfo.frame_width, mInfo.frame_height, mInfo.pic_x, mInfo.pic_y);

			mHeaderDecoded = true;
			fCreateRenderTexture( );

	//		report_colorspace(&ti);
	//		dump_comments(&tc);

			th_setup_free(mSetup);
			mSetup=0;

			return false;
		}

		void fBlitToScreen()
		{
			// PRESENT the image to the screen... just copy and stretch blit it for now...

			IDirect3DSurface9 *source;
			IDirect3DSurface9 *dest;
			Gfx::tDevicePtr device = Gfx::tDevice::fGetDefaultDevice( );
			IDirect3DDevice9 *d3d = device->fGetDevice( );

			int backbufferWidth = device->fCreationPresentParams( ).BackBufferWidth;
			int backbufferHeight = device->fCreationPresentParams( ).BackBufferHeight;
			RECT rect;
			if (backbufferWidth > backbufferHeight)
			{
				rect.left = 0;
				rect.right = backbufferWidth;
				int height = backbufferWidth*720/1280;
				int border = backbufferHeight - height;
				rect.top = border/2;
				rect.bottom = backbufferHeight-border/2;
			}
			else
			{
				rect.top = 0;
				rect.bottom = backbufferHeight;
				int width = backbufferHeight*1280/720;
				int border = backbufferWidth - width;
				rect.left = border/2;
				rect.right = backbufferWidth-border/2;
			}

			mTexture->GetSurfaceLevel(0, &source);
			d3d->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &dest );

			// clear the screen if we are not 16_9, so we will get black borders, otherwise the steam overlay bleeds
			if (rect.top != 0 || rect.left != 0)
				d3d->Clear( 0, 0, D3DCLEAR_TARGET, 0, 0, 0 );

			HRESULT hr = d3d->StretchRect( source, NULL, dest, &rect, D3DTEXF_NONE ); 
			sigassert( !FAILED( hr ) );

			source->Release();
			dest->Release();
			d3d->Present( 0, 0, 0, 0 );
		}

		// in floating point:
		//   r = y + (1.4065 * (cr - 128));
		//   g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
		//   b = y + (1.7790 * (cb - 128));
		#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)
		#define CYCbCr2R(Y, Cb, Cr) ( Y + ((92176 * Cr) >> 16) )
		#define CYCbCr2G(Y, Cb, Cr) ( Y - ((22642 * Cb) >> 16) - ((46982 * Cr)>>16) )
		#define CYCbCr2B(Y, Cb, Cr) ( Y + ((116588 * Cb) >> 16) )
		virtual void fDecodeBody(ogg_packet &packet)
		{
			//log_line(0, "THEORA BODY");
			ogg_int64_t granulePos = -1;
			int ret = th_decode_packetin(mDecodeContext,&packet,&granulePos);
			if (ret == 0)
			{
				th_ycbcr_buffer buffer;
				ret = th_decode_ycbcr_out(mDecodeContext,buffer);
				//log_line(0,"Got Y Buffer " << buffer[0].width << "x" << buffer[0].height << " @ " << buffer[0].stride);
				//log_line(0," +  U Buffer " << buffer[1].width << "x" << buffer[1].height << " @ " << buffer[1].stride);
				//log_line(0," +  V Buffer " << buffer[2].width << "x" << buffer[2].height << " @ " << buffer[2].stride);

				// write YUV to the texture

				D3DLOCKED_RECT lr;
				mTexture->LockRect( 0, &lr, 0, 0 );

				u8* out = ( u8* )lr.pBits;
				u8* inY = buffer[0].data;
				u8* inCb = buffer[1].data;
				u8* inCr = buffer[2].data;
				for (int y=0; y<buffer->height; y++)
				{
					for (int x=0; x<buffer->width; x++)
					{
						u32 *out32 = (u32*)(out + x*4);
						int y = inY[x];
						int cr = inCr[x>>1]-128;
						int cb = inCb[x>>1]-128;
						int r = CYCbCr2R(y,cr,cb);
						int g = CYCbCr2G(y,cr,cb);
						int b = CYCbCr2B(y,cr,cb);
						r = CLIP(r);
						g = CLIP(g);
						b = CLIP(b);
						*out32++ = 0xff000000 | (b<<16) | (g<<8) | (r);
					}
					inY += buffer[0].stride;
					if ((y&1) == 1)
					{
						inCb += buffer[1].stride;
						inCr += buffer[2].stride;
					}
					out += lr.Pitch;
				}

				mTexture->UnlockRect( 0 );
				mNextDecodeTime = th_granule_time( mDecodeContext, granulePos );

				fBlitToScreen();
				//log_line(0, "Read Packet: " << granulePos << " ==> " << mNextDecodeTime);
			}
		}

		th_info           mInfo;
		th_comment        mComment;
		th_setup_info *   mSetup;
		th_dec_ctx *      mDecodeContext;
		th_pixel_fmt      mPxfmt;
		IDirect3DTexture9* mTexture;
		double            mSecondsPerFrame;
		b32               mHeaderDecoded;
	};

	class tOggDecoderVorbis : public tOggDecoder
	{
	public:
		static const int AudioBufferSamples = 65536;
		static const int AudioBufferSlice = 4096;
		static const int StereoSampleSize = 4;	// stereo 16bit
		static const int StreamLookAhead = 4096; // small buffer to ensure we have some audio pushed before we need it

		tOggDecoderVorbis( const vorbis_info &vorbisInfo,  const vorbis_comment &vorbisComment )
		{
			memcpy(&mInfo, &vorbisInfo, sizeof(vorbis_info));
			memcpy(&mComment, &vorbisComment, sizeof(vorbis_comment));
			mNextDecodeTime = 0;
			mAudio = false;
			mTotalSamplesQueued = 0;
			mBufferRaw = new u16 [AudioBufferSamples*2];
			mBufferWriteEntry = 0;
			mTotalSamplesOutput = 0;

			fInitDirectSound();
		}

		~tOggDecoderVorbis()
		{
			vorbis_block_clear(&mBlock);
			vorbis_dsp_clear(&mDSPState);
			vorbis_comment_clear(&mComment);
			vorbis_info_clear(&mInfo);

			fShutdownDirectSound();
		}

		void fDecodeHeaderPacket()
		{

		}

		virtual bool fDecodeHeader(ogg_packet &packet)
		{
			if (vorbis_synthesis_headerin( &mInfo, &mComment, &packet ) == 0)
			{
				//log_line(0, "VORBIS HEADER");
				return true;
			}

			vorbis_synthesis_init(&mDSPState,&mInfo);
			vorbis_block_init(&mDSPState,&mBlock);

			//log_line(0, "VORBIS STREAM " << mInfo.channels << " channels @ " << mInfo.rate << "Hz");
			sigassert(mInfo.channels == 2);
			sigassert(mInfo.rate == 48000);

			return false;
		}

		virtual void fDecodeBody(ogg_packet &packet)
		{
			//log_line(0, "VORBIS BODY");

			int ret=0;
			if (vorbis_synthesis(&mBlock, &packet) == 0)
			{
				ret = vorbis_synthesis_blockin(&mDSPState, &mBlock);
				sigassert(ret == 0);
			}

			float **pcm = 0;
			int samples = 0;
			while ((samples = vorbis_synthesis_pcmout(&mDSPState, &pcm)) > 0)
			{
				for (int i=0;i < samples; ++i) 
				{
					for(int j=0; j < mInfo.channels; ++j) 
					{
						int v = static_cast<int>(floorf(0.5f + pcm[j][i]*32767.0f));
						if (v > 32767) v = 32767;
						if (v <-32768) v = -32768;
						mBufferRaw[mBufferWriteEntry++] = v;
					}
					mBufferWriteEntry = mBufferWriteEntry & (AudioBufferSamples*2-1);
				}
				//log_line(0, "Write audio buffer " << (samples*StereoSampleSize) << " bytes.");
				mTotalSamplesQueued += samples;
				vorbis_synthesis_read(&mDSPState,samples);
			}

			// fill the sound buffers...
			bool playNow = (mTotalSamplesOutput == 0);
			int samplesToOutput = mTotalSamplesQueued - mTotalSamplesOutput;
			while (samplesToOutput > AudioBufferSlice)
			{
				int startSample = mTotalSamplesOutput & (AudioBufferSamples-1);
				u16 *bufferPtr;
				DWORD bufferSize;

				if (mSecondaryBuffer)
				{
					mSecondaryBuffer->Lock(startSample*StereoSampleSize,AudioBufferSlice*StereoSampleSize,(void**)&bufferPtr,&bufferSize,0,0,0);
					for (int i=0; i<AudioBufferSlice; i++)
					{
						int inIdx = (startSample+i)*2;
						int outIdx = i*2;
						bufferPtr[outIdx] = mBufferRaw[inIdx];
						bufferPtr[outIdx+1] = mBufferRaw[inIdx+1];
					}
					mSecondaryBuffer->Unlock(bufferPtr,bufferSize,0,0);
				}

				mTotalSamplesOutput += AudioBufferSlice;
				samplesToOutput -= AudioBufferSlice;
			}
			if (playNow && mSecondaryBuffer)
				mSecondaryBuffer->Play(0,0,DSBPLAY_LOOPING);

			mNextDecodeTime = (double)(mTotalSamplesOutput-StreamLookAhead) / (double)mInfo.rate;
			//log_line(0, "Audio Samples " << mTotalSamplesQueued << " Time " << mNextDecodeTime);
		}

		void fFinishStream( )
		{
			if (mSecondaryBuffer)
				mSecondaryBuffer->Stop();
		}

		void fInitDirectSound()
		{
			HRESULT result;
			mDirectSound = 0;
			mDSPrimaryBuffer = 0;
			mSecondaryBuffer = 0;
 
			// Initialize the direct sound interface pointer for the default sound device.
			result = DirectSoundCreate8(NULL, &mDirectSound, NULL);
			if( FAILED(result) )
			{
				log_warning( 0, "Unable to create Direct Sound object for vorbis stream!");
				return;
			}
			else
			{
				mDirectSound->SetCooperativeLevel(tApplication::fInstance( ).fGetHwnd( ), DSSCL_PRIORITY);
			}

			// Setup the primary buffer description.
			DSBUFFERDESC bufferDesc;
			bufferDesc.dwSize = sizeof(DSBUFFERDESC);
			bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
			bufferDesc.dwBufferBytes = 0;
			bufferDesc.dwReserved = 0;
			bufferDesc.lpwfxFormat = NULL;
			bufferDesc.guid3DAlgorithm = GUID_NULL;
 
			// Get control of the primary sound buffer on the default sound device.
			result = mDirectSound->CreateSoundBuffer(&bufferDesc, &mDSPrimaryBuffer, NULL);
			if(FAILED(result))
			{
				log_warning( 0, "unable to create direct sound primary buffer!");
				return;
			}

			// Setup the format of the primary sound bufffer.
			// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
			WAVEFORMATEX waveFormat;
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nSamplesPerSec = 48000;
			waveFormat.wBitsPerSample = 16;
			waveFormat.nChannels = 2;
			waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;
 
			// Set the primary buffer to be the wave format specified.
			result = mDSPrimaryBuffer->SetFormat(&waveFormat);
			if(FAILED(result))
			{
				log_warning( 0, "Unable to set the wave format for the primary buffer!");
				return;
			}

			// need a secondary buffer for playing sound...
			bufferDesc.dwSize = sizeof(DSBUFFERDESC);
			bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
			bufferDesc.dwBufferBytes = AudioBufferSamples*2*2;
			bufferDesc.dwReserved = 0;
			bufferDesc.lpwfxFormat = &waveFormat;
			bufferDesc.guid3DAlgorithm = GUID_NULL;

			// Create a temporary sound buffer with the specific buffer settings.
			IDirectSoundBuffer* tempBuffer;
			result = mDirectSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
			if(FAILED(result))
			{
				log_warning( 0, "Failed to create secondary sound buffer!");
				return;
			}

			result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&mSecondaryBuffer);
			if(FAILED(result))
			{
				log_warning( 0, "Failed to create secondary sound buffer!");
				return;
			}

			mSecondaryBuffer->SetCurrentPosition(0);
			mSecondaryBuffer->SetVolume(DSBVOLUME_MAX);
		}

		void fShutdownDirectSound()
		{
			if (mDSPrimaryBuffer)
			{
				mDSPrimaryBuffer->Release();
				mDSPrimaryBuffer = 0;
			}

			if (mDirectSound)
			{
				mDirectSound->Release();
				mDirectSound = 0;
			}
		}

		vorbis_info       mInfo;
		vorbis_comment    mComment;
		vorbis_dsp_state  mDSPState;
		vorbis_block      mBlock;

		bool mAudio;
		int mTotalSamplesQueued;
		u16 *mBufferRaw;
		int mBufferWriteEntry;		// index into BufferRaw for next decoded stereo sample
		int mTotalSamplesOutput;	// total stereo samples output to direct sound buffers so far

		IDirectSound8 *mDirectSound;
		IDirectSoundBuffer *mDSPrimaryBuffer;
		IDirectSoundBuffer8* mSecondaryBuffer;
	};

	class tOggStream
	{
	public:
		enum tStreamType
		{
			cStreamTypeTheora,
			cStreamTypeVorbis,
			cStreamTypeUnknown
		};

		tOggStream( int serialNmbr, ogg_page &page ) : mSerial( serialNmbr )
		{
			ogg_packet packet;
			ogg_stream_init( &mStream, serialNmbr );
			ogg_stream_pagein( &mStream, &page );
			ogg_stream_packetout( &mStream, &packet );

			th_info         theoraInfo;
			th_comment      theoraComment;
			th_setup_info * theoraSetup = 0;
			th_comment_init( &theoraComment );
			th_info_init( &theoraInfo );

			vorbis_info     vorbisInfo;
			vorbis_comment  vorbisComment;
			vorbis_info_init( &vorbisInfo );
			vorbis_comment_init( &vorbisComment );

			if( vorbis_synthesis_headerin( &vorbisInfo, &vorbisComment, &packet ) == 0 )
			{
				//log_line(0, "VORBIS STREAM " << mSerial);
				mDecoder = new tOggDecoderVorbis(vorbisInfo, vorbisComment);
				mType = cStreamTypeVorbis;
			}
			else if( th_decode_headerin( &theoraInfo, &theoraComment, &theoraSetup, &packet ) > 0 )
			{
				//log_line(0, "THEORA STREAM " << mSerial);
				mDecoder = new tOggDecoderTheora( theoraInfo, theoraComment, theoraSetup );
				mType = cStreamTypeTheora;
			}
			else
			{
				//log_line(0, "UNKNOWN STREAM " << mSerial);
				mDecoder = new tOggDecoder;
				mType = cStreamTypeUnknown;
			}
			mDecodingHeader = true;
			mFinishedStream = false;
		}

		~tOggStream()
		{
			delete mDecoder;
		    ogg_stream_clear(&mStream);
		}

		bool fDecodePacket()
		{
			ogg_packet packet;
			if (ogg_stream_packetout(&mStream, &packet) == 1)
			{
				if (mDecodingHeader)
					mDecodingHeader = mDecoder->fDecodeHeader(packet);

				if (!mDecodingHeader)
					mDecoder->fDecodeBody(packet);

				return true;
			}
			return false;
		}

		void fFinishStream( )
		{
			if( !mFinishedStream )
			{
				if (mDecoder)
					mDecoder->fFinishStream( );
				mFinishedStream = true;
			}
		}

		bool fIsFinishedStream( )
		{
			return mFinishedStream;
		}

		tStreamType mType;
		int mSerial;
		ogg_stream_state mStream;
		tOggDecoder *mDecoder;
		bool mDecodingHeader;
		bool mFinishedStream;
	};


	namespace
	{
		static tMoviePlayer* gInstance = NULL;
	}


	tMoviePlayer* tMoviePlayer::fInstance( )
	{
		return gInstance;
	}

	#define OGG_BUFFER_SIZE 4096
	void tMoviePlayer::fReadPage()
	{
		if (mPagesFinished)
			return;

		// read an entire page (as a series of buffers)
		ogg_page page;
		int size = 0;
		do
		{
			char* buffer = ogg_sync_buffer(&mOggSyncState, OGG_BUFFER_SIZE);

			DWORD readBytes = 0;
			::ReadFile(hFile, buffer, OGG_BUFFER_SIZE, &readBytes, 0);
			size += readBytes;
			if (readBytes > 0)
			{
				int ret = ogg_sync_wrote(&mOggSyncState, readBytes);
				sigassert(ret == 0);
			}
		}
		while ( size > 0 && ogg_sync_pageout(&mOggSyncState, &page) != 1);

		// if the page was not empty, push it onto the appropriate stream, or create a stream
		if (size > 0)
		{
			int serialNmbr = ogg_page_serialno(&page);
			if (ogg_page_bos(&page))
			{
				tOggStream *stream = new tOggStream(serialNmbr, page);
				mStreams.insert( std::pair<int,tOggStream*>(serialNmbr,stream) );
			}
			else
				ogg_stream_pagein(&(mStreams[serialNmbr]->mStream), &page);
		}
		else
			mPagesFinished = true;
	}

	bool tMoviePlayer::fAnyStreamFinishedHeader()
	{
		for (auto it = mStreams.begin(); it != mStreams.end(); ++it)
		{
			if (it->second->mType != tOggStream::cStreamTypeUnknown && !it->second->mDecodingHeader)
				return true;
		}
		return false;
	}

	tMoviePlayer::tMoviePlayer( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad )
	{
		gInstance = this;
		mPagesFinished = false;
		mTop = t;
		mLeft = l;
		mRight = r;
		mBottom = b;
		mLoop = loop;
		mPreload = preLoad;
		mComplete = false;
		mPlayTime = 0.0;

		// rip off the path and extension - we just want  video/<filename>.ogv
		char pathCopy[256];
		::strcpy_s( pathCopy, 256, path.fCStr() );
		char *end = strrchr(pathCopy,'.');
		char *start = strrchr(pathCopy,'\\');
		if (end == 0 || start == 0)
		{
			log_warning( 0, "Unable to isolate filename in: " << pathCopy );
			mComplete = true;
			return;
		}

		*end = 0;
		char trimmedPath[256];
		::strcpy_s(trimmedPath, 256, "video/");
		::strcat_s(trimmedPath, 256, start+1);
		::strcat_s(trimmedPath, 256, ".ogv");

		// load the entire movie - its only 16meg - no need to implement streaming yet
		hFile = ::CreateFile( trimmedPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (hFile == INVALID_HANDLE_VALUE)
		{
			log_warning( 0, "Unable to load video file:" << trimmedPath );
			mComplete = true;
			return;
		}

		BY_HANDLE_FILE_INFORMATION info;
		if( !GetFileInformationByHandle( hFile, &info ) )
		{
			log_warning( 0, "Unable to get info on video file:" << trimmedPath );
			mComplete = true;
			::CloseHandle( hFile );
			return;
		}

		// read in the video header..
		ogg_sync_init( &mOggSyncState );
		while( !mComplete && !fAnyStreamFinishedHeader() )
		{
			fReadPage();
			for( auto it = mStreams.begin( ); it != mStreams.end( ); ++it )
				it->second->fDecodePacket();
		}

		fRegisterWithDevice( Gfx::tDevice::fGetDefaultDevice( ).fGetRawPtr( ) );
	}

	void tMoviePlayer::fTick( )
	{
		if (mComplete)
			return;

		mPlayTime += tGameAppBase::fInstance( ).fGetFrameDeltaTime( );
		
		bool nothingDecoded = true;
		for( auto it = mStreams.begin( ); it != mStreams.end( ); ++it )
		{
			// there is a stream that is ahead of the playtime, so we can't be complete yet...
			if (mPlayTime <= it->second->mDecoder->mNextDecodeTime)
			{
				nothingDecoded = false;
				continue;
			}

			while (!it->second->fIsFinishedStream( ) && it->second->mType != tOggStream::cStreamTypeUnknown && mPlayTime > it->second->mDecoder->mNextDecodeTime)
			{
				if ( !it->second->fDecodePacket( ) )
				{
					fReadPage();
					if (mPagesFinished)
					{
						it->second->fFinishStream( );
						break;
					}
				}
				else
				{
					// managed to decode a packet... so we can't be complete yet
					nothingDecoded = false;
				}
			}
		}
		mComplete = nothingDecoded;
	}


	void tMoviePlayer::fRefreshVideoRect( u32 t, u32 l, u32 b, u32 r )
	{
	}

	tMoviePlayer::~tMoviePlayer( )
	{
		::CloseHandle( hFile );
		for (auto it = mStreams.begin(); it != mStreams.end(); ++it)
			delete it->second;
		ogg_sync_clear(&mOggSyncState);
		gInstance = 0;
	}


	void tMoviePlayer::fPause( b32 unpause )
	{
	}

	b32 tMoviePlayer::fPaused( ) const
	{
		return false;
	}

	void tMoviePlayer::fCancel( )
	{
	}

	void tMoviePlayer::fRenderFrame( Gfx::tDevice& device )
	{
	}

	void tMoviePlayer::fOnDeviceLost( Gfx::tDevice* device )
	{
		for (auto it = mStreams.begin(); it != mStreams.end(); ++it)
		{
			it->second->mDecoder->fOnDeviceLost( );
		}
	}

	void tMoviePlayer::fOnDeviceReset( Gfx::tDevice* device )
	{
		for (auto it = mStreams.begin(); it != mStreams.end(); ++it)
		{
			it->second->mDecoder->fOnDeviceReset( );
		}
	}

}


#elif !defined(target_game) // NULL player for editor

namespace Sig
{
	namespace
	{
		static tMoviePlayer* gInstance = NULL;
	}
	namespace MoviePlayer
	{
		void fMoviePlayerWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) { }
	}
	tMoviePlayer* tMoviePlayer::fInstance( ) { return gInstance; }
	tMoviePlayer::tMoviePlayer( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad ) { }
	void tMoviePlayer::fRefreshVideoRect( u32 t, u32 l, u32 b, u32 r ) { }
	tMoviePlayer::~tMoviePlayer( ) { }
	b32 tMoviePlayer::fFailed( HRESULT hr, const char *msg ) { return false; }
	void tMoviePlayer::fCreateMovie( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad ) { }
	b32	tMoviePlayer::fFinishedPlaying( ) const { return mComplete;	}
	void tMoviePlayer::fOnGraphEvent( ) {}
	void tMoviePlayer::fPause( b32 unpause ) {}
	b32 tMoviePlayer::fPaused( ) const	{ return false;	}
	void tMoviePlayer::fCancel( ) {	}
	void tMoviePlayer::fRenderFrame( Gfx::tDevice& device )	{ }
	void tMoviePlayer::fReleasePlayer( ) { }
}

#else

#include <vmr9.h>

#define XAUDIO2_HELPER_FUNCTIONS
#include <XAudio2.h>

#pragma comment(lib, "Strmiids.lib")

namespace Sig
{
	class tMoviePresenter : public IVMRSurfaceAllocator9, public IVMRImagePresenter9
	{
	public:
		tMoviePresenter() : mRefCount(0), mSurface(0), mNotify(0)
		{
		}
		virtual ~tMoviePresenter() 
		{
		}

		virtual ULONG STDMETHODCALLTYPE AddRef( void )
		{
			return InterlockedIncrement(&mRefCount);
		}

		ULONG STDMETHODCALLTYPE Release( void )
		{
			ULONG uCount = InterlockedDecrement(&mRefCount);
			if (uCount <= 0)
			{
				TerminateDevice(0);
				delete this;
			}
			return uCount;
		}

		HRESULT STDMETHODCALLTYPE tMoviePresenter::QueryInterface(REFIID riid, void** ppvObject)
		{
			if(riid == IID_IVMRSurfaceAllocator9)
			{
				AddRef();
				*ppvObject = static_cast<IVMRSurfaceAllocator9*>(this);
			}
			else if(riid == IID_IVMRImagePresenter9)
			{
				AddRef();
				*ppvObject = static_cast<IVMRImagePresenter9*>(this);
			}
			else if(riid == IID_IUnknown)
			{
				AddRef();
				*ppvObject = static_cast<IUnknown*>(static_cast<IVMRSurfaceAllocator9*>(this));
			}
			else
				return E_NOINTERFACE;

			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE InitializeDevice( 
			/* [in] */ DWORD_PTR dwUserID,
			/* [in] */ VMR9AllocationInfo *lpAllocInfo,
			/* [out][in] */ DWORD *lpNumBuffers)
		{
			HRESULT hr = mNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &mSurface);
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE TerminateDevice( 
			/* [in] */ DWORD_PTR dwID)
		{
			if (mSurface)
			{
				mSurface->Release();
				mSurface = 0;
			}
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE GetSurface( 
			/* [in] */ DWORD_PTR dwUserID,
			/* [in] */ DWORD SurfaceIndex,
			/* [in] */ DWORD SurfaceFlags,
			/* [out] */ IDirect3DSurface9 **lplpSurface)
		{
			if (!lplpSurface)
				return E_POINTER;
			if (SurfaceIndex != 0)
				return E_INVALIDARG;

			mSurface->AddRef();
			lplpSurface[SurfaceIndex] = mSurface;
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE AdviseNotify( 
			/* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify)
		{
			mNotify = lpIVMRSurfAllocNotify;

			// Get monitor for primary display adapter
			IDirect3DDevice9 *d3dDevice = Gfx::tDevice::fGetDefaultDevice( )->fGetDevice( );
			HMONITOR hMonitor = Gfx::tDevice::fGetDefaultDevice( )->fGetObject( )->GetAdapterMonitor(D3DADAPTER_DEFAULT);
			HRESULT hr = mNotify->SetD3DDevice(d3dDevice, hMonitor);
			if( FAILED( hr ) )
				log_line( 0, "FAILED TO SET D3D Device" );
			return hr;
		}

		virtual HRESULT STDMETHODCALLTYPE StartPresenting( 
			/* [in] */ DWORD_PTR dwUserID)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE StopPresenting( 
			/* [in] */ DWORD_PTR dwUserID)
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE PresentImage( 
			/* [in] */ DWORD_PTR dwUserID,
			/* [in] */ VMR9PresentationInfo *lpPresInfo)
		{
			IDirect3DSurface9 *source = lpPresInfo->lpSurf;
			IDirect3DSurface9 *dest;

			Gfx::tDevicePtr device = Gfx::tDevice::fGetDefaultDevice( );
			device->fGetDevice( )->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &dest );
			s32 backbufferWidth = device->fCreationPresentParams( ).BackBufferWidth;
			s32 backbufferHeight = device->fCreationPresentParams( ).BackBufferHeight;
			RECT rect;
			if (backbufferWidth > backbufferHeight)
			{
				rect.left = 0;
				rect.right = backbufferWidth;
				s32 height = backbufferWidth*720/1280;
				s32 border = backbufferHeight - height;
				rect.top = border/2;
				rect.bottom = backbufferHeight-border/2;
			}
			else
			{
				rect.top = 0;
				rect.bottom = backbufferHeight;
				s32 width = backbufferHeight*1280/720;
				s32 border = backbufferWidth - width;
				rect.left = border/2;
				rect.right = backbufferWidth-border/2;
			}

			HRESULT hr = Gfx::tDevice::fGetDefaultDevice( )->fGetDevice( )->StretchRect( source, NULL, dest, &rect, D3DTEXF_NONE ); 
			dest->Release();

			Gfx::tDevice::fGetDefaultDevice( )->fGetDevice( )->Present( 0, 0, 0, 0 );
			return S_OK;
		}

	protected:
	public:
		ULONG mRefCount;
		IVMRSurfaceAllocatorNotify9 *mNotify;
		IDirect3DSurface9 *mSurface;
	};


	namespace
	{
		static tMoviePlayer* gInstance = NULL;
		static const UINT WM_GRAPHEVENT = WM_USER + 200;
	}

	namespace MoviePlayer
	{
		void fMoviePlayerWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
		{
			if( !gInstance )
				return;

			switch( msg )
			{
			case WM_PAINT:

				break;

			case WM_DISPLAYCHANGE:
				break;

			case WM_SIZE:
				//gInstance->fRefreshVideoRect( );
				break;

			case WM_GRAPHEVENT:
				gInstance->fOnGraphEvent( );
				break;
			}
		}
	}

	tMoviePlayer* tMoviePlayer::fInstance( )
	{
		return gInstance;
	}

	tMoviePlayer::tMoviePlayer( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad )
	{
		gInstance = this;
		mGraphBuilder = NULL;
		mMediaControl = NULL;
		mMediaEvent = NULL;
		mMoviePresenter = NULL;
		mVMR = NULL;
		mNotify = NULL;
		mComplete = false;
		fCreateMovie( device, system, path, t, l, b, r, loop, preLoad );
		log_line( 0, "START MOVIE : " << path );
	}

	void tMoviePlayer::fRefreshVideoRect( u32 t, u32 l, u32 b, u32 r )
	{
	}

	tMoviePlayer::~tMoviePlayer( )
	{
		log_line( 0, "END MOVIE" );
		fReleasePlayer( );
		gInstance = NULL;
	}

	b32 tMoviePlayer::fFailed( HRESULT hr, const char *msg )
	{
		if( FAILED( hr ) )
		{
			log_warning( 0, msg << " HR: " << hr );
			fReleasePlayer( );
			return true;
		}
		return false;
	}

#if defined(platform_pcdx9)
	static b32 IsVistaOrGreator()
	{
		OSVERSIONINFOEX osvi;
		DWORDLONG dwlConditionMask = 0;
		int op=VER_GREATER_EQUAL;

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		osvi.dwMajorVersion = 6;
		osvi.dwMinorVersion = 0;

		// Initialize the condition mask.
		VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
		VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );

		// Perform the test.
		return VerifyVersionInfo(
			&osvi, 
			VER_MAJORVERSION | VER_MINORVERSION,
			dwlConditionMask);
	}
#endif

	void tMoviePlayer::fCreateMovie( Gfx::tDevice& device, Audio::tSystem& system, const tFilePathPtr& path, u32 t, u32 l, u32 b, u32 r, b32 loop, b32 preLoad )
	{
		// skip video playback on XP because VRM9 player and the Steam Overlay DLL don't play together and the next D3D call will crash after creating the VRM9 player
#if defined(platform_pcdx9)
		if (!IsVistaOrGreator())
		{
			mComplete = true;
			return;
		}
#endif

		tFilePathPtr absPath = tFilePathPtr::fConstructPath( tApplication::fInstance( ).fGameRoot( ), path );
		if( !FileSystem::fFileExists( absPath ) )
		{
			log_warning( 0, "Video file doesnt exist: " << absPath );
			return;
		}

		// Create filter graph
		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, ( void** )&mGraphBuilder );
		if( fFailed( hr, "Could not create Graph Builder" ) )
			return;

#if 0 // register the video graph for debugging in graphedit
		{
			IMoniker* pMoniker;
			IRunningObjectTable* pROT;
			GetRunningObjectTable( 0, &pROT );

			WCHAR wsz[256]; 	
			swprintf_s( wsz, L"FilterGraph %08p pid %08x", (DWORD_PTR)mGraphBuilder, GetCurrentProcessId() );
			CreateItemMoniker( L"!", wsz, &pMoniker );

			DWORD regi;
			pROT->Register( 0, mGraphBuilder, pMoniker, &regi );

			// Clean up any COM stuff here ...
		}
#endif

		// Create the VMR-9 filter
		hr = CoCreateInstance( CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&mVMR );
		if( fFailed( hr, "Failed to get video mixing renderer interface" ) )
			return;

		// Hook up this filter
		hr = mGraphBuilder->AddFilter( mVMR, L"VMR9" );
		if( fFailed( hr, "adding VMR9 filter failed" ) )
			return;

		// Get filter config interface
		IVMRFilterConfig9* config = NULL;
		hr = mVMR->QueryInterface( IID_IVMRFilterConfig9, (void**)&config );
		if( fFailed( hr, "Failed to get filter config interface" ) )
			return;

		hr = config->SetRenderingMode( VMR9Mode_Renderless );
		if( fFailed( hr, "Failed to set to renderless" ) )
			return;

		hr = config->SetNumberOfStreams( 5 );
		if( fFailed( hr, "Failed to set number of streams" ) )
			return;
		config->Release();

		// create hook up the VMR surface allocator
		mMoviePresenter = new tMoviePresenter;
		mMoviePresenter->AddRef();

		hr = mVMR->QueryInterface(IID_IVMRSurfaceAllocatorNotify9, (void**)&mNotify);
		if( fFailed( hr, "Failed to get Surface Allocator Notify interface" ) )
			return;

		// Let the allocator and the notify know about each other
		hr = mNotify->AdviseSurfaceAllocator(0, mMoviePresenter);
		if( fFailed( hr, "Advise Surface Allocator failed" ) )
			return;

		hr = mMoviePresenter->AdviseNotify(mNotify);
		if( fFailed( hr, "Advise Notify failed!" ) )
			return;

		// Auto-create the entire graph
		// this should see the VMR9 and hook it up
		IBaseFilter* pSource = NULL;
		std::wstringstream ss;
		ss.str( StringUtil::fStringToWString( std::string( path.fCStr( ) ) ) );
		hr = mGraphBuilder->RenderFile( ss.str( ).c_str( ), NULL );
		if( fFailed( hr, "Could not add the source filter to the graph" ) )
			return;

		// media control lets us start and stop playback
		hr = mGraphBuilder->QueryInterface( IID_IMediaControl, (void**)&mMediaControl );
		if( fFailed( hr, "Failed to get media control interface" ) )
			return;

		// Media event lets us know when the playback is finished
		hr = mGraphBuilder->QueryInterface( IID_IMediaEventEx, ( void** )&mMediaEvent );
		if( fFailed( hr, "Filed to query media event interface" ) )
			return;
		HWND hWnd = tApplication::fInstance( ).fGetHwnd( );
		mMediaEvent->SetNotifyWindow( ( OAHWND )hWnd, WM_GRAPHEVENT, NULL );
		mMediaEvent->SetNotifyFlags( 0 );

		// Set volume level
		IBasicAudio* audio;
		hr = mGraphBuilder->QueryInterface( IID_IBasicAudio, ( void** )&audio );
		if( FAILED( hr ) )
		{
			log_warning( 0, "GraphBuilder couldn't query IBasicAudio HR: " << hr );
			fReleasePlayer( );
			return;
		}
		// Audio system returns an amplitude between 0.0 and 1.0.
		// Video playback requires a millidecibel value ranging from -10,000 to 0
		float amplitude = system.fUserMusicVolume();
		float decibels = XAudio2AmplitudeRatioToDecibels( amplitude );
		if( decibels < -100 )
			decibels = -100;
		audio->put_Volume( ( long )( decibels * 100 ) );
		audio->Release( );
		audio = NULL;

		// run the movie
		hr = mMediaControl->Run( );
		if( fFailed( hr, "Movie player couldn't be run" ) )
			return;
	}


	b32	tMoviePlayer::fFinishedPlaying( ) const
	{
		return mComplete;
	}

	void tMoviePlayer::fOnGraphEvent( )
	{
		LONG eventCode, param1, param2;
		while( mMediaEvent->GetEvent( &eventCode, &param1, &param2, 0 ) != E_ABORT )
		{
			switch (eventCode )
			{
			case EC_COMPLETE:
				mComplete = true;
				break;
			}
		}
		mMediaEvent->FreeEventParams( eventCode, param1, param2 );
	}

	void tMoviePlayer::fPause( b32 unpause )
	{
		if( mMediaControl )
		{
			if( unpause )
			{
				HRESULT hr = mMediaControl->Run( );
				if( FAILED( hr ) )
				{
					log_warning( 0, "Movie player couldn't be resumed HR: " << hr );
					fReleasePlayer( );
					return;
				}
			}
			else
			{
				HRESULT hr = mMediaControl->Pause( );
				if( FAILED( hr ) )
				{
					log_warning( 0, "Movie player couldn't be paused HR: " << hr );
					fReleasePlayer( );
					return;
				}
			}
		}
	}

	b32 tMoviePlayer::fPaused( ) const
	{
		if( !mMediaControl )
			return false;

		OAFilterState state = State_Running;
		const HRESULT hr = mMediaControl->GetState( 10, &state );
		if( FAILED( hr ) )
		{
			log_warning( 0, "Movie player state couldn't be queried to check if paused, HR: " << hr );
			return false; // Possibly temporary?
		}

		return state != State_Running;
	}

	void tMoviePlayer::fCancel( )
	{
		if( mMediaControl )
		{
			HRESULT hr = mMediaControl->Stop( );
			if( FAILED( hr ) )
			{
				log_warning( 0, "Movie player couldn't be stopped HR: " << hr );
			}
		}
		fReleasePlayer( );
	}

	void tMoviePlayer::fRenderFrame( Gfx::tDevice& device )
	{
	}

#define RELEASE_INTERFACE( i )	if( i ) { i->Release( ); i = NULL; }

	void tMoviePlayer::fReleasePlayer( )
	{
		mComplete = true;
		RELEASE_INTERFACE( mMediaEvent );
		RELEASE_INTERFACE( mMediaControl );
		RELEASE_INTERFACE( mGraphBuilder );
		RELEASE_INTERFACE( mNotify );
		RELEASE_INTERFACE( mVMR );
		RELEASE_INTERFACE( mMoviePresenter );
	}
}

#endif

#endif

