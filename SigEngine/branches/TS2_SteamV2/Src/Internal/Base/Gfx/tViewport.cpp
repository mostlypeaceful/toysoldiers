#include "BasePch.hpp"
#include "tViewport.hpp"
#include "tScreen.hpp"
#include "tRenderContext.hpp"
#include "tMaterial.hpp"
#include "tLightCombo.hpp"



namespace Sig { namespace Gfx
{
	tViewport::tViewport( u32 vpIndex )
		: mClipBox( 0.f, 0.f, 1.f, 1.f )
		, m16_9ClipBox( 0.f, 0.f, 1.f, 1.f )
		, mViewportIndex( vpIndex )
		, mIsVirtual( false )
	{
	}

	tViewport::~tViewport( )
	{
	}

	void tViewport::fSetCameras( const tCamera& camera ) 
	{
		sync_event_v_c( camera.fGetTripod( ).mEye, tSync::cSCCamera );
		sync_event_v_c( camera.fGetWorldToProjection( ), tSync::cSCCamera );

		mLogicCamera = camera; 
		mRenderCamera = camera; 
	}

	void tViewport::fRenderLitWorld( 
		tScreen& screen,
		const tLightComboList& lightCombos,
		const tRenderableEntityList& explicitWorldObjects, 
		const tRenderableEntityList& explicitWorldTopObjects, 
		tDisplayStats& worldDisplayStats ) const
	{
		const tSceneGraph& sg = *screen.fSceneGraph( );

		// use the semi-globally available shared white texture as a default shadow map
		tTextureReference whiteShadowMap;
		whiteShadowMap.fSetRaw( ( tTextureReference::tPlatformHandle )screen.fWhiteTexture( ) );
		whiteShadowMap.fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeClamp );

		// set default render state for world/scene rendering (uses scene camera)
		tRenderContext renderContext;
		renderContext.fFromScreen( screen );
		renderContext.mViewportTLBR = mClipBox.fConvertToLTRB( );
		renderContext.mCamera = &mRenderCamera;
		renderContext.mRenderPassMode = tRenderState::cRenderPassLighting;
		renderContext.mViewportIndex = mViewportIndex;
		renderContext.mViewportCount = screen.fGetViewportCount( );
		for( s32 v = renderContext.mViewportCount - 1; v >= 0; --v )
		{
			if( screen.fViewport( v )->fIsVirtual( ) )
				--renderContext.mViewportCount;
		}


		// invalidate render states
		screen.fGetDevice( )->fInvalidateLastRenderState( );

		// add all "explicitly-added" objects to the temp display list; these are objects that got added 
		// during the course of the last frame, which don't live in the scene graph permanently; this
		// list includes debug geometry, helper objects, etc, unlit usually
		tWorldSpaceDisplayList displayList;
		for( u32 i = 0; i < explicitWorldObjects.fCount( ); ++i )
			displayList.fInsert( tDrawCall( *explicitWorldObjects[ i ], mRenderCamera.fCameraDepth( explicitWorldObjects[ i ]->fObjectToWorld( ).fGetTranslation( ) ) ) );

		sg.fDebugGeometry( ).fAddToDisplayList( *this, displayList );

		// we must do this in two passes (although no object actually gets rendered twice, the objects are all separated into
		// different lists); first we render all the opaque objects from all the display lists, then we render all the
		// transparent objects from all the display lists
		const u32 cOpaque	= 0;
		const u32 cXparent	= 1;
		for( u32 opaqueOrXparent = cOpaque; opaqueOrXparent <= cXparent; ++opaqueOrXparent )
		{
			// render objects with corresponding light combos
			if( opaqueOrXparent == cOpaque )
			{
				lightCombos.mDisplayList.fRenderOpaque( screen, renderContext );
				worldDisplayStats.fCombine( lightCombos.mDisplayList.fGetOpaqueStats( ) );
			}
			else
			{
				lightCombos.mDisplayList.fRenderXparent( screen, renderContext );
				worldDisplayStats.fCombine( lightCombos.mDisplayList.fGetXparentStats( ) );
			}

			// render display list of explicitly enqueued objects
			renderContext.fClearLights( &whiteShadowMap );
			if( opaqueOrXparent == cOpaque )
			{
				displayList.fSeal( );
				displayList.fRenderOpaque( screen, renderContext );
				worldDisplayStats.fCombine( displayList.fGetOpaqueStats( ) );
			}
			else
			{
				displayList.fRenderXparent( screen, renderContext );
				worldDisplayStats.fCombine( displayList.fGetXparentStats( ) );
				displayList.fInvalidate( );
			}
		}

