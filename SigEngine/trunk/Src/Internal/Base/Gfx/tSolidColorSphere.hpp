#ifndef __tSolidColorSphere__
#define __tSolidColorSphere__
#include "tSolidColorGeometry.hpp"

namespace Sig { namespace Gfx
{

	class base_export tSolidColorSphere : public tSolidColorGeometry
	{
		f32 mRadius;
	public:
		tSolidColorSphere( );
		virtual void fOnDeviceReset( tDevice* device );
		void fGenerate( f32 radius = 1.f, f32 capsuleHeight = 0.f );

		Math::tAabbf fGetBounds( ) const;
		Math::tSpheref fGetSphere( const Math::tMat3f& objectToWorld ) const;

		b32 fIntersectsRay( const Math::tRayf& ray, f32& tOut ) const;
		b32 fIntersectsFrustum( const Math::tFrustumf& frustum ) const;

	public:
		template< class vertType >
		static void fGenerateData( tGrowableArray< vertType >& vertsOut, tGrowableArray< u16 >& indsOut, f32 radius, f32 capsuleHeight )
		{
			const u32 numVertSegs = 24;
			const u32 numHorzSegs = 24;

			const u32 numVertsPerRow = numHorzSegs;

			f32 phi = 0.f;
			const f32 dPhi = Math::cPi / ( numVertSegs - 1.f );

			for( u32 v = 0; v < numVertSegs; ++v )
			{
				f32 theta = 0.f;
				const f32 dTheta = Math::c2Pi / ( numHorzSegs - 1.f );

				f32 yCoord = Math::fCos( phi );
				const f32 innerRadius = Math::fSin( phi );

				f32 yShift = 0;
				if( yCoord > 0 )
					yShift = capsuleHeight;
				else if ( yCoord < 0 )
					yShift = -capsuleHeight;

				for( u32 h = 0; h < numHorzSegs; ++h )
				{
					const f32 xCoord = innerRadius * Math::fCos( theta );
					const f32 zCoord = innerRadius * Math::fSin( theta );

					const Math::tVec3f n( xCoord, yCoord, zCoord );
					const Math::tVec3f p = n * radius + Math::tVec3f( 0, yShift, 0 );

					vertsOut.fPushBack( vertType( p ) );

					if( h < numHorzSegs - 1 && v < numVertSegs - 1 )
					{
						indsOut.fPushBack( ( v + 0 ) * numVertsPerRow + h + 0 );
						indsOut.fPushBack( ( v + 0 ) * numVertsPerRow + h + 1 );
						indsOut.fPushBack( ( v + 1 ) * numVertsPerRow + h + 1 );

						indsOut.fPushBack( ( v + 1 ) * numVertsPerRow + h + 1 );
						indsOut.fPushBack( ( v + 1 ) * numVertsPerRow + h + 0 );
						indsOut.fPushBack( ( v + 0 ) * numVertsPerRow + h + 0 );
					}

					theta += dTheta;
				}

				sigassert( fEqual( theta, Math::c2Pi + dTheta ) );

				phi += dPhi;
			}

			sigassert( fEqual( phi, Math::cPi + dPhi ) );
		}
	};
}}

#endif//__tSolidColorSphere__
