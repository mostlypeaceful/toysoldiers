#ifndef __tEditableLightProbeEntity__
#define __tEditableLightProbeEntity__
#include "tEditableObject.hpp"
#include "Gfx/tDynamicGeometry.hpp"
#include "Math/tConvexHull.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tSolidColorSphere.hpp"
#include "gfx/tSphericalHarmonics.hpp"
#include "tEditablePropertyTypes.hpp"

namespace Sig { namespace Sigml { class tLightProbeObject; }}

namespace Sig
{

	struct tEditableLightProbeData
	{
		// True if this has already been automatically calculated once. so it doesnt keep nuking your data.
		b32 mComputed;

		// The computed harmonics, that will be used for rendering, serialized to game.
		Gfx::tSphericalHarmonics mHarmonics;

		// Things the user can adjust about how the harmonics are computed. Editor only info.
		tEditablePropertyTable mUserOptions;

		// Convenient functions for accessing user options.
		std::string fCubeMapFileName( ) const;
		void fSetCubeMapFileName( const std::string& name );

		tEditableLightProbeData( )
			: mComputed( false )
		{ }

		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			s( "C", mComputed );
			s( "H", mHarmonics );
			s( "O", mUserOptions );
		}

		b32 operator == ( const tEditableLightProbeData& other ) const
		{
			return mComputed == other.mComputed 
				&& mHarmonics == other.mHarmonics 
				&& mUserOptions.fEqual( other.mUserOptions );
		}

		static const char* fEditablePropHarmonicsEq( u32 index );
		static const char* fEditablePropCubeMapButtons( );
		static const char* fEditablePropCubeMapFile( );
		
		static const std::string cCommandSave;
		static const std::string cCommandLoad;
		static const std::string cCommandRender;
		static const std::string cCommandRefresh;
	};


	class tools_export tEditableLightProbeEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableLightProbeEntity, tEditableObject );
	public:
		Gfx::tRenderState mRenderState;

		tEditableLightProbeEntity( tEditableObjectContainer& container );
		tEditableLightProbeEntity( tEditableObjectContainer& container, const Sigml::tLightProbeObject& ao );
		~tEditableLightProbeEntity( );

		virtual std::string fGetToolTip( ) const { return "Light probe"; }
		virtual b32 fUniformScaleOnly( ) { return false; }
		virtual b32 fUniformScaleInXZ( ) { return false; }
		virtual b32 fSupportsScale( ) { return false; }
		virtual b32 fSupportsRotation( ) { return false; }

	private:
		void fAddEditableProperties( );
		void fCommonCtor( );
		void fSetObjectBounds( );
		void fApply( );

	protected:
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
	};

}

#endif//__tEditableLightProbeEntity__
