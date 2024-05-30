#ifndef __tSkeletonVisualizer__
#define __tSkeletonVisualizer__
#include "tAnimatedSkeleton.hpp"
#include "Gfx/tSolidColorLines.hpp"
#include "Gfx/tSolidColorSphere.hpp"

namespace Sig { namespace Gfx
{
	class tScreenPtr;
}}

namespace Sig
{
	class base_export tSkeletonVisualizer
	{
		// the animated skeleton instance
		tAnimatedSkeletonPtr mSkeleton;

		// transparent renderstate
		Gfx::tRenderState mRenderStateOverride;

		// bones (as lines)
		Gfx::tSolidColorLinesPtr mLines;
		tGrowableArray<Gfx::tSolidColorRenderVertex> mLineVerts;

		// joints (as spheres)
		u32 mCurrentJoint;
		Gfx::tSolidColorSphere mSphereTemplate;
		tGrowableArray<Gfx::tRenderableEntityPtr> mJointInstances;

		// skeleton attachments
		tGrowableArray<Gfx::tRenderableEntityPtr> mBoneProxies;

	public:
		tSkeletonVisualizer( );
		~tSkeletonVisualizer( );
		void fResetDeviceObjects(
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );
		void fSetSkeleton( const tAnimatedSkeletonPtr& animatedSkel );
		void fSyncToSkeleton( const Math::tMat3f& xform );
		void fAddToScreen( const Gfx::tScreenPtr& screen );

		b32 fNeedsDeviceObjects( ) { return mLines.fNull( ); }

	private:
		void fGenerateRenderables( const tAnimatedSkeleton::tBoneLine& root );
		void fUpdateRenderables( const tAnimatedSkeleton::tBoneLine& root );
	};

}


#endif//__tSkeletonVisualizer__

