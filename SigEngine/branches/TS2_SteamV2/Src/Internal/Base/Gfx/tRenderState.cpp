#include "BasePch.hpp"
#include "tRenderState.hpp"
#include "tRenderBatch.hpp"
#include "tRenderContext.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	const tRenderState tRenderState::cDefaultColorOpaque( 
		tRenderState::cColorBuffer | tRenderState::cDepthBuffer | tRenderState::cDepthWrite,
		tRenderState::cBlendOne,
		tRenderState::cBlendZero,
		0 );

	const tRenderState tRenderState::cDefaultColorTransparent(
		tRenderState::cColorBuffer | tRenderState::cDepthBuffer | tRenderState::cAlphaBlend | tRenderState::cCutOut, 
		tRenderState::cBlendSrcAlpha,
		tRenderState::cBlendOneMinusSrcAlpha,
		0 );

	const tRenderState tRenderState::cDefaultColorCutOut( 
		tRenderState::cColorBuffer | tRenderState::cDepthBuffer | tRenderState::cDepthWrite | tRenderState::cCutOut,
		tRenderState::cBlendOne,
		tRenderState::cBlendZero,
		0 );

	const tRenderState tRenderState::cDefaultColorAdditive(
		tRenderState::cColorBuffer | tRenderState::cDepthBuffer | tRenderState::cAlphaBlend, 
		tRenderState::cBlendOne,
		tRenderState::cBlendOne,
		0 );

	tRenderState::tRenderState( u32 flags, tBlendMode srcBlend, tBlendMode dstBlend, u8 cutOutThresh )
		: mFlags( flags )
		, mCutOutThreshold( cutOutThresh )
		, mSrcDstBlend( 0 )
		, mDepthBias( 0 )
		, mSlopeScaleBias( 0 )
	{
		fSetSrcBlendMode( srcBlend );
		fSetDstBlendMode( dstBlend );
	}

	void tRenderState::fSetBlendOpFromIndex( u32 index )
	{
		fEnableDisable( cBlendOpMask, false );
		switch( index )
		{
		case 1: fEnableDisable( cBlendOpSubtract, true ); break;
		case 2: fEnableDisable( cBlendOpRevSubtract, true ); break;
		case 3: fEnableDisable( cBlendOpMin, true ); break;
		case 4: fEnableDisable( cBlendOpMax, true ); break;
		}
	}

	u32 tRenderState::fGetBlendOpAsIndex( ) const
	{
		if( mFlags & cBlendOpSubtract )		return 1;
		if( mFlags & cBlendOpRevSubtract )	return 2;
		if( mFlags & cBlendOpMin )			return 3;
		if( mFlags & cBlendOpMax )			return 4;
		/*default: additive*/				return 0;
	}

	void tRenderState::fApply( const tDevicePtr& device, const tRenderContext& context ) const
	{
		tRenderState copy = *this;

		if( context.mRenderPassMode == cRenderPassShadowMap )
		{
			// TODO anything?
			//copy.fEnableDisable( cColorBuffer, false );
		}
		else
		{
			if( context.mGlobalFillMode == cGlobalFillSmooth )
				copy.fEnableDisable( cFillWireFrame, false );
			else if( context.mGlobalFillMode == cGlobalFillWire )
				copy.fEnableDisable( cFillWireFrame, true );
			else if( context.mGlobalFillMode == cGlobalFillEdgedFace )
			{
				copy = cDefaultColorAdditive;
				copy.fEnableDisable( cFillWireFrame, true );
			}
		}

		copy.fApplyCopy( device, context );
	}

	void tRenderState::fApplyCopy( const tDevicePtr& device, const tRenderContext& context ) const
	{
		if( !device->fLastRenderStateInvalid( ) && device->fLastAppliedRenderState( ) == *this )
			return;

		fApplyInternal( device, context );

		device->fSetLastAppliedRenderState( *this );
		device->fInvalidateLastRenderState( false );
	}

}}

