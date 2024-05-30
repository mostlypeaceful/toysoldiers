#ifndef __tFontMaterial__
#define __tFontMaterial__
#include "tMaterial.hpp"

namespace Sig { namespace Gfx
{
	struct base_export tGlyphRenderVertex
	{
		Math::tVec3f	mP;
		Math::tVec2f	mUv;

		inline tGlyphRenderVertex( ) { }
		inline tGlyphRenderVertex( const Math::tVec3f& p, const Math::tVec2f& uv )
			: mP( p ), mUv( uv ) { }
	};

	class base_export tFontMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tFontMaterial, 0x1F0966F4 );
		define_dynamic_cast(tFontMaterial, tMaterial);
	public:

		enum tVSShaderSlot
		{
			cVSFirst = 0,

			cVSStandard = cVSFirst,

			// last
			cVSShaderCount
		};

		enum tPSShaderSlot
		{
			cPSFirst = cVSShaderCount,

			cPSStandard = cPSFirst,
			cPSOutline,

			// last
			cShaderSlotCount,
			cPSShaderCount = cShaderSlotCount - cPSFirst
		};

		enum tVSConstants
		{
			cVSLocalToWorld		= 0,
			cVSWorldToProj		= 3,
		};

		enum tPSConstants
		{
			cPSRgbaTint			= 0,
		};

		static const tVertexFormat cVertexFormat;
		static tFilePathPtr fMaterialFilePath( );

	public:

		tEnum<tVSShaderSlot,u16>	mVsSlot;
		tEnum<tPSShaderSlot,u16>	mPsSlot;
		tLoadInPlaceResourcePtr* 	mFontMap;

	public:
		tFontMaterial( );
		tFontMaterial( tNoOpTag );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;

	private:
		void fApplyTexture( const tDevicePtr& device, u32 slot, tLoadInPlaceResourcePtr* texture ) const;
	};

}}

#endif//__tFontMaterial__
