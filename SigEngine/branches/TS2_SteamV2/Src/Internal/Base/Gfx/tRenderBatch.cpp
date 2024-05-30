#include "BasePch.hpp"
#include "tRenderBatch.hpp"
#include "tGeometryBufferVRam.hpp"
#include "tMaterial.hpp"
#include "tDisplayList.hpp"
#include "Threads/tMutex.hpp"

namespace Sig { namespace Gfx
{
	void tRenderBatchData::fApplyBatch( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& prevBatch ) const
	{
		fApplyBatchWithoutMaterial( device, context, prevBatch );

		sigassert( mMaterial );
		if( prevBatch.mMaterial != mMaterial )
			mMaterial->fApplyShared( device, context, *this );
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
			inline u32 operator( )( const tRenderBatchData* key, const u32 maxSize ) const
			{
				return Hash::fGenericHash( ( Sig::byte* )key, sizeof( *key ), maxSize );
			}
		};

		struct tRenderBatchEqual
		{
			inline u32 operator( )( const tRenderBatchData* a, const tRenderBatchData* b ) const
			{
				return *a == *b;
			}
		};

		typedef tHashTable<
			const tRenderBatchData*,
			tRenderBatch*,
			tHashTableExpandAndShrinkResizePolicy,
			tRenderBatchHash,
			tRenderBatchEqual> tRenderBatchTableBase;

		class tRenderBatchTable : public RenderBatchDetail::tRenderBatchTableBase
		{
			declare_singleton_define_own_ctor_dtor( tRenderBatchTable );
		public:
			tRenderBatchTable( ) : RenderBatchDetail::tRenderBatchTableBase( 32 ) { }
			~tRenderBatchTable( ) { }
		};

		if_devmenu( Threads::tCriticalSection gDebugRenderBatchCritSec; )
	}

	tRenderBatchPtr tRenderBatch::fCreate( const tRenderBatchData& data )
	{
		if_devmenu( Threads::tMutex safe( RenderBatchDetail::gDebugRenderBatchCritSec ); )

		tRenderBatch** find = RenderBatchDetail::tRenderBatchTable::fInstance( ).fFind( &data );
		if( !find )
		{
			tRenderBatch* newBatch = NEW tRenderBatch( data );
			find = RenderBatchDetail::tRenderBatchTable::fInstance( ).fInsert( newBatch, newBatch );
		}

		sigassert( find && *find );
		return tRenderBatchPtr( *find );
	}

	tRenderBatch::tRenderBatch( const tRenderBatchData& data )
		: tRenderBatchData( data )
		, mXparentClone( 0 )
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
		tRenderBatch** find = RenderBatchDetail::tRenderBatchTable::fInstance( ).fFind( this );
		if( find )
			RenderBatchDetail::tRenderBatchTable::fInstance( ).fRemove( find );
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

