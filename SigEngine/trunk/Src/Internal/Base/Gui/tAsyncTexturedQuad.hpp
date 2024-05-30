#ifndef __tAsyncTexturedQuad__
#define __tAsyncTexturedQuad__

#include "tTexturedQuad.hpp"
#include "tResource.hpp"

namespace Sig { namespace Gui
{
	class tAsyncTexturedQuad : public tCanvasFrame
	{
		define_dynamic_cast( tAsyncTexturedQuad, tCanvasFrame );

	public:
		tAsyncTexturedQuad( );
		~tAsyncTexturedQuad( );

		void fSetTexture( const tFilePathPtr& texturePath );
		void fSetTexture( const tResourcePtr& textureResource );
		void fBlockForLoad( );
		b32 fIsLoaded( ) const;
		Sqrat::Function fOnLoadedCallback( ) const { return mCallback; }
		void fSetOnLoadedCallback( const Sqrat::Function& func );
		void fOnLoad( tResource& resource, b32 success );
		void fUnload( );
		Math::tVec2f fTextureDimensions( ) const;
	private:
		void fAddTexturedQuad( );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		Gfx::tDefaultAllocators&	mAllocators;
		tResourcePtr mTextureResource;
		tResourcePtr mNewTextureResource;
		Sqrat::Function mCallback;
		tResource::tOnLoadComplete::tObserver mOnLoadObserver;
	};
} }

#endif //__tAsyncTexturedQuad__