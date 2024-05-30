#include "BasePch.hpp"
#include "tMeshParticleList.hpp"
#include "tEmitters.hpp"
#include "tParticleSystemStates.hpp"
#include "tParticleAttractor.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "Gfx/tDefaultAllocators.hpp"

namespace Sig { namespace FX
{
	void tMeshParticle::fSyncMesh( f32 dt, tEntity& parent, const tResourcePtr& res, b32 localSpace )
	{
		if( !mMesh )
		{
			mMesh = NEW tSceneRefEntity( res );
			mMesh->fSetLockedToParent( localSpace );
			mMesh->fSpawn( parent );
			mMesh->fCollectEntities( tEntityCreationFlags( ) );
			Gfx::tRenderableEntity::fSetUseEffectSpatialSet( *mMesh, true );
		}

		const Math::tVec3f zAxis = Math::tQuatf( Math::tAxisAnglef( mYAxis, mSpin ) ).fRotate( mZAxis );

		Math::tMat3f xform;
		xform.fYAxis( mYAxis );
		xform.fXAxis( mYAxis.fCross( zAxis ).fNormalizeSafe( Math::tVec3f::cXAxis ) );
		xform.fZAxis( xform.fXAxis( ).fCross( mYAxis ).fNormalizeSafe( Math::tVec3f::cZAxis ) );
		mZAxis = xform.fZAxis( );
		xform.fScaleLocal( mScale );
		xform.fSetTranslation( mPosition );
		mMesh->fMoveTo( xform );

		mYAxis += xform.fXformVector( mYAxisDelta ) * dt;
		mYAxis.fNormalizeSafe( Math::tVec3f::cYAxis );

		Gfx::tRenderableEntity::fSetRgbaTint( *mMesh, mCurrentColor );
	}

	tSceneRefEntity* tMeshParticle::fDisownMesh( )
	{
		tSceneRefEntity* o = mMesh;
		mMesh = 0;
		return o;
	}
}}

