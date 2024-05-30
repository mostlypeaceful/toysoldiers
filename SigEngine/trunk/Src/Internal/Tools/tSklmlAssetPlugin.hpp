#ifndef __tSklmlAssetGen__
#define __tSklmlAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Sklml { class tFile; }

	///
	/// \brief Converts .sklml files exported from maya to .sklb files suitable for the game engine.
	class tools_export tSklmlAssetGenPlugin : public iAssetGenPlugin
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

		void fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Sklml::tFile& sigmlFile );
	};

	///
	/// \brief Asset plugin acces point for .sklml-related plugins.
	class tools_export tSklmlAssetPlugin : public iAssetPlugin
	{
		tSklmlAssetGenPlugin		mAssetGenPlugin;
	public:
		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tSklmlAssetGen__

