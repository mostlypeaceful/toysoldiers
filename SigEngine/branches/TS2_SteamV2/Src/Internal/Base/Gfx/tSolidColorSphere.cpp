#include "BasePch.hpp"
#include "tSolidColorSphere.hpp"
#include "tSolidColorMaterial.hpp"

#include "Math/tIntersectionRaySphere.hpp"
#include "Math/tIntersectionSphereFrustum.hpp"

namespace Sig { namespace Gfx
{


	tSolidColorSphere::tSolidColorSphere( )
		: mRadius( 1.f )
	{
		fSetPrimTypeOverride( tIndexFormat::cPrimitiveTriangleList );
	}

	void tSolidColorSphere::fOnDeviceReset( tDevice* device )
	{
		fGenerate( mRadius );
	}

	void tSolidColorSphere::fGenerate( f32 radius )
	{
		mRadius = radius;

		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > ids;

		const u32 numVertSegs = 24;
		const u32 numHorzSegs = 24;

		const u32 numVertsPerRow = numHorzSegs;

		const Math::tVec3f l = Math::tVec3f( 1.f ).fNormalize( );

		f32 phi = 0.f;
		const f32 dPhi = Math::cPi / ( numVertSegs - 1.f );

		for( u32 v = 0; v < numVertSegs; ++v )
		{
			f32 theta = 0.f;
			const f32 dTheta = Math::c2Pi / ( numHorzSegs - 1.f );

			const f32 yCoord = Math::fCos( phi );
			const f32 innerRadius = Math::fSin( phi );

			for( u32 h = 0; h < numHorzSegs; ++h )
			{
				const f32 xCoord = innerRadius * Math::fCos( theta );
				const f32 zCoord = innerRadius * Math::fSin( theta );

				const Math::tVec3f n( xCoord, yCoord, zCoord );
				const Math::tVec3f p = n * radius;
				const f32 nDotL = n.fDot( l );
				const f32 intensity = 0.5f + 0.5f * nDotL;

				const u32 c = Gfx::tVertexColor( intensity, intensity, intensity ).fForGpu( );

				verts.fPushBack( Gfx::tSolidColorRenderVertex( p, c ) );

				if( h < numHorzSegs - 1 && v < numVertSegs - 1 )
				{
					ids.fPushBack( ( v + 0 ) * numVertsPerRow + h + 0 );
					ids.fPushBack( ( v + 0 ) * numVertsPerRow + h + 1 );
					ids.fPushBack( ( v + 1 ) * numVertsPerRow + h + 1 );

					ids.fPushBack( ( v + 1 ) * numVertsPerRow + h + 1 );
					ids.fPushBack( ( v + 1 ) * numVertsPerRow + h + 0 );
					ids.fPushBack( ( v + 0 ) * numVertsPerRow + h + 0 );
				}

				theta += dTheta;
			}

			sigassert( fEqual( theta, Math::c2Pi + dTheta ) );

			phi += dPhi;
		}

		sigassert( fEqual( phi, Math::cPi + dPhi ) );


		fBake( verts, ids, ids.fCount( ) / 3 );
	}

	Math::tAabbf tSolidColorSphere::fGetBounds( ) const
	{
		return Math::tAabbf( Math::tVec3f( -mRadius ), Math::tVec3f( +mRadius ) );
	}

	Math::tSpheref tSolidColorSphere::fGetSphere( const Math::tMat3f& objectToWorld ) const
	{
		return Math::tSpheref( objectToWorld.fGetTranslation( ), objectToWorld.fGetScale( ).fMaxMagnitude( ) * mRadius );
	}

	b32 tSolidColorSphere::fIntersectsRay( const Math::tRayf& ray, f32& tOut ) const
	{
		Math::tIntersectionRaySphere<f32> intersection( ray, Math::tSpheref( Math::tVec3f::cZeroVector, mRadius ) );
		if( intersection.fIntersects( ) )
		{
			tOut = intersection.fT( );
			return true;
		}

		return false;
	}

	b32 tSolidColorSphere::fIntersectsFrustum( const Math::tFrustumf& frustum ) const
	{
		Math::tIntersectionSphereFrustum<f32> intersection( frustum, Math::tSpheref( Math::tVec3f::cZeroVector, mRadius ) );
		if( intersection.fIntersects( ) )
		{
			return true;
		}

		return false;
	}


}}

