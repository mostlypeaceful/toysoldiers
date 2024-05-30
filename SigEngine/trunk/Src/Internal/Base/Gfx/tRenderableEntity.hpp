//------------------------------------------------------------------------------
// \file tRenderableEntity.hpp - 31 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tRenderableEntity__
#define __tRenderableEntity__
#include "tDisplayList.hpp"
#include "tSpatialEntity.hpp"
#include "tLinkedList.hpp"

namespace Sig { 

	class tLinearFrustumCulling;
	struct tLFCObject;
	
namespace Gfx
{
	class tViewport;
	class tWorldSpaceDisplayList;
	class tCamera;
	class tVisibilitySetRef;

	class base_export tRenderableEntity : public tSpatialEntity, public tRenderInstance
	{
		debug_watch( tRenderableEntity );
		declare_uncopyable( tRenderableEntity );
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
			cFlagHasLODs							= ( 1 << 6 ),
			cFlagForceHighLOD						= ( 1 << 7 ),
			cFlagNotPotentiallyVisible				= ( 1 << 8 ),
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
		
		static f32 fGetGlobalFarFadeSetting( u32 fadeSetting );
		static void fSetGlobalFarFadeSetting( u32 fadeSetting, f32 distance );

		static f32 fGetGlobalNearFadeSetting( u32 fadeSetting );
		static void fSetGlobalNearFadeSetting( u32 fadeSetting, f32 distance );
		
	public:
		static const u32 cSpatialSetIndex;
		static const u32 cHeightFieldSpatialSetIndex;
		static const u32 cEffectSpatialSetIndex;
		static void fAddRenderableSpatialSetIndices( tDynamicArray<u32>& ids, b32 includeEffects = false );

	private:
		f32 mCameraDepthOffset;
		u32	mViewportMask;
		u32	mFlags;
		f32 mNearFadeDistance;
		f32 mFarFadeDistance;
		tEntityWeakPtr mFadeOutRoot;

	protected:
		f32 mLODMediumOverride;
		f32 mLODFarOverride;
		f32 mLastLODDistance;
		tRefCounterPtr< tLFCObject > mRenderSpatial;

		tRefCounterPtr<tVisibilitySetRef> mVisibilitySet;
		
	public:
		tRenderableEntity( );
		explicit tRenderableEntity( const tRenderBatchPtr& batch );
		tRenderableEntity( const tRenderBatchPtr& batch, const Math::tAabbf& objectSpaceBox );
		virtual ~tRenderableEntity( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );

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
		
		virtual Math::tVec4f	fRI_DynamicVec4( const tStringPtr& varName, u32 viewportIndex ) const;
		virtual u32				fRI_TransitionObjects( Math::tVec4f* o, u32 osize ) const;
		virtual void			fUpdateLOD( const Math::tVec3f & eye ) {  }

		tVisibilitySetRef*		fVisibilitySet( ) const { return mVisibilitySet.fGetRawPtr( ); }
		void					fSetVisibilitySet( tVisibilitySetRef* set );

		inline void				fSetRgbaTint( const Math::tVec4f& rgbaTint ) { tRenderInstance::fSetRgbaTint( rgbaTint ); }

