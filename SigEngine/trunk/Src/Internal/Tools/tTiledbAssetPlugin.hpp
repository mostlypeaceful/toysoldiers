#ifndef __tTiledbAssetGen__
#define __tTiledbAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Tiledml { class tFile; }

	///
	/// \brief Converts .tiledml files exported from Maya to .tiledb files suitable for the game engine.
	class tools_export tTiledmlAssetGenPlugin : public iAssetGenPlugin
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

		void fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Tiledml::tFile& tiledbFile );
	};

	///
	/// \brief Asset plugin access point for .tiledml-related plugins.
	class tools_export tTiledmlAssetPlugin : public iAssetPlugin
	{
		tTiledmlAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tSigmlAssetGen__

