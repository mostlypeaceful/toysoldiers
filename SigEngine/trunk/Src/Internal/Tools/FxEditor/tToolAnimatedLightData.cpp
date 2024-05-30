#include "ToolsPch.hpp"
#include "tToolAnimatedLightData.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// Tool data
	//------------------------------------------------------------------------------
	const Math::tVec3f tToolAnimatedLightData::mColors[ FX::cLightGraphCount ] =
	{
		Math::tVec3f ( 128, 128, 156 ),
		Math::tVec3f( 128, 156, 128 ),
		Math::tVec3f( 156, 128, 128 ),
		Math::tVec3f( 156, 128, 156 ),
		Math::tVec3f( 156, 156, 128 ),
		//Math::tVec3f( 128, 156, 156 ),
	};

	const tStringPtr tToolAnimatedLightData::mGraphNames[ FX::cLightGraphCount ] =
	{
		tStringPtr( "Color" ),
		tStringPtr( "Intensity" ),
		tStringPtr( "Inner Radius" ),
		tStringPtr( "Outer Radius" ),
		tStringPtr( "Translation" ),
	};


	const tStringPtr tToolAnimatedLightData::mFlagNames[ FX::cLightFlagCount ] =
	{
		tStringPtr( "Do not use" ),
	};

	tToolAnimatedLightData::tToolAnimatedLightData( tToolAnimatedLightData* rhs )
	{
		mFlags = rhs->mFlags;

		mGraphs.fSetCount( FX::cLightGraphCount );

		mGraphs[ FX::cLightColorGraph ].fReset( NEW_TYPED( FX::tFxGraphV3f )( rhs->mGraphs[ FX::cLightColorGraph ].fGetRawPtr( ) ) );
		mGraphs[ FX::cIntensityGraph ].fReset( NEW_TYPED( FX::tFxGraphF32 )( rhs->mGraphs[ FX::cIntensityGraph ].fGetRawPtr( ) ) );
		mGraphs[ FX::cInnerRadiusGraph ].fReset( NEW_TYPED( FX::tFxGraphF32 )( rhs->mGraphs[ FX::cInnerRadiusGraph ].fGetRawPtr( ) ) );
		mGraphs[ FX::cOuterRadiusGraph ].fReset( NEW_TYPED( FX::tFxGraphF32 )( rhs->mGraphs[ FX::cOuterRadiusGraph ].fGetRawPtr( ) ) );
		mGraphs[ FX::cTranslationGraph ].fReset( NEW_TYPED( FX::tFxGraphV3f )( rhs->mGraphs[ FX::cTranslationGraph ].fGetRawPtr( ) ) );

		fUpdate( );
	}

	tToolAnimatedLightData::tToolAnimatedLightData( )
	{
		mFlags = 0;

		mGraphs.fSetCount( FX::cLightGraphCount );

		mGraphs[ FX::cLightColorGraph ].fReset( NEW_TYPED( FX::tFxGraphV3f )() );
		mGraphs[ FX::cIntensityGraph ].fReset( NEW_TYPED( FX::tFxGraphF32 )() );
		mGraphs[ FX::cInnerRadiusGraph ].fReset( NEW_TYPED( FX::tFxGraphF32 )() );
		mGraphs[ FX::cOuterRadiusGraph ].fReset( NEW_TYPED( FX::tFxGraphF32 )() );
		mGraphs[ FX::cTranslationGraph ].fReset( NEW_TYPED( FX::tFxGraphV3f )() );

		mGraphs[ FX::cLightColorGraph ]->fAddKeyframe( NEW_TYPED( FX::tFxKeyframeV3f )( 0.f, Math::tVec3f( 1.f ) ) );
		mGraphs[ FX::cIntensityGraph ]->fAddKeyframe( NEW_TYPED( FX::tFxKeyframeF32 )( 0.f, 1.f ) );		
		mGraphs[ FX::cInnerRadiusGraph ]->fAddKeyframe( NEW_TYPED( FX::tFxKeyframeF32 )( 0.f, 10.f ) );
		mGraphs[ FX::cOuterRadiusGraph ]->fAddKeyframe( NEW_TYPED( FX::tFxKeyframeF32 )( 0.f, 15.5f ) );
		mGraphs[ FX::cTranslationGraph ]->fAddKeyframe( NEW_TYPED( FX::tFxKeyframeV3f )( 0.f, Math::tVec3f( 0.f ) ) );
	}

	tToolAnimatedLightData::~tToolAnimatedLightData( )
	{

	}

	void tToolAnimatedLightData::fUpdate( )
	{
		for( u32 i = 0; i < FX::cLightGraphCount; ++i )
			mGraphs[ i ]->fUpdate( );
	}

	FX::tBinaryAnimatedLightData tToolAnimatedLightData::fCreateBinaryData( ) const
	{
		FX::tBinaryAnimatedLightData data;

		data.mGraphs.fNewArray( FX::cLightGraphCount );

		// Create graphs, these will be updated later by fBuildValues
		for( u32 i = 0; i < FX::cLightGraphCount; ++i )
		{
			data.mGraphs[ i ] = FX::fCreateNewGraph( mGraphs[ i ] );
			data.mGraphs[ i ]->fCopyFromGraph( mGraphs[ i ] );
		}

		return data;
	}
}

