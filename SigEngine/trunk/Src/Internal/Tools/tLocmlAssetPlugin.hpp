#ifndef __tLocmlAssetGen__
#define __tLocmlAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Locml { class tFile; }

	///
	/// \brief Converts .Locml files (localization files).
	class tools_export tLocmlAssetGenPlugin : public iAssetGenPlugin
	{
        virtual tLoadInPlaceResourcePtr* fAddDependencyInternal( 
            tLoadInPlaceFileBase& outputFileObject, 
            const tFilePathPtr& dependencyPath );

		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles );

		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo );

		void fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Locml::tFile& locmlFile );
	};

	///
	/// \brief Asset plugin acces point for .Locml-related plugins.
	class tools_export tLocmlAssetPlugin : public iAssetPlugin
	{
		tLocmlAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tLocmlAssetGen__

