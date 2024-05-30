#ifndef __tSigFxLight__
#define __tSigFxLight__
#include "Editor/tEditableLightEntity.hpp"
#include "FX/tAnimatedLightEntity.hpp"
#include "Fxml.hpp"
#include "tToolAnimatedLightData.hpp"


namespace Sig
{
	///
	/// \class tSigFxLight
	/// \brief Editor-side editable object for an animated light.
	class tools_export tSigFxLight : public tEditableLightEntity
	{
		define_dynamic_cast( tSigFxLight, tEditableLightEntity );

		tToolAnimatedLightDataPtr mToolData;
	public:

		tSigFxLight( tEditableObjectContainer& container );
		tSigFxLight( tEditableObjectContainer& container, const Fxml::tFxLightObject& fxl );

		virtual ~tSigFxLight( ) { }

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		void fClone( const tSigFxLight& light );

		FX::tAnimatedLightEntityPtr fGetLight( ) const { return FX::tAnimatedLightEntityPtr( mRealLight->fStaticCast< FX::tAnimatedLightEntity >( ) ); }

		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		virtual void fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max );
		void fSetCastsShadows( b32 casts ); // Sets the editable property as well.
		b32 fCastsShadows( ) const;

		void fSetShadowIntensity( f32 intensity ); // Ditto on setting the property
		f32 fShadowIntensity( ) const;

		u32 mLastOpenGraphIdx;

	private:

		FX::tAnimatedLightEntity* fRealAnimatedLight( ) const;
		void fCommonCtor( tResourceDepot& resDep );
		void fCreateAnimatedLightData( const tToolAnimatedLightDataPtr& toolData );
	};
}

#endif // __tSigFxLight__
