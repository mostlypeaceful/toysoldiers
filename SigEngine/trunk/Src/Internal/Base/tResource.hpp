#ifndef __tResource__
#define __tResource__
#include "tDelegate.hpp"
#include "tResourceId.hpp"
#include "tResourceLoader.hpp"
#include "tAsyncFileReader.hpp"
#include "Memory/tPool.hpp"

namespace Sig
{
	class tResourceDepot;
	class tResourceProvider;

	typedef tGrowableArray<tResourcePtr> tResourcePtrList;

	///
	/// \brief The tResource class is a lightweight wrapper and manager of the
	/// underlying resource buffer. Facilitates asynchronous loading and resource 
	/// initialization in conjunction with the tResourceLoader class. tResource
	/// objects are expected to be owned/stored/created by a tResourceDepot object.
	class base_export tResource : public tUncopyable, public tRefCounter, public tResourceId
	{
		friend class tResourceLoader;
		friend class tResourceDepot;
		define_class_pool_new_delete( tResource, 1024 );
		declare_null_reflector( );

	public:

		typedef tResourceLoader::tLoadResult tLoadResult;

		///
		/// \brief Used as a unique id for each individual load caller.
		/// \see fLoad
		typedef void* tLoadCallerId;

		typedef tGrowableArray<const tResource*> tVisitedList;

		///
		/// \brief Global function signature for type specific post load and pre destroy resource callbacks.
		typedef b32 (*tTypeSpecificResourceLifetimeCb)( tResource& theResource, tGenericBuffer* theBuffer );

		///
		/// \brief Global function signature for querying whether a resource is a sub-resource.
		typedef b32 (*tTypeSpecificIsSubResourceCheckCb)( const tResource& primary, const tResource& sub, tVisitedList& visited );

		///
		/// \brief Global function signature for gathering all sub-resources
		typedef void (*tTypeSpecificGatherSubResourcesCb)( const tResource& theResource, tResourcePtrList& subs );

		///
		/// \brief Contains miscellaneous resource type-specific properties, including
		/// type-specific lifetime callbacks.
		struct base_export tTypeSpecificProperties
		{
			enum
			{
				cIsDirectResource = ( 1 << 0 )
			};

			u16									mFlags;
			u16									mOffsetToHeaderSize;
			tTypeSpecificResourceLifetimeCb		mPostLoad;
			tTypeSpecificResourceLifetimeCb		mPreDestroy;
			tTypeSpecificIsSubResourceCheckCb	mIsSubResource;
			tTypeSpecificGatherSubResourcesCb	mGatherSubResources;

			inline tTypeSpecificProperties( )
				: mFlags( 0 ), mOffsetToHeaderSize( 0 ), mPostLoad( 0 ), mPreDestroy( 0 ), mIsSubResource( 0 ), mGatherSubResources( 0 ) { }
		};

		///
		/// \brief Global singleton table mapping a resource's type id to its type specific properties (if any).
		class base_export tTypeSpecificPropertiesTable :
			public tHashTable< Rtti::tClassId, tTypeSpecificProperties, tHashTableExpandOnlyResizePolicy >
		{
			declare_singleton( tTypeSpecificPropertiesTable );
		};

		///
		/// \brief Event for notifying callers of load completion status.
		typedef tEvent<void ( tResource& theResource, b32 success )> tOnLoadComplete;

		///
		/// \brief List of load caller ids.
		typedef tGrowableArray<tLoadCallerId> tLoadCallerIdArray;

	public:

		static tFilePathPtr fConvertPathAddB( const tFilePathPtr& path );
		static tFilePathPtr fConvertPathAddBB( const tFilePathPtr& path );
		static tFilePathPtr fConvertPathML2B( const tFilePathPtr& path );
		static tFilePathPtr fConvertPathPK2B( const tFilePathPtr& path );
		static tFilePathPtr fConvertPathSubB( const tFilePathPtr& path );
		static tFilePathPtr fConvertPathSubBB( const tFilePathPtr& path );
		static tFilePathPtr fConvertPathB2ML( const tFilePathPtr& path );
		static tFilePathPtr fConvertPathB2PK( const tFilePathPtr& path );

	private:

