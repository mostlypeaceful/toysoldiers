#ifndef __tWorldSpaceQuads__
#define __tWorldSpaceQuads__
#include "tRenderableEntity.hpp"
#include "tDeviceResource.hpp"
#include "tMaterial.hpp"
#include "tDynamicGeometry.hpp"

namespace Sig { namespace Gfx
{
	class tScreenPtr;
	class tDevice;
	struct tFullBrightRenderVertex;

	// Does not manage its own bounding box

	class base_export tWorldSpaceQuads : public tRenderableEntity, public tDeviceResource
	{
		define_dynamic_cast( tWorldSpaceQuads, tRenderableEntity );
	private:
		tResourcePtr		mColorMap;
		tMaterialPtr		mMaterial;
		tDynamicGeometry	mGeometry;

		tGrowableArray<tFullBrightRenderVertex> mSysMemVerts;
		u32 mQuadCount;

	public:
		tWorldSpaceQuads( );
		~tWorldSpaceQuads( );
		virtual b32 fIsHelper( ) const { return true; }
		virtual u32 fSpatialSetIndex( ) const { return cEffectSpatialSetIndex; }
		virtual void fOnDeviceLost( tDevice* device );
		virtual void fOnDeviceReset( tDevice* device );

		///
		/// \brief Reset device objects using a full bright material and a texture.
		void fResetDeviceObjectsTexture( 
			const tDevicePtr& device,
			const tResourcePtr& colorMap,
			const tResourcePtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );

		///
		/// \brief Dynamically swap the texture associated with this screen space quad.
		void fChangeColorMap( const tResourcePtr& texResource, const tResourcePtr& materialFile );
		
		///
		/// \brief Specifies the number of quads allocated internally.
		void fSetQuadCount( u32 cnt );
		u32 fQuadCount( ) const;

		///
		/// \brief Allows users to fill out quad data. returns pointer to
		/// a contiguous 4 vertices in the order { -x-y, -x y, x y, x -y }
		tFullBrightRenderVertex* fQuad( u32 index );

		///
		/// \brief Creates a single quad in the XZ plane extending .5 in each direction.
		void fCreateDefaultQuad( );

		///
		/// \brief Why not. 
		tDynamicGeometry& fGeometry( ) { return mGeometry; }


		///
		/// \brief Resubmit geometry.
		void fCreateGeometry( Gfx::tDevice& device );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tWorldSpaceQuads );

}}

#endif//__tWorldSpaceQuads__
