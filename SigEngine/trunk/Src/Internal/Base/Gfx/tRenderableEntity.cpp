//------------------------------------------------------------------------------
// \file tRenderableEntity.cpp - 31 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tScreen.hpp"
#include "tEntityDef.hpp"
#include "tApplication.hpp"
#include "tLinearFrustumCulling.hpp"
#include "tLightProbeEntity.hpp"
#include "tSceneGraph.hpp"
#include "Gfx/tPotentialVisibilitySet.hpp"
#include "Physics/tGJK.hpp" //only for barycentric functions.
#include "tFxFileRefEntity.hpp" // recursive fSetVisibilitySet will include tFxFileRefEntity due to the strange deferred way effects are spawn it must be set there.

namespace Sig { namespace Gfx
{
	namespace
	{
		static f32 gFarFadeSettings[tRenderableEntity::cFadeSettingCount] = { 0.f, 125.f, 250.f, 500.f };
		devvarptr_clamp( f32, Renderer_FarFadeOut_0Near,	gFarFadeSettings[tRenderableEntity::cFadeNear], 1.f, 10000.f, 0 );
		devvarptr_clamp( f32, Renderer_FarFadeOut_1Medium, gFarFadeSettings[tRenderableEntity::cFadeMedium], 1.f, 10000.f, 0 );
		devvarptr_clamp( f32, Renderer_FarFadeOut_2Far,	gFarFadeSettings[tRenderableEntity::cFadeFar], 1.f, 10000.f, 0 );

		static f32 gNearFadeSettings[tRenderableEntity::cFadeSettingCount] = { 0.f, 65.f, 125.f, 250.f };
		devvarptr_clamp( f32, Renderer_NearFadeOut_0Near,	gNearFadeSettings[tRenderableEntity::cFadeNear], 1.f, 10000.f, 0 );
		devvarptr_clamp( f32, Renderer_NearFadeOut_1Medium, gNearFadeSettings[tRenderableEntity::cFadeMedium], 1.f, 10000.f, 0 );
		devvarptr_clamp( f32, Renderer_NearFadeOut_2Near,	gNearFadeSettings[tRenderableEntity::cFadeNear], 1.f, 10000.f, 0 );

		static b32 gProjectLODEnable = true;
	}

#ifdef target_tools
		b32 tRenderableEntityToolsHelper::gFadeAlphaEnabled = false;
#endif // target_tools

	const u32 tRenderableEntity::cSpatialSetIndex = tSceneGraph::fNextSpatialSetIndex( );
	const u32 tRenderableEntity::cHeightFieldSpatialSetIndex = tSceneGraph::fNextSpatialSetIndex( );
	const u32 tRenderableEntity::cEffectSpatialSetIndex = tSceneGraph::fNextSpatialSetIndex( );

	void tRenderableEntity::fAddRenderableSpatialSetIndices( tDynamicArray<u32>& ids, b32 includeEffects )
	{
		ids.fNewArray( 2 + ( includeEffects ? 1 : 0 ) );
		ids[ 0 ] = Gfx::tRenderableEntity::cSpatialSetIndex;
		ids[ 1 ] = Gfx::tRenderableEntity::cHeightFieldSpatialSetIndex;
		if( includeEffects )
			ids[ 2 ] = Gfx::tRenderableEntity::cEffectSpatialSetIndex;
	}

	void tRenderableEntity::fSetObjectSpaceBoxOverride( tEntity& root, const Math::tAabbf& newObjSpaceBox )
	{
		tRenderableEntity* spatial = root.fDynamicCast< tRenderableEntity >( );
		if( spatial )
			spatial->fSetObjectSpaceBox( newObjSpaceBox );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetObjectSpaceBoxOverride( *root.fChild( i ), newObjSpaceBox );
	}

	f32 tRenderableEntity::fGetGlobalFarFadeSetting( u32 fadeSetting )
	{
		sigassert( fadeSetting < cFadeSettingCount );
		return gFarFadeSettings[ fadeSetting ];
	}

	void tRenderableEntity::fSetGlobalFarFadeSetting( u32 fadeSetting, f32 distance )
	{
		sigassert( fadeSetting < cFadeSettingCount );
		gFarFadeSettings[ fadeSetting ] = distance;
	}

	f32 tRenderableEntity::fGetGlobalNearFadeSetting( u32 fadeSetting )
	{
		sigassert( fadeSetting < cFadeSettingCount );
		return gNearFadeSettings[ fadeSetting ];
	}

