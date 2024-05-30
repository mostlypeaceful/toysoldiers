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
	/// using the tSolidColorMaterial (or any material actually.)

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

		const tMaterialPtr& fMaterial( ) const { return mMaterial; }

		//clones another objects devices
		void fResetDeviceObjects( const tSolidColorGeometry& clone );

		template< class vertType >
		void fBake( tGrowableArray< vertType >& verts,
					tGrowableArray< u16 >& ids, u32 numPrims )
		{
			if( !fAllocateGeometry( *mMaterial, verts.fCount( ), ids.fCount( ), numPrims ) )
				return; // couldn't get geometry

			// copy vert data to gpu
			fCopyVertsToGpu( verts.fBegin( ), verts.fCount( ) );

			// generate indices
			fCopyIndicesToGpu( ids.fBegin( ), ids.fCount( ) );
		}
	};
}}


#endif//__tSolidColorGeometry__
