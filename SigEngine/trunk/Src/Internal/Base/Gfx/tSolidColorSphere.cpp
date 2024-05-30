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

	void tSolidColorSphere::fGenerate( f32 radius, f32 capsuleHeight )
	{
		mRadius = radius;

		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > ids;

		fGenerateData( verts, ids, radius, capsuleHeight );
	
		// bake some fake lighting
		const Math::tVec3f l = Math::tVec3f( 1.f ).fNormalize( );
		for( u32 i = 0; i < verts.fCount( ); ++i )
		{
			const Math::tVec3f n = verts[ i ].mP;
			const f32 nDotL = n.fDot( l );
			const f32 intensity = 0.5f + 0.5f * nDotL;

			const u32 c = Gfx::tVertexColor( intensity, intensity, intensity ).fForGpu( );
			verts[ i ].mColor = c;
		}

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

