#include "BasePch.hpp"
#if defined( platform_metro )
#include "BannedApiConfig.hpp"
#include "tSystemImage.hpp"
#include <wrl\client.h>
#include <wincodec.h>

using namespace Microsoft::WRL;

namespace Sig
{
	namespace
	{
		IWICImagingFactory* fCreateWICImagingFactory()
		{
			IWICImagingFactory* factory = 0;

			MULTI_QI mqi = {};
			mqi.hr = S_OK;
			mqi.pIID = &__uuidof(IWICImagingFactory);
			mqi.pItf = 0;

			HRESULT hr = CoCreateInstanceFromApp(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				nullptr,
				1,
				&mqi );

			factory = static_cast<IWICImagingFactory*>( mqi.pItf );
			sigassert(SUCCEEDED(hr) && SUCCEEDED(mqi.hr) && factory);

			return factory;
		}

		IWICBitmapSource* fCreateSystemImageInternal( IWICImagingFactory* factory, IWICStream* stream )
		{
			if( !factory || !stream )
			{
				log_assert( 0, "Failed to create a WIC factory or stream" );
				return NULL;
			}



			ComPtr<IWICBitmapDecoder> decoder = NULL;
			HRESULT hr = factory->CreateDecoderFromStream(
				stream,
				NULL,
				WICDecodeMetadataCacheOnDemand,
				&decoder
				);
			if( !SUCCEEDED(hr) || !decoder )
			{
				log_assert( 0, "Failed to create a WIC decoder" );
				return NULL;
			}
			



			UINT uiFrameCount = 0;
			hr = decoder->GetFrameCount(&uiFrameCount);
			if( !SUCCEEDED(hr) || uiFrameCount<=0 )
			{
				log_assert( 0, "Couldn't get or invalid frame count" );
				return NULL;
			}



			IWICBitmapFrameDecode* decode = NULL;
			hr = decoder->GetFrame(0, &decode);
			if( !SUCCEEDED(hr) || !decode )
			{
				log_assert( 0, "Couldn't get the IWICBitmapSource" );
				return NULL;
			}
			
			IWICBitmapSource *source = decode;
			// source->AddRef();
			return source;
		}
	}

	tRefCounterPtr<tSystemImage> tSystemImage::fFromMemory(  byte* begin, u32 size )
	{
		IWICImagingFactory* factory = fCreateWICImagingFactory();
		IWICStream* stream = 0;
		HRESULT hr = factory->CreateStream(&stream);
		sigassert(SUCCEEDED(hr) && stream);
		hr = stream->InitializeFromMemory(begin,size);
		sigassert(SUCCEEDED(hr));

		tSystemImagePtr sip( new tSystemImage() );
		sip->mRawPlatformHandle = fCreateSystemImageInternal(factory,stream);
		return sip;
	}

	tRefCounterPtr<tSystemImage> tSystemImage::fFromFile( const tFilePathPtr& file )
	{
		IWICImagingFactory* factory = fCreateWICImagingFactory();
		IWICStream* stream = 0;
		HRESULT hr = factory->CreateStream(&stream);
		sigassert(SUCCEEDED(hr) && stream);
		hr = stream->InitializeFromFilename( StringUtil::fMultiByteToWString(file.fCStr()).c_str(), GENERIC_READ );
		sigassert(SUCCEEDED(hr));

		tSystemImagePtr sip( new tSystemImage() );
		sip->mRawPlatformHandle = fCreateSystemImageInternal(factory,stream);
		return sip;
	}

	u32 tSystemImage::fWidth() const
	{
		sigassert( mRawPlatformHandle );
		if( !mRawPlatformHandle )
			return 0;
		UINT w, h;
		IWICBitmapSource* src = (IWICBitmapSource*)mRawPlatformHandle;
		src->GetSize(&w,&h);
		return w;
	}

	u32 tSystemImage::fHeight() const
	{
		sigassert( mRawPlatformHandle );
		if( !mRawPlatformHandle )
			return 0;
		UINT w, h;
		IWICBitmapSource* src = (IWICBitmapSource*)mRawPlatformHandle;
		src->GetSize(&w,&h);
		return h;
	}

	void tSystemImage::fCopyTo( byte* buffer, tFormat format, u32 w, u32 h ) const
	{
		u32 pixelStride = 0;

		switch( format )
		{
		case Gfx::tTextureFile::cFormatA8B8G8R8:
		case Gfx::tTextureFile::cFormatA8R8G8B8:
			pixelStride = 4;
			break;
		case Gfx::tTextureFile::cFormatR5G6B5:
			pixelStride = 2;
			break;
		case Gfx::tTextureFile::cFormatA8:
			pixelStride = 1;
			break;
		default:
			log_assert( 0, "Invalid format: " << format );
			break;
		}

		fCopyTo( buffer, format, w, h, pixelStride*w );
	}

	void tSystemImage::fCopyTo( byte* buffer, tFormat format, u32 w, u32 h, u32 stride ) const
	{
		sigassert( buffer );

		IWICBitmapSource* source = (IWICBitmapSource*)mRawPlatformHandle;
		sigassert( source );
		if( !source )
			return;

		WICPixelFormatGUID pixelFormat;
		HRESULT hr = source->GetPixelFormat(&pixelFormat);
		sigassert(SUCCEEDED(hr));

		ComPtr<IWICBitmapSource> converted = source;

		if( !IsEqualGUID(pixelFormat, GUID_WICPixelFormat32bppBGRA) )
		{
			hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, source, &converted);
			sigassert( SUCCEEDED(hr) );
		}

		WICRect rc;
		rc.X = 0;
		rc.Y = 0;
		rc.Width = w;
		rc.Height = h;
		hr = converted->CopyPixels( &rc, stride, h*stride, buffer );
		sigassert(SUCCEEDED(hr));
	}
}
#endif //defined( platform_metro )
