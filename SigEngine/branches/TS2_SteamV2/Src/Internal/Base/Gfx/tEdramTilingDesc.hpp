#ifndef __tEdramTilingDesc__
#define __tEdramTilingDesc__

namespace Sig { namespace Gfx
{
	struct tEdramTilingDesc
	{
	public:

		enum tScenario
		{
			cScenario_1280x720,
			cScenario_1280x720_Msaa2xHorz,
			cScenario_1280x720_Msaa2xVert,
			cScenario_1280x720_Msaa2xHorz_1ZPass,
			cScenario_1280x720_Msaa2xVert_1ZPass,
			cScenario_1280x720_Msaa4xHorz,
			cScenario_1280x720_Msaa4xVert,

			// last
			cScenarioCount
		};
	public:
		static const tEdramTilingDesc& fGetDesc( tScenario scenario );
		static const tEdramTilingDesc& fGetDefaultDescFromMSAAPower( u32 msaaPower );

	public:
		u32							mScreenWidth;
		u32							mScreenHeight;
		D3DMULTISAMPLE_TYPE			mMsaaType;
		u32							mTilingFlags;
		D3DSCREENEXTENTQUERYMODE	mScreenExtentQueryMode;
		u32							mTileCount;
		D3DRECT						mTilingRects[8];

	public:
		Math::tVec2u fComputeTileDims( ) const;

	private:
		Math::tVec2u fLargestTileRectSize( ) const;
		static const tEdramTilingDesc gTilingDescs[cScenarioCount];
	};
}}


#endif//__tEdramTilingDesc__
