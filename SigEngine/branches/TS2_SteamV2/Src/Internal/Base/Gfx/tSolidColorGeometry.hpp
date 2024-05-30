#ifndef __tSolidColorGeometry__
#define __tSolidColorGeometry__
#include "tMaterial.hpp"
#include "tDynamicGeometry.hpp"
#include "tDeviceResource.hpp"

namespace Sig { namespace Gfx
{
	struct tSolidColorRenderVertex;

	///
	/// \brief Base type supporting dynamic geometry shapes rendered
	/// using the tSolidColorMaterial.
	class base_export tSolidColorGeometry : public tDynamicGeometry, public tDeviceResource
	{
		tMaterialPtr mMaterial;
	public:

		tSolidColorGeometry( );
		virtual void fOnDeviceLost( tDevice* device ) { }
		virtual void fOnDeviceReset( tDevice* device ) { }

		void fResetDeviceObjects( 
			const tDevicePtr& device,
			const tMaterialPtr& material, 
			const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const tIndexBufferVRamAllocatorPtr& indexAllocator );
		void fBake( 
			tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
			tGrowableArray< u16 >& ids, u32 numPrims );
		const tMaterialPtr& fMaterial( ) const { return mMaterial; }
	};
}}


#endif//__tSolidColorGeometry__
