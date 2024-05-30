#ifndef __tStaticDecalTris__
#define __tStaticDecalTris__
#include "tRenderableEntity.hpp"
#include "tDeviceResource.hpp"
#include "tMaterial.hpp"
#include "tDynamicGeometry.hpp"

namespace Sig { namespace Gfx
{
	class tScreenPtr;
	class tDevice;
	class tDecalMaterial;
	struct tDecalRenderVertex;

	// Does not manage its own bounding box

	class base_export tStaticDecalGeometry : public tRenderableEntity, public tDeviceResource
	{
		define_dynamic_cast( tStaticDecalGeometry, tRenderableEntity );
	private:
		tResourcePtr		mDiffuseMap;
		tResourcePtr		mNormalMap;
		tMaterialPtr		mMaterial;
		tDynamicGeometry	mGeometry;
		tRenderState		mRenderState;

	public:
		tStaticDecalGeometry( );
		~tStaticDecalGeometry( );
		virtual b32 fIsHelper( ) const { return true; }
		virtual u32 fSpatialSetIndex( ) const { return cEffectSpatialSetIndex; }
		virtual void fOnDeviceLost( tDevice* device );
		virtual void fOnDeviceReset( tDevice* device );

		///
		/// \brief Reset device objects using a material
		void fResetDeviceObjectsMaterial( 
			const tDevicePtr& device,
			const u16* indices, u32 indexCount,
			const Gfx::tDecalRenderVertex* verts, u32 vertCount,
			Gfx::tDecalMaterial* newMat,
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );

		void fChangeMaterial( Gfx::tDecalMaterial* newMat );

		///
		/// \brief Why not. 
		tDynamicGeometry& fGeometry( ) { return mGeometry; }

		///
		/// \brief Resubmit geometry.
		void fCreateGeometry( 
			Gfx::tDevice& device,
			const u16* indices, u32 indexCount,
			const Gfx::tDecalRenderVertex* verts, u32 vertCount );

		void fSetRenderState( const tRenderState& state ) { mRenderState = state; }
		const tRenderState& fRenderState( ) const { return mRenderState; }
	};

	define_smart_ptr( base_export, tRefCounterPtr, tStaticDecalGeometry );

}}

#endif//__tStaticDecalTris__
