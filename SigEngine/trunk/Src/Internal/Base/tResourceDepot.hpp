#ifndef __tResourceDepot__
#define __tResourceDepot__
#include "tResource.hpp"
#include "tHashTable.hpp"
#include "tResourceProvider.hpp"

#ifdef target_tools
#include "Gfx/tTextureReference.hpp"
#endif

namespace Sig
{
	///
	/// \brief The resource depot stores and manages tResource objects, providing fast
	/// table lookup access by resource id. It provides a central point of service for
	/// updating the asynchronous loading of resources.
	class base_export tResourceDepot : public tUncopyable, public tRefCounter
	{
		friend class tResource;
	public:
		typedef tHashTable<tFilePathPtr, tFilePathPtr> tRemapTable;
		typedef tHashTable<tResourceId, tResource*> tResourceTable;
		typedef tGrowableArray<tResource*> tLoadingResourceList;
		typedef tGrowableArray<tResourceProviderPtr> tResourceProviderList;
		typedef tDelegate<tResource* ( const tResourceId& rid, tResourceDepot& )> tAllocResource;

	private:
		tResourceProviderList	mResourceProviders;
		tRemapTable				mRemapTable;
		tResourceTable			mResourceTable;
		tLoadingResourceList	mLoadingResources;
		f32						mResourceLoadingTimeoutMS;

	public:
		tResourceDepot( );
		~tResourceDepot( );

		///
		/// \brief Update target time limit for fUpdateLoadingResources().
		/// \note This is a complete hack. We should be processing the 
		///  loading of files on another thread but until then we need 
		///  this variable to control the time spent loading assets per frame.
		void fSetResourceLoadingTimeoutMS( f32 ms );

		///
		/// \brief Add a resource provider. Resource providers are queried in the same order that they are added.
		void fAddResourceProvider( const tResourceProviderPtr& provider );

		///
		/// \brief Remove a resource provider.
		void fRemoveResourceProvider( const tResourceProviderPtr& provider );

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
		/// \brief Should be called once per frame to update async loads.
		/// \note To tweak time alloted call fSetResourceLoadingTimeoutMS()
		///  but keep in mind that the value is a guideline and not strictly
		///  enforced. This method is guaranteed to update at least one file.
		void fUpdateLoadingResources( );


		///
		/// \brief Cancels all currently loading resources.
		void fCancelLoadingResources( );

		///
		/// \brief Should generally not be called! It will only update one resource, not considering that
		///   dependent resources may be waiting to get loaded first.
		void fUpdateLoadingOneResource( tResource* res );
		
		///
		/// \brief Obtain a list of all resources of the specified type.
		void fQueryByType( Rtti::tClassId cid, tGrowableArray<tResourcePtr>& resources ) const;

		///
		/// \brief Obtain a list of all resources of the specified types.
		void fQueryByTypes( const tGrowableArray<Rtti::tClassId>& cids, tGrowableArray<tResourcePtr>& resources ) const;

		///
		/// \brief Query each resource provider, or else if supplied, a list of file packages, for common resource types within the specified resourceFolders
		void fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const;

		///
		/// \brief Query each resource provider, or else if supplied, a list of file packages for the files of the specified type within the resource folders
		void fQueryResourcesByFolder( 
			tFilePathPtrList& filePaths, 
			const tFilePathPtr & ext, 
			const tFilePathPtr resourceFolders[], 
			u32 resourceFolderCount ) const;

		///
		/// \brief Force reloads all resources of the specified type.
		void fReloadResourcesByType( const tGrowableArray<Rtti::tClassId>& cids );

		///
		/// \brief Find out how many resources are currently loading.
		inline u32 fLoadingResourceCount( ) const { return mLoadingResources.fCount( ); }

		///
		/// \brief Find out how many resources total are in the system.
		inline u32 fResourceCount( ) const { return mResourceTable.fGetItemCount( ); }

		///
		/// \brief Find out how many resources with load refs are still in the system.
		u32 fResourcesWithLoadRefsCount( ) const;

		///
		/// \brief Remap enables a "bait-and-switch" for the resource depot. A resource
		///  Query() might request asset "res/a.tga" and the remap can say "nope, you get
		///  'res/B.tga' instead." This allows for easy swapping of whole groups of assets
		///  automatically!
		void fAddRemap( const tFilePathPtr& from, const tFilePathPtr& to );
		void fClearRemapTable( );
		void fDumpRemapTable( ) const;

		///
		/// \brief Call fUpdateForBlockingLoad( ) on all resource providers
		void fUpdateForBlockingLoad( );
		
#ifdef target_tools
		///
		/// \brief Any easy way, for base to load textures in tools. When assets haven't been generated yet.
		Gfx::tTextureReference fToolsTextureQuery( const tFilePathPtr& path, Math::tVec2f& sizeOut );

		///
		/// \brief Initialize this from tools to allow the base to query textures.
		typedef tDelegate< Gfx::tTextureReference ( const tFilePathPtr& path, Math::tVec2f& sizeOut ) > tToolsTextureDelegate;
		static tToolsTextureDelegate sToolsTextureDelegate;
#endif

		void fAddToStatText( std::wstringstream& currentStats );

	private:
		void fRemoveResource( tResource* res );
		void fRemoveLoadingResource( tResource* res );
		void fAddLoadingResource( tResource* res );

		tResourceId fResolveRemap( const tResourceId& in_rid );
	};

	typedef tRefCounterPtr<tResourceDepot> tResourceDepotPtr;
}


#endif//__tResourceDepot__
