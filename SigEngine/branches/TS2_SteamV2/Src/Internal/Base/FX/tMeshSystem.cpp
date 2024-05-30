#include "BasePch.hpp"
#include "tMeshSystem.hpp"
#include "tSceneGraph.hpp"
#include "tMesh.hpp"
#include "tSceneGraphFile.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tGeometryFile.hpp"
#include "tProfiler.hpp"
#include "tSync.hpp"

namespace Sig
{
namespace FX
{

	tMeshSystemDef::tMeshSystemDef( )
	{

	}
	tMeshSystemDef::tMeshSystemDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mBinaryData( cNoOpTag )
	{

	}
	tMeshSystemDef::~tMeshSystemDef( )
	{
		delete mBinaryData;
	}

	void tMeshSystemDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		tMeshSystem* entity = NEW tMeshSystem( this );
		entity->fMoveTo( mObjectToLocal );
		entity->fSpawn( parent );
	}

	void tMeshSystemDef::fFromToolData( const tToolFxMeshSystemData* data )
	{
		mBinaryData = NEW tBinaryFxMeshSystemData( );
		mBinaryData->mSystemFlags = data->mSystemFlags;
		mBinaryData->mEmitterType = data->mEmitterType;
		
		mBinaryData->mGraphs.fNewArray( cFxMeshSystemGraphCount );
		for( u32 i = 0; i < cFxMeshSystemGraphCount; ++i )
		{
			mBinaryData->mGraphs[ i ] = fCreateNewGraph( data->mGraphs[ i ] );
			mBinaryData->mGraphs[ i ]->fCopyFromGraph( data->mGraphs[ i ] );
		}

		mBinaryData->mAttractorIgnoreIds.fNewArray( data->mAttractorIgnoreIds.fCount( ) );
		for( u32 i = 0; i < mBinaryData->mAttractorIgnoreIds.fCount( ); ++i )
			mBinaryData->mAttractorIgnoreIds[ i ] = data->mAttractorIgnoreIds[ i ];
	}

	const Math::tVec3f tToolFxMeshSystemData::mColours[ cFxMeshSystemGraphCount ] =
	{
		Math::tVec3f ( 128, 128, 156 ),
		Math::tVec3f( 128, 156, 128 ),
		Math::tVec3f( 156, 128, 128 ),
		Math::tVec3f( 156, 128, 156 ),
		Math::tVec3f( 156, 156, 128 ),
		Math::tVec3f( 128, 156, 156 ),
		Math::tVec3f( 81, 145, 183 ),
		Math::tVec3f( 81, 121, 183 ),
		Math::tVec3f( 81, 97, 183 ),
	};

	const tStringPtr tToolFxMeshSystemData::mGraphNames[ cFxMeshSystemGraphCount ] =
	{
		tStringPtr( "Spawn Rate" ),
		tStringPtr( "Spawn Size" ),
		tStringPtr( "Spawn Velocity" ),
		tStringPtr( "Emitter Scale" ),
		tStringPtr( "Mesh Life" ),
		tStringPtr( "Mesh Scale" ),
		tStringPtr( "Mesh Velocity" ),
		tStringPtr( "Mesh Spin" ),
		tStringPtr( "Mesh Tint" ),
	};
	
	const tStringPtr tToolFxMeshSystemData::mSystemFlagStrings[ cMeshSystemFlagsCount ] =
	{
		tStringPtr( "Burst Emit" ),
		tStringPtr( "Collide With Ground" ),
		tStringPtr( "Random Orientation" ),
	};

