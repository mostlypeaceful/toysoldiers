#include "BasePch.hpp"
#include "tPostEffect.hpp"
#include "tScreen.hpp"
#include "tViewport.hpp"
#include "tRenderContext.hpp"

namespace Sig { namespace Gfx
{

	void tPostEffectSettings::fClear( u32 count )
	{
		mSettings.fSetCount( count );
	}

	void tPostEffectSettings::fAddSetting( u32 index, tSettingType type, const Math::tVec4f& value, const Math::tVec4f& min, const Math::tVec4f& max, const char* name, const char** componentNames )
	{
		sigassert( index < mSettings.fCount( ) );
		
		Math::tVec4f& valueStorage = mSettings[ index ].mValue;
		valueStorage = value;

#ifdef sig_devmenu
		tDevMenuItem* var = NULL;

		switch( type )
		{
		case cColor3:
			var = NEW tColorDevVarPtr4( name, valueStorage, min.x, max.z, false );
			break;
		case cColor4:
			var = NEW tColorDevVarPtr4( name, valueStorage, min.x, max.z, true );
			break;
		case cVec3:
			var = NEW tDevVar<Math::tVec4f*>( name, valueStorage, min, max, 2, componentNames, 3 );
			break;
		case cVec4:
			var = NEW tDevVar<Math::tVec4f*>( name, valueStorage, min, max, 2, componentNames, 4 );
			break;
		}

		mSettings[ index ].mDevVar.fReset( var );
#endif
	}

	const Math::tVec4f& tPostEffectSettings::fSetting( u32 setting ) const
	{ 
		return mSettings[ setting ].mValue; 
	}
	
	void tPostEffectSettings::fSetSetting( u32 setting, const Math::tVec4f& value )
	{
		mSettings[ setting ].mValue = value;
	}

	void tPostEffectSettings::fGetState( tSettingState& dest )
	{
		dest.fSetCount( mSettings.fCount( ) );
		for( u32 i = 0; i < mSettings.fCount( ); ++i )
			dest[ i ] = mSettings[ i ].mValue;
	}

	void tPostEffectSettings::fSetState( const tSettingState& state )
	{
		sigassert( state.fCount( ) == mSettings.fCount( ) );
		for( u32 i = 0; i < mSettings.fCount( ); ++i )
			mSettings[ i ].mValue = state[ i ];
	}

	void tPostEffectSettings::fLerped( const tSettingState& a, const tSettingState& b, f32 t, tSettingState& dest )
	{
		sigassert( a.fCount( ) == b.fCount( ) );
		dest.fSetCount( a.fCount( ) );
		for( u32 i = 0; i < a.fCount( ); ++i )
			dest[ i ] = Math::fLerp( a[ i ], b[ i ], t );
	}

	void tPostEffectManager::fCopyTexture( tScreen& screen, const tRenderToTexturePtr& input, const tRenderToTexturePtr& output ) const
	{
		const tDevicePtr& device = screen.fGetDevice( );

		// setup render context for post effects
		tRenderContext renderContext; tRenderState renderState; tRenderBatchData batch;
		fSetupRenderStructuresWithDefaults( screen, renderContext, renderState, batch );

		// set input
		mCopyPassMaterial->fClearInputs( );
		mCopyPassMaterial->fAddInput( input );

		// apply output as current render target
		output->fApply( screen );

		renderContext.mRenderTargetDims = Math::tVec4f( ( f32 )output->fRenderTarget( )->fWidth( ), ( f32 )output->fRenderTarget( )->fHeight( ), 0.f, 0.f );

		// apply render batch
		batch.fApplyBatchWithoutMaterial( device, renderContext );

		// draw full screen quad
		mCopyPassMaterial->fApplyShared( device, renderContext, batch );
	}

	tPostEffectManager::tPostEffectManager( const tResourcePtr& postEffectsMtlFile )
		: mSequences( 16 )
		, mPostEffectsMaterialFile( postEffectsMtlFile )
	{
		// setup camera mapping the rectangle (0,0,1,1) to the full viewport
		tLens lens;
		lens.fSetScreen( 0.0f, 1.f, 0.f, 1.f, 1.f, 0.f );

		mCamera.fSetup( lens, tTripod( Math::tVec3f::cZeroVector, Math::tVec3f::cZAxis, Math::tVec3f::cYAxis ) );

		mCopyPassMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		mCopyPassMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		mCopyPassMaterial->mPS = tPostEffectsMaterial::cPShaderCopy;
	}

	tPostEffectManager::~tPostEffectManager( )
	{
	}

	void tPostEffectManager::fOnTick( f32 dt )
	{
	}

