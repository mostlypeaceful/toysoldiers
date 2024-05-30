#include "BasePch.hpp"
#include "tQuadTrailEntity.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tDefaultAllocators.hpp"

using namespace Sig::Math;

namespace Sig { namespace FX
{
	tQuadTrailEntity::tElement tQuadTrailEntity::fCatmullRom( const tElement& p0, const tElement& p1, const tElement& p2, const tElement& p3, f32 t )
	{
		tElement o;

		o.mPosition = Math::fCatmullRom( 
			p0.mPosition,
			p1.mPosition,
			p2.mPosition,
			p3.mPosition,
			t );

		o.mAxis = Math::fCatmullRom( 
			p0.mAxis,
			p1.mAxis,
			p2.mAxis,
			p3.mAxis,
			t );

		o.mAgeAlpha = Math::fCatmullRom( 
			p0.mAgeAlpha,
			p1.mAgeAlpha,
			p2.mAgeAlpha,
			p3.mAgeAlpha,
			t );

		o.mPredicted = false;

		return o;
	}

	tQuadTrailEntity::tElement tQuadTrailEntity::fCatmullRom( const tElement& p0, const tElement& p1, const tElement& p2, f32 t )
	{
		tElement p3 = p2;
		p3.mPosition = p2.mPosition + 1.f * ( p2.mPosition - p1.mPosition );

		tElement o = fCatmullRom( p0, p1, p2, p3, t );
		o.mPredicted = true;

		return o;
	}

	tQuadTrailEntity::tQuadTrailEntity( const tResourcePtr& texture, const Gfx::tVertexColor& tint, const Gfx::tRenderState* renderState, b32 beefy )
		: mBounds( -0.1f, +0.1f )
		, mColor( tint )
		, mBeefy( beefy )
	{
		fSetLockedToParent( false );

		tResourcePtr colorMapResource = texture;

		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );
		fResetDeviceObjectsTexture(
			Gfx::tDevice::fGetDefaultDevice( )
			, colorMapResource
			, allocator.mFullBrightMaterialFile
			, allocator.mFullBrightGeomAllocator
			, allocator.mIndexAllocator );

		fGeometry( ).fSetRenderStateOverride( renderState );
	}

	void tQuadTrailEntity::fOnSpawn( )
	{
		tWorldSpaceQuads::fOnSpawn( );

		fOnPause( false );
	}

	b32 tQuadTrailEntity::fReadyForDeletion( )
	{
		return !fAlive( );
	}
	
	void tQuadTrailEntity::fOnDelete( )
	{
		tWorldSpaceQuads::fOnDelete( );
	}

	void tQuadTrailEntity::fFillGraphics( )
	{
		// fill out system memory of quads
		const b32 beefyQuads = mBeefy;
		const u32 numQuadsPerElem = beefyQuads ? 3 : 1;
		const u32 quadCnt = numQuadsPerElem * mElements.fNumItems( );
		if( quadCnt > 0 )
		{
			u32 qIndex = 0;
			fSetQuadCount( quadCnt );
			mBounds.fInvalidate( );

			for( u32 q = 0; q < quadCnt; q += numQuadsPerElem )
			{
				const u32 eIndex = q / numQuadsPerElem;
				const tElement& e1 = mElements[ eIndex ];
				const tElement& e2 = (eIndex == mElements.fNumItems( )-1) ? mHead : mElements[ eIndex + 1 ];

				const f32 alpha1 = e1.mAlpha * e1.mAgeAlpha;
				const f32 alpha2 = e2.mAlpha * e2.mAgeAlpha;

				f32 tv1 = 1.0f - alpha1;
				f32 tv2 = 1.0f - alpha2;

				if( eIndex == 0 ) tv1 = 1.0f;
				if( eIndex == mElements.fNumItems( )-1 ) tv2 = 0.0f;

				fPushQuad( 
					fQuad( qIndex++ ),
					mBounds,
					e1.mPosition, e1.mAxis, alpha1, tv1,
					e2.mPosition, e2.mAxis, alpha2, tv2 );

				if( beefyQuads )
				{
					const f32 e1axislen = e1.mAxis.fLength( );
					const f32 e2axislen = e2.mAxis.fLength( );
					const Math::tVec3f e1e2 = e2.mPosition - e1.mPosition;
					const Math::tVec3f up1 = e1.mAxis.fCross(  e1e2 ).fNormalizeSafe( Math::tVec3f::cYAxis ) * e1axislen;
					const Math::tVec3f up2 = e2.mAxis.fCross(  e1e2 ).fNormalizeSafe( Math::tVec3f::cYAxis ) * e2axislen;

					//fPushQuad( 
					//	fQuad( qIndex++ ),
					//	mBounds,
					//	e1.mPosition, up1, alpha1, tv1,
					//	e2.mPosition, up2, alpha2, tv2 );

					fPushQuad( 
						fQuad( qIndex++ ),
						mBounds,
						e1.mPosition, Math::fLerp( e1.mAxis, up1, 0.5f ).fSetLength( e1axislen ), alpha1, tv1,
						e2.mPosition, Math::fLerp( e2.mAxis, up2, 0.5f ).fSetLength( e2axislen ), alpha2, tv2 );

					fPushQuad( 
						fQuad( qIndex++ ),
						mBounds,
						e1.mPosition, Math::fLerp( e1.mAxis, up1, 1.5f ).fSetLength( e1axislen ), alpha1, tv1,
						e2.mPosition, Math::fLerp( e2.mAxis, up2, 1.5f ).fSetLength( e2axislen ), alpha2, tv2 );
				}
			}
		}
	}

	void tQuadTrailEntity::fThinkST( f32 dt )
	{
		fSetObjectSpaceBox( mBounds );
		fCreateGeometry( *Gfx::tDevice::fGetDefaultDevice( ) );
	}

	void tQuadTrailEntity::fPushQuad( Gfx::tFullBrightRenderVertex *verts, tAabbf& bounds, const tVec3f& p1, const tVec3f& axis1, f32 alpha1, f32 tv1
		, const tVec3f& p2, const tVec3f& axis2, f32 alpha2, f32 tv2 )
	{
		Gfx::tVertexColor color = mColor;
		color.mA = u8(alpha1 * 255.0f);

		u32 gpuColor = color.fForGpu( );

		verts[0].mP = p1 - axis1;
		verts[0].mColor = gpuColor;
		verts[0].mUv = tVec2f( 1.0f, tv1 );
		bounds |= verts[0].mP;

		verts[1].mP = p1 + axis1;
		verts[1].mColor = gpuColor;
		verts[1].mUv = tVec2f( 0.0f, tv1 );
		bounds |= verts[1].mP;


		color.mA = u8(alpha2 * 255.0f);
		gpuColor = color.fForGpu( );

		verts[2].mP = p2 + axis2;
		verts[2].mColor = gpuColor;
		verts[2].mUv = tVec2f( 0.0f, tv2 );
		bounds |= verts[2].mP;

		verts[3].mP = p2 - axis2;
		verts[3].mColor = gpuColor;
		verts[3].mUv = tVec2f( 1.0f, tv2 );
		bounds |= verts[3].mP;
	}

}}