	void tRenderableEntity::fSetGlobalNearFadeSetting( u32 fadeSetting, f32 distance )
	{
		sigassert( fadeSetting < cFadeSettingCount );
		gNearFadeSettings[ fadeSetting ] = distance;
	}

	tRenderableEntity::tRenderableEntity( )
	{
		fCommonCtor( tRenderBatchPtr( ) );
	}
	tRenderableEntity::tRenderableEntity( const tRenderBatchPtr& batch )
	{
		fCommonCtor( batch );
	}
	tRenderableEntity::tRenderableEntity( const tRenderBatchPtr& batch, const Math::tAabbf& objectSpaceBox )
		: tSpatialEntity( objectSpaceBox )
	{
		fCommonCtor( batch );
	}
	tRenderableEntity::~tRenderableEntity( )
	{
	}
	void tRenderableEntity::fOnSpawn( )
	{
		if( mVisibilitySet )
			fSceneGraph( )->fPotentialVisibilitySet( )->fRegister( *this, *mVisibilitySet );

		tSpatialEntity::fOnSpawn( );
	}
	void tRenderableEntity::fOnDelete( )
	{
		if( mVisibilitySet )
			fSceneGraph( )->fPotentialVisibilitySet( )->fUnRegister( *this, *mVisibilitySet );
		mVisibilitySet.fRelease( );

		tSpatialEntity::fOnDelete( );
	}
	void tRenderableEntity::fSetVisibilitySet( tVisibilitySetRef* set )
	{
		if( fSceneGraph( ) && mVisibilitySet )
			fSceneGraph( )->fPotentialVisibilitySet( )->fUnRegister( *this, *mVisibilitySet );

		mVisibilitySet.fReset( set );

		if( fSceneGraph( ) && mVisibilitySet )
			fSceneGraph( )->fPotentialVisibilitySet( )->fRegister( *this, *mVisibilitySet );
	}
	void tRenderableEntity::fCommonCtor( const tRenderBatchPtr& batch )
	{
		mCameraDepthOffset = 0.f;
		mFarFadeDistance = 0.f;
		mNearFadeDistance = 0.f;
		mFlags = 0;
		mViewportMask = ~0;
		fConnectRenderable( batch );
		mLastLODDistance = 0.f;
		mLODMediumOverride = 0.f;
		mLODFarOverride = 0.f;
	}
	void tRenderableEntity::fConnectRenderable( const tRenderBatchPtr& batch )
	{
		fSetRenderBatch( batch );
		fRI_SetObjectToWorld( &fObjectToWorld( ) );
	}

	void tRenderableEntity::fComputeDisplayStats( tDisplayStats& displayStatsOut ) const
	{
		const tRenderBatchData& renderBatch = fRenderBatch( )->fBatchData( );
		displayStatsOut.mNumDrawCalls = 1;
		displayStatsOut.mBatchSwitches = 1;
		displayStatsOut.mPrimitiveCounts[ renderBatch.mPrimitiveType ] = renderBatch.mPrimitiveCount;
	}

	void tRenderableEntity::fUpdateSpatialPresence( )
	{
		if( fDisabledInSomeWay( ) )
		{
			fRemoveFromSpatialTree( );
		}
		else
			fAddToSpatialTree( );
	}

	// Factors for the various possible eye <-> renderable entity calculation factors.  These should probably sum up to 1.00 for sanity/consistency.
	// Doing depth sorting soley by the nearest point of the AABB results in all AABBs surrounding the eye having the same depth: 0, hence we sprinkle
	// a little distance in based on the center as well.
	devvar_clamp( f32, Renderer_Settings_RenderableEntityAABBNearestFactor	, 0.99f, 0.00f, 1.00f, 2 );
	devvar_clamp( f32, Renderer_Settings_RenderableEntityCenterNearestFactor, 0.01f, 0.00f, 1.00f, 2 );

