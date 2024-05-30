#include "BasePch.hpp"
#include "tRenderBatch.hpp"
#include "tGeometryBufferVRam.hpp"
#include "tMaterial.hpp"
#include "tDisplayList.hpp"
#include "Threads/tMutex.hpp"
#include <map>
#include <list>

namespace Sig { namespace Gfx
{
	void tRenderBatchData::fRenderInstance( const tDevicePtr& device ) const
	{
		if( fBehaviorUseInstancing( ) )
			mIndexBuffer->fRenderInstanced( device, *this );
		else
			mIndexBuffer->fRender( device, *this );
	}
	void tRenderBatchData::fApplyBatch( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& prevBatch ) const
	{
		fApplyBatchWithoutMaterial( device, context, prevBatch );

		sigassert( mMaterial );
		const b32 diffMaterials = prevBatch.mMaterial != mMaterial || prevBatch.mUserFlags != mUserFlags;
		const b32 diffShadow = prevBatch.fBehaviorRecieveShadow( ) != fBehaviorRecieveShadow( );

		if( diffMaterials || diffShadow )
			mMaterial->fApplyShared( device, context, *this );

		if( mGeometryBuffer2 )
			mGeometryBuffer2->fApply( device );
	}
	
	void tRenderBatchData::fApplyBatchWithoutMaterial( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& prevBatch ) const
	{
		sigassert( fValid( ) );

		if( prevBatch.mRenderState != mRenderState )
			mRenderState->fApply( device, context );

		if( prevBatch.mGeometryBuffer != mGeometryBuffer )
			mGeometryBuffer->fApply( device );

		if( !prevBatch.mVertexFormat || prevBatch.mVertexFormat->fPlatformHandle( ) != mVertexFormat->fPlatformHandle( ) )
			mVertexFormat->fApply( device );

		if( prevBatch.mIndexBuffer != mIndexBuffer )
			mIndexBuffer->fApply( device );
	}
}}

namespace Sig { namespace Gfx
{
	devvar( bool, Renderer_Settings_EnableInstancing, false );
	devvar_clamp( u32, Renderer_Settings_MinDrawCallsToInstance, 20, 1, 99999, 0 );

	namespace RenderBatchDetail
	{
		struct tRenderBatchHash
		{
			inline static u32 fHash( const tRenderBatchData* key, const u32 maxSize )
			{
				return Hash::fGenericHash( ( Sig::byte* )key, sizeof( *key ), maxSize );
			}
		};

		struct tRenderBatchEqual
		{
			inline static u32 fEquals( const tRenderBatchData* a, const tRenderBatchData* b )
			{
				return *a == *b;
			}
		};

		typedef std::list< tRenderBatch* > tRenderBatchList;
		typedef std::map< u32, tRenderBatchList > tRenderBatchHashMap;

		class tRenderBatchSet
		{
		public:

			declare_singleton( tRenderBatchSet );

			tRenderBatch* fFindRenderBatch( const tRenderBatchData* key )
			{
				u32 hashKey = RenderBatchDetail::tRenderBatchHash::fHash( key, ~0u);
				tRenderBatchHashMap::iterator batch_chain_iter = mRenderBatches.find( hashKey );
				if( batch_chain_iter == mRenderBatches.end( ) )
					return NULL;

				for( tRenderBatchList::iterator batch_iter = batch_chain_iter->second.begin( ); 
					 batch_iter != batch_chain_iter->second.end( ); batch_iter++ )
				{
					if( *(*batch_iter) == *key )
						return *batch_iter;
				}
				
				return NULL;
			}

			tRenderBatch* fInsertRenderBatch( tRenderBatch* batch )
			{
				sigassert( !fFindRenderBatch( batch ) && "tRenderBatchSet trying to insert a duplicate item." );

				const u32 hashKey = RenderBatchDetail::tRenderBatchHash::fHash( static_cast< const tRenderBatchData* >( batch ), ~0u );
				mRenderBatches[ hashKey ].push_back( batch );
				return batch;
			}

			void fRemoveRenderBatch( tRenderBatch* batch )
			{
				u32 hashKey = RenderBatchDetail::tRenderBatchHash::fHash( batch, ~0u );
				tRenderBatchHashMap::iterator batch_chain_iter = mRenderBatches.find( hashKey );
				if( batch_chain_iter == mRenderBatches.end( ) )
					return;

				for( tRenderBatchList::iterator batch_iter = batch_chain_iter->second.begin( ); 
					batch_iter != batch_chain_iter->second.end( ); batch_iter++ )
				{
					if( *(*batch_iter) == *batch )
					{
						batch_chain_iter->second.erase( batch_iter );
						
						if( batch_chain_iter->second.empty( ) )
							mRenderBatches.erase( batch_chain_iter );
						return;
					}
				}
			}
		private:

