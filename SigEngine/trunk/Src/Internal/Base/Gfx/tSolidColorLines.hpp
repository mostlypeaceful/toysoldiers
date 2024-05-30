#ifndef __tSolidColorLines__
#define __tSolidColorLines__
#include "tSolidColorGeometry.hpp"
#include "tRenderableEntity.hpp"

namespace Sig { namespace Gfx
{

	class base_export tSolidColorLines : public tRenderableEntity
	{
		define_dynamic_cast( tSolidColorLines, tRenderableEntity );
	public:
		void fResetDeviceObjects( 
			const tDevicePtr& device,
			const tMaterialPtr& material, 
			const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const tIndexBufferVRamAllocatorPtr& indexAllocator );
		void fBake( const Sig::byte* verts, u32 numVerts, b32 strip );
		void fSetRenderStateOverride( const tRenderState* rs ) { mGeometry.fSetRenderStateOverride( rs ); }
		const tMaterialPtr& fMaterial( ) const { return mGeometry.fMaterial( ); }
		const tRenderBatchPtr& fGetRenderBatch( ) const { return mGeometry.fGetRenderBatch( ); }

	private:
		tSolidColorGeometry mGeometry;
	};

	typedef tRefCounterPtr<tSolidColorLines> tSolidColorLinesPtr;

}}


#endif//__tSolidColorLines__
