#ifndef __tLightEntity__
#define __tLightEntity__
#include "tEntityDef.hpp"
#include "tSpatialEntity.hpp"
#include "tLight.hpp"
#include "tDeviceResource.hpp"
#include "tCamera.hpp"
#include "tRenderToTexture.hpp"

namespace Sig { 

	class tDataTable;

	namespace Gui { class tLenseFlareCanvas; }
	
namespace Gfx
{
	class tScreen;
	class tScreenPtr;
	class tViewport;
	class tWorldSpaceDisplayList;

	struct tShadowMapValues
	{
		f32 mLightDistanceFromOrigin;
		f32 mNearPlane;
		f32 mFarPlane;
		f32 mViewPlaneSize;
		f32 mDepthBias;

		tShadowMapValues( )
		{
			fZeroOut( *this );
		}
		tShadowMapValues( 
			f32 lightDistanceFromOrigin,
			f32 shadowMapNearPlane,
			f32 shadowMapFarPlane,
			f32 shadowMapViewPlaneSize,
			f32 depthBias )
			: mLightDistanceFromOrigin( lightDistanceFromOrigin )
			, mNearPlane( shadowMapNearPlane )
			, mFarPlane( shadowMapFarPlane )
			, mViewPlaneSize( shadowMapViewPlaneSize )
			, mDepthBias( depthBias )
		{
		}
	};

	class base_export tLightEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tLightEntityDef, 0x63DDF66F );
	public:
		tLight mLightDesc;

		b8 mCastsShadows;
		u8 pad0;
		b8 pad1;
		b8 pad2;
		
		u32 mLenseFlareKey; // ~0 is none

		f32 mShadowIntensity; //0 = shadows are black, 1 = shadows are indistinguishable from lit.

		static const u32 cNoLenseFlare = ~0;

	public:
		tLightEntityDef( );
		tLightEntityDef( tNoOpTag );
		~tLightEntityDef( );
		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;
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
		static void fSetupShadowMapping( );
		static std::string fCreateLightNameFromLevelPath( const tFilePathPtr& levelPath );
		static Math::tMat3f fCreateDirectionalMatrix( const Math::tVec3f& dir, f32 distFromOrigin = -1.f, const Math::tVec3f& target = Math::tVec3f::cZeroVector );
		static tLightEntity* fCreateDefaultLight( tScreen& screen, const tSceneGraphFile& sgFile );
	public:
		b8 mOn;
		b8 mCastsShadow;
		u8 mNumShadowLayers;
		b8 mNotPotentiallyVisible;

		u32 mLenseFlareKey;
		f32 mShadowAmount;
		f32 mShadowFade; // 1.0 for full shadow effect, 0.0 for no shadow effect.

		tLight mLight;
		tDynamicArray<tCamera> mCameras;
		u32 mShadowMapResolution;
		f32 mShadowMapDistFromOrigin;
		Math::tVec3f mShadowMapLightDir;
		Math::tVec3f mShadowMapTarget;
		mutable tDynamicArray<tWorldSpaceDisplayList> mShadowDisplayLists;
		mutable tGrowableArray< void* > mCulledResults;
		tGrowableArray< tRefCounterPtr< Gui::tLenseFlareCanvas > > mLenseFlares; // one per viewport

#ifdef sig_devmenu
		typedef tDevVar<Math::tVec3f> tDevVarColor;
		typedef tRefCounterPtr<tColorDevVar> tDevVarColorPtr;
		tDevVarColorPtr mDevVarColorAmbient;
		tDevVarColorPtr mDevVarColor0;
		tDevVarColorPtr mDevVarColor1;
		tDevVarColorPtr mDevVarColor2;
#endif//sig_devmenu

	public:

		explicit tLightEntity( const Math::tMat3f& objectToWorld, const tLight& light = tLight( ), const char* debugName = 0 );
		~tLightEntity( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual u32	fSpatialSetIndex( ) const { return cSpatialSetIndex; }
		virtual b32 fIsHelper( ) const { return true; }

		void fSetOn( bool on ) { mOn = (on==true); }
		bool fOn( ) const { return mOn!=0; }

		void fSetNotPotentiallyVisible( b32 notPotentiallyVisible ) OVERRIDE { mNotPotentiallyVisible = notPotentiallyVisible; }
		b32 fNotPotentiallyVisible( ) const OVERRIDE { return mNotPotentiallyVisible; }

		b32 fVisible( ) const { return !fNotPotentiallyVisible( ) && fOn( ); }

		void fSetCastsShadow( b32 cast ) { mCastsShadow = cast; }
		b32 fCastsShadow( ) const { return mCastsShadow; }
		u32 fShadowLayerCount( ) const { return mNumShadowLayers; }
		u32 fShadowMapResolution( ) const { return mShadowMapResolution; }
		f32 fShadowAmount( ) const { return Math::fLerp( 1.0f, mShadowAmount, mShadowFade ); }

		// 1.0 for full shadow effect, 0.0 for no shadow effect.
		void fSetShadowFade( f32 fade ) { mShadowFade = fade; }

		const Math::tVec3f& fShadowMapTarget( ) const { return mShadowMapTarget; }

		const tLight& fLightDesc( ) const { return mLight; }
		const tCamera& fCamera( u32 ithCam ) const { return mCameras[ ithCam ]; }

		void fSetLightDesc( const tLight& light );
		void fSetLightDescForScript( tLight* desc ) { sigassert( desc ); fSetLightDesc( *desc ); }
		void fSetRadii( const Math::tVec2f& radii );
		const Math::tVec2f& fRadii( ) const { return mLight.fRadii( ); }
		f32	fEffectiveRadius( ) const { return fObjectToWorld( ).fGetScale( ).fMax( ) * mLight.fRadii( ).y; }
		f32	fEffectiveInnerRadius( ) const { return fObjectToWorld( ).fGetScale( ).fMax( ) * mLight.fRadii( ).x; }
		b32 fCanQueryWithFrustum( ) const { return fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection; }

		void fSetColor( tColorType ct, const Math::tVec4f& clr );
		void fSetColors( const Math::tVec3f& front, const Math::tVec3f& rim, const Math::tVec3f& back, const Math::tVec3f& ambient );
		const Math::tVec4f& fColor( tColorType ct ) const { return mLight.fColor( ct ); }
		
		void fSyncBeforeRender( const tScreen& screen );
		void fSetupShadowCasting( const tScreen& screen );
		void fUpdateShadowMapTarget( const Math::tVec3f& newPos );
		void fUpdateDefaultLightDirection( const Math::tVec3f& newDir );

		void fResetShadowMaps( tScreen& screen ) const;
		void fBuildShadowDisplayLists( tScreen& screen ) const;
		void fRenderShadowMap( tScreen& screen ) const;

		void fSetShadowParams( const tScreen& screen, tRenderContext& contextOut ) const;
		void fToShaderConstants( tLightShaderConstants& shaderConstants, const Math::tVec3f& eyePos ) const;

		const tRenderToTexturePtr& fShadowMap( const tScreen& screen ) const;

	private:
		void fQueryIntersectionForRendering( tScreen& screen, const tSceneGraph& sg, tDynamicArray<tWorldSpaceDisplayList>& displayLists ) const;
		void fUpdateObjectSpaceBox( );

		void fSetupLenseFlares( );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tLightEntity );

	class base_export tLightEntityList : public tGrowableArray< tLightEntity* >
	{
	};

}}

#endif//__tLightEntity__
