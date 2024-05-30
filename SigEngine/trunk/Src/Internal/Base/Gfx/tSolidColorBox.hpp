#ifndef __tSolidColorBox__
#define __tSolidColorBox__
#include "tSolidColorGeometry.hpp"

namespace Sig { namespace Gfx
{

	///
	/// \brief Lightweight wrapper around tDynamicGeometry, can generate
	/// basic 3D box geometry.
	class base_export tSolidColorBox : public tSolidColorGeometry
	{
		f32 mHalfEdgeLength;
	public:
		tSolidColorBox( );
		virtual void fOnDeviceReset( tDevice* device );
		void fGenerate( f32 halfEdgeLen = 1.f );

		Math::tAabbf fGetBounds( ) const
		{
			return Math::tAabbf( Math::tVec3f( -mHalfEdgeLength ), Math::tVec3f( +mHalfEdgeLength ) );
		}

		b32 fIntersectsRay( const Math::tRayf& ray, f32& tOut ) const;
		b32 fIntersectsFrustum( const Math::tFrustumf& frustum ) const;

	protected:
		void fAddBoxFace( 
			const Math::tVec3f& v0, const Math::tVec3f& v1, const Math::tVec3f& v2, const Math::tVec3f& v3,
			tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
			tGrowableArray< u16 >& ids );
	};

	class base_export tSolidColorCylinder : public tSolidColorGeometry
	{
		f32 mHalfHeight;
		f32 mRadius;

	public:
		tSolidColorCylinder( );
		virtual void fOnDeviceReset( tDevice* device );

		void fGenerate( f32 halfHeight = 1.f, f32 radius = 1.f );

		Math::tAabbf fGetBounds( ) const
		{
			return Math::tAabbf( Math::tVec3f( -mRadius, -mHalfHeight, -mRadius ), Math::tVec3f( mRadius, mHalfHeight, mRadius ) );
		}

		b32 fIntersectsRay( const Math::tRayf& ray, f32& tOut ) const;
		b32 fIntersectsFrustum( const Math::tFrustumf& frustum ) const;
	};
}}

#endif//__tSolidColorBox__
