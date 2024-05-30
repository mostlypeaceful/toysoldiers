#ifndef __tAnimatedQuad__
#define __tAnimatedQuad__

#include "Gui/tTexturedQuad.hpp"

namespace Sig { namespace Gui
{
	class tAnimatedQuad : public tTexturedQuad
	{
		define_dynamic_cast( tAnimatedQuad, tTexturedQuad );

	public:
		//static tAnimatedQuad fConstruct( const tFilePathPtr& texturePath, u32 framerate, u32 frameCount, const Math::tVec2f frameSize );

	public:
		tAnimatedQuad( );
		explicit tAnimatedQuad( Gfx::tDefaultAllocators& allocators );
		~tAnimatedQuad( ) { }

		void fSetFramerate( u32 framesPerSecond ) { mFramerate = framesPerSecond; }
		void fSetFrameCount( u32 count ) { mFrameCount = count; }
		void fSetFrameSize( const Math::tVec2f& frameSize ) { mFrameSize = frameSize; }

		u32 fFramerate( ) const { return mFramerate; }
		u32 fFrameCount( ) const { return mFrameCount; }
		const Math::tVec2f& fFrameSize( ) const { return mFrameSize; }

		virtual void fOnTickCanvas( f32 dt );

	protected:
		void fInternalSetTextureRect( );

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		u32 mFramerate;
		u16 mCurrentFrame;
		u16 mFrameCount;
		Math::tVec2f mFrameSize;
		Math::tRect mCurrentTextureRect;
	};
}}

#endif //__tAnimatedQuad__