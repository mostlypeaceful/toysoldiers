#ifndef __tMeshParticleList__
#define __tMeshParticleList__
#include "tParticleList.hpp"
#include "tSceneRefEntity.hpp"

namespace Sig { namespace FX
{
	class base_export tMeshParticle : public tParticle
	{
		define_class_pool_new_delete_mt( tMeshParticle, 1024 );
	public:
		tMeshParticle( 
			f32 lifetime, 
			f32 curLife, 
			f32 yaw, 
			const Math::tVec2f& scale,
			f32 scaleZ,
			const Math::tVec3f& pos, 
			const Math::tVec3f& dir, 
			const Math::tVec4f& color )
			: tParticle( lifetime, curLife, yaw, scale, pos, dir, color )
			, mMesh( NULL )
			, mScale0z( scaleZ )
			, mSpin( 0.f )
			, mScale( Math::tVec3f::cOnesVector )
			, mYAxis( Math::tVec3f::cYAxis )
			, mZAxis( Math::tVec3f::cZAxis )
			, mYAxisDelta( Math::tVec3f::cZeroVector )
		{
		}
		inline tMeshParticle( ) 
			: mMesh( NULL )
			, mScale0z( 1.f ) 
			, mSpin( 0.f )
			, mScale( Math::tVec3f::cOnesVector )
			, mYAxis( Math::tVec3f::cYAxis )
			, mZAxis( Math::tVec3f::cZAxis )
			, mYAxisDelta( Math::tVec3f::cZeroVector )
		{ 
		}
		inline ~tMeshParticle( ) { sigassert( !mMesh ); }
		void fSyncMesh( f32 dt, tEntity& parent, const tResourcePtr& res, b32 localSpace );
		tSceneRefEntity* fDisownMesh( );

		tSceneRefEntity* mMesh;
		f32 mScale0z;

		f32 mSpin;
		Math::tVec3f mScale;
		Math::tVec3f mYAxis;
		Math::tVec3f mZAxis;
		Math::tVec3f mYAxisDelta;
	};

	class base_export tMeshParticleList : public tParticleList
	{
	protected:
		tGrowableArray< tMeshParticle* > mParticles;
		tGrowableArray< tMeshParticle* > mGhostParticles;
		tGrowableArray< tSceneRefEntity* > mMeshesToDelete;
		tResourcePtr mMeshResource; // TODO get this
		u32 mFirstNewParticleThisFrame, mFirstNewGhostParticleThisFrame;
	public:
		explicit tMeshParticleList( const tResourcePtr& meshResource );
		virtual ~tMeshParticleList( );
		virtual u32 fNormalParticleCount( ) const { return mParticles.fCount( ); }
		virtual u32 fGhostParticleCount( ) const { return mGhostParticles.fCount( ); }
		virtual u32 fTotalParticleCount( ) const { return mParticles.fCount( ) + mGhostParticles.fCount( ); }
		virtual void fResetDeviceObjects( Gfx::tDefaultAllocators& allocators, const Gfx::tRenderState* override = 0 );
		virtual void fAllocateGeometry( Gfx::tRenderableEntity& parent, u32 emitCountThisFrame, Gfx::tMaterial* material );
		virtual void fChangeMaterial( Gfx::tRenderableEntity& parent, Gfx::tMaterial* material );
		virtual void fRefreshParentRenderable( Gfx::tRenderableEntity& parent, b32 unlockBuffers );
		virtual void fClear( );
		virtual void fFastUpdate( f32 dt );
		virtual void fRemoveParticlesYoungerThan( f32 time );
		virtual void fSort( tParticleSortMode sortMode );
		virtual void fEmitParticles( tParticleListUpdateParams& updateParams, u32 emitCount );
		virtual void fUpdateParticles( tParticleListUpdateParams& updateParams );
		virtual void fEmitGhostParticles( f32 ghostParticleLifetime, f32 ghostParticleFrequency, f32 timeFromLastEmit );
		virtual void fSyncST( f32 dt, Gfx::tRenderableEntity& parent, b32 localSpace );
	protected:
		void fEmitGhostParticle( const tMeshParticle* particle, f32 delta, f32 ghostParticleLifetime, f32 timeFromLastEmit );
		void fPurgeMeshesToDelete( );
		template<b32 updatePos>
		void fUpdateParticleList( tGrowableArray< tMeshParticle* >& particleList, tGrowableArray< u32 >& removalList, tParticleListUpdateParams& updateParams );
		void fWriteToVB( tParticleListUpdateParams& updateParams, tMeshParticle* particle, f32 delta );
		void fWriteToVB( tParticleListUpdateParams& updateParams, tMeshParticle* p1, tMeshParticle* p2, tMeshParticle* p3, tMeshParticle* p4, f32 d1, f32 d2, f32 d3, f32 d4 );
	};
}}

#endif//__tMeshParticleList__
