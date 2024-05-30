#include "BasePch.hpp"
#include "tFxFileRefEntity.hpp"
#include "FX/tFxFile.hpp"
#include "tSceneGraph.hpp"
#include "tProfiler.hpp"

namespace Sig
{
	///
	/// \section tEffectRefEntityDef
	///

	register_rtti_factory( tEffectRefEntityDef, true )

	tEffectRefEntityDef::tEffectRefEntityDef( )
		: mReferenceFile( 0 )
		, mLODSettings( 0 )
		, mStartupFlags( 0 )
		, mLoopCount( -1 )
		, mPreLoadTime( 0.f )
		, mStateMask( 0xFFFF )
		, mSurfaceOrientation( cSurfaceOrientationSigml )
	{
	}

	tEffectRefEntityDef::tEffectRefEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
	{
	}

	tEffectRefEntityDef::~tEffectRefEntityDef( )
	{
		delete mLODSettings;
	}

	b32 tEffectRefEntityDef::fOnSubResourcesLoaded( const tResource& ownerResource )
	{
		const tResourcePtr& resource = mReferenceFile->fGetResourcePtr( );
		const FX::tFxFile* fxFile = resource->fCast<FX::tFxFile>( );
		if( !fxFile )
			return false;
		mBounds = Math::tAabbf(-1.f,+1.f);//fxFile->mBounds.fTransform( mObjectToLocal );
		return true;
	}

	void tEffectRefEntityDef::fCollectEntities( const tCollectEntitiesParams& paramsParent ) const
	{
		tEntityCreationFlags creationFlags = paramsParent.mCreationFlags | fCreationFlags( );
		
		const tResourcePtr& resource = mReferenceFile->fGetResourcePtr( );
		FX::tFxFileRefEntity* entity = NEW_TYPED( FX::tFxFileRefEntity )( resource, mLoopCount, mStartupFlags & cStartupFlagLocalSpace, mStartupFlags & cStartupFlagNeverCull, mPreLoadTime, this, creationFlags.mVisibilitySet.fVisibilitySet( ) );
		fApplyPropsAndSpawnWithScript( *entity, tCollectEntitiesParams( paramsParent.mParent, creationFlags ) );

		if( mLODSettings )
		{
			sigassert( mLODSettings->mFadeOverride || mLODSettings->mFadeSetting );
			Gfx::tRenderableEntity::fSetFadeSettings( 
				*entity, 
				( Gfx::tRenderableEntity::tFadeSetting )mLODSettings->mFadeSetting, 
				Gfx::tRenderableEntity::cFadeNever, // Fade settings not configurable through SigFx
				mLODSettings->mFadeOverride );

			if( mLODSettings->mLODMediumDistanceOverride || mLODSettings->mLODFarDistanceOverride )
				Gfx::tRenderableEntity::fSetLODDists( *entity, mLODSettings->mLODMediumDistanceOverride, mLODSettings->mLODFarDistanceOverride );
		}
	}
}


namespace Sig { namespace FX
{

	void tFxFileRefEntity::fDetachAllFromParent( tEntity& parent )
	{
		tFxFileRefEntity* fx = parent.fDynamicCast< tFxFileRefEntity >( );
		if( fx )
		{
			fx->fSetUserManagedDelete( false );
			fx->fFinishUp( );
		}
		else
		{
			for( u32 i = 0; i < parent.fChildCount( ); ++i )
				fDetachAllFromParent( *parent.fChild( i ) );
		}
	}

	devvar( bool, Debug_Effects_DisableParticles, false );
	devvar( bool, Debug_Effects_PrintNamesWhenPlayed, false );
	
	devvar( bool, Debug_Effects_UseLockedToParentStateChangeFixOverride, false );
	devvar( bool, Debug_Effects_UseLockedToParentStateChangeFix, false );

