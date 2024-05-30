#include "BasePch.hpp"
#include "tLightCombo.hpp"
#include "tViewport.hpp"
#include "tRenderableEntity.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tLinearFrustumCulling.hpp"
#include "tScreen.hpp"
#include "tGroundCoverCloud.hpp"

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

	struct tBuilderData
	{
		static const u32 cMaxShadowMaps = 8;
		typedef tFixedGrowingArray<tWorldSpaceDisplayList*, cMaxShadowMaps> tShadowMapDisplayLists;

		// Input data
		const tCamera&					mCamera;
		u32								mVpMask;
		const tLightEntityList&			mInputLights;

		tFixedGrowingArray<tLightEntity*,tMaterial::cMaxLights> mLightsOnObject;
		tLightEntityList*				mShadows;
		tShadowMapDisplayLists*			mShadowDisplayLists;

		// output data
		tWorldSpaceDisplayList&			mDisplayList;
		tWorldSpaceDisplayList*			mDepthDisplayList;
		tLightComboList&				mLightCombos;
		u32								mNoLightsComboSlot;

		tBuilderData( const tCamera& camera
			, u32 vpIndex
			, const tLightEntityList& inputLights
			, tLightComboList& lightCombos
			, tWorldSpaceDisplayList& displayList
			, tWorldSpaceDisplayList* depthDl
			, tLightEntityList* shadows
			, tShadowMapDisplayLists* shadowDisplayLists ) 
			: mCamera( camera )
			, mVpMask( 1 << vpIndex )
			, mInputLights( inputLights )
			, mNoLightsComboSlot( lightCombos.fCount( ) )
			, mLightCombos( lightCombos )
			, mDisplayList( displayList )
			, mDepthDisplayList( depthDl )
			, mShadows( shadows )
			, mShadowDisplayLists( shadowDisplayLists )
		{
			// add implicit first slot
			if( mLightCombos.fCount( ) == 0 )
				mLightCombos.fPushBack( NEW tLightCombo );
		}
	};

	///
	/// \brief This scene graph intersection callback performs two actions
	/// 1) collects renderable objects 2) builds the unique light combinations affecting
	/// those renderable objects. The output of this callback is then rendered.

	template< int deferredShading >
	struct tBuilder
	{
		tBuilderData mData;

		tBuilder( const tBuilderData& data )
			: mData( data )
		{ }

		/// not inlined for profiling.
		/*inline*/ void operator()( tLinearFrustumCulling::tUserData s, const tLinearFrustumCulling::tFrustumResultMask& mask, const tLinearFrustumCulling::tFlags& flags )
		{
			tRenderableEntity* renderable = static_cast< tRenderableEntity* >( s );

			if( flags & tRenderableEntity::cFrustumCullFlagsLODed )
				renderable->fUpdateLOD( mData.mCamera.fGetTripod( ).mEye );

			if( mask & (1<<0) )
				operator()( renderable ); //main query

			if( mData.mShadows && (flags & tRenderableEntity::cFrustumCullFlagsShadowCaster) && renderable->fShouldBeRendered(~0u) )
			{
				if( mData.mShadowDisplayLists )
				{
					for( u32 i = 0; i < mData.mShadowDisplayLists->fCount( ); ++i )
					{
						// shadow bits start at 1.
						if( mask & (1<< (i+1)) )
							(*mData.mShadowDisplayLists)[ i ]->fInsert( renderable->fGetDrawCall( 0.f ) );
					}
				}
			}
		}

		inline void operator()( const tRenderableEntity* renderable )
		{
			// by the time we get here we know that we're intersecting.
			if( !renderable->fShouldBeRendered( mData.mVpMask ) )
				return;

			const tRenderBatchPtr& renderBatch = renderable->fRenderBatch( );
			if( renderBatch.fNull( ) || !renderBatch->fBatchData( ).mMaterial )
			{
				//log_warning( "Invalid render batch" );
				return;
			}

			const f32 fadeAlpha = renderable->fComputeFadeAlpha( mData.mCamera );
			if( fadeAlpha > Math::cEpsilon )
			{
				fAddToDepthList( renderable, renderBatch );
				fAddToLightCombo( renderable, renderBatch, fadeAlpha );
			}
		}

		inline void fAddToDepthList( const tRenderableEntity* test, const tRenderBatchPtr& renderBatch )
		{
			if( !mData.mDepthDisplayList )
				return;

			const tMaterial* material = renderBatch->fBatchData( ).mMaterial;
			const b32 rendersDepth = material->fRendersDepth( );
			if( !rendersDepth )
				return;
			if( !renderBatch->fBatchData( ).mRenderState->fQuery( tRenderState::cDepthWrite ) )
				return;

			tDrawCall drawCall = test->fGetDrawCall( mData.mCamera );
			if( !drawCall.fRequiresXparentSort( ) )
				mData.mDepthDisplayList->fInsert( drawCall );
		}

		void fAddToLightCombo( const tRenderableEntity* test, const tRenderBatchPtr& renderBatch, f32 fadeAlpha ) 
		{
			tLightCombo* myCombo = 0;
			tDrawCall drawCall = test->fGetDrawCall( mData.mCamera, fadeAlpha );
			b32 Xparent = drawCall.fRequiresXparentSort( );

			b32 noLightCombo = !renderBatch->fBatchData( ).mMaterial->fIsLit( );

			// skip light combo creation for opaque deferred shading.
			if( deferredShading && !Xparent )
				noLightCombo = true;

			if( noLightCombo )
			{
				myCombo = mData.mLightCombos[ mData.mNoLightsComboSlot ];
			}
			else
			{
				// see which lights the object intersects
				mData.mLightsOnObject.fSetCount( 0 );
				for( u32 i = 0; i < mData.mInputLights.fCount( ) && mData.mLightsOnObject.fCount( ) < mData.mLightsOnObject.fCapacity( ); ++i )
				{
					if( mData.mInputLights[ i ]->fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection ||
						mData.mInputLights[ i ]->fIntersects( test->fWorldSpaceBox( ) ) )
						mData.mLightsOnObject.fPushBack( mData.mInputLights[ i ] );
				}

				// now seek a light combo that's suitable for this object, or else create a new light combo
				for( u32 icombo = 0; icombo < mData.mLightCombos.fCount( ); ++icombo )
				{
					tLightCombo* combo = mData.mLightCombos[ icombo ];
					if( fSuitable( *combo, mData.mLightsOnObject ) )
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
					mData.mLightCombos.fPushBack( myCombo );
					myCombo->mLights.fSetCount( mData.mLightsOnObject.fCount( ) );
					for( u32 i = 0; i < mData.mLightsOnObject.fCount( ); ++i )
						myCombo->mLights[ i ] = mData.mLightsOnObject[ i ];
				}
			}

			drawCall.fSetLightCombo( myCombo );
			mData.mDisplayList.fInsert( drawCall, Xparent );
		}


		static b32 fSuitable( tLightCombo& combo, const tFixedGrowingArray<tLightEntity*,tMaterial::cMaxLights>& forList )
		{
			// must have same number of lights to be an equivalent combo
			if( combo.mLights.fCount( ) != forList.fCount( ) )
				return false;

			// if same number of lights, then each light must be the same
			for( u32 i = 0; i < forList.fCount( ); ++i )
			{
				if( !combo.mLights.fFind( forList[ i ] ) )
					return false;
			}

			return true;
		}
	};



	tLightComboList::tLightComboList( )
	{
	}

	tLightComboList::~tLightComboList( )
	{
		fClearCombos( );
	}

	void tLightComboList::fBuildLightCombos( tScreen& screen, tSceneGraph& sg, const tViewport& vp, b32 deferred, const tLightEntityList& lightList, tWorldSpaceDisplayList& depthDisplayList, b32 buildDepthList, tLightEntityList* shadows )
	{
		mDisplayList.fInvalidate( );

		tLinearFrustumCulling::tFrustumArray frusts;
		frusts.fPushBack( vp.fRenderCamera( ).fGetWorldSpaceFrustum( ) );

		tBuilderData::tShadowMapDisplayLists shadowDLs;

		if( shadows )
		{
			// Reset the shadow map display lists for all shadow casting lights
			for( u32 i = 0; i < shadows->fCount( ); ++i )
			{
				tLightEntity* le = (*shadows)[ i ];
				le->fResetShadowMaps( screen );

				// If the light can be queried through the frustum then grab it's display lists
				// NOTE: Lights that cannot be queried through the frustum have their shadow maps built
				// after the frustum culling so that all lods are guaranteed to be selected and the 
				// renderbatch pointers stabalized
				if( le->mShadowDisplayLists.fCount( ) && le->fCanQueryWithFrustum( ) )
				{
					for( u32 f = 0; f < le->mCameras.fCount( ) && frusts.fCount( ) < frusts.fCapacity( ); ++f )
					{
						shadowDLs.fPushBack( &le->mShadowDisplayLists[ f ] );
						frusts.fPushBack( le->mCameras[ f ].fGetWorldSpaceFrustum( ) );
					}
				}
			}
		}

		// Should we build display lists?
		if( screen.fGetBuildDisplayLists( ) )
		{
			sigassert( sg.fRenderCulling( ) );

			tBuilderData data( vp.fRenderCamera( ), vp.fViewportIndex( ), lightList, *this, mDisplayList, buildDepthList ? &depthDisplayList : 0, shadows, shadows ? &shadowDLs : NULL );

			if( deferred )
			{
				tBuilder< 1 > builder( data );
				sg.fRenderCulling( )->fIntersect( frusts, builder );

				//append all the entity clouds (like ground cover) to our render lists
				if( tEntityCloud::fInstanced( ) )
				{
					const tSceneGraph::tCloudList& cl = sg.fCloudList( );
					const u32 clCount = cl.fCount( );
					for( u32 i = 0; i < clCount; ++i )
					{
						if( cl[ i ]->fShouldRender( ) )
						{
							for( u32 j = 0; j < cl[ i ]->mInstancedRenderables.fCount( ); ++j )
							{
								const tRenderableEntity* re = cl[ i ]->mInstancedRenderables[ j ];
								builder( re );
							}
						}
					}
				}
				else
				{
					const tSceneGraph::tCloudList& cl = sg.fCloudList( );
					const u32 clCount = cl.fCount( );
					for( u32 i = 0; i < clCount; ++i )
					{
						if( cl[ i ]->fShouldRender( ) )
							cl[ i ]->fCuller( ).fIntersect( frusts, builder );
					}
				}
			}
			else
			{
				tBuilder< 0 > builder( data );
				sg.fRenderCulling( )->fIntersect( frusts, builder );

				//append all the entity clouds (like ground cover) to our render lists
				if( tEntityCloud::fInstanced( ) )
				{
					const tSceneGraph::tCloudList& cl = sg.fCloudList( );
					const u32 clCount = cl.fCount( );
					for( u32 i = 0; i < clCount; ++i )
					{
						if( cl[ i ]->fShouldRender( ) )
						{
							for( u32 j = 0; j < cl[ i ]->mInstancedRenderables.fCount( ); ++j )
							{
								const tRenderableEntity* re = cl[ i ]->mInstancedRenderables[ j ];
								builder( re );
							}
						}
					}
				}
				else
				{
					const tSceneGraph::tCloudList& cl = sg.fCloudList( );
					const u32 clCount = cl.fCount( );
					for( u32 i = 0; i < clCount; ++i )
					{
						if( cl[ i ]->fShouldRender( ) )
							cl[ i ]->fCuller( ).fIntersect( frusts, builder );
					}
				}
			}

			// Now that all the lods have been updated by the frustum culler
			// we can gather draw calls for the non-frustum query lights
			if( shadows )
			{
				for( u32 i = 0; i < shadows->fCount( ); ++i )
				{
					tLightEntity* le = (*shadows)[ i ];
					if( !le->fCanQueryWithFrustum( ) )
						le->fBuildShadowDisplayLists( screen );
				}
			}
			
			mDisplayList.fSeal( );
			if( buildDepthList )
				depthDisplayList.fSeal( );
		}
	}

	void tLightComboList::fInvalidate( )
	{
		fClearCombos( );
		mDisplayList.fInvalidate( );
	}

	void tLightComboList::fClearCombos( )
	{
		for( u32 i = 0; i < fCount( ); ++i )
			delete fIndex( i );

		fSetCount( 0 );
	}


}}
