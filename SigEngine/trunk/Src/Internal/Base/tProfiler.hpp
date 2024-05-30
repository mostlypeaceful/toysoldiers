#ifndef __tProfiler__
#define __tProfiler__
#include "Time.hpp"
#include "Threads/tMutex.hpp"

namespace Sig
{
	enum tProfilerCategoryName
	{
		cProfilePerfRunListAct = 0,
		cProfilePerfRunListAnimate,
		cProfilePerfRunListCollide,
		cProfilePerfRunListPhysics,
		cProfilePerfRunListMove,
		cProfilePerfRunListEffects,
		cProfilePerfRunListThink,
		cProfilePerfRunListCamera,
		cProfilePerfRunListPreRender,
		cProfilePerfRunListCoRender,

		cProfilePerfUpdateAudio,
		cProfilePerfAudioEvent,
		cProfilePerfBuildLightLists,
		cProfilePerfBuildDisplayLists,
		cProfilePerfRenderWorld,
		cProfilePerfRenderShadowMaps,
		cProfilePerfRenderShadowMapsQuery,
		cProfilePerfRenderShadowMapsRender,
		cProfilePerfOnTickCanvas,
		cProfilePerfOnRenderCanvas,
		cProfilePerfOnTickFUI,
		cProfilePerfRenderFUI,

		cProfilePerfOnSpawn,

		cProfilePerfVehiclePhysicsRayCast,
		cProfilePerfVehiclePhysicsDynamics,
		cProfilePerfVehiclePhysicsKinematics,

		cProfilePerfScriptEventHandlers,
		cProfilePerfGameEffects,

		cProfilePerfStepBoneProxiesST,
		cProfilePerfCullDeadTracksST,
		cProfilePerfSkeletonEventsST,
		cProfilePerfWorldSpaceSCST,

		cProfilePerfGatherGroundCover,
		cProfilePerfPrepareRenderables,

		cProfilePerfParticlesMoveST,
		cProfilePerfParticlesEffectsMT,
		cProfilePerfParticlesThinkST,

		cProfilePerfParticlesMeshesMoveST,
		cProfilePerfParticlesMeshesEffectsMT,
		cProfilePerfParticlesFXThinkST,
		cProfilePerfLightEffectsMoveST,

		cProfilePerfDebrisLogicMoveST,
		cProfilePerfDebrisLogicCoRenderMT,


		cProfilePerfPhysicsTotal,
		cProfilePerfPhysicsCollisionPurge,
		cProfilePerfPhysicsCollisionTest,
		cProfilePerfPhysicsCollisionResolve,
		cProfilePerfPhysicsCollisionMaintain,
		cProfilePerfPhysicsRigidBodyStep,
		cProfilePerfPhysicsContraintStep,

		cProfilePerfNetworkTotal,
		cProfilePerfNetworkOnReceive,
		cProfilePerfNetworkSend,
		cProfilePerfNetworkSearch,
		cProfilePerfNetworkService,
		cProfilePerfNetworkVoice,


		cProfilePerfCategoryCount,

		cProfileMemMain = 0,
		cProfileMemRes,
		cProfileMemSqrat,

		cProfileMemVramSystem,
		cProfileMemVramSysTextures,
		cProfileMemVramSysGeom,
		cProfileMemVramSysShader,
		cProfileMemTexture,
		cProfileMemGeometry,
		cProfileMemFui,
		cProfileMemMovie,
		cProfileMemGroundCover,

		cProfileMemAudio,
		cProfileMemAudioSys,
		cProfileMemAudioPhys,

		cProfileMemCategoryCount,

		cProfilerCategoryNull = ~0,
	};

#if defined( sig_profile ) && defined( sig_devmenu )

	namespace Gfx
	{
		class tScreen;
		class tGeometryBufferVRamAllocator;
		class tIndexBufferVRamAllocator;
	}

	class tProfilerRenderImpl;

	class base_export tScopedProfile : public tUncopyable
	{
		u32 mCategory;
		Time::tStamp mBegin;
		f32 mTimeMs;
	public:
		explicit tScopedProfile( u32 category );
		~tScopedProfile( );
		inline u32 fCategory( ) const { return mCategory; }
		inline f32 fTimeMs( ) const { return mTimeMs; }
	};

	///
	/// \brief Global profiler class tracks profiling information for the entire application.
	/// Essential functionality exposed is to begin a profile, end a profile, and retrieve
	/// the complete set of profiling data for the current frame, or a previous frame within
	/// the window of saved frames.
	/// \note To facilitate beginning/ending a profile, use the tScopedProfile type and appropriate macros (see below).
	class base_export tProfiler
	{
		friend class tScopedProfile;
		declare_singleton_define_own_ctor_dtor( tProfiler );

	public:
		struct tPerfCategory
		{
			tStringPtr	mDisplayName;
			tStringPtr	mGroupName;
			u32			mNumberOfThreads;
			f32			mTimeMs;
			b32			mExcludeFromGroupTime;

			tPerfCategory( ) 
				: mNumberOfThreads( 1 ), mTimeMs( 0.0f ), mExcludeFromGroupTime( false ) { }
			explicit tPerfCategory( const tStringPtr& displayName, const tStringPtr& groupName, u32 numThreads = 1, b32 excludeFromGroupTime = false ) 
				: mDisplayName( displayName ), mGroupName( groupName ), mNumberOfThreads( numThreads ), mTimeMs( 0.0f ), mExcludeFromGroupTime( excludeFromGroupTime ) { }
		};

