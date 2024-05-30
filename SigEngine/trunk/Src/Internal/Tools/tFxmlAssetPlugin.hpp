#ifndef __tFxmlAssetGen__
#define __tFxmlAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Fxml { class tFile; }

	///
	/// \brief Converts .mshml files exported from maya to .mshb files suitable for the game engine.
	class tools_export tFxmlAssetGenPlugin : public iAssetGenPlugin
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

		void fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Fxml::tFile& fxmlFile );
	};

	///
	/// \brief Asset plugin acces point for .mshml-related plugins.
	class tools_export tFxmlAssetPlugin : public iAssetPlugin
	{
		tFxmlAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tFxmlAssetGen__

