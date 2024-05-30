#include "BasePch.hpp"
#include "tRenderableEntity.hpp"
#include "tLightEntity.hpp"
#include "tSceneGraphFile.hpp"
#include "tScreen.hpp"
#include "tViewport.hpp"
#include "tRenderContext.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tProfiler.hpp"
#include "Gui/tLenseFlareCanvas.hpp"
#include "tApplication.hpp"
#include "tProjectFile.hpp"

namespace Sig { namespace Gfx
{
	register_rtti_factory( tLightEntityDef, true )

	tLightEntityDef::tLightEntityDef( )
		: mCastsShadows( false )
		, mLenseFlareKey( cNoLenseFlare )
		, mShadowIntensity( 0.2f )
	{
	}

	tLightEntityDef::tLightEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mLightDesc( cNoOpTag )
	{
	}

	tLightEntityDef::~tLightEntityDef( )
	{
	}

	void tLightEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		const tStringPtr name = fEntityName( );

		tLightEntity* entity = new tLightEntity( mObjectToLocal, mLightDesc, name.fCStr( ) );
		entity->mCastsShadow = mCastsShadows;
		entity->mShadowAmount = mShadowIntensity;
		entity->mLenseFlareKey = mLenseFlareKey;

		fApplyPropsAndSpawnWithScript( *entity, params );
	}
}}

namespace Sig { namespace Gfx
{

	struct tBuildShadowMapDisplayList : public tEntityBVH::tIntersectVolumeCallback<Math::tFrustumf>
	{
		tScreen& mScreen;
		const tDynamicArray<tCamera>&					mCameras;
		tDynamicArray<tWorldSpaceDisplayList>&	mDisplayLists;
		u32 mCameraIndex;

		tBuildShadowMapDisplayList( const tDynamicArray<tCamera>& cameras, tDynamicArray<tWorldSpaceDisplayList>& displayLists, tScreen& screen, u32 cameraIndex ) 
			: mScreen( screen )
			, mCameras( cameras )
			, mDisplayLists( displayLists )
			, mCameraIndex( cameraIndex )
		{ }

		inline void operator()( const Math::tFrustumf& v, tEntityBVH::tObjectPtr i, b32 aabbWhollyContained ) const
		{
			tRenderableEntity* test = static_cast< tRenderableEntity* >( static_cast< tSpatialEntity* >( i->fOwner( ) ) );
			return operator()( test );
		}
		inline void operator()( const Math::tAabbf& v, tEntityBVH::tObjectPtr i, b32 aabbWhollyContained ) const
		{
			tRenderableEntity* test = static_cast< tRenderableEntity* >( static_cast< tSpatialEntity* >( i->fOwner( ) ) );
			return operator()( test );
		}
		inline void operator()( const Math::tSpheref& v, tEntityBVH::tObjectPtr i, b32 aabbWhollyContained ) const
		{
			tRenderableEntity* test = static_cast< tRenderableEntity* >( static_cast< tSpatialEntity* >( i->fOwner( ) ) );
			return operator()( test );
		}
		inline void operator()( tEntityBVH::tObjectPtr i ) const
		{
			tRenderableEntity* test = static_cast< tRenderableEntity* >( static_cast< tSpatialEntity* >( i->fOwner( ) ) );
			return operator()( test );
		}

		inline void operator()( tRenderableEntity* test ) const
		{
			if( test->fCastsShadow( ) )
			{
				mDisplayLists[ mCameraIndex ].fInsert( test->fGetDrawCall( 0.f ) );

				////if( test->fComputeFadeAlpha( mScreen ) < 1.f )
				////	return;

				//if( mCameras[cameraIndex].fGetWorldSpaceFrustum( ).fIntersects( test->fWorldSpaceBox( ) ) )
				//{
				//	mDisplayLists[0].fInsert( test->fGetDrawCall( 0.f ) );

				//	if( mDisplayLists.fCount( ) > 1 && mCameras[1].fGetWorldSpaceFrustum( ).fIntersects( test->fWorldSpaceBox( ) ) )
				//	{
				//		mDisplayLists[1].fInsert( test->fGetDrawCall( 0.f ) );

				//		if( mDisplayLists.fCount( ) > 2 && mCameras[2].fGetWorldSpaceFrustum( ).fIntersects( test->fWorldSpaceBox( ) ) )
				//			mDisplayLists[2].fInsert( test->fGetDrawCall( 0.f ) );
				//	}
				//}
			}
		}
	};

	namespace
	{
		enum tShadowMapFidelity
		{
			cFidelityLoRes,
			cFidelityHiRes,
			cFidelityCount
		};

		f32 gShadowAmount = 0.6f;