		struct tMemCategory
		{
			tStringPtr	mDisplayName;
			s32			mTotalBytes;
			u32			mParentCategory;
			explicit tMemCategory( u32 parentCategory = cProfilerCategoryNull )
				: mTotalBytes( 0 ), mParentCategory( parentCategory ) { }
			explicit tMemCategory( const tStringPtr& displayName, u32 parentCategory = cProfilerCategoryNull )
				: mDisplayName( displayName ), mTotalBytes( 0 ), mParentCategory( parentCategory ) { }
			inline f32 fInMB( ) const { return ( f32 )mTotalBytes / ( 1024.f * 1024.f ); }
		};
		
	private:

		tProfiler( );
		~tProfiler( );

		///
		/// \brief Used internally by tScopedProfile. Don't call this.
		void fEndProfile( const tScopedProfile& profile );

	public:

		///
		/// \brief Adds a performance profile category associated with the specified integer id. If an
		/// existing category exists at that slot, it is overwritten.
		void fAddPerfCategory( u32 id, const tPerfCategory& cat );

		///
		/// \brief Adds a memory profile category associated with the specified integer id. If an
		/// existing category exists at that slot, it is overwritten.
		void fAddMemCategory( u32 id, const tMemCategory& cat );

		///
		/// \brief Query the amount of memory used for a specific memory category.
		u32 fQueryMemUsage( u32 id ) const { return mMemCategories[ id ].mTotalBytes; }

		///
		/// \brief Register a VRAM geometry allocator (don't call directly, use macros below).
		void fTrackVRAMAllocator( const Gfx::tGeometryBufferVRamAllocator& allocator, const char* name, b32 remove = false );

		///
		/// \brief Register a VRAM index allocator (don't call directly, use macros below).
		void fTrackVRAMAllocator( const Gfx::tIndexBufferVRamAllocator& allocator, const char* name, b32 remove = false );

		///
		/// \brief Retrieve total consumption in bytes.
		s32 fQueryGeomAlloc( );

		///
		/// \brief Reset timing data (don't call directly, use macros below).
		void fNewFrame( );

		///
		/// \brief Track memory for specified category (don't call directly, use macros below).
		void fTrackMemory( u32 id, s32 bytesDelta );

		///
		/// \brief Set the time for specified category (don't call directly, use macros below).
		void fSetPerfTime( u32 id, f32 time );

		///
		/// \brief Render profiling data (don't call directly, use macros below).
		void fRender( Gfx::tScreen& screen );

		///
		/// \brief Should be called once after registering all categories/VRAM allocators (don't call directly, use macros below).
		void fInitialize( Gfx::tScreen& screen );

		///
		/// \brief Should be called before shutting down graphics sub-systems (don't call directly, use macros below).
		void fShutdown( );

		///
		/// \brief Access to the high level performance group names.
		tGrowableArray< tStringPtr >& fPerfGroupNames( ) { return mPerfGroupNames; }

	private:
		tProfilerRenderImpl* mRenderer;
		Threads::tCriticalSection mCritSec;
		tGrowableArray< tPerfCategory > mPerfCategories;
		tGrowableArray< tStringPtr > mPerfGroupNames;
		tGrowableArray< tMemCategory > mMemCategories;
		tGrowableArray< tPair<const Gfx::tGeometryBufferVRamAllocator*,tStringPtr> > mGeomAllocators;
		tGrowableArray< tPair<const Gfx::tIndexBufferVRamAllocator*,tStringPtr> > mIdxAllocators;
	};


#endif//defined( sig_profile ) && defined( sig_devmenu )

#if defined( sig_profile ) && defined( sig_devmenu )

#	define if_profiling( x ) x
#	define profile(category) ::Sig::tScopedProfile _auto_profiler_object_##__LINE__(category)
#	define profile_time(category, time) ::Sig::tProfiler::fInstance( ).fSetPerfTime(category, time)
#	define profile_mem(category, delta) ::Sig::tProfiler::fInstance( ).fTrackMemory(category, delta)
#	define profile_query_mem(category) ::Sig::tProfiler::fInstance( ).fQueryMemUsage(category)
#	define profile_geom_allocator(allocator, name, remove) ::Sig::tProfiler::fInstance( ).fTrackVRAMAllocator(allocator, name, remove)
#	define profile_reset( ) ::Sig::tProfiler::fInstance( ).fNewFrame( )
#	define profile_early_init( ) ::Sig::tProfiler::fInstance( )
#	define profile_init(screen) ::Sig::tProfiler::fInstance( ).fInitialize( screen )
#	define profile_shutdown( ) ::Sig::tProfiler::fInstance( ).fShutdown( )
#	define profile_render(screen) \
		::Sig::tProfiler::fInstance( ).fRender(screen); \
		profile_reset( );

#else

#	define if_profiling( x )
#	define profile(category)
#	define profile_mem(category, delta)
#	define profile_query_mem(category) 0
#	define profile_geom_allocator(allocator, name, remove)
#	define profile_reset( )
#	define profile_early_init( )
#	define profile_init(screen)
#	define profile_shutdown( )
#	define profile_render(screen)

#endif//defined( sig_profile ) && defined( sig_devmenu )

}


#endif//__tProfiler__
