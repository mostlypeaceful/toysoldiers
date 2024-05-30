#include "BasePch.hpp"
#include "tLightCombo.hpp"
#include "tViewport.hpp"
#include "tRenderableEntity.hpp"
#include "tSceneGraphCollectTris.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		define_static_function( fEnsureLightComboCriticalSectionInitialized )
		{
			tLightCombo::fAllocClassPoolPage( );
			tLightCombo::fClassPoolCriticalSection( );
		}
	}

	///
	/// \brief This scene graph intersection callback performs two actions at once, as it traverses
	/// the scene graph: 1) collects renderable objects 2) builds the unique light combinations affecting
	/// those renderable objects. The output of this callback is then rendered.
	struct tLightComboBuilder : public tEntityBVH::tIntersectVolumeCallback<Math::tFrustumf>
	{
		const tCamera&					mCamera;
		u32								mVpIndex;
		const tLightEntityList&			mInputLights;
		mutable u32						mNoLightsComboSlot;
		tLightComboList&		        mLightCombos;
		tWorldSpaceDisplayList&         mDisplayList;
		mutable tWorldSpaceDisplayList*	mDepthDisplayList;
		mutable tFixedArray<tLightEntity*,tMaterial::cMaxLights> mLightsOnObject;

		tLightComboBuilder( const tCamera&	camera, u32 vpIndex, const tLightEntityList& inputLights, tLightComboList& lightCombos, tWorldSpaceDisplayList& displayList, tWorldSpaceDisplayList* depthDl ) 
			: mCamera( camera ), mVpIndex( vpIndex ), mInputLights( inputLights ), mNoLightsComboSlot( lightCombos.fCount( ) ), mLightCombos( lightCombos ), mDisplayList( displayList ), mDepthDisplayList( depthDl )
		{
			// add implicit first slot for "no lights"
			if( mLightCombos.fCount( ) == 0 )
				mLightCombos.fPushBack( NEW tLightCombo );
		}

		inline void fAddToDepthList( tRenderableEntity* test, const tRenderBatchPtr& renderBatch ) const
		{
			if( !mDepthDisplayList )
				return;

			const tMaterial* material = renderBatch->fBatchData( ).mMaterial;
			const b32 rendersDepth = material->fRendersDepth( );
			if( !rendersDepth )
				return;
			if( !renderBatch->fBatchData( ).mRenderState->fQuery( tRenderState::cDepthWrite ) )
				return;

			tDrawCall drawCall = test->fGetDrawCall( mCamera );
			mDepthDisplayList->fInsert( drawCall );
		}

		void fAddToLightCombo( tRenderableEntity* test, const tRenderBatchPtr& renderBatch, f32 fadeAlpha ) const
		{
			tLightCombo* myCombo = 0;

			const b32 isLit = renderBatch->fBatchData( ).mMaterial->fIsLit( );
			if( !isLit )
			{
				myCombo = mLightCombos[ mNoLightsComboSlot ];
			}
			else
			{
				// see which lights the object intersects
				u32 numLightsOnObject = 0;
				for( u32 i = 0; i < mInputLights.fCount( ) && numLightsOnObject < mLightsOnObject.fCount( ); ++i )
				{
					if( mInputLights[ i ]->fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection ||
						mInputLights[ i ]->fIntersects( test->fWorldSpaceBox( ) ) )
						mLightsOnObject[ numLightsOnObject++ ] = mInputLights[ i ];
				}

				// now seek a light combo that's suitable for this object, or else create a new light combo
				for( u32 icombo = 0; icombo < mLightCombos.fCount( ); ++icombo )
				{
					tLightCombo* combo = mLightCombos[ icombo ];

					// must have same number of lights to be an equivalent combo
					if( combo->mLights.fCount( ) != numLightsOnObject )
						continue;

					// if same number of lights, then each light must be the same
					b32 found = true;
					for( u32 i = 0; i < numLightsOnObject; ++i )
					{
						if( !combo->mLights.fFind( mLightsOnObject[ i ] ) )
						{
							found = false;
							break;
						}
					}

					if( found )
					{
						// this light combo works for me
						myCombo = combo;
						break;
					}
				}

				// if we didn't find a light combo, then we create a new one now
				if( !myCombo )
				{
					myCombo = NEW tLightCombo;
					mLightCombos.fPushBack( myCombo );
					myCombo->mLights.fSetCount( numLightsOnObject );
					for( u32 i = 0; i < numLightsOnObject; ++i )
						myCombo->mLights[ i ] = mLightsOnObject[ i ];
				}
			}

			tDrawCall drawCall = test->fGetDrawCall( mCamera, fadeAlpha );
			drawCall.fSetLightCombo( myCombo );
			mDisplayList.fInsert( drawCall );
		}

		inline void operator()( const Math::tFrustumf& v, tEntityBVH::tObjectPtr i, b32 aabbWhollyContained ) const
		{
			tRenderableEntity* renderable = static_cast< tRenderableEntity* >( i );
			if( !renderable->fShouldBeRendered( mVpIndex ) )
				return;

			if( !fQuickAabbTest( v, i, aabbWhollyContained ) )
				return;

			const tRenderBatchPtr& renderBatch = renderable->fRenderBatch( );
			if( renderBatch.fNull( ) || !renderBatch->fBatchData( ).mMaterial )
			{
				//log_warning( Log::cFlagGraphics, "Invalid render batch" );
				return;
			}

			const f32 fadeAlpha = renderable->fComputeFadeAlpha( mCamera );
			if( fadeAlpha > 0.f )
			{
				fAddToDepthList( renderable, renderBatch );
				fAddToLightCombo( renderable, renderBatch, fadeAlpha );
			}
		}
	};

	tLightComboList::~tLightComboList( )
	{
		for( u32 i = 0; i < fCount( ); ++i )
			delete fIndex( i );
	}

	void tLightComboList::fBuildLightCombosMT( tSceneGraph& sg, const tViewport& vp, const tLightEntityList& lightList, tWorldSpaceDisplayList& depthDisplayList, b32 buildDepthList )
	{
		tLightComboBuilder builder( vp.fRenderCamera( ), vp.fViewportIndex( ), lightList, *this, mDisplayList, buildDepthList ? &depthDisplayList : 0 );

		sg.fIntersect( vp.fRenderCamera( ).fGetWorldSpaceFrustum( ), builder, tRenderableEntity::cSpatialSetIndex );
		sg.fIntersect( vp.fRenderCamera( ).fGetWorldSpaceFrustum( ), builder, tRenderableEntity::cHeightFieldSpatialSetIndex );
		sg.fIntersect( vp.fRenderCamera( ).fGetWorldSpaceFrustum( ), builder, tRenderableEntity::cEffectSpatialSetIndex );
		sg.fIntersectCloudRenderables( vp.fRenderCamera( ).fGetWorldSpaceFrustum( ), builder, false );

		mDisplayList.fSeal( );
		if( buildDepthList )
			depthDisplayList.fSeal( );
	}


}}
