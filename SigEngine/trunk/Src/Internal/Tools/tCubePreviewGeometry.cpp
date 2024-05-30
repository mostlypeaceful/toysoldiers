#include "ToolsPch.hpp"
#include "tCubePreviewGeometry.hpp"
#include "Gfx/tGeometryBufferSysRam.hpp"
#include "Gfx/tDefaultAllocators.hpp"

using namespace Sig::Math;

namespace Sig
{
	class tCubePreviewGeometryConverter : public Gfx::tGeometryBufferSysRam::tForEachVertexElement
	{
	public:
		static const u32 cVertexCount = 24;
		enum tFaces
		{
			cXPlus,
			cXMinus,
			cYPlus,
			cYMinus,
			cZPlus,
			cZMinus,
			cFaceCount
		};

	private:
		tFixedArray<tVec3f,cVertexCount> mP;
		tFixedArray<tVec3f,cVertexCount> mT;
		tFixedArray<tVec3f,cVertexCount> mB;
		tFixedArray<tVec3f,cVertexCount> mN;
		tFixedArray<tVec4f,cVertexCount> mUv;
	public:
		tCubePreviewGeometryConverter( const tMat3f& formVertices, b32 cubeMap );
		virtual void operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc );
	};

	tCubePreviewGeometryConverter::tCubePreviewGeometryConverter( const tMat3f& xformVertices, b32 cubeMap )
	{
		// template face is layed out in xy plane. IE. normal in the Z direction.
		tFixedArray<tMat3f, cFaceCount> xForms;

		const f32 cShift = 1.f;
		xForms[ cXPlus ] = tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, cPiOver2 ) ), tVec3f( +cShift, 0, 0 ) );
		xForms[ cXMinus ] = tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, -cPiOver2 ) ), tVec3f( -cShift, 0, 0 ) );

		xForms[ cYPlus ] = tMat3f( tQuatf( tAxisAnglef( tVec3f::cXAxis, -cPiOver2 ) ), tVec3f( 0, +cShift, 0 ) );
		xForms[ cYMinus ] = tMat3f( tQuatf( tAxisAnglef( tVec3f::cXAxis, cPiOver2 ) ), tVec3f( 0, -cShift, 0 ) );

		xForms[ cZPlus ] = tMat3f( tQuatf( tAxisAnglef( tVec3f::cXAxis, 0.0f ) ), tVec3f( 0, 0, +cShift ) );
		xForms[ cZMinus ] = tMat3f( tQuatf( tAxisAnglef( tVec3f::cXAxis, cPi ) ), tVec3f( 0, 0, -cShift ) );

		for( u32 i = 0; i < cFaceCount; ++i )
		{
			u32 start = 4 * i;

			tMat3f xform = xformVertices * xForms[ i ];
			{
				mP[ start + 0 ] = xform.fXformPoint( tVec3f( -1.f, +1.f, 0.f ) );
				mP[ start + 1 ] = xform.fXformPoint( tVec3f( -1.f, -1.f, 0.f ) );
				mP[ start + 2 ] = xform.fXformPoint( tVec3f( +1.f, +1.f, 0.f ) );
				mP[ start + 3 ] = xform.fXformPoint( tVec3f( +1.f, -1.f, 0.f ) );
			}

			tVec3f n = tVec3f::cZAxis;
			tVec3f t = tVec3f::cYAxis.fCross( n ).fNormalizeSafe( );
			tVec3f b = n.fCross( t ).fNormalizeSafe( );

			{
				n = xform.fXformVector( n );
				t = xform.fXformVector( t );
				b = xform.fXformVector( b );
			}

			mT[ start + 0 ] = t; mT[ start + 1 ] = t; mT[ start + 2 ] = t; mT[ start + 3 ] = t;
			mB[ start + 0 ] = b; mB[ start + 1 ] = b; mB[ start + 2 ] = b; mB[ start + 3 ] = b;
			mN[ start + 0 ] = n; mN[ start + 1 ] = n; mN[ start + 2 ] = n; mN[ start + 3 ] = n;

			if( cubeMap )
			{
				const f32 cThird = 0.3333f;
				const f32 cForth = 0.25f;

				switch( i )
				{
				case cXMinus:
					{
						mUv[ start + 0 ] = tVec4f( 1*cForth, 2*cThird, 0.f, 0.f );
						mUv[ start + 1 ] = tVec4f( 1*cForth, cThird, 0.f, 0.f );
						mUv[ start + 2 ] = tVec4f( 0*cForth, 2*cThird, 0.f, 0.f );
						mUv[ start + 3 ] = tVec4f( 0*cForth, cThird, 0.f, 0.f );
					}
					break;
				case cZMinus:
					{
						mUv[ start + 0 ] = tVec4f( 1*cForth, cThird, 0.f, 0.f );
						mUv[ start + 1 ] = tVec4f( 1*cForth, 2*cThird, 0.f, 0.f );
						mUv[ start + 2 ] = tVec4f( 2*cForth, cThird, 0.f, 0.f );
						mUv[ start + 3 ] = tVec4f( 2*cForth, 2*cThird, 0.f, 0.f );
					}
					break;
				case cXPlus:
					{
						mUv[ start + 0 ] = tVec4f( 3*cForth, 2*cThird, 0.f, 0.f );
						mUv[ start + 1 ] = tVec4f( 3*cForth, cThird, 0.f, 0.f );
						mUv[ start + 2 ] = tVec4f( 2*cForth, 2*cThird, 0.f, 0.f );
						mUv[ start + 3 ] = tVec4f( 2*cForth, cThird, 0.f, 0.f );
					}
					break;
				case cZPlus:
					{
						mUv[ start + 0 ] = tVec4f( 4*cForth, 2*cThird, 0.f, 0.f );
						mUv[ start + 1 ] = tVec4f( 4*cForth, cThird, 0.f, 0.f );
						mUv[ start + 2 ] = tVec4f( 3*cForth, 2*cThird, 0.f, 0.f );
						mUv[ start + 3 ] = tVec4f( 3*cForth, cThird, 0.f, 0.f );
					}
					break;
				case cYMinus:
					{
						mUv[ start + 0 ] = tVec4f( 2*cForth, 1*cThird, 0.f, 0.f );
						mUv[ start + 1 ] = tVec4f( 2*cForth, 0*cThird, 0.f, 0.f );
						mUv[ start + 2 ] = tVec4f( 1*cForth, 1*cThird, 0.f, 0.f );
						mUv[ start + 3 ] = tVec4f( 1*cForth, 0*cThird, 0.f, 0.f );
					}
					break;
				case cYPlus:
					{
						mUv[ start + 0 ] = tVec4f( 2*cForth, 3*cThird, 0.f, 0.f );
						mUv[ start + 1 ] = tVec4f( 2*cForth, 2*cThird, 0.f, 0.f );
						mUv[ start + 2 ] = tVec4f( 1*cForth, 3*cThird, 0.f, 0.f );
						mUv[ start + 3 ] = tVec4f( 1*cForth, 2*cThird, 0.f, 0.f );
					}
					break;
				}
			}
			else
			{
				mUv[ start + 0 ] = tVec4f( 0.f, 0.f, 0.f, 0.f );
				mUv[ start + 1 ] = tVec4f( 0.f, 1.f, 0.f, 0.f );
				mUv[ start + 2 ] = tVec4f( 1.f, 0.f, 0.f, 0.f );
				mUv[ start + 3 ] = tVec4f( 1.f, 1.f, 0.f, 0.f );
			}
		}
	}
	void tCubePreviewGeometryConverter::operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc )
	{
		const u32 itri = ivtx / 3;

		switch( vertexElemDesc.mSemantic )
		{
		case Gfx::tVertexElement::cSemanticPosition:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				tVec3f* o = reinterpret_cast<tVec3f*>( vertexElem );
				*o = mP[ ivtx ];
			}
			break;
		case Gfx::tVertexElement::cSemanticNormal:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				tVec3f* o = reinterpret_cast<tVec3f*>( vertexElem );
				*o = mN[ ivtx ];
			}
			break;
		case Gfx::tVertexElement::cSemanticTangent:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				tVec3f* o = reinterpret_cast<tVec3f*>( vertexElem );
				*o = mT[ ivtx ];
			}
			break;
		case Gfx::tVertexElement::cSemanticBinormal:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default
				tVec3f* o = reinterpret_cast<tVec3f*>( vertexElem );
				*o = mB[ ivtx ];
			}
			break;
		case Gfx::tVertexElement::cSemanticTexCoord:
			{
				{
					sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_2 ); // until we support all formats, ensure it's the default
					tVec2f* o = reinterpret_cast<tVec2f*>( vertexElem );
					*o = tVec2f( mUv[ ivtx ].x, mUv[ ivtx ].y );
				}
			}
			break;
		case Gfx::tVertexElement::cSemanticColor:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_u8_4_Color ); // until we support all formats, ensure it's the default

				u8* o = reinterpret_cast<u8*>( vertexElem );

				const tVec4f rgba( 1.f );

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


	tCubePreviewGeometry::tCubePreviewGeometry( const tMat3f& xformVertices, b32 cubeMap )
		: mXformVertices( xformVertices )
		, mCubeMap( cubeMap )
	{
		mRenderState = Gfx::tRenderState::cDefaultColorOpaque;
		//mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );
	}
	void tCubePreviewGeometry::fRegenerateGeometry( const Gfx::tDevicePtr& device, const Gfx::tVertexFormatVRam* vtxFormat, const Gfx::tMaterial* mtl )
	{
		if( !vtxFormat )
		{
			mVerts.fDeallocate( );
			mIndices.fDeallocate( );
			return;
		}

		Gfx::tGeometryBufferSysRam sysRamGeom;
		sysRamGeom.fAllocate( *vtxFormat, tCubePreviewGeometryConverter::cVertexCount );
		tCubePreviewGeometryConverter converter( mXformVertices, mCubeMap );
		sysRamGeom.fForEachVertexElement( converter );
		
		mVerts.fAllocate( device, sysRamGeom.fVertexFormat( ), sysRamGeom.fVertexCount( ), 0 );
		sigassert( mVerts.fBufferSize( ) == sysRamGeom.fSizeInBytes( ) );
		Sig::byte* vertMem = mVerts.fDeepLock( );
		fMemCpy( vertMem, sysRamGeom.fBegin( ), mVerts.fBufferSize( ) );
		mVerts.fDeepUnlock( );

		mIndices.fAllocate( device, Gfx::tIndexFormat( Gfx::tIndexFormat::cStorageU16, Gfx::tIndexFormat::cPrimitiveTriangleList ), 6 * tCubePreviewGeometryConverter::cFaceCount, 2 * tCubePreviewGeometryConverter::cFaceCount, 0 );
		Sig::byte* idsMem = mIndices.fDeepLock( );
		u16* ids = ( u16* )idsMem;
		for( u32 i = 0; i < tCubePreviewGeometryConverter::cFaceCount; ++i )
		{
			u32 iStart = 6 * i;
			u32 vStart = 4 * i;
			ids[ iStart + 0 ] = vStart + 0; ids[ iStart + 1 ] = vStart + 1; ids[ iStart + 2 ] = vStart + 2;
			ids[ iStart + 3 ] = vStart + 2; ids[ iStart + 4 ] = vStart + 1; ids[ iStart + 5 ] = vStart + 3;
		}
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
		tRenderableEntity::fSetObjectSpaceBox( tAabbf( tVec3f( -1.f, -1.f, -0.1f ), tVec3f( +1.f, +1.f, 0.1f ) ) );
	}
}
