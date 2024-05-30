#include "BasePch.hpp"
#include "tParticleAttractor.hpp"
#include "tSceneGraph.hpp"
#include "tMeshSystem.hpp"
#include "tMeshParticleList.hpp"

namespace Sig
{
namespace FX
{

	tParticleAttractorDef::tParticleAttractorDef( )
	{

	}
	tParticleAttractorDef::tParticleAttractorDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mBinaryData( cNoOpTag )
	{

	}
	tParticleAttractorDef::~tParticleAttractorDef( )
	{
		delete mBinaryData;
	}

	void tParticleAttractorDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		tParticleAttractor* entity = NEW tParticleAttractor( this );
		entity->fMoveTo( mObjectToLocal );
		entity->fSpawn( parent );
	}

	void tParticleAttractorDef::fFromToolData( const tToolAttractorData* data )
	{
		mBinaryData = NEW tBinaryAttractorData( );
		mBinaryData->mAffectParticlesDirection = data->mAffectParticlesDirection;
		mBinaryData->mParticleMustBeInRadius = data->mParticleMustBeInRadius;
		mBinaryData->mType = data->mType;
		mBinaryData->mId = data->mId;
		mBinaryData->mFlags = data->mFlags;

		mBinaryData->mGraphs.fNewArray( cAttractorGraphCount );
		for( u32 i = 0; i < cAttractorGraphCount; ++i )
		{
			mBinaryData->mGraphs[ i ] = fCreateNewGraph( data->mGraphs[ i ] );
			mBinaryData->mGraphs[ i ]->fCopyFromGraph( data->mGraphs[ i ] );
		}
	}

	const Math::tVec3f tToolAttractorData::mColours[ cAttractorGraphCount ] =
	{
		Math::tVec3f ( 128, 128, 156 ),
		Math::tVec3f( 128, 156, 128 ),
		Math::tVec3f( 156, 128, 128 ),
		Math::tVec3f( 156, 128, 156 ),
		Math::tVec3f( 156, 156, 128 ),
		Math::tVec3f( 128, 156, 156 ),
	};

	const tStringPtr tToolAttractorData::mGraphNames[ cAttractorGraphCount ] =
	{
		tStringPtr( "Mass" ),
		tStringPtr( "Scale" ),
		tStringPtr( "Gravity" ),
		tStringPtr( "Position" ),
		tStringPtr( "Particle Color" ),
		tStringPtr( "Particle Size" ),
	};


	const tStringPtr tToolAttractorData::mAttractorFlagNames[ cAttractorFlagCount ] =
	{
		tStringPtr( "Use World Gravity Direction" ),
	};

