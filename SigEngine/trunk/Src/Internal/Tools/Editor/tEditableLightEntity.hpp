#ifndef __tEditableLightEntity__
#define __tEditableLightEntity__
#include "tEditableObject.hpp"
#include "Gfx/tLightEntity.hpp"

namespace Sig { 
namespace Sigml { 
	class tools_export tPointLightObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tPointLightObject, 0xE3C6F1C4 );
	public:

	public:
		tPointLightObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );

		void fCleanupProps( );
	};
} }

namespace Sig
{
	class tools_export tLightBulbEntity : public tEditableObject::tDummyObjectEntity
	{
	public:
		tLightBulbEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox, b32 useSphereCollision = false );
		virtual void				fOnMoved( b32 recomputeParentRelative );
	};
	define_smart_ptr( tools_export, tRefCounterPtr, tLightBulbEntity );

	class tools_export tEditableLightEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableLightEntity, tEditableObject );

	public:
		static const char* fEditablePropApplyToScene( ) { return "Display.ApplyToScene"; }
		static const char* fEditablePropDisplayShells( ) { return "Display.RadiiSpheres"; }

		static const char* fEditablePropCastShadows( ) { return "Shadows.Shadows"; }
		static const char* fEditablePropShadowIntensity( ) { return "Shadows.Intensity"; }

		static const char* fEditablePropInnerRadius( ) { return "Light.RadiusInner"; }
		static const char* fEditablePropOuterRadius( ) { return "Light.RadiusOuter"; }
		static const char* fEditablePropIntensity( ) { return "Light.Intensity"; }
		static const char* fEditablePropFrontColor( ) { return "Light.ColorFront"; }
		static const char* fEditablePropBackColor( ) { return "Light.ColorBack"; }
		static const char* fEditablePropRimColor( ) { return "Light.ColorRim"; }
		static const char* fEditablePropAmbientColor( ) { return "Light.ColorAmbient"; }

		static const char* fEditablePropLenseFlare( ) { return "Fx.LenseFlare"; }

	protected:
		Gfx::tLightEntityPtr mRealLight;
		b32 mExternallyDriven;
		tLightBulbEntityPtr mLightBulb;
		Gfx::tRenderableEntityPtr mInnerRadiusSphere, mOuterRadiusSphere;
		Gfx::tRenderState mShellRenderState;
	public:
		tEditableLightEntity( tEditableObjectContainer& container );
		tEditableLightEntity( tEditableObjectContainer& container, const Sigml::tPointLightObject& ao );
		virtual ~tEditableLightEntity( );
		virtual std::string fGetToolTip( ) const;
		virtual b32 fUniformScaleOnly( )	{ return true; }
		f32 fInnerRadius( ) const;
		f32 fOuterRadius( ) const;
		b32 fDisplayShells( ) const;
		b32 fApplyLightToScene( ) const;
		Math::tVec4f fFrontColor( ) const;
		Math::tVec4f fBackColor( ) const;
		Math::tVec4f fRimColor( ) const;
		Math::tVec4f fAmbientColor( ) const;
		virtual void fOnDeviceLost( Gfx::tDevice* device );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
	protected:
		void fAddEditableProperties( );
		void fCommonCtor( );
		void fUpdateBulbTint( );
		void fUpdateRealLight( );
		virtual void fRemoveRealLight( );
		Math::tVec4f fGetColor( const char* colorName ) const;
	protected:
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
	};

}

#endif//__tEditableLightEntity__