	f32 tRenderableEntity::fCameraDepth( const tCamera& camera, f32 fadeAlpha ) const
	{
		f32 d = 0.f;

		if( fBatchHasXparency( ) || fRequiresXparentSort( fadeAlpha ) )
		{
			const Math::tVec3f& eye = camera.fGetTripod( ).mEye;

			//d = eye.fDistance( fObjectToWorld( ).fGetTranslation( ) );

			d += Renderer_Settings_RenderableEntityAABBNearestFactor * eye.fDistance( fClamp( eye, fWorldSpaceBox( ).mMin, fWorldSpaceBox( ).mMax ) );
			d += Renderer_Settings_RenderableEntityCenterNearestFactor * eye.fDistance( fWorldSpaceBox( ).fComputeCenter( ) );

			//d = fWorldSpaceBox( ).fDistanceToPoint( eye );
			//if( d == 0.f )
			//	d = camera.fCameraDepth( fWorldSpaceBox( ).fComputeCenter( ) ) + 0.5f * fWorldSpaceBox( ).fComputeDiagonal( ).fMax( );

			d += mCameraDepthOffset;
		}

		return d;
	}

	void tRenderableEntity::fAddToDisplayList( const tViewport& viewport, tWorldSpaceDisplayList& displayList )
	{
		displayList.fInsert( fGetDrawCall( viewport.fRenderCamera( ) ) );
	}

	void tRenderableEntity::fApplyCreationFlags( const tEntityCreationFlags& creationFlags )
	{
		if( creationFlags.mRenderFlags & cFlagInvisible )
			fSetInvisible( true );
		if( creationFlags.mRenderFlags & cFlagReceiveShadow )
			fSetReceivesShadow( true );
		if( creationFlags.mRenderFlags & cFlagCastShadow )
			fSetCastsShadow( true );
		if( creationFlags.mRenderFlags & cFlagForceHighLOD )
			fSetForceHighLOD( true );
	}

	Math::tVec4f tRenderableEntity::fRI_DynamicVec4( const tStringPtr& varName, u32 viewportIndex ) const
	{
		tEntity* logicOwner = fFirstAncestorWithLogic( );
		if( logicOwner )
			return logicOwner->fLogic( )->fQueryDynamicVec4Var( varName, viewportIndex );
		return Math::tVec4f::cZeroVector;
	}

	u32 tRenderableEntity::fRI_TransitionObjects( Math::tVec4f* o, u32 osize ) const
	{
		tEntity* logicOwner = fFirstAncestorWithLogic( );
		if( logicOwner )
			return logicOwner->fLogic( )->fQueryTransitionObjects( o, osize, fWorldSpaceBox( ) );
		return 0;
	}

	f32 tRenderableEntity::fComputeFadeAlpha( tScreen& screen ) const
	{

#ifdef target_tools
		if( !tRenderableEntityToolsHelper::fFadeAlphaEnabled( ) )
			return 1.f;
#endif // target_tools

		f32 fadeAlpha = 1.f;
		const f32 fadeDistance = fRI_FarFadeOutDistance( );
		if( fFadeOutRoot( ) && fadeDistance )
		{
			sigassert( fadeDistance > 0.f );
			
			// If we have no fade out root, use ourselves
			const tEntity * fadeOutRoot = fFadeOutRoot( );

			f32 distToViewPlane = 0.f;
			for( u32 i = 0; i < screen.fGetViewportCount( ); ++i )
			{
				const tCamera& camera = screen.fViewport( i )->fRenderCamera( );
				distToViewPlane = fMax( distToViewPlane, 
					fadeOutRoot->fObjectToWorld( ).fGetTranslation( ).fDistance( tApplication::fInstance( ).fFadePos( camera ) ) );
			}

			if( distToViewPlane >= fadeDistance )
				return 0.f;

			const f32 fadeRange = 10.f;
			const f32 beginFadeAt = fadeDistance - fadeRange;
			if( distToViewPlane > beginFadeAt )
				fadeAlpha = 1.f - ( ( distToViewPlane - beginFadeAt ) / fadeRange );
			sigassert( fInBounds( fadeAlpha, 0.f, 1.f ) );
		}

		return fadeAlpha;
	}
	f32 tRenderableEntity::fComputeFadeAlpha( const tCamera& camera ) const
	{
		
#ifdef target_tools
		if( !tRenderableEntityToolsHelper::fFadeAlphaEnabled( ) )
			return 1.f;
#endif // target_tools

		f32 fadeAlpha = 1.f;
		const f32 farFadeDistance = fRI_FarFadeOutDistance( );
		const f32 nearFadeDistance = fNearFadeOutDistance( );

		const b32 hasFarFade = farFadeDistance != 0.f;
		const b32 hasNearFade = nearFadeDistance != 0.f;
		if( fFadeOutRoot( ) && ( hasFarFade || hasNearFade ) )
		{
			sigassert( farFadeDistance >= 0.f );
			sigassert( nearFadeDistance >= 0.f );

			// If we have no fade out root, use ourselves
			const tEntity * fadeOutRoot = fFadeOutRoot( );

			const f32 distToViewPlane = 
				fadeOutRoot->fObjectToWorld( ).fGetTranslation( ).fDistance( tApplication::fInstance( ).fFadePos( camera ) );

			if( hasFarFade && distToViewPlane >= farFadeDistance )
				return 0.f;
			if( hasNearFade && distToViewPlane <= nearFadeDistance )
				return 0.f;

			const f32 fadeRange = 10.f;
			if( hasFarFade )
			{
				const f32 beginFadeAt = farFadeDistance - fadeRange;
				if( distToViewPlane > beginFadeAt )
					fadeAlpha = 1.f - ( ( distToViewPlane - beginFadeAt ) / fadeRange );
			}

			if( hasNearFade )
			{
				const f32 beginFadeAt = nearFadeDistance + fadeRange;
				if( distToViewPlane < beginFadeAt )
					fadeAlpha = 1 - ( ( beginFadeAt - distToViewPlane ) / fadeRange );
			}

			sigassert( fInBounds( fadeAlpha, 0.f, 1.f ) );
		}

		return fadeAlpha;
	}

