#ifndef __tRenderBatch__
#define __tRenderBatch__
#include "tRenderState.hpp"
#include "Memory/tPool.hpp"
#include "tIndexBufferVRam.hpp"

namespace Sig { namespace Gfx
{
	class tDevicePtr;
	class tRenderState;
	class tMaterial;
	class tVertexFormatVRam;
	class tGeometryBufferVRam;
	class tIndexBufferVRam;
	class tRenderContext;
	class tRenderBatch;
	class tRenderInstance;
	class tDisplayList;

	define_smart_ptr( base_export, tRefCounterPtr, tRenderBatch );

	///
	/// \brief Encapsulates the minimal data that must be shared/identical between
	/// render instances to be considered a "batch". This data is set once prior
	/// to rendering all instances in a batch, thus minimizing state changes between objects.
	class base_export tRenderBatchData
	{
	public:
		enum tBehaviorFlags
		{
			cBehaviorIgnoreStats = ( 1 << 0 ),
		};

		const tRenderState*				mRenderState;
		const tMaterial*				mMaterial;
		const tVertexFormatVRam*		mVertexFormat;
		const tGeometryBufferVRam*		mGeometryBuffer;
		const tIndexBufferVRam*			mIndexBuffer;
		u32								mVertexCount;
		u32								mBaseVertexIndex;
		u32								mPrimitiveCount;
		u32								mBaseIndexIndex;
		u32								mPrimitiveType;
		u32								mBehaviorFlags;

		inline tRenderBatchData( ) { fZeroOut( this ); }
		inline b32 fValid( ) const { return mRenderState && mVertexFormat && mGeometryBuffer && mIndexBuffer; }
		inline b32 operator==( const tRenderBatchData& other ) const { return fMemCmp( this, &other, sizeof( *this ) ) == 0; }
		inline b32 operator!=( const tRenderBatchData& other ) const { return fMemCmp( this, &other, sizeof( *this ) ) != 0; }
		inline b32 operator< ( const tRenderBatchData& other ) const { return fMemCmp( this, &other, sizeof( *this ) ) <  0; }
		inline b32 operator<=( const tRenderBatchData& other ) const { return fMemCmp( this, &other, sizeof( *this ) ) <= 0; }
		inline b32 operator> ( const tRenderBatchData& other ) const { return fMemCmp( this, &other, sizeof( *this ) ) >  0; }
		inline b32 operator>=( const tRenderBatchData& other ) const { return fMemCmp( this, &other, sizeof( *this ) ) >= 0; }

		inline void fRenderInstance( const tDevicePtr& device ) const
		{
			mIndexBuffer->fRender( 
				device, 
				mVertexCount, 
				mBaseVertexIndex, 
				mPrimitiveCount, 
				mBaseIndexIndex, 
				( tIndexFormat::tPrimitiveType )mPrimitiveType );
		}

		///
		/// \brief Apply the batch state without actually rendering any instances. This method will check against the previously set batch to avoid setting any redundant state.
		void fApplyBatch( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& prevBatch = tRenderBatchData( ) ) const;
		void fApplyBatchWithoutMaterial( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& prevBatch = tRenderBatchData( ) ) const;
	};

	///
	/// \brief Shared object type encapsulating a render batch.
	class base_export tRenderBatch :
		public		tUncopyable,
		protected	tRenderBatchData,
		public		tRefCounter
	{
		define_class_pool_new_delete( tRenderBatch, 256 );
	private:
		mutable tRenderBatch* mXparentClone;

	public:
		~tRenderBatch( );

		///
		/// \brief Create a unique render batch representing the specified render data. There can only
		/// be one tRenderBatch object representing the specified tRenderBatchData. This way, render batches
		/// can safely be compared by pointer value and treated as unique keys.
		static tRenderBatchPtr fCreate( const tRenderBatchData& data );

		///
		/// \brief Creates a secondary version of itself which has transparent render state.
		void fCreateXparentClone( ) const;

		///
		/// \brief Access the transparent clone
		const tRenderBatch* fXparentClone( ) const { return mXparentClone; }

		///
		/// \brief Apply batch state (see fApplyBatch), and then proceed to render all consecutive instances in this batch.
		/// \note When calling this method, you should NOT call fApplyBatch, as it will be handled internally.
		u32 fRenderInstances( 
			const tDevicePtr& device, 
			const tRenderContext& context, 
			const tDisplayList& dl, 
			u32 startingInstanceIndex,
			const tRenderBatchData& prevBatch ) const;

		///
		/// \brief We only allow const/read-only to underlying batch data. To modify this data, you must create
		/// a new tRenderBatch object via the fCreate static function.
		inline const tRenderBatchData& fBatchData( ) const { return *static_cast<const tRenderBatchData*>( this ); }

		inline b32 fIgnoreStats( ) const { return mBehaviorFlags & cBehaviorIgnoreStats; }

	private:

		explicit tRenderBatch( const tRenderBatchData& data );
	};

}}


#endif//__tRenderBatch__