	void tPostEffectManager::fDestroyRenderTargets( )
	{
		mRenderTargets.fSetCount( 0 );
		mSequences.fClear( );
		mSequences.fSetCapacity( 16 );
	}

	void tPostEffectManager::fDetermineFirstAndLastViewportsWithSequence( tScreen& screen, s32& first, s32& last ) const
	{
		const u32 vpCount = screen.fGetViewportCount( );
		first = -1;
		last  = -1;
		for( u32 i = 0; i < vpCount; ++i )
		{
			tViewport& vp = *screen.fViewport( i );

			if( vp.fIsVirtual( ) )
				continue;

			if( mSequences.fFind( vp.fPostEffectSequence( ) ) )
			{
				first = first < 0 ? i : first;
				last  = i;
			}
		}
	}

	void tPostEffectManager::fRender( tScreen& screen, u32 vpIndex, b32 firstViewport, b32 lastViewport ) const
	{
		profile_pix( "tPostEffectManager::fRender" );
		const tViewport& viewport = *screen.fViewport( vpIndex );
		sigassert( !viewport.fIsVirtual( ) );

		// if no post effects, return
		const tPostEffectSequencePtr* findSequence = mSequences.fFind( viewport.fPostEffectSequence( ) );
		if( !findSequence )
			return;
		const tPostEffectSequence& sequence = (**findSequence);

		const tDevicePtr& device = screen.fGetDevice( );

		device->fInvalidateLastRenderState( );

		tRenderContext renderContext; tRenderState renderState; tRenderBatchData batch;
		fSetupRenderStructuresWithDefaults( screen, renderContext, renderState, batch );

		// set viewport specific stuff on render context
		renderContext.mViewportTLBR = viewport.fClipBox( ).fConvertToLTRB( );
		renderContext.mSceneCamera = &viewport.fRenderCamera( );

		const b32 outputRendersToSelf = screen.fScreenSpaceRenderToTexture( )->fCanRenderToSelf( );
		const tRenderToTexturePtr& defaultOutput = 
			outputRendersToSelf ? screen.fScreenSpaceRenderToTexture( ) : screen.fSceneRenderToTexture( );

		// for each post effect
		for( const tPostEffect* i = sequence.mPostEffects.fBegin( ), *iend = sequence.mPostEffects.fEnd( ); i != iend; ++i )
		{
			profile_pix( i->mDebugName );

			const tRenderToTexturePtr& output = i->mOutput ? i->mOutput : defaultOutput;

			if( outputRendersToSelf && !firstViewport && output == defaultOutput )
			{
				// need to copy current contents of default output to itself
				fCopyTexture( screen, output, output );
			}
			else
			{
				// apply current render-to-texture as a render target
				output->fApply( screen );
			}

			// apply clip box
			output->fSetClipBox( screen, renderContext.mViewportTLBR );

			renderContext.mRenderTargetDims = Math::tVec4f( ( f32 )output->fRenderTarget( )->fWidth( ), ( f32 )output->fRenderTarget( )->fHeight( ), 0.f, 0.f );

			// clear render targets
			if( i->mClearRT || i->mClearDT )
				screen.fClearCurrentRenderTargets( i->mClearRT, i->mClearDT, i->mRgbaClearColor, 0x0 );

			// assign current material
			batch.mMaterial = i->mMaterial.fGetRawPtr( );

			// apply render batch
			batch.fApplyBatchWithoutMaterial( device, renderContext );

			// draw full screen quad
			i->mMaterial->fApplyShared( device, renderContext, batch );

			if( lastViewport && ( i + 1 == iend ) )
				break;

			// resolve the render target
			output->fResolve( screen );
		}
	}

	b32 tPostEffectManager::fAddSequence( tScreen& screen, const tStringPtr& seqName, const tPostEffectSequencePtr& sequence )
	{
		tPostEffectSequencePtr* find = mSequences.fFind( seqName );
		if( find ) // sequence with given name has already been added
		{
			log_warning( "Attempting to add Post Effect Sequence [" << seqName << "], but a sequence with that name already exists." );
			return false;
		}

		sigassert( sequence->mPostEffects.fCount( ) > 0 );

		const tRenderToTexturePtr& lastRtt = sequence->mPostEffects.fBack( ).mOutput;
		if( lastRtt )
		{
			sigassert( lastRtt && lastRtt->fRenderTarget( ) );
			sigassert( lastRtt->fRenderTarget( )->fWidth( ) == screen.fSceneRenderToTexture( )->fWidth( ) );
			sigassert( lastRtt->fRenderTarget( )->fHeight( ) == screen.fSceneRenderToTexture( )->fHeight( ) );
		}

		mSequences.fInsert( seqName, sequence );
		return true;
	}

