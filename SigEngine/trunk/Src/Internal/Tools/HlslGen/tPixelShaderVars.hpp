#ifndef __tPixelShaderInputVars__
#define __tPixelShaderInputVars__
#include "tHlslVariable.hpp"

namespace Sig { namespace HlslGen
{

	class tools_export tPixelShaderVars : public tVariableRegistry
	{
		declare_singleton_define_own_ctor_dtor( tPixelShaderVars );
	private:
		u32 mBaseConstantRegister;
		u32 mBaseSamplerRegister;
	private:
		tPixelShaderVars( );
		~tPixelShaderVars( );
	public:
		enum tVarId
		{
			// globals
			cGlobal_BackFace_ShadowMapEpsilon_TexelSize_Amount,
			cGlobal_ShadowMapEpsilon,
			cGlobal_ShadowMapTargetPos,
			cGlobal_ShadowMapSplits,
			cGlobalWorldToLightSpaceArray,
			cGlobalFogParams,
			cGlobalFogColor,
			cGlobalFlatParticleColor,
			cGlobalTime,
			cGlobalInstanceTint,
			cGlobalRimLight,
			cGlobalLightParams,
			cGlobalShadowMapSampler,
			cGlobalWorldEyePos,
			cGlobalDebugHalfLambert,
			cGlobalSphericalHarmonics,

			// inputs
			cInWorldSpacePos,
			cInProjSpaceDepth,
			cInFaceSign,
			cInLitSpacePos,
			cInLitSpaceNormal,
			cInLitSpaceTangent,
			cInVertexColor,
			cInUv0,
			cInUv1,
			cInUv2,
			cInUv3,
			cInUv01,
			cInUv23,
			cInLightPos,

			// common temporaries
			cTempEyeToVertexDir,
			cTempEyeToVertexLen,
			cTempEffectiveNormal,
			cTempEmissionResult,
			cTempAmbientResult,

			// outputs
			cOutColor0,
			cOutColor1,
			cOutColor2,
			cOutColor3,
			cOutColorDepth,

			// last
			cVarCount
		};

		enum tLightParamVarId
		{
			cLightParamDirection,
			cLightParamPosition,
			cLightParamAttenuation,
			cLightParamColorAmbient,
			cLightParamColorFront,
			cLightParamColorSurround,
			cLightParamColorBack,

			// last
			cLightParamCount
		};

		static tHlslVariableConstPtr fVariable( tVarId ithVariable, tHlslPlatformId pid = cPidDefault ) {
			return fInstance( ).fFindMostAppropriateVariable( ithVariable, pid ); 
		}
		static u32 fBaseConstantRegister( ) {
			return fInstance( ).mBaseConstantRegister;
		}
		static u32 fBaseSamplerRegister( ) {
			return fInstance( ).mBaseSamplerRegister;
		}
	};


}}

#endif//__tPixelShaderInputVars__