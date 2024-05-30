#ifndef __tLightEntity__
#define __tLightEntity__
#include "tEntityDef.hpp"
#include "tSpatialEntity.hpp"
#include "tLight.hpp"
#include "tRenderToTexture.hpp"
#include "tDeviceResource.hpp"
#include "tCamera.hpp"

namespace Sig { namespace Gfx
{
	class tScreen;
	class tScreenPtr;
	class tViewport;
	class tWorldSpaceDisplayList;

	class base_export tLightEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tLightEntityDef, 0x63DDF66F );
	public:
		tLight mLightDesc;

	public:
		tLightEntityDef( );
		tLightEntityDef( tNoOpTag );
		~tLightEntityDef( );
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};


	class base_export tLightEntity : public tSpatialEntity
	{
		define_dynamic_cast( tLightEntity, tSpatialEntity );
	public:
		typedef tLight::tLightType tLightType;
		typedef tLight::tColorType tColorType;
	public:
		static const u32 cSpatialSetIndex;
	public:
		static void fSetShadowMapDefaults( f32 dist, f32 nearPlane, f32 farPlane, f32 vpWidth, f32 vpHeight );
		static std::string fCreateLightNameFromLevelPath( const tFilePathPtr& levelPath );
		static Math::tMat3f fCreateDirectionalMatrix( const Math::tVec3f& dir, f32 distFromOrigin = -1.f, const Math::tVec3f& target = Math::tVec3f::cZeroVector );
		static tLightEntity* fCreateDefaultLight( const tScreenPtr& screen, const tSceneGraphFile& sgFile );
	public:
		b8 mOn;
		b8 mCastsShadow;
		u8 mNumShadowLayers;
		b8 pad0;
		tLight mLight;
		tDynamicArray<tCamera> mCameras;
		u32 mShadowMapResolution;
		f32 mShadowMapDistFromOrigin;
		Math::tVec3f mShadowMapLightDir;
		Math::tVec3f mShadowMapTarget;

#ifdef sig_devmenu
		typedef tDevVar<Math::tVec3f> tDevVarColor;
		typedef tRefCounterPtr<tDevVarColor> tDevVarColorPtr;
		tDevVarColorPtr mDevVarColorAmbient;
		tDevVarColorPtr mDevVarColor0;
		tDevVarColorPtr mDevVarColor1;
		tDevVarColorPtr mDevVarColor2;
#endif//sig_devmenu

	public:

		explicit tLightEntity( const Math::tMat3f& objectToWorld, const tLight& light = tLight( ), const char* debugName = 0 );
		virtual u32	fSpatialSetIndex( ) const { return cSpatialSetIndex; }
		virtual b32 fIsHelper( ) const { return true; }

		void fSetOn( bool on ) { mOn = (on==true); }
		bool fOn( ) const { return mOn!=0; }

		void fSetCastsShadow( b32 cast ) { mCastsShadow = cast; }
		b32 fCastsShadow( ) const { return mCastsShadow; }
		u32 fShadowLayerCount( ) const { return mNumShadowLayers; }
		u32 fShadowMapResolution( ) const { return mShadowMapResolution; }
		f32 fShadowMapCascadeSplit( u32 ithSplit ) const;
		const Math::tVec3f& fShadowMapTarget( ) const { return mShadowMapTarget; }

		const tLight& fLightDesc( ) const { return mLight; }
		const tCamera& fCamera( u32 ithCam ) const { return mCameras[ ithCam ]; }

		void fSetLightDesc( const tLight& light );
		void fSetRadii( const Math::tVec2f& radii );
		Math::tVec4f&		fColor( tColorType ct ) { return mLight.fColor( ct ); }
		const Math::tVec4f& fColor( tColorType ct ) const { return mLight.fColor( ct ); }
		void fSyncBeforeRender( );
		void fSetupShadowCasting( const tDevicePtr& device, u32 shadowMapResolution, u32 numShadowMapLayers = 1 );
		void fUpdateShadowMapTarget( const Math::tVec3f& newPos );
		void fUpdateDefaultLightDirection( const Math::tVec3f& newDir );
		void fRenderShadowMap( tScreen& screen ) const;
		void fToShaderConstants( tLightShaderConstants& shaderConstants, const Math::tVec3f& eyePos ) const;

	private:
		void fQueryIntersectionForRendering( tScreen& screen, const tSceneGraph& sg, tDynamicArray<tWorldSpaceDisplayList>& displayLists ) const;
		void fUpdateObjectSpaceBox( );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tLightEntity );

	class base_export tLightEntityList : public tGrowableArray< tLightEntity* >
	{
	};

}}

#endif//__tLightEntity__
