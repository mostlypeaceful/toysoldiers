#include "BasePch.hpp"
#include "tDebugGeometry.hpp"

#ifdef sig_devmenu

#include "tResourceDepot.hpp"
#include "tViewport.hpp"
#include "tRenderableEntity.hpp"
#include "tSolidColorBox.hpp"
#include "tSolidColorSphere.hpp"
#include "tSolidColorLines.hpp"
#include "tMaterialFile.hpp"
#include "tSolidColorMaterial.hpp"
#include "Threads/tMutex.hpp"
#include "Math/tIntersectionAabbFrustum.hpp"
#include "Math/tConvexHull.hpp"

namespace Sig { namespace Gfx
{
	define_smart_ptr( base_export, tRefCounterPtr, tDebugGeometry );

	class base_export tDebugGeometry : public tRenderableEntity
	{
		define_dynamic_cast( tDebugGeometry, tRenderableEntity );
	public:
		Math::tAabbf mWorldSpaceBox;
		virtual ~tDebugGeometry( )
		{
		}
	};

	class base_export tDebugSphere : public tDebugGeometry
	{
		define_dynamic_cast( tDebugSphere, tDebugGeometry );
	public:
		tDebugSphere( const Math::tSpheref& shape, const Math::tMat3f& xform = Math::tMat3f::cIdentity )
		{
			fMoveTo( shape.fAdjustMatrix( xform, 0.f ) );
			mWorldSpaceBox = Math::tAabbf( Math::tSpheref( xform.fXformPoint( shape.mCenter ), xform.fXformVector( Math::tVec3f(shape.mRadius) ).fMaxMagnitude( ) ) );
		}
	};

	class base_export tDebugBox : public tDebugGeometry
	{
		define_dynamic_cast( tDebugBox, tDebugGeometry );
	public:
		tDebugBox( const Math::tAabbf& shape )
		{
			fMoveTo( shape.fAdjustMatrix( Math::tMat3f::cIdentity, 0.f ) );
			mWorldSpaceBox = shape;
		}
		tDebugBox( const Math::tObbf& shape )
		{
			fMoveTo( shape.fAdjustMatrix( Math::tMat3f::cIdentity, 0.f ) );
			mWorldSpaceBox = Math::tAabbf( shape );
		}
	};

	class base_export tDebugCylinder : public tDebugGeometry
	{
		define_dynamic_cast( tDebugCylinder, tDebugGeometry );
	public:
		tDebugCylinder( const Math::tCylinder& shape )
		{
			Math::tMat3f xform = shape.fGetTransform( );
			xform.fScaleLocal( Math::tVec3f( shape.mRadius, shape.mHalfHeight, shape.mRadius ) );
			fMoveTo( xform );
			mWorldSpaceBox = shape.fToAABB( );
		}
	};

	class base_export tDebugCapsule : public tDebugGeometry
	{
		define_dynamic_cast( tDebugCapsule, tDebugGeometry );
	public:
		tDebugCapsule( const Math::tCapsule& shape, const tSolidColorSphere& templateSphere )
		{
			mCapsuleGeometry.fResetDeviceObjects( templateSphere );
			mCapsuleGeometry.fGenerate( shape.mRadius, shape.mHalfHeight );

			Math::tMat3f xform = shape.fGetTransform( );
			fMoveTo( xform );
			mWorldSpaceBox = shape.fToAABB( );
		}

		Gfx::tSolidColorSphere		mCapsuleGeometry;
	};

	struct base_export tDebugGeometryContainerImpl
	{
		tResourcePtr							mSolidColorMaterialFile;
		tMaterialPtr							mSolidColorMaterial;
		tGeometryBufferVRamAllocatorPtr			mSolidColorGeometry;
		tIndexBufferVRamAllocatorPtr			mSolidColorIndices;
		tRenderState							mRenderStateOverride;
		tSolidColorBox							mBoxTemplate;
		tSolidColorSphere						mSphereTemplate;
		tSolidColorCylinder						mCylinderTemplate;
		tSolidColorLines						mLines;
		tGrowableArray< tSolidColorRenderVertex > mLineVerts;
		tGrowableArray< tDebugGeometryPtr >		mDebugGeomObjects;
		Threads::tCriticalSection				mCritSec;
	};


