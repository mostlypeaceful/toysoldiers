#include "BasePch.hpp"
#if defined( platform_ios )
#include "tRenderTarget.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	
	GLenum tRenderTarget::fConvertInternalFormat( tFormat format )
	{
		switch ( format )
		{
#if defined( platform_ios ) // OpenGL ES
			case cFormatNull  : return 0;
			case cFormatRGBA8 : return GL_RGBA;
			case cFormatR32F  : return GL_ALPHA;
			case cFormatD24S8 : return GL_DEPTH24_STENCIL8_OES;
#else // OpenGL
			case cFormatNull  : return 0;
			case cFormatRGBA8 : return 4;
			case cFormatR32F  : return 1;
			case cFormatD24S8 : return GL_DEPTH24_STENCIL8_OES;
#endif
			default : sigassert( !"unrecognized format passed to tRenderTarget" ); break;
		}
		return 0;
	}
	
	void tRenderTarget::fConvertFormatType( tFormat format, GLint &glInternalFormat, GLenum &glFormat, GLenum &glType )
	{
		switch ( format )
		{
			case cFormatNull  : glFormat = 0; glType = 0; break;
			case cFormatRGBA8 : glFormat = GL_RGBA; glType = GL_UNSIGNED_BYTE; break;
			case cFormatR32F  : glFormat = GL_ALPHA; glType = GL_FLOAT; break; /* GL_RED not recognized, using GL_ALPHA for now */
			case cFormatD24S8 : glFormat = GL_DEPTH_STENCIL_OES; glType = GL_UNSIGNED_INT_24_8_OES; break;
			default : sigassert( !"unrecognized format passed to tRenderTarget" ); break;
		}
		
		glInternalFormat = fConvertInternalFormat(format);
		
#if defined( platform_ios )
		log_assert( glFormat == glInternalFormat, "OpenGL ES requires format and internalFormat match" );
#endif
	}
	
	tRenderTarget::tRenderTarget( const tDevicePtr& device, u32 width, u32 height, tFormat format, u32 multiSamplePower )
		: tRenderTargetPlatformBase( width, height, format, multiSamplePower )
		, mSurface( 0 )
		, mTexture( false )
		, mOwnsSurface( true )
	{
		tFormat formatToConvert;
		if ( format == cFormatNull )
			formatToConvert = cFormatRGBA8;
		else 
			formatToConvert = format;
		
		GLenum internalFormat = fConvertInternalFormat( formatToConvert );
		ignore_unused(internalFormat);
		
		GLuint renderbuffer = 0;
		glGenRenderbuffers( 1, &renderbuffer );
		glBindRenderbuffer( GL_RENDERBUFFER, renderbuffer );
		glRenderbufferStorage( GL_RENDERBUFFER, internalFormat, width, height);
		if( glGetError() != GL_NO_ERROR )
		{
			glDeleteRenderbuffers( 1, &renderbuffer );
			renderbuffer = 0;
			log_assert( 0, "Failed to create GL_RENDERBUFFER of size " << width << "x" << height << " with format " << internalFormat );
		}
		
		mSurface = renderbuffer;
		mTexture = false;
		mOwnsSurface = true;
	}
	
	tRenderTarget::tRenderTarget( GLenum handleType, GLuint handle, u32 width, u32 height, tFormat format, u32 multiSamplePower )
		: tRenderTargetPlatformBase( width, height, format, multiSamplePower )
		, mSurface( handle )
		, mTexture( handleType==GL_TEXTURE || handleType==GL_TEXTURE_2D )
		, mOwnsSurface( false )
	{
	}

	tRenderTarget::~tRenderTarget( )
	{
		if( mSurface && mOwnsSurface )
		{
			if( mTexture )
				glDeleteTextures( 1, &mSurface );
			else
				glDeleteRenderbuffers( 1, &mSurface );
		}
	}

	b32 tRenderTargetPlatformBase::fFailed( ) const
	{
		const tRenderTarget* self = static_cast< const tRenderTarget* >( this );
		return self->mSurface == 0;
	}
	
	void fDumpRenderBufferOrTexture2D( const char* prefix, GLint type, GLint handle )
	{
		GLint width=0, height=0;
		switch( type )
		{
		case GL_RENDERBUFFER:
			glGetRenderbufferParameteriv( type, GL_RENDERBUFFER_WIDTH , &width );
			glGetRenderbufferParameteriv( type, GL_RENDERBUFFER_HEIGHT, &height );
			log_line( 0, prefix << "GL_RENDERBUFFER ("<<handle<<") of size " << width << "x" << height );
			break;
		case GL_TEXTURE:
			log_line( 0, prefix << "GL_TEXTURE ("<<handle<<") of size ???x???" );
			break;
		case GL_TEXTURE_2D:
			log_line( 0, prefix << "GL_TEXTURE_2D ("<<handle<<") of size ???x???" );
			break;
		default:
			log_line( 0, prefix << "UNKNOWN ("<<handle<<") of type " << type );
			return;
		}
	}
	
	void fDumpFramebufferState()
	{
		GLint colorType, depthType;
		GLint color, depth;
		glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &colorType );
		glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT , GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthType );
		glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &color );
		glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT , GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depth );
		log_line( 0, "GL_FRAMEBUFFER with the following attachments:" );
		fDumpRenderBufferOrTexture2D( "  GL_COLOR_ATTACHMENT0: ", colorType, color );
		fDumpRenderBufferOrTexture2D( "  GL_DEPTH_ATTACHMENT:  ", depthType, depth );
	}

	void tRenderTarget::fApply( const tDevicePtr& device, u32 rtIndex ) const
	{
		log_assert( rtIndex==0, "Only one render target instance supported on iOS" );
		const GLenum attachment = fIsDepthTarget() ? GL_DEPTH_ATTACHMENT : (GL_COLOR_ATTACHMENT0+rtIndex);
		
		glBindFramebuffer( GL_FRAMEBUFFER, device->fFrameBuffer() );
		log_assert( glGetError()==GL_NO_ERROR, "Error binding frame buffer" );
		if( mTexture )
		{
			glFramebufferTexture2D( GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, mSurface, 0 );
			log_assert( glGetError()==GL_NO_ERROR, "Error binding GL_TEXTURE_2D to frame buffer" );
		}
		else
		{
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, mSurface );
			log_assert( glGetError()==GL_NO_ERROR, "Error binding GL_RENDERBUFFER to frame buffer" );
		}
		
		const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if( status != GL_FRAMEBUFFER_COMPLETE )
		{
			fDumpFramebufferState( );
			log_assert( 0, "GL_FRAMEBUFFER not complete (glCheckFramebufferStatus returned " << status << ")" );
		}
	}

}}
#endif//#if defined( platform_ios )