	void tRenderableEntity::fSetFadeSettingsOnThis(
		tEntity * fadeRoot, 
		tFadeSetting farFadeSetting, tFadeSetting nearFadeSetting, 
		f32 explicitFarOverride, f32 explicitNearOverride )
	{
		mFadeOutRoot.fReset( fadeRoot );

		// Far
		if( explicitFarOverride )
			mFarFadeDistance = explicitFarOverride;
		else if( farFadeSetting < cFadeSettingCount )
			mFarFadeDistance = gFarFadeSettings[ farFadeSetting ];

		// Near
		if( explicitNearOverride )
			mNearFadeDistance = explicitNearOverride;
		else if( nearFadeSetting < cFadeSettingCount )
			mNearFadeDistance = gNearFadeSettings[ nearFadeSetting ];
	}

	void tRenderableEntity::fSetFadeSettings( 
		tEntity & fadeRoot, 
		tFadeSetting farFadeSetting, tFadeSetting nearFadeSetting, 
		f32 explicitFarOverride, f32 explicitNearOverride )
	{
		fSetFadeSettings( fadeRoot, fadeRoot, farFadeSetting, explicitFarOverride, nearFadeSetting, explicitNearOverride );
	}

	void tRenderableEntity::fSetFadeSettings( 
		tEntity& realRoot, tEntity& currentRoot, 
		tFadeSetting farFadeSetting, f32 explicitFarOverride,
		tFadeSetting nearFadeSetting, f32 explicitNearOverride )
	{
		tRenderableEntity* renderable = currentRoot.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetFadeSettingsOnThis( &realRoot, farFadeSetting, nearFadeSetting, explicitFarOverride, explicitNearOverride );
		for( u32 i = 0; i < currentRoot.fChildCount( ); ++i )
			fSetFadeSettings( realRoot, *currentRoot.fChild( i ), farFadeSetting, explicitFarOverride, nearFadeSetting, explicitNearOverride );
	}

	void tRenderableEntity::fSetLODDistsOnThis( tEntity& realRoot, f32 mediumDist, f32 farDist )
	{
		mLODMediumOverride = mediumDist;
		mLODFarOverride = farDist;
	}

	void tRenderableEntity::fSetLODDists( tEntity& realRoot, f32 mediumDist, f32 farDist )
	{
		fSetLODDists( realRoot, realRoot, mediumDist, farDist );
	}

	void tRenderableEntity::fSetLODDists( tEntity& realRoot, tEntity& currentRoot, f32 mediumDist, f32 farDist )
	{
		tRenderableEntity* renderable = currentRoot.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetLODDistsOnThis( realRoot, mediumDist, farDist );
		for( u32 i = 0; i < currentRoot.fChildCount( ); ++i )
			fSetLODDists( realRoot, *currentRoot.fChild( i ), mediumDist, farDist );
	}

	void tRenderableEntity::fSetProjectLODEnable( b32 enable )
	{
		gProjectLODEnable = enable;
	}

	b32 tRenderableEntity::fProjectLODEnable( )
	{
		return gProjectLODEnable;
	}