	tDebugGeometryContainer::tDebugGeometryContainer( )
		: mImpl( NEW tDebugGeometryContainerImpl )
	{
		mImpl->mRenderStateOverride = tRenderState::cDefaultColorTransparent;
	}

	tDebugGeometryContainer::~tDebugGeometryContainer( )
	{
		if( mImpl->mSolidColorGeometry )
			mImpl->mSolidColorGeometry->fDeallocate( );
		if( mImpl->mSolidColorIndices )
			mImpl->mSolidColorIndices->fDeallocate( );
		delete mImpl;
	}

	void tDebugGeometryContainer::fResetDeviceObjects( tResourceDepot& resourceDepot, const tDevicePtr& device )
	{
		// load solid color material
		mImpl->mSolidColorMaterialFile = resourceDepot.fQueryLoadBlock( tResourceId::fMake<tMaterialFile>( tSolidColorMaterial::fMaterialFilePath( ) ), this );
		mImpl->mSolidColorMaterial.fReset( NEW tSolidColorMaterial( ) );
		mImpl->mSolidColorMaterial->fSetMaterialFileResourcePtrOwned( mImpl->mSolidColorMaterialFile );

		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );

		// create geometry allocators
		const u32 numVerts = 1024 * 6;
		mImpl->mSolidColorGeometry.fReset( NEW tGeometryBufferVRamAllocator );
		mImpl->mSolidColorGeometry->fAllocate( device, tSolidColorMaterial::cVertexFormat, numVerts, tGeometryBufferVRam::cAllocDynamic );

		// create index allocators
		const tIndexFormat indexFormat( tIndexFormat::cStorageU16, tIndexFormat::cPrimitiveTriangleList );
		const u32 numTris = 512 * 1024;
		const u32 numIds = numTris * 3;
		mImpl->mSolidColorIndices.fReset( NEW tIndexBufferVRamAllocator );
		mImpl->mSolidColorIndices->fAllocate( device, indexFormat, numIds, numTris, tGeometryBufferVRam::cAllocDynamic );

		Memory::tHeap::fResetVramContext( );

