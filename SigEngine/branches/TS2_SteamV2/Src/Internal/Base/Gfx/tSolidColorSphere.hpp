#ifndef __tSolidColorSphere__
#define __tSolidColorSphere__
#include "tSolidColorGeometry.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \brief Lightweight wrapper around tDynamicGeometry, can generate
	/// basic 3D box geometry.
	class base_export tSolidColorSphere : public tSolidColorGeometry
	{
		f32 mRadius;
	public:
		tSolidColorSphere( );
		virtual void fOnDeviceReset( tDevice* device );
		void fGenerate( f32 radius = 1.f );

		Math::tAabbf fGetBounds( ) const;
		Math::tSpheref fGetSphere( const Math::tMat3f& objectToWorld ) const;

		b32 fIntersectsRay( const Math::tRayf& ray, f32& tOut ) const;
		b32 fIntersectsFrustum( const Math::tFrustumf& frustum ) const;

	};
}}

#endif//__tSolidColorSphere__
