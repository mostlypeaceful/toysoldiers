#ifndef __tFullBrightMaterial__
#define __tFullBrightMaterial__
#include "tMaterial.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	struct base_export tFullBrightRenderVertex
	{
		Math::tVec3f	mP;
		u32				mColor;
		Math::tVec2f	mUv;

		inline tFullBrightRenderVertex( ) { }
		inline tFullBrightRenderVertex( const Math::tVec3f& p, u32 color )
			: mP( p ), mColor( color ) { }
		inline tFullBrightRenderVertex( const Math::tVec3f& p, u32 color, const Math::tVec2f& uv )
			: mP( p ), mColor( color ), mUv( uv ) { }
	};

	class base_export tFullBrightMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tFullBrightMaterial, 0xB3656786 );
		define_dynamic_cast(tFullBrightMaterial, tMaterial);
	public:

		enum tShaderSlots
		{
			cShaderSlotVS,
			cShaderSlotPS,
			cShaderSlotPS_Gbuffer,

			// last
			cShaderSlotCount
		};

		enum tVSConstants
		{
			cVSLocalToWorld		= 0,
			cVSWorldToProj		= 3,
			cVSWorldEyePos		= 7,
		};

		enum tPSConstants
		{
			cPSRgbaTint			= 0,
		};

		static const tVertexFormat cVertexFormat;
		static tFilePathPtr fMaterialFilePath( );

	public:

		tEnum<tShaderSlots,u16>		mShaderSlotVS;
		tEnum<tShaderSlots,u16>		mShaderSlotPS;

		tTextureReference			mColorMap;

	public:

		tFullBrightMaterial( );
		tFullBrightMaterial( tNoOpTag );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
	};

	typedef tRefCounterPtr<tFullBrightMaterial> tFullBrightMaterialPtr;

}}


#endif//__tFullBrightMaterial__