	tFxFileRefEntity::tFxFileRefEntity( 
		const tResourcePtr& fxResource, 
		s32 playCount, 
		b32 local, 
		b32 neverCull, 
		f32 preloadTime, 
		const tEffectRefEntityDef* entityDef, 
		Gfx::tVisibilitySetRef& visibility )
		: mFxResource( fxResource )
		, mEntityDef( entityDef )
		, mSpawnCount( 0 )
		, mAlive( false )
		, mRemoveable( false )
		, mLoop( playCount < 0 )
		, mOriginalPlayCount( playCount )
		, mLocal( local )
		, mPaused( false )
		, mQuickFire( false )
		, mFinishUpCalled( false )
		, mFadeEmissionRatesOutOverTime( false )
		, mUserManagedDelete( false )
		, mNeverCull( neverCull )
		, pad1( false )
		, pad2( false )
		, mCurrentTime( 0.f )
		, mLifetime( 5.f )
		, mPlayCount( playCount )
		, mPreloadTime( preloadTime )
		, mTimeToFadeOutOver( 1.f )
		, mFadeOutOverTimer( 0.f )
		, mEmissionPercent( 1.f )
		, mOriginalParentRelative( Math::tMat3f::cIdentity )
	{
		mVisibilitySet.fReset( &visibility );

		mOnReload.fFromMethod<tFxFileRefEntity, &tFxFileRefEntity::fOnReload>( this );
		mFxResourceLoaded.fFromMethod< tFxFileRefEntity, &tFxFileRefEntity::fOnFxFileResourceLoaded >( this );

		if( entityDef )
		{
			fMoveTo( entityDef->mObjectToLocal );

			fSetStateMask( entityDef->mStateMask );

			if( !fStateEnabled( 0 ) )
				fPause( true );
		}

		sigassert( !mFxResource.fNull( ) );
		mFxResource->fLoadDefault( this );
		mFxResource->fCallWhenLoaded( mFxResourceLoaded );
	}

	tFxFileRefEntity::~tFxFileRefEntity( )
	{
	}

	void tFxFileRefEntity::fBindToParent( b32 bind )
	{
		if( bind )
		{
			if( !fLockedToParent( ) )
			{
				fSetLockedToParent( true );
				fSetParentRelativeXform( mOriginalParentRelative );
			}
		}
		else
		{
			if( fLockedToParent( ) )
			{
				fSetLockedToParent( false );
			}
		}
	}

	void tFxFileRefEntity::fOnReload( tResource& fxResource, b32 success )
	{
		if( mSpawnCount == 0 )
			return;

		fClearChildren( );
		mParticleSystems.fSetCount( 0 );
		mAttractors.fSetCount( 0 );
		mMeshSystems.fSetCount( 0 );
		mLights.fSetCount( 0 );
		fTriageChildrenByType( );
		fOnSpawnInternal( );
	}

	void tFxFileRefEntity::fOnFxFileResourceLoaded( tResource& theResource, b32 success )
	{
		sigassert( mFxResource == &theResource );

		const tFxFile* fxFile = mFxResource->fCast< tFxFile >( );
		if( !fxFile )
			return;

		tEntityCreationFlags flags;
		flags.mVisibilitySet.fSetVisibilitySet( mVisibilitySet.fGetRawPtr( ) );
		fxFile->fCollectEntities( tCollectEntitiesParams( *this, flags ) );

		fTriageChildrenByType( );

		mLifetime = fxFile->fLifetime( );
		if( fRandomStartTime( fxFile->mFlags ) && mPreloadTime == 0.f )
			mCurrentTime = tRandom::fObjectiveRand( ).fFloatZeroToValue( mLifetime );
		else
			mCurrentTime = mPreloadTime;
	}

