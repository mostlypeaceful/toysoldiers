#include "BasePch.hpp"
#include "tPostEffect.hpp"
#include "tScreen.hpp"
#include "tViewport.hpp"
#include "tRenderContext.hpp"

namespace Sig { namespace Gfx
{

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
		mCamera.fSetup(
			tLens( 0.0f, 1.f, 0.f, 1.f, 1.f, 0.f, tLens::cProjectionScreen ),
			tTripod( Math::tVec3f::cZeroVector, Math::tVec3f::cZAxis, Math::tVec3f::cYAxis ) );

		mCopyPassMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		mCopyPassMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		mCopyPassMaterial->mPS = tPostEffectsMaterial::cPShaderCopy;
	}

	tPostEffectManager::~tPostEffectManager( )
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

		const b32 outputRendersToSelf = screen.fScreenSpaceRenderToTexture( )->fCanRenderToSelf( );
		const tRenderToTexturePtr& defaultOutput = 
			outputRendersToSelf ? screen.fScreenSpaceRenderToTexture( ) : screen.fSceneRenderToTexture( );

		// for each post effect
		for( const tPostEffect* i = sequence.mPostEffects.fBegin( ), *iend = sequence.mPostEffects.fEnd( ); i != iend; ++i )
		{
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
				screen.fClearCurrentRenderTargets( i->mClearRT, i->mClearDT, i->mRgbaClearColor, i->mDepthClear, 0x0 );

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
			log_warning( Log::cFlagGraphics, "Attempting to add Post Effect Sequence [" << seqName << "], but a sequence with that name already exists." );
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
	void tPostEffectManager::fSetupRenderStructuresWithDefaults( tScreen& screen, tRenderContext& renderContext, tRenderState& renderState, tRenderBatchData& batch ) const
	{
		// setup render context for post effects
		renderContext.fFromScreen( screen );
		renderContext.mCamera = &mCamera;

		// setup render state
		renderState = tRenderState::cDefaultColorOpaque;
		renderState.fEnableDisable( tRenderState::cDepthBuffer | tRenderState::cDepthWrite, false ); // no need for depth testing or writes
		renderState.fEnableDisable( tRenderState::cPolyTwoSided, true );

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
		tPostEffect pass;
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
		tPostEffect pass;
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderCopy;
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
		tPostEffect pass;
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
		tPostEffect pass;
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
		tPostEffect pass;
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
		tPostEffect pass;
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( input );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderSample2x2;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddGaussBlurPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& firstInputLastOutput, 
		const tRenderToTexturePtr& dummyCopy )
	{
		tPostEffect pass;

		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( firstInputLastOutput );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderGaussBlurH;
		pass.mOutput = dummyCopy;
		sequence->mPostEffects.fPushBack( pass );

		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( dummyCopy );
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderGaussBlurV;
		pass.mOutput = firstInputLastOutput;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddDepthOfFieldPass( 
		const tPostEffectSequencePtr& sequence, 
		const tRenderToTexturePtr& scene, 
		const tRenderToTexturePtr& blurry, 
		const tRenderToTexturePtr& depth, 
		const tRenderToTexturePtr& output,
		const Math::tVec4f& depthOfFieldMagicNumbers )
	{
		tPostEffect pass;
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( scene );
		pass.mMaterial->fAddInput( blurry );
		pass.mMaterial->fAddInput( depth );
		pass.mMaterial->mTargetDepthValues = depthOfFieldMagicNumbers;
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderBlendUsingDepth;
		pass.mOutput = output;
		sequence->mPostEffects.fPushBack( pass );
		return sequence->mPostEffects.fBack( ).mMaterial;
	}

	const tPostEffectsMaterialPtr& tPostEffectManager::fAddDepthOfFieldWithOverlayPass( 
		const tPostEffectSequencePtr& sequence, 
		const tTextureReference& overlay, u32 overlayWidth, u32 overlayHeight,
		const tRenderToTexturePtr& scene, 
		const tRenderToTexturePtr& blurry, 
		const tRenderToTexturePtr& depth, 
		const tRenderToTexturePtr& output,
		const Math::tVec4f& depthOfFieldMagicNumbers )
	{
		tPostEffect pass;
		pass.mMaterial.fReset( NEW tPostEffectsMaterial( mPostEffectsMaterialFile ) );
		pass.mMaterial->fAddInput( scene );
		pass.mMaterial->fAddInput( blurry );
		pass.mMaterial->fAddInput( depth );
		pass.mMaterial->fAddInput( &overlay, overlayWidth, overlayHeight );
		pass.mMaterial->mTargetDepthValues = depthOfFieldMagicNumbers;
		pass.mMaterial->mVS = tPostEffectsMaterial::cVShaderOutputUv;
		pass.mMaterial->mPS = tPostEffectsMaterial::cPShaderBlendUsingDepthAndOverlay;
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
		tPostEffect pass;
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
		tPostEffect pass;
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
		tPostEffect pass;
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

}}


