#ifndef __tEditableCameraEntity__
#define __tEditableCameraEntity__
#include "tEditableObject.hpp"
#include "Gfx/tDynamicGeometry.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tCamera.hpp"
#include "Gfx/tCameraEntity.hpp"

namespace Sig { namespace Sigml { class tCameraObject; }}

namespace Sig
{
	namespace Gfx { class tWorldSpaceLines; }

	class tCameraDummyObjectEntity : public tEditableObject::tDummyObjectEntity
	{
	public:
		tCameraDummyObjectEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox );

		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32	fIntersects( const Math::tFrustumf& v ) const;
	};

	class tools_export tEditableCameraEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableCameraEntity, tEditableObject );
	public:
		// Only for display
		tRefCounterPtr<Gfx::tWorldSpaceLines>		mFrustumGeometry;
		tRefCounterPtr<tCameraDummyObjectEntity>	mDummyFrustum;

		tEditableCameraEntity( tEditableObjectContainer& container );
		tEditableCameraEntity( tEditableObjectContainer& container, const Sigml::tCameraObject& ao );
		~tEditableCameraEntity( );

		virtual std::string fGetToolTip( ) const;
		virtual b32 fSupportsScale( ) { return false; }

		virtual void fOnMoved( b32 recomputeParentRelative );

		std::string fDisplayName( ) const;

		void fExtractCameraData( Gfx::tCamera& camera ) const;
		void fDriveCamera( const Gfx::tCamera& newPositionData );

		// returns half the total horizontal FOV in radians.
		f32 fFOV( ) const;

	private:

		b32 mAdjustingNewLook;

		void fAddEditableProperties( );
		void fCommonCtor( );
		Gfx::tLens fBuildLens( b32 buildVisualizationLens = false ) const;
		void fRebuildVisualization( );
		void fSetFOV( f32 newFOV );

	protected:
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
	};

}

#endif//__tEditableCameraEntity__
