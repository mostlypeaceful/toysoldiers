#ifndef __tSpherePreviewGeometry__
#define __tSpherePreviewGeometry__
#include "tMaterialPreviewBundle.hpp"
#include "Gfx/tGeometryBufferVRam.hpp"
#include "Gfx/tIndexBufferVRam.hpp"

namespace Sig
{
	///
	/// \brief Encapsulates sphere object used to render shader/material preview in the material editor.
	class tSpherePreviewGeometry : public tMaterialPreviewGeometry
	{
		Gfx::tGeometryBufferVRam		mVerts;
		Gfx::tIndexBufferVRam			mIndices;
		Gfx::tRenderState				mRenderState;
	public:
		tSpherePreviewGeometry( b32 xparentBatch );
		virtual void fOnDeviceLost( Gfx::tDevice* device ) { }
		virtual void fOnDeviceReset( Gfx::tDevice* device ) { }
		virtual void fRegenerateGeometry( const Gfx::tDevicePtr& device, const Gfx::tVertexFormatVRam* vtxFormat, const Gfx::tMaterial* mtl );
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
	};
}

#endif//__tSpherePreviewGeometry__
