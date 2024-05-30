#ifndef __tEditableLightEntity__
#define __tEditableLightEntity__
#include "tEditableObject.hpp"
#include "Gfx/tLightEntity.hpp"

namespace Sig { namespace Sigml { class tPointLightObject; } }

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
	private:
		Gfx::tLightEntityPtr mRealLight;
		tLightBulbEntityPtr mLightBulb;
		Gfx::tRenderableEntityPtr mInnerRadiusSphere, mOuterRadiusSphere;
		Gfx::tRenderState mShellRenderState;
	public:
		tEditableLightEntity( tEditableObjectContainer& container );
		tEditableLightEntity( tEditableObjectContainer& container, const Sigml::tPointLightObject& ao );
		~tEditableLightEntity( );
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
	private:
		void fAddEditableProperties( );
		void fCommonCtor( );
		void fUpdateBulbTint( );
		void fUpdateRealLight( );
		void fRemoveRealLight( );
		Math::tVec4f fGetColor( const char* colorName ) const;
	protected:
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
	};

}

#endif//__tEditableLightEntity__
