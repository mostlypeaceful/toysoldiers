#ifndef __tQuadParticleList__
#define __tQuadParticleList__
#include "tParticleList.hpp"
#include "Gfx/tDynamicGeometry.hpp"

namespace Sig { namespace FX
{

	class base_export tQuadParticleList : public tParticleList
	{
	protected:
		tGrowableArray< tParticle* > mParticles;
		tGrowableArray< tParticle* > mGhostParticles;
		b32 mValidGeometry;
		u32 mBaseVertexIndex;
		u32 mCurrentIndiceIndex;
		Sig::byte* mGpuVerts;
		Sig::byte* mGpuIndices;
		Gfx::tDynamicGeometry mGeometry;
	public:
		tQuadParticleList( );
		virtual ~tQuadParticleList( );
		virtual u32 fNormalParticleCount( ) const { return mParticles.fCount( ); }
		virtual u32 fGhostParticleCount( ) const { return mGhostParticles.fCount( ); }
		virtual u32 fTotalParticleCount( ) const { return mParticles.fCount( ) + mGhostParticles.fCount( ); }
		virtual void fResetDeviceObjects( Gfx::tDefaultAllocators& allocators, const Gfx::tRenderState* override = 0 );
		virtual void fAllocateGeometry( Gfx::tRenderableEntity& parent, u32 emitCountThisFrame, Gfx::tMaterial* material );
		virtual void fChangeMaterial( Gfx::tRenderableEntity& parent, Gfx::tMaterial* material );
		virtual void fRefreshParentRenderable( Gfx::tRenderableEntity& parent, b32 unlockBuffers );
		virtual void fClear( );
		virtual void fFreeVram( Gfx::tRenderableEntity& parent );
		virtual void fFastUpdate( f32 dt );
		virtual void fRemoveParticlesYoungerThan( f32 time );
		virtual void fSort( tParticleSortMode sortMode );
		virtual void fEmitParticles( tParticleListUpdateParams& updateParams, u32 emitCount );
		virtual void fUpdateParticles( tParticleListUpdateParams& updateParams );
		virtual void fEmitGhostParticles( f32 ghostParticleLifetime, f32 ghostParticleFrequency, f32 timeFromLastEmit );
		virtual void fSyncST( f32 dt, Gfx::tRenderableEntity& parent, b32 localSpace ) { }
	protected:
		void fEmitGhostParticle( const tParticle* particle, f32 delta, f32 ghostParticleLifetime, f32 timeFromLastEmit );
		template<b32 updatePos>
		void fUpdateParticleList( tGrowableArray< tParticle* >& particleList, tGrowableArray< u32 >& removalList, tParticleListUpdateParams& updateParams );
		void fWriteToVB( tParticleListUpdateParams& updateParams, const tParticle* particle, f32 delta );
		void fWriteToVB( tParticleListUpdateParams& updateParams, const tParticle* p1, const tParticle* p2, const tParticle* p3, const tParticle* p4, f32 d1, f32 d2, f32 d3, f32 d4 );
		void fValidateGeometry( );
	};
}}

#endif//__tQuadParticleList__