	void tFxFileRefEntity::fTriageChildrenByType( )
	{
		for( u32 i = 0; i < fChildCount( ); ++i )
		{
			tParticleSystem* particleSystem = fChild( i )->fDynamicCast< tParticleSystem >( );
			if( particleSystem )
			{
				mParticleSystems.fPushBack( tParticleSystemPtr( particleSystem ) );
				continue;
			}

			tParticleAttractor* attractor = fChild( i )->fDynamicCast< tParticleAttractor >( );
			if( attractor )
			{
				mAttractors.fPushBack( tParticleAttractorPtr( attractor ) );
				continue;
			}

			tMeshSystem* meshSystem = fChild( i )->fDynamicCast< tMeshSystem >( );
			if( meshSystem )
			{
				mMeshSystems.fPushBack( tMeshSystemPtr( meshSystem ) );
				continue;
			}

			tAnimatedLightEntity* light = fChild( i )->fDynamicCast< tAnimatedLightEntity >( );
			if( light )
			{
				mLights.fPushBack( tAnimatedLightEntityPtr( light ) );
				continue;
			}
		}
	}

	const tFilePathPtr& tFxFileRefEntity::fResourcePath( ) const
	{
		return mFxResource ? mFxResource->fGetPath( ) : tFilePathPtr::cNullPtr;
	}

	void tFxFileRefEntity::fOnSpawn( )
	{
		tEntity::fOnSpawn( );

		if( Debug_Effects_DisableParticles )
		{
			mAlive = true;
			mLifetime = 0.5f;
			mCurrentTime = 0.f;
			mLoop = false;
			mPlayCount = 1;
		}

		if( mAlive )
			return;
		mAlive = true;
		fOnPause( false );
		fOnSpawnInternal( );

		if( Debug_Effects_PrintNamesWhenPlayed )
		{
			if( mFxResource )
				log_line( 0, "tFxFileRefEntity::fOnSpawn - [" << mFxResource->fGetPath( ) << "]" );
		}

		if( !fStateEnabled( 0 ) )
		{
			mOriginalParentRelative = fParentRelative( );
			fBindToParent( false );
		}
	}

	void tFxFileRefEntity::fOnSpawnInternal( )
	{
#ifdef target_game // we only do dynamic reloading this way in-game
		if( mSpawnCount == 0 )
			mFxResource->fCallWhenLoaded( mOnReload );
		++mSpawnCount;
#endif//target_game

		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
		{
			mParticleSystems[ i ]->fSetPlayCount( mPlayCount );
			mParticleSystems[ i ]->fSetLocalSpace( mLocal );
			mParticleSystems[ i ]->fSetLifetime( mLifetime );
			mParticleSystems[ i ]->fSetAttractors( mAttractors );
			mParticleSystems[ i ]->fStop( mPaused );
			mParticleSystems[ i ]->fSetEmissionPercent( mEmissionPercent );
			mParticleSystems[ i ]->fSetNeverCull( mNeverCull );
			if( mCurrentTime > 0.f )
				mParticleSystems[ i ]->fFastUpdate( mCurrentTime, true );		// preload us!			
		}
		for( u32 i = 0; i < mMeshSystems.fCount( ); ++i )
		{
			mMeshSystems[ i ]->fSetLifetime( mLifetime );
			mMeshSystems[ i ]->fSetAttractors( mAttractors );
			mMeshSystems[ i ]->fSetCurrentTime( mCurrentTime );
			mMeshSystems[ i ]->fStop( mPaused );
			mMeshSystems[ i ]->fSetEmissionPercent( mEmissionPercent );
			if( mCurrentTime > 0.f )
				mMeshSystems[ i ]->fFastUpdate( mCurrentTime, true );		// preload us!	
		}
	}

	void tFxFileRefEntity::fOnPause( b32 paused )
	{
		if( paused )
			fRunListRemove( cRunListActST );
		else
			fRunListInsert( cRunListActST );
		tEntity::fOnPause( paused );
	}

	void tFxFileRefEntity::fOnParentDelete( )
	{
		if( ( mAlive || mPaused || !mRemoveable ) && !mFinishUpCalled )
			fFinishUp( );
		
		fPause( false );

		sigassert( fParent( ) != &fSceneGraph( )->fRootEntity( ) );
		fMoveTo( fParent( )->fObjectToWorld( ).fGetTranslation( ) );
	
		tEntity::fOnParentDelete( );
	}

	void tFxFileRefEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

	b32	tFxFileRefEntity::fReadyForDeletion( )
	{
		return mRemoveable;
	}

