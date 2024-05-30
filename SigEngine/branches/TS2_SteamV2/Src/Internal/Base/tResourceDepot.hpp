#ifndef __tResourceDepot__
#define __tResourceDepot__
#include "tResource.hpp"
#include "tHashTable.hpp"

namespace Sig
{
	class tFilePackage;

	///
	/// \brief The resource depot stores and manages tResource objects, providing fast
	/// table lookup access by resource id. It provides a central point of service for
	/// updating the asynchronous loading of resources.
	class base_export tResourceDepot : public tUncopyable, public tRefCounter
	{
		friend class tResource;
		friend class tFilePackage;
	public:
		typedef tHashTable<tResourceId,tResource*> tResourceTable;
		typedef tGrowableArray<tResource*> tLoadingResourceList;
		typedef tGrowableArray<tFilePackage*> tFilePackageList;
		typedef tDelegate<tResource* ( const tResourceId& rid, tResourceDepot& )> tAllocResource;

	private:
		tFilePathPtr			mRootPath;
		tResourceTable			mResourceTable;
		tLoadingResourceList	mLoadingResources;
		tFilePackageList		mFilePackages;

	public:
		tResourceDepot( );
		~tResourceDepot( );

		///
		/// \brief Obtain the root path.
		const tFilePathPtr& fRootPath( ) const { return mRootPath; }

		///
		/// \brief Sets the physical root directory; this should be set prior to 
		/// loading resources (happens automatically within tApplication startup).
		void fSetRootPath( const tFilePathPtr& rootPath );

		///
		/// \brief Check if an actual hard-copy of the resource exists on the file system. Doesnt include packages
		b32 fResourceExistsOnFileSystem( const tFilePathPtr& path ) const;

		///
		/// \brief Check if a resource can be found (in a package if using a package, on the HDD if not)
		b32 fResourceExists( const tFilePathPtr& path ) const;

		///
		/// \brief Query for the resource pointer. If the resource is not found, a new
		/// resource record will be created.
		/// \return A valid resource pointer, either newly created, or the record that
		/// already existed in the depot.
		/// \note The resource will not be loaded by this call, though it may have already
		/// been loaded prior to your call to fQuery. To load/unload, you must use
		/// methods on the resource object itself.
		tResourcePtr fQuery( const tResourceId& rid );

		///
		/// \brief Convenience method, equivalent to the following:
		/// resource = depot->fQuery( rid );
		/// resource->fLoadDefault( lcid );
		/// resource->fBlockUntilLoaded( );
		tResourcePtr fQueryLoadBlock( const tResourceId& rid, const tResource::tLoadCallerId& lcid );

		///
		/// \brief Convenience method, same as above, how ever will not try to load any previously requested resources.
		///  only safe to use this on resources with no dependencies, such as textures.
		tResourcePtr fQueryLoadBlockNoDependencies( const tResourceId& rid, const tResource::tLoadCallerId& lcid );


		///
		/// \brief Should be called once per frame (could be called more often, but might
		/// lead to stuttering frame rate) to check the status of async resources.
		/// \param timeAllotted Maximum time in milliseconds for the function to run; note
		///	that it is still possible the function will take longer, but it will
		///	return as soon as it has realized it has gone over. For this reason,
		///	you may want to pass a smaller value than the actual time you can afford.
		/// \note This method is guaranteed to update at least one file, even if you pass 0
		/// for the allotted time.
		void fUpdateLoadingResources( f32 timeAllottedMs );

		///
		/// \brief Should generally not be called! It will only update one resource, not considering that
		///   dependent resources may be waiting to get loaded first.
		void fUpdateLoadingOneResource( tResource* res );
		

		///
		/// \brief Obtain a list of all resources of the specified type.
		void fQueryByType( Rtti::tClassId cid, tGrowableArray<tResourcePtr>& resources );

		///
		/// \brief Scan the hard-drive, or else if supplied, a list of file packages, for commond resource types within the specified resourceFolders
		void fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount );

		///
		/// \brief Force reloads all resources of the specified type.
		void fReloadResourcesByType( const tGrowableArray<Rtti::tClassId>& cids );

		///
		/// \brief Adds/removes a unique load reference to all resources of the specified class id types
		void fReserveAll( const tGrowableArray<Rtti::tClassId>& cids, b32 unreserve = false );

		///
		/// \brief Find out how many resources are currently loading.
		inline u32 fLoadingResourceCount( ) const { return mLoadingResources.fCount( ); }

		///
		/// \brief Find out how many resources total are in the system.
		inline u32 fResourceCount( ) const { return mResourceTable.fGetItemCount( ); }

		///
		/// \brief Find out how many resources with load refs are still in the system.
		u32 fResourcesWithLoadRefsCount( ) const;

	private:
		void fRemoveResource( tResource* res );
		void fRemoveLoadingResource( tResource* res );
		void fAddLoadingResource( tResource* res );
	};

	typedef tRefCounterPtr<tResourceDepot> tResourceDepotPtr;
}


#endif//__tResourceDepot__
