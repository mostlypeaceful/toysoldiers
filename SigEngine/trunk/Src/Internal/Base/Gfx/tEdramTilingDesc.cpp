#include "BasePch.hpp"

#if defined( platform_xbox360 )
#include "tEdramTilingDesc.hpp"
#include "xgraphics.h"

namespace Sig { namespace Gfx
{

	tEdramTilingDesc tEdramTilingDesc::fChooseTiling( u32 width, u32 height, u32 numTargets, u32 antiAliasing, u32 bitsPerPixel, u32 bitsPerDepth )
	{
		const u32 cSizeOfByte = 8;
		u32 aaScale = (u32)pow( 2.f, (f32)antiAliasing );
		u32 frameBytes = width * height * (bitsPerPixel * numTargets + bitsPerDepth) / cSizeOfByte * aaScale;
		u32 tiles = fRoundUp<u32>( (f32)frameBytes / GPU_EDRAM_SIZE );

		// How ever many tiles their are, tile them horizontally.
		u32 tilesHorz = tiles;
		u32 tilesVert = 1;

		u32 rectWidth = fRoundUp<u32>( (f32)width / tilesHorz );
		u32 rectHeight = fRoundUp<u32>( (f32)height / tilesVert );

		tEdramTilingDesc desc;
		desc.mScreenWidth = width;
		desc.mScreenHeight = height;

		switch( aaScale )
		{
		case 1: desc.mMsaaType = D3DMULTISAMPLE_NONE; break;
		case 2: desc.mMsaaType = D3DMULTISAMPLE_2_SAMPLES; break;
		case 4: desc.mMsaaType = D3DMULTISAMPLE_4_SAMPLES; break;
		default: sigassert( !"Invalid multisample type!" ) ;
		}

		desc.mScreenExtentQueryMode = D3DSEQM_PRECLIP;
		desc.mTileCount = (tiles > 1) ? tiles : 0; // zero means: tiling disabled.

		//build tiles
		u32 xPos = 0;
		u32 yPos = 0;
		for( u32 y = 0; y < tilesVert; ++y )
		{
			u32 th = (y == tilesVert - 1 ) ? height - yPos : rectHeight;
			for( u32 x = 0; x < tilesHorz; ++x )
			{
				u32 tw = (x==tilesHorz - 1) ? width - xPos : rectWidth;

				D3DRECT& r = desc.mTilingRects[ y * tilesHorz + x ];
				r.x1 = xPos;
				r.y1 = yPos;
				r.x2 = xPos + tw;
				r.y2 = yPos + th;

				xPos += tw;
			}

			yPos += th;
			xPos = 0;
		}

		return desc;
	}

	Math::tVec2u tEdramTilingDesc::fComputeTileDims( ) const
	{
		// Find largest tiling rect size
		const Math::tVec2u largestTileSize = fLargestTileRectSize( );

		Math::tVec2u dims( 0, 0 );
		switch( mMsaaType )
		{
		case D3DMULTISAMPLE_NONE:
			dims.x = XGNextMultiple( largestTileSize.x, GPU_EDRAM_TILE_WIDTH_1X );
			dims.y = XGNextMultiple( largestTileSize.y, GPU_EDRAM_TILE_HEIGHT_1X );
			break;
		case D3DMULTISAMPLE_2_SAMPLES:
			dims.x = XGNextMultiple( largestTileSize.x, GPU_EDRAM_TILE_WIDTH_2X );
			dims.y = XGNextMultiple( largestTileSize.y, GPU_EDRAM_TILE_HEIGHT_2X );
			break;
		case D3DMULTISAMPLE_4_SAMPLES:
			dims.x = XGNextMultiple( largestTileSize.x, GPU_EDRAM_TILE_WIDTH_4X );
			dims.y = XGNextMultiple( largestTileSize.y, GPU_EDRAM_TILE_HEIGHT_4X );
			break;
		}

		if( mTileCount > 1 )
		{
			// Expand tile surface dimensions to texture tile size, if it isn't already
			dims.x = XGNextMultiple( dims.x, GPU_TEXTURE_TILE_DIMENSION );
			dims.y = XGNextMultiple( dims.y, GPU_TEXTURE_TILE_DIMENSION );
		}

		return dims;
	}

	Math::tVec2u tEdramTilingDesc::fLargestTileRectSize( ) const
	{
		Math::tVec2u dims( 0, 0 );
		for( u32 i = 0; i < mTileCount; i++ )
		{
			const u32 w = mTilingRects[ i ].x2 - mTilingRects[ i ].x1;
			const u32 h = mTilingRects[ i ].y2 - mTilingRects[ i ].y1;
			dims.x = fMax( w, dims.x );
			dims.y = fMax( h, dims.y );
		}
		return dims;
	}



}}

#endif//#if defined( platform_xbox360 )
