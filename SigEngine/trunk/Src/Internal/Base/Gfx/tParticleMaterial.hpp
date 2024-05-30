#ifndef __tParticleMaterial__
#define __tParticleMaterial__
#include "tMaterial.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	struct base_export tParticleRenderVertex
	{
		Math::tVec3f	mP;
		u32				mColor;
		Math::tVec4f	mCorners;

		inline tParticleRenderVertex( ) { }
		inline tParticleRenderVertex( const Math::tVec3f& p, u32 color, const f32 sx, const f32 sy, f32 roll, f32 cutout )
			: mP( p ), mColor( color ), mCorners( sx, sy, roll, cutout ) { }
	};

	class base_export tParticleMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tParticleMaterial, 0xE942D6A );
		define_dynamic_cast(tParticleMaterial, tMaterial);
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
			cVSLocalToView		= 0,
			cVSViewToProj		= 3,
			cVSWorldEyePos		= 7,
		};

		enum tPSConstants
		{
			cPSRgbaTint			= 0,
		};

	public:

		tTextureReference			mEmissiveMap;

	public:

		tParticleMaterial( );
		tParticleMaterial( const tResourcePtr& particleMtlFile );
		tParticleMaterial( tNoOpTag );
		~tParticleMaterial( );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tParticleMaterial );

}}


#endif//__tParticleMaterial__
