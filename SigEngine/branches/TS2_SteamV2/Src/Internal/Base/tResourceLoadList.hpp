#ifndef __tResourceLoadList__
#define __tResourceLoadList__
#include "tResource.hpp"

namespace Sig
{
	///
	/// \brief Data-wise, this class essentially represents a growable array of tResourcePtr.
	/// Functionality-wise, this class allows you to asynchronously load multiple resources
	/// (i.e., load without blocking), and receive a single call back when all resources have completed loading.
	///
	/// \note This class has two typical usage patterns: 1) as a base class, where you can simply use the virtual
	/// method fOnAllResourcesLoaded( ) to implement custom functionality after all resources have loaded; 2) as a
	/// a member or external object, where you can pass a delegate to fCallOnLoadComplete in order to receive
	/// notification of resource load completion.
	class base_export tResourceLoadList : public tUncopyable, public tRefCounter
	{
	private:
		typedef tGrowableArray< tResourcePtr >					tLoadList;
	public:
		typedef tEvent<void ( tResourceLoadList& loader )>		tOnLoadComplete;
	private:
		u32 									mLoadAttemptCount;
		u32 									mLoadSuccessCount;
		u32 									mLoadFailureCount;
		tLoadList								mLoadList;
		tResource::tOnLoadComplete::tObserver	mOnResourceLoaded;
		tOnLoadComplete							mOnLoadComplete;

	public:
		tResourceLoadList( );
		virtual ~tResourceLoadList( );

		///
		/// \brief Call to add a resource to the load list.
		void fAddToLoadList( const tResourcePtr& resource );

		///
		/// \brief Call to initiate the loading of all resources in the list.
		void fLoadAll( );

		///
		/// \brief Call to initiate unloading of all resources in the list.
		void fUnloadAll( );

		///
		/// \brief Add a delegate that will get called when all resource loading is complete.
		void fCallOnLoadComplete( tOnLoadComplete::tObserver& cb );

		u32 fGetLoadAttemptCount( ) const		{ return mLoadAttemptCount; }
		u32 fGetLoadSuccessCount( ) const		{ return mLoadSuccessCount; }
		u32 fGetLoadFailureCount( ) const		{ return mLoadFailureCount; }

		b32 fLoadComplete( ) const				{ return ( mLoadSuccessCount + mLoadFailureCount ) == mLoadAttemptCount; }
		b32 fLoadSuccessful( ) const			{ return mLoadSuccessCount == mLoadAttemptCount; }

		const tLoadList& fGetLoadList( ) const	{ return mLoadList; }

	private:

		void fLoad( const tResourcePtr& resource );
		void fUnload( const tResourcePtr& resource );
		void fOnResourceLoaded( tResource& theResource, b32 success );

	protected:

		virtual void fOnAllResourcesLoaded( );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tResourceLoadList );
}

#endif//__tResourceLoadList__

