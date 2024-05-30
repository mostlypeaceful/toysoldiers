#ifndef __DerivedShadeMaterialGlue__
#define __DerivedShadeMaterialGlue__
#include "tMaterialFile.hpp"

namespace Sig { namespace Gfx
{

	class base_export tVsGlueModelToWorld : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueModelToWorld, 0x3602757A );
	private:
		u16 mPRegister, mNRegister;
	public:
		explicit tVsGlueModelToWorld( u32 pReg = 0, u32 nReg = ~0 ) : mPRegister( pReg ), mNRegister( nReg == ~0 ? pReg : nReg ) { }
		tVsGlueModelToWorld( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueModelToWorld( mPRegister, mNRegister ); }
		virtual b32 fIsShared( ) const { return false; }
		virtual void fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
	};
	class base_export tVsGlueModelToView : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueModelToView, 0x7A188D18 );
	private:
		u32 mRegister;
	public:
		explicit tVsGlueModelToView( u32 reg = 0 ) : mRegister( reg ) { }
		tVsGlueModelToView( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueModelToView( mRegister ); }
		virtual b32 fIsShared( ) const { return false; }
		virtual void fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
	};
	class base_export tVsGlueWorldToProjection : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueWorldToProjection, 0x27771CBF );
	private:
		u32 mRegister;
	public:
		explicit tVsGlueWorldToProjection( u32 reg = 0 ) : mRegister( reg ) { }
		tVsGlueWorldToProjection( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueWorldToProjection( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tVsGlueViewToProjection : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueViewToProjection, 0x161AF1AC );
	private:
		u32 mRegister;
	public:
		explicit tVsGlueViewToProjection( u32 reg = 0 ) : mRegister( reg ) { }
		tVsGlueViewToProjection( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueViewToProjection( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tVsGlueViewToWorld : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueViewToWorld, 0x5A5D5C43 );
	private:
		u32 mRegister;
	public:
		explicit tVsGlueViewToWorld( u32 reg = 0 ) : mRegister( reg ) { }
		tVsGlueViewToWorld( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueViewToWorld( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tVsGlueWorldEyePos : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueWorldEyePos, 0xC1E9D427 );
	private:
		u32 mRegister;
	public:
		explicit tVsGlueWorldEyePos( u32 reg = 0 ) : mRegister( reg ) { }
		tVsGlueWorldEyePos( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueWorldEyePos( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tVsGlueWorldToLight : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueWorldToLight, 0xF173664C );
	private:
		u32 mRegister;
	public:
		explicit tVsGlueWorldToLight( u32 reg = 0 ) : mRegister( reg ) { }
		tVsGlueWorldToLight( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueWorldToLight( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tVsGlueViewToLight : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueViewToLight, 0x63B6FFF6 );
	private:
		u32 mRegister;
	public:
		explicit tVsGlueViewToLight( u32 reg = 0 ) : mRegister( reg ) { }
		tVsGlueViewToLight( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueViewToLight( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tVsGlueSkinningPalette : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tVsGlueSkinningPalette, 0x584DC626 );
	private:
		u32 mRegister;
	public:
		explicit tVsGlueSkinningPalette( u32 reg = 0 ) : mRegister( reg ) { }
		tVsGlueSkinningPalette( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tVsGlueSkinningPalette( mRegister ); }
		virtual b32 fIsShared( ) const { return false; }
		virtual void fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
	};

	class base_export tPsGlueBackFacePlusShadowMapParams : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueBackFacePlusShadowMapParams, 0x62E7F29A );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueBackFacePlusShadowMapParams( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueBackFacePlusShadowMapParams( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueBackFacePlusShadowMapParams( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueShadowMapTargetPos : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueShadowMapTargetPos, 0xC66850B3 );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueShadowMapTargetPos( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueShadowMapTargetPos( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueShadowMapTargetPos( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};	
	class base_export tPsGlueShadowMapSplits : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueShadowMapSplits, 0x705AF453 );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueShadowMapSplits( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueShadowMapSplits( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueShadowMapSplits( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};	
	class base_export tPsGlueWorldToLightSpaceArray : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueWorldToLightSpaceArray, 0x42FC7C2E );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueWorldToLightSpaceArray( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueWorldToLightSpaceArray( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueWorldToLightSpaceArray( mRegister ); }
		virtual b32 fIsShared( ) const { return false; }
		virtual void fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
	};
	class base_export tPsGlueFog : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueFog, 0x1B1AEE61 );
	private:
		u16 mPRegister, mCRegister;
	public:
		explicit tPsGlueFog( u32 pReg = 0, u32 cReg = 0 ) : mPRegister( pReg ), mCRegister( cReg ) { }
		tPsGlueFog( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueFog( mPRegister, mCRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueFlatParticleColor : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueFlatParticleColor, 0x5A4BE623 );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueFlatParticleColor( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueFlatParticleColor( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueFlatParticleColor( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueTime : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueTime, 0x3879FC4 );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueTime( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueTime( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueTime( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueInstanceTint : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueInstanceTint, 0x413F8917 );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueInstanceTint( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueInstanceTint( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueInstanceTint( mRegister ); }
		virtual b32 fIsShared( ) const { return false; }
		virtual void fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
	};
	class base_export tPsGlueDynamicVec4 : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueDynamicVec4, 0xC634109A );
	private:
		u16 mRegister, mNameIndex;
	public:
		explicit tPsGlueDynamicVec4( u32 reg = 0, u32 idx = 0 ) : mRegister( reg ), mNameIndex( idx ) { }
		tPsGlueDynamicVec4( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueDynamicVec4( mRegister, mNameIndex ); }
		virtual b32 fIsShared( ) const { return false; }
		virtual b32 fIsUnique( ) const { return false; }
		virtual void fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
	};
	class base_export tPsGlueRimLightParams : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueRimLightParams, 0xA188903 );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueRimLightParams( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueRimLightParams( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueRimLightParams( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueLightParams : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueLightParams, 0x4762A16D );
	private:
		u16 mRegister, mNumLights;
	public:
		explicit tPsGlueLightParams( u32 reg = 0, u32 numLights = 0 ) : mRegister( reg ), mNumLights( numLights ) { }
		tPsGlueLightParams( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueLightParams( mRegister, mNumLights ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueShadowMap : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueShadowMap, 0x18A96062 );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueShadowMap( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueShadowMap( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueShadowMap( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueWorldEyePos : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueWorldEyePos, 0x2F8FB0AB );
	private:
		u32 mRegister;
	public:
		explicit tPsGlueWorldEyePos( u32 reg = 0 ) : mRegister( reg ) { }
		tPsGlueWorldEyePos( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueWorldEyePos( mRegister ); }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueMaterialVector : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueMaterialVector, 0x2C29AFA1 );
	private:
		u16 mRegister, mVectorIndex;
	public:
		explicit tPsGlueMaterialVector( u32 reg = 0, u32 vIdx = 0 ) : mRegister( reg ), mVectorIndex( vIdx ) { }
		tPsGlueMaterialVector( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueMaterialVector( mRegister, mVectorIndex ); }
		virtual b32 fIsUnique( ) const { return false; }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};
	class base_export tPsGlueMaterialSampler : public tShadeMaterialGlue
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPsGlueMaterialSampler, 0x3E99972A );
	private:
		u16 mRegister, mSamplerIndex;
	public:
		explicit tPsGlueMaterialSampler( u32 reg = 0, u32 sIdx = 0 ) : mRegister( reg ), mSamplerIndex( sIdx ) { }
		tPsGlueMaterialSampler( tNoOpTag ) { }
		virtual tShadeMaterialGlue* fClone( ) const { return NEW tPsGlueMaterialSampler( mRegister, mSamplerIndex ); }
		virtual b32 fIsUnique( ) const { return false; }
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
	};

}}


#endif//__DerivedShadeMaterialGlue__
