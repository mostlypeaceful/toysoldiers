#ifndef __tSigmlAssetGen__
#define __tSigmlAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Sigml { class tFile; }

	///
	/// \brief Converts .mshml files exported from maya to .mshb files suitable for the game engine.
	class tools_export tSigmlAssetGenPlugin : public iAssetGenPlugin
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

		void fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Sigml::tFile& sigmlFile );
	};

	///
	/// \brief Asset plugin acces point for .mshml-related plugins.
	class tools_export tSigmlAssetPlugin : public iAssetPlugin
	{
		tSigmlAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tSigmlAssetGen__

