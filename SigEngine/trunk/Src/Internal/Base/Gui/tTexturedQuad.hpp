#ifndef __tTexturedQuad__
#define __tTexturedQuad__
#include "tRenderableCanvas.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tDynamicGeometry.hpp"
#include "Gfx/tTextureFile.hpp"

namespace Sig { namespace Gfx
{
	class tDevice;
	struct tDefaultAllocators;
	class tTextureReference;
}}

namespace Sig { namespace Gui
{

	class base_export tTexturedQuad : public tRenderableCanvas
	{
		define_dynamic_cast( tTexturedQuad, tRenderableCanvas );
	public:
		tTexturedQuad( );
		explicit tTexturedQuad( Gfx::tDefaultAllocators& allocators );
		virtual ~tTexturedQuad( );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
		
		b32 fHasTexture( ) const;
		
		void fSetTexture( const tFilePathPtr& texturePath );
		void fSetTexture( const tResourcePtr& textureResource );
		void fSetTexture( const Gfx::tTextureReference & texture, const Math::tVec2f & dims );
		void fSetTexture( Gfx::tTextureFile::tPlatformHandle handle, const Math::tVec2f & dims );

		Math::tVec2f fTextureDimensions( ) const;

		void fSetPivot( const Math::tVec2f& point );
		void fCenterPivot( );
		
		void fSetRectFromTexture( b32 center = false );
		void fSetRect( const Math::tVec2f& topLeft, const Math::tVec2f& widthHeight );
		void fSetRect( const Math::tVec2f& widthHeight );
		void fSetRect( const Math::tRect& rect );

		void fSetTextureRect( const Math::tVec2f& corner, const Math::tVec2f& extent );

		const tResourcePtr&			fColorMapResource( ) const { return mColorMap; }
		const Gfx::tTextureFile&	fColorMapTexture( ) const { return *mColorMap->fCast<Gfx::tTextureFile>( ); }
		const Gfx::tRenderState&	fRenderState( ) const { return mRenderState; }
		void						fSetRenderState( const Gfx::tRenderState& state ) { mRenderState = state; }

		void fSetFilterMode( Gfx::tTextureFile::tFilterMode filterMode );

	protected:

		void fCommonCtor( );
		void fResetDeviceObjects( Gfx::tDevice& device );
		void fAutoConfigureSamplingModes( );
		void fSetTextureFromScript( Gfx::tTextureReference * texture, const Math::tVec2f & dims );

		virtual void fOnMoved( );
		virtual void fOnParentMoved( );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		Gfx::tFullBrightMaterialPtr	mMaterial;
		tResourcePtr				mColorMap;
		typedef tFixedArray<Gfx::tFullBrightRenderVertex,4> tVertexSysMemArray;
		tVertexSysMemArray			mVerts;
		Gfx::tDynamicGeometry		mGeometry;
		Gfx::tRenderState			mRenderState;
		Gfx::tDefaultAllocators&	mAllocators;
		Math::tVec2f				mTextureDimensions;

		Gfx::tTextureReference* fColorMapScript( ) const { return fHasTexture( ) ? &mMaterial->mColorMap : NULL; }
	};

}}

#endif//__tTexturedQuad__
