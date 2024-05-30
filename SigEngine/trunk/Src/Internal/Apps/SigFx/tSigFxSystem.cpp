#include "SigFxPch.hpp"
#include "tSigFxSystem.hpp"
#include "tSceneGraph.hpp"
#include "tEntityDef.hpp"
#include "FX/tParticleSystem.hpp"
#include "FX/tFxFile.hpp"

namespace Sig { namespace FX
{
	tSigFxSystem::tSigFxSystem( )
		: mAlive( true )
		, mRemoveable( false )
		, mLoop( true )
		, mCurrentTime( 0.f )
		, mLifetime( 5.f )
	{

	}

	tSigFxSystem::~tSigFxSystem( )
	{
	}

	u32 tSigFxSystem::fParticleCount( ) const
	{
		u32 total = 0;
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			total += mParticleSystems[ i ]->fParticleCount( );
		return total;
	}
	u32 tSigFxSystem::fMeshCount( ) const
	{
		u32 total = 0;
		for( u32 i = 0; i < mFxMeshSystems.fCount( ); ++i )
			total += mFxMeshSystems[ i ]->fMeshCount( );
		return total;
	}

	void tSigFxSystem::fClearSystems( )
	{
		mParticleSystems.fSetCount( 0 );
		mParticleAttractors.fSetCount( 0 );
		mFxMeshSystems.fSetCount( 0 );
		mLights.fSetCount( 0 );
	}

	void tSigFxSystem::fAddParticleSystem( const tParticleSystemPtr system )
	{
		system->fSetAttractors( mParticleAttractors );

		if( mParticleSystems.fFind( system ) )
			sigassert( !"This should not be happening!!!" );

		system->fSetLifetime( mLifetime );
		system->fSetCurrentTime( mCurrentTime );
		mParticleSystems.fPushBack( system );
	}

	void tSigFxSystem::fAddParticleAttractor( const tParticleAttractorPtr attractor )
	{
		mParticleAttractors.fPushBack( attractor );
	}

	void tSigFxSystem::fAddFxMeshSystem( const tMeshSystemPtr system )
	{
		system->fSetAttractors( mParticleAttractors );
		system->fSetLifetime( mLifetime );
		system->fSetCurrentTime( mCurrentTime );
		mFxMeshSystems.fPushBack( system );
	}

	void tSigFxSystem::fAddAttachmentEntity( const tAttachmentEntityPtr attachment )
	{
		mAttachmentEntities.fPushBack( attachment );
	}

	void tSigFxSystem::fAddLight( const tAnimatedLightEntityPtr light )
	{
		mLights.fPushBack( light );
	}

	void tSigFxSystem::fSetCurrentTime( const f32 time )
	{		
		mCurrentTime = time;
		const f32 delta = mCurrentTime / mLifetime;

		for( u32 i = 0; i < mParticleAttractors.fCount( ); ++i )
			mParticleAttractors[ i ]->fUpdateGraphValues( delta );

		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fFastUpdate( mCurrentTime );

		for( u32 i = 0; i < mFxMeshSystems.fCount( ); ++i )
			mFxMeshSystems[ i ]->fFastUpdate( mCurrentTime );

		for( u32 i = 0; i < mLights.fCount( ); ++i )
			mLights[ i ]->fUpdateGraphValues( delta );
	}

	void tSigFxSystem::fSetLifetime( const f32 lifetime )
	{
		mLifetime = lifetime;

		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fSetLifetime( mLifetime );
		for( u32 i = 0; i < mFxMeshSystems.fCount( ); ++i )
			mFxMeshSystems[ i ]->fSetLifetime( mLifetime );

		if( mCurrentTime > mLifetime )
		{
			mCurrentTime = 0.f;
			fSetCurrentTime( mCurrentTime );
		}
	}

	void tSigFxSystem::fRefreshScene( )
	{
		const f32 delta = mCurrentTime / mLifetime;

		for( u32 i = 0; i < mParticleAttractors.fCount( ); ++i )
			mParticleAttractors[ i ]->fUpdateGraphValues( delta );

		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fFastUpdate( mCurrentTime, true );

		for( u32 i = 0; i < mFxMeshSystems.fCount( ); ++i )
			mFxMeshSystems[ i ]->fFastUpdate( mCurrentTime, true );

		for( u32 i = 0; i < mLights.fCount( ); ++i )
			mLights[ i ]->fUpdateGraphValues( delta );
	}

	void tSigFxSystem::fAcquireAttachmentPoints( tEntity& entity )
	{
		for( u32 i = 0; i < entity.fChildCount( ); ++i )
		{
			tAttachmentEntity* attachment = entity.fChild( i )->fDynamicCast< tAttachmentEntity >( );
			if( attachment )
				fAddAttachmentEntity( tAttachmentEntityPtr( attachment ) );
			else
				fAcquireAttachmentPoints( *entity.fChild( i ) );
		}
	}

	tAttachmentEntity* tSigFxSystem::fCheckForAttachmentToAttachmentEntities( tParticleSystemPtr system )
	{
		f32 bestDistance = Math::cInfinity;
		const Math::tVec3f systemPos = system->fObjectToWorld( ).fGetTranslation( );
		tAttachmentEntityPtr bestAttachment( 0 );
		for( u32 i = 0; i < mAttachmentEntities.fCount( ); ++i )
		{
			f32 dist = ( systemPos - mAttachmentEntities[ i ]->fObjectToWorld( ).fGetTranslation( ) ).fLength( );
			if( dist < bestDistance )
			{
				bestDistance = dist;
				bestAttachment = mAttachmentEntities[ i ];
			}
		}

		if( bestDistance < .25f && bestAttachment )
			return bestAttachment.fGetRawPtr( );
		return 0;
	}

	void tSigFxSystem::fOnSpawn( )
	{
		fOnPause( false );
		tEntity::fOnSpawn( );
	}

	void tSigFxSystem::fOnPause( b32 paused )
	{
		if( paused )
			fRunListRemove( cRunListThinkST );
		else
			fRunListInsert( cRunListThinkST );
		tEntity::fOnPause( paused );
	}

	void tSigFxSystem::fThinkST( f32 dt )
	{
		// Render all attachment points...
		mAttachmentEntities.fSetCount( 0 );
		fAcquireAttachmentPoints( fSceneGraph( )->fRootEntity( ) );
		for( u32 i = 0; i < mAttachmentEntities.fCount( ); ++i )
		{
			Math::tSpheref attachment( mAttachmentEntities[ i ]->fObjectToWorld( ).fGetTranslation( ), 0.1f );
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( attachment, Math::tVec4f( .3f, .3f, 1.f, .666f ) );
		}

		f32 delta = mCurrentTime / mLifetime;
		mCurrentTime += dt;
		if( mCurrentTime > mLifetime )
		{
			if( mLoop )
			{
				mCurrentTime = 0.f;
				delta = 0.f;
			}
			else
			{
				mAlive = false;
			}
			delta = 1.f;
		}
		
		for( u32 i = 0; i < mParticleAttractors.fCount( ); ++i )
			mParticleAttractors[ i ]->fUpdateGraphValues( delta );

		for( u32 i = 0; i < mLights.fCount( ); ++i )
			mLights[ i ]->fUpdateGraphValues( delta );

		if( !mAlive && !mRemoveable )
		{
			// check to see if all our systems are also dead, then remove ourselves
			mRemoveable = true;
			for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			{
				if( mParticleSystems[ i ]->fAlive( ) )
				{
					mRemoveable = false;
					break;
				}
			}

			if( mRemoveable )
				fDelete( );
		}
	}
}
}

