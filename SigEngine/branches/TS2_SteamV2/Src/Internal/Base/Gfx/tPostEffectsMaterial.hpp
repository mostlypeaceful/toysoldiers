#ifndef __tPostEffectsMaterial__
#define __tPostEffectsMaterial__
#include "tMaterial.hpp"
#include "tRenderToTexture.hpp"

namespace Sig
{
	class tRandom;
}

namespace Sig { namespace Gfx
{
	struct base_export tPostEffectsRenderVertex
	{
		Math::tVec2f	mP;
		inline tPostEffectsRenderVertex( ) { }
		inline tPostEffectsRenderVertex( const Math::tVec2f& p )
			: mP( p ) { }
	};

	class base_export tPostEffectsMaterial : public tMaterial
	{
		define_dynamic_cast(tPostEffectsMaterial, tMaterial);
	public:

		static const tVertexFormat cVertexFormat;
		static tFilePathPtr fMaterialFilePath( );

		enum tVShaders
		{
			cVShaderBasic,
			cVShaderOutputUv,

			// last
			cVShaderCount
		};

		enum tPShaders
		{
			cPShaderCopy,
			cPShaderAdd,
			cPShaderSample2x2,
			cPShaderGaussBlurH,
			cPShaderGaussBlurV,
			cPShaderBlend,
			cPShaderBlendUsingSource1Alpha,
			cPShaderBlendUsingDepth,
			cPShaderSaturation,
			cPShaderFilmGrain,
			cPShaderTransform,
			cPShaderBlendUsingDepthAndOverlay,

			// last
			cPShaderCount
		};

		enum tVSConstants
		{
			cVSWorldToProj		= 0,
			cVSRenderTargetDims	= 4,
			cVSViewportXform	= 5,
		};

		enum tPSConstants
		{
			cPSSampleOffsets = 16,
			cPSRgbaTint0 = 0,
			cPSRgbaTint1 = 1,
			cPSRgbaBlend = 2,
			cPSTargetDepthValues = 3,
			cPSSaturation = 4,
			cPSFilmGrainFlicker = 5,
			cPSFilmGrainOffsets0 = 6,
			cPSFilmGrainOffsets1 = 7,
			cPSContrast = 8,
			cPSTransformAdd = 9,
			cPSTransformMul = 10,
		};

		struct tInput
		{
			const tTextureReference*	mTexture;
			u32							mWidth, mHeight;
			tInput( ) : mTexture( 0 ), mWidth( 1 ), mHeight( 1 ) { }
			tInput( const tTextureReference* tex, u32 width, u32 height ) : mTexture( tex ), mWidth( width ), mHeight( height ) { }
		};

		struct tParameters : public tRefCounter
		{
			tStringPtr mTextureKey;

			Math::tVec3f mExposure;
			Math::tVec3f mSaturation;

			f32				mGrainFreq;
			f32				mGrainScale;
			Math::tVec2f	mGrainSpeed;
			f32				mHairsFreq;
			f32				mHairsScale;
			Math::tVec2f	mHairsSpeed;
			f32				mLinesFreq;
			f32				mLinesScale;
			Math::tVec2f	mLinesSpeed;
			f32				mSmudgeFreq;
			f32				mSmudgeScale;
			Math::tVec2f	mSmudgeSpeed;

			const Math::tVec3f& fExposure( ) const;
			const Math::tVec3f& fSaturation( ) const;

			f32					fGrainFreq( ) const;
			f32					fGrainScale( ) const;
			const Math::tVec2f&	fGrainSpeed( ) const;
			f32					fHairsFreq( ) const;
			f32					fHairsScale( ) const;
			const Math::tVec2f&	fHairsSpeed( ) const;
			f32					fLinesFreq( ) const;
			f32					fLinesScale( ) const;
			const Math::tVec2f&	fLinesSpeed( ) const;
			f32					fSmudgeFreq( ) const;
			f32					fSmudgeScale( ) const;
			const Math::tVec2f&	fSmudgeSpeed( ) const;

			tParameters( );
		};
		typedef tRefCounterPtr<tParameters> tParametersPtr;

	public:

		tEnum<tVShaders,u16> mVS;
		tEnum<tPShaders,u16> mPS;

		tFixedArray<tInput,8>	mInputs;
		u32						mInputCount;

		Math::tVec4f			mRgbaTint0;
		Math::tVec4f			mRgbaTint1;
		Math::tVec4f			mRgbaBlend;
		Math::tVec4f			mTargetDepthValues;
		Math::tVec4f			mSaturation;
		Math::tVec4f			mFilmGrainFlicker;
		Math::tVec4f			mFilmGrainOffsets0;
		Math::tVec4f			mFilmGrainOffsets1;
		Math::tVec4f			mContrast;
		Math::tVec4f			mTransformAdd;
		Math::tVec4f			mTransformMul;

		tParameters				mParameters;

	public:

		tPostEffectsMaterial( );
		explicit tPostEffectsMaterial( const tResourcePtr& postEffectsMtlFile );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
		void fClearInputs( );
		void fSetInputCount( u32 count );
		void fAddInput( const tRenderToTexturePtr& rtt );
		void fAddInput( const tTextureReference* texture, u32 width, u32 height );
		void fStepFilmGrain( f32 dt, tRandom& rand, f32 scale = 1.f );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tPostEffectsMaterial );

}}


#endif//__tPostEffectsMaterial__