			tRenderBatchHashMap mRenderBatches;
		};

		if_devmenu( Threads::tCriticalSection gDebugRenderBatchCritSec; )
	}

	tRenderBatchPtr tRenderBatch::fCreate( const tRenderBatchData& data )
	{
		if_devmenu( Threads::tMutex safe( RenderBatchDetail::gDebugRenderBatchCritSec ); )

		tRenderBatch* find = RenderBatchDetail::tRenderBatchSet::fInstance( ).fFindRenderBatch( &data );
		if( !find )
		{
			tRenderBatch* newBatch = NEW_TYPED( tRenderBatch )( data );
			find = RenderBatchDetail::tRenderBatchSet::fInstance( ).fInsertRenderBatch( newBatch );
		}

		// Create prepass clone
		if( find->mMaterial && find->mMaterial->fRequiresXparentDepthPrepass( ) )
			find->fCreateXparentDepthPrepassClone( );

		sigassert( find );
		return tRenderBatchPtr( find );
	}

	tRenderBatch::tRenderBatch( const tRenderBatchData& data )
		: tRenderBatchData( data )
		, mXparentClone( 0 )
		, mXparentDepthPrepassClone( 0 )
	{
	}

	tRenderBatch::~tRenderBatch( )
	{
		if_devmenu( Threads::tMutex safe( RenderBatchDetail::gDebugRenderBatchCritSec ); )

		if( mXparentClone )
		{
			delete mXparentClone->mRenderState;
			delete mXparentClone;
		}

		if( mXparentDepthPrepassClone )
		{
			delete mXparentDepthPrepassClone->mRenderState;
			delete mXparentDepthPrepassClone;
		}


		RenderBatchDetail::tRenderBatchSet::fInstance( ).fRemoveRenderBatch( this );
	}

	void tRenderBatch::fCreateXparentClone( ) const
	{
		if( !mXparentClone )
		{
			tRenderBatchData clone = fBatchData( );
			tRenderState* rs = NEW tRenderState( *mRenderState );
			rs->fEnableDisable( tRenderState::cAlphaBlend | tRenderState::cCutOut, true );
			rs->fEnableDisable( tRenderState::cDepthWrite, false );
			rs->fSetSrcBlendMode( tRenderState::cBlendSrcAlpha );
			rs->fSetDstBlendMode( tRenderState::cBlendOneMinusSrcAlpha );
			rs->fSetCutOutThreshold( 1 );
			clone.mRenderState = rs;
			mXparentClone = NEW tRenderBatch( clone );
		}
	}

	void tRenderBatch::fCreateXparentDepthPrepassClone( ) const
	{
		if( !mXparentDepthPrepassClone )
		{
			tRenderBatchData clone = fBatchData( );
			tRenderState* rs = NEW_TYPED(tRenderState)( *mRenderState );

			rs->fEnableDisable( tRenderState::cAlphaBlend, false );
			if( rs->fQuery( tRenderState::cCutOut ) && !rs->fGetCutOutThreshold( ) )
				rs->fEnableDisable( tRenderState::cCutOut, false );
			rs->fEnableDisable( tRenderState::cDepthWrite, true );

			clone.mRenderState = rs;
			mXparentDepthPrepassClone = NEW_TYPED(tRenderBatch)( clone );
		}
	}

	u32 tRenderBatch::fRenderInstances( 
		const tDevicePtr& device, 
		const tRenderContext& context, 
		const tDisplayList& dl, 
		u32 startingInstanceIndex, 
		const tRenderBatchData& prevBatch ) const
	{
		fApplyBatch( device, context, prevBatch );

		u32 endInstanceIndex = startingInstanceIndex;
		
		const tLightCombo* firstLightCombo = dl[ startingInstanceIndex ].fLightCombo( );

		for( ; endInstanceIndex < dl.fCount( ) && dl[ endInstanceIndex ].fCanDrawWithBatch( this, firstLightCombo ); ++endInstanceIndex )
			mMaterial->fApplyInstance( device, context, *this, dl[ endInstanceIndex ] );

		return endInstanceIndex;
	}


}}

