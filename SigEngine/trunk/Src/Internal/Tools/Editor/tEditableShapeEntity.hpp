#ifndef __tEditableShapeEntity__
#define __tEditableShapeEntity__
#include "tEditableObject.hpp"
#include "Gfx/tDynamicGeometry.hpp"
#include "Math/tConvexHull.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tSolidColorSphere.hpp"

namespace Sig { namespace Sigml { class tShapeObject; }}

namespace Sig
{

	class tShapeDummyObjectEntity : public tEditableObject::tDummyObjectEntity
	{
	public:
		tShapeDummyObjectEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox );

		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32	fIntersects( const Math::tFrustumf& v ) const;

		b32 mConvexMode;
	};

	class tools_export tEditableShapeEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableShapeEntity, tEditableObject );
	public:
		Gfx::tRenderState mRenderState;

		Math::tConvexHull			mHull;
		Gfx::tDynamicGeometry		mHullGeometry;
		tRefCounterPtr<tShapeDummyObjectEntity>	mDummyHull;
		Gfx::tSolidColorSphere		mCapsuleGeometry;
		tRefCounterPtr<tShapeDummyObjectEntity>	mDummyCapsule;
		Gfx::tMaterialPtr			mHullMaterial;
		Gfx::tRenderableEntityPtr	mDummyCylinder;

		tEditableShapeEntity( tEditableObjectContainer& container );
		tEditableShapeEntity( tEditableObjectContainer& container, const Sigml::tShapeObject& ao );
		~tEditableShapeEntity( );
		virtual std::string fGetToolTip( ) const;
		virtual b32 fUniformScaleOnly( );
		virtual b32 fUniformScaleInXZ( );
		virtual b32 fSupportsScale( );

		virtual void fOnMoved( b32 recomputeParentRelative );

		virtual void fOnDeviceLost( Gfx::tDevice* device );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
	private:
		void fAddEditableProperties( );
		void fCommonCtor( );
		void fSetObjectBounds( );
	protected:
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
		virtual std::string fGetSoftName( ) const;
		void fGenerateCapsule( );
	};

}

#endif//__tEditableShapeEntity__