		static tShadowMapValues gShadowMapValues[cFidelityCount]=
		{
			tShadowMapValues( 800.f, 0.f, 1300.f, 500.f, 0.0009f ),
			tShadowMapValues( 800.f, 0.f, 1300.f, 20.f, 0.0002f ),
		};
	}

	devvar( bool, Renderer_Shadows_DrawLightPosDir, false );
	devvar( bool, Renderer_Debug_DrawLightPositions, false );
	devvar_clamp( f32, Renderer_Debug_DrawLightPositionsInnerOpacity, 0.6f, 0.0f, 1.0f, 2 );
	devvar_clamp( f32, Renderer_Debug_DrawLightPositionsOuterOpacity, 0.2f, 0.0f, 1.0f, 2 );
	devvar( f32, Renderer_Shadows_DPNearPlane, 0.85f );
	devvar( f32, Renderer_Shadows_DPFarPlane, 1000.0f );
	devvar( bool, Renderer_Shadows_DPUseGlobalAmount, false );
	devvar( bool, Renderer_ShadowS_DPUseInvertedEditorAmount, true );
	devvarptr_clamp( f32, Renderer_Shadows_Amount, gShadowAmount, 0.f, 1.0f, 3 );

	devvar( bool, Renderer_HackClampLightInnerRadiusEnable, false );
	devvar_clamp( f32, Renderer_HackClampLightInnerRadius, 0.0f, 0.0f, 1000.0f, 2 );

	devvarptr_clamp( f32, Renderer_Shadows_LoRes_Distance, gShadowMapValues[cFidelityLoRes].mLightDistanceFromOrigin, 1.f, 10000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_LoRes_NearPlane, gShadowMapValues[cFidelityLoRes].mNearPlane, 0.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_LoRes_FarPlane, gShadowMapValues[cFidelityLoRes].mFarPlane, 1.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_LoRes_ViewPlaneSize, gShadowMapValues[cFidelityLoRes].mViewPlaneSize, 1.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_LoRes_DepthBias, gShadowMapValues[cFidelityLoRes].mDepthBias, -10.f, 10.f, 4 );

	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_Distance, gShadowMapValues[cFidelityHiRes].mLightDistanceFromOrigin, 1.f, 10000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_NearPlane, gShadowMapValues[cFidelityHiRes].mNearPlane, 0.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_FarPlane, gShadowMapValues[cFidelityHiRes].mFarPlane, 1.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_ViewPlaneSize, gShadowMapValues[cFidelityHiRes].mViewPlaneSize, 1.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_DepthBias, gShadowMapValues[cFidelityHiRes].mDepthBias, -10.f, 10.f, 4 );

	const u32 tLightEntity::cSpatialSetIndex = tSceneGraph::fNextSpatialSetIndex( );

	void tLightEntity::fSetupShadowMapping( )
	{
		const tProjectFile::tShadowRuntimeSettings& shadowRuntimeSettings = tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mShadowRuntimeSettings;

		gShadowAmount = shadowRuntimeSettings.mShadowAmount;
		gShadowMapValues[ cFidelityHiRes ].mViewPlaneSize = shadowRuntimeSettings.mHighResShadowDist;
		gShadowMapValues[ cFidelityLoRes ].mViewPlaneSize = shadowRuntimeSettings.mLowResShadowDist;
	}

	std::string tLightEntity::fCreateLightNameFromLevelPath( const tFilePathPtr& levelPath )
	{
		std::string lightName = "$";
		lightName += StringUtil::fStripExtension( StringUtil::fNameFromPath( levelPath.fCStr( ) ).c_str( ) );
		StringUtil::fReplaceAllOf( lightName, "_", "." );
		return lightName;
	}

	Math::tMat3f tLightEntity::fCreateDirectionalMatrix( const Math::tVec3f& dir, f32 distFromOrigin, const Math::tVec3f& target )
	{
		Math::tMat3f m = Math::tMat3f::cIdentity;
		if( dir.fEqual( Math::tVec3f::cYAxis ) || dir.fEqual( -Math::tVec3f::cYAxis ) )
			m.fOrientZAxis( dir, Math::tVec3f::cXAxis );
		else
			m.fOrientZAxis( dir, Math::tVec3f::cYAxis );
		m.fSetTranslation( target - ( distFromOrigin < 0.f ? Renderer_Shadows_LoRes_Distance : distFromOrigin ) * dir );
		return m;
	}

