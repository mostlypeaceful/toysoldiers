#ifndef __tSolidColorMaterial__
#define __tSolidColorMaterial__
#include "tMaterial.hpp"

namespace Sig { namespace Gfx
{
	struct base_export tSolidColorRenderVertex
	{
		Math::tVec3f	mP;
		u32				mColor;

		inline tSolidColorRenderVertex( ) { }
		inline tSolidColorRenderVertex( const Math::tVec3f& p, u32 color = 0xFFFFFFFF ) : mP( p ), mColor( color ) { }
	};

	class base_export tSolidColorMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tSolidColorMaterial, 0xECEAE8B1 );
		define_dynamic_cast(tSolidColorMaterial, tMaterial);
	public:

		static const tVertexFormat cVertexFormat;
		static tFilePathPtr fMaterialFilePath( );

		enum tShaderSlots
		{
			cShaderSlotVS,
			cShaderSlotPS,

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

		tSolidColorMaterial( );
		tSolidColorMaterial( tNoOpTag );
		explicit tSolidColorMaterial( const tResourcePtr& solidColorMaterialFile );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual b32 fRendersDepth( ) const { return false; }
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
	};

	typedef tRefCounterPtr<tSolidColorMaterial> tSolidColorMaterialPtr;

}}


#endif//__tSolidColorMaterial__