	void tRenderableEntity::fSetDisallowIndirectColorControllers( tEntity& root, b32 set )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetDisallowIndirectColorControllers( set );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetDisallowIndirectColorControllers( *root.fChild( i ), set );
	}
	void tRenderableEntity::fSetDisabled( tEntity& root, b32 set )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetDisabled( set );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetDisabled( *root.fChild( i ), set );
	}
	void tRenderableEntity::fSetRgbaTint( tEntity& root, const Math::tVec4f& tint )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetRgbaTint( tint );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetRgbaTint( *root.fChild( i ), tint );
	}
	void tRenderableEntity::fSetRgbaTintRespectNoIndirectTinting( tEntity& root, const Math::tVec4f& tint, b32 ignoreChildrenWithLogic )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable && !renderable->fDisallowIndirectColorControllers( ) )
			renderable->fSetRgbaTint( tint );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
		{
			if( !ignoreChildrenWithLogic || !root.fChild( i )->fLogic( ) )
				fSetRgbaTintRespectNoIndirectTinting( *root.fChild( i ), tint, ignoreChildrenWithLogic );
		}
	}
	void tRenderableEntity::fSetInvisible( tEntity& root, b32 set )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetInvisible( set );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetInvisible( *root.fChild( i ), set );
	}
	void tRenderableEntity::fSetNotPotentiallyVisibleRecursive( tEntity& root, b32 set )
	{
		if( tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( ) )
			renderable->fSetNotPotentiallyVisible( set );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetNotPotentiallyVisibleRecursive( *root.fChild( i ), set );
	}
	void tRenderableEntity::fSetViewportMask( tEntity& root, u32 mask )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetViewportMask( mask );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetViewportMask( *root.fChild( i ), mask );
	}
	void tRenderableEntity::fEnableViewport( tEntity& root, u32 ithViewport, b32 enable )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fEnableViewport( ithViewport, enable );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fEnableViewport( *root.fChild( i ), ithViewport, enable );
	}
	void tRenderableEntity::fSetUseEffectSpatialSet( tEntity& root, b32 set )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetUseEffectSpatialSet( set );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetUseEffectSpatialSet( *root.fChild( i ), set );
	}
	void tRenderableEntity::fSetForceHighLOD( tEntity& root, b32 set )
	{
		tRenderableEntity* re = root.fDynamicCast< tRenderableEntity >( );
		if( re )
		{
			re->fSetForceHighLOD( set );
			if( set && fProjectLODEnable( ) )
				re->fUpdateLOD( Math::tVec3f::cZeroVector );
		}
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetForceHighLOD( *root.fChild( i ), set );
	}
	void tRenderableEntity::fSetVisibilitySet( tEntity& root, tVisibilitySetRef* set )
	{
		tRenderableEntity* re = root.fDynamicCast< tRenderableEntity >( );
		if( re )
			re->fSetVisibilitySet( set );

		FX::tFxFileRefEntity* fx = root.fDynamicCast< FX::tFxFileRefEntity >( );
		if( fx )
			fx->fSetVisibilitySet( set );

		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetVisibilitySet( *root.fChild( i ), set );
	}
    b32 tRenderableEntity::fIsAnyDisabledInSomeWayRecursive( tEntity& root )
    {
        if( tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( ) )
            if( renderable->fDisabledInSomeWay( ) )
                return true;

        for( u32 i = 0; i < root.fChildCount( ); ++i )
            if( fIsAnyDisabledInSomeWayRecursive( *root.fChild( i ) ) )
                return true;

        return false;
    }

	void tRenderableEntity::fUpdateRenderCulling( tLinearFrustumCulling& cull )
	{
		//ghetto
		Math::tVec4f sphere = tLinearFrustumCulling::fToSphere( fWorldSpaceBox( ) );

		//Math::tObbf tightBounds( mObjectSpaceBox, fObjectToWorld( ) );
		//Math::tVec4f sphere = tLinearFrustumCulling::fToSphere( tightBounds );
		cull.fMove( sphere, mRenderSpatial );
	}

	b32 tRenderableEntity::fInRenderCullingList( ) const
	{
		return mRenderSpatial && mRenderSpatial->fInList( );
	}

	void tRenderableEntity::fAddToRenderCulling( tLinearFrustumCulling& cull )
	{
		sigassert( !mRenderSpatial || !mRenderSpatial->fInList( ) );
		mRenderSpatial.fReset( NEW_TYPED( tLFCObject )( ) );
		cull.fInsert( tLinearFrustumCulling::fToSphere( fWorldSpaceBox( ) ), mRenderSpatial, (void*)this, fDetermineFrustmCullingFlags( ) );
	}

	void tRenderableEntity::fRemoveFromRenderCulling( tLinearFrustumCulling& cull )
	{
		cull.fRemove( mRenderSpatial );
	}

	u32 tRenderableEntity::fDetermineFrustmCullingFlags( ) const
	{
		u32 flags = 0;
		if( fCastsShadow( ) )
			flags |= cFrustumCullFlagsShadowCaster;
		if( fTestBits( mFlags, cFlagHasLODs ) )
			flags |= cFrustumCullFlagsLODed;
		return flags;
	}

	namespace
	{
		struct tFindLightProbeCallback
		{
			tFindLightProbeCallback( const Math::tVec3f& center )
				: mCenter( center )
			{ }

			void operator()( tEntityBVH::tObjectPtr octreeObject ) const
			{
				tLightProbeEntity* test = static_cast< tLightProbeEntity* >( octreeObject->fOwner( ) );
				const f32 distSqr = (test->fObjectToWorld( ).fGetTranslation( ) - mCenter).fLengthSquared( );

				for( u32 i = 0; i < mResults.fCount( ); ++i )
				{
					if( distSqr < mResults[ i ].mDistSqr )
					{
						if( mResults.fCount( ) == mResults.fCapacity( ) )
							mResults.fPopBack( );

						mResults.fInsert( i, tResult( test, distSqr ) );
						return; // all done with this guy.
					}
				}

				// didnt beat anyone, push on end if we have room.
				if( mResults.fCount( ) != mResults.fCapacity( ) )
					mResults.fPushBack( tResult( test, distSqr ) );
			}

			struct tResult
			{
				tLightProbeEntity* mEnt;
				f32 mDistSqr;

				tResult( tLightProbeEntity* ent = NULL, f32 distSqr = 0.f )
					: mEnt( ent )
					, mDistSqr( distSqr )
				{ }
			};

			mutable tFixedGrowingArray< tResult, 3 > mResults;
			Math::tVec3f mCenter;
		};
	}

}}


