#include "BasePch.hpp"
#include "tRenderableEntity.hpp"
#include "tLightEntity.hpp"
#include "tSceneGraphFile.hpp"
#include "tScreen.hpp"
#include "tViewport.hpp"
#include "tRenderContext.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tGameAppBase.hpp"

namespace Sig { namespace Gfx
{
	register_rtti_factory( tLightEntityDef, true )

	tLightEntityDef::tLightEntityDef( )
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

	void tLightEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		const tStringPtr name = fEntityName( );
		tLightEntity* entity = NEW tLightEntity( mObjectToLocal, mLightDesc, name.fCStr( ) );
		fApplyPropsAndSpawnWithScript( *entity, parent, creationFlags );
	}
}}

namespace Sig { namespace Gfx
{
	template<class tVolume>
	struct tBuildShadowMapDisplayList : public tEntityBVH::tIntersectVolumeCallback<tVolume>
	{
		tScreen& mScreen;
		const tDynamicArray<tCamera>&					mCameras;
		tDynamicArray<tWorldSpaceDisplayList>&			mDisplayLists;

		tBuildShadowMapDisplayList( const tDynamicArray<tCamera>& cameras, tDynamicArray<tWorldSpaceDisplayList>& displayLists, tScreen& screen ) 
			: mScreen( screen ), mCameras( cameras ), mDisplayLists( displayLists ) { }

		inline void operator()( const tVolume& v, tEntityBVH::tObjectPtr i, b32 aabbWhollyContained ) const
		{
			return operator()( i );
		}
		inline void operator()( tEntityBVH::tObjectPtr i ) const
		{
			tRenderableEntity* test = static_cast< tRenderableEntity* >( static_cast< tSpatialEntity* >( i ) );
			if( test->fCastsShadow( ) && !test->fDisabled( ) )
			{
				if( test->fComputeFadeAlpha( mScreen ) < 1.f )
					return;

				if( mCameras[0].fGetWorldSpaceFrustum( ).fIntersects( i->mWorldSpaceBox ) )
				{
					mDisplayLists[0].fInsert( test->fGetDrawCall( 0.f ) );

					if( mDisplayLists.fCount( ) > 1 && mCameras[1].fGetWorldSpaceFrustum( ).fIntersects( i->mWorldSpaceBox ) )
					{
						mDisplayLists[1].fInsert( test->fGetDrawCall( 0.f ) );

						if( mDisplayLists.fCount( ) > 2 && mCameras[2].fGetWorldSpaceFrustum( ).fIntersects( i->mWorldSpaceBox ) )
							mDisplayLists[2].fInsert( test->fGetDrawCall( 0.f ) );
					}
				}
			}
		}
	};

	namespace
	{
		struct tShadowMapValues
		{
			f32 mLightDistanceFromOrigin;
			f32 mNearPlane;
			f32 mFarPlane;
			f32 mViewPlaneSize;

			tShadowMapValues( )
			{
				fZeroOut( *this );
			}
			tShadowMapValues( 
				f32 lightDistanceFromOrigin,
				f32 shadowMapNearPlane,
				f32 shadowMapFarPlane,
				f32 shadowMapViewPlaneSize )
				: mLightDistanceFromOrigin( lightDistanceFromOrigin )
				, mNearPlane( shadowMapNearPlane )
				, mFarPlane( shadowMapFarPlane )
				, mViewPlaneSize( shadowMapViewPlaneSize )
			{
			}
		};

		enum tShadowMapFidelity
		{
			cFidelityLoRes,
			cFidelityHiRes0,
			cFidelityHiRes1,
			cFidelityCount
		};

		static tShadowMapValues gShadowMapValues[cFidelityCount]=
		{
			tShadowMapValues( 500.f, 0.f, 1000.f, 500.f ),
			tShadowMapValues( 500.f, 0.f, 1000.f, 100.f ),
			tShadowMapValues( 500.f, 0.f, 1000.f, 20.f ),
		};
	}

	devvar( bool, Renderer_Shadows_DrawLightPosDir, false );

	devvarptr_clamp( f32, Renderer_Shadows_LoRes_Distance, gShadowMapValues[cFidelityLoRes].mLightDistanceFromOrigin, 1.f, 10000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_LoRes_NearPlane, gShadowMapValues[cFidelityLoRes].mNearPlane, 0.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_LoRes_FarPlane, gShadowMapValues[cFidelityLoRes].mFarPlane, 1.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_LoRes_ViewPlaneSize, gShadowMapValues[cFidelityLoRes].mViewPlaneSize, 1.f, 100000.f, 0 );

	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_Distance, gShadowMapValues[cFidelityHiRes0].mLightDistanceFromOrigin, 1.f, 10000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_NearPlane, gShadowMapValues[cFidelityHiRes0].mNearPlane, 0.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_FarPlane, gShadowMapValues[cFidelityHiRes0].mFarPlane, 1.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes0_ViewPlaneSize, gShadowMapValues[cFidelityHiRes0].mViewPlaneSize, 1.f, 100000.f, 0 );