	tLightEntity* tLightEntity::fCreateDefaultLight( tScreen& screen, const tSceneGraphFile& sgFile )
	{
		if( !sgFile.mDefaultLight )
			return 0;

		tLight desc;
		desc.fConvertFrom( sgFile.mDefaultLight );

		const Math::tVec3f shadowMapLightDir = Math::tVec3f( sgFile.mDefaultLight->mDirection ).fNormalizeSafe( Math::tVec3f::cZAxis );
		const f32 shadowMapDistFromOrigin = -1.f; // means use default
		const Math::tVec3f shadowMapTarget = Math::tVec3f::cZeroVector; // targets the world origin by default
		const Math::tMat3f m = fCreateDirectionalMatrix( shadowMapLightDir, shadowMapDistFromOrigin, shadowMapTarget );

		tLightEntity* lightEntity = new tLightEntity( m, desc, "$DefaultLight" );
		lightEntity->mShadowMapLightDir = shadowMapLightDir;
		lightEntity->mShadowMapDistFromOrigin = shadowMapDistFromOrigin;
		lightEntity->mShadowMapTarget = shadowMapTarget;

		if( sgFile.mDefaultLight->mCastShadow && !desc.fColor( tLight::cColorTypeFront ).fXYZ( ).fEqual( Math::tVec3f::cZeroVector ) )
			lightEntity->fSetupShadowCasting( screen );

		return lightEntity;
	}

	tLightEntity::tLightEntity( const Math::tMat3f& objectToWorld, const tLight& light, const char* debugName )
		: mOn( true )
		, mCastsShadow( false )
		, mNumShadowLayers( 0 )
		, mNotPotentiallyVisible( false )
		, mLenseFlareKey( tLightEntityDef::cNoLenseFlare )
		, mShadowAmount( 0.f )
		, mLight( light )
		, mShadowMapResolution( 0 )
		, mShadowMapDistFromOrigin( -1.f )
		, mShadowMapLightDir( -Math::tVec3f::cYAxis )
		, mShadowMapTarget( Math::tVec3f::cZeroVector )
		, mShadowFade( 1.f )
	{
		fUpdateObjectSpaceBox( );
		fMoveTo( objectToWorld );

#ifdef sig_devmenu
		if( debugName && StringUtil::fStrLen( debugName ) )
		{
			std::string devVarNameBase = "Renderer_Lights_";
			devVarNameBase += debugName;
			mDevVarColorAmbient.fReset( new tColorDevVar( ( devVarNameBase + "_Ambient" ).c_str( ), mLight.fColor3( tLight::cColorTypeAmbient ), 0.f, 2.f, false ) );
			mDevVarColor0.fReset( new tColorDevVar( ( devVarNameBase + "_Front" ).c_str( ), mLight.fColor3( tLight::cColorTypeFront ), 0.f, 2.f, false ) );
			//mDevVarColor1.fReset( new tColorDevVar( ( devVarNameBase + "_Rim" ).c_str( ), mLight.fColor3( tLight::cColorTypeRim ), 0.f, 2.f, false ) );
			mDevVarColor2.fReset( new tColorDevVar( ( devVarNameBase + "_Back" ).c_str( ), mLight.fColor3( tLight::cColorTypeBack ), 0.f, 2.f, false ) );
		}
#endif//sig_devmenu
	}


	tLightEntity::~tLightEntity( )
	{
		// destructor here so that tWorldSpaceDisplayList type is complete.
	}

	void tLightEntity::fOnSpawn( )
	{
		tSpatialEntity::fOnSpawn( );

		if( mLenseFlareKey != tLightEntityDef::cNoLenseFlare )
			fSetupLenseFlares( );
	}

	void tLightEntity::fOnDelete( )
	{
		for( u32 i = 0; i < mLenseFlares.fCount( ); ++i )
			mLenseFlares[ i ]->fDeleteSelf( );
		mLenseFlares.fSetCount( 0 );

		tSpatialEntity::fOnDelete( );
	}

	void tLightEntity::fSetupLenseFlares( )
	{
		sigassert( fSceneGraph( ) );

		tScreen* screen = fSceneGraph( )->fScreen( );
		if( screen )
		{
			for( u32 i = 0; i < screen->fGetViewportCount( ); ++i )
			{
				Gui::tLenseFlareCanvas* canvas = NEW Gui::tLenseFlareCanvas( );
				mLenseFlares.fPushBack( tRefCounterPtr<Gui::tLenseFlareCanvas>( canvas ) );
				canvas->fSetLenseFlare( mLenseFlareKey );
				canvas->fSetViewport( *screen->fViewport( i ) );
				canvas->fSetTrackingEntity( *this );
				screen->fWorldSpaceCanvas( ).fAddChild( Gui::tCanvasPtr( canvas ) );
			}
		}
	}

