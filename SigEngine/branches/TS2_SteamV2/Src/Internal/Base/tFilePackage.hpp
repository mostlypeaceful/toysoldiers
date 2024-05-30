#ifndef __tFilePackage__
#define __tFilePackage__
#include "tResource.hpp"
#include "tAsyncFileReader.hpp"

namespace Sig
{
	class tFilePackageFile;

	///
	/// \brief TODO document
	class base_export tFilePackage : tUncopyable, public tRefCounter
	{
		friend class tResource;

	private:

		tResource::tOnLoadComplete::tObserver	mOnLoadComplete;
		tResource::tLoadCallerId				mLoadCallerId;
		tResourcePtr							mFilePackage;
		tAsyncFileReaderPtr						mFileReader;
		tDynamicArray<tResourcePtr>				mSubFiles;
		b32										mLoadAllFiles;

	public:

		tFilePackage( );
		~tFilePackage( );

		///
		/// \brief Load the specified file package.
		/// \param loadAllFiles If true, the file package will iterate over all sub files
		/// after the header is loaded and initiate loading all sub files.
		void fLoad( const tResource::tLoadCallerId& lcid, tResourceDepot& depot, const tFilePathPtr& path, b32 loadAllFiles = false );

		///
		/// \brief Unload the file package; this will also unload any sub files that this
		/// file package has loaded.
		void fUnload( );

		///
		/// \brief Blocks until the file header is loaded (not sub files). This method will also
		/// return if there is an error during the file loading process, hence you should verify
		/// that the file was actually successfully loaded (i.e., call fLoaded).
		void fBlockUntilLoaded( );

		///
		/// \brief Loads the file contained in this file package; if the file is not contained
		/// in this file package, a null resource pointer is returned; otherwise, a resource
		/// load is kicked off (if it wasn't already in the process of loading or loaded).
		tResourcePtr fLoadSubFile( const tResource::tLoadCallerId& lcid, const tResourceId& rid );

		///
		/// \brief Unloads a specific sub file contained in this file package; if the file is not contained
		/// in this file package, then this method has no effect.
		void fUnloadSubFile( const tResourcePtr& subFile );

		///
		/// \brief Query for the number of sub files
		inline u32 fGetNumSubFiles( ) const { return mSubFiles.fCount( ); }

		///
		/// \brief Get the i'th sub file resource ptr.
		inline tResourcePtr fGetIthSubFile( u32 i ) { return mSubFiles[ i ]; }

		///
		/// \brief Query for whether the sub-files for which this file package has initiated loads are finished loading.
		b32 fQuerySubFilesLoaded( );

		///
		/// \brief Blocks until all sub files that have had their fLoad method called are finished loading.
		void fBlockUntilSubFilesLoaded( );

		///
		/// \brief Retrieve a pointer to the actual file package file, mostly for debugging purposes.
		/// You shouldn't have to use this for any normal purpose.
		const tFilePackageFile* fAccessFile( ) const;

		///
		/// \brief Determine whether the file package's underlying file header is loaded.
		/// \note This method refers only to the file header; if any sub files are loaded,
		/// this method does not take them into account.
		inline b32 fLoaded( ) const { return !mFilePackage.fNull( ) && mFilePackage->fLoaded( ); }

		///
		/// \brief Determine whether the file package's underlying file header is currently loading.
		/// \note This method refers only to the file header; if any sub files are currently loading,
		/// this method does not take them into account.
		inline b32 fLoading( ) const { return !mFilePackage.fNull( ) && mFilePackage->fLoading( ); }

	private:

		tResourceLoaderPtr fCreateResourceLoader( const tResourcePtr& resourcePtr, const tFilePackageFile* filePackageFile, u32 ithHeader );
		void fLoadSubFile( const tResource::tLoadCallerId& lcid, const tResourcePtr& resourcePtr, const tFilePackageFile* filePackageFile, u32 ithHeader );
		void fOnLoadComplete( tResource& resource, b32 success );
	};

	typedef tRefCounterPtr< tFilePackage > tFilePackagePtr;

}


#endif//__tFilePackage__

