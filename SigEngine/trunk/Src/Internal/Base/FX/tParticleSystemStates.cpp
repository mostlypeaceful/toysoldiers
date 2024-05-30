#include "BasePch.hpp"
#include "tParticleSystemStates.hpp"

namespace Sig
{
namespace FX
{
	//------------------------------------------------------------------------------
	// tBinaryParticleSystemState
	//------------------------------------------------------------------------------
	tBinaryParticleSystemState::tBinaryParticleSystemState( )
	{
	}

	tBinaryParticleSystemState::tBinaryParticleSystemState( tNoOpTag )
		: mEmissionGraphs( cNoOpTag )
		, mPerParticleGraphs( cNoOpTag )
		, mMeshGraphs( cNoOpTag )
		, mAttractorIgnoreIds( cNoOpTag )
	{

	}

	tBinaryParticleSystemState::~tBinaryParticleSystemState( )
	{

	}

	//------------------------------------------------------------------------------
	// tToolParticleSystemState
	//------------------------------------------------------------------------------
	const Math::tVec3f tToolParticleSystemState::mEmissionColours[ cEmissionGraphCount ] =
	{
		Math::tVec3f( 145, 183, 81 ),
		Math::tVec3f( 121, 183, 81 ),
		Math::tVec3f( 97, 183, 81 ),
		Math::tVec3f( 81, 183, 88 ),
		
		Math::tVec3f( 81, 145, 183 ),
		Math::tVec3f( 81, 121, 183 ),
		Math::tVec3f( 81, 97, 183 ),
		
		Math::tVec3f( 183, 81, 145 )
	};	
	const Math::tVec3f tToolParticleSystemState::mParticleColours[ cParticleGraphCount ] =
	{
		Math::tVec3f( 183, 81, 121 ),
		Math::tVec3f( 183, 81, 97 ),
		Math::tVec3f( 183, 88, 81 ),
		Math::tVec3f( 183, 112, 81 ),
		Math::tVec3f( 183, 152, 81 ),
	};	
	const Math::tVec3f tToolParticleSystemState::mMeshColours[ cMeshGraphCount ] =
	{
		Math::tVec3f( 10, 10, 10 ),
		Math::tVec3f( 40, 40, 40 ),
	};	

	const Math::tVec3f tToolParticleSystemState::mEmissionHighlights[ cEmissionGraphCount ] =
	{
		Math::tVec3f( 157, 183, 113 ),
		Math::tVec3f( 140, 183, 113 ),
		Math::tVec3f( 124, 183, 113 ),
		Math::tVec3f( 124, 183, 113),
		
		Math::tVec3f( 113, 157, 183 ),
		Math::tVec3f( 113, 140, 183 ),
		Math::tVec3f( 113, 124, 183 ),
		
		Math::tVec3f( 183, 113, 157 )
	};
	const Math::tVec3f tToolParticleSystemState::mParticleHighlights[ cParticleGraphCount ] =
	{
		Math::tVec3f( 183, 113, 140 ),
		Math::tVec3f( 183, 113, 140 ),
		Math::tVec3f( 183, 113, 124 ),
		Math::tVec3f( 183, 118, 113 ),
		Math::tVec3f( 183, 172, 101 ),
	};			
	const Math::tVec3f tToolParticleSystemState::mMeshHighlights[ cMeshGraphCount ] =
	{
		Math::tVec3f( 30, 30, 30 ),
		Math::tVec3f( 70, 70, 70 ),
	};			
	
	const Math::tVec3f tToolParticleSystemState::mEmissionSelects[ cEmissionGraphCount ] =
	{
		Math::tVec3f( 145, 183, 81 ),
		Math::tVec3f( 121, 183, 81 ),
		Math::tVec3f( 97, 183, 81 ),
		Math::tVec3f( 81, 183, 88 ),
		
		Math::tVec3f( 81, 145, 183 ),
		Math::tVec3f( 81, 121, 183 ),
		Math::tVec3f( 81, 97, 183 ),
		
		Math::tVec3f( 183, 81, 145 )
	};
	const Math::tVec3f tToolParticleSystemState::mParticleSelects[ cParticleGraphCount ] =
	{
		Math::tVec3f( 183, 81, 121 ),
		Math::tVec3f( 183, 81, 97 ),
		Math::tVec3f( 183, 88, 81 ),
		Math::tVec3f( 183, 112, 81 ),
		Math::tVec3f( 183, 152, 81 ),
	};	
	const Math::tVec3f tToolParticleSystemState::mMeshSelects[ cMeshGraphCount ] =
	{
		Math::tVec3f( 120, 120, 120 ),
		Math::tVec3f( 170, 170, 170 ),
	};	