	void tLightEntity::fSetLightDesc( const tLight& light )
	{
		mLight = light;
		fUpdateObjectSpaceBox( );
#ifdef sig_devmenu
		if( mDevVarColorAmbient )
		{
			mDevVarColor0->fSetFromVector( light.fColor( tLight::cColorTypeFront ) );
			//mDevVarColor1->fSetFromVector( light.fColor( tLight::cColorTypeRim ) );
			mDevVarColor2->fSetFromVector( light.fColor( tLight::cColorTypeBack ) );
			mDevVarColorAmbient->fSetFromVector( light.fColor( tLight::cColorTypeAmbient ) );
		}
#endif//sig_devmenu
	}

	void tLightEntity::fSetRadii( const Math::tVec2f& radii )
	{
		mLight.fRadii( ) = radii;
		fUpdateObjectSpaceBox( );
	}

	void tLightEntity::fSetColor( tColorType ct, const Math::tVec4f& clr )
	{
		mLight.fColor( ct ) = clr;
#ifdef sig_devmenu
		if( mDevVarColorAmbient )
		{
			switch( ct )
			{
			case tLight::cColorTypeFront: mDevVarColor0->fSetFromVector( clr ); break;
			//case tLight::cColorTypeRim: mDevVarColor1->fSetFromVector( clr ); break;
			case tLight::cColorTypeBack: mDevVarColor2->fSetFromVector( clr ); break;
			case tLight::cColorTypeAmbient: mDevVarColorAmbient->fSetFromVector( clr ); break;
			}
		}
#endif//sig_devmenu
	}

	void tLightEntity::fSetColors( const Math::tVec3f& front, const Math::tVec3f& rim, const Math::tVec3f& back, const Math::tVec3f& ambient )
	{
		mLight.fSetColors( front, rim, back, ambient );
#ifdef sig_devmenu
		if( mDevVarColorAmbient )
		{
			mDevVarColor0->fSetFromVector( Math::tVec4f( front, 1.f ) );
			//mDevVarColor1->fSetFromVector( Math::tVec4f( rim, 1.f ) );
			mDevVarColor2->fSetFromVector( Math::tVec4f( back, 1.f ) );
			mDevVarColorAmbient->fSetFromVector( Math::tVec4f( ambient, 1.f ) );
		}
#endif//sig_devmenu
	}

