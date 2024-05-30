#include "BasePch.hpp"
#include "tAnimatedQuad.hpp"

namespace Sig { namespace Gui
{

	tAnimatedQuad::tAnimatedQuad( )
		: mFramerate( 0 )
		, mCurrentFrame( 0 )
		, mFrameCount( 0 )
	{
	}

	tAnimatedQuad::tAnimatedQuad( Gfx::tDefaultAllocators& allocators )
		: tTexturedQuad( allocators )
		, mFramerate( 0 )
		, mCurrentFrame( 0 )
		, mFrameCount( 0 )
	{
	}

	/*tAnimatedQuad tAnimatedQuad::fConstruct( const tFilePathPtr& texturePath, u32 framerate, u32 frameCount, const Math::tVec2f frameSize )
	{
		tAnimatedQuad o;
		o.fSetTexture( texturePath );
		o.fSetFramerate( framerate );
		o.fSetFrameCount( frameCount );
		o.fSetFrameSize( frameSize );
		return o;
	}*/

	void tAnimatedQuad::fOnTickCanvas( f32 dt )
	{
		tTexturedQuad::fOnTickCanvas( dt );

		if( mFramerate == 0 || mFrameSize.x == 0 || mFrameSize.y == 0 || mFrameCount == 0 )
			return;

		static f32 timer = 0;
		const f32 timeBetweenFrames = 1.0f / mFramerate;

		timer += dt;

		if( timer > timeBetweenFrames )
		{
			while( timer > timeBetweenFrames )
			{
				// Dec Timer
				timer -= timeBetweenFrames;

				// Time for the next frame!
				mCurrentFrame = ( (mCurrentFrame + 1) % mFrameCount );
			}

			fInternalSetTextureRect( );
		}
	}

	void tAnimatedQuad::fInternalSetTextureRect( )
	{
		const u32 framesPerRow = (u32)(fTextureDimensions( ).x / mFrameSize.x);
		const f32 texX = mFrameSize.x * ( mCurrentFrame % framesPerRow );
		const f32 texY = mFrameSize.y * ( mCurrentFrame / framesPerRow );

		Math::tVec2f texPos;
		texPos.x = Math::fRemapZeroToOne( 0.0f, fTextureDimensions( ).x, texX );
		texPos.y = Math::fRemapZeroToOne( 0.0f, fTextureDimensions( ).y, texY );

		Math::tVec2f texSize;
		texSize.x = Math::fRemapZeroToOne( 0.0f, fTextureDimensions( ).x, mFrameSize.x );
		texSize.y = Math::fRemapZeroToOne( 0.0f, fTextureDimensions( ).y, mFrameSize.y );

		fSetTextureRect( texPos, texSize );
	}

}}

namespace Sig { namespace Gui
{
	void tAnimatedQuad::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass< tAnimatedQuad, tTexturedQuad, Sqrat::NoCopy<tAnimatedQuad> > classDesc( vm.fSq( ) );

		classDesc
			.Prop( _SC( "Framerate" ), &tAnimatedQuad::fFramerate, &tAnimatedQuad::fSetFramerate )
			.Prop( _SC( "FrameCount" ), &tAnimatedQuad::fFrameCount, &tAnimatedQuad::fSetFrameCount )
			.Prop( _SC( "FrameSize" ), &tAnimatedQuad::fFrameSize, &tAnimatedQuad::fSetFrameSize )
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("AnimatedQuad"), classDesc);
	}
}}
