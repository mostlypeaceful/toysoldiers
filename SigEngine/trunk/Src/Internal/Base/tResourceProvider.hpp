//------------------------------------------------------------------------------
// \file tResourceProvider.hpp - 31 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tResourceProvider__
#define __tResourceProvider__
#include "tResourceLoader.hpp"

namespace Sig
{
	class iCloudStorage;
	class iCloudStoragePtr;
	class tFilePackage;

	///
	/// \class tResourceProvider
	/// \brief Provides resources to the resource depot
	class base_export tResourceProvider : public tRefCounter
	{
	private:
		tFilePathPtr mRootPath;

	public:
		virtual ~tResourceProvider( );

		///
		/// \brief Obtain the root path.
		const tFilePathPtr& fRootPath( ) const { return mRootPath; }

		///
		/// \brief Sets the physical root directory; this should be set prior to 
		/// loading resources (happens automatically within tApplication startup).
		void fSetRootPath( const tFilePathPtr& rootPath );

		///
		/// \brief Create a loader for the specified resource
		virtual tResourceLoaderPtr fCreateResourceLoader( const tResourcePtr& resourcePtr ) = 0;

		///
		/// \brief Check if a resource can be found
		virtual b32 fResourceExists( const tFilePathPtr& path ) const = 0;

		///
		/// \brief Query for common resource types within the specified resourceFolders
		virtual void fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const = 0;

		///
		/// \brief Query for the files of the specified type within the resource folders
		virtual void fQueryResourcesByFolder( 
			tFilePathPtrList& filePaths, 
			const tFilePathPtr & ext, 
			const tFilePathPtr resourceFolders[], 
			u32 resourceFolderCount ) const = 0;

		///
		/// \brief Perform any updates necessary to advance resource loading
		virtual void fUpdateForBlockingLoad( ) { }
	};

	typedef tRefCounterPtr< tResourceProvider > tResourceProviderPtr;


	///
	/// \class tFileSystemResourceProvider
	/// \brief Provides resources from the file system to the resource depot
	class base_export tFileSystemResourceProvider : public tResourceProvider
	{
	public:
		// virtuals overriden from tResourceProvider
		virtual tResourceLoaderPtr fCreateResourceLoader( const tResourcePtr& resourcePtr );
		virtual b32 fResourceExists( const tFilePathPtr& path ) const;
		virtual void fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const;
		virtual void fQueryResourcesByFolder( 
			tFilePathPtrList& filePaths, 
			const tFilePathPtr & ext, 
			const tFilePathPtr resourceFolders[], 
			u32 resourceFolderCount ) const;
	};

	///
	/// \class tFilePackageResourceProvider
	/// \brief Provides resources from the file packages to the resource depot
	class base_export tFilePackageResourceProvider : public tResourceProvider, public tUncopyable
	{
	public:
		tFilePackageResourceProvider( );
		~tFilePackageResourceProvider( );

		// virtuals overriden from tResourceProvider
		virtual tResourceLoaderPtr fCreateResourceLoader( const tResourcePtr& resourcePtr );
		virtual b32 fResourceExists( const tFilePathPtr& path ) const;
		virtual void fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const;
		virtual void fQueryResourcesByFolder( 
			tFilePathPtrList& filePaths, 
			const tFilePathPtr & ext, 
			const tFilePathPtr resourceFolders[], 
			u32 resourceFolderCount ) const;

		void fAddPackage( tFilePackage& package );
		b32 fFindPackage( const tFilePathPtr& path, const tFilePackage*& packageOut, u32& headerIndexOut ) const;
		void fRemove( const tGrowableArray< tRefCounterPtr< tFilePackage > >& packages );

	private:
		tGrowableArray< tRefCounterPtr< tFilePackage > > mFilePackages;
	};

	typedef tRefCounterPtr< tFilePackageResourceProvider > tFilePackageResourceProviderPtr;

	///
	/// \class tCloudStorageResourceProvider
	/// \brief Provides resources from cloud storage to the resource depot
	class base_export tCloudStorageResourceProvider : public tResourceProvider, public tUncopyable
	{
	public:
		tCloudStorageResourceProvider( const iCloudStoragePtr& cloudStorage );
		~tCloudStorageResourceProvider( );

		// virtuals overriden from tResourceProvider
		virtual tResourceLoaderPtr fCreateResourceLoader( const tResourcePtr& resourcePtr );
		virtual b32 fResourceExists( const tFilePathPtr& path ) const;
		virtual void fQueryCommonResourcesByFolder( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount ) const;
		virtual void fQueryResourcesByFolder( 
			tFilePathPtrList& filePaths, 
			const tFilePathPtr & ext, 
			const tFilePathPtr resourceFolders[], 
			u32 resourceFolderCount ) const;
		virtual void fUpdateForBlockingLoad( );

	private:
		tRefCounterPtr< iCloudStorage > mCloudStorage;
	};
}

#endif//__tResourceProvider__
