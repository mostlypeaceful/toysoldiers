#include "ToolsPch.hpp"
#include "tVertexShaderVars.hpp"
#include "tPixelShaderVars.hpp"
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

	tVertexShaderVars::tVertexShaderVars( )
	{
		mVariables.fSetCount( cVarCount );

		// globals

		const u32 numSkinPaletteEntries = Gfx::tMaterial::cMaxBoneCount;

		u32 baseReg = 0;

		const u32 modelPosToWorldRegIndex = baseReg;
		fAddGlobal( mVariables[ cGlobalModelPosToWorld ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gModelPosToWorld", tHlslVariable::cGlobal, 4, 3, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueModelToWorld( modelPosToWorldRegIndex ) );
		const u32 modelNormalToWorldRegIndex = baseReg;
		fAddGlobal( mVariables[ cGlobalModelNormalToWorld ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gModelNormalToWorld", tHlslVariable::cGlobal, 3, 3, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueModelToWorld( modelPosToWorldRegIndex, modelNormalToWorldRegIndex ) );
		fAddGlobal( mVariables[ cGlobalModelToView ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gModelToView", tHlslVariable::cGlobal, 4, 3, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueModelToView( baseReg ) );
		fAddGlobal( mVariables[ cGlobalWorldToProjection ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gWorldToProjection", tHlslVariable::cGlobal, 4, 4, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueWorldToProjection( baseReg ) );
		fAddGlobal( mVariables[ cGlobalViewToProjection ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gViewToProjection", tHlslVariable::cGlobal, 4, 4, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueViewToProjection( baseReg ) );
		fAddGlobal( mVariables[ cGlobalViewToWorld ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gViewToWorld", tHlslVariable::cGlobal, 4, 3, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueViewToWorld( baseReg ) );
		fAddGlobal( mVariables[ cGlobalWorldEyePos ][ cPidDefault ], tHlslVariable::fMakeVectorFloat( "gWorldEyePos", tHlslVariable::cGlobal, 3, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueWorldEyePos( baseReg ) );
		fAddGlobal( mVariables[ cGlobalWorldToLight ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gWorldToLight", tHlslVariable::cGlobal, 4, 4, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueWorldToLight( baseReg ) );
		fAddGlobal( mVariables[ cGlobalViewToLight ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gViewToLight", tHlslVariable::cGlobal, 4, 4, tHlslVariable::tSemantic( "c", baseReg ) ), baseReg,
			new Gfx::tVsGlueViewToLight( baseReg ) );
		fAddGlobal( mVariables[ cGlobalSkinningPalette ][ cPidDefault ], tHlslVariable::fMakeMatrixFloat( "gSkinningPalette", tHlslVariable::cGlobal, 4, 3, tHlslVariable::tSemantic( "c", baseReg ), numSkinPaletteEntries ), baseReg,
			new Gfx::tVsGlueSkinningPalette( baseReg ) );



		// inputs...

		mVariables[ cInModelSpacePos ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iModelSpacePos", tHlslVariable::cInput, 3, "POSITION" ) );
		mVariables[ cInModelSpacePos ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticPosition, Gfx::tVertexElement::cFormat_f32_3 ) );

		mVariables[ cInModelSpaceNormal ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iModelSpaceNormal", tHlslVariable::cInput, 3, "NORMAL" ) );
		mVariables[ cInModelSpaceNormal ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticNormal, Gfx::tVertexElement::cFormat_f32_3 ) );

		mVariables[ cInModelSpaceNormal ][ cPidXbox360 ].fReset( tHlslVariable::fMakeVectorFloat( "iModelSpaceNormal", tHlslVariable::cInput, 4, "NORMAL" ) );
		mVariables[ cInModelSpaceNormal ][ cPidXbox360 ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticNormal, Gfx::tVertexElement::cFormat_f16_4 ) );

		mVariables[ cInModelSpaceTangent ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iModelSpaceTangent", tHlslVariable::cInput, 4, "TANGENT" ) );
		mVariables[ cInModelSpaceTangent ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTangent, Gfx::tVertexElement::cFormat_f32_4 ) );

		mVariables[ cInModelSpaceTangent ][ cPidXbox360 ].fReset( tHlslVariable::fMakeVectorFloat( "iModelSpaceTangent", tHlslVariable::cInput, 4, "TANGENT" ) );
		mVariables[ cInModelSpaceTangent ][ cPidXbox360 ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTangent, Gfx::tVertexElement::cFormat_f16_4 ) );

		mVariables[ cInModelSpaceTangent3 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iModelSpaceTangent3", tHlslVariable::cInput, 3, "TANGENT" ) );
		mVariables[ cInModelSpaceTangent3 ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTangent, Gfx::tVertexElement::cFormat_f32_3 ) );

		mVariables[ cInVertexColor ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iVertexColor", tHlslVariable::cInput, 4, "COLOR0" ) );
		mVariables[ cInVertexColor ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticColor, Gfx::tVertexElement::cFormat_u8_4_Color ) );

		mVariables[ cInUv0 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv0", tHlslVariable::cInput, 2, "TEXCOORD0" ) );
		mVariables[ cInUv0 ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f32_2, 0 ) );
		mVariables[ cInUv1 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv1", tHlslVariable::cInput, 2, "TEXCOORD1" ) );
		mVariables[ cInUv1 ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f32_2, 1 ) );
		mVariables[ cInUv2 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv2", tHlslVariable::cInput, 2, "TEXCOORD2" ) );
		mVariables[ cInUv2 ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f32_2, 2 ) );
		mVariables[ cInUv3 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv3", tHlslVariable::cInput, 2, "TEXCOORD3" ) );
		mVariables[ cInUv3 ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f32_2, 3 ) );

		mVariables[ cInUv0 ][ cPidXbox360 ].fReset( tHlslVariable::fMakeVectorFloat( "iUv0", tHlslVariable::cInput, 2, "TEXCOORD0" ) );
		mVariables[ cInUv0 ][ cPidXbox360 ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f16_2, 0 ) );
		mVariables[ cInUv1 ][ cPidXbox360 ].fReset( tHlslVariable::fMakeVectorFloat( "iUv1", tHlslVariable::cInput, 2, "TEXCOORD1" ) );
		mVariables[ cInUv1 ][ cPidXbox360 ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f16_2, 1 ) );
		mVariables[ cInUv2 ][ cPidXbox360 ].fReset( tHlslVariable::fMakeVectorFloat( "iUv2", tHlslVariable::cInput, 2, "TEXCOORD2" ) );
		mVariables[ cInUv2 ][ cPidXbox360 ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f16_2, 2 ) );
		mVariables[ cInUv3 ][ cPidXbox360 ].fReset( tHlslVariable::fMakeVectorFloat( "iUv3", tHlslVariable::cInput, 2, "TEXCOORD3" ) );
		mVariables[ cInUv3 ][ cPidXbox360 ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f16_2, 3 ) );

		mVariables[ cInUv01 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv01", tHlslVariable::cInput, 4, "TEXCOORD0" ) );
		mVariables[ cInUv01 ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f32_4, 0 ) );
		mVariables[ cInUv23 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iUv23", tHlslVariable::cInput, 4, "TEXCOORD1" ) );
		mVariables[ cInUv23 ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f32_4, 1 ) );

		mVariables[ cInUv01 ][ cPidXbox360 ].fReset( tHlslVariable::fMakeVectorFloat( "iUv01", tHlslVariable::cInput, 4, "TEXCOORD0" ) );
		mVariables[ cInUv01 ][ cPidXbox360 ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f16_4, 0 ) );
		mVariables[ cInUv23 ][ cPidXbox360 ].fReset( tHlslVariable::fMakeVectorFloat( "iUv23", tHlslVariable::cInput, 4, "TEXCOORD1" ) );
		mVariables[ cInUv23 ][ cPidXbox360 ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticTexCoord, Gfx::tVertexElement::cFormat_f16_4, 1 ) );

		mVariables[ cInBlendWeights ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "iBlendWeights", tHlslVariable::cInput, 4, "BLENDWEIGHT" ) );
		mVariables[ cInBlendWeights ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticBoneWeights, Gfx::tVertexElement::cFormat_u8_4_Normalized ) );

		mVariables[ cInBlendIndices ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorInt( "iBlendIndices", tHlslVariable::cInput, 4, "BLENDINDICES" ) );
		mVariables[ cInBlendIndices ][ cPidDefault ]->fSetVertexElement( Gfx::tVertexElement( Gfx::tVertexElement::cSemanticBoneIndices, Gfx::tVertexElement::cFormat_u8_4 ) );



		// outputs...

		mVariables[ cOutProjSpacePos ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oProjSpacePos", tHlslVariable::cOutput, 4, "POSITION" ) );
		mVariables[ cOutProjSpaceDepth ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oProjSpaceDepth", tHlslVariable::cOutput, 2, tPixelShaderVars::fVariable( tPixelShaderVars::cInProjSpaceDepth )->fSemantic( ) ) );
		mVariables[ cOutLitSpacePos ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oLitSpacePos", tHlslVariable::cOutput, 3, tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpacePos )->fSemantic( ) ) );
		mVariables[ cOutLitSpaceNormal ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oLitSpaceNormal", tHlslVariable::cOutput, 3, tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceNormal )->fSemantic( ) ) );
		mVariables[ cOutLitSpaceTangent ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oLitSpaceTangent", tHlslVariable::cOutput, 4, tPixelShaderVars::fVariable( tPixelShaderVars::cInLitSpaceTangent )->fSemantic( ) ) );
		mVariables[ cOutVertexColor ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oVertexColor", tHlslVariable::cOutput, 4, "COLOR" ) );
		mVariables[ cOutUv0 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oUv0", tHlslVariable::cOutput, 2, tPixelShaderVars::fVariable( tPixelShaderVars::cInUv0 )->fSemantic( ) ) );
		mVariables[ cOutUv1 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oUv1", tHlslVariable::cOutput, 2, tPixelShaderVars::fVariable( tPixelShaderVars::cInUv1 )->fSemantic( ) ) );
		mVariables[ cOutUv2 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oUv2", tHlslVariable::cOutput, 2, tPixelShaderVars::fVariable( tPixelShaderVars::cInUv2 )->fSemantic( ) ) );
		mVariables[ cOutUv3 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oUv3", tHlslVariable::cOutput, 2, tPixelShaderVars::fVariable( tPixelShaderVars::cInUv3 )->fSemantic( ) ) );
		mVariables[ cOutUv01 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oUv01", tHlslVariable::cOutput, 4, tPixelShaderVars::fVariable( tPixelShaderVars::cInUv01 )->fSemantic( ) ) );
		mVariables[ cOutUv23 ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oUv23", tHlslVariable::cOutput, 4, tPixelShaderVars::fVariable( tPixelShaderVars::cInUv23 )->fSemantic( ) ) );
		mVariables[ cOutLightPos ][ cPidDefault ].fReset( tHlslVariable::fMakeVectorFloat( "oLightPos", tHlslVariable::cOutput, 4, tPixelShaderVars::fVariable( tPixelShaderVars::cInLightPos )->fSemantic( ) ) );
	}

	tVertexShaderVars::~tVertexShaderVars( )
	{
	}



}}

