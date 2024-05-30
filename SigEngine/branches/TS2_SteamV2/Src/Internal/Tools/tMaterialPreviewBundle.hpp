#ifndef __tMaterialPreviewBundle__
#define __tMaterialPreviewBundle__
#include "Gfx/tDevice.hpp"
#include "Gfx/tDeviceResource.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "HlslGen/tHlslOutput.hpp"

namespace Sig
{
	class tSceneGraph;
	namespace Derml { class tFile; class tMtlFile; }

	class tools_export tMaterialPreviewGeometry : public Gfx::tDeviceResource, public Gfx::tRenderableEntity
	{
	public:
		tMaterialPreviewGeometry( );
		virtual void fRegenerateGeometry( const Gfx::tDevicePtr& device, const Gfx::tVertexFormatVRam* vtxFormat, const Gfx::tMaterial* mtl ) = 0;
		void fSetYaw( f32 yawInRads );
	};

	typedef tRefCounterPtr<tMaterialPreviewGeometry> tMaterialPreviewGeometryPtr;

	class tools_export tMaterialPreviewBundle : public tRefCounter
	{
	private:
		Gfx::tDevicePtr	mDevice;
		Gfx::tMaterialPtr mMaterial;
		HlslGen::tVshStyle mVshStyle;
		HlslGen::tHlslOutput mHlslGenOutput;
		tGrowableArray<tMaterialPreviewGeometryPtr> mPreviewGeometry;
	public:
		explicit tMaterialPreviewBundle( const Gfx::tDevicePtr& device, HlslGen::tVshStyle vshStyle = HlslGen::cVshMeshModel );
		~tMaterialPreviewBundle( );
		const Gfx::tDevicePtr& fDevice( ) const { return mDevice; }
		const Gfx::tMaterialPtr& fMaterial( ) const { return mMaterial; }
		const HlslGen::tHlslOutput& fHlslGenOutput( ) const { return mHlslGenOutput; }
		b32  fHasShaders( ) const;
		void fGenerateShaders( const Derml::tFile& dermlFile, HlslGen::tToolType toolType );
		void fUpdateMaterial( const Derml::tMtlFile& dermlMtlFile, Dx9Util::tTextureCache& textureCache );
		void fAddToSceneGraph( tSceneGraph& sg );
		void fRemoveFromSceneGraph( );
		void fSetYaw( f32 yawInRads );
		void fSetCurrentPreviewGeometry( u32 currentIndex );
	};
	define_smart_ptr( tools_export,  tRefCounterPtr, tMaterialPreviewBundle );
}

#endif//__tMaterialPreviewBundle__
