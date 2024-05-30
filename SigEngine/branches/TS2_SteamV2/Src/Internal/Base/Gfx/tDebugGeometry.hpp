#ifndef __tDebugGeometry__
#define __tDebugGeometry__

namespace Sig { class tResourceDepot; }

namespace Sig { namespace Gfx
{
	class tDevicePtr;
	class tViewport;
	class tRenderBatchPtr;
	class tWorldSpaceDisplayList;
	class tDebugGeometry;
	class tDebugGeometryPtr;

#ifdef sig_devmenu

	struct tDebugGeometryContainerImpl;

	class base_export tDebugGeometryContainer : public tUncopyable
	{
	private:
		tDebugGeometryContainerImpl* mImpl;
	public:
		tDebugGeometryContainer( );
		~tDebugGeometryContainer( );

		void fResetDeviceObjects( tResourceDepot& resourceDepot, const tDevicePtr& device );
		void fRenderOnce( const Math::tTrianglef& tri, const Math::tVec4f& rgba );
		void fRenderOnce( const Math::tRayf& ray, const Math::tVec4f& rgba );
		void fRenderOnce( const Math::tAabbf& aabb, const Math::tVec4f& rgba );
		void fRenderOnce( const Math::tObbf& obb, const Math::tVec4f& rgba );
		void fRenderOnce( const Math::tSpheref& sphere, const Math::tVec4f& rgba );
		void fRenderOnce( const Math::tSpheref& sphere, const Math::tMat3f& xform, const Math::tVec4f& rgba );
		void fRenderOnce( const tPair<Math::tVec3f,Math::tVec3f>& line, const Math::tVec4f& rgba );
		void fRenderOnce( const Math::tVec3f& linePtA, const Math::tVec3f& linePtB, const Math::tVec4f& rgba );
		void fRenderOnce( const Math::tMat3f& basis, f32 length, f32 alpha = 1.0f );
		void fRenderOnce( const Math::tPlanef& plane, const Math::tVec3f& center, f32 scale, const Math::tVec4f& rgba );

		void fAddToDisplayList( const tViewport& viewport, tWorldSpaceDisplayList& displayList );
		void fClear( );

	private:
		void fAddObject( const tDebugGeometryPtr& object, const Math::tVec4f& rgba, const tRenderBatchPtr& batch );
	};

#else//sig_devmenu

	class base_export tDebugGeometryContainer : public tUncopyable
	{
	public:
		inline void fResetDeviceObjects( tResourceDepot& resourceDepot, const tDevicePtr& device ) { }
		inline void fRenderOnce( const Math::tTrianglef& tri, const Math::tVec4f& rgba ) { }
		inline void fRenderOnce( const Math::tRayf& ray, const Math::tVec4f& rgba ) { }
		inline void fRenderOnce( const Math::tAabbf& aabb, const Math::tVec4f& rgba ) { }
		inline void fRenderOnce( const Math::tObbf& obb, const Math::tVec4f& rgba ) { }
		inline void fRenderOnce( const Math::tSpheref& sphere, const Math::tVec4f& rgba ) { }
		inline void fRenderOnce( const Math::tSpheref& sphere, const Math::tMat3f& xform, const Math::tVec4f& rgba ) { }
		inline void fRenderOnce( const tPair<Math::tVec3f,Math::tVec3f>& line, const Math::tVec4f& rgba ) { }
		inline void fRenderOnce( const Math::tVec3f& linePtA, const Math::tVec3f& linePtB, const Math::tVec4f& rgba ) { }
		inline void fRenderOnce( const Math::tMat3f& basis, f32 length, f32 alpha = 1.0f ) { }
		inline void fRenderOnce( const Math::tPlanef& plane, const Math::tVec3f& center, f32 scale, const Math::tVec4f& rgba ) { }
		inline void fAddToDisplayList( const tViewport& viewport, tWorldSpaceDisplayList& displayList ) { }
		inline void fClear( ) { }
	};

#endif//sig_devmenu

}}

#endif//__tDebugGeometry__

