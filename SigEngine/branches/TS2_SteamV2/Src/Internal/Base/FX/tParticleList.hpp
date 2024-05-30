#ifndef __tParticleList__
#define __tParticleList__
#include "tParticle.hpp"

namespace Sig { namespace Gfx
{
	struct tDefaultAllocators;
	class tRenderState;
	class tMaterial;
	class tRenderableEntity;
}}

namespace Sig { namespace FX
{
	class tState;
	class tParticleEmitter;
	class tParticleAttractor;

	struct tParticleListUpdateParams
	{
		f32 mDt;
		f32 mSystemDelta; 
		b8 mMoveGhostParticles;
		b8 mLocalSpace;
		b8 mEmitInVolume;
		b8 pad;
		Math::tMat3f mObjToWorld;
		Math::tMat3f mObjToWorldInv;
		Math::tVec3f mPositionalDelta;
		tParticleEmitter* mEmitter;
		const tState* mState;
		const tMersenneGenerator* mSystemRand;
		const tGrowableArray< tRefCounterPtr< tParticleAttractor > >* mAttractorsList;
		Math::tAabbf* mBoxToUpdate;

		tParticleListUpdateParams( ) { fZeroOut( *this ); }
	};

	class base_export tParticleList : public tRefCounter
	{
	public:
		virtual ~tParticleList( ) { }
		virtual u32 fNormalParticleCount( ) const { return 0; }
		virtual u32 fGhostParticleCount( ) const { return 0; }
		virtual u32 fTotalParticleCount( ) const { return 0; }
		virtual void fResetDeviceObjects( Gfx::tDefaultAllocators& allocators, const Gfx::tRenderState* override = 0 ) { }
		virtual void fAllocateGeometry( Gfx::tRenderableEntity& parent, u32 emitCountThisFrame, Gfx::tMaterial* material ) { }
		virtual void fChangeMaterial( Gfx::tRenderableEntity& parent, Gfx::tMaterial* material ) { }
		virtual void fRefreshParentRenderable( Gfx::tRenderableEntity& parent, b32 unlockBuffers ) { }
		virtual void fClear( ) { }
		virtual void fFastUpdate( f32 dt ) { }
		virtual void fRemoveParticlesYoungerThan( f32 time ) { }
		virtual void fSort( tParticleSortMode sortMode ) { }
		virtual void fEmitParticles( tParticleListUpdateParams& updateParams, u32 emitCount ) { }
		virtual void fUpdateParticles( tParticleListUpdateParams& updateParams ) { }
		virtual void fEmitGhostParticles( f32 ghostParticleLifetime, f32 ghostParticleFrequency, f32 timeFromLastEmit ) { }
		virtual void fSyncST( f32 dt, Gfx::tRenderableEntity& parent, b32 localSpace ) { }
	};
	typedef tRefCounterPtr<tParticleList> tParticleListPtr;
}}

#endif//__tParticleList__
