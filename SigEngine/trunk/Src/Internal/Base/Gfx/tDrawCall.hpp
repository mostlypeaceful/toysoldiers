#ifndef __tDrawCall__
#define __tDrawCall__
#include "tRenderInstance.hpp"

namespace Sig { namespace Gfx
{
	class tLightCombo;

	///
	/// \brief Represents a single draw call.
	class base_export tDrawCall
	{
	private:
		static const tLightCombo* cNullLightCombo;

	private:
		const tRenderInstance*	mRenderInstance;
		const tRenderBatch*		mRenderBatch;
		const tLightCombo*		mLightCombo;
		f32						mCameraDepth;
		f32						mFadeAlpha;

	public:

		inline tDrawCall( ) 
			: mRenderInstance( 0 )
			, mRenderBatch( 0 )
			, mLightCombo( cNullLightCombo )
			, mCameraDepth( 0.f )
			, mFadeAlpha( 1.f )
		{
		}
		
		inline tDrawCall( const tRenderInstance& ri, f32 camDepth, f32 fadeAlpha = 1.f ) 
			: mRenderInstance( &ri )
			, mRenderBatch( ri.fRenderBatch( ).fGetRawPtr( ) )
			, mLightCombo( cNullLightCombo )
			, mCameraDepth( camDepth )
			, mFadeAlpha( fadeAlpha )
		{
			if( mRenderBatch && ri.fRequiresXparentSort( mFadeAlpha ) && !ri.fBatchHasXparency( ) )
			{
				mRenderBatch->fCreateXparentClone( );
				mRenderBatch = mRenderBatch->fXparentClone( );
			}
		}

		inline void fSetLightCombo( const tLightCombo* lightCombo )
		{
			mLightCombo = lightCombo;
		}

		inline const tLightCombo* fLightCombo( ) const
		{
			return mLightCombo;
		}

		inline f32 fCameraDepth( ) const
		{
			return mCameraDepth;
		}

		inline f32 fFadeAlpha( ) const
		{
			return mFadeAlpha;
		}

		inline Math::tVec4f fInstanceRgbaTint( ) const
		{
			return mRenderInstance->fRgbaTint( mFadeAlpha );
		}

		inline const tRenderBatch* fRenderBatch( ) const
		{
			return mRenderBatch;
		}

		inline const tRenderInstance& fRenderInstance( ) const
		{
			return *mRenderInstance;
		}

		inline b32 fRequiresXparentSort( ) const
		{
			return mRenderInstance->fRequiresXparentSort( mFadeAlpha ) || ( mRenderBatch && mRenderBatch->fBatchData( ).mRenderState->fHasTransparency( ) );
		}

		inline b32 fRequiresXparentDepthPrepass( ) const
		{
			return mRenderBatch && mRenderBatch->fRequiresXparentDepthPrepass( );
		}

		inline b32 fValid( ) const
		{
			return mRenderBatch != 0;
		}

		inline b32 fUsesBatch( const tRenderBatch* testBatch ) const
		{
			return mRenderBatch == testBatch;
		}
		
		inline b32 fCanDrawWithBatch( const tRenderBatch* testBatch, const tLightCombo* lightCombo ) const
		{
			return fUsesBatch( testBatch ) && mLightCombo == lightCombo;
		}

		inline tDrawCall fCloneForXparentDepthPrepass( ) const
		{
			sigassert( fRequiresXparentDepthPrepass( ) );

			tDrawCall call = *this;
			call.mRenderBatch = call.mRenderBatch->fXparentDepthPrepassClone( );
			return call;
		}
	};

}}

#endif//__tDrawCall__
