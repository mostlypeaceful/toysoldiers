#ifndef __tVertexShaderInputVars__
#define __tVertexShaderInputVars__
#include "tHlslVariable.hpp"

namespace Sig { namespace HlslGen
{

	class tools_export tVertexShaderVars : public tVariableRegistry
	{
		declare_singleton_define_own_ctor_dtor( tVertexShaderVars );
	private:
		tVertexShaderVars( );
		~tVertexShaderVars( );
	public:
		enum tVarId
		{
			// globals
			cGlobalModelPosToWorld,
			cGlobalModelNormalToWorld,
			cGlobalModelToView,
			cGlobalWorldToProjection,
			cGlobalViewToProjection,
			cGlobalViewToWorld,
			cGlobalWorldEyePos,
			cGlobalWorldToLight,
			cGlobalViewToLight,
			cGlobalSkinningPalette,
			cGlobalShadowMapSplits,
			cGlobalInstancingData,

			// inputs
			cInModelSpacePos,
			cInModelSpaceNormal,
			cInModelSpaceTangent,
			cInModelSpaceTangent3,
			cInVertexColor,
			cInUv0,
			cInUv1,
			cInUv2,
			cInUv3,
			cInUv01,
			cInUv23,
			cInBlendWeights,
			cInBlendIndices,

			// outputs
			cOutWorldSpacePos,
			cOutProjSpacePos,
			cOutProjSpaceDepth,
			cOutLitSpacePos,
			cOutLitSpaceNormal,
			cOutLitSpaceTangent,
			cOutVertexColor,
			cOutUv0,
			cOutUv1,
			cOutUv2,
			cOutUv3,
			cOutUv01,
			cOutUv23,
			cOutLightPos,

			// last
			cVarCount
		};

		static tHlslVariableConstPtr fVariable( tVarId ithVariable, tHlslPlatformId pid ) {
			return fInstance( ).fFindMostAppropriateVariable( ithVariable, pid ); 
		}
	};

}}

#endif//__tVertexShaderInputVars__