	devvarptr_clamp( f32, Renderer_Shadows_HiRes1_Distance, gShadowMapValues[cFidelityHiRes1].mLightDistanceFromOrigin, 1.f, 10000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes1_NearPlane, gShadowMapValues[cFidelityHiRes1].mNearPlane, 0.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes1_FarPlane, gShadowMapValues[cFidelityHiRes1].mFarPlane, 1.f, 100000.f, 0 );
	devvarptr_clamp( f32, Renderer_Shadows_HiRes1_ViewPlaneSize, gShadowMapValues[cFidelityHiRes1].mViewPlaneSize, 1.f, 100000.f, 0 );

	const u32 tLightEntity::cSpatialSetIndex = tSceneGraph::fNextSpatialSetIndex( );

	void tLightEntity::fSetShadowMapDefaults( f32 dist, f32 nearPlane, f32 farPlane, f32 vpWidth, f32 vpHeight )
	{
		gShadowMapValues[cFidelityLoRes].mLightDistanceFromOrigin = dist;
		gShadowMapValues[cFidelityLoRes].mNearPlane = nearPlane;
		gShadowMapValues[cFidelityLoRes].mFarPlane = farPlane;
		gShadowMapValues[cFidelityLoRes].mViewPlaneSize = fMin( vpWidth, vpHeight );
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

	tLightEntity* tLightEntity::fCreateDefaultLight( const tScreenPtr& screen, const tSceneGraphFile& sgFile )
	{
		if( !sgFile.mDefaultLight )
			return 0;

		tLight desc;
		desc.fSetTypeDirection( );
		desc.fColor( tLight::cColorTypeFront ) = Math::tVec4f( sgFile.mDefaultLight->mFrontColor, 1.f );
		desc.fColor( tLight::cColorTypeBack ) = Math::tVec4f( sgFile.mDefaultLight->mBackColor, 1.f );
		desc.fColor( tLight::cColorTypeRim ) = Math::tVec4f( sgFile.mDefaultLight->mRimColor, 1.f );
		desc.fColor( tLight::cColorTypeAmbient ) = Math::tVec4f( sgFile.mDefaultLight->mAmbientColor, 1.f );

		const Math::tVec3f shadowMapLightDir = Math::tVec3f( sgFile.mDefaultLight->mDirection ).fNormalizeSafe( Math::tVec3f::cZAxis );
		const f32 shadowMapDistFromOrigin = -1.f; // means use default
		const Math::tVec3f shadowMapTarget = Math::tVec3f::cZeroVector; // targets the world origin by default
		const Math::tMat3f m = fCreateDirectionalMatrix( shadowMapLightDir, shadowMapDistFromOrigin, shadowMapTarget );

		tLightEntity* lightEntity = NEW tLightEntity( m, desc, "$DefaultLight" );
		lightEntity->mShadowMapLightDir = shadowMapLightDir;
		lightEntity->mShadowMapDistFromOrigin = shadowMapDistFromOrigin;
		lightEntity->mShadowMapTarget = shadowMapTarget;

		if( sgFile.mDefaultLight->mCastShadow )
			lightEntity->fSetupShadowCasting( screen->fGetDevice( ), screen->fShadowMap0( )->fWidth( ), screen->fShadowMap0( )->fLayerCount( ) );

		return lightEntity;
	}

	tLightEntity::tLightEntity( const Math::tMat3f& objectToWorld, const tLight& light, const char* debugName )
		: mOn( true )
		, mCastsShadow( false )
		, mNumShadowLayers( 1 )
		, pad0( false )
		, mLight( light )
		, mShadowMapResolution( 0 )
		, mShadowMapDistFromOrigin( -1.f )
		, mShadowMapLightDir( -Math::tVec3f::cYAxis )
		, mShadowMapTarget( Math::tVec3f::cZeroVector )
	{
		fUpdateObjectSpaceBox( );
		fMoveTo( objectToWorld );

#ifdef sig_devmenu
		if( debugName )
		{
			std::string devVarNameBase = "Renderer_Lights_";
			devVarNameBase += debugName;
			mDevVarColorAmbient.fReset( NEW tDevVar<Math::tVec3f>( ( devVarNameBase + "_Ambient" ).c_str( ), mLight.fColor3( tLight::cColorTypeAmbient ), 0.f, 2.f, 3, true ) );
			mDevVarColor0.fReset( NEW tDevVar<Math::tVec3f>( ( devVarNameBase + "_Front" ).c_str( ), mLight.fColor3( tLight::cColorTypeFront ), 0.f, 2.f, 3, true ) );
			mDevVarColor1.fReset( NEW tDevVar<Math::tVec3f>( ( devVarNameBase + "_Rim" ).c_str( ), mLight.fColor3( tLight::cColorTypeRim ), 0.f, 2.f, 3, true ) );
			mDevVarColor2.fReset( NEW tDevVar<Math::tVec3f>( ( devVarNameBase + "_Back" ).c_str( ), mLight.fColor3( tLight::cColorTypeBack ), 0.f, 2.f, 3, true ) );
		}
#endif//sig_devmenu
	}

	f32 tLightEntity::fShadowMapCascadeSplit( u32 ithSplit ) const
	{
		if( mNumShadowLayers <= 1 )
			return 0.f;
		return gShadowMapValues[cFidelityHiRes0 + ithSplit].mViewPlaneSize / 2.f;
	}

	void tLightEntity::fSetLightDesc( const tLight& light )
	{
		mLight = light;
		fUpdateObjectSpaceBox( );
	}

	void tLightEntity::fSetRadii( const Math::tVec2f& radii )
	{
		mLight.fRadii( ) = radii;
		fUpdateObjectSpaceBox( );
	}

	void tLightEntity::fSyncBeforeRender( )
	{
#ifdef sig_devmenu
		if( mDevVarColorAmbient )
		{
			mLight.fColor( tLight::cColorTypeAmbient ) = Math::tVec4f( *mDevVarColorAmbient, 1.f );
			mLight.fColor( tLight::cColorTypeFront ) = Math::tVec4f( *mDevVarColor0, 1.f );
			mLight.fColor( tLight::cColorTypeRim ) = Math::tVec4f( *mDevVarColor1, 1.f );
			mLight.fColor( tLight::cColorTypeBack ) = Math::tVec4f( *mDevVarColor2, 1.f );
		}
#endif//sig_devmenu

		// synchronize shadow map camera if we cast shadow
		if( fCastsShadow( ) )
		{
			sigassert( mCameras.fCount( ) == mNumShadowLayers );
			for( u32 i = 0; i < mNumShadowLayers; ++i )
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

				const Math::tVec3f up = fEqual(fAbs(zaxis.fDot(Math::tVec3f::cYAxis)), 1.f) ? Math::tVec3f::cXAxis : Math::tVec3f::cYAxis;
#if defined( target_game )
				float shadowRange = tGameAppBase::fInstance().fGetShadowRange();
				mCameras[ i ].fSetup( 
					tLens( gShadowMapValues[ i ].mNearPlane, fMax<f32>( gShadowMapValues[ i ].mNearPlane + 1.f, gShadowMapValues[ i ].mFarPlane ), shadowRange, shadowRange, tLens::cProjectionOrtho ), 
					tTripod( origin, origin + zaxis, up ) );
#else
				mCameras[i].fSetup(
					tLens( gShadowMapValues[ i ].mNearPlane, fMax<f32>( gShadowMapValues[ i ].mNearPlane + 1.f, gShadowMapValues[ i ].mFarPlane ), gShadowMapValues[ i ].mViewPlaneSize, gShadowMapValues[ i ].mViewPlaneSize, tLens::cProjectionOrtho ), 
					tTripod(origin, origin + zaxis, up));
#endif
				mCameras[ i ].fCorrectForShadowMap( mShadowMapResolution );
			}
		}
	}