	void tPostEffectManager::fClearSequences( )
	{
		mSequences.fClear( );
	}

	tPostEffectSequencePtr tPostEffectManager::fRemoveSequence( const tStringPtr& seqName )
	{
		tPostEffectSequencePtr* find = mSequences.fFind( seqName );

		tPostEffectSequencePtr o;
		if( find )
		{
			o = *find;
			mSequences.fRemove( find );
		}

		return o;
	}

	void tPostEffectManager::fAddDebugSequences( tScreen& screen )
	{
		const Gfx::tRenderToTexturePtr  nullRtt;
		const Gfx::tRenderToTexturePtr& sceneRtt = screen.fSceneRenderToTexture( );
		const Gfx::tRenderToTexturePtr& altRtt = screen.fScreenSpaceRenderToTexture( ); // abuse this buffer for scratch

		// Setup GBuffer debug sequences
		{
			static const tStringPtr names[] =
			{
				tStringPtr("SigEngine.Debug.GBuffer0"),
				tStringPtr("SigEngine.Debug.GBuffer1"),
				tStringPtr("SigEngine.Debug.GBuffer2"),
				tStringPtr("SigEngine.Debug.GBuffer3"),
			};

			for( u32 i=0; i<array_length(names); ++i )
			{
				Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );
				fAddCopyPass( sequence, screen.fDeferredRenderToTexture( ), i, sceneRtt );
				fAddSequence( screen, names[i], sequence );
			}

			// Specular.  Just remix the gbuffers directly in place?
			{
				Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );
				fAddSwizzlePass( sequence, screen.fDeferredRenderToTexture( ), 0, 0, sceneRtt, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector, Math::tVec4f( 1, 0, 0, 0 ) );
				fAddSequence( screen, tStringPtr("SigEngine.Debug.SpecPower"), sequence );
			}

			{
				Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );
				fAddSwizzlePass( sequence, screen.fDeferredRenderToTexture( ), 1, 0, sceneRtt, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector, Math::tVec4f( 0, 1, 0, 0 ) );
				fAddSequence( screen, tStringPtr("SigEngine.Debug.SpecValue"), sequence );
			}

