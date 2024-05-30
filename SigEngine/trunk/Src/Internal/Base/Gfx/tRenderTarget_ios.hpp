#if defined( platform_ios )
#ifndef __tRenderTarget_ios__
#define __tRenderTarget_ios__

namespace Sig { namespace Gfx
{
	class tDevicePtr;

	class base_export tRenderTarget : public tRenderTargetPlatformBase
	{
		friend class tRenderTargetPlatformBase;
		define_class_pool_new_delete( tRenderTarget, 32 );
	private :
		GLuint	mSurface;		///< a GL_RENDERBUFFER or GL_TEXTURE_?D
		b32		mTexture;		///< indicates what mSurface is (true=GL_TEXTURE_?D, false=GL_RENDERBUFFER)
		b32		mOwnsSurface;	///< Destroy mSurface on destruction?
	public :
		static void fConvertFormatType( tFormat format, GLint &glInternalFormat, GLenum &glFormat, GLenum &glType );
		static GLenum fConvertInternalFormat( tFormat format );
	public:
		tRenderTarget( const tDevicePtr& device, u32 width, u32 height, tFormat format, u32 multiSamplePower = 0 );			///< Create a new GL_RENDERBUFFER of "width" x "height" with "format" and own it.
		tRenderTarget( GLenum handleType, GLuint handle, u32 width, u32 height, tFormat format, u32 multiSamplePower = 0 );	///< Wrap an existing OpenGL handle of "width" x "height" with "format" -- don't own it.
		~tRenderTarget( );
		void fApply( const tDevicePtr& device, u32 rtIndex = 0 ) const;
	};

}}



#endif//__tRenderTarget_ios__
#endif//#if defined( platform_ios )

