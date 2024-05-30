//------------------------------------------------------------------------------
// \file tSolidColorQuads.cpp - 07 Dec 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tSolidColorQuads.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig { namespace Gfx
{
	tSolidColorQuads::tSolidColorQuads( )
	{
		fSetPrimTypeOverride( tIndexFormat::cPrimitiveTriangleList );
	}

	void tSolidColorQuads::fOnDeviceReset( tDevice* device )
	{
		fSubmitGeometry( );
	}

	void tSolidColorQuads::fGenerate( f32 halfEdgeLen )
	{
		const Math::tVec3f sx = halfEdgeLen * Math::tVec3f::cXAxis;
		const Math::tVec3f sz = halfEdgeLen * Math::tVec3f::cZAxis;
		fAddQuad( sx + sz, sx - sz, -sx - sz, -sx + sz );

		fBake( mVerts, mIds, mIds.fCount( ) / 3 );
	}

	Math::tAabbf tSolidColorQuads::fGetBounds( ) const
	{
		Math::tVec3f max( std::numeric_limits<f32>::min( ) );
		Math::tVec3f min( std::numeric_limits<f32>::max( ) );

		for( u32 i = 0; i < mVerts.fCount( ); ++i )
		{
			const Math::tVec3f& vertPos = mVerts[i].mP;

			max.x = fMax( max.x, vertPos.x );
			max.y = fMax( max.y, vertPos.y );
			max.z = fMax( max.z, vertPos.z );

			min.x = fMin( min.x, vertPos.x );
			min.y = fMin( min.y, vertPos.y );
			min.z = fMin( min.z, vertPos.z );
		}

		return Math::tAabbf( min, max );
	}

	void tSolidColorQuads::fSetQuadCount( u32 count )
	{
		mVerts.fSetCount( count * 4 ); // 4 verts  per quad
		mIds.fSetCount( count * 3 * 2 ); // 3 ids per tri, 2 tris per quad
	}

	u32 tSolidColorQuads::fQuadCount( )
	{
		return mVerts.fCount( ) / 4;
	}

	tSolidColorRenderVertex* tSolidColorQuads::fQuad( u32 index )
	{
		return &mVerts[ index * 4 ];
	}

	void tSolidColorQuads::fAddQuad( const Math::tVec3f& v0, const Math::tVec3f& v1, const Math::tVec3f& v2, const Math::tVec3f& v3 )
	{
		const Math::tVec3f n = ( v1 - v0 ).fCross( v2 - v0 ).fNormalizeSafe( Math::tVec3f::cZAxis );
		const f32 shade = fClamp( 0.5f + fMax( 0.f, n.fDot( Math::tVec3f( 1.0f, 0.5f, 2.0f ).fNormalizeSafe( Math::tVec3f::cZAxis ) ) ), 0.f, 1.f );
		const u32 vtxColor = Gfx::tVertexColor( shade, shade, shade ).fForGpu( );

		const u32 baseVertex = mVerts.fCount( );

		mVerts.fPushBack( Gfx::tSolidColorRenderVertex( v0, vtxColor ) );
		mVerts.fPushBack( Gfx::tSolidColorRenderVertex( v1, vtxColor ) );
		mVerts.fPushBack( Gfx::tSolidColorRenderVertex( v2, vtxColor ) );
		mVerts.fPushBack( Gfx::tSolidColorRenderVertex( v3, vtxColor ) );

		mIds.fPushBack( baseVertex + 0 );
		mIds.fPushBack( baseVertex + 1 );
		mIds.fPushBack( baseVertex + 2 );

		mIds.fPushBack( baseVertex + 2 );
		mIds.fPushBack( baseVertex + 3 );
		mIds.fPushBack( baseVertex + 0 );
	}

	void tSolidColorQuads::fSubmitGeometry( )
	{
		fBake( mVerts, mIds, mIds.fCount( ) / 3 );
	}
}}
