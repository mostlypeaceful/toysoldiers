#ifndef __tPhongShadeNode__
#define __tPhongShadeNode__
#include "tShadeNode.hpp"
#include "HlslGen/tHlslGenTree.hpp"

namespace Sig
{
	class tools_export tPhongShadeNode : public tShadeNode
	{
	private:
		enum tChannel
		{
			cChannelDiffuse,
			cChannelSpecColor,
			cChannelSpecFalloff,
			cChannelNormal,
			cChannelEmissive,
			cChannelTransmission,
			cChannelAmbient, // ambient occlusion
			cChannelOpacity,
			cChannelRim,
			cChannelCount
		};
	public:
		implement_derived_shade_node( tPhongShadeNode, 0xD1238904 ) { }
		explicit tPhongShadeNode( const wxPoint& p = wxPoint( 0, 0 ) );
		virtual b32 fInputNeedsWritingToHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tHlslGenTree& hlslGenTree, u32 ithInput );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree );
	private:
		HlslGen::tHlslVariableConstPtr fWriteShadowTerm( HlslGen::tHlslWriter& writer );
		HlslGen::tHlslVariableConstPtr fWriteLightLoop( 
			HlslGen::tHlslWriter& writer, 
			const HlslGen::tShaderRequirements& reqs, 
			const HlslGen::tHlslGenTree::tInputResultArray& inputResults,
			const HlslGen::tHlslVariableConstPtr& effectiveNormal,
			const HlslGen::tHlslVariableConstPtr& shadowTerm );
		void fWriteLightLoopInner( 
			HlslGen::tHlslWriter& writer, 
			const HlslGen::tHlslVariableConstPtr& effectiveNormal,
			const HlslGen::tHlslVariableConstPtr& shadowTerm,
			const HlslGen::tHlslVariableConstPtr& ambientResult,
			const HlslGen::tHlslVariableConstPtr& diffAccum,
			const HlslGen::tHlslVariableConstPtr& transAccum,
			const HlslGen::tHlslVariableConstPtr& specAccum,
			const HlslGen::tHlslVariableConstPtr& specMagAccum,
			const HlslGen::tHlslVariableConstPtr& rimAccum,
			const std::string& specFalloffResultText,
			u32 numLights,
			u32 startLight,
			b32 pointLights );
	};

}

#endif//__tPhongShadeNode__
