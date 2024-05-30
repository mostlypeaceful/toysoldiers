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

	class tTexturedQuad : public tRenderableCanvas
	{
		define_dynamic_cast( tTexturedQuad, tRenderableCanvas );
	public:
		tTexturedQuad( );
		explicit tTexturedQuad( Gfx::tDefaultAllocators& allocators );
		virtual void fOnDeviceReset( Gfx::tDevice* device );

		b32 fHasTexture( ) const;
		void fSetTexture( const tFilePathPtr& texturePath );
		void fSetTexture( Gfx::tTextureReference::tPlatformHandle handle, const Math::tVec2f & dims );
		virtual void fSetTexture( const tResourcePtr& textureResource );
		Math::tVec2f fTextureDimensions( ) const;
		void fSetPivot( const Math::tVec2f& point );
		void fCenterPivot( );
		void fSetRectFromTexture( b32 center = false );
		void fSetRect( const Math::tVec2f& topLeft, const Math::tVec2f& widthHeight );
		void fSetRect( const Math::tVec2f& widthHeight );
		void fSetRect( const tRect& rect );
		void fSetTextureRect( const Math::tVec2f& corner, const Math::tVec2f& extent );

		const tResourcePtr&			fColorMapResource( ) const { return mColorMap; }
		const Gfx::tTextureFile&	fColorMapTexture( ) const { return *mColorMap->fCast<Gfx::tTextureFile>( ); }
		const Gfx::tRenderState&	fRenderState( ) const { return mRenderState; }
		void						fSetRenderState( const Gfx::tRenderState& state ) { mRenderState = state; }

	protected:

		void fCommonCtor( );
		void fResetDeviceObjects( Gfx::tDevice& device );
		void fSetFilterMode( Gfx::tTextureFile::tFilterMode filterMode );
		void fAutoConfigureSamplingModes( );
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

		int mLastSetRectMode;
		tRect mLastSetRect;
		Math::tVec2f mLastSetCorner;
		Math::tVec2f mLastSetExtent;
	};

}}

#endif//__tTexturedQuad__
