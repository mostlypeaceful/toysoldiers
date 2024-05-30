#include "BasePch.hpp"
#include "tScreen.hpp"
#include "tEntityDef.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		static f32 gFadeSettings[tRenderableEntity::cFadeSettingCount]=
		{
			0.f,
			125.f,
			250.f,
			500.f,
		};
		devvarptr_clamp( f32, Renderer_FadeOut_0Near,	gFadeSettings[tRenderableEntity::cFadeNear], 1.f, 10000.f, 0 );
		devvarptr_clamp( f32, Renderer_FadeOut_1Medium, gFadeSettings[tRenderableEntity::cFadeMedium], 1.f, 10000.f, 0 );
		devvarptr_clamp( f32, Renderer_FadeOut_2Far,	gFadeSettings[tRenderableEntity::cFadeFar], 1.f, 10000.f, 0 );
	}

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

	void tRenderableEntity::fSetGlobalFadeSetting( tFadeSetting slot, f32 distance )
	{
		sigassert( fInBounds( ( s32 )slot, 0, ( s32 )cFadeFar ) );
		gFadeSettings[ slot ] = distance;
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
	void tRenderableEntity::fCommonCtor( const tRenderBatchPtr& batch )
	{
		mCameraDepthOffset = 0.f;
		mFadeOutDistance = 0.f;
		mFadeOutRoot = NULL;
		mFlags = 0;
		mViewportMask = ~0;
		fConnectRenderable( batch );
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

	f32 tRenderableEntity::fCameraDepth( const tCamera& camera, f32 fadeAlpha ) const
	{
		f32 d = 0.f;

		if( fBatchHasXparency( ) || fRequiresXparentSort( fadeAlpha ) )
		{
			const Math::tVec3f& eye = camera.fGetTripod( ).mEye;

			//d = eye.fDistance( fObjectToWorld( ).fGetTranslation( ) );

			d = eye.fDistance( fWorldSpaceBox( ).fComputeCenter( ) );

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
	}

	b32	tRenderableEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		if( fInvisible( ) || fDisabled( ) )
			return false;
		return tSpatialEntity::fIntersects( v );
	}

	b32	tRenderableEntity::fIntersects( const Math::tAabbf& v ) const
	{
		if( fInvisible( ) || fDisabled( ) )
			return false;
		return tSpatialEntity::fIntersects( v );
	}

	b32	tRenderableEntity::fIntersects( const Math::tObbf& v ) const
	{
		if( fInvisible( ) || fDisabled( ) )
			return false;
		return tSpatialEntity::fIntersects( v );
	}

	b32	tRenderableEntity::fIntersects( const Math::tSpheref& v ) const
	{
		if( fInvisible( ) || fDisabled( ) )
			return false;
		return tSpatialEntity::fIntersects( v );
	}

	Math::tVec4f tRenderableEntity::fRI_DynamicVec4( const tStringPtr& varName, u32 viewportIndex ) const
	{
		tEntity* logicOwner = fFirstAncestorWithLogic( );
		if( logicOwner )
			return logicOwner->fLogic( )->fQueryDynamicVec4Var( varName, viewportIndex );
		return Math::tVec4f::cZeroVector;
	}

	f32 tRenderableEntity::fComputeFadeAlpha( tScreen& screen ) const
	{
		f32 fadeAlpha = 1.f;

#ifdef target_game
		const f32 fadeDistance = fFadeOutDistance( );
		if( fadeDistance )
		{
			sigassert( fadeDistance > 0.f );
			sigassert( fFadeOutRoot( ) );

			f32 distToViewPlane = 0.f;
			for( u32 i = 0; i < screen.fGetViewportCount( ); ++i )
			{
				const tCamera& camera = screen.fViewport( i )->fRenderCamera( );
				distToViewPlane = fMax( distToViewPlane, 
					//camera.fCameraDepth( fFadeOutRoot( )->fObjectToWorld( ).fGetTranslation( ) ) * camera.fGetLens( ).mZoom
					fFadeOutRoot( )->fObjectToWorld( ).fGetTranslation( ).fDistance( camera.fGetTripod( ).mLookAt )
					);
			}

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

	void tRenderableEntity::fSetFadeSettingOnThis( tEntity& realRoot, tFadeSetting fadeSetting, f32 explicitOverride )
	{
		mFadeOutDistance = explicitOverride ? explicitOverride : gFadeSettings[ fadeSetting ];
		mFadeOutRoot = &realRoot;
	}

	void tRenderableEntity::fSetFadeSetting( tEntity& root, tFadeSetting fadeSetting, f32 explicitOverride )
	{
		fSetFadeSetting( root, root, fadeSetting, explicitOverride );
	}

	void tRenderableEntity::fSetFadeSetting( tEntity& realRoot, tEntity& currentRoot, tFadeSetting fadeSetting, f32 explicitOverride )
	{
		tRenderableEntity* renderable = currentRoot.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fSetFadeSettingOnThis( realRoot, fadeSetting, explicitOverride );
		for( u32 i = 0; i < currentRoot.fChildCount( ); ++i )
			fSetFadeSetting( realRoot, *currentRoot.fChild( i ), fadeSetting, explicitOverride );
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
	void tRenderableEntity::fAddFlags( tEntity& root, u32 flags )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fAddFlags( flags );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fAddFlags( *root.fChild( i ), flags );
	}
	void tRenderableEntity::fRemoveFlags( tEntity& root, u32 flags )
	{
		tRenderableEntity* renderable = root.fDynamicCast< tRenderableEntity >( );
		if( renderable )
			renderable->fRemoveFlags( flags );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fRemoveFlags( *root.fChild( i ), flags );
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
}}


namespace Sig { namespace Gfx
{
	namespace
	{
		static void fSetRenderableEntityTint( tEntity* root, const Math::tVec4f& tint )
		{
			sigassert( root );
			tRenderableEntity::fSetRgbaTint( *root, tint );
		}
	}

	void tRenderableEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tRenderableEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("SetTint"),		&fSetRenderableEntityTint)
			;

		vm.fNamespace(_SC("Gfx")).Bind( _SC("RenderableEntity"), classDesc );
	}

}}