		tResourceDepot*						mOwner;
		tRefCounterPtr< tResourceProvider >	mProvider;
		b16									mDisableLoadCompleteEvent;
		b16									mLockLoadCompleteEvent;
		tOnLoadComplete						mOnLoadComplete;
		tResourceLoaderPtr					mLoader;
		mutable tGenericBufferPtr			mBuffer;
		tLoadCallerIdArray					mLoadCallerIds; // these can be removed, or reworked to simply be 1 pointer, or a flag. not doing that now because we're on a time crunch and resource will need to be rebuilt.
		u64									mFileTimeStamp;

		if_logging( public: typedef tGrowableArray<tFilePathPtr> tPaperTrail; )
		if_logging( private: tPaperTrail mPaperTrail; )

	public:

		tResource( const tResourceId& rid, tResourceProvider* provider );
		~tResource( );

	private:

		///
		/// \brief Free underlying resource buffer
		inline void					fFreeResourceBuffer( ) { if( !mBuffer.fNull( ) ) { mBuffer->fFree( ); mBuffer.fRelease( ); } }

	public:

		///
		/// \brief Access the resource buffer pointer
		inline Sig::byte*			fGetResourceBuffer( ) const { return mBuffer.fNull( ) ? 0 : mBuffer->fGetBuffer( ); }

		///
		/// \brief Access the size of the resource buffer.
		inline u32					fGetResourceBufferSize( ) const { return mBuffer.fNull( ) ? 0 : mBuffer->fGetBufferSize( ); }

		///
		/// \brief Returns a positive value in bytes if the resource will need temporary space.
		inline u32					fGetTemporaryResourceBufferSize( ) const { return mBuffer.fNull( ) ? 0 : mBuffer->fGetBufferSize( ); }

		///
		/// \brief Access to the file loader
		tResourceLoader *			fGetLoader( ) const { return mLoader; }

		///
		/// \brief Find out if the resource is loaded or not.
		inline b32					fLoaded( ) const { return fGetResourceBuffer( ) && !mLoader && !mDisableLoadCompleteEvent; }

		///
		/// \brief Find out if the resource is loading or not.
		inline b32					fLoading( ) const { return mLoader || mDisableLoadCompleteEvent; }

		///
		/// \brief Find out if the resource failed to load.
		inline b32					fLoadFailed( ) const { return mLoadCallerIds.fCount( ) > 0 && !mLoader && !fGetResourceBuffer( ); }

		///
		/// \brief Returns true if someone has ever called fLoad of some sort
		inline b32					fHasLoadCaller( ) const { return (mLoadCallerIds.fCount( ) != 0); }

		/// 
		/// \brief Access the resource id.
		inline const tResourceId&	fGetResourceId( ) const { return static_cast<const tResourceId&>( *this ); }

		///
		/// \brief Obtain the absolute physical path to the resource (should be used only when actually hitting actual physical IO/media devices).
		tFilePathPtr				fAbsolutePhysicalPath( ) const;

		///
		/// \brief Access the owner of this resource.
		void						fSetOwner( tResourceDepot* owner );
		inline tResourceDepot*		fGetOwner( ) { return mOwner; }

		///
		/// \brief Access the provider of this resource.
		inline tResourceProvider*	fGetProvider( ) const { return mProvider.fGetRawPtr( ); }

		///
		/// \brief Get the timestamp associated with the underlying file.
		inline u64					fFileTimeStamp( ) const { return mFileTimeStamp; }

		///
		/// \brief Check to see if the resource is of type class. Sometimes this is necessary to
		/// prevent an assert when doing an fCast()
		template<class t>
		inline b32					fIsType( ) const { return Rtti::fGetClassId<t>( ) == fGetClassId( ); }

		///
		/// \brief Cast the resource to the proper derived type. This method will sigassert in debug
		/// builds if the resource was not created with the specified type's class id.
		template<class t>
		inline t*					fCast( ) const { sigassert( Rtti::fGetClassId<t>( ) == fGetClassId( ) ); return ( t* )fGetResourceBuffer( ); }

		///
		/// \brief Add a callback to get called when the resource loading is complete; note that if the
		/// resource is already loaded, the callback will fire immediately.
		void						fCallWhenLoaded( tOnLoadComplete::tObserver& cb );

		///
		/// \brief
		void						fRemoveLoadCallback( tOnLoadComplete::tObserver& cb );

