#include "BasePch.hpp"
#include "tParticleSystem.hpp"
#include "tSceneGraph.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "tQuadParticleList.hpp"
#include "tMeshParticleList.hpp"
#include "tGameAppBase.hpp"
#include "tProfiler.hpp"
#include "tSync.hpp"

namespace Sig { namespace FX
{
	namespace
	{
		define_static_function( fEnsureNewParticleCriticalSectionInitialized )
		{
			tParticle::fAllocClassPoolPage( );
			tParticle::fClassPoolCriticalSection( );

			tMeshParticle::fAllocClassPoolPage( );
			tMeshParticle::fClassPoolCriticalSection( );
		}
	}

	tParticleSystemDef::tParticleSystemDef( )
		: mParticleSystemName( 0 )
		, mMeshResource( 0 )
		, mLocalSpace( false )
		, mCameraDepthOffset( 0.f )
		, mUpdateSpeedMultiplier( 1.f )
		, mLodFactor( 1.f )
		, mGhostParticleFrequency( 0.f )
		, mGhostParticleLifetime( 0.f )
		, mSystemFlags( 0 )
		, mEmitterType( cPoint )
		, mSortMode( cNoSort )
		, mPad0( 0 )
		, mPad1( 0 )
		, mRenderState( Gfx::tRenderState::cDefaultColorTransparent )
		, mMaterial( 0 )
	{
	}
	tParticleSystemDef::tParticleSystemDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mStates( cNoOpTag )
		, mAttractorIgnoreIds( cNoOpTag )
		, mRenderState( cNoOpTag )
	{
	}
	tParticleSystemDef::~tParticleSystemDef( )
	{
		delete mMaterial;
	}
	void tParticleSystemDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		tParticleSystem* entity = NEW tParticleSystem( this );
		entity->fMoveTo( mObjectToLocal );
		entity->fSpawn( parent );
	}
	tParticleList* tParticleSystemDef::fCreateParticleList( ) const
	{
		if( mMeshResource && mMeshResource->fGetResourcePtr( ) && mMeshResource->fGetResourcePtr( )->fLoaded( ) )
			return NEW tMeshParticleList( mMeshResource->fGetResourcePtr( ) );
		return NEW tQuadParticleList( );
	}
	void tParticleSystemDef::fAddFromToolState( const tToolParticleSystemState& state )
	{
		mStates.fPushBack( tBinaryParticleSystemState( ) );
		tBinaryParticleSystemState* binaryState = &mStates.fBack( );

		binaryState->mEmissionGraphs.fNewArray( cEmissionGraphCount );
		binaryState->mPerParticleGraphs.fNewArray( cParticleGraphCount );
		binaryState->mMeshGraphs.fNewArray( cMeshGraphCount );
		binaryState->mAttractorIgnoreIds.fNewArray( state.mAttractorIgnoreIds.fCount( ) );

		for( u32 i = 0; i < state.mAttractorIgnoreIds.fCount( ); ++i )
			binaryState->mAttractorIgnoreIds[ i ] = state.mAttractorIgnoreIds[ i ];

		for( u32 i = 0; i < cEmissionGraphCount; ++i )
		{
			binaryState->mEmissionGraphs[ i ] = fCreateNewGraph( state.mEmissionGraphs[ i ] );
			binaryState->mEmissionGraphs[ i ]->fCopyFromGraph( state.mEmissionGraphs[ i ] );
		}
		for( u32 i = 0; i < cParticleGraphCount; ++i )
		{
			binaryState->mPerParticleGraphs[ i ] = fCreateNewGraph( state.mPerParticleGraphs[ i ] );
			binaryState->mPerParticleGraphs[ i ]->fCopyFromGraph( state.mPerParticleGraphs[ i ] );
		}
		for( u32 i = 0; i < cMeshGraphCount; ++i )
		{
			binaryState->mMeshGraphs[ i ] = fCreateNewGraph( state.mMeshGraphs[ i ] );
			binaryState->mMeshGraphs[ i ]->fCopyFromGraph( state.mMeshGraphs[ i ] );
		}

		binaryState->mSystemFlags = state.mSystemFlags;
	}


