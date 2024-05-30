#ifndef __tCubePreviewGeometry__
#define __tCubePreviewGeometry__
#include "tMaterialPreviewBundle.hpp"
#include "Gfx/tGeometryBufferVRam.hpp"
#include "Gfx/tIndexBufferVRam.hpp"
#include "gfx/tDynamicGeometry.hpp"

namespace Sig
{
	///
	/// \brief Encapsulates cube object used to render shader/material preview in the material editor.
	class tCubePreviewGeometry : public tMaterialPreviewGeometry
	{
		Gfx::tGeometryBufferVRam		mVerts;
		Gfx::tIndexBufferVRam			mIndices;
		Gfx::tRenderState				mRenderState;
		Math::tMat3f					mXformVertices;
		b32								mCubeMap;
	public:
		explicit tCubePreviewGeometry( const Math::tMat3f& xformVertices = Math::tMat3f::cIdentity, b32 cubeMap = false );
		virtual void fOnDeviceLost( Gfx::tDevice* device ) { }
		virtual void fOnDeviceReset( Gfx::tDevice* device ) { }
		virtual void fRegenerateGeometry( const Gfx::tDevicePtr& device, const Gfx::tVertexFormatVRam* vtxFormat, const Gfx::tMaterial* mtl );
	};
}

#endif//__tCubePreviewGeometry__
