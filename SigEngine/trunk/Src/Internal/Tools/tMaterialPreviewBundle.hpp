#ifndef __tMaterialPreviewBundle__
#define __tMaterialPreviewBundle__
#include "Gfx/tDevice.hpp"
#include "Gfx/tDeviceResource.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "HlslGen/tHlslOutput.hpp"
#include "Gfx/tLightEntity.hpp"

namespace Sig
{
	class tSceneGraph;
	namespace Derml { class tFile; class tMtlFile; }
	namespace Gfx { class tLightEntity; }

	class tools_export tMaterialPreviewGeometry : public Gfx::tDeviceResource, public Gfx::tRenderableEntity
	{
	public:
		tMaterialPreviewGeometry( );

		virtual void fRegenerateGeometry( const Gfx::tDevicePtr& device, const Gfx::tVertexFormatVRam* vtxFormat, const Gfx::tMaterial* mtl ) = 0;
		void fSetYaw( f32 yawInRads );
		void fEnableRaycast( b32 enable ) { mEnableRayCast = enable; }

	protected:
		b32 mEnableRayCast;
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
		tRefCounterPtr< Gfx::tLightEntity > mLensFlareLight;

		Gfx::tMaterialPtr mCubeMapRoomMaterial;
		Dx9Util::tBaseTexturePtr mCubeMapRoomTexture;
		Gfx::tVertexFormatVRam mFullBrightFormat;

		enum tPreviewGeometryComponent
		{
			cPreviewGeometryComponentSphere,		//supports transparent
			cPreviewGeometryComponentPlane,			//does not support transparent
			cPreviewGeometryComponentGroundPlane,	// does not support transparent
			cPreviewGeometryComponentOpaqueSphere,	// does not support transparent
			cPreviewGeometryComponentSkyBox,		// full bright material
			cPreviewGeometryComponentLight,			// full bright material
			cPreviewGeometryComponentCount
		};

		static b32 fFullBrightObject( u32 index ) { return (index == cPreviewGeometryComponentSkyBox); }

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
		void fSetCubeMapTexture( Dx9Util::tBaseTexturePtr& newTex );

		enum tPreviewGeometryMode
		{
			cPreviewGeometryModeSphere,
			cPreviewGeometryModePlane,
			cPreviewGeometryModeSphereAndShadowedPlane,
			cPreviewGeometryModeCount
		};

		void fSetCurrentPreviewGeometry( tPreviewGeometryMode mode );

		Gfx::tLightEntity* fLenseFlareLight( ) { return mLensFlareLight.fGetRawPtr( ); }
	};
	define_smart_ptr( tools_export,  tRefCounterPtr, tMaterialPreviewBundle );
}

#endif//__tMaterialPreviewBundle__
