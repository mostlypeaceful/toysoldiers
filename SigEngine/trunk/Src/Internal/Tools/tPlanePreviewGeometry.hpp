#ifndef __tPlanePreviewGeometry__
#define __tPlanePreviewGeometry__
#include "tMaterialPreviewBundle.hpp"
#include "Gfx/tGeometryBufferVRam.hpp"
#include "Gfx/tIndexBufferVRam.hpp"

namespace Sig
{
	///
	/// \brief Encapsulates plane object used to render shader/material preview in the material editor.
	class tPlanePreviewGeometry : public tMaterialPreviewGeometry
	{
		Gfx::tGeometryBufferVRam		mVerts;
		Gfx::tIndexBufferVRam			mIndices;
		Gfx::tRenderState				mRenderState;
		b32								mFacingParticleQuad;
		Math::tMat3f					mXformVertices;
	public:
		explicit tPlanePreviewGeometry( b32 facingParticleQuad = false, const Math::tMat3f& xformVertices = Math::tMat3f::cIdentity, b32 xparentBatch = false );
		virtual void fOnDeviceLost( Gfx::tDevice* device ) { }
		virtual void fOnDeviceReset( Gfx::tDevice* device ) { }
		virtual void fRegenerateGeometry( const Gfx::tDevicePtr& device, const Gfx::tVertexFormatVRam* vtxFormat, const Gfx::tMaterial* mtl );
	};
}

#endif//__tPlanePreviewGeometry__
