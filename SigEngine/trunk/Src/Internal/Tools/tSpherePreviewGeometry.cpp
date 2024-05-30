#include "ToolsPch.hpp"
#include "tSpherePreviewGeometry.hpp"
#include "Gfx/tGeometryBufferSysRam.hpp"
#include "Math/tIntersectionRaySphere.hpp"

namespace Sig
{
	class tSpherePreviewGeometryConverter : public Gfx::tGeometryBufferSysRam::tForEachVertexElement
	{
	public:
		struct tVertex
		{
			Math::tVec3f mP;
			Math::tVec3f mT;
			Math::tVec3f mB;
			Math::tVec3f mN;
			Math::tVec2f mUv;
		};
	private:
		tGrowableArray< tVertex > mVerts;
		tGrowableArray< u16 > mIds;
	public:
		tSpherePreviewGeometryConverter( );
		const tGrowableArray< tVertex >& fVerts( ) const { return mVerts; }
		const tGrowableArray< u16 >& fIds( ) const { return mIds; }
		virtual void operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc );
	};
	tSpherePreviewGeometryConverter::tSpherePreviewGeometryConverter( )
	{
		const u32 numVertSegs = 24;
		const u32 numHorzSegs = 32;
		const f32 radius = 1.f;

		const u32 numVertsPerRow = numHorzSegs+1;

		f32 phi = 0.f;
		const f32 dPhi = Math::cPi / ( numVertSegs - 1.f );

		for( u32 v = 0; v < numVertSegs; ++v )
		{
			f32 theta = 0.f;
			const f32 dTheta = Math::c2Pi / ( numHorzSegs - 0.f );

			const f32 yCoord = std::cosf( phi );
			const f32 innerRadius = std::sinf( phi );

			for( u32 h = 0; h <= numHorzSegs; ++h )
			{
				const f32 xCoord = innerRadius * std::cosf( theta );
				const f32 zCoord = innerRadius * std::sinf( theta );

				const Math::tVec3f n( xCoord, yCoord, zCoord );
				const Math::tVec3f p = n * radius;

				tVertex vtx;
				vtx.mP = p;
				vtx.mT = Math::tVec3f::cYAxis.fCross( n ).fNormalizeSafe( );
				vtx.mB = n.fCross( vtx.mT ).fNormalizeSafe( );
				vtx.mN = n;
				vtx.mUv = Math::tVec2f( 1.f - theta / Math::c2Pi, phi / Math::cPi );
				mVerts.fPushBack( vtx );

				if( h < numHorzSegs && v < numVertSegs - 1 )
				{
					mIds.fPushBack( ( v + 0 ) * numVertsPerRow + h + 0 );
					mIds.fPushBack( ( v + 0 ) * numVertsPerRow + h + 1 );
					mIds.fPushBack( ( v + 1 ) * numVertsPerRow + h + 1 );

					mIds.fPushBack( ( v + 1 ) * numVertsPerRow + h + 1 );
					mIds.fPushBack( ( v + 1 ) * numVertsPerRow + h + 0 );
					mIds.fPushBack( ( v + 0 ) * numVertsPerRow + h + 0 );
				}

				theta += dTheta;
			}

			sigassert( fEqual( theta, Math::c2Pi + dTheta ) );

			phi += dPhi;
		}

		sigassert( fEqual( phi, Math::cPi + dPhi ) );
	}
	void tSpherePreviewGeometryConverter::operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc )
	{
		const u32 itri = ivtx / 3;

		switch( vertexElemDesc.mSemantic )
		{
		case Gfx::tVertexElement::cSemanticPosition:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mVerts[ ivtx ].mP;
			}
			break;
		case Gfx::tVertexElement::cSemanticNormal:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mVerts[ ivtx ].mN;
			}
			break;
		case Gfx::tVertexElement::cSemanticTangent:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mVerts[ ivtx ].mT;
			}
			break;
		case Gfx::tVertexElement::cSemanticBinormal:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mVerts[ ivtx ].mB;
			}
			break;
		case Gfx::tVertexElement::cSemanticTexCoord:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_2 ); // until we support all formats, ensure it's the default
				Math::tVec2f* o = reinterpret_cast<Math::tVec2f*>( vertexElem );
				*o = Math::tVec2f( mVerts[ ivtx ].mUv.x, mVerts[ ivtx ].mUv.y );
			}
			break;
		case Gfx::tVertexElement::cSemanticColor:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_u8_4_Color ); // until we support all formats, ensure it's the default

				u8* o = reinterpret_cast<u8*>( vertexElem );

				const Math::tVec4f rgba( 1.f );

				// our default color ordering is b-g-r-a; this is native Pc Dx9
				o[0] = ( u8 )( rgba.z * 255.f );
				o[1] = ( u8 )( rgba.y * 255.f );
				o[2] = ( u8 )( rgba.x * 255.f );
				o[3] = ( u8 )( rgba.w * 255.f );
			}
			break;
		case Gfx::tVertexElement::cSemanticBoneWeights:
		case Gfx::tVertexElement::cSemanticBoneIndices:
		default:
			sigassert( !"invalid vertex semantic" );
			break;
		}
	}
	tSpherePreviewGeometry::tSpherePreviewGeometry( b32 xparentBatch )
	{
		mRenderState = xparentBatch ? Gfx::tRenderState::cDefaultColorTransparent : Gfx::tRenderState::cDefaultColorOpaque;
	}
	void tSpherePreviewGeometry::fRegenerateGeometry( const Gfx::tDevicePtr& device, const Gfx::tVertexFormatVRam* vtxFormat, const Gfx::tMaterial* mtl )
	{
		if( !vtxFormat )
		{
			mVerts.fDeallocate( );
			mIndices.fDeallocate( );
			return;
		}

		Gfx::tGeometryBufferSysRam sysRamGeom;
		tSpherePreviewGeometryConverter converter;
		sysRamGeom.fAllocate( *vtxFormat, converter.fVerts( ).fCount( ) );
		sysRamGeom.fForEachVertexElement( converter );

		mVerts.fAllocate( device, sysRamGeom.fVertexFormat( ), sysRamGeom.fVertexCount( ), 0 );
		sigassert( mVerts.fBufferSize( ) == sysRamGeom.fSizeInBytes( ) );
		Sig::byte* vertMem = mVerts.fDeepLock( );
		fMemCpy( vertMem, sysRamGeom.fBegin( ), mVerts.fBufferSize( ) );
		mVerts.fDeepUnlock( );

		const tGrowableArray<u16>& ids = converter.fIds( );
		mIndices.fAllocate( device, Gfx::tIndexFormat( Gfx::tIndexFormat::cStorageU16, Gfx::tIndexFormat::cPrimitiveTriangleList ), ids.fCount( ), ids.fCount( )/3, 0 );
		Sig::byte* idsMem = mIndices.fDeepLock( );
		fMemCpy( idsMem, ids.fBegin( ), ids.fCount( ) * sizeof( ids[ 0 ] ) );
		mIndices.fDeepUnlock( );

		Gfx::tRenderBatchData batch;
		batch.mRenderState = &mRenderState;
		batch.mMaterial = mtl;
		batch.mVertexFormat = vtxFormat;
		batch.mGeometryBuffer = &mVerts;
		batch.mIndexBuffer = &mIndices;
		batch.mVertexCount = mVerts.fVertexCount( );
		batch.mBaseVertexIndex = 0;
		batch.mPrimitiveCount = mIndices.fPrimitiveCount( );
		batch.mBaseIndexIndex = 0;
		batch.mPrimitiveType = mIndices.fIndexFormat( ).mPrimitiveType;

		tRenderableEntity::fSetRenderBatch( Gfx::tRenderBatch::fCreate( batch ) );
		tRenderableEntity::fSetObjectSpaceBox( Math::tAabbf( Math::tVec3f( -1.f, -1.f, -1.f ), Math::tVec3f( 1.f, 1.f, 1.f ) ) );
	}

	void tSpherePreviewGeometry::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		if( mEnableRayCast )
		{
			Math::tSpheref sphere( fObjectToWorld( ).fGetTranslation( ), fObjectToWorld( ).fGetScale( ).x );

			Math::tIntersectionRaySphere<f32> test( ray, sphere );
			if( test.fIntersects( ) )
			{
				hit.mT = test.fT( );
				hit.mN = ( ray.fPointAtTime( hit.mT ) - sphere.fCenter( ) ).fNormalizeSafe( Math::tVec3f::cYAxis );
			}
		}
	}
}
