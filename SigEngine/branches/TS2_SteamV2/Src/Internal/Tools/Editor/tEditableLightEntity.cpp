#include "ToolsPch.hpp"
#include "tEditableLightEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tEditablePropertyColor.hpp"
#include "tSigmlConverter.hpp"

namespace
{
	static const char* fEditablePropApplyToScene( ) { return "Display.ApplyToScene"; }
	static const char* fEditablePropDisplayShells( ) { return "Display.RadiiSpheres"; }
	static const char* fEditablePropCastShadows( ) { return "Light.CastShadows"; }
	static const char* fEditablePropInnerRadius( ) { return "Light.RadiusInner"; }
	static const char* fEditablePropOuterRadius( ) { return "Light.RadiusOuter"; }
	static const char* fEditablePropIntensity( ) { return "Light.Intensity"; }
	static const char* fEditablePropFrontColor( ) { return "Light.ColorFront"; }
	static const char* fEditablePropBackColor( ) { return "Light.ColorBack"; }
	static const char* fEditablePropRimColor( ) { return "Light.ColorRim"; }
	static const char* fEditablePropAmbientColor( ) { return "Light.ColorAmbient"; }
}

namespace Sig { namespace Sigml
{
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
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPointLightObject& o )
	{
	}

	register_rtti_factory( tPointLightObject, false );

	tPointLightObject::tPointLightObject( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
	}

	void tPointLightObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tPointLightObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}

	tEntityDef* tPointLightObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		Gfx::tLightEntityDef* entityDef = new Gfx::tLightEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );

		Gfx::tLight lightData;

		Math::tVec2f radii;
		radii.x = mEditableProperties.fGetValue( fEditablePropInnerRadius( ), 10.f );
		radii.y = mEditableProperties.fGetValue( fEditablePropOuterRadius( ), 15.f );
		lightData.fSetTypePoint( radii );

		tColorPickerData cpd = mEditableProperties.fGetValue( fEditablePropFrontColor( ), tColorPickerData( ) );
		cpd.mRgbScale = mEditableProperties.fGetValue( fEditablePropIntensity( ), 1.f );

		lightData.fColor( Gfx::tLight::cColorTypeFront ) = cpd.fExpandRgba( );
		lightData.fColor( Gfx::tLight::cColorTypeBack ) = Math::tVec4f::cZeroVector;//mEditableProperties[fEditablePropBackColor( )]->fGetData( c ).fExpandRgba( );
		lightData.fColor( Gfx::tLight::cColorTypeRim ) = Math::tVec4f::cZeroVector;//mEditableProperties[fEditablePropRimColor( )]->fGetData( c ).fExpandRgba( );
		lightData.fColor( Gfx::tLight::cColorTypeAmbient ) = Math::tVec4f::cZeroVector;//mEditableProperties[fEditablePropAmbientColor( )]->fGetData( c ).fExpandRgba( );

		entityDef->mLightDesc = lightData;

		return entityDef;
	}

	tEditableObject* tPointLightObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableLightEntity( container, *this );
	}
}}




namespace Sig
{
	namespace
	{
		static const f32 gLightBulbRadius = 1.0f;
		static const Math::tVec4f gInnerShellSphereTint = Math::tVec4f( 1.0f, 1.0f, 0.0f, 1.0f );
		static const Math::tVec4f gOuterShellSphereTint = Math::tVec4f( 0.0f, 0.0f, 0.0f, 1.0f );
	}

