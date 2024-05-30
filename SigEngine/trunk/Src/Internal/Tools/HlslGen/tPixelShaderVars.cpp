#include "ToolsPch.hpp"
#include "tPixelShaderVars.hpp"
#include "tMaterialGenBase.hpp"
#include "Gfx/DerivedShadeMaterialGlue.hpp"

namespace Sig { namespace HlslGen
{
	namespace
	{
		static void fAddGlobal( tHlslVariablePtr& varSlot, tHlslVariable* var, u32& baseRegister, Gfx::tShadeMaterialGlue* glue = 0 )
		{
			varSlot.fReset( var );
			if( glue )
				var->fSetMaterialGlue( glue );
			baseRegister += var->fRegisterCount( );
		}
	}

	tPixelShaderVars::tPixelShaderVars( )
		: mBaseConstantRegister( 0 )
		, mBaseSamplerRegister( 0 )
	{
		mVariables.fSetCount( cVarCount );

		// globals

		mVariables[ cGlobal_BackFace_ShadowMapEpsilon_TexelSize_Amount ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "gBackFace_ShadowMapEpsilon_TexelSize_Amount", mBaseConstantRegister, 4 ) );
		mVariables[ cGlobal_ShadowMapEpsilon ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "ShadowMapEpsilon", mBaseConstantRegister + 1, 4 ) );
		mVariables[ cGlobal_BackFace_ShadowMapEpsilon_TexelSize_Amount ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueBackFacePlusShadowMapParams( mBaseConstantRegister, mBaseConstantRegister + 1 ));
		mBaseConstantRegister += 2;

		fAddGlobal( mVariables[ cGlobal_ShadowMapTargetPos ][ cPidDefault ], tHlslVariable::fMakeGlobalVector( "gShadowMapTargetPos", mBaseConstantRegister, 4 ), mBaseConstantRegister,
			new Gfx::tPsGlueShadowMapTargetPos( mBaseConstantRegister ) );

		fAddGlobal( mVariables[ cGlobal_ShadowMapSplits ][ cPidDefault ], tHlslVariable::fMakeGlobalVector( "gShadowMapSplits", mBaseConstantRegister, 4 ), mBaseConstantRegister,
			new Gfx::tPsGlueShadowMapSplits( mBaseConstantRegister ) );

