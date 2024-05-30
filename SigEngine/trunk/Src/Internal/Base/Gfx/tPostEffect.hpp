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
		const char*					mDebugName;

		tPostEffect( const char* debugName = "" ) 
			: mClearRT( false )
			, mClearDT( false )
			, mRgbaClearColor( 0.f )
			, mDebugName( debugName )
		{ }
	};

	///
	/// \brief: Data container for the game to use to manage its settings.
	struct tPostEffectSettings
	{
		enum tSettingType
		{
			cColor3,
			cColor4,
			cVec3,
			cVec4
		};

		typedef tGrowableArray< Math::tVec4f > tSettingState;

		void fClear( u32 count );
		void fAddSetting( u32 index, tSettingType type, const Math::tVec4f& value, const Math::tVec4f& min, const Math::tVec4f& max, const char* name, const char** componentNames = NULL );

		const Math::tVec4f& fSetting( u32 setting) const;
		void fSetSetting( u32 setting, const Math::tVec4f& value );

		void fGetState( tSettingState& dest );
		void fSetState( const tSettingState& state );

		static void fLerped( const tSettingState& a, const tSettingState& b, f32 t, tSettingState& dest );

	private:
		struct tSetting
		{
			Math::tVec4f	mValue;
			if_devmenu( tDevVarPtr mDevVar; )
		};

		tGrowableArray< tSetting > mSettings;
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
	public:
		void fCopyTexture( tScreen& screen, const tRenderToTexturePtr& input, const tRenderToTexturePtr& output ) const;

	public:
		explicit tPostEffectManager( const tResourcePtr& postEffectsMtlFile );
		virtual ~tPostEffectManager( );

		virtual void fOnTick( f32 dt );

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

		///
		/// \brief Remove all sequences
		void fClearSequences( );

	protected:
		void fAddDebugSequences( tScreen& screen );
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

		const tPostEffectsMaterialPtr& fAddCopyPass(
			const tPostEffectSequencePtr&	sequence,
			const tRenderToTexturePtr&		input,
			u32								inputRenderTargetIndex,
			const tRenderToTexturePtr&		output );

		const tPostEffectsMaterialPtr& fAddSwizzlePass(
			const tPostEffectSequencePtr&	sequence,
			const tRenderToTexturePtr&		input,
			u32								inputRenderTargetIndex,
			u32								inputRenderTargetLayer,
			const tRenderToTexturePtr&		output,
			const Math::tVec4f&				tintR = Math::tVec4f( 1, 0, 0, 0 ),
			const Math::tVec4f&				tintG = Math::tVec4f( 0, 1, 0, 0 ),
			const Math::tVec4f&				tintB = Math::tVec4f( 0, 0, 1, 0 ),
			const Math::tVec4f&				tintA = Math::tVec4f( 0, 0, 0, 1 ) );

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
		void							fAddGaussBlurPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& firstInputLastOutput, 
			const tRenderToTexturePtr& dummyCopy,
			tGrowableArray<tPostEffectsMaterialPtr>* materialsOut = NULL );
		const tPostEffectsMaterialPtr& fAddDepthOfFieldPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& scene, 
			const tRenderToTexturePtr& blurry, 
			const tRenderToTexturePtr& depth, 
			const tRenderToTexturePtr& output,
			const Math::tVec4f& depthOfFieldMagicNumbers,
			b32 doFog,
			const Math::tVec3f* saturationRgb = NULL );
		const tPostEffectsMaterialPtr& fAddDepthOfFieldWithOverlayPass( 
			const tPostEffectSequencePtr& sequence, 
			const tTextureReference& overlay, u32 overlayWidth, u32 overlayHeight,
			const tRenderToTexturePtr& scene, 
			const tRenderToTexturePtr& blurry, 
			const tRenderToTexturePtr& depth, 
			const tRenderToTexturePtr& output,
			const Math::tVec4f& depthOfFieldMagicNumbers,
			b32 doFog,
			const Math::tVec3f* saturationRgb = NULL );
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
		const tPostEffectsMaterialPtr& fAddHighPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input, 
			const tRenderToTexturePtr& output );
		const tPostEffectsMaterialPtr& fAddFXAAPass( 
			const tPostEffectSequencePtr& sequence, 
			const tRenderToTexturePtr& input, 
			const tRenderToTexturePtr& output );

	private:
		tCamera						mCamera;
		tPostEffectsMaterialPtr		mCopyPassMaterial;
		tRenderTargetArray			mRenderTargets;
		tPostEffectSequenceMap		mSequences;

	protected:
		tResourcePtr				mPostEffectsMaterialFile;

	};

	define_smart_ptr( base_export, tRefCounterPtr, tPostEffectManager );

}}

#endif//__tPostEffect__

