#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tEdramTilingDesc.hpp"
#include "xgraphics.h"

namespace Sig { namespace Gfx
{

	const tEdramTilingDesc tEdramTilingDesc::gTilingDescs[cScenarioCount] =
	{
		{
			//L"1280x720 No MSAA",
			1280, 720, D3DMULTISAMPLE_NONE, 0, D3DSEQM_PRECLIP, 1,
			{
				{ 0,   0, 1280, 720 },
			}
		},
		{
			//L"1280x720 2xMSAA Horizontal Split",
			1280, 720, D3DMULTISAMPLE_2_SAMPLES, 0, D3DSEQM_PRECLIP, 2,
			{
				{ 0,   0, 1280, 384 },
				{ 0, 384, 1280, 720 }
			}
		},
		{
			//L"1280x720 2xMSAA Vertical Split",
			1280, 720, D3DMULTISAMPLE_2_SAMPLES, 0, D3DSEQM_PRECLIP, 2,
			{
				{   0, 0,  640, 720 },
				{ 640, 0, 1280, 720 }
			}
		},
		{
			//L"1280x720 2xMSAA Horizontal Split, One Pass Z Pass",
			1280, 720, D3DMULTISAMPLE_2_SAMPLES, D3DTILING_ONE_PASS_ZPASS | D3DTILING_FIRST_TILE_INHERITS_DEPTH_BUFFER, D3DSEQM_CULLED, 2,
			{
				{ 0,   0, 1280, 384 },
				{ 0, 384, 1280, 720 }
			}
		},
		{
			//L"1280x720 2xMSAA Vertical Split, One Pass Z Pass",
			1280, 720, D3DMULTISAMPLE_2_SAMPLES, D3DTILING_ONE_PASS_ZPASS, D3DSEQM_CULLED, 2,
			{
				{   0, 0,  640, 720 },
				{ 640, 0, 1280, 720 }
			}
		},
		{
			//L"1280x720 4xMSAA Horizontal Split",
			1280, 720, D3DMULTISAMPLE_4_SAMPLES, 0, D3DSEQM_PRECLIP, 3,
			{
				{ 0,   0, 1280, 256 },
				{ 0, 256, 1280, 512 },
				{ 0, 512, 1280, 720 },
			}
		},
		{
			//L"1280x720 4xMSAA Vertical Split",
			1280, 720, D3DMULTISAMPLE_4_SAMPLES, 0, D3DSEQM_PRECLIP, 4,
			{
				{   0,   0,  320, 720 },
				{ 320,   0,  640, 720 },
				{ 640,   0,  960, 720 },
				{ 960,   0, 1280, 720 },
			}
		},
	};

	const tEdramTilingDesc& tEdramTilingDesc::fGetDesc( tScenario scenario )
	{
		sigassert( scenario < cScenarioCount );
		return gTilingDescs[ scenario ];
	}

	const tEdramTilingDesc& tEdramTilingDesc::fGetDefaultDescFromMSAAPower( u32 msaaPower )
	{
		switch( msaaPower )
		{
		case 0: return fGetDesc( cScenario_1280x720 );
		//case 1: return fGetDesc( cScenario_1280x720_Msaa2xVert_1ZPass );
		case 1: return fGetDesc( cScenario_1280x720_Msaa2xVert );
		case 2: return fGetDesc( cScenario_1280x720_Msaa4xVert );
		default: break;
		}

		log_warning( Log::cFlagGraphics, "Invalid MSAA Power (" << msaaPower << ") passed to tEdramTilingDesc::fGetDefaultDescFromMSAAPower." );
		return fGetDesc( cScenario_1280x720 );
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