		// create geometry template objects
		mImpl->mBoxTemplate.fSetRenderStateOverride( &mImpl->mRenderStateOverride );
		mImpl->mBoxTemplate.fResetDeviceObjects( device, mImpl->mSolidColorMaterial, mImpl->mSolidColorGeometry, mImpl->mSolidColorIndices );
		mImpl->mBoxTemplate.fGenerate( 1.f );
		mImpl->mSphereTemplate.fSetRenderStateOverride( &mImpl->mRenderStateOverride );
		mImpl->mSphereTemplate.fResetDeviceObjects( device, mImpl->mSolidColorMaterial, mImpl->mSolidColorGeometry, mImpl->mSolidColorIndices );
		mImpl->mSphereTemplate.fGenerate( 1.f );
		mImpl->mCylinderTemplate.fSetRenderStateOverride( &mImpl->mRenderStateOverride );
		mImpl->mCylinderTemplate.fResetDeviceObjects( device, mImpl->mSolidColorMaterial, mImpl->mSolidColorGeometry, mImpl->mSolidColorIndices );
		mImpl->mCylinderTemplate.fGenerate( 1.f, 1.f );
		mImpl->mLines.fSetRenderStateOverride( &mImpl->mRenderStateOverride );
		mImpl->mLines.fResetDeviceObjects( device, mImpl->mSolidColorMaterial, mImpl->mSolidColorGeometry, mImpl->mSolidColorIndices );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tTrianglef& tri, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		const u32 vtxColor = tVertexColor( rgba.x, rgba.y, rgba.z, rgba.w ).fForGpu( );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( tri.mA, vtxColor ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( tri.mB, vtxColor ) );

		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( tri.mB, vtxColor ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( tri.mC, vtxColor ) );

		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( tri.mC, vtxColor ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( tri.mA, vtxColor ) );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tCylinder& circle, u32 numSegments, const Math::tVec4f& rgba )
	{
		// create a matrix that is oriented with it's up the input axis
		Math::tMat3f m;		
		f32 len = 0.0f;
		Math::tVec3f axis = circle.mPrimaryAxis;
		axis.fNormalize( len );
		m.fOrientYAxis( axis );
		
		// clamp the segments and calc an angle delta
		numSegments = fMax( numSegments, (u32)12 ); // less than 12 looks too chunky
		f32 delta = Math::c2Pi / numSegments;
		f32 angle = 0.0f;

		// calc the initial position
		Math::tVec3f p0 = m.fXAxis( ) * Math::fCos( angle ) * circle.mRadius
						+ m.fZAxis( ) * Math::fSin( angle ) * circle.mRadius
						+ circle.mCenter;

		for( u32 i = 0; i <= numSegments; ++i, angle += delta )
		{
			// calc the next position
			Math::tVec3f p1 = m.fXAxis( ) * Math::fCos( angle ) * circle.mRadius
							+ m.fZAxis( ) * Math::fSin( angle ) * circle.mRadius
							+ circle.mCenter;

			// render the segment and iterate the point
			fRenderOnce( p0, p1, rgba );
			p0 = p1;
		}
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tPlanef& plane, const Math::tVec3f& center, f32 scale, const Math::tVec4f& rgba )
	{
		const Math::tVec3f normal = plane.fGetNormalUnit( );
		const Math::tVec3f reference = fEqual( normal.fDot( Math::tVec3f::cYAxis ), 1.f ) ? Math::tVec3f::cZAxis : Math::tVec3f::cYAxis;

		const Math::tVec3f left = normal.fCross( reference ).fNormalize( ) * scale;
		const Math::tVec3f top = normal.fCross( left ).fNormalize( ) * scale;

		const Math::tVec3f topLeft = center + left + top;
		const Math::tVec3f topRight = center - left + top;
		const Math::tVec3f botLeft = center + left - top;
		const Math::tVec3f botRight = center - left - top;

		//render the plane
		fRenderOnce( Math::tTrianglef( botLeft, topLeft, topRight ), rgba );
		fRenderOnce( Math::tTrianglef( topRight, botRight, botLeft ), rgba );

		//and render the normal
		fRenderOnce( center, center + normal * scale / 2, rgba );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tRayf& ray, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		const u32 vtxColor = tVertexColor( rgba.x, rgba.y, rgba.z, rgba.w ).fForGpu( );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( ray.mOrigin, vtxColor ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( ray.mOrigin + ray.mExtent, vtxColor ) );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tAabbf& shape, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		fAddObject( 
			tDebugGeometryPtr( NEW tDebugBox( shape ) ),
			rgba,
			mImpl->mBoxTemplate.fGetRenderBatch( ) );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tObbf& shape, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		fAddObject( 
			tDebugGeometryPtr( NEW tDebugBox( shape ) ),
			rgba,
			mImpl->mBoxTemplate.fGetRenderBatch( ) );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tSpheref& shape, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		fAddObject( 
			tDebugGeometryPtr( NEW tDebugSphere( shape ) ),
			rgba,
			mImpl->mSphereTemplate.fGetRenderBatch( ) );		
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tSpheref& shape, const Math::tMat3f& xform, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		fAddObject( 
			tDebugGeometryPtr( NEW tDebugSphere( shape, xform ) ),
			rgba,
			mImpl->mSphereTemplate.fGetRenderBatch( ) );		
	}

	void tDebugGeometryContainer::fRenderOnce( const tPair<Math::tVec3f,Math::tVec3f>& line, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		const u32 vtxColor = tVertexColor( rgba.x, rgba.y, rgba.z, rgba.w ).fForGpu( );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( line.mA, vtxColor ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( line.mB, vtxColor ) );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tVec3f& linePtA, const Math::tVec3f& linePtB, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		const u32 vtxColor = tVertexColor( rgba.x, rgba.y, rgba.z, rgba.w ).fForGpu( );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( linePtA, vtxColor ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( linePtB, vtxColor ) );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tMat3f& basis, f32 length, f32 alpha )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		const u32 vtxColorRed = tVertexColor( 1.0f, 0.0f, 0.0f, alpha ).fForGpu( );
		const u32 vtxColorGreen = tVertexColor( 0.0f, 1.0f, 0.0f, alpha ).fForGpu( );
		const u32 vtxColorBlue = tVertexColor( 0.0f, 0.0f, 1.0f, alpha ).fForGpu( );

