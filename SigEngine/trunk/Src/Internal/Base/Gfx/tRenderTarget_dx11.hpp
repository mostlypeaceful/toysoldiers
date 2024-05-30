#if defined( platform_pcdx11 ) || defined( platform_metro )
#ifndef __tRenderTarget_dx11__
#define __tRenderTarget_dx11__

namespace Sig { namespace Gfx
{
	class tDevicePtr;

	class base_export tRenderTarget : public tRenderTargetPlatformBase
	{
		friend class tRenderTargetPlatformBase;
		define_class_pool_new_delete( tRenderTarget, 32 );
	public:
		static DXGI_FORMAT fConvertFormatType( tFormat format );

		tRenderTarget( const tDevicePtr& device, u32 width, u32 height, tFormat format, u32 multiSamplePower = 0 );
		tRenderTarget( const tDevicePtr& device, ID3D11Texture2D* rt, u32 width, u32 height, tFormat format, u32 multiSamplePower = 0 );
		~tRenderTarget( );
		void fApply( const tDevicePtr& device, u32 rtIndex = 0 ) const;
		ID3D11Texture2D* fGetTexture( ) const { return mTexture; }
		ID3D11DepthStencilView* fGetDepthStencilView( ) const { sigassert( fIsDepthTarget()); return _mDsView; }
		ID3D11RenderTargetView* fGetRenderTargetView( ) const { sigassert(!fIsDepthTarget()); return _mRtView; }
	private:
		void fCreateView( const tDevicePtr& device );

		ID3D11Texture2D* mTexture;
		union
		{
			ID3D11DepthStencilView* _mDsView; // if  fIsDepthTarget()
			ID3D11RenderTargetView* _mRtView; // if !fIsDepthTarget()
		};
	};

}}



#endif//__tRenderTarget_dx11__
#endif//#if defined( platform_pcdx11 ) || defined( platform_metro )

