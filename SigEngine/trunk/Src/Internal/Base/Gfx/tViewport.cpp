#include "BasePch.hpp"
#include "tViewport.hpp"
#include "tScreen.hpp"
#include "tRenderContext.hpp"
#include "tMaterial.hpp"
#include "tLightCombo.hpp"



namespace Sig { namespace Gfx
{
	tViewport::tViewport( u32 vpIndex, tScreen& screen )
		: mClipBox( 0.f, 0.f, 1.f, 1.f )
		, mViewportIndex( vpIndex )
		, mIsVirtual( false )
		, mScreen( screen )
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
	}

	Math::tRect tViewport::fComputeRect( f32 screenWidth, f32 screenHeight ) const
	{
		const f32 l = fRound<f32>( screenWidth * fClipBox( ).mL );
		const f32 t = fRound<f32>( screenHeight * fClipBox( ).mT );
		const f32 r = fRound<f32>( screenWidth * fClipBox( ).mR );
		const f32 b = fRound<f32>( screenHeight * fClipBox( ).mB );

		return Math::tRect( t, l, b, r );
	}

	Math::tRect tViewport::fComputeRect( ) const
	{
		const f32 screenWidth  = ( f32 )mScreen.fCreateOpts( ).mBackBufferWidth;
		const f32 screenHeight = ( f32 )mScreen.fCreateOpts( ).mBackBufferHeight;

		return fComputeRect( screenWidth, screenHeight );
	}

	Math::tRect tViewport::fComputeSafeRect( f32 screenWidth, f32 screenHeight, const Math::tVec2u& safeEdge ) const
	{
		Math::tRect rect = fComputeRect( screenWidth, screenHeight );

		rect.mL += safeEdge.x * ( 1.f - fClipBox( ).mL );
		rect.mT += safeEdge.y * ( 1.f - fClipBox( ).mT );
		rect.mR -= safeEdge.x * ( fClipBox( ).mR - 0.f );
		rect.mB -= safeEdge.y * ( fClipBox( ).mB - 0.f );

		return rect;
	}

	Math::tRect tViewport::fComputeSafeRect( ) const
	{
		const f32 screenWidth  = ( f32 )mScreen.fCreateOpts( ).mBackBufferWidth;
		const f32 screenHeight = ( f32 )mScreen.fCreateOpts( ).mBackBufferHeight;
		const Math::tVec2u safeEdge = mScreen.fComputeGuiSafeEdge( );

		return fComputeSafeRect( screenWidth, screenHeight, safeEdge );
	}
	
	b32 tViewport::fProjectToScreenClamped( const Math::tVec3f& worldPos, Math::tVec3f& posOut ) const
	{
		const Gfx::tCamera& worldCam = fRenderCamera( );
		return fProjectToScreenClamped( worldCam, worldPos, posOut );
	}

	b32 tViewport::fProjectToScreenClamped( const Gfx::tCamera& camera, const Math::tVec3f& worldPos, Math::tVec3f& posOut ) const
	{
		Math::tRect rect = fComputeRect( );

		b32 onScreen = true;
		Math::tVec3f screenPos = fProjectToScreen( camera, worldPos );
		if( screenPos.z > 1.f || screenPos.z < 0.f )
		{
			//behind camera, mirror projection
			posOut.x = rect.fWidth( ) - screenPos.x;
			posOut.y = rect.fHeight( ) - screenPos.y;
			posOut.z = screenPos.z;

			onScreen = false;
		}
		else 
			posOut = Math::tVec3f( screenPos.fXY( ), 0.f );

		return onScreen;
	}

	Math::tVec3f tViewport::fProjectToScreen( const Math::tVec3f& worldPos ) const
	{
		const Gfx::tCamera& worldCam = fRenderCamera( );
		return fProjectToScreen( worldCam, fComputeRect( ), worldPos );
	}

	Math::tVec3f tViewport::fProjectToScreen( const Gfx::tCamera& camera, const Math::tVec3f& worldPos ) const
	{
		return fProjectToScreen( camera, fComputeRect( ), worldPos );
	}

	Math::tVec3f tViewport::fProjectToScreen( const Gfx::tCamera& camera, const Math::tRect& safeRect, const Math::tVec3f& worldPos ) const
	{
		const Math::tVec3f projPos = camera.fProject( worldPos );
		return Math::tVec3f( 
			safeRect.mL + fRound<f32>( projPos.x * safeRect.fWidth( ) ), 
			safeRect.mT + fRound<f32>( projPos.y * safeRect.fHeight( ) ), 
			projPos.z );
	}

	Math::tRectf tViewport::fProjectToScreen( const Math::tAabbf& worldBounds, b32& entireOutsideFrustumOutput ) const
	{
		const Gfx::tCamera& worldCam = fRenderCamera( );
		return fProjectToScreen( worldCam, fComputeRect( ), worldBounds, entireOutsideFrustumOutput );
	}

	Math::tRectf tViewport::fProjectToScreen( const Gfx::tCamera& camera, const Math::tRect& safeRect, const Math::tAabbf& worldBounds, b32& entireOutsideFrustumOutput ) const
	{
		Math::tRectf bounds( Math::cInfinity, Math::cInfinity, -Math::cInfinity, -Math::cInfinity );
		entireOutsideFrustumOutput = true;

		for( u32 i = 0; i < 8; ++i )
		{
			Math::tVec3f screenPt = fProjectToScreen( camera, safeRect, worldBounds.fCorner( i ) );
			if( screenPt.z <= 1.f && screenPt.z >= 0.f )
				entireOutsideFrustumOutput = false; // one corner at least was inside frustum

			bounds |= screenPt.fXY( );
		}

		return bounds;
	}


}}

 