	const tStringPtr tToolParticleSystemState::mParticleSystemFlagStrings[ cParticleSystemFlagsCount ] =
	{
		tStringPtr( "Burst Mode" ),
		tStringPtr( "Velocity Align" ),
		tStringPtr( "Velocity Stretch" ),
		tStringPtr( "Ghost Trails" ),
		tStringPtr( "Keep Y Up" ),
		tStringPtr( "Move Ghost Particles" ),
		tStringPtr( "Emit in Volume" ),
	};

	const tStringPtr tToolParticleSystemState::mEmissionGraphNames[ cEmissionGraphCount ] =
	{
		tStringPtr( "Spawn Rate" ),
		tStringPtr( "Spawn Spin" ),
		tStringPtr( "Spawn Energy" ),
		tStringPtr( "Spawn Size" ),
		tStringPtr( "Translation" ),
		tStringPtr( "Emitter Rotation" ),
		tStringPtr( "Emitter Scale" ),
		tStringPtr( "Particle Lifetime" )
	};
	const tStringPtr tToolParticleSystemState::mPerParticleGraphNames[ cParticleGraphCount ] = 
	{
		tStringPtr( "Particle Size" ),
		tStringPtr( "Particle Spin" ),
		tStringPtr( "Particle Energy" ),
		tStringPtr( "Particle Color" ),
		tStringPtr( "Emitter Cutout" )
	};
	const tStringPtr tToolParticleSystemState::mMeshGraphNames[ cMeshGraphCount ] = 
	{
		tStringPtr( "Mesh Initial Axis" ),
		tStringPtr( "Mesh Axis Delta" ),
	};