namespace Sig { namespace Gfx
{
	namespace
	{
		static void fSetRenderableEntityTint( tEntity* root, const Math::tVec4f& tint )
		{
			sigcheckfail( root, return );
			tRenderableEntity::fSetRgbaTint( *root, tint );
		}
		static void fSetRenderableEntityInvisible( tEntity* root, b32 invisible )
		{
			sigcheckfail( root, return );
			tRenderableEntity::fSetInvisible( *root, invisible );
		}
		static void fSetRenderableEntityCameraDepthOffset( tEntity* root, f32 offset )
		{
			sigcheckfail( root, return );
			if( tRenderableEntity* renderable = root->fDynamicCast< tRenderableEntity >( ) )
				renderable->fSetCameraDepthOffset( offset );
			for( u32 i = 0; i < root->fChildCount( ); ++i )
				fSetRenderableEntityCameraDepthOffset( root->fChild( i ).fGetRawPtr( ), offset );
		}
		static void fSetRenderableEntityObjectSpaceBoxOverride( tEntity* root, tEntity* overrideOBB )
		{
			sigcheckfail( root, return );
			sigcheckfail( overrideOBB, return );
			const tSpatialEntity* spatial = overrideOBB->fDynamicCast< tSpatialEntity >( );
			sigcheckfail( spatial, return );
			const Math::tMat3f spatialToParentXform = root->fWorldToObject( ) * spatial->fObjectToWorld( );
			const Math::tAabbf s = spatial->fObjectSpaceBox( ).fTransform( spatialToParentXform );
			tRenderableEntity::fSetObjectSpaceBoxOverride( *root, s );
		}
	}

	void tRenderableEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tRenderableEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("SetTint"),						&fSetRenderableEntityTint)
			.StaticFunc(_SC("SetInvisible"),				&fSetRenderableEntityInvisible)
			.StaticFunc(_SC("SetCameraDepthOffset"),		&fSetRenderableEntityCameraDepthOffset)
			.StaticFunc(_SC("SetObjectSpaceBoxOverride"),	&fSetRenderableEntityObjectSpaceBoxOverride)
			;

		vm.fNamespace(_SC("Gfx")).Bind( _SC("RenderableEntity"), classDesc );
	}

}}
