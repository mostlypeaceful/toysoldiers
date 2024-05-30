#ifndef __tDisplayList__
#define __tDisplayList__
#include "tDrawCall.hpp"
#include "tIndexFormat.hpp"

namespace Sig { namespace Gfx
{
	class tScreen;

	///
	/// \brief Misc. display stats for a frame.
	struct base_export tDisplayStats
	{
		u32 mNumDrawCalls;
		u32 mBatchSwitches;
		tFixedArray< u32, tIndexFormat::cPrimitiveTypeCount > mPrimitiveCounts;

		tDisplayStats( ) { fReset( ); }
		void fReset( ) { fZeroOut( this ); }
		void fCombine( const tDisplayStats& other );

		inline u32 fTotalTriCount( ) const { return mPrimitiveCounts[ tIndexFormat::cPrimitiveTriangleList ] + mPrimitiveCounts[ tIndexFormat::cPrimitiveTriangleStrip ]; }
	};

	///
	/// \brief Base building block for more complex display lists. A glorified
	/// array of renderable instances.
	class base_export tDisplayList
	{
	protected:
		tGrowableArray< tDrawCall > mDrawCalls;
		mutable tDisplayStats		mStats;

	public:

		explicit tDisplayList( u32 capacity = 128 ) { mDrawCalls.fSetCapacity( capacity ); }

		inline const tDrawCall& operator[]( u32 i ) const { return mDrawCalls[ i ]; }

		typedef tDelegate<void ( const tDrawCall& )> tDummyHandler;

		///
		/// \brief Render the list of renderable instances using the currently bound render target.
		void fRender( tScreen& screen, const tRenderContext& context, const tDummyHandler& dummyHandler = tDummyHandler( ) ) const;

		///
		/// \brief Add a render instance.
		void fInsert( const tDrawCall& drawCall ) { if( drawCall.fValid( ) ) mDrawCalls.fPushBack( drawCall ); }

		///
		/// \brief See if the display list is empty.
		b32 fEmpty( ) const { return fCount( ) == 0; }

		///
		/// \brief See how many draw calls are in the list
		u32 fCount( ) const { return mDrawCalls.fCount( ); }

		///
		/// \brief This method should be called prior to adding any objects.
		void fInvalidate( ) { mStats.fReset( ); mDrawCalls.fSetCount( 0 ); }

		///
		/// \brief This method asserts that all draw calls are valid
		void fAssertValidity( ) const;

		///
		/// \brief Get stats. Note that these will be reset after each call to fInvalidate.
		const tDisplayStats& fGetStats( ) const { return mStats; }
	};

	class base_export tOpaqueDisplayList : public tDisplayList
	{
	public:
		void fSeal( );
	};

	class base_export tXparentDisplayList : public tDisplayList
	{
	public:
		void fSeal( );
	};

	///
	/// \brief Display list for screen space instances.
	class base_export tScreenSpaceDisplayList : public tDisplayList
	{
	public:
		void fSeal( );
	};


	///
	/// \brief Display list for objects in the world
	class base_export tWorldSpaceDisplayList
	{
	private:
		tOpaqueDisplayList		mOpaque;
		tXparentDisplayList		mXparent;

		tOpaqueDisplayList		mXparentWithDepthPrepass;
		tXparentDisplayList		mXparentWithDepth;
		

	public:

		///
		/// \brief Add a render instance.
		void fInsert( const tDrawCall& drawCall );

		///
		/// \brief A more optimal version of the above function if you already know this drawcall is transparent.
		void fInsert( const tDrawCall& drawCall, b32 xParent );

		///
		/// \brief This method should be called prior to adding any objects.
		void fInvalidate( );

		///
		/// \brief This method asserts validity on all underlying display lists
		void fAssertValidity( ) const;

		///
		/// \brief This method should be called after adding all objects.
		void fSeal( );

		///
		/// \brief Called to traverse the display list and render everything in it.
		void fRenderAll( tScreen& screen, const tRenderContext& context ) const;

		///
		/// \brief See if the display list is empty.
		b32 fEmpty( ) const { return mOpaque.fEmpty( ) && mXparent.fEmpty( ) && mXparentWithDepth.fEmpty( ); }

		const tOpaqueDisplayList& fOpaque( ) const { return mOpaque; }
		const tXparentDisplayList& fXparent( ) const { return mXparent; }

		const tOpaqueDisplayList& fXparentWithDepthPrepass( ) const { return mXparentWithDepthPrepass; }
		const tXparentDisplayList& fXparentWithDepth( ) const { return mXparentWithDepth; }
	};


}}


#endif//__tDisplayList__
