#ifndef __tRenderableEntity__
#define __tRenderableEntity__
#include "tDisplayList.hpp"
#include "tSpatialEntity.hpp"
#include "tCamera.hpp"

namespace Sig { namespace Gfx
{
	class tViewport;
	class tWorldSpaceDisplayList;

	class base_export tRenderableEntity : public tSpatialEntity, public tRenderInstance
	{
		define_dynamic_cast( tRenderableEntity, tSpatialEntity );
	public:
		enum tFlags
		{
			cFlagInvisible							= ( 1 << 0 ),
			cFlagCastShadow							= ( 1 << 1 ),
			cFlagReceiveShadow						= ( 1 << 2 ),
			cFlagDisallowIndirectColorControllers	= ( 1 << 3 ),
			cFlagDisabled							= ( 1 << 4 ),
			cFlagUseEffectSpatialSet				= ( 1 << 5 ),
		};
		enum tFadeSetting
		{
			cFadeNever,
			cFadeNear,
			cFadeMedium,
			cFadeFar,
			cFadeSettingCount
		};
		static void fSetObjectSpaceBoxOverride( tEntity& root, const Math::tAabbf& newObjSpaceBox );
		static void fSetGlobalFadeSetting( tFadeSetting slot, f32 distance );
	public:
		static const u32 cSpatialSetIndex;
		static const u32 cHeightFieldSpatialSetIndex;
		static const u32 cEffectSpatialSetIndex;
		static void fAddRenderableSpatialSetIndices( tDynamicArray<u32>& ids, b32 includeEffects = false );
	private:
		f32 mCameraDepthOffset;
		u32	mViewportMask;
		u32	mFlags;
		f32 mFadeOutDistance;
		const tEntity* mFadeOutRoot;
	public:
		tRenderableEntity( );
		explicit tRenderableEntity( const tRenderBatchPtr& batch );
		tRenderableEntity( const tRenderBatchPtr& batch, const Math::tAabbf& objectSpaceBox );
		virtual void			fComputeDisplayStats( tDisplayStats& displayStatsOut ) const;
		inline tDrawCall		fGetDrawCall( const tCamera& camera, f32 fadeAlpha = 1.f ) const { return tDrawCall( *this, fCameraDepth( camera, fadeAlpha ), fadeAlpha ); }
		inline tDrawCall		fGetDrawCall( f32 overrideCamDepth ) const	{ return tDrawCall( *this, overrideCamDepth ); }
		inline Math::tVec3f		fObjectCenterInWorld( ) const				{ return tSpatialEntity::fObjectToWorld( ).fGetTranslation( ); }
		f32						fCameraDepth( const tCamera& camera, f32 fadeAlpha ) const;
		inline f32				fCameraDepthOffset( ) const { return mCameraDepthOffset; }
		void					fSetCameraDepthOffset( f32 newOffset ) { mCameraDepthOffset = newOffset; }
		void					fAddToDisplayList( const tViewport& viewport, tWorldSpaceDisplayList& displayList );

		virtual u32				fSpatialSetIndex( ) const { return fUseEffectSpatialSet( ) ? cEffectSpatialSetIndex : cSpatialSetIndex; }
		virtual void			fApplyCreationFlags( const tEntityCreationFlags& creationFlags );
		virtual b32				fIntersects( const Math::tFrustumf& v ) const;
		virtual b32				fIntersects( const Math::tAabbf& v ) const;
		virtual b32				fIntersects( const Math::tObbf& v ) const;
		virtual b32				fIntersects( const Math::tSpheref& v ) const;
		virtual Math::tVec4f	fRI_DynamicVec4( const tStringPtr& varName, u32 viewportIndex ) const;

		inline void				fSetRgbaTint( const Math::tVec4f& rgbaTint ) { tRenderInstance::fSetRgbaTint( rgbaTint ); }