		const Math::tVec3f origin = basis.fGetTranslation( );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( origin, vtxColorRed ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( origin + basis.fXAxis( ) * length, vtxColorRed ) );

		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( origin, vtxColorGreen ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( origin + basis.fYAxis( ) * length, vtxColorGreen ) );

		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( origin, vtxColorBlue ) );
		mImpl->mLineVerts.fPushBack( tSolidColorRenderVertex( origin + basis.fZAxis( ) * length, vtxColorBlue ) );
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tConvexHull& hull, const Math::tMat3f& xform, const Math::tVec4f& rgba )
	{
		for( u32 i = 0; i < hull.fFaces( ).fCount( ); ++i )
		{
			Math::tTrianglef tri = hull.fMakeTriangle( i );
			tri = tri.fTransform( xform );
			fRenderOnce( tri, rgba );

			Math::tVec3f center = tri.fComputeCenter( );

			fRenderOnce( center, center + tri.fComputeUnitNormal( ), Math::tVec4f( 0,1,0,0.5f ) );
		}
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tCylinder& shape, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		fAddObject( 
			tDebugGeometryPtr( NEW tDebugCylinder( shape ) ),
			rgba,
			mImpl->mCylinderTemplate.fGetRenderBatch( ) );		
	}

	void tDebugGeometryContainer::fRenderOnce( const Math::tCapsule& shape, const Math::tVec4f& rgba )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		tDebugCapsule* caps = NEW tDebugCapsule( shape, mImpl->mSphereTemplate );
		fAddObject( 
			tDebugGeometryPtr( caps ),
			rgba,
			caps->mCapsuleGeometry.fGetRenderBatch( ) );		
	}

	void tDebugGeometryContainer::fAddToDisplayList( const tViewport& viewport, tWorldSpaceDisplayList& displayList )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		if( mImpl->mLineVerts.fCount( ) > 0 )
		{
			mImpl->mSolidColorGeometry->fDeepLock( );
			mImpl->mSolidColorIndices->fDeepLock( );

			mImpl->mLines.fBake( ( const Sig::byte* )mImpl->mLineVerts.fBegin( ), mImpl->mLineVerts.fCount( ), false );
			mImpl->mLines.fAddToDisplayList( viewport, displayList );

			mImpl->mSolidColorGeometry->fDeepUnlock( );
			mImpl->mSolidColorIndices->fDeepUnlock( );
		}

		const Math::tFrustumf& frustum = viewport.fRenderCamera( ).fGetWorldSpaceFrustum( );

		for( u32 i = 0; i < mImpl->mDebugGeomObjects.fCount( ); ++i )
		{
			tDebugGeometry& dg = *mImpl->mDebugGeomObjects[ i ];
			if( Math::tIntersectionAabbFrustum<f32>( dg.mWorldSpaceBox, frustum ).fIntersects( ) )
				dg.fAddToDisplayList( viewport, displayList );
		}
	}

	void tDebugGeometryContainer::fClear( )
	{
		Threads::tMutex lock( mImpl->mCritSec );

		mImpl->mDebugGeomObjects.fSetCount( 0 );
		mImpl->mLineVerts.fSetCount( 0 );
	}

	void tDebugGeometryContainer::fAddObject( const tDebugGeometryPtr& object, const Math::tVec4f& rgba, const tRenderBatchPtr& batch )
	{
		object->fSetRenderBatch( batch );
		object->fSetRgbaTint( rgba );
		mImpl->mDebugGeomObjects.fPushBack( object );
	}

}}

#else

// Fixes no object linker warning
void tDebugGeometryCPP_NoObjFix( ) { }

#endif//sig_devmenu