namespace Sig { namespace FX
{
	tMeshParticleList::tMeshParticleList( const tResourcePtr& meshResource )
		: mMeshResource( meshResource )
		, mFirstNewParticleThisFrame( 0 )
		, mFirstNewGhostParticleThisFrame( 0 )
	{
	}
	tMeshParticleList::~tMeshParticleList( )
	{
		fClear( );
	}
	void tMeshParticleList::fResetDeviceObjects( Gfx::tDefaultAllocators& allocators, const Gfx::tRenderState* override )
	{
	}
	void tMeshParticleList::fChangeMaterial( Gfx::tRenderableEntity& parent, Gfx::tMaterial* material )
	{
	}
	void tMeshParticleList::fAllocateGeometry( Gfx::tRenderableEntity& parent, u32 emitCountThisFrame, Gfx::tMaterial* material )
	{
		const u32 numNormalParticles = mParticles.fCount( ) + emitCountThisFrame;
		//const u32 totalNumParticles = numNormalParticles + mGhostParticles.fCount( );

		mParticles.fSetCapacity( numNormalParticles );
	}
	void tMeshParticleList::fRefreshParentRenderable( Gfx::tRenderableEntity& parent, b32 unlockBuffers )
	{
	}
	void tMeshParticleList::fClear( )
	{
		for( u32 i = 0; i < mParticles.fCount( ); ++i )
		{
			mMeshesToDelete.fPushBack( mParticles[ i ]->fDisownMesh( ) );
			delete mParticles[ i ];
		}
		for( u32 i = 0; i < mGhostParticles.fCount( ); ++i )
		{
			mMeshesToDelete.fPushBack( mGhostParticles[ i ]->fDisownMesh( ) );
			delete mGhostParticles[ i ];
		}
		mParticles.fSetCount( 0 );
		mGhostParticles.fSetCount( 0 );
		fPurgeMeshesToDelete( );
	}
	void tMeshParticleList::fFastUpdate( f32 dt )
	{
		for( u32 i = 0; i < mParticles.fCount( ); )
		{
			tMeshParticle* p = mParticles[ i ];
			if( ( p->mCurrentLife + dt ) > p->mLifetime )
			{
				mParticles.fErase( i );
				mMeshesToDelete.fPushBack( p->fDisownMesh( ) );
				delete p;
				continue;
			}
			++i;
		}
	}
	void tMeshParticleList::fRemoveParticlesYoungerThan( f32 time )
	{
		for( u32 i = 0; i < mParticles.fCount( ); )
		{
			tMeshParticle* p = mParticles[ i ];
			if( p->mLifetime < time )
			{
				mParticles.fErase( i );
				mMeshesToDelete.fPushBack( p->fDisownMesh( ) );
				delete p;
				continue;
			}
			++i;
		}
	}
	void tMeshParticleList::fSort( tParticleSortMode sortMode )
	{
		// no purpose in sorting, as each mesh will get sorted during the render phase due to transparency
	}
	void tMeshParticleList::fEmitParticles( tParticleListUpdateParams& updateParams, u32 emitCount )
	{
		const f32 dt = updateParams.mDt;
		const f32 systemDelta = updateParams.mSystemDelta;
		const FX::tBinaryParticleSystemState& state = *updateParams.mState;

		Math::tMat3f boxXform = Math::tMat3f::cIdentity;
		if( !updateParams.mLocalSpace )
			boxXform = updateParams.mObjToWorldInv;

		const Math::tVec3f xformScale = updateParams.mObjToWorld.fGetScale( );

		mFirstNewParticleThisFrame = mParticles.fCount( );

		for( u32 iemit = 0; iemit < emitCount; ++iemit )
		{
			tMeshParticle *p = NEW tMeshParticle( );

			p->mLifetime = fMax( 0.001f, state.fSampleEmissionGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cParticleLifeGraph, systemDelta ) );
			p->mCurrentLife = updateParams.mSystemRand->fFloatZeroToOne( ) * dt;
			p->mYaw0 = state.fSampleEmissionGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cSpawnYawGraph, systemDelta );
			p->mEnergy0 = state.fSampleEmissionGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cSpawnEnergyGraph, systemDelta );
			const Math::tVec3f scale0 = state.fSampleEmissionGraph< FX::tBinaryV3Graph >( updateParams.mSystemRand, cSpawnSizeGraph, systemDelta ) * xformScale;			
			p->mScale0 = scale0.fXY( );
			p->mScale0z = scale0.z;
			p->mCurrentColor = state.fSamplePerParticleGraph< FX::tBinaryV4Graph >( updateParams.mSystemRand, cColorGraph, 0.f );

			updateParams.mEmitter->fEmit( p->mDirection, p->mPosition, updateParams.mEmitInVolume );	// will setup p->mPosition/mDirection
			p->mDirection *= p->mEnergy0;

			if( !updateParams.mLocalSpace )
			{
				p->mPosition = updateParams.mObjToWorld.fXformPoint( ( p->mPosition + ( updateParams.mPositionalDelta * updateParams.mSystemRand->fFloatZeroToOne( ) ) ) );
				p->mDirection = updateParams.mObjToWorld.fXformVector( p->mDirection );
			}
			