	u32 tParticleSystem::gTotalParticleSystemCount = 0;
	tParticleSystem::tEmissionReductionFactor tParticleSystem::gEmissionReductionFactor = 0;

	tParticleSystem::tParticleSystem( const Gfx::tMaterialPtr& particleMaterial )
		: mMaterial( particleMaterial )
		, mParticles( NEW tQuadParticleList( ) )
		, mFromBinaryFile( false )
	{
		fCommonCtor( );
		fMakeDefaultParticleSystemState( );
		fSetSortMode( cNoSort, false );
	}

	tParticleSystem::tParticleSystem( const tParticleSystemDef* def )
		: mMaterial( def->mMaterial )
		, mParticles( def->fCreateParticleList( ) )
		, mFromBinaryFile( true )
	{
		fCommonCtor( );

		mParticleSystemName = def->mParticleSystemName->fGetStringPtr( );
		mLocalSpace = def->mLocalSpace;
		fSetCameraDepthOffset( def->mCameraDepthOffset );
		fSetUpdateSpeedMultiplier( def->mUpdateSpeedMultiplier );
		fSetLodFactor( def->mLodFactor );
		fSetGhostParticleFrequency( def->mGhostParticleFrequency );
		fSetGhostParticleLifetime( def->mGhostParticleLifetime );

		FX::tStatePtr state0( NEW FX::tState( &def->mStates[ 0 ] ) );
		mStates.fPushBack( state0 );

		fSetEmitterType( def->mEmitterType );
		fSetRenderState( def->mRenderState );
		fSetSortMode( def->mSortMode, false );
	}
	
	void tParticleSystem::fCommonCtor( )
	{
		++gTotalParticleSystemCount;
		Gfx::tDefaultAllocators& defFxAllocators = Gfx::tDefaultAllocators::fInstance( );
		
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mRenderState.fSetSrcBlendMode( Gfx::tRenderState::cBlendSrcAlpha );
		mRenderState.fSetDstBlendMode( Gfx::tRenderState::cBlendOne );
		mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );

		mParticles->fResetDeviceObjects( defFxAllocators, &mRenderState );

		fSetDisallowIndirectColorControllers( true );

		mLastSpawnPosition = mNewSpawnPosition = Math::tVec3f::cZeroVector;
		mGraphRotation = Math::tQuatf::cIdentity;

		mSeed = sync_rand( fUInt( ) );
		mSystemRand = tMersenneGenerator( mSeed );

