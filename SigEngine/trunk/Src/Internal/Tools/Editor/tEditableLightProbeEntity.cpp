#include "ToolsPch.hpp"
#include "tEditableLightProbeEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tSigmlConverter.hpp"
#include "tEditableSgFileRefEntity.hpp"
#include "tLightProbeEntity.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tWxSlapOnButton.hpp"
#include "tSigEdLightEdDialog.hpp"
#include "Gfx/tRenderContext.hpp"

namespace Sig { 

	namespace
	{
		const f32 gDefaultRadius = 1.0f; // can never change this without breaking all existing dummy objects
		static const Math::tVec4f cDummyTint = Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.5f );

		// These properties are not meant for the main "object properties dialog".
		//  They are meant for the LightEdDialog
		static const char* gHarmonicPropNames[ Gfx::tShBasisWeights::cEditableWeightCount ] =
		{
			"Harmonics Eq.Scale",
			"Harmonics Eq.Base",
			"Harmonics Eq.Contrast",
		};

		static const char* fEditablePropApply( ) { return "Probe.Apply"; }
		static const char* fEditablePropProbeData( ) { return "Probe.Data"; }
		
	}

	const char* tEditableLightProbeData::fEditablePropHarmonicsEq( u32 index ) 
	{ 
		sigassert( index < Gfx::tShBasisWeights::cEditableWeightCount ); 
		return gHarmonicPropNames[ index ]; 
	}

	const char*	tEditableLightProbeData::fEditablePropCubeMapButtons( ) { return "CubeMap.Buttons"; }
	const char*	tEditableLightProbeData::fEditablePropCubeMapFile( ) { return "CubeMap.Filename"; }

	const std::string tEditableLightProbeData::cCommandSave = "Save";
	const std::string tEditableLightProbeData::cCommandLoad = "Load";
	const std::string tEditableLightProbeData::cCommandRender = "Render";
	const std::string tEditableLightProbeData::cCommandRefresh = "Refresh";


	std::string tEditableLightProbeData::fCubeMapFileName( ) const
	{
		return mUserOptions.fGetValue( fEditablePropCubeMapFile( ), std::string("") );
	}

	void tEditableLightProbeData::fSetCubeMapFileName( const std::string& name )
	{
		mUserOptions.fSetData( fEditablePropCubeMapFile( ), name );
	}

	class tEditablePropertyLightProbeData : public tEditablePropertyUserData<tEditableLightProbeData, tSigEdLightEdDialog>
	{
		implement_rtti_serializable_base_class( tEditablePropertyLightProbeData, 0xACFD1543 );
	public:
		tEditablePropertyLightProbeData( )
		{
		}

		tEditablePropertyLightProbeData( const std::string& name, const tEditableLightProbeData& initValue = fDefaultData( ) )
			: tEditablePropertyUserData<tEditableLightProbeData, tSigEdLightEdDialog>( name, initValue )
		{ }

		virtual tEditableProperty* fClone( ) const
		{ 
			return new tEditablePropertyLightProbeData( mName, mRawData ); 
		}

		static tEditableLightProbeData fDefaultData( )
		{
			tEditableLightProbeData defaults;

			Gfx::tShBasisWeights weights;
			const f32 cMin = -2.f;
			const f32 cMax = 2.f;
			const f32 cIncrement = 0.1f;
			const u32 cPrecision = 2;

			for( u32 i = 0; i < Gfx::tShBasisWeights::cEditableWeightCount; ++i )
				defaults.mUserOptions.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( tEditableLightProbeData::fEditablePropHarmonicsEq( i ), weights.mWeights[ i ], cMin, cMax, cIncrement, cPrecision ) ) );


			tDynamicArray<std::string> buttons;
			buttons.fNewArray( 4 );
			buttons[ 0 ] = tEditableLightProbeData::cCommandSave;
			buttons[ 1 ] = tEditableLightProbeData::cCommandLoad;
			buttons[ 2 ] = tEditableLightProbeData::cCommandRender;
			buttons[ 3 ] = tEditableLightProbeData::cCommandRefresh;		
			defaults.mUserOptions.fInsert( tEditablePropertyPtr( new tEditablePropertyButtons( tEditableLightProbeData::fEditablePropCubeMapButtons( ), buttons ) ) );
			defaults.mUserOptions.fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( tEditableLightProbeData::fEditablePropCubeMapFile( ) ) ) );

			return defaults;
		}
	};

	register_rtti_factory( tEditablePropertyLightProbeData, false )

}



