#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tRenderState.hpp"
#include "tDevice.hpp"
#include "tRenderContext.hpp"

namespace Sig { namespace Gfx
{
	devvar( bool, Renderer_Shadows_FlipFaces, false );
	devvar( u32, Renderer_Settings_DepthBiasStep, 5 );

	namespace
	{
		D3DBLEND fConvertBlendMode( tRenderState::tBlendMode blendMode )
		{
			D3DBLEND o = D3DBLEND_ONE;

			switch( blendMode )
			{
			case tRenderState::cBlendOne:				o = D3DBLEND_ONE; break;
			case tRenderState::cBlendZero:				o = D3DBLEND_ZERO; break;
			case tRenderState::cBlendSrcAlpha:			o = D3DBLEND_SRCALPHA; break;
			case tRenderState::cBlendOneMinusSrcAlpha:	o = D3DBLEND_INVSRCALPHA; break;
			case tRenderState::cBlendSrcColor:			o = D3DBLEND_SRCCOLOR; break;
			case tRenderState::cBlendOneMinusSrcColor:	o = D3DBLEND_INVSRCCOLOR; break;
			case tRenderState::cBlendDstAlpha:			o = D3DBLEND_DESTALPHA; break;
			case tRenderState::cBlendOneMinusDstAlpha:	o = D3DBLEND_INVDESTALPHA; break;
			case tRenderState::cBlendDstColor:			o = D3DBLEND_DESTCOLOR; break;
			case tRenderState::cBlendOneMinusDstColor:	o = D3DBLEND_INVDESTCOLOR; break;
			default:									sigassert( !"invalid blend mode!" ); break;
			}

			return o;
		}

		D3DBLENDOP fConvertBlendOp( u32 blendOpFlags )
		{
			D3DBLENDOP o = D3DBLENDOP_ADD;

			switch( blendOpFlags )
			{
			case tRenderState::cBlendOpSubtract:		o = D3DBLENDOP_SUBTRACT; break;
			case tRenderState::cBlendOpRevSubtract:		o = D3DBLENDOP_REVSUBTRACT; break;
			case tRenderState::cBlendOpMin:				o = D3DBLENDOP_MIN; break;
			case tRenderState::cBlendOpMax:				o = D3DBLENDOP_MAX; break;
			}

			return o;
		}

		inline b32 fCompareStateToLastApplied( u32 flags, const tRenderState& rs, const tRenderState& lastApplied, b32 lastStateInvalid )
		{
			return !lastStateInvalid && ( lastApplied.fQuery( flags ) == rs.fQuery( flags ) );
		}
	}

