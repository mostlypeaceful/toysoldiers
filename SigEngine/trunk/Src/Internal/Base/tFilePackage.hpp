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
		debug_watch( tFilePackage );
		friend class tResource;

	private:
		tResource::tLoadCallerId				mLoadCallerId;
		tResourcePtr							mFilePackage;
		tAsyncFileReaderPtr						mFileReader;

	public:

		tFilePackage( );
		~tFilePackage( );

		///
		/// \brief Load the specified file package.
		/// \param loadAllFiles If true, the file package will iterate over all sub files
		/// after the header is loaded and initiate loading all sub files.
		void fLoad( const tResource::tLoadCallerId& lcid, tResourceDepot& depot, const tFilePathPtr& path );

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
		/// \brief creates a loader for the file contained in this file package; if the file is not contained
		/// in this file package, a null resource loader pointer is returned;
		tResourceLoaderPtr fCreateResourceLoader( const tResourcePtr& resourcePtr, const tFilePackageFile* filePackageFile, u32 ithHeader ) const;
		
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
	};

	typedef tRefCounterPtr< tFilePackage > tFilePackagePtr;

}


#endif//__tFilePackage__

