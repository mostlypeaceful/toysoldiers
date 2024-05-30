#ifndef __tFxFileRefEntity__
#define __tFxFileRefEntity__
#include "tEntityDef.hpp"
#include "tResource.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "FX/tParticleAttractor.hpp"
#include "FX/tParticleSystem.hpp"
#include "FX/tMeshSystem.hpp"
#include "tAttachmentEntity.hpp"


namespace Sig
{
	class base_export tEffectRefEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tEffectRefEntityDef, 0x7CF40C88 );

		enum tStartupFlags
		{
			cStartupFlagLocalSpace = ( 1 << 0 ),
		};

		// This is for use with tEntity::fSpawnSurfaceFxSystem
		enum tSurfaceOrientation
		{
			cSurfaceOrientationSigml, //unchanged, spawns as placed in the sigml.
			cSurfaceOrientationYUp,
			cSurfaceOrientationSurfaceNormal,
			cSurfaceOrientationInputNormal,
		};

	public:
		tLoadInPlaceResourcePtr*	mReferenceFile;
		tSceneLODSettings*			mLODSettings;
		u32							mStartupFlags;
		s32							mLoopCount;
		f32							mPreLoadTime;
		u16							mStateMask;
		u16							mSurfaceOrientation;

	public:
		tEffectRefEntityDef( );
		tEffectRefEntityDef( tNoOpTag );
		~tEffectRefEntityDef( );
		virtual b32 fHasRenderableBounds( ) const { return true; }
		virtual b32  fOnSubResourcesLoaded( const tResource& ownerResource );
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};
}


namespace Sig { namespace FX
{
	class base_export tFxFileRefEntity : public tStateableEntity
	{
		define_dynamic_cast( tFxFileRefEntity, tStateableEntity );
	private:

		tGrowableArray< tParticleSystemPtr > mParticleSystems;
		tGrowableArray< tParticleAttractorPtr > mAttractors;
		tGrowableArray< tMeshSystemPtr > mMeshSystems;
		tGrowableArray< tPair< u32, u32 > > mAttractorIgnoreRemovalList;
		tGrowableArray< tPair< u32, u32 > > mAttractorAddList;

		tResourcePtr mFxResource;
		const tEffectRefEntityDef* mEntityDef;
		tResource::tOnLoadComplete::tObserver mFxResourceLoaded;

		tResource::tOnLoadComplete::tObserver mOnReload;
		u32 mSpawnCount;

		b8		mAlive;
		b8		mRemoveable;
		b8		mLoop;
		b8		mLocal;

		b8		mPaused;
		b8		mQuickFire;
		b8		mFinishUpCalled;
		b8		mFadeEmissionRatesOutOverTime;

		b8		mUserManagedDelete;
		b8		pad0;
		b8		pad1;
		b8		pad2;

		f32		mCurrentTime;
		f32		mLifetime;
		s32		mPlayCount;
		s32		mOriginalPlayCount;
		f32		mPreloadTime;

		f32		mTimeToFadeOutOver;
		f32		mFadeOutOverTimer;	
		f32		mEmissionPercent;

		Math::tMat3f mOriginalParentRelative;
		void fBindToParent( b32 bind );

	public:
		static void fDetachAllFromParent( tEntity& parent );

	public:
		tFxFileRefEntity( const tResourcePtr& fxResource, s32 playCount=-1, b32 local = false, f32 preloadTime = 0.f, const tEffectRefEntityDef* entityDef = 0 );
		~tFxFileRefEntity( );
		const tResourcePtr& fFxResource( ) const { return mFxResource; }
		virtual void fPropagateSkeleton( tAnimatedSkeleton& skeleton );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );
		virtual void fOnParentDelete( );
		virtual b32	 fReadyForDeletion( );


		//System Wide Changes
		void fFastForward( f32 amount );
		
		void fPause( b32 pause );
		void fReset( );
		void fFinishUp( );
		void fFadeEmissionRateOutOverTime( f32 timeToFadeOver );
		void fSetAsScreenSpaceParticleSystem( b32 screenSpace );

		void fSetLoop( b32 loop ) { mLoop = loop; }

		Math::tAabbf fGetFxSystemBox( ) const;
		b32 fAlive( ) const { return mAlive; }
		b32 fRemoveable( ) const { return mRemoveable; }
		b32 fUserManagedDelete( ) const { return mUserManagedDelete; }
		void fSetUserManagedDelete( b32 set ) { mUserManagedDelete = set; }

		//Per Emitter Changes
		void fSetEmissionPercent( f32 percent );
		void fSetEmissionPercentByName( f32 percent, const tStringPtr& emitterName = tStringPtr( "" ) );
		void fEnableAttractor( const tStringPtr& attractorName, b32 enable = true, const tStringPtr& emitterName = tStringPtr("") );
		void fOverrideSystemAlpha( f32 alpha, const tStringPtr& emitterName = tStringPtr( "" ) );
		
		virtual void fStateMaskEnable( u32 index );
		
		const tEffectRefEntityDef& fEntityDef( ) const { sigassert( mEntityDef ); return *mEntityDef; }

		void fSetInvisible( b32 set );
	private:
		void fOnSpawnInternal( );
		void fOnReload( tResource& fxResource, b32 success );
		void fOnFxFileResourceLoaded( tResource& theResource, b32 success );
		void fTriageChildrenByType( );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tFxFileRefEntity );


	// Convenient types for adding lots of paused systems
	class tFxSystemsArray
	{
		friend class tAddPausedFxSystem;
		tGrowableArray< FX::tFxFileRefEntityPtr > mSystems;
	public:
		u32 fSystemCount( ) const { return mSystems.fCount( ); }
		void fReset( );
		void fPause( b32 pause );
		void fSetEmissionPercent( f32 percent );
		void fFinishUp( );
		void fClear( );

		void fSafeCleanup( )
		{
			fPause( false );
			fFinishUp( );
			fClear( );
			// Have a nice day :)
		}
	};

	class tAddPausedFxSystem
	{
	public:
		tFxSystemsArray& mFxSystems;
		u32 mTagsToAdd;

		tAddPausedFxSystem( tFxSystemsArray& fx, u32 tagsToAdd = ~0 );
		b32 operator( ) ( tEntity& e ) const;
	};

	class tPausedFxLogic : public tLogic
	{
		define_dynamic_cast( tPausedFxLogic, tLogic );
	public:
		virtual void fOnSpawn( ) 
		{ 
			tAddPausedFxSystem adder( mFx ); 
			adder( *fOwnerEntity( ) );
			fOwnerEntity( )->fForEachDescendent( adder ); 
		}
		
		virtual void fOnDelete( )	{ mFx.fSafeCleanup( ); }

		void fPause( b32 pause )	{ mFx.fPause( pause ); }
		void fReset( )				{ mFx.fReset( ); }

		tFxSystemsArray mFx;
	};

}
}

#endif//__tFxFileRefEntity__

