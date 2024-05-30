#include "ToolsPch.hpp"
#include "tSigFxLight.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "FX/tAnimatedLightEntity.hpp"
#include "tWxSlapOnColorPicker.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// Editor-side entity
	//------------------------------------------------------------------------------
	tSigFxLight::tSigFxLight( tEditableObjectContainer& container )
		: tEditableLightEntity( container )
	{
		mToolData.fReset( NEW_TYPED( tToolAnimatedLightData )() );
		fCommonCtor( *container.fGetResourceDepot( ) );
	}

	tSigFxLight::tSigFxLight( tEditableObjectContainer& container, const Fxml::tFxLightObject& fxl )
		: tEditableLightEntity( container, fxl )
	{
		//fDeserializeBaseObject( &fxl );
		mToolData.fReset( fxl.mData.fGetRawPtr( ) );
		fCommonCtor( *container.fGetResourceDepot( ) ); // TODO: data on fxl
	}

	void tSigFxLight::fClone( const tSigFxLight& light )
	{
		mToolData.fReset( NEW_TYPED( tToolAnimatedLightData )( light.mToolData.fGetRawPtr() ) );

		// Create tAnimatedLight's data
		fCreateAnimatedLightData( light.mToolData );

		std::string cloneName = light.fGetName( );
		cloneName += " Clone";
		fSetEditableProperty( std::string( cloneName ), Sigml::tObject::fEditablePropObjectName() );
	}

	void tSigFxLight::fCommonCtor( tResourceDepot& resDep )
	{
		// Create animated light with old light desc
		FX::tAnimatedLightEntity* newLight = NEW_TYPED( FX::tAnimatedLightEntity )( mRealLight->fLightDesc( ) );

		// Destroy the old light, assign new light
		fRemoveRealLight( );
		mRealLight.fReset( newLight );

		// Create tAnimatedLight's data
		fCreateAnimatedLightData( mToolData );

		newLight->fUpdateGraphValues( 0.f );
		newLight->fSpawnImmediate( *this );

		mLastOpenGraphIdx = 0;
		mExternallyDriven = true;
	}

	FX::tAnimatedLightEntity* tSigFxLight::fRealAnimatedLight( ) const
	{
		return mRealLight ? mRealLight->fStaticCast< FX::tAnimatedLightEntity >( ) : NULL;
	}

	void tSigFxLight::fOnSpawn( )
	{
		fOnPause( false );
		tEditableLightEntity::fOnSpawn( );
	}

	void tSigFxLight::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListThinkST );
		}
		else
		{
			fRunListInsert( cRunListThinkST );
		}
		tEditableLightEntity::fOnPause( paused );
	}

	void tSigFxLight::fThinkST( f32 dt )
	{
		FX::tAnimatedLightEntity* light = fRealAnimatedLight( );

		// Check for changes to the graph shapes and update the built versions on the light if necessary.
		for( u32 i = 0; i < mToolData->mGraphs.fCount( ); ++i )
		{
			if( !mToolData->mGraphs[ i ]->fGetDirty( ) )
				continue;

			light->fGetData( ).mGraphs[ i ]->fCopyFromGraph( mToolData->mGraphs[ i ] );

			mToolData->mGraphs[ i ]->fClean( );
		}

		// This only drives the shell visualizations
		Math::tMat3f lil( Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, Math::cPiOver4 ) ) );
		lil.fScaleLocal( light->fCurrentInnerRadius( ) );
		mInnerRadiusSphere->fSetParentRelativeXform( lil );

		const Math::tMat3f big( light->fCurrentOuterRadius( ) );
		mOuterRadiusSphere->fSetParentRelativeXform( big );
	}

	void tSigFxLight::fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max )
	{
		tEditableObject::fSetLocalSpaceMinMax( min, max );
		mDummySphere->fSetObjectSpaceBox( fObjectSpaceBox( ) );
		mDummySphere->fSetParentRelativeXform( fObjectSpaceBox( ).fAdjustMatrix( Math::tMat3f::cIdentity, 0.25f ) );
	}

	void tSigFxLight::fSetCastsShadows( b32 casts )
	{
		mRealLight->fSetCastsShadow( casts );
		fSetEditableProperty( casts, fEditablePropCastShadows( ) );
	}

	b32 tSigFxLight::fCastsShadows( ) const
	{
		return mEditableProperties.fGetValue( fEditablePropCastShadows( ), false );
	}


	void tSigFxLight::fSetShadowIntensity( f32 intensity )
	{
		const f32 clamptensity = fClamp( intensity, 0.f, 1.f );
		mRealLight->mShadowAmount = clamptensity;
		fSetEditableProperty( clamptensity, fEditablePropIntensity( ) );
	}

	f32 tSigFxLight::fShadowIntensity( ) const
	{
		return mEditableProperties.fGetValue( fEditablePropIntensity( ), 0.2f );
	}

	Sigml::tObjectPtr tSigFxLight::fSerialize( b32 clone ) const
	{
		Fxml::tFxLightObject* fxl = new Fxml::tFxLightObject( );
		fSerializeBaseObject( fxl, clone );
		fxl->mData.fReset( mToolData.fGetRawPtr( ) );
		return Sigml::tObjectPtr( fxl );
	}

	void tSigFxLight::fCreateAnimatedLightData( const tToolAnimatedLightDataPtr& toolData )
	{
		fRealAnimatedLight( )->fSetData( toolData->fCreateBinaryData() );
	}
}

