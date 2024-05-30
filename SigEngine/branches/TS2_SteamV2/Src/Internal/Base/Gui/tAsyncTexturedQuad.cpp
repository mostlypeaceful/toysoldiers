#include "BasePch.hpp"
#include "tAsyncTexturedQuad.hpp"
#include "Gfx/tDefaultAllocators.hpp"

namespace Sig { namespace Gui
{

	tAsyncTexturedQuad::tAsyncTexturedQuad( )
		: mAllocators( Gfx::tDefaultAllocators::fInstance( ) )
	{
		mOnLoadObserver.fFromMethod<tAsyncTexturedQuad, &tAsyncTexturedQuad::fOnLoad>( this );
	}

	tAsyncTexturedQuad::~tAsyncTexturedQuad()
	{
		if( mTextureResource )
			mTextureResource->fUnload( this );
		if( mNewTextureResource )
			mNewTextureResource->fUnload( this );
	}

	void tAsyncTexturedQuad::fSetTexture( const tFilePathPtr& texturePath )
	{
		tResourcePtr colorMap = mAllocators.mResourceDepot->fQuery( tResourceId::fMake<Gfx::tTextureFile>( texturePath ) );
		fSetTexture( colorMap );
	}

	void tAsyncTexturedQuad::fSetTexture( const tResourcePtr& textureResource )
	{
		if( (mTextureResource == textureResource && !mNewTextureResource) 
			|| (mNewTextureResource && textureResource == mNewTextureResource) )
		{
			log_line( 0, "Already have or loading texture in fSetTexture" );
			if( !mCallback.IsNull( ) )
				mCallback.Execute( this );
			return;
		}

		// Release old new texture
		if( mNewTextureResource )
		{
			mNewTextureResource->fRemoveLoadCallback( mOnLoadObserver );
			mNewTextureResource->fUnload( this );
		}

		mNewTextureResource = textureResource;

		// Load the next texture
		if( mNewTextureResource )
		{
			mNewTextureResource->fLoadDefault( this );
			mNewTextureResource->fCallWhenLoaded( mOnLoadObserver );
		}
	}

	void tAsyncTexturedQuad::fBlockForLoad( )
	{
		if( mNewTextureResource )
			mNewTextureResource->fBlockUnitLoadedNoDependencies( );
	}

	b32 tAsyncTexturedQuad::fIsLoaded() const
	{
		return mTextureResource && mTextureResource->fLoaded( );
	}

	void tAsyncTexturedQuad::fSetOnLoadedCallback( const Sqrat::Function& func )
	{
		mCallback = func;
	}

	void tAsyncTexturedQuad::fOnLoad( tResource& resource, b32 success )
	{
		mNewTextureResource->fRemoveLoadCallback( mOnLoadObserver );
		if( success )
			fAddTexturedQuad( );
		else
		{
			mNewTextureResource->fUnload( this );
			mNewTextureResource.fRelease( );
		}
	}

	void tAsyncTexturedQuad::fUnload( )
	{
		if( fIsLoaded( ) )
		{
			fClearChildren( );
			mTextureResource->fRemoveLoadCallback( mOnLoadObserver );
			mTextureResource->fUnload( this );
			mTextureResource.fRelease( );
		}
	}

	void tAsyncTexturedQuad::fAddTexturedQuad( )
	{
		if( !mTextureResource || (mTextureResource && mTextureResource->fGetResourceId( ) != mNewTextureResource->fGetResourceId( )) )
		{
			fUnload( );

			mTextureResource = mNewTextureResource;
			mNewTextureResource.fRelease( );

			// Create new textured quad
			tTexturedQuad* quad = NEW tTexturedQuad( );
			fAddChild( tCanvasPtr( quad ) );
			quad->fSetTexture( mTextureResource );

			if( !mCallback.IsNull( ) )
				mCallback.Execute( this );
		}
	}

	Math::tVec2f tAsyncTexturedQuad::fTextureDimensions() const
	{
		if( fIsLoaded( ) )
			return fLocalRect( ).fWidthHeight( );
		else
			return Math::tVec2f( );
	}

} }

namespace Sig { namespace Gui
{

	void tAsyncTexturedQuad::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tAsyncTexturedQuad, tCanvasFrame, Sqrat::NoCopy<tAsyncTexturedQuad> > classDesc( vm.fSq( ) );

		classDesc
			.Func<void (tAsyncTexturedQuad::*)(const tFilePathPtr&)>( _SC("SetTexture"), &tAsyncTexturedQuad::fSetTexture)
			.Func( _SC("BlockForLoad"),		&tAsyncTexturedQuad::fBlockForLoad)
			.Func( _SC("IsLoaded"),			&tAsyncTexturedQuad::fIsLoaded)
			.Prop( _SC("OnLoaded"),			&tAsyncTexturedQuad::fOnLoadedCallback, &tAsyncTexturedQuad::fSetOnLoadedCallback )
			.Func( _SC("Unload"),			&tAsyncTexturedQuad::fUnload)
			.Func( _SC("TextureDimensions"), &tAsyncTexturedQuad::fTextureDimensions)
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("AsyncTexturedQuad"), classDesc);
	}

}}
