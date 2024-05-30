#ifndef __tDefaultMaterial__
#define __tDefaultMaterial__
#include "tMaterial.hpp"

namespace Sig { namespace Gfx
{

	class base_export tDefaultMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDefaultMaterial, 0xF204FF10 );
		define_dynamic_cast(tDefaultMaterial, tMaterial);
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
		};

	public:
		tDefaultMaterial( );
		tDefaultMaterial( tNoOpTag );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
	};

}}


#endif//__tDefaultMaterial__