	tLightBulbEntity::tLightBulbEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox, b32 useSphereCollision )
		: tEditableObject::tDummyObjectEntity( batchPtr, objectSpaceBox, useSphereCollision )
	{
	}

	void tLightBulbEntity::fOnMoved( b32 recomputeParentRelative )
	{
		mObjectToWorld.fNormalizeBasis( ); // disallow scaling the light bulb
		mObjectToWorld.fScaleLocal( gLightBulbRadius );
		tEditableObject::tDummyObjectEntity::fOnMoved( recomputeParentRelative );
	}

	tEditableLightEntity::tEditableLightEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fCommonCtor( );
	}

	tEditableLightEntity::tEditableLightEntity( tEditableObjectContainer& container, const Sigml::tPointLightObject& ao )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fDeserializeBaseObject( &ao );
		fCommonCtor( );
	}

	void tEditableLightEntity::fAddEditableProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropApplyToScene( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropDisplayShells( ), true ) ) );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropInnerRadius( ), 10.f, 0.f, 9999999.f, 0.1f, 1 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropOuterRadius( ), 15.f, 0.f, 9999999.f, 0.1f, 1 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropIntensity( ), 1.f, 0.f, 9999999.f, 0.1f, 1 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropCastShadows( ), true ) ) );
		
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropFrontColor( ), tColorPickerData( 1.f ) ) ) );
		//mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropBackColor( ), tColorPickerData( 0.f ) ) ) );
		//mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropRimColor( ), tColorPickerData( 0.f ) ) ) );
		//mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropAmbientColor( ), tColorPickerData( 0.f ) ) ) );
	}

	void tEditableLightEntity::fCommonCtor( )
	{
		mShellRenderState = Gfx::tRenderState::cDefaultColorAdditive;
		//mShellRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided | Gfx::tRenderState::cFillWireFrame, true );

		const f32 lilRadius = fInnerRadius( );
		const f32 bigRadius = fOuterRadius( );

		// set the dummy object's bounds as our own
		Math::tAabbf localSpaceBox = mContainer.fGetDummySphereTemplate( ).fGetBounds( );
		localSpaceBox.mMin *= gLightBulbRadius;
		localSpaceBox.mMax *= gLightBulbRadius;
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		// setup the small, fixed sized light bulb at the center
		mLightBulb.fReset( new tLightBulbEntity( 
			mContainer.fGetDummySphereTemplate( ).fGetModifiedRenderBatch( &mShellRenderState ), 
			mContainer.fGetDummySphereTemplate( ).fGetBounds( ), true ) );
		fUpdateBulbTint( );
		const Math::tMat3f mBulb( gLightBulbRadius );
		mLightBulb->fSpawnImmediate( *this );
		mLightBulb->fSetParentRelativeXform( mBulb );

		// create the inner radius shell
		mInnerRadiusSphere.fReset( new Gfx::tRenderableEntity( 
			mContainer.fGetSphereCageSparseTemplate( ).fGetRenderBatch( ), 
			mContainer.fGetDummyBoxTemplate( ).fGetBounds( ) ) );
		mInnerRadiusSphere->fSetRgbaTint( gInnerShellSphereTint );
		Math::tMat3f mLil( Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, Math::cPiOver4 ) ) );
		mLil.fScaleLocal( lilRadius );

		// TODO REFACTOR figure out why switching the order of the following two lines doesn't work
		mInnerRadiusSphere->fSpawnImmediate( *this );
		mInnerRadiusSphere->fSetParentRelativeXform( mLil );

		// create the outer radius shell
		mOuterRadiusSphere.fReset( new Gfx::tRenderableEntity( 
			mContainer.fGetSphereCageSparseTemplate( ).fGetRenderBatch( ), 
			mContainer.fGetDummyBoxTemplate( ).fGetBounds( ) ) );
		mOuterRadiusSphere->fSetRgbaTint( gOuterShellSphereTint );
		const Math::tMat3f mBig( bigRadius );

		// TODO REFACTOR figure out why switching the order of the following two lines doesn't work
		mOuterRadiusSphere->fSpawnImmediate( *this );
		mOuterRadiusSphere->fSetParentRelativeXform( mBig );

		const b32 display = fDisplayShells( );
		mInnerRadiusSphere->fSetDisabled( !display );
		mOuterRadiusSphere->fSetDisabled( !display );

		fUpdateRealLight( );
	}

	void tEditableLightEntity::fUpdateBulbTint( )
	{
		Math::tVec4f bulbTint = fFrontColor( );
		bulbTint.w = 0.25f;
		mLightBulb->fSetRgbaTint( bulbTint );
	}

	void tEditableLightEntity::fUpdateRealLight( )
	{
		const b32 applyLight = fApplyLightToScene( );

		if( !applyLight )
			fRemoveRealLight( );
		else
		{
			const Math::tVec2f radii( fInnerRadius( ), fOuterRadius( ) );
			if( mRealLight )
				mRealLight->fSetRadii( radii );
			else
			{
				Gfx::tLight lightData;
				lightData.fSetTypePoint( radii );
				mRealLight.fReset( new Gfx::tLightEntity( Math::tMat3f::cIdentity, lightData ) );
				mRealLight->fSpawnImmediate( *this );
			}

			mRealLight->fColor( Gfx::tLight::cColorTypeFront ) = fFrontColor( );
			mRealLight->fColor( Gfx::tLight::cColorTypeBack ) = fBackColor( );
			mRealLight->fColor( Gfx::tLight::cColorTypeRim ) = fRimColor( );
			mRealLight->fColor( Gfx::tLight::cColorTypeAmbient ) = fAmbientColor( );
		}
	}

	void tEditableLightEntity::fRemoveRealLight( )
	{
		if( mRealLight )
		{
			mRealLight->fDeleteImmediate( );
			mRealLight.fRelease( );
		}
	}

	tEditableLightEntity::~tEditableLightEntity( )
	{
		fRemoveRealLight( );
	}

	std::string tEditableLightEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "Light - " + name;
		return "Light";
	}

	void tEditableLightEntity::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );
		if( !fIsFrozen( ) )
		{
			if( &entity == mLightBulb.fGetRawPtr( ) )
				fUpdateBulbTint( );
			else if( &entity == mOuterRadiusSphere.fGetRawPtr( ) )
				mOuterRadiusSphere->fSetRgbaTint( gOuterShellSphereTint );
			else if( &entity == mInnerRadiusSphere.fGetRawPtr( ) )
				mInnerRadiusSphere->fSetRgbaTint( gInnerShellSphereTint );
		}
	}

	Sigml::tObjectPtr tEditableLightEntity::fSerialize( b32 clone ) const
	{
		Sigml::tPointLightObject* ao = new Sigml::tPointLightObject( );
		fSerializeBaseObject( ao, clone );
		return Sigml::tObjectPtr( ao );
	}

	void tEditableLightEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		const f32 lilRadius = fInnerRadius( );
		const f32 bigRadius = fOuterRadius( );

		if( property.fGetName( ) == fEditablePropInnerRadius( ) )
		{
			Math::tMat3f mLil( Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, Math::cPiOver4 ) ) );
			mLil.fScaleLocal( lilRadius );
			mInnerRadiusSphere->fSetParentRelativeXform( mLil );
			if( lilRadius > bigRadius )
				mEditableProperties.fSetData( fEditablePropOuterRadius( ), lilRadius );
			else
				fUpdateRealLight( );
		}
		else if( property.fGetName( ) == fEditablePropOuterRadius( ) )
		{
			const Math::tMat3f mBig( bigRadius );
			mOuterRadiusSphere->fSetParentRelativeXform( mBig );

			if( bigRadius < lilRadius )
				mEditableProperties.fSetData( fEditablePropInnerRadius( ), bigRadius );
			else
				fUpdateRealLight( );
		}
		else if( 
			property.fGetName( ) == fEditablePropApplyToScene( ) ||
			property.fGetName( ) == fEditablePropFrontColor( ) ||
			property.fGetName( ) == fEditablePropBackColor( ) ||
			property.fGetName( ) == fEditablePropRimColor( ) ||
			property.fGetName( ) == fEditablePropAmbientColor( ) ||
			property.fGetName( ) == fEditablePropIntensity( ) )
		{
			fUpdateBulbTint( );
			fUpdateRealLight( );
		}
		else if( property.fGetName( ) == fEditablePropDisplayShells( ) )
		{
			const b32 display = fDisplayShells( );
			mInnerRadiusSphere->fSetDisabled( !display );
			mOuterRadiusSphere->fSetDisabled( !display );
		}
	}

	f32 tEditableLightEntity::fInnerRadius( ) const
	{
		return mEditableProperties.fGetValue( fEditablePropInnerRadius( ), 0.f );
	}

	f32 tEditableLightEntity::fOuterRadius( ) const
	{
		return mEditableProperties.fGetValue( fEditablePropOuterRadius( ), 0.f );
	}

	b32 tEditableLightEntity::fApplyLightToScene( ) const
	{
		return mEditableProperties.fGetValue( fEditablePropApplyToScene( ), true );
	}

	b32 tEditableLightEntity::fDisplayShells( ) const
	{
		return mEditableProperties.fGetValue( fEditablePropDisplayShells( ), true );
	}

	Math::tVec4f tEditableLightEntity::fGetColor( const char* colorName ) const
	{
		if( !mEditableProperties.fFind( colorName ) )
			return Math::tVec4f::cZeroVector;

		tColorPickerData cpd = mEditableProperties.fGetValue( colorName, tColorPickerData( ) );
		cpd.mRgbScale = mEditableProperties.fGetValue( fEditablePropIntensity( ), 1.f );

		return cpd.fExpandRgba( );
	}

	Math::tVec4f tEditableLightEntity::fFrontColor( ) const
	{
		return fGetColor( fEditablePropFrontColor( ) );
	}

	Math::tVec4f tEditableLightEntity::fBackColor( ) const
	{
		return fGetColor( fEditablePropBackColor( ) );
	}

	Math::tVec4f tEditableLightEntity::fRimColor( ) const
	{
		return fGetColor( fEditablePropRimColor( ) );
	}

	Math::tVec4f tEditableLightEntity::fAmbientColor( ) const
	{
		return fGetColor( fEditablePropAmbientColor( ) );
	}

}

