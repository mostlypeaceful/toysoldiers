#ifndef __tParticleSystem__
#define __tParticleSystem__

#include "tParticleList.hpp"
#include "Gfx/tDynamicGeometry.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "tFxGraph.hpp"
#include "tEmitters.hpp"
#include "tMersenneGenerator.hpp"
#include "tParticleAttractor.hpp"
#include "tParticleSystemStates.hpp"

namespace Sig { namespace FX
{
	class base_export tParticleSystemDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tParticleSystemDef, 0x2D7450A8 );
	public:

		tLoadInPlaceStringPtr*			mParticleSystemName;
		tLoadInPlaceResourcePtr*		mMeshResource; // if null, uses quad particles
		tBinaryParticleSystemState		mState;
		b32								mLocalSpace;
		f32								mCameraDepthOffset;
		f32								mUpdateSpeedMultiplier;
		f32								mLodFactor;
		f32								mGhostParticleFrequency;
		f32								mGhostParticleLifetime;
		tDynamicArray< u32 >			mAttractorIgnoreIds;
		u32								mSystemFlags;
		
		tEnum< tEmitterType, u8 >		mEmitterType;
		tEnum< tParticleSortMode, u8 >	mSortMode;
		u8								mPad0,mPad1;

		Gfx::tRenderState				mRenderState;

		Gfx::tMaterial*					mMaterial;

	public:
		tParticleSystemDef( );
		tParticleSystemDef( tNoOpTag );
		~tParticleSystemDef( );

		virtual b32 fHasRenderableBounds( ) const { return true; }
		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;

