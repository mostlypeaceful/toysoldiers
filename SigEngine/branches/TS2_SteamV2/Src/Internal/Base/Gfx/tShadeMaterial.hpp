#ifndef __tShadeMaterial__
#define __tShadeMaterial__
#include "tMaterial.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	class tShadeMaterial;
	class tShadeMaterialGlueValues;

	class base_export tShadeMaterialGlueValues
	{
		declare_reflector( );
	public:
		enum tTextureSource { cTexSourceFromFile, cTexSourceWhite, cTexSourceBlack, cTexSourceNoise, cTexSourceCount };
		struct tTextureGlue
		{
			declare_reflector( );
		public:
			tEnum<tTextureSource,u32>	mTexSrc;
			tTextureReference			mTexRef;
			inline tTextureGlue( ) : mTexSrc( cTexSourceFromFile ) { }
			inline tTextureGlue( const tTextureReference& tr ) : mTexSrc( cTexSourceFromFile ), mTexRef( tr ) { }
		};
		typedef tLoadInPlacePtrWrapper< tLoadInPlaceStringPtr > tStringGlue;
	private:
		tDynamicArray< tTextureGlue >	mSamplers;
		tDynamicArray< Math::tVec4f >	mVectors;
		tDynamicArray< tStringGlue >	mStrings;
		f32								mBackFaceFlip;
	public:
		tShadeMaterialGlueValues( );
		tShadeMaterialGlueValues( tNoOpTag );
		u32							fAddSampler( const tTextureReference& tr = tTextureReference( ) );
		u32							fAddVector( const Math::tVec4f& v = Math::tVec4f::cOnesVector );
		u32							fAddString( );
		b32							fUpdateSampler( const tTextureReference& tr, u32 idx );
		b32							fUpdateSampler( tTextureSource texSrc, u32 idx );
		b32							fUpdateVector( const Math::tVec4f& v, u32 idx );
		b32							fUpdateString( const char* s, tLoadInPlaceFileBase& lipFileCreator, u32 idx );
		const tTextureReference&	fFindSampler( u32 idx, const tRenderContext& context ) const;
		const Math::tVec4f*			fFindVector( u32 idx ) const;
		const tStringPtr&			fFindString( u32 idx ) const;
		f32							fBackFaceFlip( ) const { return mBackFaceFlip; }
		void						fSetBackFaceFlip( f32 bff ) { mBackFaceFlip = bff; }
	};

	class base_export tShadeMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tShadeMaterial, 0xC4B4BE0E );
		define_dynamic_cast(tShadeMaterial, tMaterial);
	public:
		enum tShaderSlot
		{
			cShaderSlotStaticVs,
			cShaderSlotSkinnedVs,
			cShaderSlotColorPs,
			cShaderSlotDepthPs,
			cShaderSlotDepthAlphaPs,
			cShaderSlotCount
		};
		enum tPassType
		{
			cPassTypeColor,
			cPassTypeDepth,

			// last
			cPassTypeCount
		};
		struct base_export tPass
		{
			declare_reflector( );
		public:
			static const u8 cInvalidShaderIndex = 0xff;
		public:
			Gfx::tVertexFormat mVertexFormat;
			u8 mVsType;
			u8 mVsIndex;
			u8 mPsType;
			u8 mPsBaseIndex;
			u8 mPsMaxLights;
			u8 pad0, pad1, pad2;
		public:
			tPass( );
			tPass( tNoOpTag );
			void fApplyShared( const tShadeMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const;
			void fApplyInstance( const tShadeMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
		};
	public:
		tPass								mColorPass;
		tPass								mDepthPass;
		tShadeMaterialGlueValues			mGlueValues;
	public:
		tShadeMaterial( );
		tShadeMaterial( tNoOpTag );
		tPass&			fPass( u32 ithPass )		{ return (&mColorPass)[ ithPass ]; }
		const tPass&	fPass( u32 ithPass ) const	{ return (&mColorPass)[ ithPass ]; }
		virtual const tVertexFormat& fVertexFormat( ) const;
		virtual b32 fIsLit( ) const;
		virtual b32 fRendersDepth( ) const;
		virtual b32 fSupportsInstancing( ) const;
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
	};

}}


#endif//__tShadeMaterial__