	void tLightEntity::fSyncBeforeRender( const tScreen& screen)
	{
#ifdef sig_devmenu
		if( mDevVarColorAmbient )
		{
			mLight.fColor( tLight::cColorTypeAmbient ) = Math::tVec4f( *mDevVarColorAmbient, 1.f );
			mLight.fColor( tLight::cColorTypeFront ) = Math::tVec4f( *mDevVarColor0, 1.f );
			//mLight.fColor( tLight::cColorTypeRim ) = Math::tVec4f( *mDevVarColor1, 1.f );
			mLight.fColor( tLight::cColorTypeBack ) = Math::tVec4f( *mDevVarColor2, 1.f );
		}
#endif//sig_devmenu

		if( mLight.fLightType( ) == tLight::cLightTypePoint && Renderer_Debug_DrawLightPositions )
		{
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tSpheref( fObjectToWorld( ).fGetTranslation( ), fMax( fEffectiveInnerRadius( ), 1.0f ) ), Math::tVec4f( 1, 0, 0, Renderer_Debug_DrawLightPositionsInnerOpacity ) );
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tSpheref( fObjectToWorld( ).fGetTranslation( ), fEffectiveRadius( ) ), Math::tVec4f( 1, 1, 0, Renderer_Debug_DrawLightPositionsOuterOpacity ) );
		}

		// synchronize shadow map camera if we cast shadow
		if( fCastsShadow( ) )
		{
			if( mNumShadowLayers == 0 )
				fSetupShadowCasting( screen );

			switch( mLight.fLightType( ) )
			{
			case tLight::cLightTypeDirection:
				{
					sigassert( mCameras.fCount( ) == mNumShadowLayers );
					for( u32 i = 0; i < mCameras.fCount( ); ++i )
					{
						const Math::tMat3f m = fCreateDirectionalMatrix( mShadowMapLightDir, gShadowMapValues[ i ].mLightDistanceFromOrigin, mShadowMapTarget );
						const Math::tVec3f origin = m.fGetTranslation( );
						const Math::tVec3f zaxis = m.fZAxis( );

						if( Renderer_Shadows_DrawLightPosDir )
						{
							const Math::tVec3f startPos = origin;
							const Math::tVec3f endPos = origin + zaxis * Renderer_Shadows_LoRes_Distance;
							const u32 numSteps = 100;
							for( u32 i = 0; i < numSteps; ++i )
							{
								const f32 t = i / ( numSteps - 1.f );
								fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tSpheref( Math::fLerp( startPos, endPos, t ), 2.f ), Math::tVec4f(0.5f+(1.f-t)*2.f,0.5f+(1.f-t)*2.f,0.f,1.f) );
							}
						}
						tLens lens;
						lens.fSetOrtho( 
							gShadowMapValues[ i ].mNearPlane, 
							fMax<f32>( gShadowMapValues[ i ].mNearPlane + 1.f, gShadowMapValues[ i ].mFarPlane ), 
							gShadowMapValues[ i ].mViewPlaneSize, 
							gShadowMapValues[ i ].mViewPlaneSize );

						const Math::tVec3f up = fEqual( fAbs( zaxis.fDot( Math::tVec3f::cYAxis ) ), 1.f ) ?  Math::tVec3f::cXAxis : Math::tVec3f::cYAxis;
						mCameras[ i ].fSetup( lens, tTripod( origin, origin + zaxis, up ) );

						mCameras[ i ].fCorrectForShadowMap( mShadowMapResolution );
					}
				}
				break;
			case tLight::cLightTypePoint:
				{
					// For paraboloid mapping. we want the projection to be the identity.
					tLens lens;
					lens.fSetOrtho( 0.0f, 1.0f, -2.0f, 2.0f );

					Math::tMat4f testM;
					lens.fConstructProjectionMatrix( testM );
					sigassert( testM.fEqual( Math::tMat4f::cIdentity ) );

					Math::tVec3f origin = fObjectToWorld( ).fGetTranslation( );
					Math::tVec3f up = Math::tVec3f::cZAxis;

					for( u32 i = 0; i < mCameras.fCount( ); ++i )
					{
						Math::tVec3f look = (i==0) ? -Math::tVec3f::cYAxis : Math::tVec3f::cYAxis;
						mCameras[ i ].fSetup( lens, tTripod( origin, origin + look, up ) );
					}
				}
				break;
			}
		}
	}

	void tLightEntity::fSetupShadowCasting( const tScreen& screen )
	{
		u32 layers = fShadowMap( screen )->fLayerCount( );
		u32 resolution = fShadowMap( screen )->fWidth( );

//#if !defined( platform_xbox360 )
//		// only support cascades on 360 for now (bcz of texture arrays)
//		layers = 1;
//#endif

		if( mLight.fLightType( ) != tLight::cLightTypeDirection
			&& mLight.fLightType( ) != tLight::cLightTypePoint)
		{
			log_warning( "Shadow mapping is currently only supported on directional and point lights" );
			return;
		}

		mCastsShadow = true;
		mShadowMapResolution = resolution;
		mNumShadowLayers = layers;
		mCameras.fNewArray( mNumShadowLayers );
	}

	void tLightEntity::fUpdateShadowMapTarget( const Math::tVec3f& newPos )
	{
		mShadowMapTarget = newPos;
		const Math::tMat3f m = fCreateDirectionalMatrix( mShadowMapLightDir, mShadowMapDistFromOrigin, mShadowMapTarget );
		fMoveTo( m );
	}

	void tLightEntity::fUpdateDefaultLightDirection( const Math::tVec3f& newDir )
	{
		mShadowMapLightDir = newDir;
		const Math::tMat3f m = fCreateDirectionalMatrix( mShadowMapLightDir, mShadowMapDistFromOrigin, mShadowMapTarget );
		fMoveTo( m );
	}

	const tRenderToTexturePtr& tLightEntity::fShadowMap( const tScreen& screen ) const
	{
		u32 index = (fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection) ? 0 : 1;
		return screen.fShadowMap( index );
	}

	void tLightEntity::fResetShadowMaps( tScreen& screen ) const
	{
		u32 shadowMapCount = fMin<u32>( fShadowMap( screen )->fLayerCount( ), screen.fLimitShadowMapLayerCount( ) );

		if( shadowMapCount )
		{
			profile( cProfilePerfRenderShadowMapsQuery );

			for( u32 i = 0; i < mShadowDisplayLists.fCount( ); ++i )
				mShadowDisplayLists[ i ].fInvalidate( ); //do this first so nothing is copied in the resize

			if( shadowMapCount != mShadowDisplayLists.fCount( ) )
				mShadowDisplayLists.fResize( shadowMapCount );
		}
	}

	void tLightEntity::fBuildShadowDisplayLists( tScreen& screen ) const
	{
		sigassert( !fCanQueryWithFrustum( ) );

		if( mShadowDisplayLists.fCount( ) )
		{
			// we need to manually do our own query. this is for point lights and non-frustum shapes.
			Math::tSpheref sphere( fObjectToWorld( ).fGetTranslation( ), fEffectiveRadius( ) );

			sigassert( mCameras.fCount( ) );
			tBuildShadowMapDisplayList build( mCameras, mShadowDisplayLists, screen, 0 );
			screen.fSceneGraph( )->fIntersect( sphere, build, tRenderableEntity::cSpatialSetIndex );

			// copy display lists to other shadow cameras, since we queried an all encompassing sphere.
			for( u32 i = 1; i < mShadowDisplayLists.fCount( ); ++i )
				mShadowDisplayLists[ i ] = mShadowDisplayLists[ 0 ];
		}
	}

	void tLightEntity::fRenderShadowMap( tScreen& screen ) const
	{
		const tRenderToTexturePtr& shadowMap = fShadowMap( screen );

		sigassert( fSceneGraph( ) );
		u32 shadowMapCount = fMin<u32>( shadowMap->fLayerCount( ), mNumShadowLayers, screen.fLimitShadowMapLayerCount( ) );

		if( shadowMapCount )
		{
			sigassert( shadowMap->fLayerCount( ) >= shadowMapCount );

			for( u32 i = 0; i < shadowMapCount; ++i )
			{
#ifdef sig_profile
				std::stringstream desc;
				desc << "Shadow map LAYER " << (i+1) << " of " << shadowMapCount;
				profile_pix( desc.str().c_str() );
#endif

				profile( cProfilePerfRenderShadowMapsRender );

				// set shadow map render targets
				shadowMap->fApplyDepthOnly( screen );

				// set clip box (full viewport)
				shadowMap->fSetClipBox( screen, Math::tVec4f( 0.f, 0.f, 1.f, 1.f ) );

				// clear render targets
				screen.fClearCurrentRenderTargets( true, true, Math::tVec4f(1.f), 0 );

				// set default render state for depth rendering
				tRenderContext renderContext;
				renderContext.fFromScreen( screen );
				renderContext.mRenderPassMode = tRenderState::cRenderPassShadowMap;
				renderContext.mSceneCamera = &mCameras[ i ];

				// These should not be cleared during this render stage. (fClearLights specifically)
				fSetShadowParams( screen, renderContext );
				renderContext.mShadowGeneratingLight = this;

				// invalidate render states
				screen.fGetDevice( )->fInvalidateLastRenderState( );

				// render display list, using light camera
				mShadowDisplayLists[ i ].fOpaque( ).fRender( screen, renderContext );
				mShadowDisplayLists[ i ].fXparent( ).fRender( screen, renderContext );
				mShadowDisplayLists[ i ].fXparentWithDepthPrepass( ).fRender( screen, renderContext );

				// resolve shadow map texture
				shadowMap->fEndDepthOnly( screen );
				shadowMap->fResolveDepth( screen, i );
			}
		}
	}

	void tLightEntity::fSetShadowParams( const tScreen& screen, tRenderContext& contextOut ) const
	{
		contextOut.mShadowMap = &fShadowMap( screen )->fTexture( );
		contextOut.mShadowMapTexelSize = fShadowMapResolution( );
		contextOut.mShadowMapTarget = fShadowMapTarget( );
		contextOut.mShadowMapRealLayerCount = fShadowLayerCount( );
		contextOut.mShadowMapLayerCount = fMin( contextOut.mShadowMapRealLayerCount, screen.fLimitShadowMapLayerCount( ) );

		for( u32 i = 0; i < contextOut.mShadowMapLayerCount; ++i )
			contextOut.mShadowMapEpsilon[ i ] = gShadowMapValues[ i ].mDepthBias;

		for( u32 i = 0; i < contextOut.mShadowMapLayerCount; ++i )
		{
			contextOut.mWorldToLightSpace[ i ] = fCamera( i ).fGetWorldToProjection( );
			contextOut.mViewToLightSpace[ i ] = fCamera( i ).fGetCameraToProjection( );
		}

		if( fLightDesc( ).fLightType( ) == tLight::cLightTypePoint )
		{
			// resuses split vector for far plane information
			contextOut.mShadowMapCascadeSplit[ 0 ] = Renderer_Shadows_DPNearPlane;
			contextOut.mShadowMapCascadeSplit[ 1 ] = Renderer_Shadows_DPFarPlane; // far plane (in parabola space, not same as fEffectiveRadius)
			contextOut.mShadowAmount
				= Renderer_Shadows_DPUseGlobalAmount ? gShadowAmount
				: Renderer_ShadowS_DPUseInvertedEditorAmount ? (1 - fShadowAmount( )) // Existing bizzaro-world behavior. Let the artists fix, then remove.
				: fShadowAmount( ); // controlled by editor
		}
		else
		{
			s32 splitCount = contextOut.mShadowMapLayerCount - 1;
			for( s32 i = 0; i < splitCount; ++i )
				contextOut.mShadowMapCascadeSplit[ i ] = gShadowMapValues[cFidelityHiRes + i].mViewPlaneSize / 2.f;

			contextOut.mShadowAmount = gShadowAmount;
		}
	}

	void tLightEntity::fToShaderConstants( tLightShaderConstants& shaderConstants, const Math::tVec3f& eyePos ) const
	{
		tProjectFile::tRendererSettings& rs = tProjectFile::fInstance( ).mEngineConfig.mRendererSettings;

		const Math::tMat3f& objToWorld = fObjectToWorld( );
		const b32 isPointLight		= mLight.fLightType( ) == tLight::cLightTypePoint;
		const f32 lightScale		= objToWorld.fGetScale( ).fMax( ) * (isPointLight ? rs.mPointLightSizeMultiplier : 1.0f);
		const f32 directionCoeff	= fLightDesc( ).fDirectionalCoefficient( );
		const f32 positionCoeff		= fLightDesc( ).fPositionalCoefficient( );
		const f32 intensityScale	= isPointLight ? rs.mPointLightIntensityMultiplier : 1.0f;

		f32 innerRadius = lightScale * fLightDesc( ).fRadii( ).x;
		if( Renderer_HackClampLightInnerRadiusEnable )
			innerRadius = fMin( innerRadius, (f32)Renderer_HackClampLightInnerRadius );
		const f32 outerRadius = lightScale * fLightDesc( ).fRadii( ).y;

		const f32 rcpInnerOuterRadiusDistance = 1.0f / fMax( 0.0001f, (outerRadius - innerRadius) ); // precalculate instead of doing it in the pixel shader.

		shaderConstants.mLightDir			= Math::tVec4f( -objToWorld.fZAxis( ), directionCoeff );
		shaderConstants.mLightPos			= Math::tVec4f(  objToWorld.fGetTranslation( ) - eyePos, positionCoeff );
		shaderConstants.mLightAttenuation	= Math::tVec4f( outerRadius, rcpInnerOuterRadiusDistance, directionCoeff, positionCoeff );
		shaderConstants.mLightAmbient		= directionCoeff * fLightDesc( ).fColor( tLight::cColorTypeAmbient ) * intensityScale;
		shaderConstants.mLightFront			= fLightDesc( ).fColor( tLight::cColorTypeFront ) * intensityScale;
		shaderConstants.mLightSurround		= fLightDesc( ).fColor( tLight::cColorTypeRim ) * intensityScale;
		shaderConstants.mLightBack			= fLightDesc( ).fColor( tLight::cColorTypeBack ) * intensityScale;
	}

	void tLightEntity::fQueryIntersectionForRendering( tScreen& screen, const tSceneGraph& sg, tDynamicArray<tWorldSpaceDisplayList>& displayLists ) const
	{
		sigassert( !"Using linear frustum culling!" );

		switch( mLight.fLightType( ) )
		{
		case tLight::cLightTypeDirection:
			{
				for( u32 i = 0; i < displayLists.fCount( ); ++i )
				{
					mCulledResults.fSetCount( 0 );

					const Math::tFrustumf& f =  mCameras[ i ].fGetWorldSpaceFrustum( );
					sg.fIntersectDeferred( f, mCulledResults, tRenderableEntity::cSpatialSetIndex );
					sg.fIntersectDeferred( f, mCulledResults, tRenderableEntity::cHeightFieldSpatialSetIndex );

					tBuildShadowMapDisplayList builder( mCameras, displayLists, screen, i );
					for( u32 r = 0; r < mCulledResults.fCount( ); ++r )
						builder( static_cast<tRenderableEntity*>( mCulledResults[ r ] ) );

					if( i == 0 )
						sg.fIntersectCloudRenderables( f, builder, true );
				}
			}
			break;
		//case tLight::cLightTypePoint:
		//	{
		//		const Math::tAabbf aabb( Math::tSpheref( fObjectToWorld( ).fGetTranslation( ), fObjectToWorld( ).fGetScale( ).fMax( ) * mLight.fRadii( ).y ) );
		//		tBuildShadowMapDisplayList<Math::tAabbf> builder( camera, displayList );
		//		sg.fIntersect( aabb, builder, tRenderableEntity::cSpatialSetIndex );
		//		sg.fIntersect( aabb, builder, tRenderableEntity::cHeightFieldSpatialSetIndex );
		//		sg.fIntersectCloudRenderables( aabb, builder, true );
		//	}
		//	break;
		//case tLight::cLightTypeSpot:
		//	{
		//		const Math::tAabbf aabb( Math::tSpheref( fObjectToWorld( ).fGetTranslation( ), fObjectToWorld( ).fGetScale( ).fMax( ) * mLight.fRadii( ).y ) );
		//		tBuildShadowMapDisplayList<Math::tAabbf> builder( camera, displayList );
		//		sg.fIntersect( aabb, builder, tRenderableEntity::cSpatialSetIndex );
		//		sg.fIntersect( aabb, builder, tRenderableEntity::cHeightFieldSpatialSetIndex );
		//		sg.fIntersectCloudRenderables( aabb, builder, true );
		//	}
		//	break;
		default:
			sigassert( !"invalid light type in tLightEntity::fQueryIntersectionForRendering" );
			break;
		}

		for( u32 i = 0; i < displayLists.fCount( ); ++i )
			displayLists[ i ].fSeal( );
	}

	void tLightEntity::fUpdateObjectSpaceBox( )
	{
		switch( mLight.fLightType( ) )
		{
		case tLight::cLightTypeDirection:
			{
				const Math::tSpheref hugeSphere( Math::cInfinity );
				fSetObjectSpaceBox( Math::tAabbf( hugeSphere ) );
			}
			break;
		case tLight::cLightTypePoint:
			{
				const Math::tSpheref pointSphere( mLight.fRadii( ).y );
				fSetObjectSpaceBox( Math::tAabbf( pointSphere ) );
			}
			break;
		default:
			sigassert( !"invalid light type in tLightEntity::fQueryIntersectionForRendering" );
			break;
		}
	}

}}

