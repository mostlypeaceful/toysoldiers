#include "BasePch.hpp"
#include "tDisplayList.hpp"
#include "tRenderState.hpp"
#include "tScreen.hpp"
#include "tRenderContext.hpp"
#include "tLightCombo.hpp"

namespace Sig { namespace Gfx
{

	namespace
	{
		struct tSortOpaque
		{
			inline b32 operator( )( const tDrawCall& a, const tDrawCall& b ) const
			{
				if( a.fRenderBatch( ) == b.fRenderBatch( ) )
				{
					if( a.fLightCombo( ) == b.fLightCombo( ) )
					{
						// if same batch, then render front to back
						return a.fCameraDepth( ) < b.fCameraDepth( );
					}

					// render by light combo
					return a.fLightCombo( ) < b.fLightCombo( );
				}

				// render by batch order
				return a.fRenderBatch( ) < b.fRenderBatch( );
			}
		};

		struct tSortXparent : public tSortOpaque
		{
			inline b32 operator( )( const tDrawCall& a, const tDrawCall& b ) const
			{
				if( fEqual( a.fCameraDepth( ), b.fCameraDepth( ), 0.01f ) )
				{
					return tSortOpaque::operator( )( a, b );
				}

				// render back to front
				return a.fCameraDepth( ) > b.fCameraDepth( );
			}
		};

		struct tSortScreenSpace
		{
			inline b32 operator( )( const tDrawCall& a, const tDrawCall& b ) const
			{
				// render back to front
				return a.fCameraDepth( ) > b.fCameraDepth( );
			}
		};
	}

	void tDisplayStats::fCombine( const tDisplayStats& other )
	{
		mNumDrawCalls += other.mNumDrawCalls;
		mBatchSwitches += other.mBatchSwitches;
		for( u32 i = 0; i < mPrimitiveCounts.fCount( ); ++i )
			mPrimitiveCounts[ i ] += other.mPrimitiveCounts[ i ];
	}

	void tDisplayList::fRender( tScreen& screen, const tRenderContext& ogContext ) const
	{
		const u32 instanceCount = fCount( );

		const tDevicePtr& device = screen.fGetDevice( );

		// make a copy of the render context
		tRenderContext context = ogContext;

		// configure number of passes and per-pass fill mode
		const b32 doEdgedFaces = ( ogContext.mGlobalFillMode == tRenderState::cGlobalFillEdgedFace );
		const u32 numFillModePasses = ( doEdgedFaces ? 2 : 1 );
		tFixedArray< tRenderState::tGlobalFillMode, 2 > fillModes;
		fillModes[ 0 ] = ( doEdgedFaces ? tRenderState::cGlobalFillSmooth : ogContext.mGlobalFillMode );
		fillModes[ 1 ] = tRenderState::cGlobalFillEdgedFace;

		// use the semi-globally available shared white texture as a default shadow map
		tTextureReference whiteShadowMap;
		whiteShadowMap.fSetRaw( ( tTextureReference::tPlatformHandle )screen.fWhiteTexture( ) );
		whiteShadowMap.fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeClamp );

		for( u32 fillModePass = 0; fillModePass < numFillModePasses; ++fillModePass )
		{
			const tLightCombo*			lastLightComboApplied = 0;

			const tRenderBatchData		nullBatch;
			const tRenderBatchData*		lastBatchApplied = &nullBatch;

			for( u32 currentInstanceIndex = 0, nextInstanceIndex = 0;
					 currentInstanceIndex < instanceCount; 
					 currentInstanceIndex = nextInstanceIndex )
			{
				const tDrawCall& currentDrawCall = mDrawCalls[ currentInstanceIndex ];
				const tRenderBatch* currentBatch = currentDrawCall.fRenderBatch( );
				const b32 ignoreStats = currentBatch->fIgnoreStats( );

				// we don't actually render edged faces for objects whose stats we ignore,
				// i.e., these objects are usually helper objects (lines, debug geometry, dummies, etc.)
				if( fillModePass > 0 && ignoreStats )
				{
					// still need to advance through the instances
					++nextInstanceIndex;
					continue;
				}

				// set current global fill mode
				context.mGlobalFillMode = ignoreStats ? tRenderState::cGlobalFillDefault : fillModes[ fillModePass ];

				const tLightCombo* currentLightCombo = currentDrawCall.fLightCombo( );
				if( currentLightCombo != lastLightComboApplied )
				{
					sigassert( currentLightCombo );
					context.fFromLightGroup( screen, currentLightCombo->mLights, &whiteShadowMap );

					// clear last batch to force batch to get applied
					lastBatchApplied = &nullBatch;
				}

				// apply and render the current batch
				nextInstanceIndex = currentBatch->fRenderInstances( device, context, *this, currentInstanceIndex, *lastBatchApplied );

				// track light combo
				lastLightComboApplied = currentLightCombo;

				// track render batch
				lastBatchApplied = &currentBatch->fBatchData( );

				// accumulate display stats
				if( fillModePass == 0 && !ignoreStats )
				{
					const u32 numDrawCalls = nextInstanceIndex - currentInstanceIndex;
					++mStats.mBatchSwitches;
					mStats.mNumDrawCalls += numDrawCalls;
					mStats.mPrimitiveCounts[ currentBatch->fBatchData( ).mPrimitiveType ] += numDrawCalls * currentBatch->fBatchData( ).mPrimitiveCount;
				}
			}
		}
	}

	void tOpaqueDisplayList::fSeal( )
	{
		std::sort( mDrawCalls.fBegin( ), mDrawCalls.fEnd( ), tSortOpaque( ) );
	}

	void tXparentDisplayList::fSeal( )
	{
		std::sort( mDrawCalls.fBegin( ), mDrawCalls.fEnd( ), tSortXparent( ) );
	}

	void tScreenSpaceDisplayList::fSeal( )
	{
		std::sort( mDrawCalls.fBegin( ), mDrawCalls.fEnd( ), tSortScreenSpace( ) );
	}

	void tWorldSpaceDisplayList::fInsert( const tDrawCall& instance )
	{
		if( instance.fRequiresXparentSort( ) )
			mXparent.fInsert( instance );
		else
			mOpaque.fInsert( instance );
	}

	void tWorldSpaceDisplayList::fInvalidate( )
	{
		mOpaque.fInvalidate( );
		mXparent.fInvalidate( );
	}

	void tWorldSpaceDisplayList::fSeal( )
	{
		mOpaque.fSeal( );
		mXparent.fSeal( );
	}

	void tWorldSpaceDisplayList::fRenderOpaque( tScreen& screen, const tRenderContext& context ) const
	{
		mOpaque.fRender( screen, context );
	}

	void tWorldSpaceDisplayList::fRenderXparent( tScreen& screen, const tRenderContext& context ) const
	{
		mXparent.fRender( screen, context );
	}

	void tWorldSpaceDisplayList::fRenderAll( tScreen& screen, const tRenderContext& context ) const
	{
		fRenderOpaque( screen, context );
		fRenderXparent( screen, context );
	}


}}
