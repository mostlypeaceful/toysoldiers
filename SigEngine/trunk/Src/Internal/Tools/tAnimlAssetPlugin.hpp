#ifndef __tAnimlAssetGen__
#define __tAnimlAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{

	///
	/// \brief Converts .animl files exported from maya to .anib files suitable for the game engine.
	class tools_export tAnimlAssetGenPlugin : public iAssetGenPlugin
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
	};

	///
	/// \brief Asset plugin acces point for .animl-related plugins.
	class tools_export tAnimlAssetPlugin : public iAssetPlugin
	{
		tAnimlAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tAnimlAssetGen__