	void tFxFileRefEntity::fStateMaskEnable( u32 index )
	{		
		if( fStateEnabled( index ) )
		{
			fBindToParent( true );
			fPause( false );
		}
		else
		{
			if( !mPaused )
			{
				fFinishUp( );
			}
		}
	}

	void tFxFileRefEntity::fFastForward( f32 amount )
	{
		mCurrentTime += amount;
		if( mCurrentTime > mLifetime )
			mCurrentTime = 0.f;

		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fSetCurrentTime( mCurrentTime );
		for( u32 i = 0; i < mMeshSystems.fCount( ); ++i )
			mMeshSystems[ i ]->fSetCurrentTime( mCurrentTime );
	}

	void tFxFileRefEntity::fPause( b32 pause )
	{
		mPaused = pause;
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fStop( mPaused );
		for( u32 i = 0; i < mMeshSystems.fCount( ); ++i )
			mMeshSystems[ i ]->fStop( mPaused );
	}

	void tFxFileRefEntity::fReset( )
	{
		mQuickFire = true;
		mCurrentTime = 0.f;
		mAlive = true;
		mRemoveable = false;
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fQuickReset( );
		for( u32 i = 0; i < mMeshSystems.fCount( ); ++i )
			mMeshSystems[ i ]->fQuickReset( );
	}

	void tFxFileRefEntity::fFinishUp( )
	{
		mAlive = false;
		mLoop = false;
		mFinishUpCalled = true;
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fFinishUp( true );
		for( u32 i = 0; i < mMeshSystems.fCount( ); ++i )
			mMeshSystems[ i ]->fFinishUp( true );
	}

	void tFxFileRefEntity::fFadeEmissionRateOutOverTime( f32 timeToFadeOver )
	{
		mFadeEmissionRatesOutOverTime = true;
		mTimeToFadeOutOver = timeToFadeOver;
		mFadeOutOverTimer = 0.f;
	}

	void tFxFileRefEntity::fSetAsScreenSpaceParticleSystem( b32 screenSpace )
	{
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fSetLodFactor( 0.f );
	}

	void tFxFileRefEntity::fSetInvisible( b32 set )
	{
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fSetInvisible( set );
	}

	void tFxFileRefEntity::fSetEmissionPercent( f32 percent )
	{
		mFadeEmissionRatesOutOverTime = false;
		mEmissionPercent = percent;

		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fSetEmissionPercent( percent );
		for( u32 i = 0; i < mMeshSystems.fCount( ); ++i )
			mMeshSystems[ i ]->fSetEmissionPercent( percent );
	}

	void tFxFileRefEntity::fSetEmissionPercentByName( f32 percent, const tStringPtr& emitterName )
	{
		mFadeEmissionRatesOutOverTime = false;
		mEmissionPercent = percent;

		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
		{
			if( emitterName.fLength( ) == 0 || emitterName == mParticleSystems[ i ]->fParticleSystemName( ) )
				mParticleSystems[ i ]->fSetEmissionPercent( percent );
		}
		for( u32 i = 0; i < mMeshSystems.fCount( ); ++i )
		{
			mMeshSystems[ i ]->fSetEmissionPercent( percent );
		}
	}

	void tFxFileRefEntity::fEnableAttractor( const tStringPtr& attractorName, b32 enable, const tStringPtr& emitterName )
	{
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
		{
			if( emitterName.fLength( ) == 0 || emitterName == mParticleSystems[ i ]->fParticleSystemName( ) )
			{
				for( u32 j = 0; j < mAttractors.fCount( ); ++j )
				{
					if( mAttractors[ j ]->fAttractorName( ) == attractorName )
					{
						if( enable )
							mAttractorAddList.fPushBack( tPair< u32, u32 >( i, j ) );
						else
							mAttractorIgnoreRemovalList.fPushBack( tPair< u32, u32 >( i, j ) );
					}
				}
			}
		}
	}