		if( explicitWorldTopObjects.fCount( ) > 0 )
		{
			// make sure all the lights are cleared
			renderContext.fClearLights( &whiteShadowMap );

			// clear depth buffer
			screen.fClearCurrentRenderTargets( false, true, 0.f, 1.f, 0x0 );

			// get display list of explicitly enqueued world "topmost" objects
			for( u32 i = 0; i < explicitWorldTopObjects.fCount( ); ++i )
				displayList.fInsert( tDrawCall( *explicitWorldTopObjects[ i ], mRenderCamera.fCameraDepth( explicitWorldTopObjects[ i ]->fObjectToWorld( ).fGetTranslation( ) ) ) );

			// render display list
			displayList.fSeal( );
			displayList.fRenderAll( screen, renderContext );
			displayList.fInvalidate( );
		}
	}

	Math::tRect tViewport::f16_9AdjustedClipBox( )
	{
		float w = m16_9ClipBox.fWidth();
		float h = m16_9ClipBox.fHeight();

		Math::tRect result;
		result.mL = m16_9ClipBox.mL + mClipBox.mL * w;
		result.mR = m16_9ClipBox.mL + mClipBox.mR * w;
		result.mT = m16_9ClipBox.mT + mClipBox.mT * h;
		result.mB = m16_9ClipBox.mT + mClipBox.mB * h;
		return result;
	}


	Math::tRect tViewport::fComputeRect( f32 screenWidth, f32 screenHeight )
	{
		const f32 l = fRound<f32>( screenWidth * fClipBox( ).mL );
		const f32 t = fRound<f32>( screenHeight * fClipBox( ).mT );
		const f32 r = fRound<f32>( screenWidth * fClipBox( ).mR );
		const f32 b = fRound<f32>( screenHeight * fClipBox( ).mB );

		return Math::tRect( t, l, b, r );
	}

	Math::tRect tViewport::fComputeRect( const tScreenPtr& screen )
	{
		// JPodesta - script actually wants the screenspace viewport rect, which is always 1280x720
		const f32 screenWidth  = 1280.0f;
		const f32 screenHeight = 720.0f;
		return fComputeRect( screenWidth, screenHeight );
	}

	Math::tRect tViewport::fComputeSafeRect( f32 screenWidth, f32 screenHeight, const Math::tVec2u& safeEdge )
	{
		Math::tRect rect = fComputeRect( screenWidth, screenHeight );

		rect.mL += safeEdge.x * ( 1.f - fClipBox( ).mL );
		rect.mT += safeEdge.y * ( 1.f - fClipBox( ).mT );
		rect.mR -= safeEdge.x * ( fClipBox( ).mR - 0.f );
		rect.mB -= safeEdge.y * ( fClipBox( ).mB - 0.f );

		return rect;
	}

	Math::tRect tViewport::fComputeSafeRect( const tScreenPtr& screen )
	{
		// JPodesta - script actually wants the screenspace viewport rect, which is always 1280x720 (or half that if splitscreen)
		const f32 screenWidth  = 1280.0f;
		const f32 screenHeight = 720.0f;
		const Math::tVec2u safeEdge = screen->fComputeGuiSafeEdge( );

		return fComputeSafeRect( screenWidth, screenHeight, safeEdge );
	}


}}

 