		fAddGlobal( mVariables[ cGlobalWorldToLightSpaceArray ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gWorldToLightArray", tHlslVariable::cGlobal, 4, 4, tHlslVariable::tSemantic( "c", mBaseConstantRegister ), Gfx::tMaterial::cMaxShadowLayers ), mBaseConstantRegister,
			new Gfx::tPsGlueWorldToLightSpaceArray( mBaseConstantRegister ) );

		mVariables[ cGlobalFogParams ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "gFogParams", mBaseConstantRegister++, 4 ) );
		mVariables[ cGlobalFogColor ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "gFogColor", mBaseConstantRegister++, 4 ) );
		mVariables[ cGlobalFogColor ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueFog( mBaseConstantRegister - 2, mBaseConstantRegister - 1 ) );

		mVariables[ cGlobalFlatParticleColor ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "gFlatParticleColor", mBaseConstantRegister++, 4 ) );
		mVariables[ cGlobalFlatParticleColor ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueFlatParticleColor( mBaseConstantRegister - 1 ) );

		mVariables[ cGlobalTime ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "gTime", mBaseConstantRegister++, 4 ) );
		mVariables[ cGlobalTime ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueTime( mBaseConstantRegister - 1 ) );

		mVariables[ cGlobalInstanceTint ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "gInstanceTint", mBaseConstantRegister++, 4 ) );
		mVariables[ cGlobalInstanceTint ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueInstanceTint( mBaseConstantRegister - 1 ) );

		const u32 numRimLightParams = 2;
		tDynamicArray<tHlslVariableConstPtr> rimLightMembers( numRimLightParams );
		rimLightMembers[ 0 ].fReset( tHlslVariable::fMakeVectorFloat( "mDirection", tHlslVariable::cMember, 4 ) );
		rimLightMembers[ 1 ].fReset( tHlslVariable::fMakeVectorFloat( "mColor", tHlslVariable::cMember, 4 ) );
		tHlslStructVariable* rimLight = new tHlslStructVariable( "gRimLightParams", tHlslVariable::cGlobal, "tRimLightParams", tHlslVariable::tSemantic( "c", mBaseConstantRegister ), rimLightMembers );
		mVariables[ cGlobalRimLight ][ cPidDefault ].fReset( rimLight );
		mVariables[ cGlobalRimLight ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueRimLightParams( mBaseConstantRegister ) );
		mBaseConstantRegister += rimLight->fRegisterCount( );

		mVariables[ cGlobalDebugHalfLambert ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "gHalfLambert", mBaseConstantRegister, 4 ) );
		mVariables[ cGlobalDebugHalfLambert ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueHalfLambert( mBaseConstantRegister ) );
		++mBaseConstantRegister;

		mVariables[ cGlobalSphericalHarmonics ][ cPidDefault ].fReset( tHlslVariable::fMakeGlobalVector( "gSphericalHarmonics", mBaseConstantRegister, 4, Gfx::tSphericalHarmonics::cFactorCount ) );
		mVariables[ cGlobalSphericalHarmonics ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueSphericalHarmonics( mBaseConstantRegister ) );
		mBaseConstantRegister += Gfx::tSphericalHarmonics::cFactorCount;

		const u32 maxLights = Gfx::tMaterial::cMaxLights;
		tDynamicArray<tHlslVariableConstPtr> lightParamsMembers( cLightParamCount );
		lightParamsMembers[ cLightParamDirection ].fReset( tHlslVariable::fMakeVectorFloat( "mDirection", tHlslVariable::cMember, 4 ) );
		lightParamsMembers[ cLightParamPosition ].fReset( tHlslVariable::fMakeVectorFloat( "mPosition", tHlslVariable::cMember, 4 ) );
		lightParamsMembers[ cLightParamAttenuation ].fReset( tHlslVariable::fMakeVectorFloat( "mAttenuation", tHlslVariable::cMember, 4 ) );
		lightParamsMembers[ cLightParamColorAmbient ].fReset( tHlslVariable::fMakeVectorFloat( "mColorAmbient", tHlslVariable::cMember, 4 ) );
		lightParamsMembers[ cLightParamColorFront ].fReset( tHlslVariable::fMakeVectorFloat( "mColorFront", tHlslVariable::cMember, 4 ) );
		lightParamsMembers[ cLightParamColorSurround ].fReset( tHlslVariable::fMakeVectorFloat( "mColorSurround", tHlslVariable::cMember, 4 ) );
		lightParamsMembers[ cLightParamColorBack ].fReset( tHlslVariable::fMakeVectorFloat( "mColorBack", tHlslVariable::cMember, 4 ) );

		tHlslStructVariable* lightParams = new tHlslStructVariable( "gLightParams", tHlslVariable::cGlobal, "tLightParams", tHlslVariable::tSemantic( "c", mBaseConstantRegister ), lightParamsMembers, maxLights );
		mVariables[ cGlobalLightParams ][ cPidDefault ].fReset( lightParams );
		mVariables[ cGlobalLightParams ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueLightParams( mBaseConstantRegister, maxLights ) );
		mBaseConstantRegister += lightParams->fRegisterCount( );


		const u32 shadowLayerCount = tMaterialGenBase::fShadowMapLayerCount( cPlatformNone );
		if( shadowLayerCount > 1 )
		{
			mVariables[ cGlobalShadowMapSampler ][ cPidDefault ].fReset( tHlslVariable::fMakeSampler3D( "gShadowMap", mBaseSamplerRegister++ ) );
			mVariables[ cGlobalShadowMapSampler ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueShadowMap( mBaseSamplerRegister - 1 ) );
		}
		else
		{
			mVariables[ cGlobalShadowMapSampler ][ cPidDefault ].fReset( tHlslVariable::fMakeSampler2D( "gShadowMap", mBaseSamplerRegister++ ) );
			mVariables[ cGlobalShadowMapSampler ][ cPidDefault ]->fSetMaterialGlue( new Gfx::tPsGlueShadowMap( mBaseSamplerRegister - 1 ) );
		}

		fAddGlobal( mVariables[ cGlobalWorldEyePos ][ cPidDefault ], tHlslVariable::fMakeVectorFloat( "gWorldEyePos", tHlslVariable::cGlobal, 3, tHlslVariable::tSemantic( "c", mBaseConstantRegister ) ), mBaseConstantRegister,
			new Gfx::tPsGlueWorldEyePos( mBaseConstantRegister ) );


		// inputs...
		mVariables[ cInFaceSign ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iFaceSign", tHlslVariable::cInput, 1, "VFACE" ) );
		mVariables[ cInLitSpaceNormal ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iLitSpaceNormal", tHlslVariable::cInput, 3, "NORMAL" ) );
		mVariables[ cInLitSpaceTangent ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iLitSpaceTangent", tHlslVariable::cInput, 4, "TANGENT" ) );
		mVariables[ cInVertexColor ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iVertexColor", tHlslVariable::cInput, 4, "COLOR" ) );

		mVariables[ cInUv0 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv0", tHlslVariable::cInput, 2, "TEXCOORD0" ) );
		mVariables[ cInUv1 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv1", tHlslVariable::cInput, 2, "TEXCOORD1" ) );
		mVariables[ cInUv2 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv2", tHlslVariable::cInput, 2, "TEXCOORD2" ) );
		mVariables[ cInUv3 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv3", tHlslVariable::cInput, 2, "TEXCOORD3" ) );

		mVariables[ cInUv01 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv01", tHlslVariable::cInput, 4, "TEXCOORD0" ) );
		mVariables[ cInUv23 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv23", tHlslVariable::cInput, 4, "TEXCOORD1" ) );

		mVariables[ cInWorldSpacePos ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iWorldSpacePos", tHlslVariable::cInput, 3, "TEXCOORD4" ) );
		mVariables[ cInProjSpaceDepth ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iProjSpaceDepth", tHlslVariable::cInput, 3, "TEXCOORD5" ) );
		mVariables[ cInLightPos ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iLightPos", tHlslVariable::cInput, 4, "TEXCOORD6" ) );
		mVariables[ cInLitSpacePos ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iLitSpacePos", tHlslVariable::cInput, 3, "TEXCOORD7" ) );

		// common temps...

		mVariables[ cTempEyeToVertexDir ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "tempEyeToVertexDir", tHlslVariable::cTemp, 3 ) );
		mVariables[ cTempEyeToVertexLen ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "tempEyeToVertexLen", tHlslVariable::cTemp, 1 ) );
		mVariables[ cTempEffectiveNormal ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "tempEffectiveNormal", tHlslVariable::cTemp, 4 ) );
		mVariables[ cTempEmissionResult ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "tempEmissionResult", tHlslVariable::cTemp, 4 ) );
		mVariables[ cTempAmbientResult ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "tempAmbientResult", tHlslVariable::cTemp, 4 ) );

		// ouputs...

		mVariables[ cOutColor0 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oColor0", tHlslVariable::cOutput, 4, "COLOR0" ) );
		mVariables[ cOutColor1 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oColor1", tHlslVariable::cOutput, 4, "COLOR1" ) );
		mVariables[ cOutColor2 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oColor2", tHlslVariable::cOutput, 4, "COLOR2" ) );
		mVariables[ cOutColor3 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oColor3", tHlslVariable::cOutput, 4, "COLOR3" ) );
		mVariables[ cOutColorDepth ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oDepth", tHlslVariable::cOutput, 4, "DEPTH" ) );
	}

	tPixelShaderVars::~tPixelShaderVars( )
	{
	}

}}