namespace Sig { namespace Sigml
{
	class tools_export tLightProbeObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tLightProbeObject, 0x45517EE6 );
	public:
		tLightProbeObject( ) { }
		tLightProbeObject( const tEditableLightProbeEntity* ao ) { }
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tLightProbeObject& o )
	{
	}

	register_rtti_factory( tLightProbeObject, false );

	void tLightProbeObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tLightProbeObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}

	tEntityDef* tLightProbeObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tLightProbeEntityDef* entityDef = new tLightProbeEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );

		tEditableLightProbeData data = mEditableProperties.fGetValue( fEditablePropProbeData( ), tEditableLightProbeData( ) );
		entityDef->mHarmonics = data.mHarmonics;
		return entityDef;
	}

	tEditableObject* tLightProbeObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableLightProbeEntity( container, *this );
	}

}}




namespace Sig
{

	/*
		Use this if you need custom "dummy object" behavior.
	*/
	//class tLightProbeObjectEntity : public tEditableObject::tDummyObjectEntity
	//{
	//public:
	//	tLightProbeObjectEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox )
	//		: tEditableObject::tDummyObjectEntity( batchPtr, objectSpaceBox, true )
	//	{
	//	}
	//};

	tEditableLightProbeEntity::tEditableLightProbeEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fCommonCtor( );
	}

	tEditableLightProbeEntity::tEditableLightProbeEntity( tEditableObjectContainer& container, const Sigml::tLightProbeObject& ao )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fDeserializeBaseObject( &ao );
		fCommonCtor( );
	}

	void tEditableLightProbeEntity::fAddEditableProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyButtons( fEditablePropApply( ), "Apply" ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyLightProbeData( fEditablePropProbeData( ) ) ) );
	}

	void tEditableLightProbeEntity::fCommonCtor( )
	{
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );

		mDummySphere->fSetInvisible( false );

		fSetObjectBounds( );
		tEditableObject::fUpdateStateTint( );

		fApply( );
	}

	tEditableLightProbeEntity::~tEditableLightProbeEntity( )
	{
	}

	void tEditableLightProbeEntity::fSetObjectBounds( )
	{
		Math::tAabbf localSpaceBox;

		// set the dummy object's bounds as our own
		localSpaceBox = mContainer.fGetDummyBoxTemplate( ).fGetBounds( );
		localSpaceBox.mMin *= gDefaultRadius;
		localSpaceBox.mMax *= gDefaultRadius;

		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );
	}

	void tEditableLightProbeEntity::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );

		if( &entity == mDummySphere.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummySphere->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummySphere->fSetRgbaTint( cDummyTint );
		}
	}

	Sigml::tObjectPtr tEditableLightProbeEntity::fSerialize( b32 clone ) const
	{
		Sigml::tLightProbeObject* ao = new Sigml::tLightProbeObject( this );
		fSerializeBaseObject( ao, clone );
		return Sigml::tObjectPtr( ao );
	}

	void tEditableLightProbeEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		if( property.fGetName( ) == fEditablePropApply( ) )
		{
			fApply( );
		}
	}

	void tEditableLightProbeEntity::fApply( )
	{
		tEditableLightProbeData data = mEditableProperties.fGetValue( fEditablePropProbeData( ), tEditableLightProbeData( ) );
		Gfx::tRenderContext::gSphericalHarmonics = data.mHarmonics;
	}

}