			// Shadow maps.
			{
				Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );
				fAddSwizzlePass( sequence, screen.fShadowMap( 0 ), 0, 0, sceneRtt, Math::tVec4f::cOnesVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector );
				fAddSequence( screen, tStringPtr("SigEngine.Debug.ShadowMap0.Layer0"), sequence );
			}
			{
				Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );
				fAddSwizzlePass( sequence, screen.fShadowMap( 0 ), 0, 1, sceneRtt, Math::tVec4f::cOnesVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector );
				fAddSequence( screen, tStringPtr("SigEngine.Debug.ShadowMap0.Layer1"), sequence );
			}
			{
				Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );
				fAddSwizzlePass( sequence, screen.fShadowMap( 1 ), 0, 0, sceneRtt, Math::tVec4f::cOnesVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector );
				fAddSequence( screen, tStringPtr("SigEngine.Debug.ShadowMap1.Layer0"), sequence );
			}
			{
				Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );
				fAddSwizzlePass( sequence, screen.fShadowMap( 1 ), 0, 1, sceneRtt, Math::tVec4f::cOnesVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector, Math::tVec4f::cZeroVector );
				fAddSequence( screen, tStringPtr("SigEngine.Debug.ShadowMap1.Layer1"), sequence );
			}

			// TODO:  Depth.
			{
				//Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );
				//fAddCopyPass( sequence, screen.fWorldDepthTexture( ), sceneRtt );
				//fAddSequence( screen, tStringPtr("SigEngine.Debug.Depth"), sequence );
			}
		}
	}

	void tPostEffectManager::fSetupRenderStructuresWithDefaults( tScreen& screen, tRenderContext& renderContext, tRenderState& renderState, tRenderBatchData& batch ) const
	{
		// setup render context for post effects
		renderContext.fFromScreen( screen );
		renderContext.mOtherCamera = &mCamera;

		// setup render state
		renderState = tRenderState::cDefaultColorOpaque;
		renderState.fEnableDisable( tRenderState::cDepthBuffer | tRenderState::cDepthWrite, false ); // no need for depth testing or writes
		renderState.fEnableDisable( tRenderState::cPolyFlipped, true ); //post process quad is backwards, shared convention with deferred shading.

		// setup render batch data descriptor
		batch.mRenderState = &renderState;
		batch.mVertexFormat = &screen.fFullScreenQuadVB( ).fVertexFormat( );
		batch.mGeometryBuffer = &screen.fFullScreenQuadVB( );
		batch.mIndexBuffer = &screen.fFullScreenQuadIB( );
		batch.mVertexCount = screen.fFullScreenQuadVB( ).fVertexCount( );
		batch.mBaseVertexIndex = 0;
		batch.mPrimitiveCount = screen.fFullScreenQuadIB( ).fPrimitiveCount( );
		batch.mBaseIndexIndex = 0;
		batch.mPrimitiveType = screen.fFullScreenQuadIB( ).fIndexFormat( ).mPrimitiveType;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddAddPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input0, 
		const tRenderToTexturePtr& input1, 
		const tRenderToTexturePtr& output,
		const Math::tVec4f& tint0,
		const Math::tVec4f& tint1 )
	{
		tPostEffect pass( "Add" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input0 );
		pass.mMaterial->fAddInput( input1 );
		pass.mMaterial->mRgbaTint0 = tint0;
		pass.mMaterial->mRgbaTint1 = tint1;
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderAdd;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddCopyPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input, 
		const tRenderToTexturePtr& output )
	{
		tPostEffect pass( "Copy" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderCopy;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddCopyPass(
		const tPostEffectSequencePtr&	sequence,
		const tRenderToTexturePtr&		input,
		u32								inputRenderTargetIndex,
		const tRenderToTexturePtr&		output )
	{
		tPostEffect pass( "Copy Specific RT" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( &input->fTexture( inputRenderTargetIndex ), input->fWidth( ), input->fHeight( ) );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderCopy;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddSwizzlePass(
		const tPostEffectSequencePtr&	sequence,
		const tRenderToTexturePtr&		input,
		u32								inputRenderTargetIndex,
		u32								inputRenderTargetLayer,
		const tRenderToTexturePtr&		output,
		const Math::tVec4f&				tintR,
		const Math::tVec4f&				tintG,
		const Math::tVec4f&				tintB,
		const Math::tVec4f&				tintA )
	{
		const u32 layers = fMax( 1u, input->fLayerCount( ) );
		sigassert( inputRenderTargetLayer <= layers );
		const f32 z = ( inputRenderTargetLayer + 0.5f ) / layers;

		tPostEffect pass( "Swizzle Specific RT" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( &input->fTexture( inputRenderTargetIndex ), input->fWidth( ), input->fHeight( ) );
		pass.mMaterial->mRgbaTint0 = tintR;
		pass.mMaterial->mRgbaTint1 = tintG;
		pass.mMaterial->mRgbaTint2 = tintB;
		pass.mMaterial->mRgbaTint3 = tintA;
		pass.mMaterial->mTransformAdd = Math::tVec4f( 0, 0, z, 0 );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = (layers>1) ? tPostEffectsMaterial::cPShaderSwizzle3D : tPostEffectsMaterial::cPShaderSwizzle;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddBlendPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input0, 
		const tRenderToTexturePtr& input1, 
		const tRenderToTexturePtr& output,
		const Math::tVec4f& rgbaBlend )
	{
		tPostEffect pass( "Blend" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input0 );
		pass.mMaterial->fAddInput( input1 );
		pass.mMaterial->mRgbaBlend = rgbaBlend;
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderBlend;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddBlendUsingSource1AlphaPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input0, 
		const tRenderToTexturePtr& input1, 
		const tRenderToTexturePtr& output )
	{
		tPostEffect pass( "BlendSrc1Alpha" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input0 );
		pass.mMaterial->fAddInput( input1 );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderBlendUsingSource1Alpha;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddBlendUsingSource1AlphaPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input0, 
		const tTextureReference& input1,
		u32 width, u32 height,
		const tRenderToTexturePtr& output )
	{
		tPostEffect pass( "BlendSrc1Alpha 2" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input0 );
		pass.mMaterial->fAddInput( &input1, width, height );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderBlendUsingSource1Alpha;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddDownsample2x2Pass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input, 
		const tRenderToTexturePtr& output )
	{
		tPostEffect pass( "Downsample 2x2" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderSample2x2;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	void tPostEffectManager::fAddGaussBlurPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& firstInputLastOutput, 
		const tRenderToTexturePtr& dummyCopy,
		tGrowableArray<tPostEffectsMaterialPtr>* materialsOut )
	{
		tPostEffect pass( "Gauss Blur" );

		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( firstInputLastOutput );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderGaussBlurH;
		pass.mOutput = dummyCopy;
		sequence->mPostEffects.fPushBack( pass );
		if( materialsOut ) materialsOut->fPushBack( sequence->mPostEffects.fBack( ).mMaterial );

		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( dummyCopy );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderGaussBlurV;
		pass.mOutput = firstInputLastOutput;
		sequence->mPostEffects.fPushBack( pass );
		if( materialsOut ) materialsOut->fPushBack( sequence->mPostEffects.fBack( ).mMaterial );
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddDepthOfFieldPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& scene, 
		const tRenderToTexturePtr& blurry, 
		const tRenderToTexturePtr& depth, 
		const tRenderToTexturePtr& output,
		const Math::tVec4f& depthOfFieldMagicNumbers,
		b32 doFog,
		const Math::tVec3f* saturationRgb )
	{
		return fAddDepthOfFieldWithOverlayPass( sequence, tTextureReference( ), 0, 0, scene, blurry, depth, output, depthOfFieldMagicNumbers, doFog, saturationRgb );
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddDepthOfFieldWithOverlayPass( 
		const tPostEffectSequencePtr& sequence, 
		const tTextureReference& overlay, u32 overlayWidth, u32 overlayHeight,
		const tRenderToTexturePtr& scene, 
		const tRenderToTexturePtr& blurry, 
		const tRenderToTexturePtr& depth, 
		const tRenderToTexturePtr& output,
		const Math::tVec4f& depthOfFieldMagicNumbers,
		b32 doFog,
		const Math::tVec3f* saturationRgb )
	{
		const b32 doOverlay = !overlay.fNull( );

		const char* passTitle = saturationRgb ?
			( doOverlay ? "DOF w/ Overlay + Saturate" : "DOF + Saturate" ) :
			( doOverlay ? "DOF w/ Overlay" : "DOF" );

		tPostEffect pass( passTitle );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( scene );
		pass.mMaterial->fAddInput( blurry );
		pass.mMaterial->fAddInput( depth );
		if( doOverlay )
			pass.mMaterial->fAddInput( &overlay, overlayWidth, overlayHeight );
		if( saturationRgb )
			pass.mMaterial->mSaturation = Math::tVec4f( *saturationRgb, 1.f );
		pass.mMaterial->mTargetDepthValues = depthOfFieldMagicNumbers;
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		
		if( saturationRgb )
		{
			if( doOverlay )
				pass.mMaterial->mPS = doFog ? tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlayAndFogAndSaturate : tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlayAndSaturate;
			else
				pass.mMaterial->mPS = doFog ? tPostEffectsMaterial::cPShaderBlendUsingDepthAndFogAndSaturate : tPostEffectsMaterial::cPShaderBlendUsingDepthAndSaturate;
		}
		else
		{
			if( doOverlay )
				pass.mMaterial->mPS = doFog ? tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlayAndFog : tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlay;
			else
				pass.mMaterial->mPS = doFog ? tPostEffectsMaterial::cPShaderBlendUsingDepthAndFog : tPostEffectsMaterial::cPShaderBlendUsingDepth;
		}

		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddSaturationPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input, 
		const tRenderToTexturePtr& output,
		const Math::tVec3f& saturationRgb )
	{
		tPostEffect pass( "Saturation" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input );
		pass.mMaterial->mSaturation = Math::tVec4f( saturationRgb, 1.f );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderSaturation;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	void tPostEffectManager::fConfigureFilmGrainInputs( tPostEffectsMaterial* material,
		const tTextureReference& input1,
		u32 width, u32 height )
	{
		material->fSetInputCount( 1 );
		material->fAddInput( &input1, width, height );
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddFilmGrainPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input0, 
		const tTextureReference& input1,
		u32 width, u32 height,
		const tRenderToTexturePtr& output )
	{
		tPostEffect pass( "FilmGrain" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );

		pass.mMaterial->fAddInput( input0 );
		fConfigureFilmGrainInputs( pass.mMaterial.fGetRawPtr( ), input1, width, height );

		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderFilmGrain;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddTransformPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input, 
		const tRenderToTexturePtr& output,
		const Math::tVec4f& transformAdd,
		const Math::tVec4f& transformMul )
	{
		tPostEffect pass( "Transform" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input );
		pass.mMaterial->mTransformAdd = transformAdd;
		pass.mMaterial->mTransformMul = transformMul;
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderTransform;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddHighPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input, 
		const tRenderToTexturePtr& output )
	{
		tPostEffect pass( "High Pass" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input );
		pass.mMaterial->mTransformCutOff = Math::tVec4f( 1.f );
		pass.mMaterial->mTransformMul = Math::tVec4f( 1.f );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderFilterHighPass;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddFXAAPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& input, 
		const tRenderToTexturePtr& output )
	{
		tPostEffect pass( "FXAA" );
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderFXAA;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

}}