namespace Sig { namespace Gfx
{
	namespace
	{
		static tLightEntity* fCast( tEntity* baseEntity )
		{
			return baseEntity->fDynamicCast< tLightEntity >( );
		}
		static Math::tVec4f fGetColorForScript( tLightEntity* light, u32 type )
		{
			sigassert( light );
			return light->fColor( (tLight::tColorType)type );
		}
		static Math::tVec4f fGetColorRgba( tLightEntity* light )
		{
			return light->fColor( tLight::cColorTypeFront );
		}
		static void fSetColorRgba( tLightEntity* light, const Math::tVec4f& rgba )
		{
			light->fSetColor( tLight::cColorTypeFront, rgba );
		}
		static f32 fGetInnerRadius( tLightEntity* light )
		{
			return light->fLightDesc( ).fRadii( ).x;
		}
		static f32 fGetOuterRadius( tLightEntity* light )
		{
			return light->fLightDesc( ).fRadii( ).y;
		}
		static void fSetInnerRadius( tLightEntity* light, f32 radius )
		{
			Math::tVec2f radii = light->fLightDesc( ).fRadii( );
			radii.x = radius;
			radii.y = fMax( radii.x, radii.y );
			light->fSetRadii( radii );
		}
		static void fSetOuterRadius( tLightEntity* light, f32 radius )
		{
			Math::tVec2f radii = light->fLightDesc( ).fRadii( );
			radii.y = radius;
			radii.x = fMin( radii.x, radii.y );
			light->fSetRadii( radii );
		}
		void fSetColorForScript( tLightEntity* light, u32 which, const Math::tVec4f& clr )
		{
			sigassert( light );
			light->fSetColor( (tLight::tColorType)which, clr );
		}
	}
	void tLightEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tLightEntity, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("Cast"), &fCast)
			.GlobalFunc(_SC("GetColor"), &fGetColorForScript)
			.GlobalFunc(_SC("GetRgba"), &fGetColorRgba)
			.GlobalFunc(_SC("SetRgba"), &fSetColorRgba)
			.GlobalFunc(_SC("GetInnerRadius"), &fGetInnerRadius)
			.GlobalFunc(_SC("SetInnerRadius"), &fSetInnerRadius)
			.GlobalFunc(_SC("GetOuterRadius"), &fGetOuterRadius)
			.GlobalFunc(_SC("SetOuterRadius"), &fSetOuterRadius)
			.Func(_SC("SetRadii"), &tLightEntity::fSetRadii)
			.GlobalFunc(_SC("SetColor"), &fSetColorForScript)
			.Prop(_SC("On"), &tLightEntity::fOn, &tLightEntity::fSetOn)
			.Prop(_SC("CastsShadow"), &tLightEntity::fCastsShadow, &tLightEntity::fSetCastsShadow)
			.Func(_SC("SetLightDesc"), &tLightEntity::fSetLightDescForScript)
			;
		vm.fNamespace(_SC("Gfx")).Bind(_SC("LightEntity"), classDesc);
	}
}}