	void tFxFileRefEntity::fOverrideSystemAlpha( f32 alpha, const tStringPtr& emitterName )
	{
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
		{
			if( emitterName.fLength( ) == 0 || emitterName == mParticleSystems[ i ]->fParticleSystemName( ) )
				mParticleSystems[ i ]->fSetRgbaTint( Math::tVec4f( 1.f, alpha ) );
		}			
	}

	void tFxFileRefEntity::fActST( f32 dt )
	{
		profile( cProfilePerfParticlesFXThinkST );

		if( Debug_Effects_UseLockedToParentStateChangeFixOverride )
			fBindToParent( Debug_Effects_UseLockedToParentStateChangeFix );

		if( mPaused )
			return;

		mCurrentTime += dt;

		for( u32 i = 0; i < mAttractorAddList.fCount( ); ++i )
		{
			tPair< u32, u32 >pair = mAttractorAddList[ i ];
			mParticleSystems[ pair.mA ]->fAddAttractorIgnoreId( mAttractors[ pair.mB ]->fId( ) );
			mParticleSystems[ pair.mA ]->fSetAttractors( mAttractors );
		}mAttractorAddList.fSetCount( 0 );

		for( u32 i = 0; i < mAttractorIgnoreRemovalList.fCount( ); ++i )
		{
			tPair< u32, u32 >pair = mAttractorIgnoreRemovalList[ i ];
			mParticleSystems[ pair.mA ]->fRemoveAttractorIgnoreId( mAttractors[ pair.mB ]->fId( ) );
			mParticleSystems[ pair.mA ]->fSetAttractors( mAttractors );
		}mAttractorIgnoreRemovalList.fSetCount( 0 );

		if( mCurrentTime > mLifetime && mAlive )
		{
			mCurrentTime = mCurrentTime - mLifetime;

			if( !mLoop && !mQuickFire )
			{
				if( --mPlayCount <= 0 )
				{
					fFinishUp( );
				}
			}
		}
			
		f32 delta = fClamp( mCurrentTime / mLifetime, 0.f, 1.f );
		
		for( u32 i = 0; i < mAttractors.fCount( ); ++i )
			mAttractors[ i ]->fUpdateGraphValues( delta );

		for( u32 i = 0; i < mLights.fCount( ); ++i )
			mLights[ i ]->fUpdateGraphValues( delta );

		if( mFadeEmissionRatesOutOverTime )
		{
			mFadeOutOverTimer += dt;
			if( mFadeOutOverTimer > mTimeToFadeOutOver )
			{
				mFadeOutOverTimer = mTimeToFadeOutOver;
				mFadeEmissionRatesOutOverTime = false;
			}
			
			f32 percent = 1.f - ( mFadeOutOverTimer / mTimeToFadeOutOver );
			for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
				mParticleSystems[ i ]->fSetEmissionPercent( percent*percent );
		}

		if( ( !mAlive || mFinishUpCalled ) && !mRemoveable )
		{
			// check to see if all our systems are also dead, then remove ourselves
			mRemoveable = true;
			for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			{
				if( !mParticleSystems[ i ]->fReadyForRemoval( ) )
				{
					mRemoveable = false;
					break;
				}
			}

			if( mRemoveable && !mUserManagedDelete )
				fDelete( );
		}
	}

	void tFxFileRefEntity::fSetVisibilitySet( Gfx::tVisibilitySetRef* set )
	{
		mVisibilitySet.fReset( set );
	}

	void tFxFileRefEntity::fSetNeverCull( b32 neverCull )
	{
		mNeverCull = neverCull;
		for( u32 i = 0; i < mParticleSystems.fCount( ); ++i )
			mParticleSystems[ i ]->fSetNeverCull( mNeverCull );	
		//for( u32 i = 0; i < mMeshSystems.fCount( ); ++i )
		//	mMeshSystems[ i ]->fSetNeverCull( mNeverCull );	
	}

	void tFxFileRefEntity::fSetNeverCullRecursive( b32 neverCull, tEntity& root )
	{
		tFxFileRefEntity* fx = root.fDynamicCast<tFxFileRefEntity>( );
		if( fx )
			fx->fSetNeverCull( neverCull );

		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetNeverCullRecursive( neverCull, *root.fChild( i ) );
	}


