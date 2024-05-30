#include "BasePch.hpp"
#include "tAnimatedLightEntity.hpp"
#include "FX/tFxGraph.hpp"


namespace Sig { namespace FX
{
	//------------------------------------------------------------------------------
	// Def
	//------------------------------------------------------------------------------
	tAnimatedLightDef::tAnimatedLightDef( )
		: tLightEntityDef( )
	{
	}
	tAnimatedLightDef::tAnimatedLightDef( tNoOpTag )
		: tLightEntityDef( cNoOpTag )
		, mBinaryData( cNoOpTag )
	{
	}

	tAnimatedLightDef::~tAnimatedLightDef( )
	{
	}

	void tAnimatedLightDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		tAnimatedLightEntity* entity = NEW_TYPED( tAnimatedLightEntity )( this );

		// This is a weird hybrid because this is how tLightEntityDef applies its info
		entity->mCastsShadow = mCastsShadows;
		entity->mShadowAmount = mShadowIntensity;

		fApplyPropsAndSpawnWithScript( *entity, params );
	}

	//------------------------------------------------------------------------------
	// Data
	//------------------------------------------------------------------------------
	tBinaryAnimatedLightData::tBinaryAnimatedLightData( )
		: mFlags( 0 )
	{

	}

	tBinaryAnimatedLightData::tBinaryAnimatedLightData( tNoOpTag )
		: mGraphs( cNoOpTag )
	{

	}

	tBinaryAnimatedLightData::~tBinaryAnimatedLightData( )
	{

	}

	//------------------------------------------------------------------------------
	// tAnimatedLightEntity
	//------------------------------------------------------------------------------
	tAnimatedLightEntity::tAnimatedLightEntity( )
		: Gfx::tLightEntity( Math::tMat3f::cIdentity )
	{
		fCommonCtor( );
	}

	tAnimatedLightEntity::tAnimatedLightEntity( const Gfx::tLight& lightData )
		: Gfx::tLightEntity( Math::tMat3f::cIdentity, lightData )
	{
		fCommonCtor( );
	}

	tAnimatedLightEntity::tAnimatedLightEntity( const tAnimatedLightDef* def )
		: Gfx::tLightEntity( def->mObjectToLocal )
	{
		fCommonCtor( );
		mData = def->mBinaryData;

		Gfx::tLight lightDesc;
		const f32 startInner = mData.fSampleGraph< tBinaryF32Graph >( &mRandomGenerator, cInnerRadiusGraph, 0.f );
		const f32 startOuter = mData.fSampleGraph< tBinaryF32Graph >( &mRandomGenerator, cOuterRadiusGraph, 0.f );
		lightDesc.fSetTypePoint( Math::tVec2f( startInner, startOuter ) );
		fSetLightDesc( lightDesc );
	}

	void tAnimatedLightEntity::fCommonCtor( )
	{
		mRandomGenerator = tMersenneGenerator( sync_rand( fUInt( ) ) );
		mTime = 0.f;
	}

	tAnimatedLightEntity::~tAnimatedLightEntity( )
	{
	}

	void tAnimatedLightEntity::fUpdateGraphValues( const f32 time )
	{
		mTime = time;
		
		mCurrentColor = mData.fSampleGraph< tBinaryV3Graph >( &mRandomGenerator, cLightColorGraph, mTime );
		mCurrentIntensity = mData.fSampleGraph< tBinaryF32Graph >( &mRandomGenerator, cIntensityGraph, mTime );
		mCurrentInnerRadius = mData.fSampleGraph< tBinaryF32Graph >( &mRandomGenerator, cInnerRadiusGraph, mTime );
		mCurrentOuterRadius = mData.fSampleGraph< tBinaryF32Graph >( &mRandomGenerator, cOuterRadiusGraph, mTime );

		if( mCurrentOuterRadius < mCurrentInnerRadius )
			fSwap( mCurrentInnerRadius, mCurrentOuterRadius );

		// Update the light.
		fSetRadii( Math::tVec2f( mCurrentInnerRadius, mCurrentOuterRadius ) );
		fSetColor( Gfx::tLight::cColorTypeFront, Math::tVec4f( mCurrentColor * mCurrentIntensity, 1.f ) );
	}
}}