		///
		/// \brief Loads a file from the most suitable package file if any file packages are registered,
		/// i.e. equivalent to fLoadFromPackage; otherwise, if no file packages are registered to this
		/// resource, it loads the file using fLoadStandard.
		void						fLoadDefault( const tLoadCallerId& lcid );

		///
		/// \brief Load the resource.
		/// \param loader Expects a non-null pointer to a derived tResourceLoader type. Use fLoadFromPackage
		/// if you want to load from an available file package, or fLoadStandard to load an isolated file.
		/// \note All resource loading is asynchronous. The proper way of finding out when a resource
		/// is finished loading is by specifying callbacks through the fCallWhenLoaded method. Alternatively, 
		/// you could poll each frame, but that's generally considered poor form.
		void						fLoad( const tLoadCallerId& lcid, const tResourceLoaderPtr& loader );

		///
		/// \brief This method will effectively unload and then re-load the given resource. While calling
		/// fLoad -> fUnload could also work, if the caller is not a load caller, then the resource will
		/// not actually get unloaded. This method bypasses the idea of ownership, forcing the effective
		/// refresh of the resource in question. Additionally, it will only reload if the file on disk
		/// is more recent than the current one.
		/// \return true if the file was reloaded, false if the resource was up to date
		b32							fReload( b32 saveDependents = true );

		///
		/// \brief Update the load progress; this should only be called by the owning
		/// resource depot, but could conceivably be called in another context if
		/// the resource weren't owned by a resource depot.
		tLoadResult					fUpdateLoad( );

		///
		/// \brief This method is purely for rare occasions where it is not imperative to
		/// load asynchronously.
		void						fBlockUntilLoaded( );

		///
		/// \brief Same as above, only will not try to load any dependencies.
		///  Mostly only safe to use with textures.
		void						fBlockUnitLoadedNoDependencies( );

		///
		/// \brief Calling this method means the resource will no longer issue tOnLoadComplete event
		/// callbacks on its own. Instead, callers of this method are taking on responsibility to
		/// issue these callbacks manually via the fIssueLoadCompleteEvent method.
		/// \note Users should use caution when calling this method. This is generally used only for
		/// implementing a more sophisiticated load-completion notification chain, involving dependent
		/// resources, etc. See tLoadInPlaceFileBase for an example.
		void						fDisableLoadCompleteEvent( );

		///
		/// \brief Called for a manual notification of any pending load complete event callbacks. Additionally,
		/// calling this method will reset the override variable for disabling load completion events.
		/// \note Not recommended for general use.
		/// \see fDisableLoadCompleteEvent.
		void						fIssueLoadCompleteEvent( b32 success );

		///
		/// \brief Check if the specified resource is a dependency of 'this'.
		b32							fIsSubResource( const tResource& testIfSub, tVisitedList& visited ) const;

		///
		/// \brief See if this resource is loaded but waiting on dependencies.
		b32							fWaitingOnDependents( ) const;

		///
		/// \brief Get a list of all immediate sub-resources.
		void						fGatherSubResources( tResourcePtrList& subs ) const;

		///
		/// \brief Debug tracking of who tried to load me
		if_logging( void fAddToPaperTrail( const tFilePathPtr& paperTrail ) { mPaperTrail.fPushBack( paperTrail ); } )
		if_logging( const tPaperTrail& fPaperTrail( ) const { return mPaperTrail; } )

	private:
		///
		/// \brief Store who called the load function.
		void						fAddLoadCallerId( const tLoadCallerId& lcid );

		///
		/// \brief Internal method for initiating a load.
		void						fInitiateLoad( const tResourceLoaderPtr& loader );

		///
		/// \brief Internal method to unload the specified resource. This generally just means deallocating the resource
		/// buffer, but will also invoke any pre-destroy callbacks that need to fire.
		void						fInitiateUnload( );

		///
		/// \brief Adds the object to the owner's load list; this means the object will get its
		/// fUpdateLoad method called when the owner depot object's update method is called.
		void						fAddToLoadList( );

		///
		/// \brief Called internally to perform post-load cleanup/notifications.
		/// \return True if the load is really complete, false if the loader has more stages.
		b32							fOnLoadComplete( tResourceLoader::tLoadResult loadResult );
		
	public:
		///
		/// \brief This should ONLY be called from the ref counter decrement function.
		void						fUnloadAndRemoveFromDepot( );
	};

}

#endif//__tResource__