		tParticleList* fCreateParticleList( ) const;
		void fAddFromToolState( const tToolParticleSystemState& state );
	};

	
	class base_export tParticleSystem : public Gfx::tRenderableEntity
	{
		define_dynamic_cast( tParticleSystem, Gfx::tRenderableEntity );
	public:
		typedef f32 (*tEmissionReductionFactor)( const tParticleSystem& psys );
		typedef void (*tSetVisibility)( tParticleSystem& psys );
	private:
		static u32 gTotalParticleSystemCount;
		static tEmissionReductionFactor gEmissionReductionFactor;
		static tSetVisibility gSetVisibility;
	public:
		static u32 fTotalParticleSystemCount( ) { return gTotalParticleSystemCount; }
		static void fSetEmissionReductionFactor( tEmissionReductionFactor erf ) { gEmissionReductionFactor = erf; }
		static void fSetSetVisibility( tSetVisibility sv ) { gSetVisibility = sv; }
	public:

		tParticleSystem( const Gfx::tMaterialPtr& particleMaterial, tBinaryParticleSystemState& defaultState );
		tParticleSystem( const tParticleSystemDef* def, Gfx::tVisibilitySetRef& visibility );
		virtual ~tParticleSystem( );

		void fCommonCtor( );

		virtual u32 fSpatialSetIndex( ) const { return cEffectSpatialSetIndex; }
		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fMoveST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		virtual void			fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;

		void					fChangeMaterial( const Gfx::tMaterialPtr& mtl );
		void					fChangeParticleList( tParticleList* newList );

		void					fClear( );
		void					fClearAllStates( );
		void					fSetCurrentTime( const f32 time );

		void					fSystemUpdate( f32 dt, const Math::tMat3f& objToWorld, const Math::tMat3f& objToWorldInv, Math::tAabbf& box );
		void					fFastUpdate( f32 curlife, b32 fromTheStart = false );

		void					fFinishUp( b32 finish ) { mFinishUp = finish; fSetVisible( true ); fSetNeverCull( true );  }
		b32						fAlive( ) const { return mAlive; }
		b32						fReadyForRemoval( ) const;

		u32						fParticleCount( ) const { return mParticles->fTotalParticleCount( ); }

		void					fSetAttractors( const tGrowableArray< tParticleAttractorPtr >& list );

		void					fSetEmitterType( tEmitterType type );
		tEmitterType			fGetEmitterType( ) const { return mEmitterType; }
		Math::tVec3f			fGetEmitterScale( ) const;
		Math::tVec3f			fEmitterTranslation( ) const;

		const tMersenneGenerator* fSystemRandomNumberGenerator( ) const { return &mSystemRand; }
	
		void					fCalculateStatistics( );

		tParticleEmitterPtr		fEmitter( ) const { return mEmitter; }
		
		FX::tBinaryParticleSystemState&				fState( ) { return mState; }
		const FX::tBinaryParticleSystemState&		fState( ) const { return mState; }

		void					fSetState( const FX::tBinaryParticleSystemState& state) { mState = state; }

		b32						fLocalSpace( ) const { return mLocalSpace; }
		void					fSetLocalSpace( b32 ls );

		void					fSetUpdateSpeedMultiplier( f32 updateSpeedMultiplier ) { mUpdateSpeedMultiplier = updateSpeedMultiplier; }
		void					fSetLodFactor( f32 lodFactor ) { mLodFactor = lodFactor; }
		void					fSetGhostParticleFrequency( f32 ghostFreq ) { mGhostParticleFrequency = ghostFreq; }
		void					fSetGhostParticleLifetime( f32 lifetimePercent ) { mGhostParticleLifetime = lifetimePercent; }

		void					fSetBlendOp( u32 blendOp );
		void					fSetBlendOpFromIndex( u32 blendOpIndex );
		void					fSetSrcBlend( u32 srcBlend );
		void					fSetDstBlend( u32 dstBlend );

		void					fSetSortMode( tParticleSortMode mode, b32 forceUpdate = true );

		f32						fUpdateSpeedMultiplier( ) const { return mUpdateSpeedMultiplier; }
		f32						fLodFactor( ) const { return mLodFactor; }
		f32						fGhostParticleFrequency( ) const { return mGhostParticleFrequency; }
		f32						fGhostParticleLifetime( ) const { return mGhostParticleLifetime; }
		tParticleSortMode		fSortMode( ) const { return mSortMode; }

		void					fSetRenderState( const Gfx::tRenderState& rs ) { mRenderState = rs; }
		const Gfx::tRenderState& fRenderState( ) const { return mRenderState; }

		b32						fHasFlag( u32 flag ) const { return fState( ).fHasFlag( flag ); }

		void					fSetLifetime( const f32 life ) { mLifetime = life; }

		void					fSetPlayCount( s32 playCnt ) { mRequiredLoopCount = playCnt; }
		b32						fIsLooping( ) const { return (mRequiredLoopCount == -1); }

		void					fSetEmissionPercent( f32 percent ) { mEmissionPercent = percent; }
		f32						fEmissionPercent( ) const { return mEmissionPercent; }

		void					fResetLastGhostParticleEmissionTime( ) { mTimeFromLastEmit = 0.f; }

		const Math::tQuatf&		fGraphRotation( ) const { return mGraphRotation; }
		void					fRemoveAttractorIgnoreId( u32 id );
		void					fAddAttractorIgnoreId( u32 id );
		void					fSyncAttractorIgnoreIds( );

		void					fQuickReset( );
		void					fStop( b32 stop );

		void					fSetParticleSystemName( const tStringPtr& name ) { mParticleSystemName = name; }
		const tStringPtr&		fParticleSystemName( ) const { return mParticleSystemName; }

		u32						fRandomSeed( ) const { return mSeed; }

		// Set this true if the particle system should be processsed and consume vram.
		//  Non-looping effects are always "visible" so they can play out and be destroyed.
		void					fSetVisible( b32 visible );

		void					fSetNeverCull( b32 neverCull ) { mNeverCull = neverCull; }

	private:

		void					fComputeEmitCountThisFrame( f32 dt, const FX::tBinaryParticleSystemState& state, const f32 systemDelta );
		void					fUpdateAllParticles( f32 dt, f32 systemDelta, const Math::tMat3f& objToWorld, const Math::tMat3f& objToWorldInv, Math::tAabbf& box );

		b32						fShouldBeCulled( ) const { return !mIsVisible && !mNeverCull; }

	private:

		tStringPtr			mParticleSystemName;
		tEmitterType		mEmitterType;
		
		f32					mUpdateSpeedMultiplier;
		f32					mLodFactor;
		f32					mGhostParticleFrequency;
		f32					mGhostParticleLifetime;
		tParticleSortMode	mSortMode;

		Gfx::tMaterialPtr					mMaterial;
		tParticleEmitterPtr					mEmitter;
		tParticleListPtr					mParticles;

		FX::tBinaryParticleSystemState mState;
		tGrowableArray< tParticleAttractorPtr > mAttractorsList;
		tGrowableArray< u32 >					mAttractorIgnoreIDs;

		tMersenneGenerator		mSystemRand;
		u32						mSeed;
		u32						mCurrentState;
		u32						mEmitCountThisFrame;
		
		b8						mAlive;
		b8						mFinishUp;
		b8						mLocalSpace;
		b8						mHasEmitted;

		f32						mLifetime;
		f32						mCurrentTime;
		f32						mParticlesEmittedPerFrame;
		f32						mTimeFromLastEmit;

		s32						mRequiredLoopCount;
		s32						mCurrentLoopCounter;
		
		b8						mFromBinaryFile;
		b8						mQuickReset;
		b8						mStopped;
		b8						mIsVisible; //do processing, consume memory, etc. if not, dont.

		b8						mNeverCull;
		b8						pad0;
		b8						pad1;
		b8						pad2;

		f32						mEmissionPercent;

		Gfx::tRenderState		mRenderState;
		Math::tAabbf			mBoxMT;
		Math::tVec3f			mLastSpawnPosition;
		Math::tVec3f			mNewSpawnPosition;
		Math::tQuatf			mGraphRotation;

		struct tParticleSystemStatistics
		{
			tParticleSystemStatistics( )
			{
				mMaxParticles = 0.f;
				mLongestLivingParticle = 0.f;
				mCurrentTotalParticles = 0.f;
				mCurrentAverageColor = Math::tVec4f::cZeroVector;
			}

			void fFrameReset( )
			{
				mCurrentTotalParticles = 0.f;
				mCurrentAverageColor = Math::tVec4f::cZeroVector;
			}

			f32				mMaxParticles;
			f32				mLongestLivingParticle;

			f32				mCurrentTotalParticles;
			Math::tVec4f	mCurrentAverageColor;

		} mStatistics;
	};

	typedef tRefCounterPtr< tParticleSystem > tParticleSystemPtr;

}}

#endif //__tParticleSystem__