		mParticleSystemName = tStringPtr( "" );
		mAlive = true;
		mFinishUp = false;
		mHasEmitted = false;
		mQuickReset = false;
		mLocalSpace = false;
		mStopped = false;
		mLifetime = 5.f;
		mCurrentTime = 0.f;
		mParticlesEmittedPerFrame = 0.f;
		mTimeFromLastEmit = 0.f;
		mUpdateSpeedMultiplier = 1.f;
		mLodFactor = 1.f;
		mGhostParticleFrequency = 60.f;
		mGhostParticleLifetime = 0.5f;	// as a percent of it's parents life.
		mCurrentState = 0;
		mEmissionPercent = 1.f;
		mRequiredLoopCount = -1;	// looper
		mCurrentLoopCounter = 0;
		mBoxMT = Math::tAabbf( -0.001f, +0.001f );
	}

	tParticleSystem::~tParticleSystem( )
	{
		sigassert( gTotalParticleSystemCount > 0 );
		--gTotalParticleSystemCount;

		mParticles->fClear( );

		if( mFromBinaryFile )
			mMaterial.fDisown( );
	}

	void tParticleSystem::fSetLocalSpace( b32 ls )
	{
		mLocalSpace = ls;
		fRI_SetObjectToWorld( mLocalSpace ? &fObjectToWorld( ) : 0 );
	}

	b32 tParticleSystem::fReadyForRemoval( ) const
	{
		if( mRequiredLoopCount > 0 && mCurrentLoopCounter < mRequiredLoopCount )
			return false;

		const u32 particlesLeft = mParticles->fTotalParticleCount( );
		return particlesLeft == 0;
	}

	void tParticleSystem::fRemoveAttractorIgnoreId( u32 id )
	{
		mAttractorIgnoreIDs.fFindAndErase( id );
	}
	
	void tParticleSystem::fAddAttractorIgnoreId( u32 id )
	{
		mAttractorIgnoreIDs.fPushBack( id );
	}

	void tParticleSystem::fSyncAttractorIgnoreIds( )
	{
		mAttractorIgnoreIDs.fSetCount( 0 );
		u32 ignoreCount = fState( ).fAttractorIgnoreListCount( );
		for( u32 i = 0; i < ignoreCount; ++i )
			mAttractorIgnoreIDs.fPushBack( fState( ).fAttractorIgnoreId( i ) );
	}

	void tParticleSystem::fSetAttractors( const tGrowableArray< tParticleAttractorPtr >& list )
	{
		mAttractorsList.fSetCount( 0 );
		if( !mAttractorIgnoreIDs.fCapacity( ) )
			fSyncAttractorIgnoreIds( );

		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			const u32 id = list[ i ]->fId( );
			b32 ignore = false;
			for( u32 j = 0; j < mAttractorIgnoreIDs.fCount( ) && !ignore; ++j )
			{
				const u32 ignoreID = mAttractorIgnoreIDs[ j ];
				if( id == ignoreID )
					ignore = true;
			}

			if( !ignore )
				mAttractorsList.fPushBack( list[ i ] );
		}
	}

	void tParticleSystem::fSetSortMode( tParticleSortMode mode, b32 forceUpdate )
	{
		mSortMode = mode;
	}

	void tParticleSystem::fSetBlendOp( u32 blendOp )
	{
		mRenderState.fEnableDisable( Gfx::tRenderState::cBlendOpMask, false );
		mRenderState.fEnableDisable( blendOp, true );
	}
	void tParticleSystem::fSetBlendOpFromIndex( u32 blendOpIndex )
	{
		mRenderState.fSetBlendOpFromIndex( blendOpIndex );
	}
	void tParticleSystem::fSetSrcBlend( u32 srcBlend )
	{
		mRenderState.fSetSrcBlendMode( ( Gfx::tRenderState::tBlendMode )srcBlend );
	}
	void tParticleSystem::fSetDstBlend( u32 dstBlend )
	{
		mRenderState.fSetDstBlendMode( ( Gfx::tRenderState::tBlendMode )dstBlend );
	}
	
	void tParticleSystem::fQuickReset( )
	{
		fStop( false );
		mQuickReset = true;
		mAlive = true;
		mFinishUp = false;
		mHasEmitted = false;
		mCurrentTime = 0.f;
		mParticlesEmittedPerFrame = 0.f;
		mCurrentLoopCounter = 0;
		mGraphRotation = Math::tQuatf::cIdentity;
	}

	void tParticleSystem::fStop( b32 stop )
	{
		mStopped = stop;
	}

	void tParticleSystem::fClear( )
	{
		mParticles->fClear( );

		//mVertexData.fClear( );

		mCurrentTime = 0.f;
		mParticlesEmittedPerFrame = 0.f;
		mHasEmitted = false;
		mCurrentLoopCounter = 0;
		mGraphRotation = Math::tQuatf::cIdentity;

		Math::tAabbf box( Math::tSpheref( .25f ) );
		fSetObjectSpaceBox( box );
	}

	void tParticleSystem::fClearAllStates( )
	{
		for( u32 i = 0; i < mStates.fCount( ); ++i )
			mStates[ i ].fRelease( );
		mStates.fSetCount( 0 );
	}

	void tParticleSystem::fSetCurrentTime( const f32 time )
	{
		mCurrentTime = time;
		if( mCurrentTime > mLifetime )
			mCurrentTime = 0.f;
	}
	
	void tParticleSystem::fFastUpdate( f32 curlife, b32 fromTheStart )
	{
		Math::tAabbf box( Math::tSpheref( .25f ) );
		mHasEmitted = false;

		if( curlife >= mCurrentTime && !fromTheStart ) // going forward in time....
		{
			const f32 dt = ( curlife - mCurrentTime );
			const f32 delta = fClamp( mCurrentTime / mLifetime, 0.f, 1.f );
			for( u32 i = 0; i < mAttractorsList.fCount( ); ++i )
				mAttractorsList[ i ]->fUpdateGraphValues( delta );

			mParticles->fFastUpdate( dt );

			fMoveST( dt );
			fSystemUpdate( dt, fObjectToWorld( ), fWorldToObject( ), box );
		}
		else // time-traveling backwards.
		{
			fClear( );
			mSystemRand = tMersenneGenerator( mSeed );
			mEmitter->fReset( );
			mAlive = true;
			mFinishUp = false;
			//fCalculateStatistics( );

			f32 updateTime = curlife;	//fMax( curlife, mStatistics.mLongestLivingParticle );
			if( updateTime > 0.f )
			{
				f32 updateSteps = 30.f;
				const f32 dt = curlife / (updateSteps-1.f);

				for( f32 f = 0.f; f <= updateTime; f += dt )
				{
					const f32 delta = mCurrentTime / mLifetime;
					
					for( u32 i = 0; i < mAttractorsList.fCount( ); ++i )
						mAttractorsList[ i ]->fUpdateGraphValues( delta );

					// do a quick check to try and delete particles that won't be around at our
					// desired time
					mParticles->fRemoveParticlesYoungerThan( curlife - f );

					fMoveST( dt );	
					fSystemUpdate( dt, fObjectToWorld( ), fWorldToObject( ), box );
				}
			}
			else	// then give us just one quick update from zero
			{
				fMoveST( 0.f );	
				fSystemUpdate( 0.f, fObjectToWorld( ), fWorldToObject( ), box );
			}
		}

		mCurrentTime = curlife;
		fSetObjectSpaceBox( box );
		mParticles->fRefreshParentRenderable( *this, false );
	}

	Math::tVec3f tParticleSystem::fGetEmitterScale( ) const
	{
		if( mEmitter )
			return mEmitter->fScale( );
		return Math::tVec3f::cZeroVector;
	}

	Math::tVec3f tParticleSystem::fEmitterTranslation( ) const
	{
		if( mEmitter )
			return mEmitter->fTranslation( );
		return Math::tVec3f::cZeroVector;
	}

	void tParticleSystem::fSetEmitterType( tEmitterType type )
	{
		mEmitterType = type;

		switch( mEmitterType )
		{
		case cPoint:		mEmitter.fReset( NEW tPointEmitter( mSystemRand ) );		break;
		case cSphere:		mEmitter.fReset( NEW tSphereEmitter( mSystemRand ) );		break;
		case cBox:			mEmitter.fReset( NEW tBoxEmitter( mSystemRand ) );			break;
		case cFountain:		mEmitter.fReset( NEW tFountainEmitter( mSystemRand ) );		break;
		case cShockwave:	mEmitter.fReset( NEW tShockwaveEmitter( mSystemRand ) );	break;
		case cCylinder:		mEmitter.fReset( NEW tCylinderEmitter( mSystemRand ) );		break;
		default:
			sigassert( !"No valid emitter type was set!" );
			break;
		}

		if( mEmitter )
		{
			const f32 delta = mCurrentTime / mLifetime;
			Math::tVec3f emitterScale = fState( ).fSampleEmissionGraph< Math::tVec3f >( &mSystemRand, cEmitterScaleGraph, delta );
			mEmitter->fSetScale( emitterScale );
		}
	}

	void tParticleSystem::fCalculateStatistics( )
	{
		mStatistics.mMaxParticles = 0.f;
		mStatistics.mLongestLivingParticle = 0.f;
	}

	void tParticleSystem::fMakeDefaultParticleSystemState( )
	{
		FX::tStatePtr state0( NEW FX::tState( NEW tToolParticleSystemState( ) ) );
		mStates.fPushBack( state0 );

		//fCalculateStatistics( );
		fSetEmitterType( cPoint );
	}


	void tParticleSystem::fOnSpawn( )
	{
		fOnPause( mStopped || fSceneGraph( )->fIsPaused( ) );
		Gfx::tRenderableEntity::fOnSpawn( );
	}

	void tParticleSystem::fOnPause( b32 paused )
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
		tRenderableEntity::fOnPause( paused );
	}

	void tParticleSystem::fMoveST( f32 dt )
	{
		profile( cProfilePerfParticlesMoveST );

		if( mStopped )
			return;

		if( mQuickReset && mCurrentTime > mLifetime )
			return;

		const f32 delta = fClamp( ( mCurrentTime / mLifetime ), 0.f, 1.f );
		fComputeEmitCountThisFrame( dt*mUpdateSpeedMultiplier, fState( ), delta );

		sync_event_v_c( mEmitCountThisFrame, tSync::cSCParticles );
		mParticles->fAllocateGeometry( *this, mEmitCountThisFrame, mMaterial.fGetRawPtr( ) );
	}
	void tParticleSystem::fEffectsMT( f32 dt )
	{
		profile( cProfilePerfParticlesEffectsMT );

		mBoxMT = Math::tAabbf( -0.001f, +0.001f );
		if( mStopped )
			return;
		if( mQuickReset && mCurrentTime > mLifetime )
			return;

		Math::tMat3f xform = fObjectToWorld( );
		if( fState( ).fHasFlag( cKeepYUp ) )
			xform.fYAxis( Math::tVec3f::cYAxis );

		fSystemUpdate( dt, xform, fWorldToObject( ), mBoxMT );
	}
	void tParticleSystem::fThinkST( f32 dt )
	{
		profile( cProfilePerfParticlesThinkST );
		mParticles->fSyncST( dt, *this, mLocalSpace );
		mParticles->fRefreshParentRenderable( *this, true );
		fSetObjectSpaceBox( mBoxMT );
	}
	void tParticleSystem::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
	}

	void tParticleSystem::fSystemUpdate( f32 dt, const Math::tMat3f& objToWorld, const Math::tMat3f& objToWorldInv, Math::tAabbf& box )
	{
		mStatistics.fFrameReset( );
		mCurrentTime += dt;
		dt *= mUpdateSpeedMultiplier;	// we do this after updating mCurrentTime by the real frame-dt so when we sample system-emission graphs we aren't out-of-sync with the tFxSystem.

		if( mCurrentTime > mLifetime )
		{
			++mCurrentLoopCounter;
			if( mRequiredLoopCount >= 0 && mCurrentLoopCounter >= mRequiredLoopCount )
				mAlive = false;
			else
				mAlive = true;
			mCurrentTime = ( mCurrentTime - mLifetime );
			mGraphRotation = Math::tQuatf::cIdentity;
		}

		const f32 delta = fClamp( ( mCurrentTime / mLifetime ), 0.f, 1.f );
		const Math::tVec4f rotation = fState( ).fSampleEmissionGraph< Math::tVec4f >( &mSystemRand, cRotationGraph, delta );
		if( rotation.fIsZero( ) )
		{
			mGraphRotation = Math::tQuatf::cIdentity;
		}
		else
		{
			const Math::tVec3f rotationNorm = Math::tVec3f( rotation.x, rotation.y, rotation.z ).fNormalizeSafe( Math::tVec3f::cZeroVector );
			mGraphRotation = Math::tQuatf( Math::tAxisAnglef( rotationNorm, ( rotation.w * dt ) ) );
		}

		const Math::tVec3f emitterScale = fState( ).fSampleEmissionGraph< Math::tVec3f >( &mSystemRand, cEmitterScaleGraph, delta );
		const Math::tVec3f translation = fState( ).fSampleEmissionGraph< Math::tVec3f >( &mSystemRand, cEmitterTranslationGraph, delta );

		mEmitter->fSetParentSystemDelta( delta );
		mEmitter->fSetScale( emitterScale );
		mEmitter->fSetTranslation( translation );
		
		mLastSpawnPosition = mNewSpawnPosition;
		mNewSpawnPosition = objToWorld.fGetTranslation( ) + translation;
		if( mParticles->fNormalParticleCount( ) == 0 )
			mLastSpawnPosition = mNewSpawnPosition;

		box.fInvalidate( );
		fUpdateAllParticles( dt, delta, objToWorld, objToWorldInv, box );

		sync_event_v_c( box, tSync::cSCParticles );
	}

	void tParticleSystem::fUpdateAllParticles( f32 dt, f32 systemDelta, const Math::tMat3f& objToWorld, const Math::tMat3f& objToWorldInv, Math::tAabbf& box )
	{
		tParticleListUpdateParams updateParams;
		updateParams.mDt = dt;
		updateParams.mSystemDelta = systemDelta;
		updateParams.mLocalSpace = mLocalSpace;
		updateParams.mMoveGhostParticles = fHasFlag( cMoveGhostParticles );
		updateParams.mEmitInVolume = fHasFlag( cVolumeEmit );
		updateParams.mObjToWorld = objToWorld;
		updateParams.mObjToWorldInv = objToWorldInv;
		updateParams.mEmitter = mEmitter.fGetRawPtr( );
		updateParams.mState = &fState( );
		updateParams.mSystemRand = &mSystemRand;
		updateParams.mAttractorsList = &mAttractorsList;
		updateParams.mBoxToUpdate = &box;

		updateParams.mPositionalDelta = Math::tVec3f::cZeroVector;
		if( !mLocalSpace )
			updateParams.mPositionalDelta = objToWorldInv.fXformVector( mLastSpawnPosition - mNewSpawnPosition );

		mParticles->fEmitParticles( updateParams, mEmitCountThisFrame );
		mParticles->fSort( mSortMode );
		mParticles->fUpdateParticles( updateParams );

		mTimeFromLastEmit += dt;
		if( fState( ).fHasFlag( cGhostImage ) && mTimeFromLastEmit > (1.f/mGhostParticleFrequency) )
		{
			mParticles->fEmitGhostParticles( mGhostParticleLifetime, mGhostParticleFrequency, mTimeFromLastEmit );
			mTimeFromLastEmit = 0.f;
		}
	}

	namespace
	{
		//Notes on current usage of mLodFactor: In the editor, a value of 1.0 for System LOD Factor will result in a particle system that
		//reduces it's particles at the current rate set by 'gEmissionReductionFactor', while a LOD Factor of 0.1 is a particle system that
		//tries not to reduce it's particle count...changes by rgaule.
		static f32 fComputeEmissionReduction( tParticleSystem& ps, tParticleSystem::tEmissionReductionFactor reducer )
		{
#ifdef target_game
			const f32 reduce = reducer ? reducer( ps ) : 1.f;
			const f32 bestFadeAlpha = ps.fComputeFadeAlpha( *tGameAppBase::fInstance( ).fScreen( ) );	
			return fMin( 1.f, ( ( 1.f - ps.fLodFactor( ) ) + ( reduce * bestFadeAlpha ) ) );
#else
			return 1.f;
#endif//target_game
		}
	}

	void tParticleSystem::fComputeEmitCountThisFrame( f32 dt, const FX::tState& state, const f32 systemDelta )
	{
		const f32 extraEmissionReduce = fComputeEmissionReduction( *this, gEmissionReductionFactor );

		mEmitCountThisFrame = 0;

		if( mAlive && ( !mFinishUp || !mHasEmitted || mCurrentLoopCounter < mRequiredLoopCount ) )
		{
			// sample emission rate graph
			f32 emitRate = state.fSampleEmissionGraph< f32 >( &mSystemRand, cEmissionRateGraph, systemDelta ) * mEmissionPercent;
			
			if( emitRate > 0.f )
			{
				mHasEmitted = true;
				if( !state.fHasFlag( cBurstMode ) )
				{
					emitRate *= extraEmissionReduce * dt;
				}
				else
				{
					mAlive = false;
					if( emitRate > 5 )
						emitRate *= extraEmissionReduce;
				}

				mParticlesEmittedPerFrame += emitRate;

				mEmitCountThisFrame = ( u32 )mParticlesEmittedPerFrame;
				mParticlesEmittedPerFrame -= mEmitCountThisFrame;
			}

			if( mFinishUp )
				mEmitCountThisFrame = 0;
		}
	}

	void tParticleSystem::fChangeMaterial( const Gfx::tMaterialPtr& mtl )
	{
		if( mFromBinaryFile )
			mMaterial.fDisown( );
		mMaterial = mtl;
		mParticles->fChangeMaterial( *this, mMaterial.fGetRawPtr( ) );
	}
	void tParticleSystem::fChangeParticleList( tParticleList* newList )
	{
		fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
		mParticles.fReset( newList );
		mParticles->fResetDeviceObjects( Gfx::tDefaultAllocators::fInstance( ), &mRenderState );
		mParticles->fChangeMaterial( *this, mMaterial.fGetRawPtr( ) );
	}
}}

