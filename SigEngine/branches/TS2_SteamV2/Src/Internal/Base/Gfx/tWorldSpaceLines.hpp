#ifndef __tWorldSpaceLines__
#define __tWorldSpaceLines__
#include "tRenderableEntity.hpp"
#include "tDeviceResource.hpp"
#include "tSolidColorLines.hpp"

namespace Sig { namespace Gfx
{
	class base_export tWorldSpaceLines : public tRenderableEntity, public tDeviceResource
	{
		define_dynamic_cast( tWorldSpaceLines, tRenderableEntity );
	private:
		tSolidColorLines mGeometry;
		tGrowableArray<tSolidColorRenderVertex> mSysMemVerts;
		b32 mStrip;
	public:
		tWorldSpaceLines( );
		virtual b32 fIsHelper( ) const { return true; }
		virtual u32 fSpatialSetIndex( ) const { return cEffectSpatialSetIndex; }
		virtual void fOnDeviceLost( tDevice* device );
		virtual void fOnDeviceReset( tDevice* device );

		///
		/// \brief Reset device objects using a solid color material.
		void fResetDeviceObjects( 
			const tDevicePtr& device,
			const tMaterialPtr& material, 
			const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const tIndexBufferVRamAllocatorPtr& indexAllocator );

		void fSetGeometry( tGrowableArray<tSolidColorRenderVertex>& solidColorVerts, b32 strip );
		void fSetRenderStateOverride( const tRenderState* rs ) { mGeometry.fSetRenderStateOverride( rs ); }

	private:
		void fSetGeometryInternal( const tSolidColorRenderVertex* solidColorVerts, u32 numVerts, b32 strip );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tWorldSpaceLines );

}}

#endif//__tWorldSpaceLines__
