#ifndef __tWorldSpaceDecal__
#define __tWorldSpaceDecal__
#include "tRenderableEntity.hpp"
#include "tDeviceResource.hpp"
#include "tMaterial.hpp"
#include "tDynamicGeometry.hpp"
#include "tSceneGraphCollectTris.hpp"

namespace Sig { namespace Gfx
{
	struct tFullBrightRenderVertex;
	class tDevice;

	class base_export tWorldSpaceDecal : public tRenderableEntity, public tDeviceResource
	{
		define_dynamic_cast( tWorldSpaceDecal, tRenderableEntity );
	public:
		explicit tWorldSpaceDecal( const tResourcePtr& colorMap, const Math::tObbf& projectionBox, Gfx::tDevice& device, tEntityTagMask hitFlags );
		~tWorldSpaceDecal( );
		virtual b32 fIsHelper( ) const { return true; }
		virtual u32 fSpatialSetIndex( ) const { return cEffectSpatialSetIndex; }
		virtual void fOnDeviceLost( tDevice* device );
		virtual void fOnDeviceReset( tDevice* device );
		virtual void fOnSpawn( );
		virtual void fActST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

	private:
		Math::tObbf			mProjectionBox;
		Math::tAabbf		mBoundingBox;
		tResourcePtr		mColorMap;
		tMaterialPtr		mMaterial;
		tDynamicGeometry	mGeometry;
		Gfx::tRenderState	mRenderState;
		tRefCounterPtr<tDevice> mDevice;
		tEntityTagMask		mHitFlags;

		f32 mAge;
		b8 mGeometryCreated;
		b8 pad0, pad1, pad2;

		Math::tVec2f fComputeTexCoord( const Math::tVec3f& pos );
		void fBuildGeometryMT( );
		void fChangeColorMap( const tResourcePtr& texResource, const tResourcePtr& materialFile );
		void fCreateGeometry( Gfx::tDevice& device );

		tGrowableArray<tFullBrightRenderVertex> mSysMemVerts;
		tGrowableArray<Math::tTrianglef> mTris;
	};

	typedef tRefCounterPtr<tWorldSpaceDecal> tWorldSpaceDecalPtr;
}}

#endif//__tWorldSpaceDecal__