	void tRenderState::fApplyInternal( const tDevicePtr& device, const tRenderContext& context ) const
	{
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		const tRenderState& lastApplied = device->fLastAppliedRenderState( );
		const b32 lastStateInvalid = device->fLastRenderStateInvalid( );

#define set_render_state d3ddev->SetRenderState

		//
		// configure cull mode...

		if( fQuery( cPolyTwoSided ) )
			set_render_state( D3DRS_CULLMODE, D3DCULL_NONE );
		else
		{
			b32 flip = fQuery( cPolyFlipped );
			if( Renderer_Shadows_FlipFaces && context.mRenderPassMode == tRenderState::cRenderPassShadowMap )
				flip = !flip;

			if( flip )
				set_render_state( D3DRS_CULLMODE, D3DCULL_CCW );
			else
				set_render_state( D3DRS_CULLMODE, D3DCULL_CW );
		}

		//
		// configure blend mode...

		const tBlendMode srcBlend = fGetSrcBlendMode( );
		const tBlendMode dstBlend = fGetDstBlendMode( );
		const b32 alphaBlend = fQuery( cAlphaBlend );
		const u32 blendOp = mFlags & cBlendOpMask;

		if( lastStateInvalid || lastApplied.fGetSrcBlendMode( ) != srcBlend )
			set_render_state( D3DRS_SRCBLEND, fConvertBlendMode( srcBlend ) );

		if( lastStateInvalid || lastApplied.fGetDstBlendMode( ) != dstBlend )
			set_render_state( D3DRS_DESTBLEND, fConvertBlendMode( dstBlend ) );

		if( !fCompareStateToLastApplied( cAlphaBlend, *this, lastApplied, lastStateInvalid ) )
			set_render_state( D3DRS_ALPHABLENDENABLE, alphaBlend ? TRUE : FALSE );

		if( lastStateInvalid || ( ( lastApplied.mFlags & cBlendOpMask ) != blendOp ) )
			set_render_state( D3DRS_BLENDOP, fConvertBlendOp( blendOp ) );


		//
		// configure cut out state...

		const b32 cutOut = fQuery( cCutOut );
		const DWORD cutOutThresh = mCutOutThreshold;

		if( !fCompareStateToLastApplied( cCutOut, *this, lastApplied, lastStateInvalid ) )
			set_render_state( D3DRS_ALPHATESTENABLE, cutOut ? TRUE : FALSE );
		if( lastStateInvalid || lastApplied.fGetCutOutThreshold( ) != cutOutThresh )
			set_render_state( D3DRS_ALPHAREF, cutOutThresh );

		//
		// configure color buffer mode...

		const u32 colorWriteMask = D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE;
		const b32 colorBuffer = fQuery( cColorBuffer );

		if( !fCompareStateToLastApplied( cColorBuffer, *this, lastApplied, lastStateInvalid ) )
			set_render_state( D3DRS_COLORWRITEENABLE, colorBuffer ? colorWriteMask : 0 );

		//
		// configure depth buffer mode...

		const b32 depthBuffer = fQuery( cDepthBuffer );
		const b32 depthWrite = fQuery( cDepthWrite );
		const b32 depthModeLess = fQuery( cDepthModeLess );

		if( !fCompareStateToLastApplied( cDepthBuffer, *this, lastApplied, lastStateInvalid ) )
			set_render_state( D3DRS_ZENABLE, depthBuffer ? D3DZB_TRUE : D3DZB_FALSE );

		if( !fCompareStateToLastApplied( cDepthModeLess, *this, lastApplied, lastStateInvalid ) )
			set_render_state( D3DRS_ZFUNC, depthModeLess ? D3DCMP_LESS : D3DCMP_LESSEQUAL );

		if( !fCompareStateToLastApplied( cDepthWrite, *this, lastApplied, lastStateInvalid ) )
			set_render_state( D3DRS_ZWRITEENABLE, depthWrite ? TRUE : FALSE );

		//
		// configure stencil buffer...

		const b32 stencilBuffer = fQuery( cStencilBuffer );

		if( !fCompareStateToLastApplied( cStencilBuffer, *this, lastApplied, lastStateInvalid ) )
			set_render_state( D3DRS_STENCILENABLE, stencilBuffer ? TRUE : FALSE );

		//
		// configure fill mode...

		const b32 wireFrame = fQuery( cFillWireFrame );

		if( !fCompareStateToLastApplied( cFillWireFrame, *this, lastApplied, lastStateInvalid ) )
			set_render_state( D3DRS_FILLMODE, wireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

		//
		// configure depth bias...

		const s8 depthBias = fGetDepthBias( );
		const s8 slopeScaleBias = fGetSlopeScaleBias( );
		const f32 oneBitOf24BitDepthBuffer = 1.f / 0xFFFFFF;
		const f32 defaultDepthBiasIncrement = Renderer_Settings_DepthBiasStep * oneBitOf24BitDepthBuffer;

		if( lastStateInvalid || lastApplied.fGetDepthBias( ) != depthBias )
		{
			const f32 fpBias = depthBias * defaultDepthBiasIncrement;
			set_render_state( D3DRS_DEPTHBIAS, *( u32* )&fpBias );
		}

		if( lastStateInvalid || lastApplied.fGetSlopeScaleBias( ) != slopeScaleBias )
		{
			const f32 fpBias = slopeScaleBias * defaultDepthBiasIncrement;
			set_render_state( D3DRS_SLOPESCALEDEPTHBIAS, *( u32* )&fpBias );
		}

#undef set_render_state
	}

}}
#endif//#if defined( platform_pcdx9 )