	tToolParticleSystemState::tToolParticleSystemState( const tToolParticleSystemState& rhs )
	{
		mSystemFlags = rhs.mSystemFlags;
		mAttractorIgnoreIds = rhs.mAttractorIgnoreIds;
		mEmissionGraphs.fSetCount( rhs.mEmissionGraphs.fCount( ) );
		mPerParticleGraphs.fSetCount( rhs.mPerParticleGraphs.fCount( ) );
		mMeshGraphs.fSetCount( rhs.mMeshGraphs.fCount( ) );
	
		for( u32 i = 0; i < rhs.mEmissionGraphs.fCount( ); ++i )
		{
			u32 id = rhs.mEmissionGraphs[ i ]->fGetID( );
			
			if( id == Rtti::fGetClassId< f32 >( ) )
				mEmissionGraphs[ i ].fReset( NEW tFxGraphF32( rhs.mEmissionGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
				mEmissionGraphs[ i ].fReset( NEW tFxGraphV2f( rhs.mEmissionGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
				mEmissionGraphs[ i ].fReset( NEW tFxGraphV3f( rhs.mEmissionGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
				mEmissionGraphs[ i ].fReset( NEW tFxGraphV4f( rhs.mEmissionGraphs[ i ].fGetRawPtr( ) ) );

			mEmissionGraphs[ i ]->fBuildValues( );
		}

		for( u32 i = 0; i < rhs.mPerParticleGraphs.fCount( ); ++i )
		{
			u32 id = rhs.mPerParticleGraphs[ i ]->fGetID( );
			
			if( id == Rtti::fGetClassId< f32 >( ) )
				mPerParticleGraphs[ i ].fReset( NEW tFxGraphF32( rhs.mPerParticleGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
				mPerParticleGraphs[ i ].fReset( NEW tFxGraphV2f( rhs.mPerParticleGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
				mPerParticleGraphs[ i ].fReset( NEW tFxGraphV3f( rhs.mPerParticleGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
				mPerParticleGraphs[ i ].fReset( NEW tFxGraphV4f( rhs.mPerParticleGraphs[ i ].fGetRawPtr( ) ) );

			mPerParticleGraphs[ i ]->fBuildValues( );
		}

		for( u32 i = 0; i < rhs.mMeshGraphs.fCount( ); ++i )
		{
			u32 id = rhs.mMeshGraphs[ i ]->fGetID( );
			
			if( id == Rtti::fGetClassId< f32 >( ) )
				mMeshGraphs[ i ].fReset( NEW tFxGraphF32( rhs.mMeshGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
				mMeshGraphs[ i ].fReset( NEW tFxGraphV2f( rhs.mMeshGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
				mMeshGraphs[ i ].fReset( NEW tFxGraphV3f( rhs.mMeshGraphs[ i ].fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
				mMeshGraphs[ i ].fReset( NEW tFxGraphV4f( rhs.mMeshGraphs[ i ].fGetRawPtr( ) ) );

			mMeshGraphs[ i ]->fBuildValues( );
		}

		// these lines can be taken out after awhile, but for now we need them ensured.
		mEmissionGraphs[ cEmitterTranslationGraph ]->fSetKeepLastKeyValue( true );
		mEmissionGraphs[ cEmitterScaleGraph ]->fSetKeepLastKeyValue( true );
		mEmissionGraphs[ cRotationGraph ]->fSetKeepLastKeyValue( true );
	}

	tToolParticleSystemState::tToolParticleSystemState( )
	{
		mSystemFlags = 0;

		mEmissionGraphs.fSetCount( cEmissionGraphCount );
		mPerParticleGraphs.fSetCount( cParticleGraphCount );
		mMeshGraphs.fSetCount( cMeshGraphCount );
		
		// EMISSION GRAPHS

		mEmissionGraphs[ cEmissionRateGraph ]		.fReset( NEW tFxGraphF32( ) );
		mEmissionGraphs[ cParticleLifeGraph ]		.fReset( NEW tFxGraphF32( ) );
		mEmissionGraphs[ cSpawnYawGraph ]			.fReset( NEW tFxGraphF32( ) );
		mEmissionGraphs[ cSpawnEnergyGraph ]		.fReset( NEW tFxGraphF32( ) );
		mEmissionGraphs[ cSpawnSizeGraph ]			.fReset( NEW tFxGraphV3f( ) );
		mEmissionGraphs[ cEmitterTranslationGraph ]	.fReset( NEW tFxGraphV3f( ) );
		mEmissionGraphs[ cEmitterScaleGraph ]		.fReset( NEW tFxGraphV3f( ) );
		mEmissionGraphs[ cRotationGraph ]			.fReset( NEW tFxGraphV3f( ) );

		tRandom& r = tRandom::fSubjectiveRand( );

		const b32 doRandom = true;

		const f32 emissionRate0 = doRandom ? r.fFloatInRange( 50, 150 ) : 100.f;
		const f32 particleLife0 = doRandom ? r.fFloatInRange( 0.5f, 2.f ) : 1.f;
		const f32 particleYaw0 = doRandom ? r.fFloatInRange( 0.f, 1.f ) : 1.f;
		const f32 spawnergy0 = doRandom ? r.fFloatInRange( 0.5f, 5.f ) : 1.f;
		const f32 size0 = doRandom ? r.fFloatInRange( 0.5f, 3.f ) : 1.f;

		mEmissionGraphs[ cEmissionRateGraph ]->		 fAddKeyframe( NEW tFxKeyframeF32( 0.f, emissionRate0 ) );
		mEmissionGraphs[ cParticleLifeGraph ]->		 fAddKeyframe( NEW tFxKeyframeF32( 0.f, particleLife0 ) );
		mEmissionGraphs[ cSpawnYawGraph ]->			 fAddKeyframe( NEW tFxKeyframeF32( 0.f, particleYaw0 ) );
		mEmissionGraphs[ cSpawnEnergyGraph ]->		 fAddKeyframe( NEW tFxKeyframeF32( 0.f, spawnergy0 ) );
		mEmissionGraphs[ cSpawnSizeGraph ]->		 fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( size0 ) ) );
		mEmissionGraphs[ cEmitterTranslationGraph ]->fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( 0.f ) ) );
		mEmissionGraphs[ cEmitterScaleGraph ]->		 fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( 1.f ) ) );
		mEmissionGraphs[ cRotationGraph ]->			 fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( 0.f ) ) );

		mEmissionGraphs[ cEmitterTranslationGraph ]->fSetKeepLastKeyValue( true );
		mEmissionGraphs[ cEmitterScaleGraph ]->fSetKeepLastKeyValue( true );
		mEmissionGraphs[ cRotationGraph ]->fSetKeepLastKeyValue( true );

		// PER-PARTICLE GRAPHS

		mPerParticleGraphs[ cScaleGraph ]	.fReset( NEW tFxGraphV3f( ) );
		mPerParticleGraphs[ cSpinGraph ]	.fReset( NEW tFxGraphF32( ) );
		mPerParticleGraphs[ cEnergyGraph ]	.fReset( NEW tFxGraphF32( ) );
		mPerParticleGraphs[ cColorGraph ]	.fReset( NEW tFxGraphV4f( ) );
		mPerParticleGraphs[ cCutoutGraph ]	.fReset( NEW tFxGraphF32( ) );

		mPerParticleGraphs[ cScaleGraph ]->	fAddKeyframe( NEW tFxKeyframeV3f( 0.f, 1.f ) );
		mPerParticleGraphs[ cSpinGraph ]->	fAddKeyframe( NEW tFxKeyframeF32( 0.f, 1.f ) );
		mPerParticleGraphs[ cEnergyGraph ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 1.f ) );
		mPerParticleGraphs[ cCutoutGraph ]->	fAddKeyframe( NEW tFxKeyframeF32( 0.f, 0.f ) );

		const u32 colors = doRandom ? r.fIntInRange( 2, 4 ) : 2;
		for( u32 i = 0; i < colors; ++i )
		{
			const f32 x = ( f32 )i / ( f32 ) ( colors - 1 );
			Math::tVec4f color;
			if( doRandom )
			{
				const f32 invX = 1.f - x;
				color = Math::tVec4f( r.fFloatZeroToOne( ), r.fFloatZeroToOne( ), r.fFloatZeroToOne( ), r.fFloatInRange( invX*.5f, invX ) );
			}
			else
				color = Math::tVec4f( 1.f, 1.f, 1.f, 1.f - i );
			mPerParticleGraphs[ cColorGraph ]->	fAddKeyframe( NEW tFxKeyframeV4f( x, color ) );
		}

		// MESH GRAPHS

		mMeshGraphs[ cMeshInitialAxis ]		.fReset( NEW tFxGraphV3f( ) );
		mMeshGraphs[ cMeshAxisDelta ]		.fReset( NEW tFxGraphV3f( ) );

		mMeshGraphs[ cMeshInitialAxis ]->	fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( 0.f, 1.f, 0.f ) ) );
		mMeshGraphs[ cMeshAxisDelta ]->		fAddKeyframe( NEW tFxKeyframeV3f( 0.f, 0.f ) );
	}

	tToolParticleSystemState::~tToolParticleSystemState( )
	{
		
	}

	tBinaryParticleSystemState tToolParticleSystemState::fCreateBinaryState( ) const
	{
		tBinaryParticleSystemState returnState;

		returnState.mEmissionGraphs.fNewArray( cEmissionGraphCount );
		returnState.mPerParticleGraphs.fNewArray( cParticleGraphCount );
		returnState.mMeshGraphs.fNewArray( cMeshGraphCount );
		returnState.mAttractorIgnoreIds.fNewArray( mAttractorIgnoreIds.fCount( ) );

		for( u32 i = 0; i < mAttractorIgnoreIds.fCount( ); ++i )
			returnState.mAttractorIgnoreIds[ i ] = mAttractorIgnoreIds[ i ];

		for( u32 i = 0; i < cEmissionGraphCount; ++i )
		{
			returnState.mEmissionGraphs[ i ] = fCreateNewGraph( mEmissionGraphs[ i ] );
			returnState.mEmissionGraphs[ i ]->fCopyFromGraph( mEmissionGraphs[ i ] );
		}
		for( u32 i = 0; i < cParticleGraphCount; ++i )
		{
			returnState.mPerParticleGraphs[ i ] = fCreateNewGraph( mPerParticleGraphs[ i ] );
			returnState.mPerParticleGraphs[ i ]->fCopyFromGraph( mPerParticleGraphs[ i ] );
		}
		for( u32 i = 0; i < cMeshGraphCount; ++i )
		{
			returnState.mMeshGraphs[ i ] = fCreateNewGraph( mMeshGraphs[ i ] );
			returnState.mMeshGraphs[ i ]->fCopyFromGraph( mMeshGraphs[ i ] );
		}

		returnState.mSystemFlags = mSystemFlags;

		return returnState;
	}

	namespace
	{
		static void fConvertVec2GraphToVec3Graph( tGraphPtr& oldGraph )
		{
			if( oldGraph->fGetID( ) != Rtti::fGetClassId< Math::tVec2f >( ) )
				return; // already converted

			tGraphPtr newGraph = tGraphPtr( NEW tFxGraphV3f( ) );

			for( u32 i = 0; i < oldGraph->fNumKeyframes( ); ++i )
			{
				const f32 oldTime = oldGraph->mKeyframes[ i ]->fX( );
				const Math::tVec2f oldValue = oldGraph->mKeyframes[ i ]->fValue< Math::tVec2f >( );
				newGraph->fAddKeyframeNoGraphUpdate( NEW tFxKeyframeV3f( oldTime, Math::tVec3f( oldValue.x, oldValue.y, 1.f ) ) );
			}

			newGraph->fSetMinRandomness( oldGraph->fMinRandomness( ) );
			newGraph->fSetMaxRandomness( oldGraph->fMaxRandomness( ) );
			newGraph->fSetKeepLastKeyValue( oldGraph->fKeepLastKeyValue( ) );
			newGraph->fSetUseLerp( oldGraph->fUseLerp( ) );
			newGraph->fSetUseRandoms( oldGraph->fUseRandoms( ) );

			newGraph->fBuildValues( );

			oldGraph = newGraph;
		}

		static void fConvertOldEmitterRotation( tGraphPtr& oldGraph )
		{
			if( oldGraph->fGetID( ) == Rtti::fGetClassId< Math::tVec3f >( ) )
				return; // already converted

			tGraphPtr newGraph = tGraphPtr( NEW tFxGraphV3f( ) );

			for( u32 i = 0; i < oldGraph->fNumKeyframes( ); ++i )
			{
				const f32 oldTime = oldGraph->mKeyframes[ i ]->fX( );
				const Math::tVec4f oldValue = oldGraph->mKeyframes[ i ]->fValue< Math::tVec4f >( );

				Math::tEulerAnglesf ea( Math::tQuatf( oldValue.x, oldValue.y, oldValue.z, oldValue.w ) );

				newGraph->fAddKeyframeNoGraphUpdate( NEW tFxKeyframeV3f( oldTime, Math::tVec3f( ea.x, ea.y, ea.z ) ) );
			}

			newGraph->fSetMinRandomness( oldGraph->fMinRandomness( ) );
			newGraph->fSetMaxRandomness( oldGraph->fMaxRandomness( ) );
			newGraph->fSetKeepLastKeyValue( oldGraph->fKeepLastKeyValue( ) );
			newGraph->fSetUseLerp( oldGraph->fUseLerp( ) );
			newGraph->fSetUseRandoms( oldGraph->fUseRandoms( ) );

			newGraph->fBuildValues( );

			oldGraph = newGraph;
		}
	}

	//------------------------------------------------------------------------------
	// tState (deprecated but still supported)
	//------------------------------------------------------------------------------
	void tState::fFixupAfterXmlLoad( )
	{
		// convert old 2-component scale graphs to 3-component scale graph 
		//		- for quad particle lists, only X and Y are used 
		//		- for mesh particle lists, X,Y,and Z are all used

		sigassert( mToolState );
		fConvertVec2GraphToVec3Graph( mToolState->mPerParticleGraphs[ cScaleGraph ] );
		fConvertVec2GraphToVec3Graph( mToolState->mEmissionGraphs[ cSpawnSizeGraph ] );

		// convert old quat rotation to euler angles rotation
		fConvertOldEmitterRotation( mToolState->mEmissionGraphs[ cRotationGraph ] );

		if( mToolState->mPerParticleGraphs.fCount( ) == cParticleGraphCount-1 )
		{
			mToolState->mPerParticleGraphs.fPushBack( tGraphPtr( NEW tFxGraphF32( ) ) );
			mToolState->mPerParticleGraphs[ cCutoutGraph ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 0.f ) );
		}
	}
}}