		inline void				fSetDisabled( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagDisabled ) : fClearBits( mFlags, cFlagDisabled ) ); fUpdateSpatialPresence( ); }
		inline b32				fDisabled( ) const { return mFlags & cFlagDisabled; }
		inline void				fSetInvisible( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagInvisible ) : fClearBits( mFlags, cFlagInvisible ) ); fUpdateSpatialPresence( ); }
		inline b32				fInvisible( ) const { return mFlags & cFlagInvisible; }
		void					fSetNotPotentiallyVisible( b32 set ) OVERRIDE { mFlags = ( set ? fSetBits( mFlags, cFlagNotPotentiallyVisible ) : fClearBits( mFlags, cFlagNotPotentiallyVisible ) ); fUpdateSpatialPresence( ); }
		b32						fNotPotentiallyVisible( ) const OVERRIDE { return mFlags & cFlagNotPotentiallyVisible; }
		inline b32				fDisabledInSomeWay( ) const { return mFlags & (cFlagInvisible | cFlagDisabled | cFlagNotPotentiallyVisible); }
		inline void				fSetCastsShadow( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagCastShadow ) : fClearBits( mFlags, cFlagCastShadow ) ); }
		inline b32				fCastsShadow( ) const { return mFlags & cFlagCastShadow; }
		inline void				fSetReceivesShadow( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagReceiveShadow ) : fClearBits( mFlags, cFlagReceiveShadow ) ); }
		inline b32				fReceivesShadow( ) const { return mFlags & cFlagReceiveShadow; }
		inline void				fSetDisallowIndirectColorControllers( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagDisallowIndirectColorControllers ) : fClearBits( mFlags, cFlagDisallowIndirectColorControllers ) ); }
		inline b32				fDisallowIndirectColorControllers( ) const { return mFlags & cFlagDisallowIndirectColorControllers; }
		inline void				fSetUseEffectSpatialSet( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagUseEffectSpatialSet ) : fClearBits( mFlags, cFlagUseEffectSpatialSet ) ); }
		inline b32				fUseEffectSpatialSet( ) const { return mFlags & cFlagUseEffectSpatialSet; }
		inline void				fSetHasLODs( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagHasLODs ) : fClearBits( mFlags, cFlagHasLODs ) ); }
		inline b32				fHasLODs( ) const { return mFlags & cFlagHasLODs; }
		inline void				fSetForceHighLOD( b32 set ) { mFlags = ( set ? fSetBits( mFlags, cFlagForceHighLOD ) : fClearBits( mFlags, cFlagForceHighLOD ) ); }
		inline b32				fForceHighLOD( ) const { return mFlags & cFlagForceHighLOD; }
		inline u32				fFlags( ) const { return mFlags; }
		
		// these are deprecated because changing invisible or disabled needs to maintain your spatial presence.
		inline u32				fViewportMask( ) const { return mViewportMask; }
		inline void				fSetViewportMask( u32 mask ) { mViewportMask = mask; }
		inline void				fEnableViewport( u32 ithViewport, b32 enable ) { mViewportMask = enable ? fSetBits( mViewportMask, 1 << ithViewport ) : fClearBits( mViewportMask, 1 << ithViewport ); }

		inline f32				fNearFadeOutDistance( ) const { return mNearFadeDistance; }
		virtual f32				fRI_FarFadeOutDistance( ) const { return mFarFadeDistance; }
		inline const tEntity*	fFadeOutRoot( ) const { return mFadeOutRoot.fObject( ); }

		inline b32				fShouldBeRendered( u32 viewPortMask ) const
		{
			// things that are disable or invisible are not in the tree.
			return ( mViewportMask & viewPortMask ) && ( fRgbaTint( ).w > Math::cEpsilon );
		}

		f32 fComputeFadeAlpha( tScreen& screen ) const;
		f32 fComputeFadeAlpha( const tCamera& camera ) const;

		// Use cFadeSettingCount and 0.f to leave the setting unadjusted
		void fSetFadeSettingsOnThis( tEntity * fadeRoot, 
			tFadeSetting farFadeSetting, tFadeSetting nearFadeSetting, 
			f32 explicitFarOverride = 0.f, f32 explicitNearOverride = 0.f );
		static void fSetFadeSettings( tEntity & fadeRoot, 
			tFadeSetting farFadeSetting, tFadeSetting nearFadeSetting, 
			f32 explicitFarOverride = 0.f, f32 explicitNearOverride = 0.f);
		
		void fSetLODDistsOnThis( tEntity& realRoot, f32 mediumDist, f32 farDist );
		static void fSetLODDists( tEntity& realRoot, f32 mediumDist, f32 farDist );

	public: //frustum culling stuff
		enum tFrustumCullingFlags
		{
			cFrustumCullFlagsShadowCaster	= (1<<0),
			cFrustumCullFlagsLODed			= (1<<1),
			// MUST NOT EXCEED CAPACITY OF tLinearFrustumCulling::tFlags size
		};

		b32 fInRenderCullingList( ) const;
		void fUpdateRenderCulling( tLinearFrustumCulling& cull );
		void fAddToRenderCulling( tLinearFrustumCulling& cull );
		void fRemoveFromRenderCulling( tLinearFrustumCulling& cull );
		u32 fDetermineFrustmCullingFlags( ) const;

	private:
		static void fSetFadeSettings( 
			tEntity& realRoot, tEntity& currentRoot, 
			tFadeSetting farFadeSetting, f32 explicitFarOverride,
			tFadeSetting nearFadeSetting, f32 explicitNearOverride );

		static void fSetFadeSetting( tEntity& realRoot, tEntity& currentRoot, tFadeSetting fadeSetting, f32 explicitOverride );
		static void fSetLODDists( tEntity& realRoot, tEntity& currentRoot, f32 mediumDist, f32 farDist );

	public:
		static void fSetDisallowIndirectColorControllers( tEntity& root, b32 set );
		static void	fSetDisabled( tEntity& root, b32 set );
		static void fSetRgbaTint( tEntity& root, const Math::tVec4f& tint );
		static void fSetRgbaTintRespectNoIndirectTinting( tEntity& root, const Math::tVec4f& tint, b32 ignoreChildrenWithLogic = false );
		static void	fSetInvisible( tEntity& root, b32 set );
		static void fSetNotPotentiallyVisibleRecursive( tEntity& root, b32 set );
		static void	fSetViewportMask( tEntity& root, u32 mask );
		static void	fEnableViewport( tEntity& root, u32 ithViewport, b32 enable );
		static void fSetUseEffectSpatialSet( tEntity& root, b32 set );
		static void fSetForceHighLOD( tEntity& root, b32 set );
		static void fSetVisibilitySet( tEntity& root, tVisibilitySetRef* set );
        static b32 fIsAnyDisabledInSomeWayRecursive( tEntity& root );

		// LOD Global enable/disable
		static void fSetProjectLODEnable( b32 enable );
		static b32  fProjectLODEnable( );

	private:
		void fCommonCtor( const tRenderBatchPtr& batch );
		void fConnectRenderable( const tRenderBatchPtr& batch );
		void fUpdateSpatialPresence( );

	public: // script-specific
		static void					fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tRenderableEntity );

	class tRenderableEntityList : public tGrowableArray<tRenderableEntityPtr>
	{
	};


#ifdef target_tools

	class base_export tRenderableEntityToolsHelper
	{
	public:

		static void fSetFadeAlphaEnabled( b32 enabled ) { gFadeAlphaEnabled = enabled; }
		static b32 fFadeAlphaEnabled( ) { return gFadeAlphaEnabled; }

	private:

		static b32 gFadeAlphaEnabled;
	};

#endif //target_tools

}}

#endif//__tRenderableEntity__
