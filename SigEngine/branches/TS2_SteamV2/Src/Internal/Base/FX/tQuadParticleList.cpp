#include "BasePch.hpp"
#include "tQuadParticleList.hpp"
#include "tEmitters.hpp"
#include "tParticleSystemStates.hpp"
#include "tParticleAttractor.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "Gfx/tDefaultAllocators.hpp"

namespace Sig { namespace FX
{
	tQuadParticleList::tQuadParticleList( )
	{
		mValidGeometry = false;
		mBaseVertexIndex = 0;
		mCurrentIndiceIndex = 0;
		mGpuVerts = 0;
		mGpuIndices = 0;
	}
	tQuadParticleList::~tQuadParticleList( )
	{
		fClear( );
	}
	void tQuadParticleList::fResetDeviceObjects( Gfx::tDefaultAllocators& allocators, const Gfx::tRenderState* override )
	{
		mBaseVertexIndex = 0;
		mCurrentIndiceIndex = 0;
		mGpuVerts = 0;
		mGpuIndices = 0;

		mGeometry.fResetDeviceObjects( allocators.mParticleGeomAllocator, allocators.mIndexAllocator );
		mGeometry.fSetPrimTypeOverride( Gfx::tIndexFormat::cPrimitiveTriangleList );
		if( override )
			mGeometry.fSetRenderStateOverride( override );
		mGpuVerts = 0;
		mGpuIndices = 0;
	}
	void tQuadParticleList::fChangeMaterial( Gfx::tRenderableEntity& parent, Gfx::tMaterial* material )
	{
		if( material )
		{
			mGeometry.fChangeMaterial( *material );
			parent.fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
		}
		else
		{
			parent.fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
		}
	}
	void tQuadParticleList::fAllocateGeometry( Gfx::tRenderableEntity& parent, u32 emitCountThisFrame, Gfx::tMaterial* material )
	{
		const u32 numNormalParticles = mParticles.fCount( ) + emitCountThisFrame;
		const u32 totalNumParticles = numNormalParticles + mGhostParticles.fCount( );
		mBaseVertexIndex = 0;
		mCurrentIndiceIndex = 0;
		mGpuVerts = 0;
		mGpuIndices = 0;
		mParticles.fSetCapacity( numNormalParticles );

		mValidGeometry = ( material && totalNumParticles > 0 ) ? mGeometry.fAllocateGeometry( *material, totalNumParticles * 4, totalNumParticles * 6, totalNumParticles * 2 ) : false;
		if( mValidGeometry )
		{
			if_assert( Gfx::tRenderBatchData batchData = mGeometry.fGetRenderBatch( )->fBatchData( ) );
			sigassert( totalNumParticles*4 == batchData.mVertexCount );
			sigassert( totalNumParticles*6 == batchData.mPrimitiveCount * 3 );

			mGpuVerts = mGeometry.fLockBuffer( );
			mGpuIndices = mGeometry.fLockIndices( );
		}
		else
			parent.fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
	}
	void tQuadParticleList::fRefreshParentRenderable( Gfx::tRenderableEntity& parent, b32 unlockBuffers )
	{
		if( mValidGeometry )
		{
			if( unlockBuffers )
			{
				if_assert( Gfx::tRenderBatchData batchData = mGeometry.fGetRenderBatch( )->fBatchData( ) );
				sigassert( mBaseVertexIndex == batchData.mVertexCount );
				sigassert( mCurrentIndiceIndex == batchData.mPrimitiveCount * 3 );

				mGeometry.fUnlockBuffer( mGpuVerts );
				mGeometry.fUnlockIndices( mGpuIndices );
				mGpuVerts = 0;
				mGpuIndices = 0;
			}

			parent.fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
		}
	}
	void tQuadParticleList::fClear( )
	{
		for( u32 i = 0; i < mParticles.fCount( ); ++i )
			delete mParticles[ i ];
		for( u32 i = 0; i < mGhostParticles.fCount( ); ++i )
			delete mGhostParticles[ i ];
		mParticles.fSetCount( 0 );
		mGhostParticles.fSetCount( 0 );
	}
	void tQuadParticleList::fFastUpdate( f32 dt )
	{
		for( u32 i = 0; i < mParticles.fCount( ); )
		{
			tParticle* p = mParticles[ i ];
			if( ( p->mCurrentLife + dt ) > p->mLifetime )
			{
				mParticles.fErase( i );
				delete p;
				continue;
			}
			++i;
		}
	}
	void tQuadParticleList::fRemoveParticlesYoungerThan( f32 time )
	{
		for( u32 i = 0; i < mParticles.fCount( ); )
		{
			tParticle* p = mParticles[ i ];
			if( p->mLifetime < time )
			{
				mParticles.fErase( i );
				delete p;
				continue;
			}
			++i;
		}
	}
	void tQuadParticleList::fSort( tParticleSortMode sortMode )
	{
		switch( sortMode )
		{
		case cNoSort: break;
		case cDistanceSort:
			{
				//Gfx::tViewportPtr viewport = tFxSystem::fScreen( )->fViewport( 0 );
				//Gfx::tCamera& cam = viewport->fGetCamera( );
				//std::sort( mParticles.fBegin( ), mParticles.fEnd( ), tDistanceSort( cam.fGetWorldToCamera( ).fGetTranslation( ) ) );
			}
			break;
		case cFirstBornSort:	std::sort( mParticles.fBegin( ), mParticles.fEnd( ), tFirstBornSort( ) );	break;
		case cLastBornSort:		std::sort( mParticles.fBegin( ), mParticles.fEnd( ), tLastBornSort( ) );	break;
		case cAlphaSort:		std::sort( mParticles.fBegin( ), mParticles.fEnd( ), tAlphaSort( ) );		break;
		}
	}
	void tQuadParticleList::fEmitParticles( tParticleListUpdateParams& updateParams, u32 emitCount )
	{
		const f32 dt = updateParams.mDt;
		const f32 systemDelta = updateParams.mSystemDelta;
		const FX::tState& state = *updateParams.mState;

		Math::tMat3f boxXform = Math::tMat3f::cIdentity;
		if( !updateParams.mLocalSpace )
			boxXform = updateParams.mObjToWorldInv;

		const f32 cParticleGraphScaleRatio = 0.1f;		// so larger numbers on the visual graph actually represent a number 1/10th the actual value.
		const Math::tVec2f xformScale = Math::tVec2f( updateParams.mObjToWorld.fGetScale( ).fXY( ) ) * cParticleGraphScaleRatio;

		for( u32 iemit = 0; iemit < emitCount; ++iemit )
		{
			tParticle *p = NEW tParticle( );

			p->mLifetime = fMax( 0.001f, state.fSampleEmissionGraph< f32 >( updateParams.mSystemRand, cParticleLifeGraph, systemDelta ) );
			p->mCurrentLife = updateParams.mSystemRand->fFloatZeroToOne( ) * dt;
			p->mYaw0 = state.fSampleEmissionGraph< f32 >( updateParams.mSystemRand, cSpawnYawGraph, systemDelta );
			p->mEnergy0 = state.fSampleEmissionGraph< f32 >( updateParams.mSystemRand, cSpawnEnergyGraph, systemDelta );
			p->mScale0 = state.fSampleEmissionGraph< Math::tVec3f >( updateParams.mSystemRand, cSpawnSizeGraph, systemDelta ).fXY( ) * xformScale;			
			p->mCurrentColor = state.fSamplePerParticleGraph< Math::tVec4f >( updateParams.mSystemRand, cColorGraph, 0.f );

			updateParams.mEmitter->fEmit( p->mDirection, p->mPosition, updateParams.mEmitInVolume );	// will setup p->mPosition/mDirection
			p->mDirection *= p->mEnergy0;

			if( !updateParams.mLocalSpace )
			{
				p->mPosition = updateParams.mObjToWorld.fXformPoint( ( p->mPosition + ( updateParams.mPositionalDelta * updateParams.mSystemRand->fFloatZeroToOne( ) ) ) );
				p->mDirection = updateParams.mObjToWorld.fXformVector( p->mDirection );
			}
			
			p->mPosition += p->mDirection * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cEnergyGraph, 0.f ) * p->mCurrentLife;
			for( u32 i = 0; i < (*updateParams.mAttractorsList).fCount( ); ++i )
				(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );

			if( mParticles.fCount( ) < mParticles.fCapacity( ) )
				mParticles.fPushBackNoGrow( p );
			else
			{
				delete p;

				log_warning( 0, "Particles could not push back!? tried to push " << emitCount << " with capacity " << mParticles.fCapacity( ) );
				break;
			}
		}
	}
	void tQuadParticleList::fUpdateParticles( tParticleListUpdateParams& updateParams )
	{
		tGrowableArray< u32 > particleRemovalList;
		tGrowableArray< u32 > ghostRemovalList;

		fUpdateParticleList<true>( mParticles, particleRemovalList, updateParams );
		if( updateParams.mMoveGhostParticles )
			fUpdateParticleList<true>( mGhostParticles, ghostRemovalList, updateParams );
		else
			fUpdateParticleList<false>( mGhostParticles, ghostRemovalList, updateParams );


#if defined( sig_assert ) && defined( sig_logging )
		if( mValidGeometry )
		{
			if_assert( Gfx::tRenderBatchData batchData = mGeometry.fGetRenderBatch( )->fBatchData( ) );
			sigassert( mBaseVertexIndex == batchData.mVertexCount );
			sigassert( mCurrentIndiceIndex == batchData.mPrimitiveCount * 3 );
		}
#endif
	}
	void tQuadParticleList::fEmitGhostParticles( f32 ghostParticleLifetime, f32 ghostParticleFrequency, f32 timeFromLastEmit )
	{
		const f32 minDelta = 1.f / ghostParticleFrequency;
		const u32 cnt = mParticles.fCount( );
		const u32 ghosts = ( u32 ) ( timeFromLastEmit / minDelta );
		for( u32 i = 0; i < cnt; ++i )
		{
			const tParticle* p1 = mParticles[ i ];
			for( u32 j = 0; j < ghosts; ++j )
			{
				const f32 d = ( f32 )j / ( f32 )ghosts;
				fEmitGhostParticle( p1, d, ghostParticleLifetime, timeFromLastEmit );
			}
		}
	}
	void tQuadParticleList::fEmitGhostParticle( const tParticle* particle, f32 delta, f32 ghostParticleLifetime, f32 timeFromLastEmit )
	{
		tParticle *p = NEW tParticle( particle->mLifetime * ghostParticleLifetime
			, particle->mCurrentLife*.5f
			, particle->mYaw0
			, particle->mScale0
			, particle->mPosition - ( particle->mDirection * timeFromLastEmit * delta )
			, Math::tVec3f::cZeroVector
			, particle->mCurrentColor );
		
		mGhostParticles.fPushBack( p );
	}
	template<b32 cUpdatePos>
	void tQuadParticleList::fUpdateParticleList( tGrowableArray< tParticle* >& particleList, tGrowableArray< u32 >& removalList, tParticleListUpdateParams& updateParams )
	{
		if_assert( u32 statVertexIndex = mBaseVertexIndex );

		removalList.fSetCount( 0 );

		const f32 dt = updateParams.mDt;
		const u32 remaining = particleList.fCount( ) % 4;
		const u32 particleUpdateCount = particleList.fCount( ) - remaining;
		
		//Math::tVec3f min = +Math::cInfinity;
		//Math::tVec3f max = -Math::cInfinity;

		const FX::tState& state = *updateParams.mState;

		for( u32 i = 0; i < particleUpdateCount; i += 4 )
		{
			tParticle* p1 = particleList[ i + 0 ];
			tParticle* p2 = particleList[ i + 1 ];
			tParticle* p3 = particleList[ i + 2 ];
			tParticle* p4 = particleList[ i + 3 ];

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

			p1->mCurrentColor = state.fSamplePerParticleGraph< Math::tVec4f >( updateParams.mSystemRand, cColorGraph, pld1 );
			p2->mCurrentColor = state.fSamplePerParticleGraph< Math::tVec4f >( updateParams.mSystemRand, cColorGraph, pld2 );
			p3->mCurrentColor = state.fSamplePerParticleGraph< Math::tVec4f >( updateParams.mSystemRand, cColorGraph, pld3 );
			p4->mCurrentColor = state.fSamplePerParticleGraph< Math::tVec4f >( updateParams.mSystemRand, cColorGraph, pld4 );

			if( cUpdatePos )
			{
				p1->mPosition += p1->mDirection * dt * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cEnergyGraph, pld1 );
				p2->mPosition += p2->mDirection * dt * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cEnergyGraph, pld2 );
				p3->mPosition += p3->mDirection * dt * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cEnergyGraph, pld3 );
				p4->mPosition += p4->mDirection * dt * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cEnergyGraph, pld4 );

				for( u32 i = 0; i < (*updateParams.mAttractorsList).fCount( ); ++i )
				{
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p1, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p2, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p3, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p4, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
				}
			}

			/*
			min = fMin( min, p1->mPosition );
			max = fMax( max, p1->mPosition );
			min = fMin( min, p2->mPosition );
			max = fMax( max, p2->mPosition );
			min = fMin( min, p3->mPosition );
			max = fMax( max, p3->mPosition );
			min = fMin( min, p4->mPosition );
			max = fMax( max, p4->mPosition );
			*/
		}

		//if( particleUpdateCount >= 4 )
		{
			//(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( min );
			//(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( max );
		}

		for( u32 i = particleUpdateCount; i < particleList.fCount( ); ++i )
		{
			tParticle* p1 = particleList[ i ];

			f32 particleLifeDelta = p1->mCurrentLife /  p1->mLifetime;
			p1->mCurrentLife += dt;
			if( particleLifeDelta > 1.f ){ removalList.fPushBack( i + 0 ); particleLifeDelta = 1.f; }

			fWriteToVB( updateParams, p1, particleLifeDelta );			

			p1->mCurrentColor = state.fSamplePerParticleGraph< Math::tVec4f >( updateParams.mSystemRand, cColorGraph, particleLifeDelta );

			if( cUpdatePos )
			{
				p1->mPosition += p1->mDirection * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cEnergyGraph, particleLifeDelta ) * dt;

				for( u32 i = 0; i < (*updateParams.mAttractorsList).fCount( ); ++i )
					(*updateParams.mAttractorsList)[ i ]->fActOnParticle( dt, p1, updateParams.mLocalSpace, updateParams.mObjToWorld, updateParams.mObjToWorldInv );
			}
		
			//(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( p1->mPosition + Math::tVec3f( p1->mScale0, .1f ) );
			//(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( p1->mPosition - Math::tVec3f( p1->mScale0, .1f ) );
		}

		sigassert( !mValidGeometry || mBaseVertexIndex == ( statVertexIndex + particleList.fCount( ) * 4 ) );

		for( u32 i = removalList.fCount( ); i > 0; --i )
		{
			delete particleList[ removalList[ i-1 ] ];
			particleList.fErase( removalList[ i-1 ] );
		}

		//sync_event_v_c( (*updateParams.mBoxToUpdate), tSync::cSCParticles );
	}
	void tQuadParticleList::fWriteToVB( tParticleListUpdateParams& updateParams, const tParticle* p1, const tParticle* p2, const tParticle* p3, const tParticle* p4, f32 d1, f32 d2, f32 d3, f32 d4 )
	{
		const FX::tState& state = *updateParams.mState;

		const u32 vtxColor[ ] = {	Gfx::tVertexColor( p1->mCurrentColor.x, p1->mCurrentColor.y, p1->mCurrentColor.z, p1->mCurrentColor.w ).fForGpu( ),
									Gfx::tVertexColor( p2->mCurrentColor.x, p2->mCurrentColor.y, p2->mCurrentColor.z, p2->mCurrentColor.w ).fForGpu( ),
									Gfx::tVertexColor( p3->mCurrentColor.x, p3->mCurrentColor.y, p3->mCurrentColor.z, p3->mCurrentColor.w ).fForGpu( ),
									Gfx::tVertexColor( p4->mCurrentColor.x, p4->mCurrentColor.y, p4->mCurrentColor.z, p4->mCurrentColor.w ).fForGpu( ) };
		
		const f32 yaw[ ] = { p1->mYaw0 * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cSpinGraph, d1 ),
							 p2->mYaw0 * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cSpinGraph, d2 ),
							 p3->mYaw0 * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cSpinGraph, d3 ),
							 p4->mYaw0 * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cSpinGraph, d4 ) };

		const Math::tVec2f size[ ] = {
			p1->mScale0 * state.fSamplePerParticleGraph< Math::tVec3f >( updateParams.mSystemRand, cScaleGraph, d1 ).fXY( ),
			p2->mScale0 * state.fSamplePerParticleGraph< Math::tVec3f >( updateParams.mSystemRand, cScaleGraph, d2 ).fXY( ),
			p3->mScale0 * state.fSamplePerParticleGraph< Math::tVec3f >( updateParams.mSystemRand, cScaleGraph, d3 ).fXY( ),
			p4->mScale0 * state.fSamplePerParticleGraph< Math::tVec3f >( updateParams.mSystemRand, cScaleGraph, d4 ).fXY( ),
		};

		Math::tVec3f pos[ ] = { p1->mPosition, p1->mPosition, p2->mPosition, p2->mPosition, p3->mPosition, p3->mPosition, p4->mPosition, p4->mPosition };

		if( state.fHasFlag( cAlignWithVelocity ) )
		{
			pos[ 0 ] += p1->mDirection * ( size[ 0 ].x + size[ 0 ].y ) * d1;
			pos[ 2 ] += p2->mDirection * ( size[ 1 ].x + size[ 1 ].y ) * d2;
			pos[ 4 ] += p3->mDirection * ( size[ 2 ].x + size[ 2 ].y ) * d3;
			pos[ 6 ] += p4->mDirection * ( size[ 3 ].x + size[ 3 ].y ) * d4;
		}
		else if( state.fHasFlag( cVelocityStretch ) )
		{
			pos[ 0 ] += p1->mDirection; pos[ 1 ] -= p1->mDirection;
			pos[ 2 ] += p2->mDirection; pos[ 3 ] -= p2->mDirection;
			pos[ 4 ] += p3->mDirection; pos[ 5 ] -= p3->mDirection;
			pos[ 6 ] += p4->mDirection; pos[ 7 ] -= p4->mDirection;
		}
	

		const Math::tMat3f& boxXform = updateParams.mLocalSpace ? Math::tMat3f::cIdentity : updateParams.mObjToWorldInv;
		for(int i = 0; i < array_length( pos ); ++i )
			(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( pos[ i ] );

		// We may not have the render geometry to do this
		if( mValidGeometry )
		{
			const Gfx::tParticleRenderVertex verts[ ] = { 
				Gfx::tParticleRenderVertex( pos[ 0 ], vtxColor[ 0 ], -size[ 0 ].x,  size[ 0 ].y, yaw[ 0 ] ),
				Gfx::tParticleRenderVertex( pos[ 0 ], vtxColor[ 0 ],  size[ 0 ].x,  size[ 0 ].y, yaw[ 0 ] ),
				Gfx::tParticleRenderVertex( pos[ 1 ], vtxColor[ 0 ],  size[ 0 ].x, -size[ 0 ].y, yaw[ 0 ] ),
				Gfx::tParticleRenderVertex( pos[ 1 ], vtxColor[ 0 ], -size[ 0 ].x, -size[ 0 ].y, yaw[ 0 ] ),
																						
				Gfx::tParticleRenderVertex( pos[ 2 ], vtxColor[ 1 ], -size[ 1 ].x,  size[ 1 ].y, yaw[ 1 ] ),
				Gfx::tParticleRenderVertex( pos[ 2 ], vtxColor[ 1 ],  size[ 1 ].x,  size[ 1 ].y, yaw[ 1 ] ),
				Gfx::tParticleRenderVertex( pos[ 3 ], vtxColor[ 1 ],  size[ 1 ].x, -size[ 1 ].y, yaw[ 1 ] ),
				Gfx::tParticleRenderVertex( pos[ 3 ], vtxColor[ 1 ], -size[ 1 ].x, -size[ 1 ].y, yaw[ 1 ] ),
																						
				Gfx::tParticleRenderVertex( pos[ 4 ], vtxColor[ 2 ], -size[ 2 ].x,  size[ 2 ].y, yaw[ 2 ] ),
				Gfx::tParticleRenderVertex( pos[ 4 ], vtxColor[ 2 ],  size[ 2 ].x,  size[ 2 ].y, yaw[ 2 ] ),
				Gfx::tParticleRenderVertex( pos[ 5 ], vtxColor[ 2 ],  size[ 2 ].x, -size[ 2 ].y, yaw[ 2 ] ),
				Gfx::tParticleRenderVertex( pos[ 5 ], vtxColor[ 2 ], -size[ 2 ].x, -size[ 2 ].y, yaw[ 2 ] ),
																						
				Gfx::tParticleRenderVertex( pos[ 6 ], vtxColor[ 3 ], -size[ 3 ].x,  size[ 3 ].y, yaw[ 3 ] ),
				Gfx::tParticleRenderVertex( pos[ 6 ], vtxColor[ 3 ],  size[ 3 ].x,  size[ 3 ].y, yaw[ 3 ] ),
				Gfx::tParticleRenderVertex( pos[ 7 ], vtxColor[ 3 ],  size[ 3 ].x, -size[ 3 ].y, yaw[ 3 ] ),
				Gfx::tParticleRenderVertex( pos[ 7 ], vtxColor[ 3 ], -size[ 3 ].x, -size[ 3 ].y, yaw[ 3 ] )
			};

			const u16 ids[ ] = {
				mBaseVertexIndex+0+0, mBaseVertexIndex+0+1, mBaseVertexIndex+0+2, mBaseVertexIndex+0+2, mBaseVertexIndex+0+3, mBaseVertexIndex+0+0,
				mBaseVertexIndex+4+0, mBaseVertexIndex+4+1, mBaseVertexIndex+4+2, mBaseVertexIndex+4+2, mBaseVertexIndex+4+3, mBaseVertexIndex+4+0,
				mBaseVertexIndex+8+0, mBaseVertexIndex+8+1, mBaseVertexIndex+8+2, mBaseVertexIndex+8+2, mBaseVertexIndex+8+3, mBaseVertexIndex+8+0,
				mBaseVertexIndex+12+0, mBaseVertexIndex+12+1, mBaseVertexIndex+12+2, mBaseVertexIndex+12+2, mBaseVertexIndex+12+3, mBaseVertexIndex+12+0
			};

			fMemCpyToGpu( &mGpuVerts[ sizeof( verts[0] ) * mBaseVertexIndex ], verts, sizeof( verts ) ); mBaseVertexIndex += array_length( verts );
			fMemCpyToGpu( &mGpuIndices[ sizeof( ids[0] ) * mCurrentIndiceIndex ], ids, sizeof( ids ) ); mCurrentIndiceIndex += array_length( ids );
		}
	}
	void tQuadParticleList::fWriteToVB( tParticleListUpdateParams& updateParams, const tParticle* particle, f32 delta )
	{
		const FX::tState& state = *updateParams.mState;

		const u32 vtxColor = Gfx::tVertexColor( particle->mCurrentColor.x, particle->mCurrentColor.y, particle->mCurrentColor.z, particle->mCurrentColor.w ).fForGpu( );

		const f32 yaw = particle->mYaw0 * state.fSamplePerParticleGraph< f32 >( updateParams.mSystemRand, cSpinGraph, delta );
		const Math::tVec2f size = particle->mScale0 * state.fSamplePerParticleGraph< Math::tVec3f >( updateParams.mSystemRand, cScaleGraph, delta ).fXY( );		
		const Math::tVec3f& pos = particle->mPosition;
		
		Math::tVec3f dir1( Math::tVec3f::cZeroVector );
		Math::tVec3f dir2( Math::tVec3f::cZeroVector );

		if( state.fHasFlag( cAlignWithVelocity ) )
			dir1 = particle->mDirection * ( size.x + size.y ) * delta;
		else if( state.fHasFlag( cVelocityStretch ) )
			dir1 = dir2 = particle->mDirection;

		const Math::tVec3f positions[] = { pos + dir1, pos - dir2 };

		const Math::tMat3f& boxXform = updateParams.mLocalSpace ? Math::tMat3f::cIdentity : updateParams.mObjToWorldInv;
		(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( positions[ 0 ] );
		(*updateParams.mBoxToUpdate) |= boxXform.fXformPoint( positions[ 1 ] );
		
		// We may not have the render geometry to do this
		if( mValidGeometry )
		{
			const Gfx::tParticleRenderVertex verts[ ] = { 
				Gfx::tParticleRenderVertex( positions[ 0 ], vtxColor, -size.x,  size.y, yaw ),
				Gfx::tParticleRenderVertex( positions[ 0 ], vtxColor,  size.x,  size.y, yaw ),
				Gfx::tParticleRenderVertex( positions[ 1 ], vtxColor,  size.x, -size.y, yaw ),
				Gfx::tParticleRenderVertex( positions[ 1 ], vtxColor, -size.x, -size.y, yaw )
			};

			const u16 ids[ ] = { 
				mBaseVertexIndex+0, mBaseVertexIndex+1, mBaseVertexIndex+2, mBaseVertexIndex+2, mBaseVertexIndex+3, mBaseVertexIndex+0
			};

			fMemCpyToGpu( &mGpuVerts[ sizeof( verts[0] ) * mBaseVertexIndex ], verts, sizeof( verts ) ); mBaseVertexIndex += array_length( verts );
			fMemCpyToGpu( &mGpuIndices[ sizeof( ids[0] ) * mCurrentIndiceIndex ], ids, sizeof( ids ) ); mCurrentIndiceIndex += array_length( ids );
		}
	}

}}