	tToolAttractorData::tToolAttractorData( tToolAttractorData* rhs, u32 id )
	{
		mType = rhs->mType;
		mId = id;
		mFlags = rhs->mFlags;

		mParticleMustBeInRadius = rhs->mParticleMustBeInRadius;
		mAffectParticlesDirection = rhs->mAffectParticlesDirection;

		mGraphs.fSetCount( cAttractorGraphCount );

		mGraphs[ cAttractorMassGraph ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cAttractorMassGraph ].fGetRawPtr( ) ) );
		mGraphs[ cAttractorScaleGraph ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cAttractorScaleGraph ].fGetRawPtr( ) ) );
		mGraphs[ cAttractorGravityGraph ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cAttractorGravityGraph ].fGetRawPtr( ) ) );
		mGraphs[ cAttractorPositionGraph ].fReset( NEW tFxGraphV3f( rhs->mGraphs[ cAttractorPositionGraph ].fGetRawPtr( ) ) );
		mGraphs[ cAttractorParticleColorGraph ].fReset( NEW tFxGraphV4f( rhs->mGraphs[ cAttractorParticleColorGraph ].fGetRawPtr( ) ) );
		mGraphs[ cAttractorParticleSizeGraph ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cAttractorParticleSizeGraph ].fGetRawPtr( ) ) );

		mGraphs[ cAttractorPositionGraph ]->fSetKeepLastKeyValue( true );

		fUpdate( );
	}

	tToolAttractorData::tToolAttractorData( )
	{
		mType = cAttract;
		mParticleMustBeInRadius = false;
		mAffectParticlesDirection = true;
		mFlags = 0;
		
		mGraphs.fSetCount( cAttractorGraphCount );

		mGraphs[ cAttractorMassGraph ].fReset( NEW tFxGraphF32( ) );
		mGraphs[ cAttractorScaleGraph ].fReset( NEW tFxGraphF32( ) );
		mGraphs[ cAttractorGravityGraph ].fReset( NEW tFxGraphF32( ) );
		mGraphs[ cAttractorPositionGraph ].fReset( NEW tFxGraphV3f( ) );
		mGraphs[ cAttractorParticleColorGraph ].fReset( NEW tFxGraphV4f(  ) );
		mGraphs[ cAttractorParticleSizeGraph ].fReset( NEW tFxGraphF32( ) );

		mGraphs[ cAttractorPositionGraph ]->fSetKeepLastKeyValue( true );

		mGraphs[ cAttractorMassGraph ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 10.f ) );		
		mGraphs[ cAttractorScaleGraph ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 1.f ) );
		mGraphs[ cAttractorGravityGraph ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 9.8f ) );
		mGraphs[ cAttractorPositionGraph ]->fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( 0.f ) ) );
		mGraphs[ cAttractorParticleColorGraph ]->fAddKeyframe( NEW tFxKeyframeV4f( 0.f, Math::tVec4f( 0.f ) ) );
		mGraphs[ cAttractorParticleSizeGraph ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 0.f ) );
	}

	tToolAttractorData::~tToolAttractorData( )
	{
		
	}

	void tToolAttractorData::fUpdate( )
	{
		for( u32 i = 0; i < cAttractorGraphCount; ++i )
			mGraphs[ i ]->fUpdate( );
	}

	tBinaryAttractorData::tBinaryAttractorData( )
	{

	}

	tBinaryAttractorData::tBinaryAttractorData( tNoOpTag )
		: mGraphs( cNoOpTag )
	{

	}

	tBinaryAttractorData::~tBinaryAttractorData( )
	{
		
	}

	tParticleAttractor::tParticleAttractor( )
	{
		fCommonCtor( );
		mData.fReset( NEW Attractors::tData( NEW tToolAttractorData( ), Attractors::tNullBinaryData ) );
		mAttractorName = tStringPtr( "" );
	}

	tParticleAttractor::tParticleAttractor( const tParticleAttractorDef* def )
	{
		fCommonCtor( );
		mData.fReset( NEW Attractors::tData( Attractors::tNullToolData, def->mBinaryData ) );
		mAttractorName = def->mAttractorName->fGetStringPtr( );
	}

	void tParticleAttractor::fCommonCtor( )
	{
		mRandomGenerator = tMersenneGenerator( sync_rand( fUInt( ) ) );
		mCurrentColor = Math::tVec4f::cOnesVector;
		mCurrentMass = 1.f;
		mCurrentScale = 1.f;
		mCurrentGravity = 1.f;
		mCurrentParticleSizeScale = 1.f;
		mScaleSquared = 1.f;
		mDelta = 0.f;
		mCurrentPosition = Math::tVec3f::cZeroVector;
		mLastPosition = Math::tVec3f::cZeroVector;
	}

	tParticleAttractor::~tParticleAttractor( )
	{
		
	}

	void tParticleAttractor::fActOnOldMeshParticle( f32 dt, tOldMeshParticle* meshParticle, b32 localSpace, const Math::tMat3f& systemXform, const Math::tMat3f& invSystemXform ) const
	{
		const Math::tVec3f attractorPos = localSpace ? invSystemXform.fXformPoint( fWorldPosition( ) ) : fWorldPosition( );
		const Math::tVec3f ppos = meshParticle->fObjectToWorld( ).fGetTranslation( );
		const Math::tVec3f vTo = ( ppos - attractorPos );
		f32 lenSq = fMax( mScaleSquared, vTo.fLengthSquared( ) );
		
		if( mData->fParticleMustBeInRadius( ) )
		{
			if( lenSq > mScaleSquared )
				return;
		}

		const f32 lenSqInv = 1.f / lenSq;
		Math::tVec3f force( Math::tVec3f::cZeroVector );

		switch( mData->fForceType( ) )
		{
		case cRepel:
			{
				force += vTo;
				force.fNormalizeSafe( Math::tVec3f::cZeroVector );
				force *= ( mCurrentGravity * mCurrentMass ) * lenSqInv;
			}
			break;
		case cAttract:
			{
				force -= vTo;
				force.fNormalizeSafe( Math::tVec3f::cZeroVector );
				force *= ( mCurrentGravity * mCurrentMass ) * lenSqInv;
			}
			break;
		case cGravity:
			{
				const Math::tVec3f gravityDir = fHasFlag( cUseWorldGravityDirection ) ? -Math::tVec3f::cYAxis : -fObjectToWorld( ).fYAxis( );
				force = gravityDir * mCurrentGravity;
			}
		default:
			break;
		}

		meshParticle->mDirection += force * systemXform.fGetScale( ) * dt;
	}

	void tParticleAttractor::fActOnParticle( f32 dt, tParticle* particle, b32 localSpace, const Math::tMat3f& systemXform, const Math::tMat3f& invSystemXform ) const
	{
		fActOnParticleInternal( dt, particle, localSpace, systemXform, invSystemXform );
	}
	f32 tParticleAttractor::fActOnParticleInternal( f32 dt, tParticle* particle, b32 localSpace, const Math::tMat3f& systemXform, const Math::tMat3f& invSystemXform ) const
	{
		Math::tVec3f force = Math::tVec3f::cZeroVector;
		tForceType forceType = mData->fForceType( );
		if( forceType == cGravity )
		{
			const Math::tVec3f gravityDir = fHasFlag( cUseWorldGravityDirection ) ? -Math::tVec3f::cYAxis : -fObjectToWorld( ).fYAxis( );
			force = gravityDir * mCurrentGravity;
		}
		else
		{
			const Math::tVec3f attractorPos = localSpace ? invSystemXform.fXformPoint( fWorldPosition( ) ) : fWorldPosition( );
			const Math::tVec3f ppos = particle->mPosition;
			const Math::tVec3f pdir = particle->mDirection;
			const Math::tVec3f vTo = ( ppos - attractorPos );
			f32 lenSq = fMax( mScaleSquared, vTo.fLengthSquared( ) );
			
			if( mData->fParticleMustBeInRadius( ) )
			{
				if( lenSq > mScaleSquared )
					return lenSq;
			}

			const f32 lenSqInv = 1.f / lenSq;
		
			switch( forceType )
			{
			case cRepel:
				{
					force += vTo;
					force.fNormalizeSafe( Math::tVec3f::cZeroVector );
					force *= ( mCurrentGravity * mCurrentMass ) * lenSqInv;
				}
				break;
			case cAttract:
				{
					force -= vTo;
					force.fNormalizeSafe( Math::tVec3f::cZeroVector );
					force *= ( mCurrentGravity * mCurrentMass ) * lenSqInv;
				}
				break;
			case cPlaneCollide:
				{
					const Math::tVec3f gravityDir = fHasFlag( cUseWorldGravityDirection ) ? -Math::tVec3f::cYAxis : -fObjectToWorld( ).fYAxis( );
					const f32 dist = Math::tPlanef( gravityDir, attractorPos ).fSignedDistance( ppos );
					if( dist < 0.f )
					{
						particle->mPosition -= dist * gravityDir;
						if( pdir.fDot( gravityDir ) < 0.f )
							particle->mDirection = Math::fReflect( pdir, gravityDir );
					}
				}
				break;
			default:
				break;
			}
		}

		if( !localSpace )
			force *= systemXform.fGetScale( );

		if( mData->fAffectParticlesDirection( ) )
			particle->mDirection += force * dt;
		else
			particle->mPosition += force * dt;

		//particle->mCurrentColor += ( mCurrentColor * lenSqInv );		// we never use this...i'm disabling it!
		//particle->mScale0 += mCurrentParticleSizeScale * lenSqInv * dt;

		//return lenSq;
		return 0.0f;
	}
	void tParticleAttractor::fActOnMeshParticle( f32 dt, tMeshParticle* particle, b32 localSpace, const Math::tMat3f& systemXform, const Math::tMat3f& invSystemXform ) const
	{
		fActOnParticleInternal( dt, particle, localSpace, systemXform, invSystemXform );

		//const f32 lenSq = fActOnParticleInternal( dt, particle, localSpace, systemXform, invSystemXform );
		//if( lenSq > 0.f )
		//	particle->mScale0z += mCurrentParticleSizeScale * (1.f/lenSq) * dt;
	}

	void tParticleAttractor::fUpdateGraphValues( const f32 delta )
	{
		mDelta = delta;
		mLastPosition = mCurrentPosition;
		mCurrentMass = mData->fSampleGraph< f32 >( &mRandomGenerator, cAttractorMassGraph, mDelta );
		mCurrentScale = mData->fSampleGraph< f32 >( &mRandomGenerator, cAttractorScaleGraph, mDelta );
		mCurrentGravity = mData->fSampleGraph< f32 >( &mRandomGenerator, cAttractorGravityGraph, mDelta );
		mCurrentPosition = mData->fSampleGraph< Math::tVec3f >( &mRandomGenerator, cAttractorPositionGraph, mDelta );
		//disabled!!!//mCurrentColor = mData->fSampleGraph< Math::tVec4f >( &mRandomGenerator, cAttractorParticleColorGraph, mDelta );
		mCurrentParticleSizeScale = mData->fSampleGraph< f32 >( &mRandomGenerator, cAttractorParticleSizeGraph, mDelta );
		mScaleSquared = mCurrentScale*mCurrentScale;

		if( mCurrentScale < 0.f )
			mCurrentScale = 0.f;
	}

	//devvar( bool, Debug_Effects_DrawAttractors, false );

	//void tParticleAttractor::fOnTick( tRunListName runList, f32 dt )
	//{
	//	if( Debug_Effects_DrawAttractors )
	//	{
	//		fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tSpheref( fGetWorldPosition( ), mCurrentScale ), Math::tVec4f( 1.f, 0.65f, 0.1f, 0.6f ) );
	//	}
	//}

}
}

