#include "ToolsPch.hpp"
#include "tEditableLightEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tEditablePropertyColor.hpp"
#include "tSigmlConverter.hpp"


namespace Sig { namespace Sigml
{
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPointLightObject& o )
	{
	}

	register_rtti_factory( tPointLightObject, false );

	tPointLightObject::tPointLightObject( )
	{
	}

	void tPointLightObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tPointLightObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
		fCleanupProps( );
	}

	void tPointLightObject::fCleanupProps( )
	{
		// Unfortunately i had released this prematurely, and need to strip out old flare data of the wrong type.
		tEditablePropertyPtr* prop = mEditableProperties.fFind( tEditableLightEntity::fEditablePropLenseFlare( ) );
		if( prop )
		{
			tEditablePropertyLenseFlare* lf = dynamic_cast<tEditablePropertyLenseFlare*>( prop->fGetRawPtr( ) );
			if( !lf )
			{
				// lense flare property was not a tEditablePropertyLenseFlare
				//  remove it
				mEditableProperties.fRemove( tEditableLightEntity::fEditablePropLenseFlare( ) );
			}
		}
	}

	tEntityDef* tPointLightObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		fCleanupProps( );

		Gfx::tLightEntityDef* entityDef = new Gfx::tLightEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );

		Gfx::tLight lightData;

		Math::tVec2f radii;
		radii.x = mEditableProperties.fGetValue( tEditableLightEntity::fEditablePropInnerRadius( ), 10.f );
		radii.y = mEditableProperties.fGetValue( tEditableLightEntity::fEditablePropOuterRadius( ), 15.f );
		lightData.fSetTypePoint( radii );

		tColorPickerData cpd = mEditableProperties.fGetValue( tEditableLightEntity::fEditablePropFrontColor( ), tColorPickerData( ) );
		cpd.mRgbScale = mEditableProperties.fGetValue( tEditableLightEntity::fEditablePropIntensity( ), 1.f );

		lightData.fColor( Gfx::tLight::cColorTypeFront ) = cpd.fExpandRgba( );
		lightData.fColor( Gfx::tLight::cColorTypeBack ) = Math::tVec4f::cZeroVector;//mEditableProperties[fEditablePropBackColor( )]->fGetData( c ).fExpandRgba( );
		lightData.fColor( Gfx::tLight::cColorTypeRim ) = Math::tVec4f::cZeroVector;//mEditableProperties[fEditablePropRimColor( )]->fGetData( c ).fExpandRgba( );
		lightData.fColor( Gfx::tLight::cColorTypeAmbient ) = Math::tVec4f::cZeroVector;//mEditableProperties[fEditablePropAmbientColor( )]->fGetData( c ).fExpandRgba( );

		// Note: tAnimatedLightDef relies on this information too. 
		// If something is added here, it should probably be added there too.
		entityDef->mLightDesc = lightData;
		entityDef->mCastsShadows = mEditableProperties.fGetValue( tEditableLightEntity::fEditablePropCastShadows( ), false );
		entityDef->mShadowIntensity = mEditableProperties.fGetValue( tEditableLightEntity::fEditablePropShadowIntensity( ), 0.2f );
		entityDef->mLenseFlareKey = mEditableProperties.fGetValue<u32>( tEditableLightEntity::fEditablePropLenseFlare( ), ~0 );

		// Add flare resources to be loaded with sigml.
		tProjectFile::tLenseFlare* flare = tProjectFile::fFindItemByKey( entityDef->mLenseFlareKey, tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mLenseFlares );
		if( flare )
		{
			for( u32 i = 0; i < flare->mData.mFlares.fCount( ); ++i )
				sigmlConverter.fAddLoadInPlaceResourcePtr( tResourceId::fMake< Gfx::tTextureFile >( flare->mData.mFlares[ i ].mTexture ) );
		}

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
		, mExternallyDriven( false )
	{
		fAddEditableProperties( );
		fCommonCtor( );
	}

	tEditableLightEntity::tEditableLightEntity( tEditableObjectContainer& container, const Sigml::tPointLightObject& ao )
		: tEditableObject( container )
		, mExternallyDriven( false )
	{
		fDeserializeBaseObject( &ao );
		fAddEditableProperties( );
		fCommonCtor( );
	}

	void tEditableLightEntity::fAddEditableProperties( )
	{
		tGrowableArray< tEditablePropertyPtr > propertiesToKeep;
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyString( Sigml::tObject::fEditablePropObjectName( ) ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropBoneRelativeAttachment( ), false ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropLockTranslation( ), false ) ) ); // Seems troubling that EO base properties don't stick.

		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropApplyToScene( ), true ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropDisplayShells( ), true ) ) );

		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropInnerRadius( ), 10.f, 0.f, 9999999.f, 0.1f, 1 ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropOuterRadius( ), 15.f, 0.f, 9999999.f, 0.1f, 1 ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropIntensity( ), 1.f, 0.f, 9999999.f, 0.1f, 1 ) ) );

		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropCastShadows( ), false ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropShadowIntensity( ), 0.2f, 0.f, 1.f, 0.1f, 2 ) ) );
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyLenseFlare( fEditablePropLenseFlare( ) ) ) );		
				
		propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropFrontColor( ), tColorPickerData( 1.f ) ) ) );
		//propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropBackColor( ), tColorPickerData( 0.f ) ) ) );
		//propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropRimColor( ), tColorPickerData( 0.f ) ) ) );
		//propertiesToKeep.fPushBack( tEditablePropertyPtr( new tEditablePropertyColor( fEditablePropAmbientColor( ), tColorPickerData( 0.f ) ) ) );

		mEditableProperties.fAssignPreferExisting( propertiesToKeep );
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
		if( mExternallyDriven )
			return;

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

				mRealLight.fReset( NEW_TYPED( Gfx::tLightEntity )( Math::tMat3f::cIdentity, lightData ) );
				mRealLight->fSpawnImmediate( *this );
			}

			mRealLight->fSetColor( Gfx::tLight::cColorTypeFront, fFrontColor( ) );
			mRealLight->fSetColor( Gfx::tLight::cColorTypeBack, fBackColor( ) );
			mRealLight->fSetColor( Gfx::tLight::cColorTypeRim, fRimColor( ) );
			mRealLight->fSetColor( Gfx::tLight::cColorTypeAmbient, fAmbientColor( ) );
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

	void tEditableLightEntity::fOnDeviceLost( Gfx::tDevice* device )
	{
		tEditableObject::fOnDeviceLost( device );
	}

	void tEditableLightEntity::fOnDeviceReset( Gfx::tDevice* device )
	{
		tEditableObject::fOnDeviceReset( device );

		mInnerRadiusSphere->fSetRenderBatch( mContainer.fGetSphereCageSparseTemplate().fGetRenderBatch() );
		mOuterRadiusSphere->fSetRenderBatch( mContainer.fGetSphereCageSparseTemplate().fGetRenderBatch() );
		mLightBulb->fSetRenderBatch( mContainer.fGetDummySphereTemplate( ).fGetModifiedRenderBatch( &mShellRenderState ) );
	}

}

