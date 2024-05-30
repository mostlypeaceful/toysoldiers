#ifndef __DerivedShadeNodes__
#define __DerivedShadeNodes__
#include "tShadeNode.hpp"

namespace Sig
{


	class tools_export tDefaultOutputShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tDefaultOutputShadeNode, 0xA84C2B99 ) { }
		explicit tDefaultOutputShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual tOutputSemantic fOutputSemantic( ) const { return cOutputColor0; }
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};


	class tools_export tFlatParticleShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tFlatParticleShadeNode, 0x75478FAC ) { }
		explicit tFlatParticleShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};



	class tools_export tUvChannelShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tUvChannelShadeNode, 0x96E6EA8A ) { }
		explicit tUvChannelShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		u32 fUvChannel( u32 maxChannel );
	};

	class tools_export tColorMapShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tColorMapShadeNode, 0x4C5BA163 ) { }
		explicit tColorMapShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};
	class tools_export tNormalMapShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tNormalMapShadeNode, 0x48B874DB ) { }
		explicit tNormalMapShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};

	class tools_export tColorMapAtlasShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tColorMapAtlasShadeNode, 0xD57EED90 ) { }
		explicit tColorMapAtlasShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( 
			Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen(
			Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};

	//class tools_export tNormalMapAtlasShadeNode : public tShadeNode
	//{
	//public:
	//	implement_derived_shade_node( tNormalMapAtlasShadeNode, 0x72AE2E50 ) { }
	//	explicit tNormalMapAtlasShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
	//	virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
	//	virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	//	virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	//	virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
	//	virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	//};

	class tools_export tCubeMapShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tCubeMapShadeNode, 0x3CE105AE ) { }
		explicit tCubeMapShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};




	class tools_export tAddShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tAddShadeNode, 0x88213EA8 ) { }
		explicit tAddShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tSubtractShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tSubtractShadeNode, 0x2CAF2AA4 ) { }
		explicit tSubtractShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tMultiplyShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tMultiplyShadeNode, 0x94B20AFF ) { }
		explicit tMultiplyShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tDivideShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tDivideShadeNode, 0xEB89DCE1 ) { }
		explicit tDivideShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tPowShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tPowShadeNode, 0x3CE5C954 ) { }
		explicit tPowShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tOneMinusShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tOneMinusShadeNode, 0x61DAA45C ) { }
		explicit tOneMinusShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tBlendShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tBlendShadeNode, 0x9177A5C8 ) { }
		explicit tBlendShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tDotShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tDotShadeNode, 0xBF19091F ) { }
		explicit tDotShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};




	class tools_export tColorRGBShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tColorRGBShadeNode, 0x66B65CDE ) { }
		explicit tColorRGBShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};
	class tools_export tColorRGBAShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tColorRGBAShadeNode, 0x9E8E7A4B ) { }
		explicit tColorRGBAShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};
	class tools_export tVertexColorShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tVertexColorShadeNode, 0x2AF8042B ) { }
		explicit tVertexColorShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};





	class tools_export tNumberShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tNumberShadeNode, 0x5F55EA59 ) { }
		explicit tNumberShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};
	class tools_export tVector2ShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tVector2ShadeNode, 0x1DD7AFF1 ) { }
		explicit tVector2ShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};
	class tools_export tVector3ShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tVector3ShadeNode, 0xFBA3A5BA ) { }
		explicit tVector3ShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};
	class tools_export tVector4ShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tVector4ShadeNode, 0xFE537E9F ) { }
		explicit tVector4ShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};


	class tools_export tDynamicVec4ShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tDynamicVec4ShadeNode, 0xFF604271 ) { }
		explicit tDynamicVec4ShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache );
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut );
	};



	class tools_export tGeometryNormalShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tGeometryNormalShadeNode, 0xF82A2743 ) { }
		explicit tGeometryNormalShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tPerPixelNormalShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tPerPixelNormalShadeNode, 0x68682FFE ) { }
		explicit tPerPixelNormalShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tReflectionVectorShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tReflectionVectorShadeNode, 0xEBF7CF29 ) { }
		explicit tReflectionVectorShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tWorldSpacePosShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tWorldSpacePosShadeNode, 0xC834A8F9 ) { }
		explicit tWorldSpacePosShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tEdgeShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tEdgeShadeNode, 0xB93376E5 ) { }
		explicit tEdgeShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};




	class tools_export tSwizzleShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tSwizzleShadeNode, 0x8F444895 ) { }
		explicit tSwizzleShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	private:
		u32 fChannelCount( );
		std::string fSwizzleText( const HlslGen::tHlslVariableConstPtr& input );
	};
	class tools_export tSaturateShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tSaturateShadeNode, 0x87539A4A ) { }
		explicit tSaturateShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tClampShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tClampShadeNode, 0x22BD0775 ) { }
		explicit tClampShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tThresholdShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tThresholdShadeNode, 0x6223E73F ) { }
		explicit tThresholdShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tPulseShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tPulseShadeNode, 0xA328C30C ) { }
		explicit tPulseShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tScrollShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tScrollShadeNode, 0x21078A46 ) { }
		explicit tScrollShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tFracShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tFracShadeNode, 0x48EF41F3 ) { }
		explicit tFracShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
	class tools_export tTruncShadeNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tTruncShadeNode, 0x2ADEC1A5 ) { }
		explicit tTruncShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};


	class tools_export tTransitionObjectsNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tTransitionObjectsNode, 0xC8A79929 ) { }

		explicit tTransitionObjectsNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};

	class tools_export tSplitPlaneNode : public tShadeNode
	{
	public:
		implement_derived_shade_node( tSplitPlaneNode, 0xD3D119F7 ) { }

		explicit tSplitPlaneNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	};
}


#endif//__DerivedShadeNodes__
