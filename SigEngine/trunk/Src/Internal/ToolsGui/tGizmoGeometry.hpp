#ifndef __tGizmoGeometry__
#define __tGizmoGeometry__
#include "Gfx/tDynamicGeometry.hpp"
#include "Gfx/tDeviceResource.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig
{
	namespace Gfx { class tDevice; }

	///
	/// \brief Base type for gizmo geometry. Provides ray-casting and
	/// other functionality common to the various derived gizmo types.
	class toolsgui_export tGizmoGeometry : public Gfx::tDynamicGeometry, public Gfx::tDeviceResource
	{
	protected:
		enum tAxis
		{
			cAxisX,
			cAxisY,
			cAxisZ,
			cAxisXY,
			cAxisXZ,
			cAxisYZ,
			cAxisXYZ,
			cAxisCount
		};
		typedef tGrowableArray< Math::tTrianglef > tTriangleArray;
		tFixedArray< tTriangleArray, cAxisCount > mAxisTris;
		Gfx::tRenderState mRenderState;
		Gfx::tMaterialPtr mMaterial;

	protected:
		tGizmoGeometry( );
	public:
		~tGizmoGeometry( );
	protected:
		virtual void fOnDeviceLost( Gfx::tDevice* device ) { }
		virtual void fOnDeviceReset( Gfx::tDevice* device ) { }
		void fResetDeviceObjects( 
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );
		void fAddLastTriangle( 
			tTriangleArray& triArray,
			const tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
			const tGrowableArray< u16 >& ids );
		void fAddTwoAxisWebbing( 
			f32 webbingLength,
			tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
			tGrowableArray< u16 >& ids );
		void fBake( const tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, const tGrowableArray<u16>& ids );

	public:
		void fRayCast( const Math::tRayf& rayInLocal, Math::tVec3f& intersectionPointInLocal, b32& x, b32& y, b32& z );
	};
	typedef tRefCounterPtr< tGizmoGeometry > tGizmoGeometryPtr;

}

#endif//__tGizmoGeometry__
