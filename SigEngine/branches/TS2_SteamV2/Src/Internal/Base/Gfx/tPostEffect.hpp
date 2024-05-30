#ifndef __tPostEffect__
#define __tPostEffect__
#include "tCamera.hpp"
#include "tRenderToTexture.hpp"
#include "tPostEffectsMaterial.hpp"

namespace Sig { namespace Gfx
{
	class tScreen;
	class tViewport;
	class tRenderBatchData;

	struct base_export tPostEffect
	{
		tPostEffectsMaterialPtr		mMaterial;
		tRenderToTexturePtr			mOutput;
		b16							mClearRT, mClearDT;
		Math::tVec4f				mRgbaClearColor;
		f32							mDepthClear;

		tPostEffect( ) : mClearRT( false ), mClearDT( false ), mRgbaClearColor( 0.f ), mDepthClear( 1.f ) { }
	};

	typedef tGrowableArray< tPostEffect >			tPostEffectArray;
	typedef tGrowableArray< tRenderToTexturePtr >	tRenderTargetArray;

	struct base_export tPostEffectSequence : public tRefCounter
	{
		tPostEffectArray mPostEffects;
	};

	typedef tRefCounterPtr< tPostEffectSequence > tPostEffectSequencePtr;

	typedef tHashTable< tStringPtr, tPostEffectSequencePtr > tPostEffectSequenceMap;

	///
	/// \brief Manages a set of shared post effect render targets, as well as
	/// a set of named post effect sequences.
	class base_export tPostEffectManager : public tRefCounter
	{
		tCamera						mCamera;
		mutable tPostEffectsMaterialPtr mCopyPassMaterial;
		tRenderTargetArray			mRenderTargets;
		tPostEffectSequenceMap		mSequences;

	protected:
		tResourcePtr				mPostEffectsMaterialFile;

	public:

		void fCopyTexture( tScreen& screen, const tRenderToTexturePtr& input, const tRenderToTexturePtr& output ) const;

	public:

		explicit tPostEffectManager( const tResourcePtr& postEffectsMtlFile );
		virtual ~tPostEffectManager( );

		///
		/// \brief You should override this and release any resources your derived object is holding on to.
		/// Be sure to call down to the base class version.
		virtual void fDestroyRenderTargets( );

		///
		/// \brief You should do all your render target and sequence creation in this method,
		/// it will get called automatically after the frame buffer is created/resized.
		virtual void fCreateRenderTargets( tScreen& screen ) { }

		///
		/// \brief How many sequences have been inserted?
		u32 fSequenceCount( ) const { return mSequences.fGetItemCount( ); }

		///
		/// \brief Access the post effects material file.
		const tResourcePtr& fPostEffectsMaterialFile( ) const { return mPostEffectsMaterialFile; }

		///
		/// \brief Query to see if any viewports have active post effect sequences.
		void fDetermineFirstAndLastViewportsWithSequence( tScreen& screen, s32& first, s32& last ) const;

		///
		/// \brief Renders the viewport's named post effects sequence, if any.
		/// Called by tScreen from within the render loop, you shouldn't be calling this method.
		void fRender( tScreen& screen, u32 vpIndex, b32 firstViewport, b32 lastViewport ) const;

		///
		/// \brief Add a shared render to texture.
		void fAddRenderTarget( const tRenderToTexturePtr& rtt ) { mRenderTargets.fPushBack( rtt ); }

		///
		/// \brief Add a new named post effect sequence; returns false if there was already
		/// a sequence with the specified name, in which case the sequence is not added.
		b32 fAddSequence( tScreen& screen, const tStringPtr& seqName, const tPostEffectSequencePtr& sequence );

		///
		/// \brief Remove and return the sequence with the given name; if the sequence is not
		/// found, a null smart pointer is returned.
		tPostEffectSequencePtr fRemoveSequence( const tStringPtr& seqName );

	protected:
		void fSetupRenderStructuresWithDefaults( tScreen& screen, tRenderContext& renderContext, tRenderState& renderState, tRenderBatchData& batch ) const;
		const tPostEffectsMaterialPtr& fAddAddPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input0, 
			const tRenderToTexturePtr& input1, 
			const tRenderToTexturePtr& output,
			const Math::tVec4f& tint0 = Math::tVec4f::cOnesVector,
			const Math::tVec4f& tint1 = Math::tVec4f::cOnesVector );
		const tPostEffectsMaterialPtr& fAddCopyPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input, 
			const tRenderToTexturePtr& output );
		const tPostEffectsMaterialPtr& fAddBlendPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input0, 
			const tRenderToTexturePtr& input1, 
			const tRenderToTexturePtr& output,
			const Math::tVec4f& rgbaBlend );
		const tPostEffectsMaterialPtr& fAddBlendUsingSource1AlphaPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input0, 
			const tRenderToTexturePtr& input1, 
			const tRenderToTexturePtr& output );
		const tPostEffectsMaterialPtr& fAddBlendUsingSource1AlphaPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input0, 
			const tTextureReference& input1,
			u32 width, u32 height,
			const tRenderToTexturePtr& output );
		const tPostEffectsMaterialPtr& fAddDownsample2x2Pass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input, 
			const tRenderToTexturePtr& output );
		const tPostEffectsMaterialPtr& fAddGaussBlurPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& firstInputLastOutput, 
			const tRenderToTexturePtr& dummyCopy );
		const tPostEffectsMaterialPtr& fAddDepthOfFieldPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& scene, 
			const tRenderToTexturePtr& blurry, 
			const tRenderToTexturePtr& depth, 
			const tRenderToTexturePtr& output,
			const Math::tVec4f& depthOfFieldMagicNumbers );
		const tPostEffectsMaterialPtr& fAddDepthOfFieldWithOverlayPass( 
			const tPostEffectSequencePtr& sequence, 
			const tTextureReference& overlay, u32 overlayWidth, u32 overlayHeight,
			const tRenderToTexturePtr& scene, 
			const tRenderToTexturePtr& blurry, 
			const tRenderToTexturePtr& depth, 
			const tRenderToTexturePtr& output,
			const Math::tVec4f& depthOfFieldMagicNumbers );
		const tPostEffectsMaterialPtr& fAddSaturationPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input, 
			const tRenderToTexturePtr& output,
			const Math::tVec3f& saturationRgb );
		void fConfigureFilmGrainInputs( tPostEffectsMaterial* material,
			const tTextureReference& input1,
			u32 width, u32 height );
		const tPostEffectsMaterialPtr& fAddFilmGrainPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input0, 
			const tTextureReference& input1,
			u32 width, u32 height,
			const tRenderToTexturePtr& output );
		const tPostEffectsMaterialPtr& fAddTransformPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input, 
			const tRenderToTexturePtr& output,
			const Math::tVec4f& transformAdd,
			const Math::tVec4f& transformMul );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tPostEffectManager );

}}

#endif//__tPostEffect__

