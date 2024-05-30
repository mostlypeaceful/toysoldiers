#ifndef __tMaterialGenBase__
#define __tMaterialGenBase__
#include "iAssetPlugin.hpp"
#include "iMaterialGenPlugin.hpp"
#include "Sigml.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tLight.hpp"

namespace Sig
{
	class tFileWriter;

	///
	/// \brief Base class for stand-alone material generator plugins. Automates a few
	/// of the more annoying, common tasks; this class is purely utility, you could
	/// accomplish all of this by deriving from iAssetPlugin and iMaterialGenPlugin.
	class tools_export tMaterialGenBase :
		public Sigml::tMaterial,
		public iAssetPlugin,
		public iMaterialGenPlugin
	{
	public:
		typedef tDynamicArray< tDynamicArray< tDynamicBuffer > > tShaderBufferSet;
	public:

	public:
		tMaterialGenBase( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual b32	 fIsFacing( ) const;

		static void tMaterialGenBase::fAddVertexShader( 
			tPlatformId pid,
			u32 shaderListIndex,
			u32 shaderIndex,
			Gfx::tMaterialFile& mtlFile, 
			tShaderBufferSet& shaderBuffers,
			const std::string& hlsl );

		static void tMaterialGenBase::fAddPixelShader( 
			tPlatformId pid,
			u32 shaderListIndex,
			u32 shaderIndex,
			Gfx::tMaterialFile& mtlFile, 
			tShaderBufferSet& shaderBuffers,
			const std::string& hlsl );

		static void tMaterialGenBase::fAddPcDx9VertexShader( 
			u32 shaderListIndex,
			u32 shaderIndex,
			Gfx::tMaterialFile& mtlFile, 
			tShaderBufferSet& shaderBuffers,
			const std::string& hlsl );

		static void tMaterialGenBase::fAddPcDx9PixelShader( 
			u32 shaderListIndex,
			u32 shaderIndex,
			Gfx::tMaterialFile& mtlFile, 
			tShaderBufferSet& shaderBuffers,
			const std::string& hlsl );

		static void tMaterialGenBase::fAddXbox360VertexShader( 
			u32 shaderListIndex,
			u32 shaderIndex,
			Gfx::tMaterialFile& mtlFile, 
			tShaderBufferSet& shaderBuffers,
			const std::string& hlsl );

		static void tMaterialGenBase::fAddXbox360PixelShader( 
			u32 shaderListIndex,
			u32 shaderIndex,
			Gfx::tMaterialFile& mtlFile, 
			tShaderBufferSet& shaderBuffers,
			const std::string& hlsl );

		static u32 fShadowMapLayerCount( tPlatformId pid );
		static std::string fShadowMapSamplerName( tPlatformId pid );
		static b32 fSupports3DTextures( tPlatformId pid );

		///
		/// \brief Declare world to light constants array
		static void fDeclareWorldToLightConstantsArray(
			tPlatformId pid,
			std::stringstream& ss, 
			u32 startRegister,
			const std::string& arrayName = "gWorldToLightArray",
			u32 maxSlots = Gfx::tMaterial::cMaxShadowLayers );

		///
		/// \brief Declare light constants array
		static void fDeclareLightConstantsArray(
			std::stringstream& ss, 
			u32 startRegister,
			const std::string& lightArrayName = "gLights",
			u32 maxLights = Gfx::tMaterial::cMaxLights );

		///
		/// \brief Linear fog.
		static void fCallFog( 
			std::stringstream& ss, 
			const std::string& pixelColorName,
			const std::string& positionName,
			const std::string& fogValuesName,
			const std::string& fogColorName,
			const std::string& resultColorName );

		/// \brief N.B. Meant to live on the stack, not for long term storage!
		/// You should be using the defaults for the most part where possible.
		/// It's called consistency mothertruckers!  XOXOXO --mrickert
		struct tComputeShadowParameters
		{
			tPlatformId			mPid;						///< REQUIRED
			std::stringstream*	mSs;						///< REQUIRED
			std::string			mShadowMapSamplerName;		///< Default: [In]  gShadowMap									An empty string will disable shadow maps sampling but still do e.g. normal based calcs.
			std::string			mWorldPosName;				///< Default: [In]  iLightPos
			std::string			mShadowEpsilonName;			///< Default: [In]  gShadowMapEpsilon
			std::string			mShadowMapTexelSizeName;	///< Default: [In]  gShadowMapEpsilon_TexelSize_Amount_Split.y
			std::string			mShadowAmountName;			///< Default: [In]  gShadowMapEpsilon_TexelSize_Amount_Split.z
			std::string			mShadowTermName;			///< Default: [Out] shadowTerm
			std::string			mShadowMapTargetName;		///< Default: [In]  gShadowMapTarget_Split.xyz
			std::string			mShadowSplitDistance0Name;	///< Default: [In]  gShadowMapTarget_Split.w
			std::string			mShadowSplitDistance1Name;	///< Default: [In]  gShadowMapEpsilon_TexelSize_Amount_Split.w
			std::string			mWorldToLightArrayName;		///< Default: [In]  gWorldToLightArray
			b32					mForceLightSpaceConversion;	///< Default: [In]  false
			std::string			mNDotL;						///< [In]  HIGHLY RECOMMENDED
			std::string			mDebugShadowTermName;		///< [Out] HIGHLY RECOMMENDED (in debug shaders only though!)

			tComputeShadowParameters()
				: mPid						( cPlatformNone )
				, mSs						( NULL )
				, mShadowMapSamplerName		( "gShadowMap" )
				, mWorldPosName				( "iLightPos" )
				, mShadowEpsilonName		( "gShadowMapEpsilon" )
				, mShadowMapTexelSizeName	( "gShadowMapEpsilon_TexelSize_Amount_Split.y" )
				, mShadowAmountName			( "gShadowMapEpsilon_TexelSize_Amount_Split.z" )
				, mShadowTermName			( "shadowTerm" )
				, mShadowMapTargetName		( "gShadowMapTarget_Split.xyz" )
				, mShadowSplitDistance0Name	( "gShadowMapTarget_Split.w" )
				, mShadowSplitDistance1Name	( "gShadowMapEpsilon_TexelSize_Amount_Split.w" )
				, mWorldToLightArrayName	( "gWorldToLightArray" )
				, mForceLightSpaceConversion( false )
				, mNDotL					( "" )
				, mDebugShadowTermName		( "" )
			{
			}
		};

		///
		/// \brief Compute shadow map term [0,1]
		static void fCallComputeShadowTerm( const tComputeShadowParameters& parameters );

		static void fCallLightAttenuate(
			std::stringstream&		ss,
			Gfx::tLight::tLightType	lightType,
			const char*				resultName,
			const char*				lightName,
			const char*				lightDistanceName );

		///
		/// \brief Lighting happens in eye-relative world space; this means all positions should have
		/// the camera's eye subtracted off, but should otherwise remain oriented in world-space.
		static void fCallAccumulateLight( 
			std::stringstream&			ss, 
			b32							calcSpec,
			Gfx::tLight::tLightType		lightType,
			const std::string&			lightName			= "light",
			const std::string&			positionName		= "p",
			const std::string&			normalName			= "n",
			const std::string&			specSizeName		= "ss",
			const std::string&			diffAccumName		= "diffAccum",
			const std::string&			specAccumName		= "specAccum",
			const std::string&			specMagAccumName	= "specMagAccum",
			const std::string&			shadowTermName		= "shadowTerm",
			b32							calcAmbient			= false,
			const std::string&			ambientWeightsName	= "ambientWeights" );

		///
		/// \brief Does an ambient look up in spherical harmonics.
		static void fSphericalLookup( std::stringstream& ss, const std::string& normal, const std::string& output, const std::string& weightsName );

		///
		/// \brief Utility method for creating an output file writer based on the
		/// derived material generator's fGetMaterialName( ) method and the current platform.
		static void fCreateOutputFile( const tFilePathPtr& resourcePath, tFileWriter& ofile, tPlatformId pid );

		///
		/// \brief Utility method for outputting a material file. Uses the fCreateOutputFile method
		/// above for generating the output file, then serializes the supplied tMaterialFile.
		static void fSaveMaterialFile( 
			const tFilePathPtr& outputPath,
			Gfx::tMaterialFile& mtlFile, 
			const tShaderBufferSet& shaderBuffers, 
			tPlatformId pid );

	protected:

		static const char gMaterialRootFolder[];

		///
		/// \brief Utility method for outputting a material file. Uses the fCreateOutputFile method
		/// above for generating the output file, then serializes the supplied tMaterialFile.
		void fSaveMaterialFile( 
			Gfx::tMaterialFile& mtlFile, 
			const tShaderBufferSet& shaderBuffers, 
			tPlatformId pid );

		///
		/// \brief Converts common material attributes.
		void fConvertBase( Gfx::tMaterial* mtl );
	
		///
		///  \brief Compare base attributes.
		b32 fIsEquivalent( const Sigml::tMaterial& other ) const;

		virtual iMaterialGenPlugin* fGetMaterialGenPluginInterface( ) { return this; }
		virtual void fGenerateMaterialFile( const tFilePathPtr& exePath, b32 force );
		virtual void fGenerateMaterialFileWii( tPlatformId pid ) = 0;
		virtual void fGenerateMaterialFilePcDx9( tPlatformId pid ) = 0;
		virtual void fGenerateMaterialFilePcDx10( tPlatformId pid ) = 0;
		virtual void fGenerateMaterialFileXbox360( tPlatformId pid ) = 0;
		virtual void fGenerateMaterialFilePs3Ppu( tPlatformId pid ) = 0;
	};
}

#endif//__tMaterialGenBase__
