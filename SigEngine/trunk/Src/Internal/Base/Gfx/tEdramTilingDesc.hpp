#ifndef __tEdramTilingDesc__
#define __tEdramTilingDesc__

namespace Sig { namespace Gfx
{
	struct tEdramTilingDesc
	{
	public:
		static const u32 cMaxNumTiles = 8;

		static tEdramTilingDesc fChooseTiling( u32 width, u32 height, u32 numTargets, u32 antiAliasing, u32 bitsPerPixel, u32 bitsPerDepth );

	public:
		u32							mScreenWidth;
		u32							mScreenHeight;
		D3DMULTISAMPLE_TYPE			mMsaaType;
		D3DSCREENEXTENTQUERYMODE	mScreenExtentQueryMode;
		u32							mTileCount;
		D3DRECT						mTilingRects[cMaxNumTiles];

	public:
		Math::tVec2u fComputeTileDims( ) const;

	private:
		Math::tVec2u fLargestTileRectSize( ) const;
	};
}}


#endif//__tEdramTilingDesc__