	Math::tAabbf tFxFileRefEntity::fGetFxSystemBox( ) const
	{
		const tFxFile* fxFile = mFxResource->fCast< tFxFile >( );
		sigassert( fxFile );
		return fxFile->mBounds;
	}

	void tFxFileRefEntity::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tFxFileRefEntity, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

			classDesc
				.Func(_SC("FastForward"),					&tFxFileRefEntity::fFastForward)
				.Func(_SC("Pause"),							&tFxFileRefEntity::fPause)
				.Func(_SC("Reset"),							&tFxFileRefEntity::fReset)
				.Func(_SC("FinishUp"),						&tFxFileRefEntity::fFinishUp)
				.Func(_SC("FadeEmissionRateOutOverTime"),	&tFxFileRefEntity::fFadeEmissionRateOutOverTime)
				.Func(_SC("SetLoop"),						&tFxFileRefEntity::fSetLoop)
				.Prop(_SC("Alive"),							&tFxFileRefEntity::fAlive)
				.Prop(_SC("Removeable"),					&tFxFileRefEntity::fRemoveable)
				.Func(_SC("SetEmissionPercent"),			&tFxFileRefEntity::fSetEmissionPercent)
				.Func(_SC("EnableAttractor"),				&tFxFileRefEntity::fEnableAttractor)
				.Func(_SC("OverrideSystemAlpha"),			&tFxFileRefEntity::fOverrideSystemAlpha)
				;

			vm.fNamespace(_SC("Effects")).Bind(_SC("FxSystem"), classDesc);
		}
		{
			Sqrat::DerivedClass<tPausedFxLogic, tLogic, Sqrat::NoCopy<tPausedFxLogic> > classDesc( vm.fSq( ) );

			classDesc
				.Func(_SC("Pause"),	&tPausedFxLogic::fPause)
				.Func(_SC("Reset"),	&tPausedFxLogic::fReset)
				;

			vm.fNamespace(_SC("Effects")).Bind(_SC("PausedFx"), classDesc);
		}
	}
}}

namespace Sig { namespace FX
{

	void tFxSystemsArray::fReset( )
	{
		for( u32 i = 0; i < mSystems.fCount( ); ++i )
			mSystems[ i ]->fReset( );
	}
	void tFxSystemsArray::fPause( b32 pause )
	{
		for( u32 i = 0; i < mSystems.fCount( ); ++i )
			mSystems[ i ]->fPause( pause );
	}
	void tFxSystemsArray::fSetEmissionPercent( f32 percent )
	{
		for( u32 i = 0; i < mSystems.fCount( ); ++i )
			mSystems[ i ]->fSetEmissionPercent( percent );
	}
	void tFxSystemsArray::fFinishUp( )
	{
		for( u32 i = 0; i < mSystems.fCount( ); ++i )
			mSystems[ i ]->fFinishUp( );
	}
	void tFxSystemsArray::fClear( )
	{
		for( u32 i = 0; i < mSystems.fCount( ); ++i )
			mSystems[ i ]->fSetUserManagedDelete( false );
		mSystems.fSetCount( 0 );
	}


	tAddPausedFxSystem::tAddPausedFxSystem( tFxSystemsArray& fx, u32 tagsToAdd ) 
		: mFxSystems( fx )
		, mTagsToAdd( tagsToAdd )
	{
	}

	b32 tAddPausedFxSystem::operator( ) ( tEntity& e ) const
	{
		FX::tFxFileRefEntity *fx = e.fDynamicCast< FX::tFxFileRefEntity >( );

		if( fx )
		{
			if( mTagsToAdd != ~0 )
				fx->fAddGameTags( mTagsToAdd );
			fx->fSetUserManagedDelete( true );
			fx->fPause( true );
			mFxSystems.mSystems.fPushBack( FX::tFxFileRefEntityPtr( fx ) );
		}

		return false;
	}
}}
