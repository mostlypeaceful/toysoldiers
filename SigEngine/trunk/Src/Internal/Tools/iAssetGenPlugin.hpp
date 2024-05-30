#ifndef __iAssetGenPlugin__
#define __iAssetGenPlugin__

namespace Sig
{
	///
	/// \brief Contains a list of all the input file paths (absolute paths)
	/// that are used to generate the output file path (also an absolute path).
	struct tools_export tAssetGenPluginInputOutput
	{
		///
		/// \brief This is the original input file from which the actual list of input files were determined.
		tFilePathPtr		mOriginalInput;

		///
		/// \brief The list of input files which are required to generate mOutput.
		tFilePathPtrList	mInputs;

		///
		/// \brief The format of the "output" path is expected to be relative
		/// to a given platform's game folder; use fCreateRelativeOutputPath above.
		tFilePathPtr		mOutput;

		///
		/// \brief This member's value is only relevant in fProcessInputOutputs.
		/// The platform id for each platform that requires conversion will be added;
		/// i.e., the platforms that are up to date will not be added.
		tGrowableArray<tPlatformId> mPlatformsToConvert;
	};

	///
	/// \brief Provides a callback style interface for generating binary game assets; this plugin
	/// will be invoked from the AssetGen application, while scanning resource directories.
	class tools_export iAssetGenPlugin
	{
	public:

		///
		/// \brief Utility function for taking a file path relative to Res and converting it to an output path
		/// that is relative to a given platform's game folder (no specific platform); this method will swap the
		/// extension of inputFile with the specified outputExtension.
		static tFilePathPtr fCreateRelativeOutputPath( const tFilePathPtr& inputFile, const char* outputExtension );

		///
		/// \brief Take a relative output path (as created with fCreateRelativeOutputPath) and make an absolute path
		/// suitable for writing an actual file to disk.
		static tFilePathPtr fCreateAbsoluteOutputPath( tPlatformId outputPlatform, const tFilePathPtr& relativeOutputPath );

        ///
        /// \brief Adds dependencies to a tLoadInPlaceFileBase based on the input (i.e., intermediate file/xml file), converting
        /// the input file name to its binary equivalent. This is a utility method which strictly speaking has little to do
        /// with iAssetGenPlugin specifically, but is common functionality that is shared by many different types of asset converters.
        static tLoadInPlaceResourcePtr* fAddDependency( tLoadInPlaceFileBase& outputFileObject, const tFilePathPtr& dependencyPath );

		typedef tAssetGenPluginInputOutput		tInputOutput;
		typedef tGrowableArray<tInputOutput>	tInputOutputList;

		virtual ~iAssetGenPlugin( ) { }

		///
		/// \brief Called once for each set of potential input/output file sets
		/// (usually a complete set of the files in a given directory will be passed in
		/// as the 'immediateInputFiles'). It is the plugin writers responsbility to
		/// append output files that the plugin wants to write to the 'inputOutputsOut' list.
		/// I.e., your plugin might scane the immediateInputFiles, looking for files with
		/// extension .xyz; it will know that it wants to convert these to .uvw, and so will
		/// append an output *.uvw file name for each *.xyz it finds.
		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles ) = 0;

		///
		/// \brief Called after fDetermineInputOutputs; the list that is passed in
		/// will potentially have had elements culled if the AssetGen application determines
		/// that a given output is "up-to-date" based on the given inputs.
		/// \param indirectGenFilesAddTo Add files to this list while processing if the plugin
		/// requires these files to also be processed (for example, a plugin would add external
		/// file dependencies to this list, to ensure all necessary files will get generated).
		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo ) = 0;

		///
		/// \brief Each plugin expected to override this. Checking the dependencyPath for the extension and returning a tLoadInPlaceResourcePtr for the path.
		virtual tLoadInPlaceResourcePtr* fAddDependencyInternal( 
			tLoadInPlaceFileBase& outputFileObject, 
			const tFilePathPtr& dependencyPath ) = 0;

	};
}

#endif // __iAssetGenPlugin__
