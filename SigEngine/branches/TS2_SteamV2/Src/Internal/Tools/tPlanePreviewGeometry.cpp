#include "ToolsPch.hpp"
#include "tPlanePreviewGeometry.hpp"
#include "Gfx/tGeometryBufferSysRam.hpp"



namespace Sig
{
	class tPlanePreviewGeometryConverter : public Gfx::tGeometryBufferSysRam::tForEachVertexElement
	{
	private:
		b32 mFacingParticleQuad;
	public:
		static const u32 cVertexCount = 4;
	private:
		tFixedArray<Math::tVec3f,cVertexCount> mP;
		tFixedArray<Math::tVec3f,cVertexCount> mT;
		tFixedArray<Math::tVec3f,cVertexCount> mB;
		tFixedArray<Math::tVec3f,cVertexCount> mN;
		tFixedArray<Math::tVec4f,cVertexCount> mUv;
	public:
		tPlanePreviewGeometryConverter( b32 facingParticleQuad );
		virtual void operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc );
	};
	tPlanePreviewGeometryConverter::tPlanePreviewGeometryConverter( b32 facingParticleQuad )
		: mFacingParticleQuad( facingParticleQuad )
	{
		if( facingParticleQuad )
		{
			mP[ 0 ] = Math::tVec3f( 0.f );
			mP[ 1 ] = Math::tVec3f( 0.f );
			mP[ 2 ] = Math::tVec3f( 0.f );
			mP[ 3 ] = Math::tVec3f( 0.f );
		}
		else
		{
			mP[ 0 ] = Math::tVec3f( -1.f, +1.f, 0.f );
			mP[ 1 ] = Math::tVec3f( -1.f, -1.f, 0.f );
			mP[ 2 ] = Math::tVec3f( +1.f, +1.f, 0.f );
			mP[ 3 ] = Math::tVec3f( +1.f, -1.f, 0.f );
		}

		const Math::tVec3f n = Math::tVec3f::cZAxis;
		const Math::tVec3f t = Math::tVec3f::cYAxis.fCross( n ).fNormalizeSafe( );
		const Math::tVec3f b = n.fCross( t ).fNormalizeSafe( );

		mT[ 0 ] = t; mT[ 1 ] = t; mT[ 2 ] = t; mT[ 3 ] = t;
		mB[ 0 ] = b; mB[ 1 ] = b; mB[ 2 ] = b; mB[ 3 ] = b;
		mN[ 0 ] = n; mN[ 1 ] = n; mN[ 2 ] = n; mN[ 3 ] = n;

		if( facingParticleQuad )
		{
			mUv[ 0 ] = Math::tVec4f( -1.f, +1.f, 0.f, 0.f );
			mUv[ 1 ] = Math::tVec4f( -1.f, -1.f, 0.f, 0.f );
			mUv[ 2 ] = Math::tVec4f( +1.f, +1.f, 0.f, 0.f );
			mUv[ 3 ] = Math::tVec4f( +1.f, -1.f, 0.f, 0.f );
		}
		else
		{
			mUv[ 0 ] = Math::tVec4f( 0.f, 0.f, 0.f, 0.f );
			mUv[ 1 ] = Math::tVec4f( 0.f, 1.f, 0.f, 0.f );
			mUv[ 2 ] = Math::tVec4f( 1.f, 0.f, 0.f, 0.f );
			mUv[ 3 ] = Math::tVec4f( 1.f, 1.f, 0.f, 0.f );
		}
	}
	void tPlanePreviewGeometryConverter::operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc )
	{
		const u32 itri = ivtx / 3;

		switch( vertexElemDesc.mSemantic )
		{
		case Gfx::tVertexElement::cSemanticPosition:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mP[ ivtx ];
			}
			break;
		case Gfx::tVertexElement::cSemanticNormal:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mN[ ivtx ];
			}
			break;
		case Gfx::tVertexElement::cSemanticTangent:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mT[ ivtx ];
			}
			break;
		case Gfx::tVertexElement::cSemanticBinormal:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mB[ ivtx ];
			}
			break;
		case Gfx::tVertexElement::cSemanticTexCoord:
			{
				if( mFacingParticleQuad )
				{
					sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_4 ); // until we support all formats, ensure it's the default
					Math::tVec4f* o = reinterpret_cast<Math::tVec4f*>( vertexElem );
					*o = mUv[ ivtx ];
				}
				else
				{
					sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_2 ); // until we support all formats, ensure it's the default
					Math::tVec2f* o = reinterpret_cast<Math::tVec2f*>( vertexElem );
					*o = Math::tVec2f( mUv[ ivtx ].x, mUv[ ivtx ].y );
				}
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


	tPlanePreviewGeometry::tPlanePreviewGeometry( b32 facingParticleQuad )
		: mFacingParticleQuad( facingParticleQuad )
	{
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );
	}
	void tPlanePreviewGeometry::fRegenerateGeometry( const Gfx::tDevicePtr& device, const Gfx::tVertexFormatVRam* vtxFormat, const Gfx::tMaterial* mtl )
	{
		if( !vtxFormat )
		{
			mVerts.fDeallocate( );
			mIndices.fDeallocate( );
			return;
		}

		Gfx::tGeometryBufferSysRam sysRamGeom;
		sysRamGeom.fAllocate( *vtxFormat, tPlanePreviewGeometryConverter::cVertexCount );
		tPlanePreviewGeometryConverter converter( mFacingParticleQuad );
		sysRamGeom.fForEachVertexElement( converter );

		mVerts.fAllocate( device, sysRamGeom.fVertexFormat( ), sysRamGeom.fVertexCount( ), 0 );
		sigassert( mVerts.fBufferSize( ) == sysRamGeom.fSizeInBytes( ) );
		Sig::byte* vertMem = mVerts.fDeepLock( );
		fMemCpy( vertMem, sysRamGeom.fBegin( ), mVerts.fBufferSize( ) );
		mVerts.fDeepUnlock( );

		mIndices.fAllocate( device, Gfx::tIndexFormat( Gfx::tIndexFormat::cStorageU16, Gfx::tIndexFormat::cPrimitiveTriangleList ), 6, 2, 0 );
		Sig::byte* idsMem = mIndices.fDeepLock( );
		u16* ids = ( u16* )idsMem;
		ids[ 0 ] = 0; ids[ 1 ] = 1; ids[ 2 ] = 2;
		ids[ 3 ] = 2; ids[ 4 ] = 1; ids[ 5 ] = 3;
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
		tRenderableEntity::fSetObjectSpaceBox( Math::tAabbf( Math::tVec3f( -1.f, -1.f, -0.1f ), Math::tVec3f( +1.f, +1.f, 0.1f ) ) );
	}
}
