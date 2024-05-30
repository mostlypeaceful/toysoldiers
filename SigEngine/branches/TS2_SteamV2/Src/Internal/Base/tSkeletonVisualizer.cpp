#include "BasePch.hpp"
#include "tSkeletonVisualizer.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tScreen.hpp"

namespace Sig
{
	tSkeletonVisualizer::tSkeletonVisualizer( )
		: mCurrentJoint( 0 )
	{
	}

	tSkeletonVisualizer::~tSkeletonVisualizer( )
	{
	}

	void tSkeletonVisualizer::fResetDeviceObjects(
		const Gfx::tDevicePtr& device,
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		mRenderStateOverride = Gfx::tRenderState::cDefaultColorTransparent;

		mLines.fReset( NEW Gfx::tSolidColorLines( ) );
		mLines->fSetRenderStateOverride( &mRenderStateOverride );
		mLines->fResetDeviceObjects( device, material, geometryAllocator, indexAllocator );

		mSphereTemplate.fSetRenderStateOverride( &mRenderStateOverride );
		mSphereTemplate.fResetDeviceObjects( device, material, geometryAllocator, indexAllocator );
		mSphereTemplate.fGenerate( 0.04f );
	}

	void tSkeletonVisualizer::fSetSkeleton( const tAnimatedSkeletonPtr& animatedSkel )
	{
		mSkeleton = animatedSkel;

		tAnimatedSkeleton::tBoneLine boneLines;
		mSkeleton->fConstructBoneLines( boneLines, Math::tMat3f::cIdentity );
		mCurrentJoint = 0;
		fGenerateRenderables( boneLines );

		const Math::tVec4f jointsRgba = Math::tVec4f( 0.6f, 0.8f, 1.f, 0.75f );
		mJointInstances.fSetCount( mCurrentJoint );
		for( u32 i = 0; i < mJointInstances.fCount( ); ++i )
		{
			mJointInstances[ i ].fReset( NEW Gfx::tRenderableEntity( mSphereTemplate.fGetRenderBatch( ) ) );
			mJointInstances[ i ]->fSetRgbaTint( jointsRgba );
		}

		const u32 numBoneProxies = fPtrDiff( mSkeleton->fBoneProxiesEnd( ), mSkeleton->fBoneProxiesBegin( ) );
		const Math::tVec4f proxiesRgba = Math::tVec4f( 2.0f, 0.2f, 0.2f, 0.75f );
		mBoneProxies.fSetCount( numBoneProxies );
		for( u32 i = 0; i < mBoneProxies.fCount( ); ++i )
		{
			mBoneProxies[ i ].fReset( NEW Gfx::tRenderableEntity( mSphereTemplate.fGetRenderBatch( ) ) );
			mBoneProxies[ i ]->fSetRgbaTint( proxiesRgba );
		}
	}

	void tSkeletonVisualizer::fSyncToSkeleton( const Math::tMat3f& ogXform )
	{
		const Math::tMat3f xform = ogXform * mSkeleton->fBindOffset( );

		tAnimatedSkeleton::tBoneLine boneLines;
		mSkeleton->fConstructBoneLines( boneLines, xform );

		mLineVerts.fSetCount( 0 );
		mCurrentJoint = 0;

		fUpdateRenderables( boneLines );

		u32 ithProxy = 0;
		for( tAnimatedSkeleton::tBoneProxyList::tConstIterator iproxy = mSkeleton->fBoneProxiesBegin( ), iproxyEnd = mSkeleton->fBoneProxiesEnd( );
				iproxy != iproxyEnd; ++iproxy, ++ithProxy )
		{
			Math::tMat3f m = (*iproxy).fParent( )->fObjectToWorld( );
			m.fNormalizeBasis( );
			mBoneProxies[ ithProxy ]->fMoveTo( m );

			const f32 axisScale = 0.1f;
			const Math::tVec3f c = m.fGetTranslation( );
			const Math::tVec3f x = axisScale * m.fXAxis( );
			const Math::tVec3f y = axisScale * m.fYAxis( );
			const Math::tVec3f z = axisScale * m.fZAxis( );
			const u32 xcolor = Gfx::tVertexColor( 1.f, 0.f, 0.f, 1.f ).fForGpu( );
			const u32 ycolor = Gfx::tVertexColor( 0.f, 1.f, 0.f, 1.f ).fForGpu( );
			const u32 zcolor = Gfx::tVertexColor( 0.f, 0.f, 1.f, 1.f ).fForGpu( );

			mLineVerts.fPushBack( Gfx::tSolidColorRenderVertex( c, xcolor ) );
			mLineVerts.fPushBack( Gfx::tSolidColorRenderVertex( c + x, xcolor ) );
			mLineVerts.fPushBack( Gfx::tSolidColorRenderVertex( c, ycolor ) );
			mLineVerts.fPushBack( Gfx::tSolidColorRenderVertex( c + y, ycolor ) );
			mLineVerts.fPushBack( Gfx::tSolidColorRenderVertex( c, zcolor ) );
			mLineVerts.fPushBack( Gfx::tSolidColorRenderVertex( c + z, zcolor ) );
		}

		mLines->fBake( ( const Sig::byte* )mLineVerts.fBegin( ), mLineVerts.fCount( ), false );
	}

	void tSkeletonVisualizer::fGenerateRenderables( const tAnimatedSkeleton::tBoneLine& root )
	{
		++mCurrentJoint;
		for( u32 i = 0; i < root.mChildren.fCount( ); ++i )
			fGenerateRenderables( root.mChildren[ i ] );
	}

	void tSkeletonVisualizer::fUpdateRenderables( const tAnimatedSkeleton::tBoneLine& root )
	{
		const Math::tVec4f linesRgba = Math::tVec4f( 0.2f, 1.0f, 0.2f, 1.f );
		const u32 vtxColor = Gfx::tVertexColor( linesRgba.x, linesRgba.y, linesRgba.z, linesRgba.w ).fForGpu( );

		Math::tMat3f m = Math::tMat3f::cIdentity;
		m.fSetTranslation( root.mXform.fGetTranslation( ) );
		mJointInstances[ mCurrentJoint ]->fMoveTo( root.mXform.fGetTranslation( ) );

		for( u32 i = 0; i < root.mChildren.fCount( ); ++i )
		{
			mLineVerts.fPushBack( Gfx::tSolidColorRenderVertex( root.mXform.fGetTranslation( ), vtxColor ) );
			mLineVerts.fPushBack( Gfx::tSolidColorRenderVertex( root.mChildren[ i ].mXform.fGetTranslation( ), vtxColor ) );
		}

		++mCurrentJoint;

		for( u32 i = 0; i < root.mChildren.fCount( ); ++i )
			fUpdateRenderables( root.mChildren[ i ] );
	}

	void tSkeletonVisualizer::fAddToScreen( const Gfx::tScreenPtr& screen )
	{
		screen->fAddWorldSpaceTopDrawCall( Gfx::tRenderableEntityPtr( mLines.fGetRawPtr( ) ) );

		for( u32 i = 0; i < mBoneProxies.fCount( ); ++i )
			screen->fAddWorldSpaceTopDrawCall( mBoneProxies[ i ] );

		for( u32 i = 0; i < mJointInstances.fCount( ); ++i )
			screen->fAddWorldSpaceTopDrawCall( mJointInstances[ i ] );
	}

}