		inline void				fSetDisabled( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagDisabled ) : fClearBits( mFlags, cFlagDisabled ) ); }
		inline b32				fDisabled( ) const { return mFlags & cFlagDisabled; }
		inline void				fSetInvisible( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagInvisible ) : fClearBits( mFlags, cFlagInvisible ) ); }
		inline b32				fInvisible( ) const { return mFlags & cFlagInvisible; }
		inline b32				fDisabledOrInvisible( ) const { return mFlags & (cFlagInvisible | cFlagDisabled ); }
		inline void				fSetCastsShadow( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagCastShadow ) : fClearBits( mFlags, cFlagCastShadow ) ); }
		inline b32				fCastsShadow( ) const { return mFlags & cFlagCastShadow; }
		inline void				fSetReceivesShadow( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagReceiveShadow ) : fClearBits( mFlags, cFlagReceiveShadow ) ); }
		inline b32				fReceivesShadow( ) const { return mFlags & cFlagReceiveShadow; }
		inline void				fSetDisallowIndirectColorControllers( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagDisallowIndirectColorControllers ) : fClearBits( mFlags, cFlagDisallowIndirectColorControllers ) ); }
		inline b32				fDisallowIndirectColorControllers( ) const { return mFlags & cFlagDisallowIndirectColorControllers; }
		inline void				fSetUseEffectSpatialSet( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagUseEffectSpatialSet ) : fClearBits( mFlags, cFlagUseEffectSpatialSet ) ); }
		inline b32				fUseEffectSpatialSet( ) const { return mFlags & cFlagUseEffectSpatialSet; }
		inline u32				fFlags( ) const { return mFlags; }
		inline void				fSetFlags( u32 flags ) { mFlags = flags; }
		inline void				fAddFlags( u32 flags ) { mFlags = fSetBits( mFlags, flags ); }
		inline void				fRemoveFlags( u32 flags ) { mFlags = fClearBits( mFlags, flags ); }
		inline u32				fViewportMask( ) const { return mViewportMask; }
		inline void				fSetViewportMask( u32 mask ) { mViewportMask = mask; }
		inline void				fEnableViewport( u32 ithViewport, b32 enable ) { mViewportMask = enable ? fSetBits( mViewportMask, 1 << ithViewport ) : fClearBits( mViewportMask, 1 << ithViewport ); }

		inline f32				fFadeOutDistance( ) const { return mFadeOutDistance; }
		inline const tEntity*	fFadeOutRoot( ) const { return mFadeOutRoot; }

		inline b32				fShouldBeRendered( u32 ithViewport ) const
		{
			return !fTestBits(mFlags,cFlagDisabled|cFlagInvisible) && ( fRgbaTint( ).w > 0.f ) && ( mViewportMask & ( 1 << ithViewport ) );
		}

		f32 fComputeFadeAlpha( tScreen& screen ) const;
		inline f32 fComputeFadeAlpha( const tCamera& camera ) const;
		void fSetFadeSettingOnThis( tEntity& realRoot, tFadeSetting fadeSetting, f32 explicitOverride=0.f );
		static void fSetFadeSetting( tEntity& realRoot, tFadeSetting fadeSetting, f32 explicitOverride=0.f );
	private:
		static void fSetFadeSetting( tEntity& realRoot, tEntity& currentRoot, tFadeSetting fadeSetting, f32 explicitOverride );
	public:
		static void fSetDisallowIndirectColorControllers( tEntity& root, b32 set );
		static void	fSetDisabled( tEntity& root, b32 set );
		static void fSetRgbaTint( tEntity& root, const Math::tVec4f& tint );
		static void fSetRgbaTintRespectNoIndirectTinting( tEntity& root, const Math::tVec4f& tint, b32 ignoreChildrenWithLogic = false );
		static void	fSetInvisible( tEntity& root, b32 set );
		static void	fAddFlags( tEntity& root, u32 flags );
		static void	fRemoveFlags( tEntity& root, u32 flags );
		static void	fSetViewportMask( tEntity& root, u32 mask );
		static void	fEnableViewport( tEntity& root, u32 ithViewport, b32 enable );
		static void fSetUseEffectSpatialSet( tEntity& root, b32 set );

	private:
		void fCommonCtor( const tRenderBatchPtr& batch );
		void fConnectRenderable( const tRenderBatchPtr& batch );

	public: // script-specific
		static void					fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tRenderableEntity );

	class tRenderableEntityList : public tGrowableArray<tRenderableEntityPtr>
	{
	};

	inline f32 tRenderableEntity::fComputeFadeAlpha( const tCamera& camera ) const
	{
		f32 fadeAlpha = 1.f;

#ifdef target_game
		const f32 fadeDistance = fFadeOutDistance( );
		if( fadeDistance )
		{
			sigassert( fadeDistance > 0.f );
			sigassert( fFadeOutRoot( ) );

			const f32 distToViewPlane = 
				//mCamera.fCameraDepth( renderable->fFadeOutRoot( )->fObjectToWorld( ).fGetTranslation( ) ) * mCamera.fGetLens( ).mZoom;
				fFadeOutRoot( )->fObjectToWorld( ).fGetTranslation( ).fDistance( camera.fGetTripod( ).mLookAt );

			if( distToViewPlane >= fadeDistance )
				return 0.f;

			const f32 fadeRange = 10.f;
			const f32 beginFadeAt = fadeDistance - fadeRange;
			if( distToViewPlane > beginFadeAt )
				fadeAlpha = 1.f - ( ( distToViewPlane - beginFadeAt ) / fadeRange );
			sigassert( fInBounds( fadeAlpha, 0.f, 1.f ) );
		}
#endif//target_game

		return fadeAlpha;
	}

}}

#endif//__tRenderableEntity__