			p->mPosition += p->mDirection * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cEnergyGraph, 0.f ) * p->mCurrentLife;

			p->mYAxis = state.fSampleMeshGraph< FX::tBinaryV3Graph >( updateParams.mSystemRand, cMeshInitialAxis, systemDelta ); 
			p->mYAxis.fNormalizeSafe( Math::tVec3f::cYAxis );

			p->mYAxisDelta = state.fSampleMeshGraph< FX::tBinaryV3Graph >( updateParams.mSystemRand, cMeshAxisDelta, systemDelta ); 
			p->mYAxisDelta.fNormalizeSafe( Math::tVec3f::cYAxis ); // what kind of delta gets normalized??
			p->mZAxis = Math::tVec3f::cZAxis;

			for( u32 i = 0; i < (*updateParams.mAttractorsList).fCount( ); ++i )
				(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );

			if( mParticles.fCount( ) < mParticles.fCapacity( ) )
				mParticles.fPushBackNoGrow( p );
			else
			{
				delete p;

				// todo, figure out how to avoid this
				//log_warning( "Mesh particles could not push back!? tried to push " << emitCount << " with capacity " << mParticles.fCapacity( ) );
				break;
			}
		}
	}
	void tMeshParticleList::fUpdateParticles( tParticleListUpdateParams& updateParams )
	{
		profile_pix( "tMeshParticleList::fUpdateParticles" );
		tGrowableArray< u32 > particleRemovalList;
		tGrowableArray< u32 > ghostRemovalList;

		fUpdateParticleList<true>( mParticles, particleRemovalList, updateParams );
		if( updateParams.mMoveGhostParticles )
			fUpdateParticleList<true>( mGhostParticles, ghostRemovalList, updateParams );
		else
			fUpdateParticleList<false>( mGhostParticles, ghostRemovalList, updateParams );
	}
	void tMeshParticleList::fEmitGhostParticles( f32 ghostParticleLifetime, f32 ghostParticleFrequency, f32 timeFromLastEmit )
	{
		const f32 minDelta = 1.f / ghostParticleFrequency;
		const u32 cnt = mParticles.fCount( );
		const u32 ghosts = ( u32 ) ( timeFromLastEmit / minDelta );
		mFirstNewGhostParticleThisFrame = mGhostParticles.fCount( );
		for( u32 i = 0; i < cnt; ++i )
		{
			const tMeshParticle* p1 = mParticles[ i ];
			for( u32 j = 0; j < ghosts; ++j )
			{
				const f32 d = ( f32 )j / ( f32 )ghosts;
				fEmitGhostParticle( p1, d, ghostParticleLifetime, timeFromLastEmit );
			}
		}
	}
	void tMeshParticleList::fSyncST( f32 dt, Gfx::tRenderableEntity& parent, b32 localSpace )
	{
		for( u32 i = 0; i < mParticles.fCount( ); ++i )
			mParticles[ i ]->fSyncMesh( dt, parent, mMeshResource, localSpace );
		for( u32 i = 0; i < mGhostParticles.fCount( ); ++i )
			mGhostParticles[ i ]->fSyncMesh( dt, parent, mMeshResource, localSpace );

		fPurgeMeshesToDelete( );
	}
	void tMeshParticleList::fEmitGhostParticle( const tMeshParticle* particle, f32 delta, f32 ghostParticleLifetime, f32 timeFromLastEmit )
	{
		tMeshParticle *p = NEW tMeshParticle( particle->mLifetime * ghostParticleLifetime
			, particle->mCurrentLife * 0.5f
			, particle->mYaw0
			, particle->mScale0
			, particle->mScale0z
			, particle->mPosition - ( particle->mDirection * timeFromLastEmit * delta )
			, Math::tVec3f::cZeroVector
			, particle->mCurrentColor );
		
		mGhostParticles.fPushBack( p );
	}
	void tMeshParticleList::fPurgeMeshesToDelete( )
	{
		for( u32 i = 0; i < mMeshesToDelete.fCount( ); ++i )
		{
			if( mMeshesToDelete[ i ] )
				mMeshesToDelete[ i ]->fDelete( );
		}
		mMeshesToDelete.fSetCount( 0 );
	}
	template<b32 cUpdatePos>
	void tMeshParticleList::fUpdateParticleList( tGrowableArray< tMeshParticle* >& particleList, tGrowableArray< u32 >& removalList, tParticleListUpdateParams& updateParams )
	{
		removalList.fSetCount( 0 );

		const f32 dt = updateParams.mDt;
		const Math::tMat3f& boxXform = updateParams.mLocalSpace ? Math::tMat3f::cIdentity : updateParams.mObjToWorldInv;
		const u32 remaining = particleList.fCount( ) % 4;
		const u32 particleUpdateCount = particleList.fCount( ) - remaining;
		
		Math::tVec3f min = +Math::cInfinity;
		Math::tVec3f max = -Math::cInfinity;

		const FX::tBinaryParticleSystemState& state = *updateParams.mState;

		for( u32 i = 0; i < particleUpdateCount; i += 4 )
		{
			tMeshParticle* p1 = particleList[ i + 0 ];
			tMeshParticle* p2 = particleList[ i + 1 ];
			tMeshParticle* p3 = particleList[ i + 2 ];
			tMeshParticle* p4 = particleList[ i + 3 ];

			f32 pld1 = p1->mCurrentLife /  p1->mLifetime;
			f32 pld2 = p2->mCurrentLife /  p2->mLifetime;
			f32 pld3 = p3->mCurrentLife /  p3->mLifetime;
			f32 pld4 = p4->mCurrentLife /  p4->mLifetime;

			p1->mCurrentLife += dt;
			p2->mCurrentLife += dt;
			p3->mCurrentLife += dt;
			p4->mCurrentLife += dt;

			if( pld1 > 1.f ){ removalList.fPushBack( i + 0 ); pld1 = 1.f; }
			if( pld2 > 1.f ){ removalList.fPushBack( i + 1 ); pld2 = 1.f; }
			if( pld3 > 1.f ){ removalList.fPushBack( i + 2 ); pld3 = 1.f; }
			if( pld4 > 1.f ){ removalList.fPushBack( i + 3 ); pld4 = 1.f; }

			fWriteToVB( updateParams, p1, p2, p3, p4, pld1, pld2, pld3, pld4 );			

			p1->mCurrentColor = state.fSamplePerParticleGraph< FX::tBinaryV4Graph >( updateParams.mSystemRand, cColorGraph, pld1 );
			p2->mCurrentColor = state.fSamplePerParticleGraph< FX::tBinaryV4Graph >( updateParams.mSystemRand, cColorGraph, pld2 );
			p3->mCurrentColor = state.fSamplePerParticleGraph< FX::tBinaryV4Graph >( updateParams.mSystemRand, cColorGraph, pld3 );
			p4->mCurrentColor = state.fSamplePerParticleGraph< FX::tBinaryV4Graph >( updateParams.mSystemRand, cColorGraph, pld4 );

			if( cUpdatePos )
			{
				p1->mPosition += p1->mDirection * dt * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cEnergyGraph, pld1 );
				p2->mPosition += p2->mDirection * dt * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cEnergyGraph, pld2 );
				p3->mPosition += p3->mDirection * dt * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cEnergyGraph, pld3 );
				p4->mPosition += p4->mDirection * dt * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cEnergyGraph, pld4 );

				for( u32 i = 0; i < (*updateParams.mAttractorsList).fCount( ); ++i )
				{
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p1, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p2, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p3, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p4, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
				}
			}

			min = fMin( min, p1->mPosition );
			max = fMax( max, p1->mPosition );
			min = fMin( min, p2->mPosition );
			max = fMax( max, p2->mPosition );
			min = fMin( min, p3->mPosition );
			max = fMax( max, p3->mPosition );
			min = fMin( min, p4->mPosition );
			max = fMax( max, p4->mPosition );
		}

		if( particleUpdateCount >= 4 )
		{
			(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( min );
			(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( max );
		}

		for( u32 i = particleUpdateCount; i < particleList.fCount( ); ++i )
		{
			tMeshParticle* p1 = particleList[ i ];
			
			f32 normalizedParticleLife = p1->mCurrentLife /  p1->mLifetime;
			p1->mCurrentLife += dt;
			if( normalizedParticleLife > 1.f )
			{
				removalList.fPushBack( i + 0 );
				normalizedParticleLife = 1.f;
			}

			fWriteToVB( updateParams, p1, normalizedParticleLife );

			p1->mCurrentColor = state.fSamplePerParticleGraph< FX::tBinaryV4Graph >( updateParams.mSystemRand, cColorGraph, normalizedParticleLife );

			if( cUpdatePos )
			{
				p1->mPosition += p1->mDirection * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cEnergyGraph, normalizedParticleLife ) * dt;

				for( u32 i = 0; i < (*updateParams.mAttractorsList).fCount( ); ++i )
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p1, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
			}
		
			(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( p1->mPosition + Math::tVec3f( p1->mScale0, p1->mScale0z ) );
			(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( p1->mPosition - Math::tVec3f( p1->mScale0, p1->mScale0z ) );
		}

		for( u32 i = removalList.fCount( ); i > 0; --i )
		{
			mMeshesToDelete.fPushBack( particleList[ removalList[ i-1 ] ]->fDisownMesh( ) );
			delete particleList[ removalList[ i-1 ] ];
			particleList.fErase( removalList[ i-1 ] );
		}
	}


	void tMeshParticleList::fWriteToVB( tParticleListUpdateParams& updateParams, tMeshParticle* p1, tMeshParticle* p2, tMeshParticle* p3, tMeshParticle* p4, f32 d1, f32 d2, f32 d3, f32 d4 )
	{
		//const f32 dt = updateParams.mDt;
		const FX::tBinaryParticleSystemState& state = *updateParams.mState;
		
		p1->mSpin = p1->mYaw0 * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cSpinGraph, d1 );
		p2->mSpin = p2->mYaw0 * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cSpinGraph, d2 );
		p3->mSpin = p3->mYaw0 * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cSpinGraph, d3 );
		p4->mSpin = p4->mYaw0 * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cSpinGraph, d4 );

		p1->mScale = Math::tVec3f(p1->mScale0, p1->mScale0z) * state.fSamplePerParticleGraph< FX::tBinaryV3Graph >( updateParams.mSystemRand, cScaleGraph, d1 );
		p2->mScale = Math::tVec3f(p2->mScale0, p2->mScale0z) * state.fSamplePerParticleGraph< FX::tBinaryV3Graph >( updateParams.mSystemRand, cScaleGraph, d2 );
		p3->mScale = Math::tVec3f(p3->mScale0, p3->mScale0z) * state.fSamplePerParticleGraph< FX::tBinaryV3Graph >( updateParams.mSystemRand, cScaleGraph, d3 );
		p4->mScale = Math::tVec3f(p4->mScale0, p4->mScale0z) * state.fSamplePerParticleGraph< FX::tBinaryV3Graph >( updateParams.mSystemRand, cScaleGraph, d4 );

		if( state.fHasFlag( cAlignWithVelocity ) )
		{
			p1->mZAxis = p1->mDirection;
			p2->mZAxis = p2->mDirection;
			p3->mZAxis = p3->mDirection;
			p4->mZAxis = p4->mDirection;
		}
	}
	void tMeshParticleList::fWriteToVB( tParticleListUpdateParams& updateParams, tMeshParticle* particle, f32 delta )
	{
		//const f32 dt = updateParams.mDt;
		const FX::tBinaryParticleSystemState& state = *updateParams.mState;

		particle->mSpin		= particle->mYaw0 * state.fSamplePerParticleGraph< FX::tBinaryF32Graph >( updateParams.mSystemRand, cSpinGraph, delta );
		particle->mScale	= Math::tVec3f( particle->mScale0, particle->mScale0z ) * state.fSamplePerParticleGraph< FX::tBinaryV3Graph >( updateParams.mSystemRand, cScaleGraph, delta );

		if( state.fHasFlag( cAlignWithVelocity ) )
			particle->mZAxis = particle->mDirection;

		//const Math::tVec3f& pos = particle->mPosition;
		//Math::tVec3f dir1 = Math::tVec3f::cZeroVector;
		//Math::tVec3f dir2 = Math::tVec3f::cZeroVector;
		//if( state.fHasFlag( cAlignWithVelocity ) )
		//	dir1 = particle->mDirection * ( size.x + size.y ) * delta;
		//else if( state.fHasFlag( cVelocityStretch ) )
		//	dir1 = dir2 = particle->mDirection;
		//const Gfx::tParticleRenderVertex verts[ ] = { 
		//	Gfx::tParticleRenderVertex( pos+dir1, vtxColor, -size.x,  size.y, yaw ),
		//	Gfx::tParticleRenderVertex( pos+dir1, vtxColor,  size.x,  size.y, yaw ),
		//	Gfx::tParticleRenderVertex( pos-dir2, vtxColor,  size.x, -size.y, yaw ),
		//	Gfx::tParticleRenderVertex( pos-dir2, vtxColor, -size.x, -size.y, yaw )
		//};
	}

}}