	void tLightEntity::fSetupShadowCasting( const tDevicePtr& device, u32 shadowMapResolution, u32 numShadowMapLayers )
	{
//#if !defined( platform_xbox360 )
		// only support cascades on 360 for now (bcz of texture arrays)
		//numShadowMapLayers = 1;
//#endif

		if( mLight.fLightType( ) != tLight::cLightTypeDirection )
		{
			log_warning( Log::cFlagGraphics, "Shadow mapping is currently only supported on directional lights" );
			return;
		}

		mCastsShadow = true;
		mShadowMapResolution = shadowMapResolution;
		mNumShadowLayers = numShadowMapLayers;
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

	void tLightEntity::fRenderShadowMap( tScreen& screen ) const
	{
		sigassert( fSceneGraph( ) );
		sigassert( screen.fShadowMap0( )->fLayerCount( ) == mNumShadowLayers );

		u32 shadowMapCount = fMin( screen.fShadowMap0( )->fLayerCount( ), screen.fLimitShadowMapLayerCount( ) );

		if( shadowMapCount )
		{
			tDynamicArray<tWorldSpaceDisplayList> displayLists( shadowMapCount );

			profile( cProfilePerfRenderShadowMapsQuery );
			fQueryIntersectionForRendering( screen, *fSceneGraph( ), displayLists );

			for( u32 i = 0; i < shadowMapCount; ++i )
			{
				profile( cProfilePerfRenderShadowMapsRender );

				// set shadow map render targets
				screen.fShadowMap0( )->fApply( screen );

				// set clip box (full viewport)
				screen.fShadowMap0( )->fSetClipBox( screen, Math::tVec4f( 0.f, 0.f, 1.f, 1.f ) );

				// clear render targets
				screen.fClearCurrentRenderTargets( true, true, Math::tVec4f(1.f), 1.f, 0 );

				// set default render state for depth rendering
				tRenderContext renderContext;
				renderContext.fFromScreen( screen );
				renderContext.mRenderPassMode = tRenderState::cRenderPassShadowMap;
				renderContext.mCamera = &mCameras[ i ];

				// invalidate render states
				screen.fGetDevice( )->fInvalidateLastRenderState( );

				// render display list, using light camera
				displayLists[ i ].fRenderAll( screen, renderContext );

				// resolve shadow map texture
				screen.fShadowMap0( )->fResolve( screen, NULL, i );
			}
		}
	}

	void tLightEntity::fToShaderConstants( tLightShaderConstants& shaderConstants, const Math::tVec3f& eyePos ) const
	{
		const Math::tMat3f& objToWorld = fObjectToWorld( );
		const f32 lightScale = objToWorld.fGetScale( ).fMax( );
		const f32 directionCoeff = fLightDesc( ).fDirectionalCoefficient( );
		const f32 positionCoeff = fLightDesc( ).fPositionalCoefficient( );

		shaderConstants.mLightDir = Math::tVec4f( -objToWorld.fZAxis( ), directionCoeff );
		shaderConstants.mLightPos = Math::tVec4f(  objToWorld.fGetTranslation( ) - eyePos, positionCoeff );
		shaderConstants.mLightAttenuation = Math::tVec4f( lightScale * fLightDesc( ).fRadii( ).x, lightScale * ( fLightDesc( ).fRadii( ).y - fLightDesc( ).fRadii( ).x ), directionCoeff, positionCoeff );
		shaderConstants.mLightAngles; // TODO
		shaderConstants.mLightAmbient = directionCoeff * fLightDesc( ).fColor( tLight::cColorTypeAmbient );
		shaderConstants.mLightColor0 = fLightDesc( ).fColor( tLight::cColorTypeFront );
		shaderConstants.mLightColor1 = fLightDesc( ).fColor( tLight::cColorTypeRim );
		shaderConstants.mLightColor2 = fLightDesc( ).fColor( tLight::cColorTypeBack );
	}

	void tLightEntity::fQueryIntersectionForRendering( tScreen& screen, const tSceneGraph& sg, tDynamicArray<tWorldSpaceDisplayList>& displayLists ) const
	{
		switch( mLight.fLightType( ) )
		{
		case tLight::cLightTypeDirection:
			{
				tBuildShadowMapDisplayList<Math::tFrustumf> builder( mCameras, displayLists, screen );
				sg.fCollect( builder, tRenderableEntity::cSpatialSetIndex );
				sg.fCollect( builder, tRenderableEntity::cHeightFieldSpatialSetIndex );
				sg.fIntersectCloudRenderables( mCameras.fFront( ).fGetWorldSpaceFrustum( ), builder, true );
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
		case tLight::cLightTypeSpot:
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
		static Math::tVec4f fGetColorRgba( tLightEntity* light )
		{
			return light->fColor( tLight::cColorTypeFront );
		}
		static void fSetColorRgba( tLightEntity* light, const Math::tVec4f& rgba )
		{
			light->fColor( tLight::cColorTypeFront ) = rgba;
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
	}
	void tLightEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tLightEntity, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("Cast"), &fCast)
			.GlobalFunc(_SC("GetRgba"), &fGetColorRgba)
			.GlobalFunc(_SC("SetRgba"), &fSetColorRgba)
			.GlobalFunc(_SC("GetInnerRadius"), &fGetInnerRadius)
			.GlobalFunc(_SC("SetInnerRadius"), &fSetInnerRadius)
			.GlobalFunc(_SC("GetOuterRadius"), &fGetOuterRadius)
			.GlobalFunc(_SC("SetOuterRadius"), &fSetOuterRadius)
			.GlobalFunc(_SC("GetRgba"), &fGetColorRgba)
			.GlobalFunc(_SC("SetRgba"), &fSetColorRgba)
			.Prop(_SC("On"), &tLightEntity::fOn, &tLightEntity::fSetOn)
			;
		vm.fNamespace(_SC("Gfx")).Bind(_SC("LightEntity"), classDesc);
	}
}}
