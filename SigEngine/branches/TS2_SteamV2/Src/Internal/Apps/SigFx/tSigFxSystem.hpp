#ifndef __tSigFxSystem__
#define __tSigFxSystem__
#include "Gfx/tMaterialFile.hpp"
#include "FX/tParticleAttractor.hpp"
#include "FX/tParticleSystem.hpp"
#include "FX/tMeshSystem.hpp"
#include "tAttachmentEntity.hpp"

namespace Sig { namespace FX
{
	
	class tSigFxSystem : public tEntity
	{
		tGrowableArray< tParticleSystemPtr > mParticleSystems;
		tGrowableArray< tParticleAttractorPtr > mParticleAttractors;
		tGrowableArray< tMeshSystemPtr > mFxMeshSystems;
		tGrowableArray< tAttachmentEntityPtr > mAttachmentEntities;

		b8		mAlive;
		b8		mRemoveable;
		b8		mLoop;
		b8		mPad0;

		f32		mCurrentTime;
		f32		mLifetime;

	public:

		tSigFxSystem( );
		~tSigFxSystem( );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		const b8				fAlive( ) const { return mAlive; }
		const b8				fLooping( ) const { return mLoop; }
		const f32				fCurrentTime( ) const { return mCurrentTime; }
		const f32				fLifetime( ) const { return mLifetime; }

		void					fSetCurrentTime( const f32 time );
		void					fSetLifetime( const f32 lifetime );

		void					fRefreshScene( );

		void					fSetLooping( const b8 loop ) { mLoop = loop; }
		void					fSetAlive( const b8 alive ) { mAlive = alive; mRemoveable = false; }

		void					fClearSystems( );

		void					fAddParticleSystem( const tParticleSystemPtr system );
		void					fAddParticleAttractor( const tParticleAttractorPtr attractor );
		void					fAddAttachmentEntity( const tAttachmentEntityPtr attachment );
		void					fAddFxMeshSystem( const tMeshSystemPtr system );

		u32						fParticleCount( ) const;
		u32						fMeshCount( ) const;
		u32						fParticleSystemCount( ) const { return mParticleSystems.fCount( ); }
		u32						fAttractorCount( ) const { return mParticleAttractors.fCount( ); }
		u32						fMeshSystemCount( ) const { return mFxMeshSystems.fCount( ); }

		void					fAcquireAttachmentPoints( tEntity& entity );
		tAttachmentEntity* 		fCheckForAttachmentToAttachmentEntities( tParticleSystemPtr system );

		const tGrowableArray< tParticleAttractorPtr >& fGetAttractorsList( ) const { return mParticleAttractors; }

	};


}
}

#endif	//  __tSigFxSystem__

