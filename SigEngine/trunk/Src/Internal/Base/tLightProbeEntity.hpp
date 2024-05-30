#ifndef __tLightProbeEntity__
#define __tLightProbeEntity__
#include "tEntityDef.hpp"
#include "tSpatialEntity.hpp"
#include "Gfx/tSphericalHarmonics.hpp"

namespace Sig
{
	class base_export tLightProbeEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tLightProbeEntityDef, 0xA24BE7AB );
	public:
		Gfx::tSphericalHarmonics mHarmonics;

	public:
		tLightProbeEntityDef( );
		tLightProbeEntityDef( tNoOpTag );
		~tLightProbeEntityDef( );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;
	};

	class base_export tLightProbeEntity : public tSpatialEntity
	{
		define_dynamic_cast( tLightProbeEntity, tSpatialEntity );
	private:
		const tLightProbeEntityDef* mEntityDef;
	public:
		tLightProbeEntity( const tLightProbeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld );

		static const u32 cSpatialSetIndex;
		virtual u32	fSpatialSetIndex( ) const { return cSpatialSetIndex; }
		virtual b32 fIsHelper( ) const { return true; }

		virtual const tEntityDef* fQueryEntityDef( ) const { return mEntityDef; }
		virtual void fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton );

		virtual void fOnSpawn( );

		const tLightProbeEntityDef* fDef( ) const { return mEntityDef; }
	};

	define_smart_ptr( base_export, tRefCounterPtr, tLightProbeEntity );

}

#endif//__tLightProbeEntity__
