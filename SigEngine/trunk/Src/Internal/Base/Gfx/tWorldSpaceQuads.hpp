#ifndef __tWorldSpaceQuads__
#define __tWorldSpaceQuads__
#include "tRenderableEntity.hpp"
#include "tDeviceResource.hpp"
#include "tMaterial.hpp"
#include "tDynamicGeometry.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	class tScreenPtr;
	class tDevice;
	struct tFullBrightRenderVertex;

	// Does not manage its own bounding box

	class base_export tWorldSpaceQuads : public tRenderableEntity, public tDeviceResource
	{
		debug_watch( tWorldSpaceQuads );
		declare_uncopyable( tWorldSpaceQuads );
		define_dynamic_cast( tWorldSpaceQuads, tRenderableEntity );
	private:
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
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
		{
			tTextureReference texRef; texRef.fSetDynamic( colorMap.fGetRawPtr( ) );
			fResetDeviceObjectsTexture(
				device, texRef, material, geometryAllocator, indexAllocator );
		}

		void fResetDeviceObjectsTexture( 
			const tDevicePtr& device,
			const tTextureReference & colorMap,
			const tResourcePtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );

		///
		/// \brief Dynamically swap the texture associated with this screen space quad.
		void fChangeColorMap( const tResourcePtr& texResource, const tResourcePtr& materialFile )
		{
			tTextureReference texRef; texRef.fSetDynamic( texResource.fGetRawPtr( ) );
			fChangeColorMap( texRef, materialFile );
		}
		void fChangeColorMap( const tTextureReference & colorMap, const tResourcePtr & materialFile );
		
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
		void fCreateDefaultQuad( ) { fCreateDefaultQuad( Math::tVec3f::cXAxis, Math::tVec3f::cZAxis ); }
		void fCreateDefaultQuad( const Math::tVec3f & x, const Math::tVec3f & y );

		///
		/// \brief Why not. 
		tDynamicGeometry& fGeometry( ) { return mGeometry; }
		const tMaterialPtr & fMaterial( ) { return mMaterial; }

		///
		/// \brief Resubmit geometry.
		void fCreateGeometry( Gfx::tDevice& device );

	private:

	};

	define_smart_ptr( base_export, tRefCounterPtr, tWorldSpaceQuads );

}}

#endif//__tWorldSpaceQuads__