	tToolFxMeshSystemData::tToolFxMeshSystemData( tToolFxMeshSystemData* rhs )
	{
		mSystemFlags = rhs->mSystemFlags;
		mEmitterType = rhs->mEmitterType;
		mAttractorIgnoreIds = rhs->mAttractorIgnoreIds;
		
		mGraphs.fSetCount( cFxMeshSystemGraphCount );

		mGraphs[ cSpawnRate ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cSpawnRate ].fGetRawPtr( ) ) );
		mGraphs[ cSpawnSize ].fReset( NEW tFxGraphV3f( rhs->mGraphs[ cSpawnSize ].fGetRawPtr( ) ) );
		mGraphs[ cSpawnSpeed ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cSpawnSpeed ].fGetRawPtr( ) ) );
		mGraphs[ cEmitterScale ].fReset( NEW tFxGraphV3f( rhs->mGraphs[ cEmitterScale ].fGetRawPtr( ) ) );
		mGraphs[ cMeshLife ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cMeshLife ].fGetRawPtr( ) ) );
		mGraphs[ cMeshScale ].fReset( NEW tFxGraphV3f( rhs->mGraphs[ cMeshScale ].fGetRawPtr( ) ) );
		mGraphs[ cMeshSpeedScale ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cMeshSpeedScale ].fGetRawPtr( ) ) );
		mGraphs[ cMeshSpinRate ].fReset( NEW tFxGraphF32( rhs->mGraphs[ cMeshSpinRate ].fGetRawPtr( ) ) );
		mGraphs[ cMeshTint ].fReset( NEW tFxGraphV4f( rhs->mGraphs[ cMeshTint ].fGetRawPtr( ) ) );
		
		fUpdate( );
	}

	tToolFxMeshSystemData::tToolFxMeshSystemData( )
	{
		mSystemFlags = 0;
		mEmitterType = cSphere;
		
		mGraphs.fSetCount( cFxMeshSystemGraphCount );
		mAttractorIgnoreIds.fSetCount( 0 );

		mGraphs[ cSpawnRate ].fReset( NEW tFxGraphF32( ) );
		mGraphs[ cSpawnSize ].fReset( NEW tFxGraphV3f( ) );
		mGraphs[ cSpawnSpeed ].fReset( NEW tFxGraphF32( ) );
		mGraphs[ cEmitterScale ].fReset( NEW tFxGraphV3f( ) );
		mGraphs[ cMeshLife ].fReset( NEW tFxGraphF32( ) );
		mGraphs[ cMeshScale ].fReset( NEW tFxGraphV3f( ) );
		mGraphs[ cMeshSpeedScale ].fReset( NEW tFxGraphF32( ) );
		mGraphs[ cMeshSpinRate ].fReset( NEW tFxGraphF32( ) );
		mGraphs[ cMeshTint ].fReset( NEW tFxGraphV4f( ) );
		mGraphs[ cSpawnRate ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 25.f ) );
		mGraphs[ cSpawnSize ]->fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( 1.f, 1.f, 1.f ) ) );
		mGraphs[ cSpawnSpeed ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 5.f ) );
		mGraphs[ cEmitterScale ]->fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( 1.f, 1.f, 1.f ) ) );
		mGraphs[ cMeshLife ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 1.f ) );
		mGraphs[ cMeshScale ]->fAddKeyframe( NEW tFxKeyframeV3f( 0.f, Math::tVec3f( 1.f, 1.f, 1.f ) ) );
		mGraphs[ cMeshSpeedScale ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 1.f ) );
		mGraphs[ cMeshSpinRate ]->fAddKeyframe( NEW tFxKeyframeF32( 0.f, 1.f ) );
		mGraphs[ cMeshTint ]->fAddKeyframe( NEW tFxKeyframeV4f( 0.f, Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) ) );
		mGraphs[ cMeshTint ]->fAddKeyframe( NEW tFxKeyframeV4f( 1.f, Math::tVec4f( 1.f, 1.f, 1.f, 0.f ) ) );
	}

	tToolFxMeshSystemData::~tToolFxMeshSystemData( )
	{
		
	}

	void tToolFxMeshSystemData::fUpdate( )
	{
		for( u32 i = 0; i < cFxMeshSystemGraphCount; ++i )
			mGraphs[ i ]->fUpdate( );
	}

	tBinaryFxMeshSystemData::tBinaryFxMeshSystemData( )
	{

	}

	tBinaryFxMeshSystemData::tBinaryFxMeshSystemData( tNoOpTag )
		: mGraphs( cNoOpTag ), mAttractorIgnoreIds( cNoOpTag )
	{

	}

	tBinaryFxMeshSystemData::~tBinaryFxMeshSystemData( )
	{
		
	}

	tMeshSystem::tMeshSystem( )
		: mLifetime( 5.f )
		, mCurrentTime( 0.f )
		, mEmissionRatePerFrame( 0.f )
		, mEmissionPercent( 1.f )
		, mStopped( false )
		, mFinishUp( false )
		, mQuickReset( false )
		, mRandomGenerator( sync_rand( fUInt( ) ) )
	{
		mOnLoadComplete.fFromMethod< tMeshSystem, &tMeshSystem::fOnLoadComplete >( this );
		mData.fReset( NEW FxMeshSystem::tData( NEW tToolFxMeshSystemData( ), FxMeshSystem::tNullBinaryData ) );
		mFxMeshSystemName = tStringPtr( "" );
		mParticleSystemToSyncWith = tStringPtr( "" );
		mMeshResourceFile = tFilePathPtr( "_tools\\SigFx\\cube.mshml" );
	}

	tMeshSystem::tMeshSystem( const tMeshSystemDef* def )
		: mLifetime( 5.f )
		, mCurrentTime( 0.f )
		, mEmissionRatePerFrame( 0.f )
		, mEmissionPercent( 1.f )
		, mStopped( false )
		, mFinishUp( false )
		, mQuickReset( false )
		, mRandomGenerator( sync_rand( fUInt( ) ) )
	{
		mOnLoadComplete.fFromMethod< tMeshSystem, &tMeshSystem::fOnLoadComplete >( this );
		mData.fReset( NEW FxMeshSystem::tData( FxMeshSystem::tNullToolData, def->mBinaryData ) );
		mFxMeshSystemName = def->mFxMeshSystemName->fGetStringPtr( );
		mParticleSystemToSyncWith = def->mParticleSystemToSyncWith->fGetStringPtr( );
		mMeshResourceFile = tFilePathPtr( def->mMeshResourceFile->fGetStringPtr( ).fCStr( ) );
	}

	b32 tMeshSystem::fReadyForRemoval( ) const
	{
		return mMeshes.fCount( ) == 0;
	}

	tOldMeshParticle::tOldMeshParticle( tEntity& parent, const tResourcePtr& res )
		: tSceneRefEntity( res )
		, mXform( Math::tMat3f::cIdentity )
	{
		fSetLockedToParent( false );
		fSpawn( parent );
		fCollectEntities( tEntityCreationFlags( ) );
	}

	tMeshSystem::~tMeshSystem( )
	{
		fClearAllMeshes( );

		if( mMeshResource )
			mMeshResource->fUnload( this );
		mMeshResource.fRelease( );
		mEmitter.fRelease( );
	}

	void tMeshSystem::fClearAllMeshes( )
	{
		for( u32 i = 0; i < mMeshes.fCount( ); ++i )
			mMeshes[ i ]->fDelete( );
		mMeshes.fSetCount( 0 );
	}

	void tMeshSystem::fOnSpawn( )
	{
		fOnPause( fSceneGraph( )->fIsPaused( ) );
		
		fSetMeshResourceFile( mMeshResourceFile );

		mRandomSeed = sync_rand( fUInt( ) );
		mRandomGenerator = tMersenneGenerator( mRandomSeed );

		fSetEmitterType( mData->fEmitterType( ) );
		tEntity::fOnSpawn( );
	}
	
	void tMeshSystem::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListEffectsMT );
			fRunListRemove( cRunListThinkST );
		}
		else
		{
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListEffectsMT );
			fRunListInsert( cRunListThinkST );
		}
		tEntity::fOnPause( paused );
	}

	void tMeshSystem::fSetMeshResourceFile( const tFilePathPtr& meshFile, b32 nameOnly )
	{
		mMeshResourceFile = meshFile; 
		if( nameOnly )
			return;
		Gfx::tDefaultAllocators& defFxAllocators = Gfx::tDefaultAllocators::fInstance( );

		if( mMeshResource )
			mMeshResource->fUnload( this );
		mMeshResource = defFxAllocators.mResourceDepot->fQuery( tResourceId::fMake< tSceneGraphFile >( mMeshResourceFile ) );
		mMeshResource->fLoadDefault( this );
		mMeshResource->fCallWhenLoaded( mOnLoadComplete );
	}

	void tMeshSystem::fOnLoadComplete( tResource& resource, b32 success )
	{
		for( u32 i = 0; i < mMeshes.fCount( ); ++i )
			mMeshes[ i ]->fDelete( );
		for( u32 i = 0; i < mMeshRemoval.fCount( ); ++i )
			mMeshRemoval[ i ]->fDelete( );
		mMeshes.fSetCount( 0 );
		mMeshRemoval.fSetCount( 0 );
	}

	void tMeshSystem::fSetRandomSeed( u32 seed )
	{
		mRandomSeed = seed;
		mRandomGenerator = tMersenneGenerator( mRandomSeed );
	}

	void tMeshSystem::fSetEmitterType( const tEmitterType type )
	{
		mData->fSetEmitterType( type );
		switch( mData->fEmitterType( ) )
		{
		case cPoint:		mEmitter.fReset( NEW tPointEmitter( mRandomGenerator ) );		break;
		case cSphere:		mEmitter.fReset( NEW tSphereEmitter( mRandomGenerator ) );		break;
		case cBox:			mEmitter.fReset( NEW tBoxEmitter( mRandomGenerator ) );			break;
		case cFountain:		mEmitter.fReset( NEW tFountainEmitter( mRandomGenerator ) );	break;
		case cShockwave:	mEmitter.fReset( NEW tShockwaveEmitter( mRandomGenerator ) );	break;
		default:
			sigassert( !"No valid emitter type was set!" );
			break;
		}
	}

	void tMeshSystem::fMoveST( f32 dt )
	{
		profile( cProfilePerfParticlesMeshesMoveST );

		if( mStopped )
			return;

		if( !mQuickReset || mCurrentTime <= mLifetime )
		{
			mCurrentTime += dt;
			if( mCurrentTime > mLifetime )
			{
				mCurrentTime = ( mCurrentTime - mLifetime );
			}

			fEmitNewMeshes( dt );
		}
	}

	void tMeshSystem::fEffectsMT( f32 dt )
	{
		profile( cProfilePerfParticlesMeshesEffectsMT );

		if( mStopped )
			return;

		fSystemUpdate( dt );
	}

	void tMeshSystem::fThinkST( f32 dt )
	{
		fUpdateMeshXforms( );
		for( u32 i = 0; i < mMeshRemoval.fCount( ); ++i )
			mMeshRemoval[ i ]->fDelete( );
		mMeshRemoval.fSetCount( 0 );
	}

	void tMeshSystem::fFastUpdate( f32 curTime, b32 fromTheStart )
	{
		if( curTime >= mCurrentTime )
		{
			f32 dt = curTime - mCurrentTime;
			mCurrentTime = curTime;
			
			fEmitNewMeshes( dt );
			fSystemUpdate( dt );
			fUpdateMeshXforms( );
			fSceneGraph( )->fAdvanceTime( dt );
		}
		else
		{
			fClearAllMeshes( );
			mRandomGenerator = tMersenneGenerator( mRandomSeed );
			mCurrentTime = 0.f;
			mEmissionRatePerFrame = 0.f;
			const f32 dt = curTime / 30.f;
			for( f32 f = 0.f; f < curTime; f += dt )
			{
				mCurrentTime += dt;
				fEmitNewMeshes( dt );
				fSystemUpdate( dt );
				fUpdateMeshXforms( );
				fSceneGraph( )->fAdvanceTime( dt );
			}
		}
	}

	void tMeshSystem::fQuickReset( )
	{
		fStop( false );
		mQuickReset = true;
		mCurrentTime = 0.f;
		mFinishUp = false;

		// This value is set to 1.f for ts2. Machine guns call fQuickReset so fast
		//  that mEmissionRatePerFrame never accumulates to 1.0f;
		//  this will force one mesh at least to be spawned after a quick reset.
		//  Without needing a super high emission count.
		//  Ideal the tools would support something like "spawn one mesh"
		//  Ie Fast emission rate, but an emission cap of 1 mesh.
		//  The current solution is a slow emission rate, short effect life, and setting this to 1.f on reset
		// -Matt Kincaid
		mEmissionRatePerFrame = 1.f;
	}

	void tMeshSystem::fUpdateMeshXforms( )
	{
		for( u32 i = 0; i < mMeshes.fCount( ); ++i )
			mMeshes[ i ]->fMoveTo( mMeshes[ i ]->mXform );
	}

	void tMeshSystem::fEmitNewMeshes( f32 dt )
	{
		const f32 delta = fSaturate( mCurrentTime / mLifetime );
		const f32 emitRate = mData->fSampleGraph< f32 >( &mRandomGenerator, cSpawnRate, delta ) * dt;
		mEmissionRatePerFrame += emitRate;
		const u32 emitCount = ( u32 ) (mEmissionRatePerFrame * mEmissionPercent);
		mEmissionRatePerFrame -= emitCount;

		if( !mMeshResource || !mMeshResource->fLoaded( ) )
			return;

		const Math::tMat3f& xform = fObjectToWorld( );
		const Math::tVec3f systemXformScale = xform.fGetScale( );

		for( u32 iemit = 0; iemit < emitCount; ++iemit )
		{
			tOldMeshParticle* p = NEW tOldMeshParticle( *this, mMeshResource );
			p->mMaxLife = mData->fSampleGraph< f32 >( &mRandomGenerator, cMeshLife, delta );
			p->mCurLife = 0.f;
			const f32 initialVelocity = mData->fSampleGraph< f32 >( &mRandomGenerator, cSpawnSpeed, delta );
			p->mInitialScale = mData->fSampleGraph< Math::tVec3f >( &mRandomGenerator, cSpawnSize, delta );
			Math::tVec3f emitPos = Math::tVec3f::cZeroVector;
			mEmitter->fEmit( p->mDirection, emitPos, false );
			p->mDirection = xform.fXformVector( p->mDirection ).fNormalize( );	// Take any of the parent's xform rotation into account
			emitPos = xform.fXformPoint( emitPos );
			
			Math::tVec3f axis = p->mDirection;
			if( fHasFlag( cRandomOrientation ) )
				axis = mRandomGenerator.fNormalizedVec3( );

			p->mDirection *= initialVelocity;
			p->mXform.fOrientZAxis( axis, axis.fCross( Math::tVec3f::cXAxis ) );
			p->mXform.fScaleLocal( systemXformScale );
			p->mXform.fSetTranslation( emitPos + p->mDirection * dt * mRandomGenerator.fFloatZeroToOne( ) );// One update to get us out of a 'normalized' look
			mMeshes.fPushBack( p );
		}
	}

	void tMeshSystem::fSystemUpdate( f32 dt )
	{
		mCurrentTime += dt;

		const f32 systemDelta = fClamp( ( mCurrentTime / mLifetime ), 0.f, 1.f );
		std::sort( mMeshes.fBegin( ), mMeshes.fEnd( ), MeshSystem::tLastBornSort( ) );

		const Math::tMat3f& xform = fObjectToWorld( );
		const Math::tMat3f& invXform = fWorldToObject( );
		const Math::tVec3f systemXformScale = xform.fGetScale( );

		// Dont inherit the particle systems scale
		Math::tVec3f sizeScale = Math::tVec3f::cOnesVector;		
		if( fParent( ) && fParent( )->fParent( ) )
			sizeScale = fParent( )->fParent( )->fObjectToWorld( ).fGetScale( );

		const Math::tVec3f emitterScale = mData->fSampleGraph< Math::tVec3f >( &mRandomGenerator, cEmitterScale, systemDelta );
		mEmitter->fSetScale( emitterScale );

		for( u32 a = 0; a < mAttractorsList.fCount( ); ++a )
			mAttractorsList[ a ]->fUpdateGraphValues( systemDelta );

		for( u32 i = 0; i < mMeshes.fCount( ); )
		{
			tOldMeshParticle* mesh = mMeshes[ i ];
			mesh->mCurLife += dt;
			f32 delta = mesh->mCurLife / mesh->mMaxLife;
			if( delta > 1.f )
			{
				mMeshRemoval.fPushBack( mesh );
				mMeshes.fErase( i );
				continue;
			}

			Gfx::tRenderableEntity::fSetRgbaTint( *mesh, mData->fSampleGraph< Math::tVec4f >( &mRandomGenerator, cMeshTint, delta ) );
			const f32 velocityMultiplier = mData->fSampleGraph< f32 >( &mRandomGenerator, cMeshSpeedScale, delta );
			Math::tVec3f scale = mesh->mInitialScale * mData->fSampleGraph< Math::tVec3f >( &mRandomGenerator, cMeshScale, delta );
			const f32 spinAmount = mData->fSampleGraph< f32 >( &mRandomGenerator, cMeshSpinRate, delta );

			Math::tMat3f meshxform = mesh->mXform;
			Math::tVec3f pos = meshxform.fGetTranslation( );

			if( fHasFlag( cCollideWithGround ) && pos.y < 0.f )
				mesh->mDirection.y *= -1.f;
			
			for( u32 a = 0; a < mAttractorsList.fCount( ); ++a )
				mAttractorsList[ a ]->fActOnOldMeshParticle( dt, mesh, false, xform, invXform );

			Math::tVec3f zAxis = ( meshxform.fZAxis( ) + ( meshxform.fYAxis( ) * spinAmount * dt ) ).fNormalizeSafe( Math::tVec3f::cZAxis );
			mesh->mXform.fOrientZAxis( zAxis, zAxis.fCross( Math::tVec3f::cXAxis ) );
			mesh->mXform.fScaleLocal( sizeScale * scale );
			mesh->mXform.fSetTranslation( pos + ( mesh->mDirection * systemXformScale * velocityMultiplier * dt ) );
			++i;
		}
	}

	void tMeshSystem::fSetAttractors( const tGrowableArray< tParticleAttractorPtr >& list )
	{
		mAttractorsList.fSetCount( 0 );
		//for( u32 i = 0; i < list.fCount( ); ++i )
		//	mAttractorsList.fPushBack( list[ i ] );

		/*
		if( !mAttractorIgnoreIDs.fCapacity( ) )
			fSyncAttractorIgnoreIds( );
		*/

		u32 ignoreCount = mData->fAttractorIgnoreListCount( );
		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			u32 id = list[ i ]->fId( );

			b32 ignore( false );
			for( u32 j = 0; j < ignoreCount && !ignore; ++j )
			{
				u32 ignoreID = mData->fAttractorIgnoreId( j );
				if( id == ignoreID )
					ignore = true;
			}

			if( !ignore )
				mAttractorsList.fPushBack( list[ i ] );
		}
	}

}
}

